/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "mdfblock.h"
#include "mdf/idatagroup.h"

namespace mdf::detail {

class DataBlock : public MdfBlock {
 public:
  void DataPosition(int64_t position) { data_position_ = position; }
  [[nodiscard]] int64_t DataPosition() const { return data_position_; }

  [[nodiscard]] virtual uint64_t DataSize() const = 0;
  virtual uint64_t CopyDataToFile( std::streambuf& from_file, std::streambuf& to_file) const;
  virtual uint64_t CopyDataToBuffer(std::streambuf& from_file,
                                  std::vector<uint8_t>& buffer,
                                  uint64_t& buffer_index) const;
  virtual void DgBlock(IDataGroup* dg_block) { dg_block_ = dg_block;}

  virtual uint64_t ReadData(std::streambuf& buffer);
  virtual bool Data(const std::vector<uint8_t>& data);
  void ClearData() {
    data_.clear();
    data_.shrink_to_fit();
  }
 protected:
  int64_t data_position_ = 0;
  IDataGroup* dg_block_ = nullptr; ///< Needed for the writing
  std::vector<uint8_t> data_; ///< Temporary storage of data

};

}  // namespace mdf::detail
