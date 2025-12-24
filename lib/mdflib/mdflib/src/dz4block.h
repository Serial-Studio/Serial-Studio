/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datablock.h"

namespace mdf::detail {
enum class Dz4ZipType : uint8_t { Deflate = 0, TransposeAndDeflate = 1 };

class Dz4Block : public DataBlock {
 public:
  void OrigBlockType(const std::string& block_type) {
    orig_block_type_ = block_type;
  }
  [[nodiscard]] std::string OrigBlockType() const { return orig_block_type_; }

  void Type(Dz4ZipType type) { type_ = static_cast<uint8_t>(type); }
  [[nodiscard]] Dz4ZipType Type() const {
    return static_cast<Dz4ZipType>(type_);
  }

  void Parameter(uint32_t parameter) {parameter_ = parameter; }
  [[nodiscard]] uint32_t Parameter() const { return parameter_; }


  [[nodiscard]] uint64_t DataSize() const override { return orig_data_length_; }
  [[nodiscard]] uint64_t CompressedDataSize() const { return data_length_; }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  uint64_t CopyDataToFile(std::streambuf& from_file,
                        std::streambuf& to_file) const override;
  uint64_t CopyDataToBuffer(std::streambuf& from_file, std::vector<uint8_t>& buffer,
                          uint64_t& buffer_index) const override;

  bool Data(const std::vector<uint8_t>& uncompressed_data) override;

 private:
  std::string orig_block_type_ = "DT";
  uint8_t type_ = 0; ///< Default is Deflate
  /* 1 byte reserved */
  uint32_t parameter_ = 0;
  uint64_t orig_data_length_ = 0;
  uint64_t data_length_ = 0;



};
}  // namespace mdf::detail
