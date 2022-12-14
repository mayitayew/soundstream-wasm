/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "audio/dsp/mfcc/dct.h"

#include <math.h>

#include "audio/dsp/porting.h"  // auto-added.


namespace audio_dsp {

Dct::Dct() : initialized_(false) {
}

bool Dct::Initialize(int input_length, int coefficient_count) {
  coefficient_count_ = coefficient_count;
  input_length_ = input_length;

  if (coefficient_count_ < 1) {
    std::cerr << "Coefficient count must be positive." << std::endl;
    return false;
  }

  if (input_length < 1) {
    std::cerr << "Input length must be positive." << std::endl;
    return false;
  }

  if (coefficient_count_ > input_length_) {
    std::cerr << "Coefficient count must be less than or equal to "
               << "input length." << std::endl;
    return false;
  }

  cosines_.resize(coefficient_count_);
  double fnorm = sqrt(2.0 / input_length_);
  double arg = M_PI / input_length_;
  for (int i = 0; i < coefficient_count_; ++i) {
    cosines_[i].resize(input_length_);
    for (int j = 0; j < input_length_; ++j) {
      cosines_[i][j] = fnorm * cos(i * arg * (j + 0.5));
    }
  }
  initialized_ = true;
  return true;
}

void Dct::Compute(const std::vector<double> &input,
                  std::vector<double> *output) const {
  if (!initialized_) {
    std::cerr << "DCT not initialized." << std::endl;
    return;
  }

  output->resize(coefficient_count_);
  int length = input.size();
  if (length > input_length_) {
    length = input_length_;
  }

  for (int i = 0; i < coefficient_count_; ++i) {
    double sum = 0.0;
    for (int j = 0; j < length; ++j) {
      sum += cosines_[i][j] * input[j];
    }
    (*output)[i] = sum;
  }
}

}  // namespace audio_dsp
