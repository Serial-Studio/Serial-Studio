
#include "mdf/ichannel.h"

#include <ctime>
#include <string>

#include "bigbuffer.h"
// #include "half.hpp"
#include "littlebuffer.h"
#include "dbchelper.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdflogstream.h"
#include "mdf/ichannelarray.h"

namespace mdf {

bool IChannel::GetUnsignedValue(const std::vector<uint8_t> &record_buffer,
                                uint64_t &dest,
                                uint64_t array_index) const {

  const size_t bit_offset = (ByteOffset() * 8 + BitOffset())
    + static_cast<size_t>( (array_index * BitCount()) );
  size_t range_check = (bit_offset + BitCount()) / 8;
  if ((bit_offset + BitCount()) % 8 != 0) {
    ++range_check;
  }
  if (range_check > record_buffer.size()) {
    MDF_ERROR() << "Range check error. Byte Index: "
      << range_check << "(" << record_buffer.size() << "). "
      << "Channel: " << Name();
    return false;
  }

  if (Type() == ChannelType::VariableLength) {
     dest = detail::DbcHelper::RawToUnsigned(true,
                                            bit_offset,
                                            BitCount(),
                                            record_buffer.data());
    return true;
  }

  switch (DataType()) {
    // Note that the string actual returns the first 4 bytes
    //
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::UnsignedIntegerLe:
      dest = detail::DbcHelper::RawToUnsigned(true,
                                              bit_offset,
                                              BitCount(),
                                              record_buffer.data());
      break;

    case ChannelDataType::UnsignedIntegerBe:
      dest = detail::DbcHelper::RawToUnsigned(false,
                                              bit_offset,
                                              BitCount(),
                                              record_buffer.data());
      break;

    default: {
      MDF_ERROR() << "Not a valid conversion. Channel: " << Name();
      return false;  // Not valid conversion
    }
  }
  return true;
}

bool IChannel::GetSignedValue(const std::vector<uint8_t> &record_buffer,
                              int64_t &dest,
                              uint64_t array_index) const {
  // Copy to a temp data buffer, so we can get rid of the bit offset nonsense
  // std::vector<uint8_t> data_buffer;
  // CopyToDataBuffer(record_buffer, data_buffer, array_index);
  const size_t bit_offset = (ByteOffset() * 8 + BitOffset())
      + static_cast<size_t>( (array_index * BitCount()) );

  size_t range_check = (bit_offset + BitCount()) / 8;
  if ((bit_offset + BitCount()) % 8 != 0) {
    ++range_check;
  }
  if (range_check > record_buffer.size()) {
    MDF_ERROR() << "Range check error. Byte Index: "
      << range_check << "(" << record_buffer.size() << "). "
      << "Channel: " << Name();
    return false;
  }

  const bool little_endian = DataType() == ChannelDataType::SignedIntegerLe;
  dest = detail::DbcHelper::RawToSigned(little_endian,
                                                   bit_offset,
                                                  BitCount(),
                                                    record_buffer.data());
  return true;
}

bool IChannel::GetFloatValue(const std::vector<uint8_t> &record_buffer,
                             double &dest,
                             uint64_t array_index ) const {
  const size_t bit_offset = (ByteOffset() * 8 + BitOffset())
        + static_cast<size_t>( (array_index * BitCount()) );

  size_t range_check = (bit_offset + BitCount()) / 8;
  if ((bit_offset + BitCount()) % 8 != 0) {
    ++range_check;
  }
  if (range_check > record_buffer.size()) {
    MDF_ERROR() << "Range check error. Byte Index: "
      << range_check << "(" << record_buffer.size() << "). "
      << "Channel: " << Name();
    return false;
  }

  const bool little_endian = DataType() == ChannelDataType::FloatLe;
  bool valid = true;
  switch (BitCount()) {
    case 16:
      dest = detail::DbcHelper::RawToHalf(little_endian,
                                          bit_offset,
                                         BitCount(),
                                           record_buffer.data());
      break;

    case 32:
      dest = detail::DbcHelper::RawToFloat(little_endian,
                                          bit_offset,
                                          BitCount(),
                                          record_buffer.data());
      break;

    case 64:
      dest = detail::DbcHelper::RawToDouble(little_endian,
                                          bit_offset,
                                          BitCount(),
                                          record_buffer.data());
      break;

    default:
      valid = false;
      break;
  }
  return valid;
}

bool IChannel::GetTextValue(const std::vector<uint8_t> &record_buffer,
                            std::string &dest) const {
  const auto offset = ByteOffset();
  const size_t nof_bytes = BitCount() / 8 + ((BitCount() % 8) != 0 ? 1 : 0);
  bool valid = true;
  dest.clear();

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (size_t ii = 0;
           ii < nof_bytes && ii + offset < record_buffer.size();
           ++ii) {
        char in = static_cast<char>(record_buffer[ii + offset]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = MdfHelper::Latin1ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
        dest = s.str();
      }
      break;
    }

    case ChannelDataType::StringUTF8: {
      // No conversion needed
      std::ostringstream s;
      for (size_t ii = offset; ii < record_buffer.size(); ++ii) {
        char in = static_cast<char>(record_buffer[ii]);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::StringUTF16Le: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        const LittleBuffer<uint16_t> data(record_buffer, ii);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = offset; (ii + 2) <= record_buffer.size(); ii += 2) {
        const BigBuffer<uint16_t> data(record_buffer, ii);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}

bool IChannel::GetByteArrayValue(const std::vector<uint8_t> &record_buffer,
                                 std::vector<uint8_t> &dest) const {
  try {
    dest.resize(static_cast<size_t>(DataBytes()), 0);
  } catch (const std::exception&) {
    return false;
  }
  if (Type() == ChannelType::VariableLength && VlsdRecordId() > 0) {
    dest = record_buffer;
  } else {
    if (dest.size() != DataBytes()) {
      try {
        dest.resize(static_cast<size_t>(DataBytes()) );
      } catch (const std::exception&) {
        return false;
      }
    }
    if (dest.empty()) {
      return true;
    }
    if (ByteOffset() + static_cast<size_t>(DataBytes()) <= record_buffer.size()) {
      memcpy(dest.data(), record_buffer.data() + ByteOffset(), static_cast<size_t>(DataBytes()) );
    } else {
      return false;
    }
  }
  return true;
}

bool IChannel::GetCanOpenDate(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const {
  std::vector<uint8_t> date_array(7, 0);
  if (ByteOffset() + date_array.size() <= record_buffer.size()) {
    memcpy(date_array.data(), record_buffer.data() + ByteOffset(),
           date_array.size());
  } else {
    return false;
  }
  dest = MdfHelper::CanOpenDateArrayToNs(date_array);
  return true;
}

bool IChannel::GetCanOpenTime(const std::vector<uint8_t> &record_buffer,
                              uint64_t &dest) const {
  std::vector<uint8_t> time_array(6, 0);
  if (ByteOffset() + time_array.size() <= record_buffer.size()) {
    memcpy(time_array.data(), record_buffer.data() + ByteOffset(),
           time_array.size());
  } else {
    return false;
  }
  dest = MdfHelper::CanOpenTimeArrayToNs(time_array);
  return true;
}

void IChannel::SetValid(bool, uint64_t) {
  // Only MDF 4 have this functionality;
}
bool IChannel::GetValid(const std::vector<uint8_t> &, uint64_t index) const {
  return true; // Only MDF 4 have this functionality;
}

void IChannel::SetUnsignedValueLe(uint64_t value, bool valid,
                                  uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }

  const auto bytes = static_cast<size_t>(DataBytes());
  const auto max_bytes = ByteOffset() +
                         (bytes * (array_index + 1));
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }

  SetValid(valid,array_index);
  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);
  switch (bytes) {
    case 1: {
      const LittleBuffer data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 2: {
      const LittleBuffer data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 4: {
      const LittleBuffer data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetUnsignedValueBe(uint64_t value, bool valid, uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }
  const auto bytes = static_cast<size_t>(DataBytes());
  const auto max_bytes = ByteOffset() +
                         (bytes * array_index);
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }
  SetValid(valid, array_index);
  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);
  switch (bytes) {
    case 1: {
      const BigBuffer data(static_cast<uint8_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 2: {
      const BigBuffer data(static_cast<uint16_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 4: {
      const BigBuffer data(static_cast<uint32_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetSignedValueLe(int64_t value, bool valid,
                                uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }
  const auto bytes = static_cast<size_t>(DataBytes());
  const auto max_bytes = ByteOffset() +
                         (bytes * (array_index + 1));
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }
  SetValid(valid, array_index);
  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);

  switch (bytes) {
    case 1: {
      const LittleBuffer data(static_cast<int8_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 2: {
      const LittleBuffer data(static_cast<int16_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 4: {
      const LittleBuffer data(static_cast<int32_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetSignedValueBe(int64_t value, bool valid,
                                uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }
  const auto bytes = static_cast<size_t>( DataBytes() );
  const auto max_bytes = ByteOffset() +
                         (bytes * (array_index + 1));
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }
  SetValid(valid, array_index);
  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);
  switch (bytes) {
    case 1: {
      const BigBuffer data(static_cast<int8_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 2: {
      const BigBuffer data(static_cast<int16_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 4: {
      const BigBuffer data(static_cast<int32_t>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetFloatValueLe(double value, bool valid, uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }
  const auto bytes = static_cast<size_t>(DataBytes());
  const auto max_bytes = ByteOffset() +
                         (bytes * (array_index + 1));
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }
  SetValid(valid, array_index);

  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);
  switch (bytes) {
    case 4: {
      const LittleBuffer data(static_cast<float>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const LittleBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetFloatValueBe(double value, bool valid, uint64_t array_index) {
  auto &buffer = SampleBuffer();
  if (buffer.empty()) {
    MDF_ERROR() << "Sample buffer not sized. Invalid use of function.";
    return; // Invalid use of function
  }
  const auto bytes = static_cast<size_t>( DataBytes() );
  const auto max_bytes = ByteOffset() +
                         (bytes * (array_index + 1));
  if (max_bytes > buffer.size()) {
    SetValid(false, array_index);
    return;
  }
  SetValid(valid, array_index);
  const uint64_t byte_offset = ByteOffset() + (bytes * array_index);
  switch (bytes) {
    case 4: {
      const BigBuffer data(static_cast<float>(value));
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    case 8: {
      const BigBuffer data(value);
      memcpy(buffer.data() + byte_offset, data.data(), bytes);
      break;
    }

    default:
      SetValid(false, array_index);
      break;
  }
}

void IChannel::SetTextValue(const std::string &value, bool valid) {
  SetValid(valid,0);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false,0);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() < bytes) {
    memcpy(buffer.data() + ByteOffset(), value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(), value.data(), bytes - 1);
  }
}

void IChannel::SetByteArray(const std::vector<uint8_t> &value, bool valid) {
  SetValid(valid,0);
  auto &buffer = SampleBuffer();
  const size_t bytes = BitCount() / 8;
  if (bytes == 0) {
    SetValid(false, 0);
    return;
  }
  // The string shall be null terminated
  memset(buffer.data() + ByteOffset(), '\0', bytes);
  if (value.size() <= bytes) {
    memcpy(buffer.data() + ByteOffset(), value.data(), value.size());
  } else {
    memcpy(buffer.data() + ByteOffset(), value.data(), bytes);
  }
}

template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::vector<uint8_t> &dest,
                               uint64_t array_index) const {
  bool valid = false;
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t value = 0;
      valid = GetUnsignedValue(record_buffer, value, array_index);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t value = 0;
      valid = GetSignedValue(record_buffer, value, array_index);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double value = 0;
      valid = GetFloatValue(record_buffer, value, array_index);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(value);
      break;
    }

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii: {
      std::string text;
      valid = GetTextValue(record_buffer, text);
      dest.resize(text.size());
      memcpy(dest.data(), text.data(), text.size());
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      valid = GetByteArrayValue(record_buffer, dest);
      break;
    }

    case ChannelDataType::CanOpenDate: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenDate(record_buffer, ms_since_1970);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(ms_since_1970);
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);
      dest.resize(1);
      dest[0] = static_cast<uint8_t>(ms_since_1970);
      break;
    }

    default:
      break;
  }
  if (valid) {
    valid = GetValid(record_buffer, array_index);
  }
  return valid;
}

template <>
bool IChannel::GetChannelValue(const std::vector<uint8_t> &record_buffer,
                               std::string &dest,
                               uint64_t array_index) const {
  bool valid = false;
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t value = 0;
      valid = GetUnsignedValue(record_buffer, value, array_index);
      std::ostringstream s;
      s << value;
      dest = s.str();
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t value = 0;
      valid = GetSignedValue(record_buffer, value, array_index);
      dest = std::to_string(value);
      break;
    }
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double value = 0;
      valid = GetFloatValue(record_buffer, value, array_index);
      dest = IsDecimalUsed() ? MdfHelper::FormatDouble(value, Decimals())
                             : std::to_string(value);
      break;
    }

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii: {
      valid = GetTextValue(record_buffer, dest);
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      if (Type() == ChannelType::VariableLength && VlsdRecordId() != 0) {
        // This is an offset.
        uint64_t offset = 0;
        valid = GetUnsignedValue(record_buffer, offset);
        dest = std::to_string(offset);
      } else {
        std::vector<uint8_t> list;
        valid = GetByteArrayValue(record_buffer, list);
        std::ostringstream s;
        for (const auto byte : list) {
          s << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
            << static_cast<uint16_t>(byte);
        }
        dest = s.str();
      }

      break;
    }

    case ChannelDataType::CanOpenDate: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenDate(record_buffer, ms_since_1970);

      const auto ms = ms_since_1970 % 1000;
      const auto time = static_cast<time_t>(ms_since_1970 / 1000);
      const struct tm *bt = std::localtime(&time);

      std::ostringstream text;
      text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
           << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    case ChannelDataType::CanOpenTime: {
      uint64_t ms_since_1970 = 0;
      valid = GetCanOpenTime(record_buffer, ms_since_1970);

      const auto ms = ms_since_1970 % 1000;
      const auto time = static_cast<time_t>(ms_since_1970 / 1000);
      const struct tm *bt = std::localtime(&time);

      std::ostringstream text;
      text << std::put_time(bt, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0')
           << std::setw(3) << ms;
      dest = text.str();
      break;
    }

    default:
      break;
  }
  if (valid) {
    valid = GetValid(record_buffer, array_index);
  }
  return valid;
}

template <>
void IChannel::SetChannelValue(const std::string &value, bool valid,
                               uint64_t array_index) {
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
      SetUnsignedValueLe(std::stoull(value), valid, array_index);
      break;

    case ChannelDataType::UnsignedIntegerBe:
      SetUnsignedValueBe(std::stoull(value), valid, array_index);
      break;

    case ChannelDataType::SignedIntegerLe:
      SetSignedValueLe(std::stoll(value), valid, array_index);
      break;

    case ChannelDataType::SignedIntegerBe:
      SetSignedValueBe(std::stoll(value), valid, array_index);
      break;

    case ChannelDataType::FloatLe:
      SetFloatValueLe(std::stod(value), valid, array_index);
      break;

    case ChannelDataType::FloatBe:
      SetFloatValueBe(std::stod(value), valid, array_index);
      break;

    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
      SetTextValue(value, valid);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:      {
        // The input string needs to be copied to a byte array.
        auto byte_array = MdfHelper::TextToByteArray(value);
        SetByteArray(byte_array, valid);
        break;
      }

    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime:
    default:
        // Cannot find any good text to date/time
      SetValid(false, array_index);
      break;
  }
}

template <>
void IChannel::SetChannelValue(const std::vector<uint8_t> &value, bool valid,
                               uint64_t array_index) {
  switch (DataType()) {
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      SetByteArray(value, valid);
      break;

    case ChannelDataType::CanOpenTime:
    case ChannelDataType::CanOpenDate:
      if (value.size() == DataBytes()) {
        SetByteArray(value, valid);
      } else {
        SetValid(false, array_index);
      }
      break;

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
    default:
      // Conversion to byte array is suspicious
      SetValid(false, array_index);
      break;
  }
}

void IChannel::Sync(ChannelSyncType type) {}

ChannelSyncType IChannel::Sync() const { return ChannelSyncType::None; }

void IChannel::Range(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::Range() const { return {}; }

void IChannel::Limit(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::Limit() const { return {}; }

void IChannel::ExtLimit(double min, double max) {}

std::optional<std::pair<double, double>> IChannel::ExtLimit() const {
  return {};
}
IMetaData *IChannel::CreateMetaData() { return nullptr; }
IMetaData *IChannel::MetaData() const { return nullptr; }

ISourceInformation *IChannel::SourceInformation() const {
  // Only supported by MDF4
  return nullptr;
}

ISourceInformation *IChannel::CreateSourceInformation() {
  // Only supported by MDF4
  return nullptr;
}

IChannelArray *IChannel::ChannelArray(size_t index) const {
  // Only supported for MDF 4 files
  return nullptr;
}

std::vector<IChannelArray*> IChannel::ChannelArrays() const {
  // Default behaviour is reurning no arrays as in MDF 3
  return {};
}

IChannelArray *IChannel::CreateChannelArray() {
  // Only supported by MDF4
  return nullptr;
}

void IChannel::Flags(uint32_t flags) {}
uint32_t IChannel::Flags() const { return 0; }

void IChannel::SetTimestamp(double timestamp,
                            std::vector<uint8_t> &record_buffer) const {
  // If conversion is in use, reverse convert to channel value
  const auto* conversion = ChannelConversion();
  if (conversion != nullptr &&
      conversion->Type() == ConversionType::Linear &&
      conversion->Parameter(1) != 0.0) {
    timestamp -= conversion->Parameter(0);
    timestamp /= conversion->Parameter(1);
  } else if (conversion != nullptr && conversion->Type() != ConversionType::NoConversion) {
    return;
  }

  const size_t bytes = BitCount() / 8;

  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerBe:
      if (timestamp < 0) {
        timestamp = 0;
      }
      if (bytes == 1) {
        const BigBuffer data(static_cast<uint8_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 2) {
          const BigBuffer data(static_cast<uint16_t>(timestamp));
          memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 4) {
          const BigBuffer data(static_cast<uint32_t>(timestamp));
          memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const BigBuffer data(static_cast<uint64_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::UnsignedIntegerLe:
      if (timestamp < 0) {
        timestamp = 0;
      }
      if (bytes == 1) {
        const LittleBuffer data(static_cast<uint8_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 2) {
        const LittleBuffer data(static_cast<uint16_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 4) {
        const LittleBuffer data(static_cast<uint32_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const LittleBuffer data(static_cast<uint64_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::SignedIntegerBe:
      if (bytes == 1) {
        const BigBuffer data(static_cast<int8_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 2) {
        const BigBuffer data(static_cast<int16_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 4) {
        const BigBuffer data(static_cast<int32_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const BigBuffer data(static_cast<int64_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::SignedIntegerLe:
      if (bytes == 1) {
        const LittleBuffer data(static_cast<int8_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 2) {
        const LittleBuffer data(static_cast<int16_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 4) {
        const LittleBuffer data(static_cast<int32_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const LittleBuffer data(static_cast<int64_t>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::FloatLe:
      if (bytes == 4) {
        const LittleBuffer data(static_cast<float>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const LittleBuffer data(static_cast<double>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    case ChannelDataType::FloatBe:
      if (bytes == 4) {
        const BigBuffer data(static_cast<float>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      } else if (bytes == 8) {
        const BigBuffer data(static_cast<double>(timestamp));
        memcpy(record_buffer.data() + ByteOffset(), data.data(), bytes);
      }
      break;

    default:
      break;
  }
}

IChannel *IChannel::CreateChannelComposition(const std::string_view &name) {
  auto list = ChannelCompositions();
  auto itr = std::find_if(list.begin(), list.end(), [&] (auto* channel) {
    return channel != nullptr && channel->Name() == name;
  });
  if (itr != list.end()) {
    return *itr;
  }
  auto* new_channel = CreateChannelComposition();
  if (new_channel != nullptr) {
    new_channel->Name(name.data());
  }
  return new_channel;
}

void IChannel::Decimals(uint8_t precision) {
  // Only used by MDF 4
}
uint64_t IChannel::RecordId() const {
  const auto* channel_group = ChannelGroup();
  return channel_group != nullptr ? channel_group->RecordId() : 0;
}

void IChannel::AddAttachmentReference(const IAttachment *) {
  // Implements the MDF 3 functionality that doesn't support attachments
}

std::vector<const IAttachment *> IChannel::AttachmentList() const {
  // Returns an empty list as MDF 3 doesn't support attachments.
  return {};
}

uint64_t IChannel::ArraySize() const {
  const auto array_list = ChannelArrays();
  if (array_list.empty()) {
    return 1;
  }

  uint64_t count = 1;
  for (const auto* array : array_list) {
    if (array != nullptr) {
      count *= array->NofArrayValues();
    }
  }
  return count;
}

void IChannel::SetCnComment(const CnComment &cn_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(cn_comment.ToXml());
  }
}

void IChannel::GetCnComment(CnComment &cn_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    cn_comment.FromXml(meta_data->XmlSnippet());
  }
}

void IChannel::SetCnUnit(const CnUnit &unit) {
  // MDF 3 can only use the TX tag
  Unit(unit.Comment());
}

void IChannel::GetCnUnit(CnUnit& unit) const {
  unit.Comment(MdString(Unit()));
}

}  // end namespace mdf
