/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "at4block.h"

#include <cerrno>
#include <sstream>
#include <fstream>

#include "mdf/cryptoutil.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdflogstream.h"
#include "mdf/zlibutil.h"
#include "platform.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

using namespace fs;

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexFilename = 1;
constexpr size_t kIndexType = 2;
constexpr size_t kIndexMd = 3;

using namespace mdf;

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & mdf::detail::At4Flags::kEmbeddedData) {
    s << "Embedded";
  }
  if (flag & mdf::detail::At4Flags::kCompressedData) {
    s << (s.str().empty() ? "Compressed" : ",Compressed");
  }
  if (flag & mdf::detail::At4Flags::kUsingMd5) {
    s << (s.str().empty() ? "MD5" : ",MD5");
  }
  return s.str();
}

bool CopyBytes(std::streambuf& source, std::streambuf& dest, uint64_t nof_bytes) {
  uint8_t temp = 0;
  for (uint64_t ii = 0; ii < nof_bytes; ++ii) {
    temp = source.sbumpc();
    dest.sputc(static_cast<char>(temp));
  }
  return true;
}

std::string ConvertMd5Buffer(const std::vector<uint8_t>& buffer) {
  std::ostringstream temp;
  for (auto byte : buffer) {
    temp << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
         << static_cast<uint16_t>(byte);
  }
  return temp.str();
}

bool FileToBuffer(const std::string& filename, mdf::ByteArray& dest) {
  try {
    path fullname = u8path(filename);
    const auto size = file_size(fullname);
    if (size > 0) {
      dest.resize(static_cast<size_t>(size), 0);
      std::FILE* file = nullptr;
      Platform::fileopen(&file, filename.c_str(), "rb");
      if (file != nullptr) {
        const auto nof_bytes = fread(dest.data(), 1, static_cast<size_t>(size), file);
        if (nof_bytes < size) {
          dest.resize(nof_bytes);
        }
      } else {
        MDF_ERROR() << "Failed to open file. File: " << filename;
        dest.clear();
        dest.shrink_to_fit();
        return false;
      }
    } else {
      dest.clear();
      dest.shrink_to_fit();
    }

  } catch (const std::exception& err) {
    MDF_ERROR() << "File error when reading file to byte array. Error: "
                << err.what() << ", File: " << filename;
    return false;
  }
  return true;
}

}  // namespace

namespace mdf::detail {
At4Block::At4Block() { block_type_ = "##AT"; }

void At4Block::GetBlockProperty(BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next AT", ToHexString(Link(kIndexNext)),
                    "Link to next attach", BlockItemType::LinkItem);
  dest.emplace_back("File Name TX", ToHexString(Link(kIndexFilename)),
                    filename_, BlockItemType::LinkItem);
  dest.emplace_back("Mime Type TX", ToHexString(Link(kIndexType)), file_type_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  std::string name;
  if (Link(kIndexFilename) > 0) {
    try {
      fs::path p = fs::u8path(filename_);
      const auto& u8str = p.filename().u8string();
      name = std::string(u8str.begin(), u8str.end());
    } catch (const std::exception&) {
      name = "<invalid>";
    }
    dest.emplace_back("File Name", name, name == filename_ ? "" : filename_);
  }
  if (Link(kIndexType) > 0) {
    dest.emplace_back("File Type", file_type_);
  }
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Creator Index", std::to_string(creator_index_));
  if (flags_ & At4Flags::kUsingMd5) {
    dest.emplace_back("MD5 Checksum", ToMd5String(md5_));
  }
  dest.emplace_back("Original Size [byte]", std::to_string(original_size_));
  dest.emplace_back("Embedded Size [byte]", std::to_string(nof_bytes_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t At4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, flags_);
  bytes += ReadNumber(buffer, creator_index_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 4);
  bytes += ReadByte(buffer, md5_, 16);
  bytes += ReadNumber(buffer, original_size_);
  bytes += ReadNumber(buffer, nof_bytes_);
  // Do not read in the data BLOB at this point but store the file position for
  // that data, so it is fast to get the data later
  data_position_ = GetFilePosition(buffer);

  filename_ = ReadTx4(buffer, kIndexFilename);
  file_type_ = ReadTx4(buffer, kIndexType);
  ReadMdComment(buffer, kIndexMd);

  return bytes;
}

uint64_t At4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  ByteArray data_buffer;
  try {
    path filename = u8path(filename_);
    if (!fs::exists(filename) && IsEmbedded()) {
      MDF_ERROR() << "Attachment File doesn't exist. File: " << filename_;
    }
    if (fs::exists(filename)) {
      if (const auto md5 = CreateMd5FileChecksum(filename_, md5_); md5) {
        flags_ |= At4Flags::kUsingMd5;
      }
      original_size_ = file_size(filename);
      if (IsEmbedded() && IsCompressed()) {
        if (const bool compress = Deflate(filename_, data_buffer); !compress) {
          MDF_ERROR() << "Compress failure. File: " << filename;
          return 0;
        }
      } else if (IsEmbedded()) {
        if (const auto read = FileToBuffer(filename_, data_buffer); !read) {
          MDF_ERROR() << "File to buffer failure. File: " << filename;
          return 0;
        }
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Attachment File error. Error: " << err.what()
                << ", File: " << filename_;
    return 0;
  }

  nof_bytes_ = data_buffer.size();

  block_type_ = "##AT";
  block_length_ = 24 + (4 * 8) + 2 + 2 + 4 + 16 + 8 + 8 + nof_bytes_;
  link_list_.resize(4, 0);

  WriteTx4(buffer, kIndexFilename, filename_);
  WriteTx4(buffer, kIndexType, file_type_);
  WriteMdComment(buffer, kIndexMd);

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteNumber(buffer, creator_index_);
  bytes += WriteBytes(buffer, 4);
  if (md5_.size() == 16) {
    bytes += WriteByte(buffer, md5_);
  } else {
    bytes += WriteBytes(buffer, 16);
  }
  bytes += WriteNumber(buffer, original_size_);
  bytes += WriteNumber(buffer, nof_bytes_);
  data_position_ = FilePosition();
  if (nof_bytes_ > 0) {
    bytes += WriteByte(buffer, data_buffer);
  }
  UpdateBlockSize(buffer, bytes);

  return bytes;
}

void At4Block::ReadData(std::streambuf& buffer,
                        const std::string& dest_file) const {
  if (data_position_ <= 0) {
    throw std::invalid_argument("File is not opened or data position not read");
  }
  SetFilePosition(buffer, data_position_);
  if (IsEmbedded()) {
    std::filebuf dest;
    dest.open( dest_file,
            std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (!dest.is_open()) {
      throw std::ios_base::failure("Failed to open the destination file");
    }
    const bool error = IsCompressed() ? !Inflate(buffer, dest, nof_bytes_)
                                      : !CopyBytes(buffer, dest, nof_bytes_);
    dest.close();
    if ( error ) {
      throw std::ios_base::failure("Failed to copy correct number of bytes");
    }
  } else {
    // Need to copy the source file
    fs::path s = fs::u8path(filename_);
    fs::path d = fs::u8path(dest_file);
    if (s != d) {
      fs::copy_file(
          s, d, fs::copy_options::overwrite_existing);
    }
  }
  if (flags_ & At4Flags::kUsingMd5) {
    std::vector<uint8_t> md5(16, 0xFF);
    CreateMd5FileChecksum(dest_file, md5);
    if (md5 != md5_) {
      throw std::runtime_error("Mismatching MD5 checksums");
    }
  }
}

void At4Block::ReadData(std::streambuf& buffer,
                        std::streambuf& dest_buffer) const {
  if (data_position_ <= 0) {
    throw std::invalid_argument("File is not opened or data position not read");
  }
  SetFilePosition(buffer, data_position_);
  if (IsEmbedded()) {
    const bool error = IsCompressed() ? !Inflate(buffer, dest_buffer, nof_bytes_)
                                      : !CopyBytes(buffer, dest_buffer, nof_bytes_);
    if ( error ) {
      throw std::ios_base::failure("Failed to copy correct number of bytes");
    }
  } else {
    // Need to copy the source file
    std::ifstream source(filename_, std::ios_base::in | std::ios_base::binary);
    if (!source.is_open()) {
      throw std::runtime_error("Failed to open the source file.");
    }
    for (int input = source.get(); !source.eof(); input = source.get()) {
      dest_buffer.sputc(static_cast<char>(input));
    }
  }
  // Difficult to check that the checksum was correct.
}

void At4Block::IsEmbedded(bool embed) {
  if (embed) {
    flags_ |= At4Flags::kEmbeddedData;
  } else {
    flags_ &= ~At4Flags::kEmbeddedData;
  }
}
void At4Block::FileName(const std::string& filename) { filename_ = filename; }
const std::string& At4Block::FileName() const { return filename_; }
void At4Block::FileType(const std::string& file_type) {
  file_type_ = file_type;
}
const std::string& At4Block::FileType() const { return file_type_; }
bool At4Block::IsEmbedded() const { return flags_ & At4Flags::kEmbeddedData; }
void At4Block::IsCompressed(bool compress) {
  if (compress) {
    flags_ |= At4Flags::kCompressedData;
  } else {
    flags_ &= ~At4Flags::kCompressedData;
  }
}
bool At4Block::IsCompressed() const {
  return flags_ & At4Flags::kCompressedData;
}

std::optional<std::string> At4Block::Md5() const {
  if ((flags_ & At4Flags::kUsingMd5) == 0) {
    return {};
  }
  return ConvertMd5Buffer(md5_);
}

void At4Block::CreatorIndex(uint16_t creator) { creator_index_ = creator; }
uint16_t At4Block::CreatorIndex() const { return creator_index_; }

}  // namespace mdf::detail