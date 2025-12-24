/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "testmostlogger.h"
#include <filesystem>
#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/mostmessage.h"
#include "mdf/mdflogstream.h"
#include "mdf/ifilehistory.h"
#include "mdf/mostconfigadapter.h"
#include "mdf/mdfreader.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"

using namespace util::log;
using namespace std::filesystem;
using namespace util::time;

namespace {

std::string kTestRootDir; ///< Test root directory (%TEMP%/test)
std::string kTestDir; ///<  'Test Root Dir'/mdf/most";
bool kSkipTest = false; ///< Set to true if the output dir is missing

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& location , mdf::MdfLogSeverity severity,
             const std::string& text) {
  const auto &log_config = LogConfig::Instance();
  LogMessage message;
  message.message = text;
  message.severity = static_cast<LogSeverity>(severity);

  log_config.AddLogMessage(message);
}
/**
 * Function that stops logging of MDF logs
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void NoLog(const MdfLocation& location , mdf::MdfLogSeverity severity ,
           const std::string& text) {
}

}

namespace mdf::test {

void TestMostLogger::SetUpTestSuite() {

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Note never deleted

    // Log to the console instead of as normally to a file
    auto& log_config = LogConfig::Instance();
    log_config.Type(LogType::LogToConsole);
    log_config.CreateDefaultLogger();

    // Connect MDF library to use the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("most");

    // Remove older files here instead as normally in the tear down function.
    // This enables the end user to manually analyzing the generated files.
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_ERROR() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    LOG_INFO() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMostLogger::TearDownTestSuite() {
  LOG_INFO() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestMostLogger, TestMostEthernetPacket) {
  MostEthernetPacket msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::EthernetPacket);

  msg.BusChannel(123);
  EXPECT_EQ(msg.BusChannel(), 123);

  msg.Speed(MostSpeed::Most150);
  EXPECT_EQ(msg.Speed(), MostSpeed::Most150);

  msg.TransferType(MostTransferType::Node);
  EXPECT_EQ(msg.TransferType(), MostTransferType::Node);

  msg.Direction(MostDirection::Tx);
  EXPECT_EQ(msg.Direction(), MostDirection::Tx);

  msg.Source(0x1234);
  EXPECT_EQ(msg.Source(), 0x1234);

  msg.Destination(0x3456);
  EXPECT_EQ(msg.Destination(), 0x3456);

  msg.SpecifiedNofBytes(45);
  EXPECT_EQ(msg.SpecifiedNofBytes(), 45);

  msg.ReceivedNofBytes(34);
  EXPECT_EQ(msg.ReceivedNofBytes(), 34);

  std::vector<uint8_t> buffer(10, 33);
  msg.DataBytes(buffer);
  EXPECT_EQ(msg.DataBytes(), buffer);

  msg.Crc(0x2345);
  EXPECT_EQ(msg.Crc(), 0x2345);

  msg.CompleteAck(MostCompleteAck::CrcError);
  EXPECT_EQ(msg.CompleteAck(), MostCompleteAck::CrcError);

  msg.PreemptiveAck(MostPreemptiveAck::BufferFull);
  EXPECT_EQ(msg.PreemptiveAck(), MostPreemptiveAck::BufferFull);

  msg.TxAck(2);
  EXPECT_EQ(msg.TxAck(), 2);
}

TEST_F(TestMostLogger, TestMostPacket) {
  MostPacket msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::Packet);

  msg.PacketIndex(0x55);
  EXPECT_EQ(msg.PacketIndex(), 0x55);

}

TEST_F(TestMostLogger, TestMostMessage) {
  MostMessage msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::Message);

  msg.TxFailed(true);
  EXPECT_EQ(msg.TxFailed(), true);

  msg.ControlMessageType(MostControlMessageType::RemoteWrite);
  EXPECT_EQ(msg.ControlMessageType(), MostControlMessageType::RemoteWrite);
}

TEST_F(TestMostLogger, TestSignalState) {
  MostSignalState msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::SignalState);

  msg.SignalState(MostStateOfSignal::StableLock);
  EXPECT_EQ(msg.SignalState(), MostStateOfSignal::StableLock);
}

TEST_F(TestMostLogger, TestMaxPosInfo) {
  MostMaxPosInfo msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::MaxPosInfo);

  msg.DeviceCount(34);
  EXPECT_EQ(msg.DeviceCount(), 34);
}

TEST_F(TestMostLogger, TestBoundDesc) {
  MostBoundDesc msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::BoundDesc);

  msg.BoundaryCount(45);
  EXPECT_EQ(msg.BoundaryCount(), 45);
}

TEST_F(TestMostLogger, TestAllocTable) {
  MostAllocTable msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::AllocTable);

  msg.FreeByteCount(56);
  EXPECT_EQ(msg.FreeByteCount(), 56);

  msg.TableLayout(MostTableLayout::LabelList);
  EXPECT_EQ(msg.TableLayout(), MostTableLayout::LabelList);

  const std::vector<uint8_t> buffer(123, 65);
  msg.TableData(buffer);
  EXPECT_EQ(msg.TableData(), buffer);
}

TEST_F(TestMostLogger, TestSysLockState) {
  MostSysLockState msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::SysLockState);

  msg.RingIsClosed(true);
  EXPECT_EQ(msg.RingIsClosed(), true);
}

TEST_F(TestMostLogger, TestShutdownFlag) {
  MostShutdownFlag msg;
  EXPECT_EQ(msg.MessageType(), MostMessageType::ShutdownFlag);

  msg.ShutdownFlag(true);
  EXPECT_EQ(msg.ShutdownFlag(), true);
}

TEST_F(TestMostLogger, MostVlsdStorage) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000;
  path mdf_file(kTestDir);
  mdf_file.append("most_vlsd_storage.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
    ASSERT_TRUE(writer);
    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    HdComment hd_comment("Test of MOST VLSD Storage");
    hd_comment.Author("Ingemar Hedvall");
    hd_comment.Department("IH Development");
    hd_comment.Project("MOST bus support");
    hd_comment.Subject("Testing MOST support");
    hd_comment.TimeSource(MdString("PC UTC Time"));
    header->SetHdComment(hd_comment);

    auto* history = header->CreateFileHistory();
    ASSERT_TRUE(history != nullptr);
    FhComment fh_comment("Initial file change");
    fh_comment.ToolId("Google Unit Test");
    fh_comment.ToolVendor("ACME Road Runner Company");
    fh_comment.ToolVersion("1.0");
    fh_comment.UserName("ihedvall");
    history->SetFhComment(fh_comment);

    writer->BusType(MdfBusType::MOST);
    writer->StorageType(MdfStorageType::VlsdStorage);
    writer->PreTrigTime(0.0);
    writer->CompressData(false);

    // Create a DG block
    auto* last_dg = header->CreateDataGroup();
    ASSERT_TRUE(last_dg != nullptr);

    // Create a MOST
    MostConfigAdapter most_config(*writer);
    most_config.CreateConfig(*last_dg);

    IChannelGroup* most_message = last_dg->GetChannelGroup("_Message");
    ASSERT_TRUE(most_message != nullptr);

    IChannelGroup* most_packet = last_dg->GetChannelGroup("_Packet");
    ASSERT_TRUE(most_packet != nullptr);

    IChannelGroup* eth_message = last_dg->GetChannelGroup("_EthernetPacket");
    ASSERT_TRUE(eth_message != nullptr);

    IChannelGroup* state_message = last_dg->GetChannelGroup("_SignalState");
    ASSERT_TRUE(state_message != nullptr);

    IChannelGroup* count_message = last_dg->GetChannelGroup("_MaxPosInfo");
    ASSERT_TRUE(count_message != nullptr);

    IChannelGroup* sbc_message = last_dg->GetChannelGroup("_BoundDesc");
    ASSERT_TRUE(sbc_message != nullptr);

    IChannelGroup* table_message = last_dg->GetChannelGroup("_AllocTable");
    ASSERT_TRUE(table_message != nullptr);

    IChannelGroup* lock_message = last_dg->GetChannelGroup("_SysLockState");
    ASSERT_TRUE(lock_message != nullptr);

    IChannelGroup* shutdown_message = last_dg->GetChannelGroup("_ShutdownFlag");
    ASSERT_TRUE(shutdown_message != nullptr);

    writer->InitMeasurement();
    auto tick_time = TimeStampToNs();
    writer->StartMeasurement(tick_time);

    for (size_t sample = 0; sample < max_sample; ++sample) {
      MostMessage msg;
      msg.BusChannel(123);
      msg.Speed(MostSpeed::Most50);

      std::vector<uint8_t> data;
      data.assign(sample + 1 , static_cast<uint8_t>(sample + 1));
      msg.DataBytes(data);

      MostPacket packet;
      packet.BusChannel(123);
      packet.DataBytes(data);

      MostEthernetPacket eth;
      eth.BusChannel(123);
      eth.DataBytes(data);

      MostSignalState state;
      state.BusChannel(123);
      state.SignalState(MostStateOfSignal::StableLock);

      MostMaxPosInfo count;
      count.BusChannel(123);
      count.DeviceCount(static_cast<uint16_t>(sample % 64));

      MostBoundDesc sbc;
      sbc.BusChannel(123);
      sbc.BoundaryCount(static_cast<uint16_t>(sample % 100));

      MostAllocTable table;
      table.BusChannel(123);
      table.FreeByteCount(static_cast<uint16_t>(sample % 100));
      table.TableLayout(MostTableLayout::ChannelResourceAllocationTable);
      table.TableData(data);

      MostSysLockState lock;
      lock.BusChannel(123);
      lock.RingIsClosed((sample % 2) == 0 ? true : false);

      MostShutdownFlag shutdown;
      shutdown.BusChannel(123);
      shutdown.ShutdownFlag((sample % 2) == 0 ? true : false);

      // Add dummy message to all for message types. Not realistic
      // but makes the test simpler.
      writer->SaveMostMessage(*last_dg, *most_message, tick_time, msg);
      writer->SaveMostMessage(*last_dg, *most_packet, tick_time, packet);
      writer->SaveMostMessage(*last_dg, *eth_message, tick_time, eth);
      writer->SaveMostMessage(*last_dg, *state_message, tick_time, state);
      writer->SaveMostMessage(*last_dg, *sbc_message, tick_time, sbc);
      writer->SaveMostMessage(*last_dg, *count_message, tick_time, count);
      writer->SaveMostMessage(*last_dg, *table_message, tick_time, table);
      writer->SaveMostMessage(*last_dg, *lock_message, tick_time, lock);
      writer->SaveMostMessage(*last_dg, *shutdown_message, tick_time, shutdown);
      tick_time += 1'000'000;
    }

    writer->StopMeasurement(tick_time);
    writer->FinalizeMeasurement();
  }

  {
    MdfReader reader(mdf_file.string());
    ChannelObserverList observer_list;

    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData());
    const MdfFile* file = reader.GetFile();
    ASSERT_TRUE(file != nullptr);

    const IHeader* header = file->Header();
    ASSERT_TRUE(header != nullptr);

    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);

    for (IDataGroup* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      const auto cg_list = data_group->ChannelGroups();
      EXPECT_EQ(cg_list.size(), 13);
      for (IChannelGroup* channel_group : cg_list) {
        ASSERT_TRUE(channel_group != nullptr);
        CreateChannelObserverForChannelGroup(*data_group, *channel_group,
                                             observer_list);
      }
      reader.ReadData(*data_group);
    }
    reader.Close();

    for (const auto& observer : observer_list) {
      ASSERT_TRUE(observer);

      EXPECT_EQ(observer->NofSamples(), max_sample) << observer->Name();
      if (observer->Name().find(".BusChannel") != std::string::npos ) {
        for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
          uint8_t bus_channel = 0;
          const bool valid = observer->GetChannelValue(sample, bus_channel);
          ASSERT_TRUE(valid);
        }
      }
    }
  }
}

TEST_F(TestMostLogger, MostCompressStorage) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000;
  path mdf_file(kTestDir);
  mdf_file.append("most_compress_storage.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
    ASSERT_TRUE(writer);
    writer->Init(mdf_file.string());
    writer->BusType(MdfBusType::MOST);
    writer->StorageType(MdfStorageType::VlsdStorage);
    writer->PreTrigTime(0.0);
    writer->CompressData(true);
    writer->MandatoryMembersOnly(true);

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    HdComment hd_comment("Test of MOST Compress Storage");
    hd_comment.Author("Ingemar Hedvall");
    hd_comment.Department("IH Development");
    hd_comment.Project("MOST bus support");
    hd_comment.Subject("Testing MOST support");
    hd_comment.TimeSource(MdString("PC UTC Time"));
    header->SetHdComment(hd_comment);

    auto* history = header->CreateFileHistory();
    ASSERT_TRUE(history != nullptr);
    FhComment fh_comment("Initial file change");
    fh_comment.ToolId("Google Unit Test");
    fh_comment.ToolVendor("ACME Road Runner Company");
    fh_comment.ToolVersion("1.0");
    fh_comment.UserName("ihedvall");
    history->SetFhComment(fh_comment);



    // Create a DG block
    auto* last_dg = header->CreateDataGroup();
    ASSERT_TRUE(last_dg != nullptr);

    // Create a MOST
    MostConfigAdapter most_config(*writer);
    most_config.CreateConfig(*last_dg);

    IChannelGroup* most_message = last_dg->GetChannelGroup("_Message");
    ASSERT_TRUE(most_message != nullptr);

    IChannelGroup* most_packet = last_dg->GetChannelGroup("_Packet");
    ASSERT_TRUE(most_packet != nullptr);

    IChannelGroup* eth_message = last_dg->GetChannelGroup("_EthernetPacket");
    ASSERT_TRUE(eth_message != nullptr);

    IChannelGroup* state_message = last_dg->GetChannelGroup("_SignalState");
    ASSERT_TRUE(state_message != nullptr);

    IChannelGroup* count_message = last_dg->GetChannelGroup("_MaxPosInfo");
    ASSERT_TRUE(count_message != nullptr);

    IChannelGroup* sbc_message = last_dg->GetChannelGroup("_BoundDesc");
    ASSERT_TRUE(sbc_message != nullptr);

    IChannelGroup* table_message = last_dg->GetChannelGroup("_AllocTable");
    ASSERT_TRUE(table_message != nullptr);

    IChannelGroup* lock_message = last_dg->GetChannelGroup("_SysLockState");
    ASSERT_TRUE(lock_message != nullptr);

    IChannelGroup* shutdown_message = last_dg->GetChannelGroup("_ShutdownFlag");
    ASSERT_TRUE(shutdown_message != nullptr);

    writer->InitMeasurement();
    auto tick_time = TimeStampToNs();
    writer->StartMeasurement(tick_time);

    for (size_t sample = 0; sample < max_sample; ++sample) {
      MostMessage msg;
      msg.BusChannel(123);
      msg.Speed(MostSpeed::Most50);

      std::vector<uint8_t> data;
      data.assign(sample + 1 , static_cast<uint8_t>(sample + 1));
      msg.DataBytes(data);

      MostPacket packet;
      packet.BusChannel(123);
      packet.DataBytes(data);

      MostEthernetPacket eth;
      eth.BusChannel(123);
      eth.DataBytes(data);

      MostSignalState state;
      state.BusChannel(123);
      state.SignalState(MostStateOfSignal::StableLock);

      MostMaxPosInfo count;
      count.BusChannel(123);
      count.DeviceCount(static_cast<uint16_t>(sample % 64));

      MostBoundDesc sbc;
      sbc.BusChannel(123);
      sbc.BoundaryCount(static_cast<uint16_t>(sample % 100));

      MostAllocTable table;
      table.BusChannel(123);
      table.FreeByteCount(static_cast<uint16_t>(sample % 100));
      table.TableLayout(MostTableLayout::ChannelResourceAllocationTable);
      table.TableData(data);

      MostSysLockState lock;
      lock.BusChannel(123);
      lock.RingIsClosed((sample % 2) == 0 ? true : false);

      MostShutdownFlag shutdown;
      shutdown.BusChannel(123);
      shutdown.ShutdownFlag((sample % 2) == 0 ? true : false);

      // Add dummy message to all for message types. Not realistic
      // but makes the test simpler.
      writer->SaveMostMessage(*last_dg, *most_message, tick_time, msg);
      writer->SaveMostMessage(*last_dg, *most_packet, tick_time, packet);
      writer->SaveMostMessage(*last_dg, *eth_message, tick_time, eth);
      writer->SaveMostMessage(*last_dg, *state_message, tick_time, state);
      writer->SaveMostMessage(*last_dg, *sbc_message, tick_time, sbc);
      writer->SaveMostMessage(*last_dg, *count_message, tick_time, count);
      writer->SaveMostMessage(*last_dg, *table_message, tick_time, table);
      writer->SaveMostMessage(*last_dg, *lock_message, tick_time, lock);
      writer->SaveMostMessage(*last_dg, *shutdown_message, tick_time, shutdown);
      tick_time += 1'000'000;
    }

    writer->StopMeasurement(tick_time);
    writer->FinalizeMeasurement();
  }

  {
    MdfReader reader(mdf_file.string());
    ChannelObserverList observer_list;

    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData());
    const MdfFile* file = reader.GetFile();
    ASSERT_TRUE(file != nullptr);

    const IHeader* header = file->Header();
    ASSERT_TRUE(header != nullptr);

    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);

    for (IDataGroup* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      const auto cg_list = data_group->ChannelGroups();
      EXPECT_EQ(cg_list.size(), 13);
      for (IChannelGroup* channel_group : cg_list) {
        ASSERT_TRUE(channel_group != nullptr);
        CreateChannelObserverForChannelGroup(*data_group, *channel_group,
                                             observer_list);
      }
      reader.ReadData(*data_group);
    }
    reader.Close();

    for (const auto& observer : observer_list) {
      ASSERT_TRUE(observer);

      EXPECT_EQ(observer->NofSamples(), max_sample) << observer->Name();
      if (observer->Name().find(".BusChannel") != std::string::npos ) {
        for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
          uint8_t bus_channel = 0;
          const bool valid = observer->GetChannelValue(sample, bus_channel);
          ASSERT_TRUE(valid);
        }
      }
    }
  }
}


}  // namespace mdf