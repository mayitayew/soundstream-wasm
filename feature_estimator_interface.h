#ifndef LYRA_CODEC_FEATURE_ESTIMATOR_INTERFACE_H_
#define LYRA_CODEC_FEATURE_ESTIMATOR_INTERFACE_H_

#include <vector>

#include "absl/types/span.h"

namespace chromemedia {
namespace codec {

class FeatureEstimatorInterface {
 public:
  virtual ~FeatureEstimatorInterface() {}

  virtual void Update(absl::Span<const float> features) = 0;

  virtual std::vector<float> Estimate() const = 0;
};

}  // namespace codec
}  // namespace chromemedia

#endif  // LYRA_CODEC_FEATURE_ESTIMATOR_INTERFACE_H_