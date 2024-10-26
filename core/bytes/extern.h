#pragma once
#include <v8.h>

namespace pand::core {

class ExternOneByte : public v8::String::ExternalOneByteStringResource {
public:
  ExternOneByte(char *data, size_t length) : data_(data), length_(length) {}

  ~ExternOneByte() override {
    delete[] data_; 
  }

  static ExternOneByte *from(char *data, size_t length) {
    return new ExternOneByte(data, length);
  }

  const char *data() const override { return data_; }
  size_t length() const override { return length_; }

private:
  char *data_;
  size_t length_;
};

} // namespace pand::core
