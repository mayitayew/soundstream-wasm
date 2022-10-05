#include "encode_and_decode_lib.h"

#include <vector>

#include "gmock/gmock.h"
#include "lyra_config.h"
#include "lyra_decoder.h"
#include "lyra_encoder.h"
#include "runfiles_util.h"

namespace chromemedia {
namespace codec {
namespace {

using tools::GetModelRunfilesPathForTest;

class EncodeAndDecodeLibTest : public testing::Test {
 protected:
  EncodeAndDecodeLibTest()
      : sample_rate_hz_(48000),
        num_channels_(1),
        bitrate_(3000),
        encoder_(LyraEncoder::Create(sample_rate_hz_, num_channels_, bitrate_,
                                     false, GetModelRunfilesPathForTest())),
        decoder_(LyraDecoder::Create(sample_rate_hz_, num_channels_, bitrate_,
                                     GetModelRunfilesPathForTest())) {}

  const int sample_rate_hz_ = 0;
  const int num_channels_ = 0;
  const int bitrate_ = 3000;

  std::unique_ptr<LyraEncoder> encoder_;
  std::unique_ptr<LyraDecoder> decoder_;
};

TEST_F(EncodeAndDecodeLibTest, EncodeSamples) {
  std::vector<int16_t> samples(1920, 4);
  auto encode_result =
      EncodeWithEncoder(encoder_.get(), samples, sample_rate_hz_);
  EXPECT_TRUE(encode_result.has_value());
  std::cerr << "Encode result size: " << encode_result.value().size()
            << std::endl;
  EXPECT_GE(encode_result.value().size(), 0u);
}

TEST_F(EncodeAndDecodeLibTest, DecodeSamples) {
    std::vector<uint8_t> samples(
        {112, 38, 238, 11, 69, 173, 121, 126, 148, 178, 78, 181, 65, 206,
        105});
  auto decode_result = DecodeWithDecoder(decoder_.get(), samples, 0.f, 1.f);
  EXPECT_TRUE(decode_result.has_value());
  std::cerr << "Decode result size: " << decode_result.value().size()
            << std::endl;
  for (const auto& sample : decode_result.value()) {
    std::cerr << sample << " ";
  }

  EXPECT_GE(decode_result.value().size(), 0u);
}

}  // namespace
}  // namespace codec
}  // namespace chromemedia