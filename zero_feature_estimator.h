#ifndef THIRD_PARTY_LYRA_CODEC_ZERO_FEATURE_ESTIMATOR_H_
#define THIRD_PARTY_LYRA_CODEC_ZERO_FEATURE_ESTIMATOR_H_

#include <vector>

#include "absl/types/span.h"
#include "feature_estimator_interface.h"

namespace chromemedia {
namespace codec {

class ZeroFeatureEstimator : public FeatureEstimatorInterface {
 public:
  explicit ZeroFeatureEstimator(int num_features)
      : estimated_features_(num_features, 0.f) {}
  void Update(absl::Span<const float> features) override {
    // Do nothing.
  }

  std::vector<float> Estimate() const override { return estimated_features_; }

 private:
  ZeroFeatureEstimator() = delete;

  std::vector<float> estimated_features_;
};

}  // namespace codec
}  // namespace chromemedia

#endif  // THIRD_PARTY_LYRA_CODEC_ZERO_FEATURE_ESTIMATOR_H_