/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace mdf {

template <typename T>
class BigBuffer {
 public:
  BigBuffer() = default;
  explicit BigBuffer(const T& value);
  BigBuffer(const std::vector<uint8_t>& buffer, size_t offset);

  [[nodiscard]] const uint8_t* data() const;
  [[nodiscard]] uint8_t* data();
  [[nodiscard]] size_t size() const;
  T value() const;

 private:
  std::array<uint8_t, sizeof(T)> buffer_;
};

template <typename T>
BigBuffer<T>::BigBuffer(const T& value) {
  if (constexpr int num = 1;*(char*) &num == 1) { // If computer using Little endian
    std::array<uint8_t, sizeof(T)> temp = {0};
    memcpy(temp.data(), &value, sizeof(T));
    for (size_t index = sizeof(T); index > 0; --index) {
      buffer_[sizeof(T) - index] = temp[index - 1];
    }
  } else {
    memcpy(buffer_.data(), &value, sizeof(T));
  }
}

template <typename T>
BigBuffer<T>::BigBuffer(const std::vector<uint8_t>& buffer, size_t offset) {
  memcpy(buffer_.data(), buffer.data() + offset, sizeof(T));
}

template <typename T>
const uint8_t* BigBuffer<T>::data() const {
  return buffer_.data();
}

template <typename T>
uint8_t* BigBuffer<T>::data() {
  return buffer_.data();
}

template <typename T>
size_t BigBuffer<T>::size() const {
  return buffer_.size();
}

template <typename T>
T BigBuffer<T>::value() const {
  std::array<uint8_t, sizeof(T)> temp = {0};
  if (constexpr int num = 1; *(char*) &num == 1) { // Computer uses little endian
    for (size_t index = sizeof(T); index > 0; --index) {
      temp[sizeof(T) - index] = buffer_[index - 1];
    }
  } else {
    memcpy(temp.data(), buffer_.data(), sizeof(T));
  }
  const T* val = reinterpret_cast<const T*>(temp.data());
  return *val;
}

}  // namespace mdf
