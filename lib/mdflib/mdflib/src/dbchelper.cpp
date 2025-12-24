/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "dbchelper.h"
#include <cstring>
#include "half.hpp"

namespace {

constexpr uint8_t kMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
using signed64 = union {
  int64_t val1 : 1;
  int64_t val2 : 2;
  int64_t val3 : 3;
  int64_t val4 : 4;
  int64_t val5 : 5;
  int64_t val6 : 6;
  int64_t val7 : 7;
  int64_t val8 : 8;
  int64_t val9 : 9;
  int64_t val10 : 10;
  int64_t val11 : 11;
  int64_t val12 : 12;
  int64_t val13 : 13;
  int64_t val14 : 14;
  int64_t val15 : 15;
  int64_t val16 : 16;
  int64_t val17 : 17;
  int64_t val18 : 18;
  int64_t val19 : 19;
  int64_t val20 : 20;
  int64_t val21 : 21;
  int64_t val22 : 22;
  int64_t val23 : 23;
  int64_t val24 : 24;
  int64_t val25 : 25;
  int64_t val26 : 26;
  int64_t val27 : 27;
  int64_t val28 : 28;
  int64_t val29 : 29;
  int64_t val30 : 30;
  int64_t val31 : 31;
  int64_t val32 : 32;
  int64_t val33 : 33;
  int64_t val34 : 34;
  int64_t val35 : 35;
  int64_t val36 : 36;
  int64_t val37 : 37;
  int64_t val38 : 38;
  int64_t val39 : 39;
  int64_t val40 : 40;
  int64_t val41 : 41;
  int64_t val42 : 42;
  int64_t val43 : 43;
  int64_t val44 : 44;
  int64_t val45 : 45;
  int64_t val46 : 46;
  int64_t val47 : 47;
  int64_t val48 : 48;
  int64_t val49 : 49;
  int64_t val50 : 50;
  int64_t val51 : 51;
  int64_t val52 : 52;
  int64_t val53 : 53;
  int64_t val54 : 54;
  int64_t val55 : 55;
  int64_t val56 : 56;
  int64_t val57 : 57;
  int64_t val58 : 58;
  int64_t val59 : 59;
  int64_t val60 : 60;
  int64_t val61 : 61;
  int64_t val62 : 62;
  int64_t val63 : 63;
  int64_t val64 : 64;
};

} // end namespace empty

namespace mdf::detail {

void DbcHelper::DoubleToRaw(bool little_endian, size_t start, size_t length,
                            double value, uint8_t* dest) {
  if (dest == nullptr || length < 64) {
    return;
  }
  length = 64;

  uint64_t mask = 1ULL << 63;
  uint64_t temp = 0;
  memcpy(&temp, &value, sizeof(temp));

  auto bit = little_endian ? static_cast<int>(start + length - 1)
                           : static_cast<int>(start);

  auto byte = bit / 8;
  bit %= 8;

  for (size_t index = 0; index < length; ++index) {
    if ((temp & mask) != 0) {
      dest[byte] |= kMask[bit];
    } else {
      dest[byte] &= ~kMask[bit];
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
    if (byte >= static_cast<int>( (start + length) /8 ) ) {
      // Buffer overrun. Most likely invalid start bit
      break;
    }
  }
}

void DbcHelper::FloatToRaw(bool little_endian, size_t start, size_t length,
                           float value, uint8_t* raw) {
  if (raw == nullptr || length < 32) {
    return;
  }
  length = 32;
  uint32_t mask = 1UL << 31;
  uint32_t temp = 0;
  memcpy(&temp, &value, sizeof(temp));

  auto bit = little_endian ? static_cast<int>(start + length - 1)
                           : static_cast<int>(start);

  auto byte = bit / 8;
  bit %= 8;
  for (size_t index = 0; index < length; ++index) {
    if ((temp & mask) != 0) {
      raw[byte] |= kMask[bit];
    } else {
      raw[byte] &= ~kMask[bit];
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
    if (byte >= static_cast<int>((start + length) / 8)) {
      // Buffer overrun. Most likely invalid start bit
      break;
    }
  }
}

void DbcHelper::SignedToRaw(bool little_endian, size_t start, size_t length,
                            int64_t value, uint8_t* raw) {
  if (raw == nullptr || length == 0) {
    return;
  }

  signed64 temp_val{};

  switch (length) {
    case 1:
      temp_val.val1 = value;
      break;
    case 2:
      temp_val.val2 = value;
      break;
    case 3:
      temp_val.val3 = value;
      break;
    case 4:
      temp_val.val4 = value;
      break;
    case 5:
      temp_val.val5 = value;
      break;
    case 6:
      temp_val.val6 = value;
      break;
    case 7:
      temp_val.val7 = value;
      break;
    case 8:
      temp_val.val8 = value;
      break;
    case 9:
      temp_val.val9 = value;
      break;
    case 10:
      temp_val.val10 = value;
      break;
    case 11:
      temp_val.val11 = value;
      break;
    case 12:
      temp_val.val12 = value;
      break;
    case 13:
      temp_val.val13 = value;
      break;
    case 14:
      temp_val.val14 = value;
      break;
    case 15:
      temp_val.val15 = value;
      break;
    case 16:
      temp_val.val16 = value;
      break;
    case 17:
      temp_val.val17 = value;
      break;
    case 18:
      temp_val.val18 = value;
      break;
    case 19:
      temp_val.val19 = value;
      break;
    case 20:
      temp_val.val20 = value;
      break;
    case 21:
      temp_val.val21 = value;
      break;
    case 22:
      temp_val.val22 = value;
      break;
    case 23:
      temp_val.val23 = value;
      break;
    case 24:
      temp_val.val24 = value;
      break;
    case 25:
      temp_val.val25 = value;
      break;
    case 26:
      temp_val.val26 = value;
      break;
    case 27:
      temp_val.val27 = value;
      break;
    case 28:
      temp_val.val28 = value;
      break;
    case 29:
      temp_val.val29 = value;
      break;
    case 30:
      temp_val.val30 = value;
      break;
    case 31:
      temp_val.val31 = value;
      break;
    case 32:
      temp_val.val32 = value;
      break;
    case 33:
      temp_val.val33 = value;
      break;
    case 34:
      temp_val.val34 = value;
      break;
    case 35:
      temp_val.val35 = value;
      break;
    case 36:
      temp_val.val36 = value;
      break;
    case 37:
      temp_val.val37 = value;
      break;
    case 38:
      temp_val.val38 = value;
      break;
    case 39:
      temp_val.val39 = value;
      break;
    case 40:
      temp_val.val40 = value;
      break;
    case 41:
      temp_val.val41 = value;
      break;
    case 42:
      temp_val.val42 = value;
      break;
    case 43:
      temp_val.val43 = value;
      break;
    case 44:
      temp_val.val44 = value;
      break;
    case 45:
      temp_val.val45 = value;
      break;
    case 46:
      temp_val.val46 = value;
      break;
    case 47:
      temp_val.val47 = value;
      break;
    case 48:
      temp_val.val48 = value;
      break;
    case 49:
      temp_val.val49 = value;
      break;
    case 50:
      temp_val.val50 = value;
      break;
    case 51:
      temp_val.val51 = value;
      break;
    case 52:
      temp_val.val52 = value;
      break;
    case 53:
      temp_val.val53 = value;
      break;
    case 54:
      temp_val.val54 = value;
      break;
    case 55:
      temp_val.val55 = value;
      break;
    case 56:
      temp_val.val56 = value;
      break;
    case 57:
      temp_val.val57 = value;
      break;
    case 58:
      temp_val.val58 = value;
      break;
    case 59:
      temp_val.val59 = value;
      break;
    case 60:
      temp_val.val60 = value;
      break;
    case 61:
      temp_val.val61 = value;
      break;
    case 62:
      temp_val.val62 = value;
      break;
    case 63:
      temp_val.val63 = value;
      break;
    case 64:
      temp_val.val64 = value;
      break;
    default:
      return;
  }

  uint64_t mask = 1ULL << (length - 1);
  uint64_t temp = 0;
  memcpy(&temp, &temp_val, sizeof(temp));
  auto bit = little_endian ? static_cast<int>(start + length - 1)
                           : static_cast<int>(start);
  auto byte = bit / 8;
  bit %= 8;
  for (size_t index = 0; index < length; ++index) {
    if ((temp & mask) != 0) {
      raw[byte] |= kMask[bit];
    } else {
      raw[byte] &= ~kMask[bit];
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
}

void DbcHelper::UnsignedToRaw(bool little_endian, size_t start, size_t length,
                              uint64_t value, uint8_t* raw) {
  if (raw == nullptr || length == 0) {
    return;
  }

  uint64_t mask = 1ULL << (length - 1);
  auto bit = little_endian ? static_cast<int>(start + length - 1)
                           : static_cast<int>(start);
  auto byte = bit / 8;
  bit %= 8;

  for (size_t index = 0; index < length; ++index) {
    if ((value & mask) != 0) {
      raw[byte] |= kMask[bit];
    } else {
      raw[byte] &= ~kMask[bit];
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
}

std::vector<uint8_t> DbcHelper::RawToByteArray(size_t start, size_t length,
                                               const unsigned char* raw )
{
  // Only byte aligned strings are supported
  if ( raw == nullptr || (length % 8) != 0 || (start % 8) != 0) {
    return {};
  }
  const auto byte_start = start / 8;
  const auto byte_size = length / 8;
  std::vector<uint8_t> temp;
  temp.resize(byte_size, 0);
  memcpy(temp.data(), raw + byte_start, byte_size);
  return temp;
}

double DbcHelper::RawToDouble(bool little_endian, size_t start, size_t length,
                              const unsigned char* raw )
{
  double value = 0.0;
  uint64_t temp = 0;
  uint64_t mask = 1ULL << ( length - 1 );
  auto byte = (little_endian ?
               static_cast<int>(start + length - 1) : static_cast<int>(start) ) / 8;
  auto bit = static_cast<int>( (start + length - 1) % 8);
  for (size_t index = 0; index < length; ++index ) {
    if ((raw[byte] & kMask[bit] ) != 0) {
      temp |= mask;
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
  memcpy( &value, &temp, sizeof(value));
  return value;
}

float DbcHelper::RawToFloat(bool little_endian, size_t start, size_t length,
                            const unsigned char* raw )
{
  float value = 0.0;
  uint32_t temp = 0;
  uint32_t mask = 1ULL << ( length - 1 );
  auto byte = (little_endian ?
               static_cast<int>(start + length - 1) : static_cast<int>(start) ) / 8;
  auto bit = static_cast<int>( (start + length - 1) % 8);
  for (size_t index = 0; index < length; ++index ) {
    if ((raw[byte] & kMask[bit] ) != 0) {
      temp |= mask;
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
  memcpy( &value, &temp, sizeof(value));
  return value;
}

float DbcHelper::RawToHalf(bool little_endian, size_t start, size_t length,
                            const unsigned char* raw )
{
  if (length > 16) {
    length = 16;
  }

  uint16_t temp = 0;
  uint16_t mask = 1ULL << ( length - 1 );
  auto byte = (little_endian ?
               static_cast<int>(start + length - 1) : static_cast<int>(start) ) / 8;
  auto bit = static_cast<int>( (start + length - 1) % 8);
  for (size_t index = 0; index < length; ++index ) {
    if ((raw[byte] & kMask[bit] ) != 0) {
      temp |= mask;
    }
    mask >>= 1;
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }

  half_float::half value;
  value.data_ = temp;
  return value;
}

int64_t DbcHelper::RawToSigned(bool little_endian, size_t start, size_t length,
                               const uint8_t* raw )
{
  int64_t value = 0;

  uint64_t temp = 0;
  auto byte = (little_endian ?
               static_cast<int>(start + length - 1) : static_cast<int>(start) ) / 8;
  auto bit = static_cast<int>( (start + length - 1) % 8);

  for (size_t index = 0; index < length; ++index) {
    if ( index > 0 ) temp <<= 1;
    if ((raw[byte] & kMask[bit]) != 0) {
      temp |= 1;
    }
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }

  signed64 temp_value {};
  memcpy( &temp_value, &temp, sizeof(temp));

  switch ( length ) {
    case 1: value = temp_value.val1; break;
    case 2: value = temp_value.val2; break;
    case 3: value = temp_value.val3; break;
    case 4: value = temp_value.val4; break;
    case 5: value = temp_value.val5; break;
    case 6: value = temp_value.val6; break;
    case 7: value = temp_value.val7; break;
    case 8: value = temp_value.val8; break;
    case 9: value = temp_value.val9; break;
    case 10: value = temp_value.val10; break;
    case 11: value = temp_value.val11; break;
    case 12: value = temp_value.val12; break;
    case 13: value = temp_value.val13; break;
    case 14: value = temp_value.val14; break;
    case 15: value = temp_value.val15; break;
    case 16: value = temp_value.val16; break;
    case 17: value = temp_value.val17; break;
    case 18: value = temp_value.val18; break;
    case 19: value = temp_value.val19; break;
    case 20: value = temp_value.val20; break;
    case 21: value = temp_value.val21; break;
    case 22: value = temp_value.val22; break;
    case 23: value = temp_value.val23; break;
    case 24: value = temp_value.val24; break;
    case 25: value = temp_value.val25; break;
    case 26: value = temp_value.val26; break;
    case 27: value = temp_value.val27; break;
    case 28: value = temp_value.val28; break;
    case 29: value = temp_value.val29; break;
    case 30: value = temp_value.val30; break;
    case 31: value = temp_value.val31; break;
    case 32: value = temp_value.val32; break;
    case 33: value = temp_value.val33; break;
    case 34: value = temp_value.val34; break;
    case 35: value = temp_value.val35; break;
    case 36: value = temp_value.val36; break;
    case 37: value = temp_value.val37; break;
    case 38: value = temp_value.val38; break;
    case 39: value = temp_value.val39; break;
    case 40: value = temp_value.val40; break;
    case 41: value = temp_value.val41; break;
    case 42: value = temp_value.val42; break;
    case 43: value = temp_value.val43; break;
    case 44: value = temp_value.val44; break;
    case 45: value = temp_value.val45; break;
    case 46: value = temp_value.val46; break;
    case 47: value = temp_value.val47; break;
    case 48: value = temp_value.val48; break;
    case 49: value = temp_value.val49; break;
    case 50: value = temp_value.val50; break;
    case 51: value = temp_value.val51; break;
    case 52: value = temp_value.val52; break;
    case 53: value = temp_value.val53; break;
    case 54: value = temp_value.val54; break;
    case 55: value = temp_value.val55; break;
    case 56: value = temp_value.val56; break;
    case 57: value = temp_value.val57; break;
    case 58: value = temp_value.val58; break;
    case 59: value = temp_value.val59; break;
    case 60: value = temp_value.val60; break;
    case 61: value = temp_value.val61; break;
    case 62: value = temp_value.val62; break;
    case 63: value = temp_value.val63; break;
    case 64: value = temp_value.val64; break;
    default:
      break;
  }
  return value;
}

uint64_t DbcHelper::RawToUnsigned(bool little_endian, size_t start,
                                  size_t length, const uint8_t* raw)
{
  uint64_t value = 0;
  auto byte = (little_endian ?
      static_cast<int>(start + length - 1) : static_cast<int>(start) ) / 8;
  auto bit = static_cast<int>( (start + length - 1) % 8);

  for (size_t index = 0; index < length; ++index) {
    if (index > 0) {
      value <<= 1;
    }
    if ((raw[byte] & kMask[bit]) != 0) {
      value |= 1;
    }
    --bit;
    if (bit < 0) {
      bit = 7;
      little_endian ? --byte : ++byte;
    }
  }
  return value;
}

void DbcHelper::SetAllBits(size_t start, size_t length, uint8_t* raw ) {
  if (raw == nullptr) {
    return;
  }

  for ( size_t index = 0; index < length; ++index) {
    auto bit = start + index;
    const auto byte = bit / 8;
    bit %= 8;
    raw[byte] |= kMask[bit];
  }
}

bool DbcHelper::IsAllBitsSet(size_t start, size_t length, const uint8_t* raw) {
  if (raw == nullptr) {
    return true;
  }
  if (length <= 1) {
    return false; // 1 bit cannot have an invalid flag
  }
  for (size_t index = 0; index < length; ++index) {
    auto bit = start + index;
    const auto byte = bit / 8;
    bit %= 8;
    if ((raw[byte] & kMask[bit]) == 0 ) {
      return false;
    }
  }
  return true;
}
/*
std::string DbcHelper::GetStem(const std::string& path) {
  // First parse out the file name
  const auto last_back_pos = path.find_last_of('\\');
  const auto back_pos = last_back_pos != std::string::npos;

  const auto last_forward_pos = path.find_last_of('/');
  const auto forward_pos = last_forward_pos != std::string::npos;

  std::string filename;
  if (back_pos && forward_pos) {
    if ( last_back_pos > last_forward_pos) {
      // Use backward slash position
      filename = path.substr(last_back_pos + 1);
    } else {
      // Use forward slash position
      filename = path.substr(last_forward_pos + 1);
    }
  } else if (back_pos) {
    filename = path.substr(last_back_pos + 1);
  } else if (forward_pos) {
    filename = path.substr(last_forward_pos + 1);
  } else {
    filename = path;
  }
  // Strip out the extension
  std::string stem;
  const auto last_dot_pos = filename.find_last_of('.');
  if (last_dot_pos != std::string::npos) {
    stem = filename.substr(0,last_dot_pos);
  } else {
    stem = filename;
  }

  return stem;
}

bool DbcHelper::FileExist(const std::string& path) {
  std::ifstream temp(path);
  return temp.good();
}
*/
bool DbcHelper::IsLittleEndian() {
  constexpr int temp = 1;
  return *((const int8_t*) &temp) == 1;
}

}
