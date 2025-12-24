/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "testflexraylogger.h"

#include <filesystem>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/flexraymessage.h"
#include "mdf/mdflogstream.h"
#include "mdf/ifilehistory.h"
#include "mdf/flexrayconfigadapter.h"
#include "mdf/mdfreader.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"

using namespace util::log;
using namespace std::filesystem;
using namespace util::time;

namespace {

std::string kTestRootDir; ///< Test root directory (%TEMP%/test)
std::string kTestDir; ///<  'Test Root Dir'/mdf/flexray";
bool kSkipTest = false; ///< Set to true if the output dir is missing

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& location, mdf::MdfLogSeverity severity,
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
void NoLog(const MdfLocation& location , mdf::MdfLogSeverity severity,
           const std::string& text ) {
}

}

namespace mdf::test {

void TestFlexRayLogger::SetUpTestSuite() {

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

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("flexray");

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

void TestFlexRayLogger::TearDownTestSuite() {
  LOG_INFO() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}


TEST_F(TestFlexRayLogger, IFlexRayEvent) {
  FlexRayFrame msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::Frame);

  msg.BusChannel(123);
  EXPECT_EQ(msg.BusChannel(), 123);

  msg.FrameId(345);
  EXPECT_EQ(msg.FrameId(),345);

  msg.CycleCount(34);
  EXPECT_EQ(msg.CycleCount(), 34);

  msg.Channel(FlexRayChannel::B);
  EXPECT_EQ(msg.Channel(), FlexRayChannel::B);

  msg.Direction(FlexRayDirection::Tx);
  EXPECT_EQ(msg.Direction(), FlexRayDirection::Tx);
}

TEST_F(TestFlexRayLogger, FlexRayFrame) {
  FlexRayFrame msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::Frame);

  msg.PayloadLength(34);
  EXPECT_EQ(msg.PayloadLength(), 34);

  const std::vector<uint8_t> data_array(45, 0xFF);
  msg.DataBytes(data_array);
  EXPECT_EQ(msg.DataBytes(),data_array);

  msg.Crc(12345);
  EXPECT_EQ(msg.Crc(), 12345);

  msg.HeaderCrc(234);
  EXPECT_EQ(msg.HeaderCrc(), 234);

  msg.Reserved(true);
  EXPECT_TRUE(msg.Reserved());

  msg.PayloadPreamble(true);
  EXPECT_TRUE(msg.PayloadPreamble());

  msg.NullFrame(FlexRayNullFlag::NullFrame);
  EXPECT_EQ(msg.NullFrame(), FlexRayNullFlag::NullFrame);

  msg.SyncFrame(true);
  EXPECT_TRUE(msg.SyncFrame());

  msg.StartupFrame(true);
  EXPECT_TRUE(msg.StartupFrame());

  msg.FrameLength(344'000'000);
  EXPECT_EQ(msg.FrameLength(), 344'000'000);
}

TEST_F(TestFlexRayLogger, FlexRayPdu) {
  FlexRayPdu msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::Pdu);

  msg.Length(34);
  EXPECT_EQ(msg.Length(), 34);

  const std::vector<uint8_t> data_array(45, 0xFF);
  msg.DataBytes(data_array);
  EXPECT_EQ(msg.DataBytes(),data_array);

  msg.Multiplexed(true);
  EXPECT_TRUE(msg.Multiplexed());

  msg.Switch(56);
  EXPECT_EQ(msg.Switch(), 56);

  msg.Valid(true);
  EXPECT_TRUE(msg.Valid());

  msg.Valid(false);
  EXPECT_FALSE(msg.Valid());

  msg.UpdateBitPos(33);
  EXPECT_EQ(msg.UpdateBitPos(), 33);

  msg.ByteOffset(3);
  EXPECT_EQ(msg.ByteOffset(), 3);

  msg.BitOffset(2);
  EXPECT_EQ(msg.BitOffset(), 2);
}

TEST_F(TestFlexRayLogger, FlexRayFrameHeader) {
  FlexRayFrameHeader msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::FrameHeader);

  const std::vector<uint8_t> data_array(45, 0xAA);
  msg.FillDataBytes(data_array);
  EXPECT_EQ(msg.FillDataBytes(),data_array);
}

TEST_F(TestFlexRayLogger, FlexRayNullFrame) {
  FlexRayNullFrame msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::NullFrame);
}

TEST_F(TestFlexRayLogger, FlexRayErrorFrame) {
  FlexRayErrorFrame msg;
  EXPECT_EQ(msg.MessageType(), FlexRayMessageType::ErrorFrame);

  msg.Syntax(true);
  EXPECT_TRUE(msg.Syntax());

  msg.Content(true);
  EXPECT_TRUE(msg.Content());

  msg.Boundary(true);
  EXPECT_TRUE(msg.Boundary());

  msg.TxConflict(true);
  EXPECT_TRUE(msg.TxConflict());

  msg.Valid(true);
  EXPECT_TRUE(msg.Valid());
 }

 TEST_F(TestFlexRayLogger, FlexRaySymbol) {
   FlexRaySymbol msg;
   EXPECT_EQ(msg.MessageType(), FlexRayMessageType::Symbol);

   msg.ChannelMask(FlexRayChannelMask::A);
   EXPECT_EQ(msg.ChannelMask(), FlexRayChannelMask::A);

   msg.Length(123);
   EXPECT_EQ(msg.Length(), 123);

   msg.Type(FlexRaySymbolType::CAS);
   EXPECT_EQ(msg.Type(), FlexRaySymbolType::CAS);
 }

TEST_F(TestFlexRayLogger, FlexRayVlsdStorage) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000;
  path mdf_file(kTestDir);
  mdf_file.append("flx_vlsd_storage.mf4");
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

    writer->BusType(MdfBusType::FlexRay);
    writer->StorageType(MdfStorageType::VlsdStorage);
    writer->PreTrigTime(0.0);
    writer->CompressData(false);

    // Create a DG block
    auto* last_dg = header->CreateDataGroup();
    ASSERT_TRUE(last_dg != nullptr);

    // Create a MOST
    FlexRayConfigAdapter flx_config(*writer);
    flx_config.CreateConfig(*last_dg);

    IChannelGroup* flx_frame = last_dg->GetChannelGroup("FLX_Frame");
    ASSERT_TRUE(flx_frame != nullptr);

    IChannelGroup* flx_pdu = last_dg->GetChannelGroup("FLX_Pdu");
    ASSERT_TRUE(flx_pdu != nullptr);

    IChannelGroup* flx_header = last_dg->GetChannelGroup("FLX_FrameHeader");
    ASSERT_TRUE(flx_header != nullptr);

    IChannelGroup* flx_null = last_dg->GetChannelGroup("FLX_NullFrame");
    ASSERT_TRUE(flx_null != nullptr);

    IChannelGroup* flx_error = last_dg->GetChannelGroup("FLX_ErrorFrame");
    ASSERT_TRUE(flx_error != nullptr);

    IChannelGroup* flx_symbol = last_dg->GetChannelGroup("FLX_Symbol");
    ASSERT_TRUE(flx_symbol != nullptr);

    writer->InitMeasurement();
    uint64_t tick_time = TimeStampToNs();
    writer->StartMeasurement(tick_time);

    for (size_t sample = 0; sample < max_sample; ++sample) {
      std::vector<uint8_t> data;
      const auto length = static_cast<uint8_t>(sample);
      data.assign(length, length);

      FlexRayFrame frame;
      frame.BusChannel(123);
      frame.FrameId(345);
      frame.CycleCount(static_cast<uint8_t>(sample & 0x3F));
      frame.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
      frame.PayloadLength(length / 2);
      frame.DataBytes(data);
      frame.Crc(1234);
      frame.HeaderCrc(345);
      frame.Reserved((sample % 2) == 0);
      frame.PayloadPreamble((sample % 2) == 1);

      FlexRayPdu pdu;
      pdu.BusChannel(123);
      pdu.FrameId(345);
      pdu.CycleCount(static_cast<uint8_t>(sample & 0x3F));
      pdu.Length(length);
      pdu.DataBytes(data);

      FlexRayFrameHeader head;
      head.BusChannel(123);
      head.FrameId(345);
      head.CycleCount(static_cast<uint8_t>(sample & 0x3F));
      head.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
      head.PayloadLength(length / 2);
      head.FillDataBytes(data);

      FlexRayNullFrame null;
      null.BusChannel(123);
      null.FrameId(345);
      null.CycleCount(static_cast<uint8_t>(sample & 0x3F));
      null.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
      null.PayloadLength(length / 2);
      null.DataBytes(data);

      FlexRayErrorFrame error;
      error.BusChannel(123);
      error.FrameId(345);
      error.CycleCount(static_cast<uint8_t>(sample & 0x3F));
      error.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
      error.PayloadLength(length / 2);
      error.DataBytes(data);

      FlexRaySymbol symbol;
      symbol.BusChannel(123);
      symbol.CycleCount(static_cast<uint8_t>(sample & 0x3F));

      // Add dummy message to all for message types. Not realistic
      // but makes the test simpler.
      writer->SaveFlexRayMessage(*last_dg, *flx_frame, tick_time, frame);
      writer->SaveFlexRayMessage(*last_dg, *flx_pdu, tick_time, pdu);
      writer->SaveFlexRayMessage(*last_dg, *flx_header, tick_time, head);
      writer->SaveFlexRayMessage(*last_dg, *flx_null, tick_time, null);
      writer->SaveFlexRayMessage(*last_dg, *flx_error, tick_time, error);
      writer->SaveFlexRayMessage(*last_dg, *flx_symbol, tick_time, symbol);
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
      EXPECT_EQ(cg_list.size(), 11);
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
    }
  }

}

TEST_F(TestFlexRayLogger, FlexRayCompresStorage) {
 if (kSkipTest) {
   GTEST_SKIP();
 }
 constexpr size_t max_sample = 20'000;
 path mdf_file(kTestDir);
 mdf_file.append("flx_compress_storage.mf4");
 {
   auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
   ASSERT_TRUE(writer);
   writer->Init(mdf_file.string());

   writer->BusType(MdfBusType::FlexRay);
   writer->StorageType(MdfStorageType::VlsdStorage);
   writer->PreTrigTime(0.0);
   writer->CompressData(true);
   writer->MandatoryMembersOnly(true);

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

   // Create a DG block
   auto* last_dg = header->CreateDataGroup();
   ASSERT_TRUE(last_dg != nullptr);

   // Create a FlexRay configuration
   FlexRayConfigAdapter flx_config(*writer);
   flx_config.CreateConfig(*last_dg);

   IChannelGroup* flx_frame = last_dg->GetChannelGroup("FLX_Frame");
   ASSERT_TRUE(flx_frame != nullptr);

   IChannelGroup* flx_pdu = last_dg->GetChannelGroup("FLX_Pdu");
   ASSERT_TRUE(flx_pdu != nullptr);

   IChannelGroup* flx_header = last_dg->GetChannelGroup("FLX_FrameHeader");
   ASSERT_TRUE(flx_header != nullptr);

   IChannelGroup* flx_null = last_dg->GetChannelGroup("FLX_NullFrame");
   ASSERT_TRUE(flx_null != nullptr);

   IChannelGroup* flx_error = last_dg->GetChannelGroup("FLX_ErrorFrame");
   ASSERT_TRUE(flx_error != nullptr);

   IChannelGroup* flx_symbol = last_dg->GetChannelGroup("FLX_Symbol");
   ASSERT_TRUE(flx_symbol != nullptr);

   writer->InitMeasurement();
   uint64_t tick_time = TimeStampToNs();
   writer->StartMeasurement(tick_time);

   for (size_t sample = 0; sample < max_sample; ++sample) {
     std::vector<uint8_t> data;
     const auto length = static_cast<uint8_t>(sample);
     data.assign(length, length);

     FlexRayFrame frame;
     frame.BusChannel(123);
     frame.FrameId(345);
     frame.CycleCount(static_cast<uint8_t>(sample & 0x3F));
     frame.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
     frame.PayloadLength(length / 2);
     frame.DataBytes(data);
     frame.Crc(1234);
     frame.HeaderCrc(345);
     frame.Reserved((sample % 2) == 0);
     frame.PayloadPreamble((sample % 2) == 1);

     FlexRayPdu pdu;
     pdu.BusChannel(123);
     pdu.FrameId(345);
     pdu.CycleCount(static_cast<uint8_t>(sample & 0x3F));
     pdu.Length(length);
     pdu.DataBytes(data);

     FlexRayFrameHeader head;
     head.BusChannel(123);
     head.FrameId(345);
     head.CycleCount(static_cast<uint8_t>(sample & 0x3F));
     head.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
     head.PayloadLength(length / 2);
     head.FillDataBytes(data);

     FlexRayNullFrame null;
     null.BusChannel(123);
     null.FrameId(345);
     null.CycleCount(static_cast<uint8_t>(sample & 0x3F));
     null.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
     null.PayloadLength(length / 2);
     null.DataBytes(data);

     FlexRayErrorFrame error;
     error.BusChannel(123);
     error.FrameId(345);
     error.CycleCount(static_cast<uint8_t>(sample & 0x3F));
     error.Direction((sample % 2) == 0 ? FlexRayDirection::Tx : FlexRayDirection::Rx);
     error.PayloadLength(length / 2);
     error.DataBytes(data);

     FlexRaySymbol symbol;
     symbol.BusChannel(123);
     symbol.CycleCount(static_cast<uint8_t>(sample & 0x3F));

     // Add dummy message to all for message types. Not realistic
     // but makes the test simpler.
     writer->SaveFlexRayMessage(*last_dg, *flx_frame, tick_time, frame);
     writer->SaveFlexRayMessage(*last_dg, *flx_pdu, tick_time, pdu);
     writer->SaveFlexRayMessage(*last_dg, *flx_header, tick_time, head);
     writer->SaveFlexRayMessage(*last_dg, *flx_null, tick_time, null);
     writer->SaveFlexRayMessage(*last_dg, *flx_error, tick_time, error);
     writer->SaveFlexRayMessage(*last_dg, *flx_symbol, tick_time, symbol);
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
     EXPECT_EQ(cg_list.size(), 8);
     for (IChannelGroup* channel_group : cg_list) {
       ASSERT_TRUE(channel_group != nullptr);
       CreateChannelObserverForChannelGroup(*data_group, *channel_group,
                                            observer_list);
     }
     EXPECT_TRUE(reader.ReadData(*data_group));
   }
   reader.Close();

   for (const auto& observer : observer_list) {
     ASSERT_TRUE(observer);

     EXPECT_EQ(observer->NofSamples(), max_sample) << observer->Name();
   }
 }

}

}  // namespace mdf