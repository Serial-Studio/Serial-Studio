/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <string>
#include <utility>

#include "dz4block.h"

#include <mdf/zlibutil.h>



namespace {

std::string MakeZipTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "ZLIB Deflate";
    case 1:
      return "Transposition + ZLIB Deflate";
    default:
      break;
  }
  return "Unknown";
}
}  // namespace

namespace mdf::detail {

void Dz4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Original Block Type", orig_block_type_);
  dest.emplace_back("Zip Type", MakeZipTypeString(type_));
  dest.emplace_back("Zip Parameter", std::to_string(parameter_));
  dest.emplace_back("Original Size  [byte]", std::to_string(orig_data_length_));
  dest.emplace_back("Data Size  [byte]", std::to_string(data_length_));
}

uint64_t Dz4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadStr(buffer, orig_block_type_, 2);
  bytes += ReadNumber(buffer, type_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 1);
  bytes += ReadNumber(buffer, parameter_);
  bytes += ReadNumber(buffer, orig_data_length_);
  bytes += ReadNumber(buffer, data_length_);
  data_position_ = GetFilePosition(buffer);
  return bytes;
}

uint64_t Dz4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;
  if (update) {
    // The DZ block properties cannot be changed after it has been written
    return block_length_;
  }

  block_type_ = "##DZ";
  link_list_.clear();
  block_length_ = 24 + 2 + 1 + 1 + 4 + 8 + 8 + data_length_;

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteStr(buffer, orig_block_type_, 2);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteBytes(buffer, 1);
  bytes += WriteNumber(buffer, parameter_);
  bytes += WriteNumber(buffer, orig_data_length_);
  bytes += WriteNumber(buffer, data_length_);
  bytes += WriteByte(buffer,data_);
  UpdateBlockSize(buffer, bytes);

  return bytes;
}

uint64_t Dz4Block::CopyDataToFile(std::streambuf& from_file,
                                std::streambuf& to_file) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }
  SetFilePosition(from_file, data_position_);

  uint64_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
    case Dz4ZipType::Deflate: {
      const bool inflate = Inflate(from_file, to_file, data_length_);
      count = inflate ? orig_data_length_ : 0;
      break;
    }

    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp(static_cast<size_t>(data_length_), 0);
      from_file.sgetn(
            reinterpret_cast<char*>(temp.data()),
                static_cast<std::streamsize>(temp.size()) );
      ByteArray out(static_cast<size_t>(orig_data_length_), 0);
      const bool inflate = Inflate(temp, out);
      InvTranspose(out, parameter_);
      to_file.sputn(reinterpret_cast<char*>(out.data()),
                    static_cast<std::streamsize>(out.size()) );
      count = inflate ? orig_data_length_ : 0;
      break;
    }

    default:
      break;
  }
  return count;
}

uint64_t Dz4Block::CopyDataToBuffer(std::streambuf& from_file,
                                  std::vector<uint8_t> &dest,
                                  uint64_t &buffer_index) const {
  if (data_position_ == 0 || orig_data_length_ == 0 || data_length_ == 0) {
    return 0;
  }

  SetFilePosition(from_file, data_position_);

  uint64_t count = 0;
  switch (static_cast<Dz4ZipType>(type_)) {
    case Dz4ZipType::Deflate: {
      ByteArray temp(static_cast<size_t>(data_length_), 0);
      from_file.sgetn( reinterpret_cast<char*>(temp.data()),
                              static_cast<std::streamsize>(temp.size()) );
      ByteArray out(static_cast<size_t>(orig_data_length_), 0);
      Inflate(temp, out);
      count = orig_data_length_;
      memcpy(dest.data() + buffer_index, out.data(), static_cast<size_t>(count) );
      buffer_index += count;
      break;
    }

    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp(static_cast<size_t>(data_length_), 0);
      from_file.sgetn(reinterpret_cast<char*>(temp.data()),
                      static_cast<std::streamsize>(temp.size()) );
      ByteArray out(static_cast<size_t>( orig_data_length_ ), 0);
      Inflate(temp, out);
      InvTranspose(out, parameter_);

      count = orig_data_length_;
      memcpy(dest.data() + buffer_index, out.data(), static_cast<size_t>(count) );
      buffer_index += count;
      break;
    }

    default:
      break;
  }
  return count;
}

bool Dz4Block::Data(const std::vector<uint8_t> &uncompressed_data) {
  bool compress = true;
  if (Type() == Dz4ZipType::TransposeAndDeflate) {
    if (Parameter() == 0) {
      Type(Dz4ZipType::Deflate);
    }
  } else {
    Type(Dz4ZipType::Deflate);
    Parameter(0);
  }
  if (uncompressed_data.empty()) {
    orig_data_length_ = 0;
    data_length_ = 0;
    data_.clear();
    data_.shrink_to_fit();
    return compress;
  }

  switch (Type()) {
    case Dz4ZipType::TransposeAndDeflate: {
      ByteArray temp = uncompressed_data;
      Transpose(temp, static_cast<size_t>(parameter_));

      data_.clear();
      data_.shrink_to_fit();
      data_.reserve(uncompressed_data.size());

      compress = Deflate(temp,data_);
      break;
    }

    case Dz4ZipType::Deflate:
    default:
      data_.clear();
      data_.shrink_to_fit();
      data_.reserve(uncompressed_data.size());
      compress = Deflate(uncompressed_data,data_);
      break;
  }
  orig_data_length_ = static_cast<uint64_t>(uncompressed_data.size()) ;
  data_length_ = static_cast<uint64_t>(data_.size());

  return compress;
}

}  // namespace mdf::detail