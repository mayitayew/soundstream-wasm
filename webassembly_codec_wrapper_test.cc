#include "webassembly_codec_wrapper.h"

#include "gmock/gmock.h"
#include "lyra_config.h"
#include "lyra_decoder.h"
#include "lyra_encoder.h"
#include "runfiles_util.h"
#include "wav_utils.h"

namespace chromemedia {
namespace codec {
namespace {

using tools::GetModelRunfilesPathForTest;
using tools::GetTestdataRunfilesPath;

class WebassemblyCodecWrapperTest : public testing::Test {
 protected:
  WebassemblyCodecWrapperTest()
      : sample_rate_hz_(48000),
        num_channels_(1),
        bitrate_(3000),
        encoder(LyraEncoder::Create(sample_rate_hz_, num_channels_, bitrate_,
                                    false, GetModelRunfilesPathForTest())),
        decoder(LyraDecoder::Create(sample_rate_hz_, num_channels_, bitrate_,
                                    GetModelRunfilesPathForTest())) {}

  const int sample_rate_hz_ = 0;
  const int num_channels_ = 0;
  const int bitrate_ = 3000;

  std::unique_ptr<LyraEncoder> encoder;
  std::unique_ptr<LyraDecoder> decoder;
};

TEST_F(WebassemblyCodecWrapperTest, Basic) {
  absl::StatusOr<ReadWavResult> read_wav_result = Read16BitWavFileToVector(
      GetTestdataRunfilesPath() + "48khz_playback.wav");

  if (!read_wav_result.ok()) {
    fprintf(stderr, "Reading wavfile failed.\n");
    std::cerr << read_wav_result.status() << std::endl;
    exit(EXIT_FAILURE);
  }

  std::vector<float> data_to_encode(read_wav_result->samples.size());
  std::transform(read_wav_result->samples.begin(),
                 read_wav_result->samples.end(), data_to_encode.begin(),
                 [](int16_t x) { return x / 32767.0f; });

  std::vector<float> decoded_data(read_wav_result->samples.size(), 0.0);

  uintptr_t data_to_encode_ptr =
      reinterpret_cast<uintptr_t>(data_to_encode.data());
  uintptr_t decoded_data_ptr = reinterpret_cast<uintptr_t>(decoded_data.data());
  EXPECT_TRUE(EncodeAndDecodeWithLyra(
      data_to_encode_ptr, read_wav_result->samples.size(), 48000,
      decoded_data_ptr, encoder.get(), decoder.get()));

  std::vector<int16_t> data_to_write(read_wav_result->samples.size());
  std::transform(decoded_data.begin(), decoded_data.end(),
                 data_to_write.begin(), [](float x) { return x * 32767.0f; });

  absl::Status write_status = Write16BitWavFileFromVector(
      GetTestdataRunfilesPath() + "wasm.wav", /*kNumChannels=*/1,
      /*sample_rate_hz=*/48000, data_to_write);
  if (write_status.ok()) {
    fprintf(stderr, "Wrote wav file to %s.\n",
            (GetTestdataRunfilesPath() + "wasm.wav").c_str());
  } else {
    fprintf(stderr, "Failed to write wav file.\n");
    std::cerr << write_status << std::endl;
    exit(EXIT_FAILURE);
  }
}

}  // namespace
}  // namespace codec
}  // namespace chromemedia