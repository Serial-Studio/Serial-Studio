/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "datablock.h"

#include <array>
namespace mdf::detail {

uint64_t DataBlock::CopyDataToFile(std::streambuf& from_file,
                                 std::streambuf& to_file) const {
  SetFilePosition(from_file, DataPosition());

  uint64_t data_size = DataSize();
  if (data_size == 0) {
    return 0;
  }
  uint64_t count = 0;
  std::array<char, 10'000> temp{};
  uint64_t bytes_to_read = std::min(data_size, static_cast<uint64_t>(temp.size()) );
  for (uint64_t reads = from_file.sgetn(temp.data(),
                                    static_cast<std::streamsize>(bytes_to_read));
       reads > 0 && bytes_to_read > 0 && data_size > 0;
       reads = from_file.sgetn(temp.data(),
                                   static_cast<std::streamsize>(bytes_to_read) )) {
    const uint64_t writes = to_file.sputn(temp.data(),
                                      static_cast<std::streamsize>(reads));
    count += writes;
    if (writes != reads) {
      break;
    }

    data_size -= reads;
    bytes_to_read = std::min(data_size, static_cast<uint64_t>(temp.size()) );
  }
  return count;
}

uint64_t DataBlock::CopyDataToBuffer(std::streambuf& buffer,
                                   std::vector<uint8_t> &dest,
                                   uint64_t &buffer_index) const {
  SetFilePosition(buffer, DataPosition());
  const uint64_t data_size = DataSize();
  if (data_size == 0) {
    return 0;
  }
  if (dest.size() < (buffer_index + data_size)) {
    throw std::runtime_error("Buffer overflow detected.");
  }

  const uint64_t reads = buffer.sgetn(
      reinterpret_cast<char*>(dest.data()) + buffer_index,
      static_cast<std::streamsize>(data_size));
  buffer_index += reads;
  return reads;
}

bool DataBlock::Data(const std::vector<uint8_t> &data) {
  data_ = data;
  return true;
}

uint64_t DataBlock::ReadData(std::streambuf& buffer) {
  if (data_position_ <= 0) {
    return 0;
  }
  SetFilePosition(buffer,data_position_);
  return ReadByte(buffer, data_,DataSize());
}

}  // namespace mdf::detail