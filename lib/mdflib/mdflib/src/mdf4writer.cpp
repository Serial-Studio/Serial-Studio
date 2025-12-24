/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf4writer.h"
#include <ctime>
#include <fstream>
#include "mdf/mdflogstream.h"
#include "mdf4file.h"
#include "platform.h"
#include "dt4block.h"
#include "hl4block.h"
#include "dg4block.h"
#include "cn4block.h"
#include "sr4block.h"


namespace mdf::detail {
Mdf4Writer::Mdf4Writer()
: write_cache_(*this) {
  type_of_writer_ = MdfWriterType::Mdf4Basic;
}

Mdf4Writer::~Mdf4Writer() {
  write_cache_.Exit();
}

IChannelConversion* Mdf4Writer::CreateChannelConversion(IChannel* parent) {
  auto* cn4 = dynamic_cast<Cn4Block*>(parent);
  IChannelConversion* cc = nullptr;
  if (cn4 != nullptr) {
    auto cc4 = std::make_unique<Cc4Block>();
    cc4->Init(*cn4);
    cn4->AddCc4(cc4);
    cc = const_cast<Cc4Block*>(cn4->Cc());
  }
  return cc;
}

void Mdf4Writer::CreateMdfFile() {
  auto mdf4 = std::make_unique<Mdf4File>();
  mdf_file_ = std::move(mdf4);
}

bool Mdf4Writer::PrepareForWriting() {

  auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header  found. Invalid use of the function.";
    return false;
  }

  // Make a list of active DG blocks
  std::vector<IDataGroup*> active_list;
  for (IDataGroup* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (IChannelGroup *channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||  channel_group->NofSamples() > 0 ||
          channel_group->NofSamples() > 0 ) {
        continue;
      }
      active_list.push_back(data_group);
      break;
    }
  }
  // Check if compression is required
  if (active_list.size() > 1 && IsSavePeriodic() && !CompressData()) {
    CompressData(true);
  }
  // Only the last DG block is updated. So go to the last
  // DG block and add an uncompressed DT block to the DG block
  // or an HL/DL/DZ thing for compressed data.
  for (IDataGroup* data_group : active_list) {
    auto* dg4 = dynamic_cast<Dg4Block*>(data_group);
    if (dg4 == nullptr) {
      MDF_ERROR() << "Invalid DG block type detected.";
      return false;
    }
    /*
    if (CompressData()) {
      auto hl4 = std::make_unique<Hl4Block>();
      hl4->Init(*dg4);
    } else {
      // The data block list should include a DT block that we later will append
      // samples (buffers) to.
      auto& block_list = dg4->DataBlockList();
      auto dt4 = std::make_unique<Dt4Block>();
      dt4->Init(*dg4);
      block_list.push_back(std::move(dt4));
    }
    */
    // Size the sample buffers for each CG block
    for (auto& cg4 : dg4->Cg4()) {
      if (!cg4) {
        continue;
      }
      // I need to update the CN to CG reference
      cg4->PrepareForWriting();
    }
  }
  return true;
}


bool Mdf4Writer::WriteSignalData(std::streambuf& buffer) {

  const auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header block found. File: " << Name();
    return false;
  }

  // Only the last DG block is updated. So go to the last DT
  const auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return true;
  }

  auto cg_list = last_dg->ChannelGroups();
  for (auto* group : cg_list) {
    if (group == nullptr) {
      continue;
    }
    auto cn_list = group->Channels();
    for (auto* channel : cn_list) {
      if (channel == nullptr) {
        continue;
      }
      auto* cn4 = dynamic_cast<Cn4Block*>(channel);
      if (cn4 == nullptr) {
        continue;
      }
      cn4->WriteSignalData(buffer, CompressData());
      cn4->ClearData();
    }
  }
  return true;
}

Dg4Block* Mdf4Writer::GetLastDg4() {
  auto *header = Header();
  if (header == nullptr) {
    return nullptr;
  }

  // Only the last DT block is updated. So go to the last DT
  auto *last_dg = header->LastDataGroup();
  if (last_dg == nullptr) {
    return nullptr;
  }
  return dynamic_cast<Dg4Block*>(last_dg);
}



void Mdf4Writer::SaveSample(const IChannelGroup &group, uint64_t time) {
  write_cache_.SaveSample(group, time);
}

void Mdf4Writer::SaveSample(const IDataGroup& data_group,
                            const IChannelGroup &channel_group, uint64_t time) {
  write_cache_.SaveSample(data_group, channel_group, time);
}

void Mdf4Writer::SaveCanMessage(const IChannelGroup &group, uint64_t time,
                                const CanMessage &msg) {
  write_cache_.SaveCanMessage(group, time, msg);
}

void Mdf4Writer::SaveCanMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group,
                                uint64_t time,
                                const CanMessage &msg) {
  write_cache_.SaveCanMessage(data_group, channel_group, time, msg);
}

void Mdf4Writer::SaveLinMessage(const IChannelGroup &group, uint64_t time,
                                const LinMessage &msg) {
  write_cache_.SaveLinMessage(group, time, msg);
}

void Mdf4Writer::SaveLinMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group, uint64_t time,
                                const LinMessage &msg) {
  write_cache_.SaveLinMessage(data_group, channel_group, time, msg);
}

void Mdf4Writer::SaveEthMessage(const IChannelGroup &group, uint64_t time,
                                const EthMessage &msg) {
  write_cache_.SaveEthMessage(group,time, msg);
}

void Mdf4Writer::SaveEthMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group,
                                uint64_t time,
                                const EthMessage &msg) {
  write_cache_.SaveEthMessage(data_group, channel_group,time, msg);
}

void Mdf4Writer::SaveMostMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group,
                                uint64_t time,
                                const IMostEvent &msg) {
  write_cache_.SaveMostMessage(data_group, channel_group,time, msg);
}

void Mdf4Writer::SaveFlexRayMessage(const IDataGroup& data_group,
                                 const IChannelGroup &channel_group,
                                 uint64_t time,
                                 const IFlexRayEvent &msg) {
  write_cache_.SaveFlexRayMessage(data_group, channel_group,time, msg);
}

void Mdf4Writer::InitWriteCache() {
  write_cache_.Init();
}

void Mdf4Writer::ExitWriteCache() {
  write_cache_.Exit();
}

void Mdf4Writer::RecalculateTimeMaster() {
  write_cache_.RecalculateTimeMaster();
}

void Mdf4Writer::NotifySample() {
  write_cache_.Notify();
}

}  // namespace mdf::detail