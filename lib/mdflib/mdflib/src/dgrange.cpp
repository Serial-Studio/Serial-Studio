/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

/** \file
 * This file define the DG range support class.
 */

#include "dgrange.h"
#include "mdf/ichannelgroup.h"

namespace mdf {
DgRange::DgRange(const IDataGroup& data_group, size_t min_sample,
                 size_t max_sample)
    : data_group_(data_group),
     min_sample_(min_sample),
     max_sample_(max_sample) {
  auto cg_list = data_group_.ChannelGroups();
  for (const IChannelGroup* channel_group : cg_list) {
    if (channel_group == nullptr) {
      continue;
    }
    const uint64_t record_id = channel_group->RecordId();
    bool used = data_group_.IsSubscribingOnRecord(record_id);

    // Check if the min_sample > number of samples, then it is no meaning
    // to read this CG group. If the group is a VSLD group, it doesn't have
    // any samples.
    if ( (channel_group->Flags() & CgFlag::VlsdChannel) == 0 &&
         min_sample_ > channel_group->NofSamples() ) {
      used = false;
    }

    CgRange cg_range(*channel_group);
    cg_range.IsUsed(used);
    cg_list_.emplace(record_id, cg_range);

  }

}

bool DgRange::IsUsed(uint64_t record_id) const {
  const auto itr = cg_list_.find(record_id);
  return !(itr == cg_list_.cend()) && itr->second.IsUsed();
}

CgRange* DgRange::GetCgRange(uint64_t record_id) {
  auto itr = cg_list_.find(record_id);
  return itr == cg_list_.cend() ? nullptr : &itr->second;
}

bool DgRange::IsReady() const {
  return std::all_of(cg_list_.begin(), cg_list_.cend(),
                     [&] (const auto& itr) -> bool {
    return !itr.second.IsUsed();
  });
}


}
// namespace mdf