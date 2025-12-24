/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "tr3block.h"

namespace {
constexpr size_t kIndexTx = 0;
}  // end namespace

namespace mdf::detail {

std::string Tr3Block::Comment() const { return comment_; }

uint64_t Tr3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 1);
  bytes += ReadNumber(buffer, nof_events_);

  for (int event = 0; event < nof_events_; ++event) {
    if (bytes + 8 > block_size_) {
      break;
    }
    Tr3Event ev;
    bytes += ReadNumber(buffer, ev.event_time);
    bytes += ReadNumber<double>(buffer, ev.pre_time);
    bytes += ReadNumber(buffer, ev.post_time);
    event_list_.emplace_back(ev);
  }
  comment_ = ReadTx3(buffer, kIndexTx);
  return bytes;
}

uint64_t Tr3Block::Write(std::streambuf& buffer) {
  // The TR block is normally added after the measurement has been stopped as it
  // cannot be appended
  if (FilePosition() > 0) {
    return block_size_;
  }

  nof_events_ = static_cast<uint16_t>(event_list_.size());
  block_type_ = "TR";
  block_size_ = (2 + 2) + (1 * 4) + 2 + (nof_events_ * 24);
  link_list_.resize(1, 0);

  if (!comment_.empty()) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(buffer);
    link_list_[kIndexTx] = tx.FilePosition();
  }

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, nof_events_);

  for (const auto &ev : event_list_) {
    bytes += WriteNumber(buffer, ev.event_time);
    bytes += WriteNumber(buffer, ev.pre_time);
    bytes += WriteNumber(buffer, ev.post_time);
  }

  return bytes;
}

}  // namespace mdf::detail
