/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "hl4block.h"
#include "mdf/mdfwriter.h"
#include "writecache.h"

namespace mdf::detail {
class Dg4Block;
class Dt4Block;
class Mdf4Writer : public MdfWriter {
 public:
  Mdf4Writer();
  ~Mdf4Writer() override;


  IChannelConversion* CreateChannelConversion(IChannel* parent) override;

 protected:
  WriteCache write_cache_;

  void SaveSample(const IChannelGroup& group, uint64_t time) override;
  void SaveSample(const IDataGroup& data_group,
                  const IChannelGroup& channel_group, uint64_t time) override;

  void SaveCanMessage(const IChannelGroup& group, uint64_t time,
                      const CanMessage& msg) override;
  void SaveCanMessage(const IDataGroup& data_group,
                      const IChannelGroup& channel_group, uint64_t time,
                      const CanMessage& msg) override;

  void SaveLinMessage(const IChannelGroup& group, uint64_t time,
                      const LinMessage& msg) override;
  void SaveLinMessage(const IDataGroup& data_group,
                      const IChannelGroup& channel_group, uint64_t time,
                      const LinMessage& msg) override;

  void SaveEthMessage(const IChannelGroup& group, uint64_t time,
                      const EthMessage& msg) override;
  void SaveEthMessage(const IDataGroup& data_group,
                      const IChannelGroup& channel_group, uint64_t time,
                      const EthMessage& msg) override;

  void SaveMostMessage(const IDataGroup& data_group,
                      const IChannelGroup& channel_group, uint64_t time,
                      const IMostEvent& msg) override;

  void SaveFlexRayMessage(const IDataGroup& data_group,
                       const IChannelGroup& channel_group, uint64_t time,
                       const IFlexRayEvent& msg) override;
  void CreateMdfFile() override;

  bool PrepareForWriting() override;

  bool WriteSignalData(std::streambuf& buffer) override;

  void InitWriteCache() override;
  void ExitWriteCache() override;
  void RecalculateTimeMaster() override;
  void NotifySample() override;

  Dg4Block* GetLastDg4();
 private:


};

}  // namespace mdf::detail
