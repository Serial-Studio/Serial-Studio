/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelgroup.h
 * \brief Defines an interface against a channel group (CG) block.
 */
#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include "mdf/ichannel.h"
#include "mdf/samplerecord.h"
#include "mdf/mdfenumerates.h"
#include "mdf/iblock.h"
#include "mdf/imetadata.h"
#include "mdf/cgcomment.h"
#include "mdf/isourceinformation.h"

namespace mdf {
/** \brief Channel group flags. */
namespace CgFlag {
/** \brief Flag is used to indicate the this block is a variable length CG
 * block.
 *
 * If a CG block is marked as a variable length data (VLSD) block. This
 * technique is used when writing byte arrays with variable length. Bus
 * recorders uses this technique instead of using SD blocks.
 *
 * The problem with
 * SD blocks is that they need to be temporary stored in primary memory. For
 * long time recording this is not acceptable due to the risk of loss of power
 * during th measurement.
 *
 * The draw-back is that the VLSD CG block requires some extra internal
 * functionality both for reading and writing.
 */
constexpr uint16_t VlsdChannel = 0x0001;

/** \brief Bus event flag. */
constexpr uint16_t BusEvent = 0x0002;

/** \brief Plain bus event flag. */
constexpr uint16_t PlainBusEvent = 0x0004;

/** \brief Remote master flag.*/
constexpr uint16_t RemoteMaster = 0x0008;

/** \brief Event signal group. The group store events not values. */
constexpr uint16_t EventSignal = 0x00010;
}  // namespace CgFlag




/** \brief Interface against a channel group (CG) block.
 *
 * A channel group defines a group of signals that are sampled simultaneously.
 * So the number of samples are the same for the channels. Each data sample is
 * stored in so-called record buffer. The record buffer normally have fixed
 * length.
 *
 * The above is somewhat not correct because if the CG block doesn't have any
 * signals, it stores some other data which typical is used when logging bus
 * messages.
 */
class IChannelGroup : public IBlock {

 public:

  virtual void RecordId(uint64_t record_id) = 0; ///< Sets the record identity.
  [[nodiscard]] virtual uint64_t RecordId() const = 0; ///< Record identity.

  virtual void Name(const std::string& name) = 0; ///< Sets the name.
  [[nodiscard]] virtual std::string Name() const = 0; ///< CG name.

  virtual void Description(const std::string& description)
      = 0; ///< Sets a descriptive text.
  [[nodiscard]] virtual std::string Description() const = 0; ///< Description.

  [[nodiscard]] virtual uint64_t NofSamples() const
      = 0; ///< Sets number of samples.
  virtual void NofSamples(uint64_t nof_samples) = 0; ///< Number of samples

  [[nodiscard]] virtual uint16_t Flags() const; ///< Sets CgFlag.
  virtual void Flags(uint16_t flags); ///< Returns CgFlag.

  [[nodiscard]] virtual char16_t PathSeparator(); ///< Sets the path separator.
  virtual void PathSeparator(char16_t path_separator); ///< Path separator.

  /** \brief Returns a list of channels. */
  [[nodiscard]] virtual std::vector<IChannel*> Channels() const = 0;

  /** \brief Creates a new channel. */
  [[nodiscard]] virtual IChannel* CreateChannel() = 0;

  /** \brief Creates a new channel or returns an existing channel. */
  [[nodiscard]] virtual IChannel* CreateChannel(const std::string_view& name);

  /** \brief Returns an existing channels part of name.
   *
   * Note that the function search for a name that includes the search name.
   * Example if the search name is '.DataLength', the signal with the name
   * 'CAN_DataFrame.DataLength' will be returned
   * the name instead of the full name */
  [[nodiscard]] virtual IChannel* GetChannel(const std::string_view& name) const;

    [[nodiscard]] virtual IChannel* GetMasterChannel() const;

  /** \brief Returns an external reference channel. */
  [[nodiscard]] virtual const IChannel* GetXChannel(
      const IChannel& reference) const = 0;

  /** \brief Creates or returns the group source information (SI) block.
   *
   * This function creates or returns the source information (SI) block
   * for the channel group.
   * @return Pointer to a source information (SI) block.
   */
  [[nodiscard]] virtual ISourceInformation* CreateSourceInformation();

  [[nodiscard]] virtual ISourceInformation* SourceInformation()
      const; ///< Returns the source information (SI) block if it exist.

  /** \brief Returns the type of bus messages this channel group contains.
   *
   * Returns what type bus message this channel group contains.
   * The function check this in the source information block belonging
   * to this block.
   * @return The type of bus. Note that it returns 'None' if it isn't a bus
   * channel group.
   */
  [[nodiscard]] BusType GetBusType() const;

  /** \brief Support function that creates a sample record. */
  [[nodiscard]] SampleRecord GetSampleRecord() const;

  /** \brief Resets the internal sample counter. Internal use only. */
  void ResetSampleCounter() const { sample_ = 0;}

  virtual void ClearData(); ///< Resets all temporary stored samples.
  void IncrementSample() const; ///< Add a sample

  [[nodiscard]] size_t Sample() const; ///< Returns number of samples.

  /** \brief Creates a meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData();

  /** \brief Returns the meta-data (MD) block if it exist. */
  [[nodiscard]] virtual IMetaData* MetaData() const;

  /** \brief Returns a pointer to data group (DG) block. */
  [[nodiscard]] virtual const IDataGroup* DataGroup() const = 0;

  void SetCgComment(const CgComment& cg_comment);
  void GetCgComment(CgComment& cg_comment) const;

  void StorageType(MdfStorageType storage_type) {
    storage_type_ = storage_type;
  }
  [[nodiscard]] MdfStorageType StorageType() const {
    return storage_type_;
  }
  void MaxLength(uint32_t max_length) {
    max_length_ = max_length < 8 ? 8 : max_length;
  }

  [[nodiscard]] uint32_t MaxLength() const {
    return max_length_;
  }

 protected:
  /** \brief Temporary record when saving samples.
   *
   * The sample buffer is used internally when reading and writing
   * a channel group record. The record is of fixed size and the
   * buffer is sized upon initializing the read or write.
   */
  mutable std::vector<uint8_t> sample_buffer_;

  /** \brief The storage type is used when creating MDF files.
   *
   * The storage type is mainly used when saving Variable Length Signal Data
   * (VLSD).
   *
   * Normally the channel group uses fixed length storage which means
   * that the byte arrays are stored in a separate signal data block and
   * the groups record buffer stores an index into that block where the
   * byte array is stored.
   *
   * If the block is defined as VLSD storage, instead of storing the VLSD
   * byte array in a separate SD block, a special channel group (CG) VLSD
   * block is used instead.
   *
   * If the block is using the MLSD storage type,instead of storing an 8-byte
   * index, the maximum length of the byte arraya are stored. The actual length
   * is stored in a separate channel. This storage type are typical used for
   * CAN and LIN messages.
   */
  MdfStorageType storage_type_ = MdfStorageType::FixedLengthStorage;

  /** \brief Only used for MLSD storage type. Defines the max number of bytes.
   *
   * This value is used when selecting MLSD storage. When using MLSD storage,
   * this value defines maximum number of data bytes that can be stored.
   *
   * This value is default set to 8 bytes and if set to anything else, do not
   * use MLSD storage.
   */
  uint32_t max_length_ = 8;



 private:
  /** brief Internally used as sample counter.
   *
   * The sample counter is used to indicate how many records of this group
   * that have been read or written.
   */
  mutable size_t sample_ = 0;
};

}  // namespace mdf
