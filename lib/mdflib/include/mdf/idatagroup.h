/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file idatagroup.h
 * \brief Interface to a data group (DG) block.
 *
 */
#pragma once
#include <string>
#include <vector>
#include <map>

#include "mdf/iblock.h"
#include "mdf/dgcomment.h"

namespace mdf {

class IMetaData;
class ISampleObserver;
class IChannelGroup;
class IChannel;
/** \brief Interface to a data group (DG) block.
 *
 * The data group block is the entry point for a measurement. The header (HD)
 * block is the entry point for a test. A test consist of one or more
 * measurements (DG).
 *
 * Each measurement is defined by one or more channel groups (CG). The channel
 * group in turn, consist of one or more channels. The group have an unique
 * record ID.
 *
 * A record is a byte array with fixed length and hold one sample for each
 * channel in the group. The DG block also points to the data blocks (DT) which
 * is an array of sample records.
 *
 * In MDF version 3, data was always stored as raw bytes in a DT block. In
 * version 4 the DT block may be split, reorganized and compressed.
 *
 */
class IDataGroup : public IBlock {
 public:

  /** \brief Sets the descriptive text for the measurement. */
  virtual void Description(const std::string& desc);
  /** \brief Return the descriptive text. */
  [[nodiscard]] virtual std::string Description() const;

  /** \brief Sets size of the record ID (bytes). Note that record
   * ID and its size, is automatically set when writing MDF files. */
  virtual void RecordIdSize(uint8_t id_size);
  /** \brief Returns the record ID size in bytes. */
  [[nodiscard]] virtual uint8_t RecordIdSize() const;

  /** \brief Returns a list of channel groups. */
  [[nodiscard]] virtual std::vector<IChannelGroup*> ChannelGroups() const = 0;

  /** \brief Create a new empty channel group. */
  [[nodiscard]] virtual IChannelGroup* CreateChannelGroup() = 0;

  /** \brief Create a new channel group or return the existing group. */
  [[nodiscard]] IChannelGroup* CreateChannelGroup(
      const std::string_view& name);

  /** \brief Returns a channel group by its full name or sub-string.
   *
   * The function return a channel group by its full name. If the full name
   * search fails, it search on a sub-string instead. The sub-string must
   * be larger than 3 characters.
   * @param name Full name of the group or a sub-string
   * @return Pointer to the matching channel group or nullptr at no match.
   */
  [[nodiscard]] IChannelGroup* GetChannelGroup(const std::string_view& name) const;

  /** \brief Return a channel group by its record id. */
  [[nodiscard]] IChannelGroup* GetChannelGroup(uint64_t record_id) const;

  /** \brief Create or return the existing meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();
  /** \brief Returns the existing meta-data (MD) block if it exist. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  /** \brief Internal function that attach a sample observer to the
   *  measurement block.  */
  void AttachSampleObserver(ISampleObserver* observer) const;
  /** \brief Detach an observer from  the measurement. */
  void DetachSampleObserver(const ISampleObserver* observer) const;
  /** \brief Detaches all observers from the measurement. */
  void DetachAllSampleObservers() const;
  /** \brief Notifies the observer that a new sample record have been read.*/
  bool NotifySampleObservers(uint64_t sample, uint64_t record_id,
                             const std::vector<uint8_t>& record) const;

  /** \brief Clear all temporary sample and data buffers.
   *
   * Clear all sample and signal data buffers. Call this function
   * when no need of any signal data (SD) or sample reduction (SR) data
   * are needed. This reduce memory usage if more DG blocks should
   * read in data bytes.
   */
  virtual void ClearData();

  /** \brief Set the DG blocks data as read. */
  void SetAsRead(bool mark_as_read = true) const {
    mark_as_read_ = mark_as_read;
  }

  /** \brief Returns true if no samples has been stored yet. */
  [[nodiscard]] bool IsEmpty() const;

  /** \brief Return true if the DG blocks data has been read not the DG block
   * itself.  */
  [[nodiscard]] bool IsRead() const { return mark_as_read_; }

  /** \brief Support function that return the first CG block that contains
   * a specific CN block.
   */
  [[nodiscard]] virtual IChannelGroup *FindParentChannelGroup(
      const IChannel &channel) const = 0;

  /**
   * \brief Checks if this data group subscribes on a specific record.
   * @param record_id Record ID of the channel group
   * @return True if the observer list subscribe on this channel group.
   */
  [[nodiscard]] bool IsSubscribingOnRecord(uint64_t record_id) const;

  /**
 * \brief Checks if this data group subscribes on a specific channel.
 * @param channel Reference to the channel.
 * @return True if the observer list subscribe on this channel.
 */
  [[nodiscard]] bool IsSubscribingOnChannel(const IChannel& channel) const;

  /**
 * \brief Checks if this data group subscribes on a specific channels VLSD raw data.
 * @param channel Reference to the channel.
 * @return True if the observer list subscribe on this channels VLSD raw data  .
 */
  [[nodiscard]] bool IsSubscribingOnChannelVlsd(const IChannel& channel) const;

  void SetDgComment(const DgComment& dg_comment);
  void GetDgComment(DgComment& dg_comment) const;

  void MandatoryMembersOnly(bool mandatory_only) const {
    mandatory_members_only_ = mandatory_only;
  }

  [[nodiscard]] bool MandatoryMembersOnly() const {
    return mandatory_members_only_;
  }
 protected:
  mutable std::vector<ISampleObserver*> observer_list_; ///< List of observers.

  /** \brief The fast observer list is used internally when reading data.
   *
   */
  std::map<uint64_t, std::vector<ISampleObserver*> > fast_observer_list_;

  ~IDataGroup() override = default; ///< Default destructor

  /** \brief The function optimize the observer list before reading data.
   *
   * The function is called before reading any data in the file. The purpose is
   * optimize the read speed of files with many sample. The function is used
   * internally only.
   */
  void InitFastObserverList();

 private:
  mutable bool mark_as_read_ = false; ///< True if the data block has been read.
  mutable bool mandatory_members_only_ = false;


};

}  // namespace mdf
