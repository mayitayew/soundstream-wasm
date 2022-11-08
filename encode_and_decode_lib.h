#ifndef LYRA_CODEC_ENCODE_AND_DECODE_LIB_H_
#define LYRA_CODEC_ENCODE_AND_DECODE_LIB_H_

#include "lyra_decoder.h"
#include "lyra_encoder.h"

namespace chromemedia {
namespace codec {

std::optional<std::vector<uint8_t>> EncodeWithEncoder(
    LyraEncoder* encoder, const std::vector<int16_t>& wav_data,
    int sample_rate_hz);

std::optional<std::vector<int16_t>> DecodeWithDecoder(
    LyraDecoder* decoder, const std::vector<uint8_t>& encoded_data,
    float packet_loss_rate, float average_burst_length, uint32_t bitrate);

std::optional<std::vector<int16_t>> EncodeAndDecode(
    LyraEncoder* encoder, LyraDecoder* decoder,
    const std::vector<int16_t>& wav_data, int sample_rate_hz,
    float packet_loss_rate, float average_burst_length);

bool End2End(const std::string& input_filename,
             const std::string& output_filename, const std::string& arg0);

}  // namespace codec
}  // namespace chromemedia

#endif  // LYRA_CODEC_ENCODE_AND_DECODE_LIB_H_