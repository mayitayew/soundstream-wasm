#include "dsp_utils.h"

#include <cmath>
#include <optional>

#include "absl/types/span.h"
#include "audio/dsp/signal_vector_util.h"

namespace chromemedia {
namespace codec {

std::optional<float> LogSpectralDistance(
    const absl::Span<const float> first_log_spectrum,
    const absl::Span<const float> second_log_spectrum) {
  const int num_features = first_log_spectrum.size();
  if (num_features != second_log_spectrum.size()) {
    std::cerr << "Spectrum sizes are not equal.";
    return std::nullopt;
  }
  float log_spectral_distance = 0.f;
  for (int i = 0; i < num_features; ++i) {
    log_spectral_distance +=
        audio_dsp::Square(first_log_spectrum[i] - second_log_spectrum[i]);
  }
  return 10 * std::sqrt(log_spectral_distance / num_features);
}

}  // namespace codec
}  // namespace chromemedia
