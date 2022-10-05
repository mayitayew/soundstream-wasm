#ifndef LYRA_CODEC_WEBASSEMBLY_CODEC_WRAPPER_H_
#define LYRA_CODEC_WEBASSEMBLY_CODEC_WRAPPER_H_

#include <vector>

#include "encode_and_decode_lib.h"
#include "lyra_decoder.h"
#include "lyra_encoder.h"

namespace chromemedia {
namespace codec {

// Clone of the method exposed to wasm in webassembly_codec_wrapper.cc. Cloned
// here for unit testing.
bool EncodeAndDecodeWithLyra(uintptr_t data, uint32_t num_samples,
                             uint32_t sample_rate_hz, uintptr_t out_data,
                             LyraEncoder* encoder, LyraDecoder* decoder) {
  fprintf(stdout, "EncodeAndDecode called with %d samples.\n", num_samples);

  // Convert the float input data to int16_t.
  float* data_ptr = reinterpret_cast<float*>(data);
  std::vector<int16_t> data_to_encode(num_samples);
  for (int i = 0; i < num_samples; i++) {
    data_to_encode[i] = static_cast<int16_t>(data_ptr[i] * 32767.0f);
  }

  auto maybe_decoded_output = chromemedia::codec::EncodeAndDecode(
      encoder, decoder, data_to_encode, sample_rate_hz,
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

}  // namespace codec
}  // namespace chromemedia

#endif  // LYRA_CODEC_WEBASSEMBLY_CODEC_WRAPPER_H_