/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file mdffile.h
 * \brief Interface against an MDF file object.
 */
#pragma once
#include <cstdio>

#include <string>
#include <vector>

#include "mdf/iheader.h"

namespace mdf {

class IAttachment;
class IDataGroup;
class IChannel;

/** \brief List of pointers to attachments.
 */
using AttachmentList = std::vector<const IAttachment*>;

/** \brief List of pointers to measurements (DG block).
 */
using DataGroupList = std::vector<IDataGroup*>;

/** \class MdfFile mdffile.h mdf/mdffile.h
 * \brief Implements an user interface against a MDF file.
 *
 * Implements an interface against a MDF file object. Note
 * that the object holds the file contents.
 *
 */
class MdfFile {
 public:
  virtual ~MdfFile() = default;  ///< Default destructor

  /** \brief Fetch a list of attachments.
   *
   * Fetches a list of file attachments. An attachment is a
   * reference to an external or an embedded file. Note that
   * MDF3 doesn't support attachments.
   * @param dest Destination list of attachments.
   */
  virtual void Attachments(AttachmentList& dest) const = 0;

  /** \brief Fetch a list of all measurements.
   *
   * Fetches a list aof all measurements (DG blocks) in the file.
   * @param dest Destination list of all measurements.
   */
  virtual void DataGroups(DataGroupList& dest) const = 0;

  /** \brief Returns the file version.
   *
   * Returns the MDF file version.
   * @return File version.
   */
  [[nodiscard]] virtual std::string Version() const = 0;

  /** \brief Returns the main version of the file.
   *
   * Returns the main version (4.10 -> 4) of the file.
   * @return Returns the main version of the file (1..4).
   */
  [[nodiscard]] int MainVersion() const;

  /** \brief Sets the minor version of the file.
   *
   * Sets the minor version of the file. This function is typical used
   * when saving a MDF file.
   * @param minor Minor version of the file.
   */
  virtual void MinorVersion(int minor) = 0;

  /** \brief Returns the minor version number of the MDF file.
   *
   * Returns the minor version of the MDF file. Note that it will
   * return 1 for version 4.1 and 11 for version 4.11.
   * @return Minor version of the MDF file
   */
  [[nodiscard]] int MinorVersion() const;

  /** \brief Sets the program identifier int the ID block.
   *
   * Sets the program identifier in the MDF file (ID block). Note
   * that only 8 characters are stored.
   *
   * @param program_id Program identifier.
   */
  virtual void ProgramId(const std::string& program_id) = 0;

  /** \brief Returns the program identifier.
   *
   * Returns the program identifier i.e. software who created the file.
   *
   * @return Program identifier (max 8 characters).
   */
  [[nodiscard]] virtual std::string ProgramId() const = 0;

  /** \brief Returns the header object
   *
   * Returns the header object. This object hold general information about
   * the file and its contents. The header is the root of most information
   * in the file.
   * @return Pointer to the header object.
   */
  [[nodiscard]] virtual IHeader* Header() const = 0;

  /** \brief Creates a new attachment (AT block).
   *
   * Creates a new attachment to the file also known as a AT block.
   * An attachemnt is either a reference to an external file or an
   * embedded file block).
   *
   * Note that MDF3 doesn't support attachments.
   * @return Pointer to the new attachment.
   */
  [[nodiscard]] virtual IAttachment* CreateAttachment();

  /** \brief Creates a new measurement (DG block).
   *
   * Creates a new measurement block in the file also known as a DG block.
   * A DG block defines one measurement with one or more sub-measurements (CG
   * block). The new DG block is always put last in the file.
   * @return Pointer to the new measurement.
   */
  [[nodiscard]] virtual IDataGroup* CreateDataGroup() = 0;

  /** \brief Returns true if this is a MDF4 file.
   *
   * Returns true if this is an MDF4 version fo the file.
   * @return True if MDF version 4.
   */
  [[nodiscard]] virtual bool IsMdf4() const = 0;

  /** \brief Reads the information about the file.
   *
   * Reads information about the file i.e. the ID and HD block. It doesn't
   * read in any other information as measurement information.
   * @param file Pointer to an opened file.
   */
  virtual void ReadHeader(std::streambuf& buffer) = 0;

  /** \brief Reads the measurement information about the file.
   *
   * Reads information about the measurements (DG) and sub-measurements (CG) in
   * the file. It doesn't read any information about the channels.
   *
   * There is no need to call the ReadHeader function before this function.
   *
   * @param file Pointer to an opened file.
   */
  virtual void ReadMeasurementInfo(std::streambuf& buffer) = 0;

  /** \brief Reads in all expect raw data from the file.
   *
   * Reads all information about the file but not raw data as sample data or
   * embedded attachment data.
   *
   * There is no need to call the ReadHeader or ReadMeasurement functions before
   * this function.
   *
   * @param file Pointer to an opened file.
   */
  virtual void
  ReadEverythingButData(std::streambuf& buffer) = 0;

  /** \brief Saves all blocks onto the file.
   *
   * Saves all blocks onto the file. You may call this function many times as it
   * keep track of which blocks has been saved or not. You may close and open
   * the file in between calls but this object keeps track of which block has
   * been written or not, so don't delete this object.
   * @param file Pointer to an open file.
   * @return True on success.
   */
  virtual bool Write(std::streambuf& buffer) = 0;

  /** \brief Display name of the file.
   *
   * Default set to the stem of the file. Not stored in the file itself. Only
   * used for display purpose.
   * @return Short name of the file.
   */
  [[nodiscard]] const std::string& Name() const { return name_; }

  /** \brief Set the display name of the file.
   *
   * Sets the display name of the file. By default this is
   * set to the file name without path and extension (stem).
   * @param name Short name of the file.
   */
  void Name(const std::string& name) { name_ = name; }

  /** \brief Returns the full name of the file.
   *
   * Returns the file name with path and extension.
   * @return File name with path and extension.
   */
  [[nodiscard]] const std::string& FileName() const { return filename_; }
  
  /** \brief Sets the file name.
   *
   * Sets the file name and the short name of the object.
   * @param filename File name with path and extension.
   */
  void FileName(const std::string& filename);

  /** \brief Sets the finalize state for the file. */
  virtual void IsFinalized(bool finalized, std::streambuf& buffer,
                           uint16_t standard_flags, uint16_t custom_flags) = 0;
  /** \brief Returns true if the file is finalized. */
  [[nodiscard]] virtual bool IsFinalized(uint16_t& standard_flags,
                                         uint16_t& custom_flags) const = 0;

  /** Returns true if the finalize was done.
   *
   * The function may be used to check on not finalized file to
   * verify that the file have been finalized.
   * @return True if the finalization was successful.
   */
  [[nodiscard]] virtual bool IsFinalizedDone() const;

  /** \brief Returns a parent data group (DG) depending a channel. */
  [[nodiscard]] virtual IDataGroup* FindParentDataGroup(
      const IChannel &channel) const = 0;
 protected:
  MdfFile() = default;  ///< Default constructor
 private:
  std::string name_;      ///< File name without path and extension.
  std::string filename_;  ///< File name with full path.
};

}  // namespace mdf
