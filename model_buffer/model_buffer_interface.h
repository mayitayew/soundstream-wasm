#ifndef LYRA_CODEC_MODEL_BUFFER_INTERFACE_H_
#define LYRA_CODEC_MODEL_BUFFER_INTERFACE_H_

#include <unordered_map>

namespace chromemedia {
namespace codec {

// Encapsulates tflite models that have been read from a source and loaded into
// memory. This abstraction is useful for constructing various components of
// lyra by fetching model files from some source (e.g a URL), loading them into
// memory and passing buffer pointers to the components. Ordinarily, the lyra
// components are built from model files on disk.
class ModelBufferInterface {
 public:
  virtual uint64_t GetBufferSize(const std::string& model_name) const = 0;

  virtual const char* GetBuffer(const std::string& model_name) const = 0;
};


class SimpleModelBuffer : public ModelBufferInterface {
 public:
  SimpleModelBuffer(
      std::unordered_map<std::string, std::pair<uint64_t, const char*>> models)
      : models_(models) {}

  uint64_t GetBufferSize(const std::string& model_name) const override {
    return models_.at(model_name).first;
  }

  const char* GetBuffer(const std::string& model_name) const override {
    return models_.at(model_name).second;
  }

 private:
  std::unordered_map<std::string, std::pair<uint64_t, const char*>> models_;
};

}  // namespace codec
}  // namespace chromemedia

#endif  // LYRA_CODEC_MODEL_BUFFER_INTERFACE_H_