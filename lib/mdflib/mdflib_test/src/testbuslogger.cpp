/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "testbuslogger.h"

#include <filesystem>
#include <algorithm>
#include <ranges>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/mdfhelper.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"
#include "mdf/ifilehistory.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/canconfigadapter.h"
#include "mdf/canbusobserver.h"

using namespace util::log;
using namespace std::filesystem;
using namespace util::time;
namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/bus";
bool kSkipTest = false;
bool kErrorDetected = false;

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
  if (severity >= mdf::MdfLogSeverity::kNotice) {
    kErrorDetected = true;
  }
  log_config.AddLogMessage(message);
}
/**
 * Function that stops logging of MDF logs
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void NoLog(const MdfLocation& location, mdf::MdfLogSeverity severity ,
           const std::string& text) {
}

}

namespace mdf::test {

void TestBusLogger::SetUpTestSuite() {

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to the console instead of as normally to a file
    auto& log_config = LogConfig::Instance();
    log_config.Type(LogType::LogToConsole);
    log_config.CreateDefaultLogger();

    // Connect MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("bus");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

    LOG_TRACE() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestBusLogger::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestBusLogger, CanMessage) {
  CanMessage msg;
  msg.MessageId(0x0010);
  EXPECT_EQ(msg.MessageId(), 0x0010);
  EXPECT_EQ(msg.CanId(), 0x0010);
  EXPECT_FALSE(msg.ExtendedId());


  msg.MessageId(0x8020);
  EXPECT_EQ(msg.MessageId(), 0x80008020);
  EXPECT_EQ(msg.CanId(), 0x8020);
  EXPECT_TRUE(msg.ExtendedId());

  msg.MessageId(0x0030);
  msg.ExtendedId(true);
  EXPECT_EQ(msg.MessageId(), 0x80000030);
  EXPECT_EQ(msg.CanId(), 0x30);

  msg.MessageId(0x0040);
  msg.ExtendedId(false);
  EXPECT_EQ(msg.MessageId(), 0x40);
  EXPECT_EQ(msg.CanId(), 0x40);

  msg.Dlc(7);
  EXPECT_EQ(msg.Dlc(), 7);

  msg.DataLength(63);
  EXPECT_EQ(msg.Dlc(), 15);
  EXPECT_EQ(msg.DataLength(), 63);

  std::vector<uint8_t> data(8, 0x55);
  msg.DataBytes(data);
  EXPECT_EQ(msg.DataBytes(), data);
  EXPECT_EQ(msg.Dlc(), 8);
  EXPECT_EQ(msg.DataLength(), 8);

  msg.Dir(true);
  EXPECT_TRUE(msg.Dir());
  msg.Dir(false);
  EXPECT_FALSE(msg.Dir());

  msg.Srr(true);
  EXPECT_TRUE(msg.Srr());
  msg.Srr(false);
  EXPECT_FALSE(msg.Srr());

  msg.Edl(true);
  EXPECT_TRUE(msg.Edl());
  msg.Edl(false);
  EXPECT_FALSE(msg.Edl());

  msg.Brs(true);
  EXPECT_TRUE(msg.Brs());
  msg.Brs(false);
  EXPECT_FALSE(msg.Brs());

  msg.Esi(true);
  EXPECT_TRUE(msg.Esi());
  msg.Esi(false);
  EXPECT_FALSE(msg.Esi());

  msg.Rtr(true);
  EXPECT_TRUE(msg.Rtr());
  msg.Rtr(false);
  EXPECT_FALSE(msg.Rtr());

  msg.WakeUp(true);
  EXPECT_TRUE(msg.WakeUp());
  msg.WakeUp(false);
  EXPECT_FALSE(msg.WakeUp());

  msg.SingleWire(true);
  EXPECT_TRUE(msg.SingleWire());
  msg.SingleWire(false);
  EXPECT_FALSE(msg.SingleWire());

  msg.R0(true);
  EXPECT_TRUE(msg.R0());
  msg.R0(false);
  EXPECT_FALSE(msg.R0());

  msg.R1(true);
  EXPECT_TRUE(msg.R1());
  msg.R1(false);
  EXPECT_FALSE(msg.R1());

  msg.BusChannel(123);
  EXPECT_EQ(msg.BusChannel(), 123);

  msg.BitPosition(34);
  EXPECT_EQ(msg.BitPosition(), 34);

  constexpr std::array<CanErrorType, 6> error_type_list = {
      CanErrorType::UNKNOWN_ERROR,
      CanErrorType::BIT_ERROR,
      CanErrorType::FORM_ERROR,
      CanErrorType::BIT_STUFFING_ERROR,
      CanErrorType::CRC_ERROR,
      CanErrorType::ACK_ERROR,
  };
  for ( const auto error_type : error_type_list ) {
    msg.ErrorType(error_type);
    EXPECT_EQ(msg.ErrorType(), error_type);
  }

  msg.FrameDuration(0xFFFF);
  EXPECT_EQ(msg.FrameDuration(), 0xFFFF);

  msg.Reset();
  EXPECT_EQ(msg.MessageId(), 0);
  EXPECT_EQ(msg.DataBytes().size(), 0);
}

TEST_F(TestBusLogger, Mdf4CanSdStorage ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 10'000;

  path mdf_file(kTestDir);
  mdf_file.append("can_sd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage); // Variable length to SD
  writer->MaxLength(8); // No meaning in this type of storage

  CanConfigAdapter config_adapter(*writer);
  config_adapter.NetworkName("IHNET");
  config_adapter.Protocol("J1939");
  auto* data_group = writer->CreateDataGroup();
  ASSERT_TRUE(data_group != nullptr);
  config_adapter.CreateConfig(*data_group);

  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < max_samples; ++sample) {
    // Assigned some dummy data
    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  bool sample_valid = false;
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);

    if (observer->Name().find("DataBytes") != std::string::npos) {
      // Check that storage type is fixed length i.e SD storage.
      const IChannel& channel = observer->Channel();
      const IChannelGroup* channel_group  = channel.ChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);
      EXPECT_EQ(channel_group->StorageType(), MdfStorageType::FixedLengthStorage);

      for (size_t sample1 = 0; sample1 < max_samples; ++sample1) {
        std::vector<uint8_t> reference_data;
        reference_data.assign(sample1 < 8 ? sample1 + 1 : 8,
          static_cast<uint8_t>(sample1 + 1));
        std::vector<uint8_t> data;
        const bool valid = observer->GetChannelValue(sample1, data);
        EXPECT_TRUE(valid);
        ASSERT_EQ(reference_data, data);
        sample_valid = true;
     }
    }
  }
  EXPECT_TRUE(sample_valid);
}

TEST_F(TestBusLogger, Mdf4VlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 22'000;
  path mdf_file(kTestDir);
  mdf_file.append("can_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(20);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < max_samples; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.BusChannel(11);
    msg.MessageId(123);
    msg.ExtendedId(true);

    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    if (sample < 5) {
      SampleRecord temp;
      msg.ToRaw(MessageType::CAN_DataFrame,temp, 8, true);
      std::cout << "Sample: " << sample
                << ", Size: " << temp.vlsd_buffer.size() << std::endl;
    }

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);


  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 6);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();


  bool sample_valid = false;
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);

    if (observer->Name().find("DataBytes") != std::string::npos ) {
      // Check that storage type is VLSD CG storage.
      const IChannel& channel = observer->Channel();
      const IChannelGroup* channel_group  = channel.ChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);
      EXPECT_EQ(channel_group->StorageType(), MdfStorageType::VlsdStorage);

      for (uint64_t sample1 = 0; sample1 < observer->NofSamples(); ++sample1) {
        std::vector<uint8_t> reference_data;
        reference_data.assign(sample1 < 8 ? sample1 + 1 : 8,
          static_cast<uint8_t>(sample1 + 1));
        std::vector<uint8_t> frame_data;
        const bool valid = observer->GetEngValue(sample1,frame_data);
        EXPECT_TRUE(valid);
        ASSERT_EQ(reference_data, frame_data);
        sample_valid = true;
      }
    }
  }
  EXPECT_TRUE(sample_valid);
}

TEST_F(TestBusLogger, Mdf4MlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 30'000;
  path mdf_file(kTestDir);
  mdf_file.append("can_mlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < max_samples; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;
  bool sample_valid = false;
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (!unique_list.contains(name)) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }

    if (observer->Name().find("DataBytes") != std::string::npos ) {
      // Check that storage type is VLSD CG storage.
      const IChannel& channel = observer->Channel();
      const IChannelGroup* channel_group  = channel.ChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);
      EXPECT_EQ(channel_group->StorageType(), MdfStorageType::MlsdStorage);
      EXPECT_EQ(channel_group->MaxLength(), channel.DataBytes());

      for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
        std::vector<uint8_t> reference_data;
        reference_data.assign(sample < 8 ? sample + 1 : 8,
          static_cast<uint8_t>(sample + 1));
        std::vector<uint8_t> frame_data;
        const bool valid = observer->GetEngValue(sample,frame_data);
        EXPECT_TRUE(valid);
        ASSERT_EQ(reference_data, frame_data);
        sample_valid = true;
      }
    }
  }
  EXPECT_TRUE(sample_valid);
}

TEST_F(TestBusLogger, Mdf4CompressedCanSdStorage ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_sd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage); // Variable length to SD
  writer->MaxLength(8); // No meaning in this type of storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration()); // Creates all DG/CG/CN
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < 10; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 10);
  }

}

TEST_F(TestBusLogger, Mdf4CompressedVlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8);

  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(true);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 6);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);

  }

}

TEST_F(TestBusLogger, Mdf4MandatoryCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 10'000;
  path mdf_file(kTestDir);
  mdf_file.append("can_mandatory.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  ASSERT_TRUE(writer);
  writer->Init(mdf_file.string());
  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8);
  writer->MandatoryMembersOnly(true);
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  FhComment fh_comment;
  fh_comment.Comment("Test data types");
  fh_comment.ToolId("MdfWrite");
  fh_comment.ToolVendor("ACME Road Runner Company");
  fh_comment.ToolVersion("1.0");
  fh_comment.UserName("Ingemar Hedvall");
  history->SetFhComment(fh_comment);

  IDataGroup* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  DgComment dg_comment("Testing mandatory members");
  last_dg->SetDgComment(dg_comment);

  CanConfigAdapter config(*writer);
  config.CreateConfig(*last_dg);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  //auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  //ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < max_samples; ++sample) {
    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    //writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 5);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);
    const auto& valid_list = observer->GetValidList();
    const bool all_valid = std::ranges::all_of(valid_list, [](bool valid) -> bool {
      return valid;
    });
    EXPECT_TRUE(all_valid);
  }

}

TEST_F(TestBusLogger, Mdf4CompressedMlsdCanConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 10'000;
  path mdf_file(kTestDir);
  mdf_file.append("compressed_can_mlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN);
  writer->StorageType(MdfStorageType::MlsdStorage);
  writer->MaxLength(8);
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(true);
  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);


  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < max_samples; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 4);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  bool sample_valid = false;
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);

    if (observer->Name().find("DataBytes") != std::string::npos) {
      for (size_t sample = 0; sample < max_samples; ++sample) {
        std::vector<uint8_t> reference_data;
        reference_data.assign(sample < 8 ? sample + 1 : 8,
          static_cast<uint8_t>(sample + 1));
        std::vector<uint8_t> data;
        const bool valid = observer->GetEngValue(sample, data);
        EXPECT_TRUE(valid);
        EXPECT_EQ(data.size(), reference_data.size());
        ASSERT_EQ(data, reference_data);
        sample_valid = true;
      }
    }
  }
  EXPECT_TRUE(sample_valid);
}

TEST_F(TestBusLogger, Mdf4SampleObserver ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("sample_observer.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::FixedLengthStorage);
  writer->MaxLength(8); // No meaning in this type of storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration()); // Creates all DG/CG/CN
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < 1000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* header1 = reader.GetHeader();
  auto* last_dg1 = header1->LastDataGroup();
  ASSERT_TRUE(last_dg1 != nullptr);

  const IChannelGroup* channel_group1 = last_dg1->GetChannelGroup("CAN_DataFrame");
  ASSERT_TRUE(channel_group1 != nullptr);

  const IChannel* channel1 = channel_group1->GetChannel("CAN_DataFrame.DataBytes");
  ASSERT_TRUE(channel1 != nullptr);

  uint64_t nof_sample_read = 0; // Testing abort of reading just 10 samples
  ISampleObserver sample_observer(*last_dg1);
  sample_observer.DoOnSample = [&] (uint64_t sample1, uint64_t record_id,
                                   const std::vector<uint8_t>& record ) -> bool {
    bool eng_valid = true;

    bool channel_valid = true;


    if (channel1->RecordId() == record_id) {
      std::string eng_value;
      std::string channel_value;
      eng_valid = sample_observer.GetEngValue(*channel1,sample1,
                                          record, eng_value );
      channel_valid = sample_observer.GetChannelValue(*channel1, sample1,
            record, channel_value);

      if (sample1 > 9) {
        return false;
      }
      std::cout << "Sample: " << sample1
          << ", Record: " << record_id
          << ", Channel Value: " << channel_value
          << ", Eng Value: " << eng_value
          << std::endl;
      ++nof_sample_read;
    }
    EXPECT_TRUE(channel_valid);
    EXPECT_TRUE(eng_valid);
    return true;
  };
  reader.ReadData(*last_dg1);
  EXPECT_EQ(nof_sample_read, 10); // The read should be interrupted after
                                   // 10 samples
  sample_observer.DetachObserver();
  reader.Close();
}

TEST_F(TestBusLogger, Mdf4CanObserver ) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 10'000;

  path mdf_file(kTestDir);
  mdf_file.append("can_observer.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::CAN); // Defines the CG/CN names
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8); // No meaning in this type of storage
  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  CanConfigAdapter config(*writer);
  config.CreateConfig(*last_dg);

  auto* can_data_frame = last_dg->GetChannelGroup("CAN_DataFrame");
  auto* can_remote_frame = last_dg->GetChannelGroup("CAN_RemoteFrame");
  auto* can_error_frame = last_dg->GetChannelGroup("CAN_ErrorFrame");
  auto* can_overload_frame = last_dg->GetChannelGroup("CAN_OverloadFrame");

  ASSERT_TRUE(can_data_frame != nullptr);
  ASSERT_TRUE(can_remote_frame != nullptr);
  ASSERT_TRUE(can_error_frame != nullptr);
  ASSERT_TRUE(can_overload_frame != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  size_t sample;
  for (sample = 0; sample < max_samples; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    CanMessage msg;
    msg.MessageId(123);
    msg.ExtendedId(true);
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);
    msg.Crc(3421);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveCanMessage(*can_data_frame, tick_time, msg);
    writer->SaveCanMessage(*can_remote_frame, tick_time, msg);
    writer->SaveCanMessage(*can_error_frame, tick_time, msg);
    writer->SaveCanMessage(*can_overload_frame, tick_time, msg);
    tick_time += 1'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* header1 = reader.GetHeader();
  ASSERT_TRUE(header1 != nullptr);

  auto* last_dg1 = header1->LastDataGroup();
  ASSERT_TRUE(last_dg1 != nullptr);

  const IChannelGroup* channel_group1 = last_dg1->GetChannelGroup("_DataFrame");
  ASSERT_TRUE(channel_group1 != nullptr);

  EXPECT_EQ(channel_group1->NofSamples(), max_samples);

  CanBusObserver sample_observer(*last_dg1, *channel_group1);
  EXPECT_EQ(sample_observer.NofSamples(), max_samples);

  /*
  sample_observer.OnCanMessage = [&] (uint64_t sample1, uint64_t record_id,
                                   const std::vector<uint8_t>& record ) -> bool {
    bool eng_valid = true;

    bool channel_valid = true;


    if (channel1->RecordId() == record_id) {
      std::string eng_value;
      std::string channel_value;
      eng_valid = sample_observer.GetEngValue(*channel1,sample1,
                                          record, eng_value );
      channel_valid = sample_observer.GetChannelValue(*channel1, sample1,
            record, channel_value);

      if (sample1 > 9) {
        return false;
      }
      std::cout << "Sample: " << sample1
          << ", Record: " << record_id
          << ", Channel Value: " << channel_value
          << ", Eng Value: " << eng_value
          << std::endl;
      ++nof_sample_read;
    }
    EXPECT_TRUE(channel_valid);
    EXPECT_TRUE(eng_valid);
    return true;
  };
  */

  reader.ReadData(*last_dg1);
  sample_observer.DetachObserver();
  reader.Close();
}

TEST_F(TestBusLogger, ErrorDetected) {
  EXPECT_FALSE(kErrorDetected);
}
}  // namespace mdf::test