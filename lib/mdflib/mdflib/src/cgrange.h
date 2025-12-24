/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
/** \file
 * This file define the channel group range support class.
 */

#include "mdf/ichannelgroup.h"

namespace mdf {

class CgRange final {
 public:
  CgRange() = delete;
  explicit CgRange(const IChannelGroup& channel_group);
  [[nodiscard]] uint64_t RecordId() const;

  void IsUsed(bool used) { is_used_ = used;}
  [[nodiscard]] bool IsUsed() const { return is_used_;}

  [[nodiscard]] const IChannelGroup& ChannelGroup() const {
    return channel_group_;
  }

 private:
  const IChannelGroup& channel_group_;
  bool is_used_ = false;
};

}  // namespace mdf


