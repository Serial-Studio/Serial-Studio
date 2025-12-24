/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "bigbuffer.h"
#include "blockproperty.h"
#include "littlebuffer.h"
#include "mdf/imetadata.h"
#include "platform.h"

namespace mdf::detail {

enum class UpdateOption : int {
  DoNotUpdateWrittenBlock = 0,
  DoUpdateOnlyLastBlock = 1,
  DoUpdateAllBlocks = 2
};

class Md4Block;
using BlockPropertyList = std::vector<BlockProperty>;
/*
int64_t GetFilePosition(std::FILE *file);
int64_t GetLastFilePosition(std::FILE *file);
void SetFilePosition(std::FILE *file, int64_t position);
void SetFirstFilePosition(std::FILE *file);

size_t StepFilePosition(std::FILE *file, size_t steps);

[[nodiscard]] std::string ReadBlockType(std::FILE *file);
std::size_t ReadByte(std::FILE *file, std::vector<uint8_t> &dest, size_t size);
std::size_t WriteByte(std::FILE *file, const std::vector<uint8_t> &source);
std::size_t WriteBytes(std::FILE *file, size_t nof_bytes);

std::size_t ReadStr(std::FILE *file, std::string &dest, size_t size);
std::size_t WriteStr(std::FILE *file, const std::string &source, size_t size);
*/
int64_t GetFilePosition(std::streambuf& buffer);
int64_t GetLastFilePosition(std::streambuf& buffer);
void SetFilePosition(std::streambuf& buffer, int64_t position);
void SetFirstFilePosition(std::streambuf& buffer);
uint64_t StepFilePosition(std::streambuf& buffer, uint64_t steps);

[[nodiscard]] std::string ReadBlockType(std::streambuf& buffer);
uint64_t ReadByte(std::streambuf& buffer, std::vector<uint8_t> &dest, uint64_t size);
uint64_t WriteByte(std::streambuf& buffer, const std::vector<uint8_t> &source);
uint64_t WriteBytes(std::streambuf& buffer, uint64_t nof_bytes);

uint64_t ReadStr(std::streambuf& buffer, std::string &dest, uint64_t size);
uint64_t WriteStr(std::streambuf& buffer, const std::string &source, uint64_t size);

template <typename T>
std::string ToHexString(const T &value) {
  std::ostringstream temp;
  temp << "0x" << std::hex << std::setfill('0') << std::uppercase;
  if (sizeof(T) == 1) {
    temp << std::setw(2) << static_cast<int>(value);
  } else {
    temp << value;
  }
  return temp.str();
}

template <typename T>
std::string ToString(const T &value) {
  std::ostringstream temp;
  if (sizeof(T) == 1) {
    temp << std::setw(2) << static_cast<int>(value);
  } else {
    temp << value;
  }
  return temp.str();
}

std::string ToMd5String(const std::vector<uint8_t> &md5);

/** \brief Support function for opening an MDF file.
 *
 * Support function that opens an MDF file. This may fail if
 * both a writer and a reader tries to use the file at the
 * same time.
 * @param buffer Reference to a stream buffer.
 * @param filename Full path to file.
 * @param mode Open mode according to a file stream..
 * @return True if the file was opened.
 */
bool OpenMdfFile(std::streambuf& buffer, const std::string &filename,
                 std::ios_base::openmode mode);
//bool OpenMdfFile(std::FILE *&file, const std::string &filename,
//                 const std::string &mode);

class MdfBlock {
 public:
  virtual ~MdfBlock() = default;
  [[nodiscard]] virtual int64_t Index() const;
  [[nodiscard]] int64_t FilePosition() const;
  [[nodiscard]] virtual std::string BlockType() const;

  virtual void GetBlockProperty(BlockPropertyList &dest) const;
  [[nodiscard]] virtual MdfBlock *Find(int64_t index) const;

  //void SetLastFilePosition(std::FILE *file) const;
  void SetLastFilePosition(std::streambuf& buffer) const;

  [[nodiscard]] uint64_t BlockLength() const {
    return block_length_ > 0 ? block_length_ : block_size_;
  }

  /**
   * Initialize the block by copying the byte order and version.
   * Note that MDF4 always uses little endian byte order.
   * @param id_block ID or parent block
   */
  virtual void Init(const MdfBlock &id_block);

  [[nodiscard]] bool IsBigEndian() const;

  [[nodiscard]] uint16_t Version() const { return version_; }

  [[nodiscard]] int64_t Link(size_t ii) const {
    return link_list_.size() > ii ? link_list_[ii] : 0;
  }

  [[nodiscard]] const Md4Block *Md4() const;
  void Md4(const std::string &xml);

  //virtual size_t Read(std::FILE *file) = 0;
  //virtual size_t Write(std::FILE *file);

  virtual uint64_t Read(std::streambuf& buffer) = 0;
  virtual uint64_t Write(std::streambuf& buffer);

  // size_t Update(std::FILE *file) const;
  uint64_t Update(std::streambuf& buffer) const;

  //void UpdateLink(std::FILE *file, size_t link_index, int64_t link);
  void UpdateLink(std::streambuf& buffer, size_t link_index, int64_t link);

  [[nodiscard]] virtual std::string Comment() const;

 // template <typename T>
 // std::size_t ReadNumber(std::FILE *file, T &dest) const;

  //template <typename T>
  //std::size_t WriteNumber(std::FILE *file, const T &source) const;

  template <typename T>
  uint64_t ReadNumber(std::streambuf& buffer, T &dest) const;

  template <typename T>
  uint64_t WriteNumber(std::streambuf& buffer, const T &source) const;

  //void UpdateBlockSize(std::FILE *file, size_t bytes);
  void UpdateBlockSize(std::streambuf& buffer, uint64_t bytes);

  [[nodiscard]] bool IsMdf4() const;

  [[nodiscard]] const MdfBlock* HeaderBlock() const;

  [[nodiscard]] const MdfBlock* DgBlock() const;
  [[nodiscard]] const MdfBlock* CgBlock() const;

 protected:
  int64_t file_position_ = 0;  ///< 64-bit file position.
  std::string block_type_;   ///< MDF header. MDF3 has 2 characters. MDF4 has 4
                               ///< characters.
  /**
   * The MDF3 has big or little endian byte order while MDF4 always uses little
   * endian byte order.
   */
  uint16_t byte_order_ = 0;  ///< Default set to Intel (little) byte order.
  uint16_t version_ = 420;   ///< Default set to 4.2.

  uint16_t block_size_ = 0;  ///< MDF3 16-bit block size.
  uint64_t block_length_ = 0;       ///< MDF4 64-bit block size.
  uint64_t link_count_ = 0;         ///< MDF4 number of links.
  std::vector<int64_t> link_list_;  ///< MDF link list

  std::unique_ptr<MdfBlock>
      md_comment_;  ///< Most MDF4 block has a MD block referenced

  MdfBlock() = default;

  // size_t ReadHeader3(std::FILE *file);  ///< Reads a MDF3 block header.
  //size_t ReadLinks3(
  //    std::FILE *file,
  //    size_t nof_links);  ///< Reads MDF3 links into the link list.

  // size_t ReadHeader4(std::FILE *file);  ///< Read in MDF4 header and links.
  uint64_t ReadHeader3(std::streambuf& buffer);  ///< Reads a MDF3 block header.
  uint64_t ReadLinks3(std::streambuf& buffer, size_t nof_links);  ///< Reads MDF3 links into the link list.
  uint64_t ReadHeader4(std::streambuf& buffer);  ///< Read in MDF4 header and links.

  void ReadMdComment(std::streambuf& buffer, size_t index_md);
  void WriteMdComment(std::streambuf& buffer, size_t index_md);

  std::string ReadTx3(std::streambuf& buffer, size_t index_tx) const;
  std::string ReadTx4(std::streambuf& buffer, size_t index_tx) const;
  void WriteTx4(std::streambuf& buffer, size_t index_tx, const std::string &text);

  //std::size_t ReadBool(std::FILE *file, bool &dest) const;
  //std::size_t WriteBool(std::FILE *file, bool value) const;

  uint64_t ReadBool(std::streambuf& buffer, bool &dest) const;
  uint64_t WriteBool(std::streambuf& buffer, bool value) const;

  [[nodiscard]] std::string MdText() const;

  [[nodiscard]] virtual IMetaData *CreateMetaData();
  [[nodiscard]] virtual IMetaData *MetaData() const;


  void CreateMd4Block();  ///< Helper function that creates an MD4 block to this
                          ///< block



  /** \brief Reads in a list of blocks from the file.
   *
   * Helper function that reads a list of MDF4 blocks from a file.
   * @tparam T Block type
   * @param buffer Reference to a stream buffer..
   * @param block_list List of blocks.
   * @param link_index Link index to the first block.
   */
  template <typename T>
  void ReadLink4List(std::streambuf& buffer,
                     std::vector<std::unique_ptr<T>> &block_list,
                     size_t link_index);

  //template <typename T>
  //void ReadLink4List(std::FILE *file,
  //                   std::vector<std::unique_ptr<T>> &block_list,
  //                   size_t link_index);

  /** \brief Writes a list of blocks to the file.
   *
   * Helper function that writes a list of MDF4 blocks onto a file.
   * @tparam T Block type
   * @param buffer Reference to stream buffer.
   * @param block_list List of blocks.
   * @param link_index Link index to the first block.
   * @param update_option 0 = Do not update already written block, 1 = Update
   * only last block, 2 = Update all blocks
   */
  template <typename T>
  void WriteLink4List(std::streambuf& buffer,
                      std::vector<std::unique_ptr<T>> &block_list,
                      size_t link_index, UpdateOption update_option);
//  template <typename T>
//  void WriteLink4List(std::FILE *file,
//                      std::vector<std::unique_ptr<T>> &block_list,
//                      size_t link_index, UpdateOption update_option);

//  template <typename T>
//  void WriteBlock4(std::FILE *file, std::unique_ptr<T> &block,
//                   size_t link_index);
  template <typename T>
  void WriteBlock4(std::streambuf& buffer, std::unique_ptr<T> &block,
                   size_t link_index);


 private:
  const MdfBlock* parent_ = nullptr; ///< Set by the Init() function
};

/*
template <typename T>
std::size_t MdfBlock::ReadNumber(std::FILE *file, T &dest) const {
  if (IsBigEndian()) {
    BigBuffer<T> buff;
    auto count = std::fread(buff.data(), sizeof(T), 1, file);
    if (count != 1) {
      throw std::ios_base::failure("Invalid number of bytes read");
    }
    dest = buff.value();
  } else {
    LittleBuffer<T> buff;
    auto count = std::fread(buff.data(), sizeof(T), 1, file);
    if (count != 1) {
      throw std::ios_base::failure("Invalid number of bytes read");
    }
    dest = buff.value();
  }
  return sizeof(T);
}

template <typename T>
std::size_t MdfBlock::WriteNumber(std::FILE *file, const T &source) const {
  if (file == nullptr) {
    throw std::runtime_error(
        "File pointer is null. Invalid use of function.");
  }
  if (IsBigEndian()) {
    const BigBuffer buff(source);
    auto count = std::fwrite(buff.data(), 1, sizeof(T), file);
    if (count != sizeof(T)) {
      throw std::runtime_error("Invalid number of bytes written");
    }
  } else {
    const LittleBuffer buff(source);
    auto count = std::fwrite(buff.data(), 1, sizeof(T), file);
    if (count != sizeof(T)) {
      throw std::runtime_error("Invalid number of bytes written");
    }
  }
  return sizeof(T);
}
*/

//template <typename T>
//void MdfBlock::ReadLink4List(std::FILE *file,
//                           std::vector<std::unique_ptr<T>> &block_list,
//                           size_t link_index) {
//  if (block_list.empty() && (Link(link_index) > 0))
//    for (auto link = Link(link_index); link > 0; /* No ++ here*/) {
//      auto block = std::make_unique<T>();
//      block->Init(*this);
//      SetFilePosition(file, link);
//      block->Read(file);
//      link = block->Link(0);
//      block_list.emplace_back(std::move(block));
//    }
//}
/*
template <typename T>
void MdfBlock::WriteBlock4(std::FILE *file, std::unique_ptr<T> &block,
                         size_t link_index) {
  if (!block || block->FilePosition() > 0) {
    return;
  }
  block->Write(file);
  UpdateLink(file, link_index, block->FilePosition());
}

template <typename T>
void MdfBlock::WriteLink4List(std::FILE *file,
                            std::vector<std::unique_ptr<T>> &block_list,
                            size_t link_index, UpdateOption update_option) {
  for (size_t index = 0; index < block_list.size(); ++index) {
    auto &block = block_list[index];
    const bool last_block = index + 1 >= block_list.size();
    if (!block) {
      continue;
    }
    bool need_update = block->FilePosition() <= 0 ||
                       update_option == UpdateOption::DoUpdateAllBlocks;
    if (block->FilePosition() > 0 && last_block &&
        update_option == UpdateOption::DoUpdateOnlyLastBlock) {
      need_update = true;
    }

    if (!need_update) {
      continue;
    }
    block->Write(file);
    if (index == 0) {
      UpdateLink(file, link_index, block->FilePosition());
    } else {
      auto &prev = block_list[index - 1];
      if (prev) {
        prev->UpdateLink(file, 0, block->FilePosition());
      }
    }
  }
}
*/

template <typename T>
uint64_t MdfBlock::ReadNumber(std::streambuf& buffer, T &dest) const {
  if (IsBigEndian()) {
    BigBuffer<T> buff;
    const auto count = buffer.sgetn(
        reinterpret_cast<char*>(buff.data()),
        sizeof(T));
    if (count < sizeof(T)) {
      throw std::ios_base::failure("Invalid number of bytes read");
    }

    dest = buff.value();
  } else {
    LittleBuffer<T> buff;
    const auto count = buffer.sgetn(
        reinterpret_cast<char*>(buff.data()),
        sizeof(T));
    if (count < sizeof(T)) {
      throw std::ios_base::failure("Invalid number of bytes read");
    }

    dest = buff.value();
  }
  return sizeof(T);
}

template <typename T>
uint64_t MdfBlock::WriteNumber(std::streambuf& buffer, const T &source) const {
  if (IsBigEndian()) {
    const BigBuffer buff(source);
    const auto count = buffer.sputn(
        reinterpret_cast<const char*>(buff.data()), sizeof(T));
    if (count < sizeof(T)) {
      throw std::runtime_error("Invalid number of bytes written");
    }
  } else {
    const LittleBuffer buff(source);
    const auto count = buffer.sputn(
        reinterpret_cast<const char*>(buff.data()), sizeof(T));
    if (count < sizeof(T)) {
      throw std::runtime_error("Invalid number of bytes written");
    }
  }
  return sizeof(T);
}

template <typename T>
void MdfBlock::ReadLink4List(std::streambuf& buffer,
                                   std::vector<std::unique_ptr<T>> &block_list,
                                   size_t link_index) {
  if (block_list.empty() && (Link(link_index) > 0))
    for (int64_t link = Link(link_index); link > 0; /* No ++ here*/) {
      auto block = std::make_unique<T>();
      block->Init(*this);
      SetFilePosition(buffer, link);
      block->Read(buffer);
      link = block->Link(0);
      block_list.emplace_back(std::move(block));
    }
}

template <typename T>
void MdfBlock::WriteBlock4(std::streambuf& buffer, std::unique_ptr<T> &block,
                                 size_t link_index) {
  if (!block || block->FilePosition() > 0) {
    return;
  }
  block->Write(buffer);
  UpdateLink(buffer, link_index, block->FilePosition());
}

template <typename T>
void MdfBlock::WriteLink4List(std::streambuf& buffer,
                                    std::vector<std::unique_ptr<T>> &block_list,
                                    size_t link_index,
                                    UpdateOption update_option) {
  for (size_t index = 0; index < block_list.size(); ++index) {
    auto &block = block_list[index];
    const bool last_block = index + 1 >= block_list.size();
    if (!block) {
      continue;
    }
    bool need_update = block->FilePosition() <= 0 ||
                       update_option == UpdateOption::DoUpdateAllBlocks;
    if (block->FilePosition() > 0 && last_block &&
        update_option == UpdateOption::DoUpdateOnlyLastBlock) {
      need_update = true;
    }

    if (!need_update) {
      continue;
    }
    block->Write(buffer);
    if (index == 0) {
      UpdateLink(buffer, link_index, block->FilePosition());
    } else {
      auto &prev = block_list[index - 1];
      if (prev) {
        prev->UpdateLink(buffer, 0, block->FilePosition());
      }
    }
  }
}

}  // namespace mdf::detail
