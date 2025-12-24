/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/iattachment.h"
#include "mdf/ievent.h"
#include "mdfblock.h"

namespace mdf::detail {
class Hd4Block;

class Ev4Block : public MdfBlock, public IEvent {
 public:
  Ev4Block();

  void GetBlockProperty(BlockPropertyList &dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;
  void FindReferencedBlocks(const Hd4Block &hd4);

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string &name) override;
  [[nodiscard]] const std::string &Name() const override;

  void GroupName(const std::string &group_name) override;
  [[nodiscard]] const std::string &GroupName() const override;

  void Type(EventType event_type) override;
  [[nodiscard]] EventType Type() const override;

  void Sync(SyncType sync_type) override;
  [[nodiscard]] SyncType Sync() const override;

  void Range(RangeType range_type) override;
  [[nodiscard]] RangeType Range() const override;

  void Cause(EventCause cause) override;
  [[nodiscard]] EventCause Cause() const override;

  void CreatorIndex(size_t index) override;
  [[nodiscard]] size_t CreatorIndex() const override;

  void SyncValue(int64_t value) override;
  [[nodiscard]] int64_t SyncValue() const override;

  void SyncFactor(double factor) override;
  [[nodiscard]] double SyncFactor() const override;

  void ParentEvent(const IEvent *parent) override;
  [[nodiscard]] const IEvent *ParentEvent() const override;

  void RangeEvent(const IEvent *range_event) override;
  [[nodiscard]] const IEvent *RangeEvent() const override;

  void AddScope(const void *scope) override;
  [[nodiscard]] const std::vector<const void *> &Scopes() const override;

  void AddAttachment(const IAttachment *attachment) override;
  [[nodiscard]] const std::vector<const IAttachment *> &Attachments()
      const override;
  IMetaData *CreateMetaData() override;
  [[nodiscard]] IMetaData *MetaData() const override;

 private:
  uint8_t type_ = static_cast<uint8_t>(EventType::Marker);
  uint8_t sync_type_ = static_cast<uint8_t>(SyncType::SyncTime);
  uint8_t range_type_ = 0;
  uint8_t cause_ = 0;
  uint8_t flags_ = 0;
  /* 1 byte reserved */
  uint32_t length_m_ = 0;  ///< Number of scope events
  uint16_t length_n_ = 0;  ///< Number of attachments
  uint16_t creator_index_ = 0;
  int64_t sync_base_value_ = 0;
  double sync_factor_ = 1.0;

  const IEvent *parent_event_ = nullptr;
  const IEvent *range_event_ = nullptr;
  std::vector<const void *> scope_list_;  ///< List of MdfBlock pointers
  std::vector<const IAttachment *> attachment_list_;

  std::string name_;
  std::string group_name_;
};
}  // namespace mdf::detail
