/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "testethlogger.h"

#include <filesystem>
#include <ranges>

#include <boost/iostreams/device/array.hpp>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/ifilehistory.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"

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

void TestEthLogger::SetUpTestSuite() {

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
    temp_dir.append("eth");
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

void TestEthLogger::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestEthLogger, TestEthMessage) {
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  ASSERT_TRUE(writer);

  EthMessage msg;

  msg.BusChannel(5);
  EXPECT_EQ(msg.BusChannel(), 5);

  msg.Dir(true);
  EXPECT_TRUE(msg.Dir());

  const uint8_t source_addr[6] = {1,2,3,4,5,6};
  const uint8_t* source_temp = source_addr;
  msg.Source(source_temp);
  const auto& source = msg.Source();
  for (size_t index1 = 0; index1 < 6; ++index1) {
    EXPECT_EQ(source[index1], source_addr[index1]);
  }

  const uint8_t dest_addr[6] = {2,3,4,5,6,7};
  const uint8_t* dest_temp = dest_addr;
  msg.Destination(dest_temp);
  const auto& dest = msg.Destination();
  for (size_t index2 = 0; index2 < 6; ++index2) {
    EXPECT_EQ(dest[index2], dest_addr[index2]);
  }

  msg.ReceivedDataByteCount(8);
  EXPECT_EQ(msg.ReceivedDataByteCount(), 8);

  msg.DataLength(7);
  EXPECT_EQ(msg.DataLength(), 7);

  std::vector<uint8_t> data = {0,1,2,3,4,5};
  msg.DataBytes(data);
  EXPECT_EQ(msg.DataBytes(), data);
  EXPECT_EQ(msg.DataLength(), 6);

}

TEST_F(TestEthLogger, Mdf4EthConfig) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_samples = 10'000;
  path mdf_file(kTestDir);
  mdf_file.append("eth_vlsd_storage.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  EXPECT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::Ethernet);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8); // Not needed for SD and VLSD storage
  EXPECT_TRUE(writer->CreateBusLogConfiguration());
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* eth_frame = last_dg->GetChannelGroup("ETH_Frame");
  ASSERT_TRUE(eth_frame != nullptr);

  IChannelGroup* eth_checksum_error = last_dg->GetChannelGroup("ETH_ChecksumError");
  ASSERT_TRUE(eth_checksum_error != nullptr);

  IChannelGroup* eth_length_error = last_dg->GetChannelGroup("ETH_LengthError");
  ASSERT_TRUE(eth_length_error != nullptr);

  IChannelGroup* eth_receive_error = last_dg->GetChannelGroup("ETH_ReceiveError");
  ASSERT_TRUE(eth_receive_error != nullptr);

  EXPECT_TRUE(writer->InitMeasurement());
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < max_samples; ++sample) {
    constexpr std::array<uint8_t, 6> source = {0,1,2,3,4,5};
    constexpr std::array<uint8_t, 6> dest = {7,8,9,0,1,2};
    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    EthMessage msg;
    msg.BusChannel(static_cast<uint8_t>(sample));
    msg.Dir(sample % 2 == 0);
    msg.ErrorType(static_cast<EthErrorType>(sample % 2));
    msg.ReceivedDataByteCount(static_cast<uint16_t>(data.size()));
    msg.Source(source);
    msg.Destination(dest);
    msg.EthType(static_cast<uint16_t>(sample));
    msg.Crc(static_cast<uint32_t>(sample));
    msg.ExpectedCrc(static_cast<uint32_t>(sample + 1));
    msg.PaddingByteCount(static_cast<uint16_t>(sample));
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveEthMessage(*eth_frame, tick_time, msg);
    writer->SaveEthMessage(*eth_checksum_error, tick_time, msg);
    writer->SaveEthMessage(*eth_length_error, tick_time, msg);
    writer->SaveEthMessage(*eth_receive_error, tick_time, msg);
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
    EXPECT_EQ(cg_list.size(), 8);
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  EXPECT_GT(observer_list.size(), 8);
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), max_samples);
    const auto& valid_list = observer->GetValidList();
    const bool all_valid =  std::ranges::all_of(valid_list,
      [] (const bool& valid) -> bool {
      return valid;
    });
    EXPECT_TRUE(all_valid) << observer->Name();

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (!unique_list.contains(name)) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }
}

TEST_F(TestEthLogger, Mdf4EthMandatory) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("eth_mandatory.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfBusLogger);
  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->BusType(MdfBusType::Ethernet);
  writer->StorageType(MdfStorageType::VlsdStorage);
  writer->MaxLength(8);
  writer->MandatoryMembersOnly(true);
  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  EXPECT_TRUE(writer->CreateBusLogConfiguration());

  IDataGroup* last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  IChannelGroup* eth_frame = last_dg->GetChannelGroup("ETH_Frame");
  ASSERT_TRUE(eth_frame != nullptr);

  IChannelGroup* eth_checksum_error = last_dg->GetChannelGroup("ETH_ChecksumError");
  ASSERT_TRUE(eth_checksum_error != nullptr);

  IChannelGroup* eth_length_error = last_dg->GetChannelGroup("ETH_LengthError");
  ASSERT_TRUE(eth_length_error != nullptr);

  IChannelGroup* eth_receive_error = last_dg->GetChannelGroup("ETH_ReceiveError");
  ASSERT_TRUE(eth_receive_error != nullptr);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 100'000; ++sample) {

    std::vector<uint8_t> data;
    data.assign(sample < 8 ? sample + 1 : 8, static_cast<uint8_t>(sample + 1));

    EthMessage msg;
    msg.BusChannel(11);
    EXPECT_EQ(msg.BusChannel(), 11);
    msg.DataBytes(data);

    // Add dummy message to all for message types. Not realistic
    // but makes the test simpler.
    writer->SaveEthMessage(*eth_frame, tick_time, msg);
    writer->SaveEthMessage(*eth_checksum_error, tick_time, msg);
    writer->SaveEthMessage(*eth_length_error, tick_time, msg);
    writer->SaveEthMessage(*eth_receive_error, tick_time, msg);

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
    CreateChannelObserverForDataGroup(*dg4, observer_list);
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 100'000);
    const auto& valid_list = observer->GetValidList();
    const bool all_valid =  std::ranges::all_of(valid_list, [] (const bool& valid) -> bool {
                                         return valid;
                                       });

    EXPECT_TRUE(all_valid) << observer->Name();

    // Verify that the CAN_RemoteFrame.DLC exist
    const auto name = observer->Name();
    if (!unique_list.contains(name)) {
      unique_list.emplace(name);
    } else if (name != "t")  {
      EXPECT_TRUE(false) << "Duplicate: " << name;
    }
  }
}

TEST_F(TestEthLogger, ErrorDetected) {
  EXPECT_FALSE(kErrorDetected);
}
} // mdf::test