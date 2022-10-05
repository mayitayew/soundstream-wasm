#ifndef LYRA_CODEC_RUNFILES_UTIL_H_
#define LYRA_CODEC_RUNFILES_UTIL_H_

#include "tools/cpp/runfiles/runfiles.h"

namespace tools {

using bazel::tools::cpp::runfiles::Runfiles;

inline std::string GetModelRunfilesPathForTest() {
  std::string error;
  std::unique_ptr<Runfiles> rfiles(Runfiles::CreateForTest(&error));
  if (rfiles == nullptr) {
    std::cerr << "FATAL: Runfiles error. " << error << std::endl;
    exit(EXIT_FAILURE);
  }
  return rfiles->Rlocation("lyra/wavegru/");
}

inline std::string GetModelRunfilesPath(const std::string& arg0) {
  std::string error;
  std::unique_ptr<Runfiles> rfiles(Runfiles::Create(arg0, &error));
  if (rfiles == nullptr) {
    std::cerr << "FATAL: Runfiles error. " << error << std::endl;
    exit(EXIT_FAILURE);
  }
  return rfiles->Rlocation("lyra/wavegru/");
}

inline std::string GetTestdataRunfilesPath() {
  std::string error;
  std::unique_ptr<Runfiles> rfiles(Runfiles::CreateForTest(&error));
  if (rfiles == nullptr) {
    std::cerr << "FATAL: Runfiles error. " << error << std::endl;
    exit(EXIT_FAILURE);
  }
  return rfiles->Rlocation("lyra/testdata/");
}

// This version is for non-test binaries.
inline std::string GetTestdataRunfilesPath(const std::string& arg0) {
  std::string error;
  std::unique_ptr<Runfiles> rfiles(Runfiles::Create(arg0, &error));
  if (rfiles == nullptr) {
    std::cerr << "FATAL: Runfiles error. " << error << std::endl;
    exit(EXIT_FAILURE);
  }
  return rfiles->Rlocation("lyra/testdata/");
}

}  // namespace tools

#endif  // LYRA_CODEC_RUNFILES_UTIL_H_