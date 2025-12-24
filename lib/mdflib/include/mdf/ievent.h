/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ievent.h
 * \brief Interface against an event block (EV)
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mdf/iattachment.h"
#include "mdf/iblock.h"
#include "mdf/evcomment.h"


namespace mdf {
/** \brief Type of event. */
enum class EventType : uint8_t {
  RecordingPeriod = 0,       ///< Specifies a recording period (range).
  RecordingInterrupt = 1,    ///< The recording was interrupted.
  AcquisitionInterrupt = 2,  ///< The data acquisition was interrupted.
  StartRecording = 3,        ///< Start recording event.
  StopRecording = 4,         ///< Stop recording event.
  Trigger = 5,               ///< Generic event (no range).
  Marker = 6                 ///< Another generic event (maybe range).
};

/** \brief Type of synchronization value (default time) */
enum class SyncType : uint8_t {
  SyncTime = 1,      ///< Sync value represent time (s).
  SyncAngle = 2,     ///< Sync value represent angle (rad).
  SyncDistance = 3,  ///< Sync value represent distance (m).
  SyncIndex = 4,     ///< Sync value represent sample index.
};

/** \brief Type of range. */
enum class RangeType : uint8_t {
  RangePoint = 0,  ///< Defines a point
  RangeStart = 1,  ///< First in a range.
  RangeEnd = 2     ///< Last in a range.
};

/** \brief Type of cause. */
enum class EventCause : uint8_t {
  CauseOther = 0,   ///< Unknown source.
  CauseError = 1,   ///< An error generated this event.
  CauseTool = 2,    ///< The tool generated this event.
  CauseScript = 3,  ///< A script generated this event.
  CauseUser = 4,    ///< A user generated this event.
};

/** \brief Interface against an event block.
 *
 * The event (EV) block is used to store events text and data. It is rather
 * flexible block type which makes it difficult to simplify.
 */
class IEvent : public IBlock {
 public:
  virtual void Name(const std::string& name) = 0; ///< Sets the name.
  [[nodiscard]] virtual const std::string& Name() const = 0; ///< Name.

  /** \brief Sets the group name. */
  virtual void GroupName(const std::string& group_name) = 0;
  /** \brief Returns the group name. */
  [[nodiscard]] virtual const std::string& GroupName() const = 0;


  virtual void Type(EventType event_type) = 0; ///< Sets type of event.
  [[nodiscard]] virtual EventType Type() const = 0; ///< Type of event.
  [[nodiscard]] std::string TypeToString() const; ///< Typ as string.

  virtual void Sync(SyncType sync_type) = 0; ///< Sets type of sync.
  [[nodiscard]] virtual SyncType Sync() const = 0; ///< Type of sync.

  virtual void Range(RangeType range_type) = 0; ///< Sets type of range.
  [[nodiscard]] virtual RangeType Range() const = 0; ///< Type of range.
  [[nodiscard]] std::string RangeToString() const; ///< Range to string.

  virtual void Cause(EventCause cause) = 0; ///< Sets the cause.
  [[nodiscard]] virtual EventCause Cause() const = 0; ///< Cause of event.
  [[nodiscard]] std::string CauseToString() const; ///< Cause to string.

  virtual void CreatorIndex(size_t index) = 0; ///< Sets the creator index.
  [[nodiscard]] virtual size_t CreatorIndex() const = 0; ///< Creator index.

  virtual void SyncValue(int64_t value) = 0; ///< Sets the sync value.
  [[nodiscard]] virtual int64_t SyncValue() const = 0; ///< Sync value.

  virtual void SyncFactor(double factor) = 0; ///< Sets the factor.
  [[nodiscard]] virtual double SyncFactor() const = 0; ///< Sync factor.
  [[nodiscard]] std::string ValueToString() const; ///< Sync value as string.

  /** \brief Sets the parent event. */
  virtual void ParentEvent(const IEvent* parent) = 0;
  /** \brief Returns the parent event. */
  [[nodiscard]] virtual const IEvent* ParentEvent() const = 0;

  /** \brief Sets the range. */
  virtual void RangeEvent(const IEvent* range_event) = 0;
  /** \brief Returns the range. */
  [[nodiscard]] virtual const IEvent* RangeEvent() const = 0;

  /** \brief Adds a scope reference. */
  virtual void AddScope(const void* scope) = 0;
  /** \brief Returns referenced CN and CG blocks. */
  [[nodiscard]] virtual const std::vector<const void*>& Scopes() const = 0;

  /** \brief Adds an attachment reference. */
  virtual void AddAttachment(const IAttachment* attachment) = 0;
  /** \brief Returns a list of attachment references. */
  [[nodiscard]] virtual const std::vector<const IAttachment*>& Attachments()
      const = 0;

  /** \brief Returns an interface against an MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual IMetaData* CreateMetaData() = 0;

  /** \brief Returns an constant interface against a MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  void Description(const std::string& description); ///< Sets description.
  [[nodiscard]] std::string Description() const; ///< Returns description.

  void PreTrig(double pre_trig); ///< Sets the pre-trig time (s).

  [[nodiscard]] double PreTrig() const; ///< Returns the pre-trig value (s).

  void PostTrig(double post_trig); ///< Sets the post-trig value (s)

  [[nodiscard]] double PostTrig() const; ///< Returns the post-trig value (s).

  void SetEvComment(const EvComment& ev_comment);
  void GetEvComment(EvComment& ev_comment) const;
};

}  // namespace mdf
