/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <ios>
#include <sstream>
#include <string>

#ifdef WIN32
#include <windows.h>
#endif

#include "idblock.h"
#include "mdf/mdfhelper.h"

using namespace std;
namespace mdf::detail {
IdBlock::IdBlock() {
  block_type_ = "ID";
  block_length_ = 64;
  block_size_ = 64;
  file_position_ = -1;  // Set to -1 which indicate that is neither read nor
                        // written to disc. 0 is
  // its file position when written or read.
}

void IdBlock::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("File ID", FileId());
  dest.emplace_back("Version", VersionString());
  dest.emplace_back("Program", ProgramId());
  if (version_ < 400) {
    dest.emplace_back(
        "Byte Order", byte_order_ == 0 ? "Little Endian" : "Big Endian",
        byte_order_ == 0 ? "Intel byte order" : "Motorola byte order");
    dest.emplace_back("Version Number", std::to_string(version_));
#ifdef WIN32
    CPINFOEXA cp_info{};
    GetCPInfoExA(code_page_number_, CP_ACP, &cp_info);
    dest.emplace_back("Code Page", std::to_string(code_page_number_),
                      cp_info.CodePageName);
#else
    dest.emplace_back("Code Page", std::to_string(code_page_number_));
#endif
  }
  dest.emplace_back("Version Number", std::to_string(version_));
  if (standard_flags_ & 0x01) {
    dest.emplace_back("Update CG count","True");
  }
  if (standard_flags_ & 0x02) {
    dest.emplace_back("Update SR Count","True");
  }
  if (standard_flags_ & 0x04) {
    dest.emplace_back("Update length of last DT Block", "True");
  }
  if (standard_flags_ & 0x08) {
    dest.emplace_back("Update length of last RD Block", "True");
  }
  if (standard_flags_ & 0x10) {
    dest.emplace_back("Update length of last DL Block", "True");
  }
  if (standard_flags_ & 0x20) {
    dest.emplace_back("Update length of data VLSD Block", "True");
  }
  if (standard_flags_ & 0x40) {
    dest.emplace_back("Update length of offset VLSD Block", "True");
  }
  if (custom_flags_ != 0) {
    dest.emplace_back("Custom Flags", std::to_string(custom_flags_));
  }

}

uint64_t IdBlock::Read(std::streambuf& buffer) {
  block_type_ = "ID";
  block_length_ = 64;
  SetFirstFilePosition(buffer);

  file_position_ = GetFilePosition(buffer);
  uint64_t bytes = ReadStr(buffer, file_identifier_, 8);
  bytes += ReadStr(buffer, format_identifier_, 8);
  bytes += ReadStr(buffer, program_identifier_, 8);
  bytes += ReadNumber(buffer, byte_order_);  // Note defined in MdfBlock class
  bytes += ReadNumber(buffer, floating_point_format_);
  bytes += ReadNumber(buffer, version_);
  bytes += ReadNumber(buffer, code_page_number_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 28);
  bytes += ReadNumber(buffer, standard_flags_);
  bytes += ReadNumber(buffer, custom_flags_);
  return bytes;
}

uint64_t IdBlock::Write(std::streambuf& buffer) {
  // Check if it has been written. Update is not supported in ID
  if (FilePosition() == 0) {
    return 64;
  }

  //  Do not call MdfBlock::Write(file) as the other block writes do

  SetFirstFilePosition(buffer);
  file_position_ = GetFilePosition(buffer);
  uint64_t bytes = WriteStr(buffer, file_identifier_, 8);
  bytes += WriteStr(buffer, format_identifier_, 8);
  bytes += WriteStr(buffer, program_identifier_, 8);
  bytes += WriteNumber(buffer, byte_order_);
  bytes += WriteNumber(buffer, floating_point_format_);
  bytes += WriteNumber(buffer, version_);
  bytes += WriteNumber(buffer, code_page_number_);
  std::vector<uint8_t> reserved(28, 0);
  bytes += WriteByte(buffer, reserved);
  bytes += WriteNumber(buffer, standard_flags_);
  bytes += WriteNumber(buffer, custom_flags_);
  if (bytes != 64) {
    throw std::runtime_error("ID block not 64 bytes");
  }
  return bytes;
}

std::string IdBlock::FileId() const {
  std::string temp = file_identifier_;
  MdfHelper::Trim(temp);
  return temp;
}

std::string IdBlock::VersionString() const {
  std::string temp = format_identifier_;
  MdfHelper::Trim(temp);
  return temp;
}

std::string IdBlock::ProgramId() const {
  std::string temp = program_identifier_;
  MdfHelper::Trim(temp);
  return temp;
}

void IdBlock::SetDefaultMdf3Values() {
  file_identifier_ = "MDF     ";
  format_identifier_ = "3.30";
  program_identifier_ = "MdfWrite";
  byte_order_ = 0;  // Little endian
  floating_point_format_ = 0;
  version_ = 330;
  code_page_number_ = 65001;  // UTF-8
  standard_flags_ = 0;
  custom_flags_ = 0;
}

void IdBlock::MinorVersion(int minor) {
  const int major = format_identifier_.empty()
                        ? 4
                        : std::stoi(format_identifier_.substr(0, 1));
  if (minor <= 0) {
    minor = 0;
  } else if (minor < 10) {
    minor *= 10;
  } else if (minor >= 100) {
    minor %= 100;
  }

  std::ostringstream temp;
  temp << major << ".";
  if (minor <= 0) {
    temp << "00";
  } else {
    temp << minor;
  }
  format_identifier_ = temp.str();
  version_ = 100 * major + minor;
}

void IdBlock::IsFinalized(bool finalized, std::streambuf& buffer,
                          uint16_t standard_flags, uint16_t custom_flags) {
  if (finalized) {
    file_identifier_ = "MDF     ";
    standard_flags_ = 0;
    custom_flags_ = 0;
  } else {
    file_identifier_ = "UnFinMF ";
    standard_flags_ = standard_flags;
    custom_flags_ = custom_flags;
  }

  // Check if the file also needs update
  if (FilePosition() == 0) {
    SetFirstFilePosition(buffer);
    WriteStr(buffer, file_identifier_, 8);
    SetFilePosition(buffer, 60);
    WriteNumber(buffer, standard_flags_);
    WriteNumber(buffer, custom_flags_);
  }
}

bool IdBlock::IsFinalized(uint16_t &standard_flags,
                          uint16_t &custom_flags) const {
  standard_flags = standard_flags_;
  custom_flags = custom_flags_;
  return !(!file_identifier_.empty() && file_identifier_[0] == 'U');
}

}  // namespace mdf::detail
