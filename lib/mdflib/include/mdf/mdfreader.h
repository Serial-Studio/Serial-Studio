/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdio>
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <fstream>

#include "mdf/ichannelobserver.h"
#include "mdf/mdffile.h"
#include "mdf/isamplereduction.h"

namespace mdf {

class IChannelGroup;

/** \brief Smart pointer to an observer. */
using ChannelObserverPtr = std::unique_ptr<IChannelObserver>;
/** \brief List of observer. */
using ChannelObserverList = std::vector<ChannelObserverPtr>;

/** \brief Returns true if the file is an MDF file.
 *
 * Check if a file is an MDF file. The function reads the first 64
 * bytes (ID block) of the file and test if it is an MDF file.
 * @param filename Full path to the file.
 * @return True if this is a MDF file.
 */
[[nodiscard]] bool IsMdfFile(const std::string& filename);

/** \brief Returns true if the stream buffer is an MDF file.
 *
 * Check if a stream buffer is an MDF file. The function reads the first 64
 * bytes (ID block) of the buffer and test if it is an MDF file.
 * @param buffer Reference to the stream buffer.
 * @return True if this is a MDF stream.
 */
[[nodiscard]] bool IsMdfFile(std::streambuf& buffer );

/// \brief Creates and attaches a channel sample observer.
[[nodiscard]] ChannelObserverPtr CreateChannelObserver(
    const IDataGroup& data_group, const IChannelGroup& group,
    const IChannel& channel);
/**
 * Function that creates a channel observer by searching the channel name. In
 * case that the channel exist in many channel groups, it selects the group with
 * the largest number of samples.
 * @param dg_group The data group with the channel.
 * @param channel_name The channel name
 * @return A smart pointer to a channel observer.
 */
[[nodiscard]] ChannelObserverPtr CreateChannelObserver(
    const IDataGroup& dg_group, const std::string& channel_name);

/** \brief Creates a channel observer. */
void CreateChannelObserverForChannelGroup(const IDataGroup& data_group,
                                          const IChannelGroup& group,
                                          ChannelObserverList& dest);

/** \brief Creates channel observers for all channels within a data group.
 *
 * This function generates channel observers for all channels within the
 * a data group. This function may a little but dangerous as it may allocate
 * a lot of primary memory. An hint is to check how many bytes there are
 * in the DG/DT blocks. If there is more than 1GByte, maybe you should do
 * some partial read instead.
 *
 * Note that channel groups without samples or data, are skipped.
 * @param data_group Reference to the data group.
 * @param dest_list The subscriber list which is appended with channel observers.
 */
void CreateChannelObserverForDataGroup(const IDataGroup& data_group,
                                          ChannelObserverList& dest_list);
/** \class MdfReader mdfreader.h "mdf/mdfreader.h"
 * \brief Reader interface to an MDF file.
 *
 * This is the main interface when reading MDF3 and MDF4 files.
 */
class MdfReader {
 public:
  /** \brief Constructor for readers using an external file.
   *
   * This constructor is used when reading from an external file. The other
   * way of reading from a C++ stream buffer. The latter is more complicated
   * but more generic as it support environment without a file block
   * storage.
   *
   * @param filename Full file path to the input file.
   */
  explicit MdfReader(std::string filename);

  explicit MdfReader(const std::shared_ptr<std::streambuf>& buffer);
  virtual ~MdfReader();  ///< Destructor that close any open file and destructs.

  MdfReader() = delete;
  MdfReader(const MdfReader&) = delete;
  MdfReader(MdfReader&&) = delete;
  MdfReader& operator=(const MdfReader&) = delete;
  MdfReader& operator=(MdfReader&&) = delete;

  /**
   * Unique index for this reader. This index is typically used when fetching
   * files from a database. The index itself is not used by the reader.
   * @return An unique index.
   */
  [[nodiscard]] int64_t Index() const { return index_; }

  /**
   * Sets a unique index to this reader or actually its file. This index is
   * typically retrieved from a database and makes finding files much easier
   * than comparing paths.
   * @param index Unique index.
   */
  void Index(int64_t index) { index_ = index; }

  /// Checks if the file was read without errors.
  /// \return True if the file was read OK.
  [[nodiscard]] bool IsOk() const { return static_cast<bool>(instance_); }

  /** \brief Return true if the file is marked as finalized
   *
   * This function returns true if the file is marked as finalized. This
   * is done by checking the ID block.
   * @return True if the file is marked as finalized.
   */
  [[nodiscard]] bool IsFinalized() const;

  /// Returns a pointer to the MDF file. This file holds references to the MDF
  /// blocks. \return Pointer to the MDF file object. Note it may return a null
  /// pointer.
  [[nodiscard]] const MdfFile* GetFile() const { return instance_.get(); }

  /** \brief Returns the header (HD) block. */
  [[nodiscard]] const IHeader* GetHeader() const;
  /** \brief Returns the data group (DG) block. */
  [[nodiscard]] IDataGroup* GetDataGroup(size_t order) const;

  [[nodiscard]] std::string ShortName()
      const;  ///< Returns the file name without paths.

  /** \brief Opens the internal stream buffer.
   *
   * Normally the stream buffer is an ordinary file but the use may also
   * attach a generic C++ stream
   * @return True if the stream was opened.
   */
  [[nodiscard]] bool Open();
  [[nodiscard]] bool IsOpen() const;
  void Close();  ///< Closes the file stream.

  bool ReadHeader();             ///< Reads the ID and the HD block.
  bool ReadMeasurementInfo();    ///< Reads everything but not CG and raw data.
  bool ReadEverythingButData();  ///< Reads all blocks but not raw data.

  /** \brief Export the attachment data to a destination file.
   *
   * Export an attachment block to a file. If the attachment is an external
   * referenced file, that file is copied to the destination file.
   * If the attachment is embedded in the MDF file, the attachment data is
   * copied to the destination file.
   *
   * Any existing file will be overwritten.
   * @param attachment Reference to the attachment block
   * @param dest_file Full path to the
   * @return True if the export was successful.
   */
  bool ExportAttachmentData(const IAttachment& attachment,
                            const std::string& dest_file);

  /** \brief Export an embedded attachment to a stream buffer.
   *
   * This function export an attachment data to an external stream buffer.
   * Note that this function shouldn't be used with external referenced
   * files.
   *
   * @param attachment Reference to an attachment.
   * @param dest_buffer Refrence to an external stream buffer.
   * @return True if the export was successful.
   */
  bool ExportAttachmentData(const IAttachment& attachment,
                            std::streambuf& dest_buffer);
  /** \brief Reads all sample, sample reduction and signal data into memory.
   *
   * Reads in all data bytes that belongs to a data group (DG). The function
   * reads in sample data (DT..) blocks, sample reduction (RD/RV/RI) blocks
   * and signal data (SD) blocks. Note that this function may consume a lot
   * of memory, so remember to call the IDataGroup::ClearData() function
   * when data not are needed anymore.
   *
   * The attached observers also consumes memory, so remember to delete
   * them when they are no more needed.
   * @param data_group Reference to the data group (DG) object.
   * @return True if the read was successful.
   */
  bool ReadData(IDataGroup& data_group);

  /** \brief Reads a range of samples.
   *
   * Reads in a range of samples a data group (DG). The function
   * reads in sample data (DT..) blocks, sample reduction (RD/RV/RI) blocks
   * and signal data (SD) blocks. The function is faster that reading all data
   * bytes, skipping records it doesn't need to read.
   *
   * Note that this function still may consume a lot
   * of memory, so remember to call the IDataGroup::ClearData() function
   * when data not are needed anymore.
   *
   * The attached observers also consumes memory, so remember to delete
   * them when they are no more needed.
   *
   * @param data_group Reference to the data group (DG) object.
   * @param min_sample First sample index to read.
   * @param max_sample Last sample index to read.
   * @return True if the read was successful.
   */
  bool ReadPartialData(IDataGroup& data_group, size_t min_sample,
                       size_t max_sample);

  /** \brief Reads in data bytes to a sample reduction (SR) block.
   *
   * To minimíze the use of time and memory, this function reads in
   * data for one sample reduction (SR) block. The function is much
   * faster than to read in all data bytes for a data group (DG).
   * @param sr_group Reference to a sample reduction (SR) block.
   * @return True if the read was successful.
   */
  bool ReadSrData(ISampleReduction& sr_group);

  /** \brief Read in partial variable length data with an offset list.
   *
   * This function reads in VLSD stored data according to an offset list.
   * For smaller MDF files that fits in the primary memory, this function is not
   * needed but sometimes the VLSD data sample holds a large amount of data
   * bytes typical a video stream. These files tends to be huge so the
   * application runs out of memory.
   *
   * This function reads in VLSD stored data in smaller batches. The
   * application first reads in all offsets to the raw data. Using these offsets
   * the application can read in typically one sample (offset) at a time. This
   * tactic saves primary memory.
   *
   * @param data_group Data group to read VLSD data from
   * @param vlsd_channel Which channel that stores the VLSD data
   * @param offset_list List of offsets (samples) to read.
   * @param callback Callback function for each offset/sample data
   * @return Returns true if the read was successful
   */
  bool ReadVlsdData(IDataGroup &data_group,
                    IChannel &vlsd_channel,
                    const std::vector<uint64_t>& offset_list,
                    std::function<void(uint64_t,
                                       const std::vector<uint8_t>&)>& callback);


 private:
  /** \brief Internal pointer to the active stream buffer.
   *
   * Shared pointer to the C++ stream buffer. This buffer is either
   * created by this class (file streams) or outside the
   */
  std::shared_ptr<std::streambuf> file_;

  std::string filename_;               ///< The file name with full path.
  std::unique_ptr<MdfFile> instance_;  ///< Pointer to the MDF file object.
  int64_t index_ = 0;  ///< Unique (database) file index that can be used to
                       ///< identify a file instead of its path.

  /** \brief Reads in the
   *
   */
  void VerifyMdfFile();

};

}  // namespace mdf
