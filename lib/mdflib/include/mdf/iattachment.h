/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file iattachment.h
 * \brief Interface against an attached file.
 */
#pragma once
#include <optional>
#include <string>
#include "mdf/iblock.h"
#include "mdf/imetadata.h"
#include "mdf/atcomment.h"

namespace mdf {

/** \brief Interface against an attached file.
 *
 * The attachment (AT) block is used to reference and/or embed the file content.
 * Note that the filename must be a valid path if the file should be embedded.
 */
class IAttachment : public IBlock {
 public:
  ~IAttachment() override = default; ///< Default destructor

  /** \brief Sets the Creator index. */
  virtual void CreatorIndex(uint16_t creator) = 0;

  /** \brief Returns the creator index.
   *
   * The index is actually the index to a File Header (FH) block in the file.
   * The first FH block has index 0 while the next has index 1 and so on.
   * @return Index to FH block.
   */
  [[nodiscard]] virtual uint16_t CreatorIndex() const = 0;


  virtual void IsEmbedded(bool embed) = 0; ///< Set true if embedded.
  [[nodiscard]] virtual bool IsEmbedded() const = 0; ///< True if embedded.

  virtual void IsCompressed(bool compress) = 0; ///< Set true tp compress.
  [[nodiscard]] virtual bool IsCompressed() const = 0; ///< True if compressed.

  /** \brief Return the MDG checksum if it exist.
   *
   * Returns the MD5 checksum string if it is supplied.
   * @return MD5 checksum string.
   */
  [[nodiscard]] virtual std::optional<std::string> Md5() const = 0;

  /** \brief Sets the filename. Include path.
   *
   * Sets the filename. Include a valid path if MD5 and contents shall be
   * included.
   * @param filename Filename with valid path.
   */
  virtual void FileName(const std::string& filename) = 0;

  /** \brief Returns the filename.
   *
   * Returns the filename.
   * @return Filename.
   */
  [[nodiscard]] virtual const std::string& FileName() const = 0;

  /** \brief Sets the MIME type of the file.
   *
   * The file type should be a MIME type string.
   * @param file_type MIME type string.
   */
  virtual void FileType(const std::string& file_type) = 0;

  /** \brief Returns the MIME type of the string.
   *
   * Returns the MIME type string
   * @return
   */
  [[nodiscard]] virtual const std::string& FileType() const = 0;

  /** \brief Create a meta-data (MD) block.
   *
   * Creates a meta-data (MD) block. Note that the AT block normally only
   * have the description TX-tag included. See also IMetaData class.
   * @return Returns a pointer to an IMetaData class.
   */
  [[nodiscard]] virtual IMetaData* CreateMetaData() = 0;

  /** \brief Returns the meta-data (MD) block.
   *
   * Returns existing meta-data block. Note, may return nullptr.
   * @return Pointer to a IMeteData block or nullptr if it doesn't exists.
   */
  [[nodiscard]] virtual IMetaData* MetaData() const = 0;

  void SetAtComment(const AtComment& at_comment);
  void GetAtComment(AtComment& at_comment) const;
};

}  // namespace mdf
