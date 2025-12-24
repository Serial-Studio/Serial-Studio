/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "mdf/mdfwriter.h"
#include "writecache.h"
namespace mdf::detail {

class Mdf3Writer : public MdfWriter {
 public:
  Mdf3Writer();  ///< Constructor that creates the ID and HD block.
  ~Mdf3Writer() override;   ///< Destructor that close any open file and
                           ///< destructs.

  IChannelConversion* CreateChannelConversion(IChannel* parent) override;
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
                      const IChannelGroup& group, uint64_t time,
                      const EthMessage& msg) override;

  void SaveMostMessage(const IDataGroup& data_group,
                      const IChannelGroup& group, uint64_t time,
                      const IMostEvent& msg) override;
  void SaveFlexRayMessage(const IDataGroup& data_group,
                       const IChannelGroup& group, uint64_t time,
                       const IFlexRayEvent& msg) override;
 protected:

  void CreateMdfFile() override;
  bool PrepareForWriting() override;


  void InitWriteCache() override;
  void ExitWriteCache() override;
  void RecalculateTimeMaster() override;
  void NotifySample() override;

 private:
  WriteCache write_cache_;
};

}  // namespace mdf::detail
