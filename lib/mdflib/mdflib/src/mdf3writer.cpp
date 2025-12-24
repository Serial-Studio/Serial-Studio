/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf3writer.h"

#include <chrono>
#include <cstdio>

#include "cc3block.h"
#include "cg3block.h"
#include "cn3block.h"
#include "dg3block.h"
#include "mdf/mdflogstream.h"
#include "mdf3file.h"
#include "platform.h"

using namespace std::chrono_literals;

namespace mdf::detail {

Mdf3Writer::Mdf3Writer()
: write_cache_(*this) {
  type_of_writer_ = MdfWriterType::Mdf3Basic;
}

Mdf3Writer::~Mdf3Writer() {
  write_cache_.Exit();
}

IChannelConversion *Mdf3Writer::CreateChannelConversion(IChannel *parent) {
  return parent != nullptr ? parent->CreateChannelConversion() : nullptr;
}

void Mdf3Writer::CreateMdfFile() {
  auto mdf3 = std::make_unique<Mdf3File>();
  mdf_file_ = std::move(mdf3);
}


bool Mdf3Writer::PrepareForWriting() {
  auto *header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No header  found. Invalid use of the function.";
    return false;
  }
  for (IDataGroup* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (IChannelGroup *channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||  channel_group->NofSamples() > 0) {
        continue;
      }
      if (auto* cg3 = dynamic_cast<Cg3Block*>(channel_group);
          cg3 != nullptr) {
        if (const bool prep = cg3->PrepareForWriting(); !prep ) {
          MDF_ERROR() << "Faled to prepare for writing.";
          return false;
        }
      }
    }
  }
  return true;
}

void Mdf3Writer::InitWriteCache() {
  write_cache_.Init();
}

void Mdf3Writer::ExitWriteCache() {
  write_cache_.Exit();
}

void Mdf3Writer::RecalculateTimeMaster() {
  write_cache_.RecalculateTimeMaster();
}

void Mdf3Writer::NotifySample() {
  write_cache_.Notify();
}

void Mdf3Writer::SaveSample(const IChannelGroup &group, uint64_t time) {
  write_cache_.SaveSample(group, time);
}

void Mdf3Writer::SaveSample(const IDataGroup& data_group,
                            const IChannelGroup &channel_group, uint64_t time) {
  write_cache_.SaveSample(data_group, channel_group, time);
}

void Mdf3Writer::SaveCanMessage(const IChannelGroup &group, uint64_t time,
                                const CanMessage &msg) {
  write_cache_.SaveCanMessage(group, time, msg);
}

void Mdf3Writer::SaveCanMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group, uint64_t time,
                                const CanMessage &msg) {
  write_cache_.SaveCanMessage(data_group, channel_group, time, msg);
}

void Mdf3Writer::SaveLinMessage(const IChannelGroup &group, uint64_t time,
                                const LinMessage &msg) {
  write_cache_.SaveLinMessage(group, time, msg);
}

void Mdf3Writer::SaveLinMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group, uint64_t time,
                                const LinMessage &msg) {
  write_cache_.SaveLinMessage(data_group, channel_group, time, msg);
}

void Mdf3Writer::SaveEthMessage(const IChannelGroup &group, uint64_t time,
                                const EthMessage &msg) {
  write_cache_.SaveEthMessage(group,time, msg);
}
void Mdf3Writer::SaveEthMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group, uint64_t time,
                                const EthMessage &msg) {
  write_cache_.SaveEthMessage(data_group, channel_group,time, msg);
}

void Mdf3Writer::SaveMostMessage(const IDataGroup& data_group,
                                const IChannelGroup &channel_group, uint64_t time,
                                const IMostEvent &msg) {
  write_cache_.SaveMostMessage(data_group, channel_group,time, msg);
}

void Mdf3Writer::SaveFlexRayMessage(const IDataGroup& data_group,
                                 const IChannelGroup &channel_group, uint64_t time,
                                 const IFlexRayEvent &msg) {
  write_cache_.SaveFlexRayMessage(data_group, channel_group,time, msg);
}
}  // namespace mdf::detail