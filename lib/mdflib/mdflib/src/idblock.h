/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdfblock.h"

namespace mdf::detail {
class IdBlock : public MdfBlock {
 public:
  IdBlock();
  void GetBlockProperty(BlockPropertyList &dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  [[nodiscard]] std::string FileId() const;
  [[nodiscard]] std::string VersionString() const;

  void ProgramId(const std::string &program_id) {
    program_identifier_ = program_id;
  }

  [[nodiscard]] std::string ProgramId() const;

  void IsFinalized(bool finalized, std::streambuf& buffer, uint16_t standard_flags,
                   uint16_t custom_flags);
  [[nodiscard]] bool IsFinalized(uint16_t &standard_flags,
                                 uint16_t &custom_flags) const;

  void MinorVersion(int minor);

  void SetDefaultMdf3Values();

 private:
  std::string file_identifier_ =
      "MDF     ";  ///< Note the string must be 8 characters including spaces
  std::string format_identifier_ = "4.20";
  std::string program_identifier_ = "MdfWrite";
  /* uint16_t byte_order_ = 0; Default 0 = Little endian (Intel byte order).
   * Defined in MdfBlock class. */
  uint16_t floating_point_format_ = 0;  ///< Default IEEE standard
  /* uint16_t version_ = 420;   Defined in MdfBlock class. */
  uint16_t code_page_number_ = 0;
  /* 2 byte reserved */
  /* 26 byte reserved */
  uint16_t standard_flags_ = 0;
  uint16_t custom_flags_ = 0;
};
}  // namespace mdf::detail
