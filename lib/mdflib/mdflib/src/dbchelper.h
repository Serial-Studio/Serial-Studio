/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace mdf::detail {
class DbcHelper {
 public:
  static void DoubleToRaw(bool little_endian, size_t start, size_t length,
                          double value, uint8_t* dest);

  static void FloatToRaw(bool little_endian, size_t start, size_t length,
                         float value, uint8_t* dest);

  static void SignedToRaw(bool little_endian, size_t start, size_t length,
                          int64_t value, uint8_t* raw);

  static void UnsignedToRaw(bool little_endian, size_t start, size_t length,
                            uint64_t value, uint8_t* raw);

  static std::vector<uint8_t> RawToByteArray(size_t start, size_t length,
                                             const unsigned char* raw);

  static double RawToDouble(bool little_endian, size_t start, size_t length,
                            const unsigned char* raw);

  static float RawToFloat(bool little_endian, size_t start, size_t length,
                          const unsigned char* raw);

  static float RawToHalf(bool little_endian, size_t start, size_t length,
                          const unsigned char* raw);

  static int64_t RawToSigned(bool little_endian, size_t start, size_t length,
                             const uint8_t* raw);

  static uint64_t RawToUnsigned(bool little_endian, size_t start, size_t length,
                                const uint8_t* raw);


  static void SetAllBits(size_t start, size_t length, uint8_t* raw);

  static bool IsAllBitsSet(size_t start, size_t length, const uint8_t* raw);
/*
  static std::string GetStem(const std::string& path);

  static bool FileExist(const std::string& path);
  */

  static bool IsLittleEndian();
};

} // end namespace
