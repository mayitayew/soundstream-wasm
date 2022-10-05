#ifndef LYRA_CODEC_BUFFERED_FILTER_INTERFACE_H_
#define LYRA_CODEC_BUFFERED_FILTER_INTERFACE_H_

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace chromemedia {
namespace codec {

// Interface for filters which can not output an arbitrary number of samples.
class BufferedFilterInterface {
 public:
  virtual ~BufferedFilterInterface() {}

  virtual std::optional<std::vector<int16_t>> FilterAndBuffer(
      const std::function<std::optional<std::vector<int16_t>>(int)>&
          sample_generator,
      int num_samples) = 0;
};

}  // namespace codec
}  // namespace chromemedia

#endif  // LYRA_CODEC_BUFFERED_FILTER_INTERFACE_H_