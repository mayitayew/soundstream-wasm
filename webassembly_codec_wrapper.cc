#include <emscripten/bind.h>
#include <emscripten/fetch.h>

#include <unordered_map>

#include "encode_and_decode_lib.h"
#include "lyra_decoder.h"
#include "lyra_encoder.h"
#include "model_buffer/model_buffer_interface.h"

// Encoder and Decoder.
std::unique_ptr<chromemedia::codec::LyraEncoder> encoder;
std::unique_ptr<chromemedia::codec::LyraDecoder> decoder;

std::unique_ptr<chromemedia::codec::LyraEncoder> encoder_32khz;
std::unique_ptr<chromemedia::codec::LyraDecoder> decoder_32khz;

std::unique_ptr<chromemedia::codec::LyraEncoder> encoder_16khz;
std::unique_ptr<chromemedia::codec::LyraDecoder> decoder_16khz;

std::unique_ptr<chromemedia::codec::LyraEncoder> encoder_8khz;
std::unique_ptr<chromemedia::codec::LyraDecoder> decoder_8khz;

std::atomic<int8_t> FETCHED_MODEL_COUNT(0);

// Forward declaration of encoder and decoder creator fucntions.
void CreateEncoder();
void CreateDecoder();
bool encoders_initialized = false;
bool decoders_initialized = false;

void asyncDownload(const std::string& url, const std::string& model_name);

class WebassemblyModelBuffer
    : public chromemedia::codec::ModelBufferInterface {
 public:
  WebassemblyModelBuffer(const std::vector<std::string>& model_names,
                           const std::string& base_url)
      : base_url_(base_url) {
    fetches_by_name_.reserve(model_names.size());
    for (const auto& model_name : model_names) {
      fetches_by_name_[model_name] = nullptr;
    }
  }

  ~WebassemblyModelBuffer() {
    for (const auto& [model_name, fetch] : fetches_by_name_) {
      if (fetch != nullptr) {
        emscripten_fetch_close(fetch);
      }
    }
  }

  void DownloadModels() {
    for (const auto& [model_name, fetch] : fetches_by_name_) {
      if (fetch == nullptr) {
        asyncDownload(ModelUrl(model_name), model_name);
      }
    }
  }

  void SetFetch(const std::string& model_name,
                emscripten_fetch_t* fetch) const {
    fetches_by_name_[model_name] = fetch;
  }

  uint64_t GetBufferSize(const std::string& model_name) const override {
    emscripten_fetch_t* fetch =
        static_cast<emscripten_fetch_t*>(fetches_by_name_[model_name]);
    return fetch->numBytes;
  }

  const char* GetBuffer(const std::string& model_name) const override {
    emscripten_fetch_t* fetch =
        static_cast<emscripten_fetch_t*>(fetches_by_name_[model_name]);
    return fetch->data;
  }

 private:
  std::string ModelUrl(const std::string& model_name) const {
    return base_url_ + model_name;
  }

  mutable std::unordered_map<std::string, emscripten_fetch_t*> fetches_by_name_;
  const std::string base_url_;
};

// Buffer for wavegru models.
WebassemblyModelBuffer model_buffer(
    {"lyragan.tflite", "quantizer.tflite", "soundstream_encoder.tflite"},
    "model_coeffs/");

void downloadSucceeded(emscripten_fetch_t* fetch) {
  const char* model_name = static_cast<char*>(fetch->userData);
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes,
         fetch->url);
  model_buffer.SetFetch(model_name, fetch);
  FETCHED_MODEL_COUNT++;
  if (FETCHED_MODEL_COUNT == 3) {
    CreateEncoder();
    if (encoder == nullptr) {
      printf("Failed to create encoder.\n");
    }
    CreateDecoder();
  }
}

void downloadFailed(emscripten_fetch_t* fetch) {
  auto fetch_ptr = static_cast<emscripten_fetch_t*>(fetch->userData);
  printf("Downloading %s failed, HTTP failure status code: %d.\n",
         fetch_ptr->url, fetch_ptr->status);
}

void asyncDownload(const std::string& url, const std::string& model_name) {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.userData = (void*)model_name.c_str();
  attr.attributes =
      EMSCRIPTEN_FETCH_PERSIST_FILE | EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = downloadSucceeded;
  attr.onerror = downloadFailed;
  emscripten_fetch(&attr, url.c_str());
}

void InitializeCodec() { model_buffer.DownloadModels(); }

void CreateEncoder() {
  encoder = chromemedia::codec::LyraEncoder::Create(
      /*sample_rate_hz=*/48000,
      /*num_channels=*/1,
      /*bitrate=*/3200,
      /*enable_dtx=*/false, model_buffer);

  // Create encoders at different sample rates because the input audio sample
  // rate is not know at this time.
  encoder_16khz = chromemedia::codec::LyraEncoder::Create(
      /*sample_rate_hz=*/16000,
      /*num_channels=*/1,
      /*bitrate=*/3200,
      /*enable_dtx=*/false, model_buffer);

  encoder_32khz = chromemedia::codec::LyraEncoder::Create(
      /*sample_rate_hz=*/32000,
      /*num_channels=*/1,
      /*bitrate=*/3200,
      /*enable_dtx=*/false, model_buffer);

  encoder_8khz = chromemedia::codec::LyraEncoder::Create(
      /*sample_rate_hz=*/8000,
      /*num_channels=*/1,
      /*bitrate=*/3200,
      /*enable_dtx=*/false, model_buffer);

  if (encoder == nullptr || encoder_32khz == nullptr ||
      encoder_16khz == nullptr || encoder_8khz == nullptr) {
    fprintf(stderr, "Failed to create encoders.\n");
  } else {
    fprintf(stdout, "Successfully created encoders!\n");
    encoders_initialized = true;
  }
}

void CreateDecoder() {
  decoder = chromemedia::codec::LyraDecoder::Create(
      /*sample_rate_hz=*/48000,
      /*num_channels=*/1,
       model_buffer);

  decoder_16khz = chromemedia::codec::LyraDecoder::Create(
      /*sample_rate_hz=*/16000,
      /*num_channels=*/1,
      model_buffer);

  decoder_32khz = chromemedia::codec::LyraDecoder::Create(
      /*sample_rate_hz=*/32000,
      /*num_channels=*/1,
      model_buffer);

  decoder_8khz = chromemedia::codec::LyraDecoder::Create(
      /*sample_rate_hz=*/8000,
      /*num_channels=*/1,
      model_buffer);

  if (decoder == nullptr || decoder_32khz == nullptr ||
      decoder_16khz == nullptr || decoder_8khz == nullptr) {
    fprintf(stderr, "Failed to create decoders.\n");
  } else {
    fprintf(stdout, "Successfully created decoders!\n");
    decoders_initialized = true;
  }
}

bool EncodeAndDecodeWithLyra(uintptr_t data, uint32_t num_samples,
                             uint32_t sample_rate_hz, uintptr_t out_data) {
  fprintf(stdout, "EncodeAndDecode called with %d samples.\n", num_samples);

  chromemedia::codec::LyraEncoder* encoder_to_use = nullptr;
  chromemedia::codec::LyraDecoder* decoder_to_use = nullptr;
  if (sample_rate_hz == 48000) {
    encoder_to_use = encoder.get();
    decoder_to_use = decoder.get();
  } else if (sample_rate_hz == 16000) {
    encoder_to_use = encoder_16khz.get();
    decoder_to_use = decoder_16khz.get();
  } else if (sample_rate_hz == 32000) {
    encoder_to_use = encoder_32khz.get();
    decoder_to_use = decoder_32khz.get();
  } else if (sample_rate_hz == 8000) {
    encoder_to_use = encoder_8khz.get();
    decoder_to_use = decoder_8khz.get();
  } else {
    fprintf(stderr,
            "Unsupported sample rate: %d. Only %d, %d, %d and %d khz sample "
            "rates are supported.\n",
            sample_rate_hz, 48000, 16000, 32000, 8000);
    return false;
  }

  // Convert the float input data to int16_t.
  float* data_ptr = reinterpret_cast<float*>(data);
  std::vector<int16_t> data_to_encode(num_samples);
  for (int i = 0; i < num_samples; i++) {
    data_to_encode[i] = static_cast<int16_t>(data_ptr[i] * 32767.0f);
  }

  auto maybe_decoded_output = chromemedia::codec::EncodeAndDecode(
      encoder_to_use, decoder_to_use, data_to_encode, sample_rate_hz,
      /*packet_loss_rate=*/0.f,
      /*float_average_burst_length=*/1.f);
  if (!maybe_decoded_output.has_value()) {
    fprintf(stderr, "Failed to encode and decode.\n");
    return false;
  }

  if (maybe_decoded_output.value().empty()) {
    fprintf(stderr,
            "No decoded output. The number of samples sent for encode and "
            "decode (%d) was probably too small.\n",
            num_samples);
    return false;
  }

  // Convert the decoded output to float.
  const int num_decoded_samples = maybe_decoded_output.value().size();
  float* out_data_ptr = reinterpret_cast<float*>(out_data);
  const std::vector<int16_t>& decoded_output = maybe_decoded_output.value();
  for (int i = 0; i < num_decoded_samples; i++) {
    out_data_ptr[i] = static_cast<float>(decoded_output[i] / 32767.0f);
  }

  fprintf(stdout, "Encode and decode succeeded. Returning %d samples.\n",
          num_decoded_samples);
  return true;
}

bool IsCodecReady() { return encoders_initialized && decoders_initialized; }

int main(int argc, char* argv[]) {
  InitializeCodec();
  return 0;
}

EMSCRIPTEN_BINDINGS(module) {
  emscripten::function("isCodecReady", IsCodecReady);
  emscripten::function("encodeAndDecode", EncodeAndDecodeWithLyra,
                       emscripten::allow_raw_pointers());
}