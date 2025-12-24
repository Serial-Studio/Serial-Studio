/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testwrite.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <thread>
#include <set>
#include <random>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/isourceinformation.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/ievent.h"
#include "mdf/ifilehistory.h"

#include "mdf/mdffactory.h"
#include "mdf/mdfreader.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"

using namespace std::this_thread;
using namespace std::chrono_literals;
using namespace util::log;
using namespace util::time;
using namespace std::filesystem;
using namespace mdf;


namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/write";
bool kSkipTest = false;

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
void NoLog(const MdfLocation& location, mdf::MdfLogSeverity severity,
           const std::string& text) {
}



}  // namespace


namespace mdf::test {

void TestWrite::SetUpTestSuite() {

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to a file as this file is used as attachment file
    auto& log_config = LogConfig::Instance();
    log_config.RootDir(kTestRootDir);
    log_config.BaseName("mdf_write");
    log_config.Type(LogType::LogToFile);
    log_config.CreateDefaultLogger();

    remove(log_config.GetLogFile()); // Remove any old log files

    // Connect MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);


    // Add console logging
    log_config.AddLogger("Console",LogType::LogToConsole, {});
    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("write");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();


    LOG_TRACE() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;
    // The log file is actually not created directly. So wait until the log
    // file exist, otherwise the attachment test will fail.

    for (size_t log = 0; log < 600; ++log) {
      if (exists(log_config.GetLogFile()) ) {
        LOG_TRACE() << "Waited for log file. Ticks: " << log;
        break;
      }
      LOG_INFO() << "Fill with dummy messages for the attachment test.";
      std::this_thread::sleep_for(100ms);
    }

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestWrite::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();

}

TEST_F(TestWrite, Mdf3WriteTest1) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("test1.mf3");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Sample");
  header->Project("Mdf3WriteHD");
  header->StartTime(TimeStampToNs());
  header->Subject("PXY");

  auto* dg3 = writer->CreateDataGroup();
  auto* cg3 = mdf::MdfWriter::CreateChannelGroup(dg3);
  for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
    auto* cn3 = mdf::MdfWriter::CreateChannel(cg3);
    ASSERT_TRUE(cn3 != nullptr);
    std::ostringstream name;
    name << "Channel_" << cn_index + 1;
    cn3->Name(name.str());
    cn3->Description("Channel description");
    cn3->Type(cn_index == 0 ? ChannelType::Master : ChannelType::FixedLength);
    cn3->DataType(ChannelDataType::FloatBe);
    cn3->DataBytes(4);
    cn3->Unit("s");
  }

  writer->InitMeasurement();

  writer->StartMeasurement(TimeStampToNs());
  for (size_t sample = 0; sample < 100; ++sample) {
    auto cn_list = cg3->Channels();
    double value = 0.01 * static_cast<double>(sample);
    for(auto* channel : cn_list) {
      channel->SetChannelValue(value);
    }
    writer->SaveSample(*cg3, TimeStampToNs());
    sleep_for(10ms);
  }
  writer->StopMeasurement(TimeStampToNs());
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite, Mdf3WriteTestValueType) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("test_types.mf3");
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);
  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Value Types");
  header->Project("Mdf3WriteHD");
  header->StartTime(TimeStampToNs());
  header->Subject("PXY");

  auto* dg3 = writer->CreateDataGroup();
  auto* cg3 = mdf::MdfWriter::CreateChannelGroup(dg3);
  {
    auto* master = mdf::MdfWriter::CreateChannel(cg3);
    master->Name("Time");
    master->Description("Time channel");
    master->Type(ChannelType::Master);
    master->DataType(ChannelDataType::FloatLe);
    master->DataBytes(4);
    master->Unit("s");
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("UnsignedLe");
    channel->Description("uint32_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::UnsignedIntegerLe);
    channel->DataBytes(sizeof(uint32_t));
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("UnsignedBe");
    channel->Description("uint16_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::UnsignedIntegerBe);
    channel->DataBytes(sizeof(uint16_t));
  }

  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("SignedLe");
    channel->Description("int32_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::SignedIntegerLe);
    channel->DataBytes(sizeof(int32_t));
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("SignedBe");
    channel->Description("int8_t");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::SignedIntegerLe);
    channel->DataBytes(sizeof(int8_t));
  }

  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("FloatLe");
    channel->Description("float");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(sizeof(float));
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("FloatBe");
    channel->Description("double");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::FloatBe);
    channel->DataBytes(sizeof(double));
  }

  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("String");
    channel->Description("string");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::StringAscii);
    channel->DataBytes(10);
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("Array");
    channel->Description("vector");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::ByteArray);
    channel->DataBytes(5);
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("Date");
    channel->Description("CANopen Date");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::CanOpenDate);
    // channel->DataBytes(7);
  }
  {
    auto* channel = mdf::MdfWriter::CreateChannel(cg3);
    channel->Name("Time");
    channel->Description("CANopen Time");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::CanOpenTime);
    // channel->DataBytes(7);
  }

  writer->InitMeasurement();
  writer->StartMeasurement(TimeStampToNs());
  for (size_t sample = 0; sample < 100; ++sample) {
    auto cn_list = cg3->Channels();
    cn_list[0]->SetChannelValue(0.01 * static_cast<double>(sample));
    cn_list[1]->SetChannelValue(sample);
    cn_list[2]->SetChannelValue(sample);
    cn_list[3]->SetChannelValue(-static_cast<int>(sample));
    cn_list[4]->SetChannelValue(-static_cast<int>(sample));
    cn_list[5]->SetChannelValue(11.1 * static_cast<double>(sample));
    cn_list[6]->SetChannelValue(11.1 * static_cast<double>(sample));

    cn_list[7]->SetChannelValue(std::to_string(sample));
    std::vector<uint8_t> temp(5, 0);
    temp[0] = 'T';
    cn_list[8]->SetChannelValue(temp);

    auto ns70 = TimeStampToNs();
    cn_list[9]->SetChannelValue(ns70);
    cn_list[10]->SetChannelValue(ns70);

    writer->SaveSample(*cg3, ns70);
    sleep_for(10ms);
  }
  writer->StopMeasurement(TimeStampToNs());
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite, Mdf4WriteHD) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("hd4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));

  const auto start_time1 = TimeStampToNs();

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  EXPECT_EQ(header->Index(), 0);

  header->Author("Ingemar Hedvall");
  EXPECT_TRUE(header->Author() == "Ingemar Hedvall");

  header->Department("Home Alone");
  header->Description("Testing Header");
  header->Project("Mdf4WriteHD");
  header->StartTime(start_time1);
  header->Subject("PXY");
  header->MeasurementId("AAA");
  header->RecorderId("BBB");
  header->RecorderIndex(45);
  header->StartAngle(1.0);
  header->StartDistance(2.0);

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  EXPECT_EQ(header->Author(), header1->Author());
  std::cout << "Author: " << header1->Author() << std::endl;
  EXPECT_EQ(header->Department(), header1->Department());
  EXPECT_EQ(header->Description(), header1->Description());

  EXPECT_EQ(header->Project(), header1->Project());

  EXPECT_EQ(header->StartTime(), header1->StartTime()) << start_time1;
  EXPECT_EQ(header1->StartTime(), start_time1);

  EXPECT_EQ(header->Subject(), header1->Subject());
  EXPECT_EQ(header->MeasurementId(), header1->MeasurementId());
  EXPECT_EQ(header->RecorderId(), header1->RecorderId());
  EXPECT_EQ(header->RecorderIndex(), header1->RecorderIndex());

  EXPECT_EQ(header->StartAngle(), header1->StartAngle());
  EXPECT_TRUE(header1->StartAngle().has_value());

  EXPECT_EQ(header->StartDistance(), header1->StartDistance());
  EXPECT_TRUE(header1->StartDistance().has_value());
}

TEST_F(TestWrite, Mdf4WriteFH) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("fd4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");

  auto time_stamp = TimeStampToNs();
  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  EXPECT_EQ(history->Index(), 0);
  history->Time(time_stamp);
  history->Description("Initial stuff");
  history->ToolName("Unit Test");
  history->ToolVendor("ACME");
  history->ToolVersion("2.3");
  history->UserName("Ducky");
  {
    const auto* md4 = history->MetaData();
    ASSERT_TRUE(md4 != nullptr);
    std::cout << "Before: " << md4->XmlSnippet() << std::endl;
  }

  EXPECT_FALSE(history->Description().empty());

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto fh_list = header1->FileHistories();
  ASSERT_EQ(fh_list.size(), 1);

  const auto* history1 = fh_list[0];
  ASSERT_TRUE(history1 != nullptr);
  EXPECT_EQ(history->Time(), history1->Time());
  EXPECT_STREQ(history1->Description().c_str(), "Initial stuff");
  EXPECT_STREQ(history1->ToolName().c_str(), "Unit Test");
  EXPECT_STREQ(history1->ToolVendor().c_str(), "ACME");
  EXPECT_STREQ(history1->ToolVersion().c_str(), "2.3");
  EXPECT_STREQ(history1->UserName().c_str(), "Ducky");

  {
    const auto* md4 = history1->MetaData();
    ASSERT_TRUE(md4 != nullptr);
    std::cout << "After: " << md4->XmlSnippet() << std::endl;
  }
}

TEST_F(TestWrite, Mdf4WriteAT) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("at4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  const auto& log_config = LogConfig::Instance();
  const auto log_file = log_config.GetLogFile();


  for (uint16_t count = 0; count < 3; ++count) {
    auto* attachment = header->CreateAttachment();
    ASSERT_TRUE(attachment != nullptr);
    EXPECT_EQ(attachment->Index(), 0);
    attachment->CreatorIndex(count);
    EXPECT_EQ(attachment->CreatorIndex(), count);

    attachment->IsEmbedded(count > 0);
    attachment->IsCompressed(count > 1);
    attachment->FileName(log_file);
    attachment->FileType("text/plain");
    const auto md5 = attachment->Md5();
    EXPECT_FALSE(md5.has_value());
  }

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadMeasurementInfo());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto at_list = header1->Attachments();
  ASSERT_EQ(at_list.size(), 3);

  for (size_t count1 = 0; count1 < at_list.size(); ++count1) {
    const auto* attachment = at_list[count1];
    ASSERT_TRUE(attachment != nullptr);
    EXPECT_EQ(attachment->CreatorIndex(), count1);
    EXPECT_EQ(attachment->IsEmbedded(), count1 > 0);
    EXPECT_EQ(attachment->IsCompressed(), count1 > 1);
    EXPECT_EQ(attachment->FileName(), log_file);
    EXPECT_STREQ(attachment->FileType().c_str(), "text/plain");
    EXPECT_TRUE(attachment->Md5().has_value());
    EXPECT_EQ(attachment->Md5().value().size(), 32);
  }
}

TEST_F(TestWrite, Mdf4WriteEV) {  // NOLINT
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("ev4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* event = header->CreateEvent();
  ASSERT_TRUE(event != nullptr);
  EXPECT_EQ(event->Index(), 0);
  event->GroupName("Olle");
  event->Description("Olle Event");
  EXPECT_STREQ(event->Description().c_str(), "Olle Event");

  writer->InitMeasurement();
  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);
  EXPECT_GT(header1->Index(), 0);

  const auto ev_list = header1->Events();
  ASSERT_EQ(ev_list.size(), 1);
}

TEST_F(TestWrite, Mdf4WriteDG_CG_SI) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("dg4.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    ASSERT_TRUE(writer->Init(mdf_file.string()));
    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);

    auto* history = header->CreateFileHistory();
    ASSERT_TRUE(history != nullptr);
    history->Description("Created");
    history->ToolName("MdfWrite");
    history->ToolVendor("ACME Road Runner Company");
    history->ToolVersion("1.0");
    history->UserName("Ingemar Hedvall");

    auto* data_group = header->CreateDataGroup();
    ASSERT_TRUE(data_group != nullptr);
    EXPECT_EQ(data_group->Index(), 0);
    data_group->Description("DG-Desc");
    EXPECT_STREQ(data_group->Description().c_str(), "DG-Desc");
    EXPECT_EQ(data_group->RecordIdSize(),0);
    EXPECT_FALSE(data_group->IsRead());

    // Test CG4 block
    auto* channel_group = data_group->CreateChannelGroup();
    ASSERT_TRUE(channel_group != nullptr);
    EXPECT_EQ(channel_group->Index(), 0);
    EXPECT_STREQ(channel_group->BlockType().c_str(), "CG");

    channel_group->Name("Group1");
    EXPECT_STREQ(channel_group->Name().c_str(), "Group1");
    channel_group->Description("Group1 desc");
    EXPECT_STREQ(channel_group->Description().c_str(), "Group1 desc");

    // Test SI4 block
    auto* source_info = channel_group->CreateSourceInformation();
    ASSERT_TRUE(source_info != nullptr);
    EXPECT_EQ(source_info->Index(), 0);
    EXPECT_STREQ(source_info->BlockType().c_str(), "SI");

    source_info->Name("SI-Name");
    EXPECT_STREQ(source_info->Name().c_str(), "SI-Name");

    source_info->Path("SI-Path");
    EXPECT_STREQ(source_info->Path().c_str(), "SI-Path");

    source_info->Description("SI-Desc");
    EXPECT_STREQ(source_info->Description().c_str(), "SI-Desc");

    source_info->Type(SourceType::Bus);
    EXPECT_EQ(source_info->Type(), SourceType::Bus);

    source_info->Bus(BusType::Can);
    EXPECT_EQ(source_info->Bus(), BusType::Can);

    writer->InitMeasurement();
    writer->FinalizeMeasurement();

    EXPECT_GT(header->Index(), 0);

  }
  {
    MdfReader reader(mdf_file.string());
    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData());
    const auto* file1 = reader.GetFile();
    ASSERT_TRUE(file1 != nullptr);

    const auto* header1 = file1->Header();
    ASSERT_TRUE(header1 != nullptr);
    EXPECT_GT(header1->Index(), 0);

    const auto dg_list = header1->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);
    for (auto* dg4 : dg_list) {
      ASSERT_TRUE(dg4 != nullptr);
      EXPECT_GT(dg4->Index(),0);
      EXPECT_STREQ(dg4->BlockType().c_str(), "DG");
      EXPECT_STREQ(dg4->Description().c_str(), "DG-Desc");
      EXPECT_EQ(dg4->RecordIdSize(), 0);
      EXPECT_TRUE(dg4->MetaData() != nullptr); // Description in MD block
      EXPECT_FALSE(dg4->IsRead());  // Check if data is read

      const auto cg_list = dg4->ChannelGroups();
      EXPECT_EQ(cg_list.size(), 1);
      for (auto* cg4 : cg_list) {
        ASSERT_TRUE(cg4 != nullptr);
        EXPECT_GT(cg4->Index(), 0);
        EXPECT_STREQ(cg4->BlockType().c_str(),"CG");
        EXPECT_STREQ(cg4->Name().c_str(), "Group1");
        EXPECT_STREQ(cg4->Description().c_str(), "Group1 desc");
        EXPECT_TRUE(cg4->MetaData() != nullptr); // Description in MD block

        const auto* si4 = cg4->SourceInformation();
        ASSERT_TRUE(si4 != nullptr);
        EXPECT_GT(si4->Index(), 0);
        EXPECT_STREQ(si4->BlockType().c_str(),"SI");
        EXPECT_STREQ(si4->Name().c_str(), "SI-Name");
        EXPECT_STREQ(si4->Path().c_str(), "SI-Path");
        EXPECT_STREQ(si4->Description().c_str(), "SI-Desc"); // Desc in MD block
        EXPECT_EQ(si4->Type(), SourceType::Bus);
        EXPECT_EQ(si4->Bus(), BusType::Can);
        EXPECT_EQ(si4->Flags(), 0);
        EXPECT_TRUE(si4->MetaData() != nullptr); // Desc in MD block

      }
    }
  }
}

TEST_F(TestWrite, Mdf4WriteCN_SI_CC) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("cn4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  ASSERT_TRUE(data_group != nullptr);

  // Test CG4 block
  auto* channel_group = data_group->CreateChannelGroup();
  ASSERT_TRUE(channel_group != nullptr);
  channel_group->Name("Group1");

  // Add channel
  auto* ch1 = channel_group->CreateChannel();
  ASSERT_TRUE(ch1 != nullptr);
  EXPECT_EQ(ch1->Index(),0);
  EXPECT_STREQ(ch1->BlockType().c_str(), "CN");
  ch1->Name("Ch1-Name");
  ch1->DisplayName("Ch1-DisplayName");
  ch1->Description("Ch1-Desc");
  ch1->Unit("Ch1-Unit");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::UnsignedIntegerLe);
  ch1->DataBytes(8);

  ch1->Range(0,10);
  ch1->Limit(1,9);
  ch1->ExtLimit(2,8);
  // ch1->SamplingRate(1.33); Not supported in MDF4

  auto* source_info = ch1->CreateSourceInformation();
  ASSERT_TRUE(source_info != nullptr);
  source_info->Name("SI-Name");
  source_info->Path("SI-Path");
  source_info->Description("SI-Desc");
  source_info->Type(SourceType::Bus);
  source_info->Bus(BusType::Can);

  auto* conv = ch1->CreateChannelConversion();
  ASSERT_TRUE(conv != nullptr);
  EXPECT_EQ(conv->Index(),0);
  EXPECT_STREQ(conv->BlockType().c_str(), "CC");
  conv->Name("Cc1-Name");
  conv->Description("Cc1-Desc");
  conv->Unit("Cc1-Unit");
  conv->Type(ConversionType::Linear);
  conv->Decimals(3);
  conv->Parameter(0, 0.0);
  conv->Parameter(1,1.0);
  conv->Range(0,10);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  uint64_t value;
  for (size_t sample = 0; sample < 10; ++sample) {
    value = sample;
    ch1->SetChannelValue(value);
    writer->SaveSample(*channel_group,tick_time);
    tick_time += 1'000'000'000;
  }
  writer->StopMeasurement(tick_time);

  writer->FinalizeMeasurement();

  EXPECT_GT(header->Index(), 0);

  MdfReader reader(mdf_file.string());
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);

  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);
  for (auto* dg4 : dg_list) {
    ASSERT_TRUE(dg4 != nullptr);
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      ASSERT_TRUE(cg4 != nullptr);
      EXPECT_EQ(cg4->NofSamples(),10);
      EXPECT_EQ(cg4->RecordId(),0);

      const auto cn_list = cg4->Channels();
      EXPECT_EQ(cn_list.size(),1);
      for (auto* cn4 : cn_list) {
        ASSERT_TRUE(cn4 != nullptr);
        EXPECT_GT(cn4->Index(),0);
        EXPECT_STREQ(cn4->BlockType().c_str(), "CN");
        EXPECT_STREQ(cn4->Name().c_str(), "Ch1-Name");
        EXPECT_STREQ(cn4->DisplayName().c_str(), "");
        EXPECT_STREQ(cn4->Description().c_str(), "Ch1-Desc");
        EXPECT_STREQ(cn4->Unit().c_str(), "Ch1-Unit");
        EXPECT_TRUE(cn4->IsUnitValid());
        EXPECT_EQ(cn4->Type(),ChannelType::FixedLength);
        EXPECT_EQ(cn4->Sync(),ChannelSyncType::None);
        EXPECT_EQ(cn4->DataType(),ChannelDataType::UnsignedIntegerLe);
        EXPECT_EQ(cn4->DataBytes(), 8);
        // EXPECT_DOUBLE_EQ(cn4->SamplingRate(), 1.33); Not supported in MDF4

        EXPECT_TRUE(cn4->Flags() & CnFlag::RangeValid);
        const auto range = cn4->Range();
        EXPECT_TRUE(range.has_value());
        EXPECT_DOUBLE_EQ(range.value().first, 0);
        EXPECT_DOUBLE_EQ(range.value().second, 10);

        EXPECT_TRUE(cn4->Flags() & CnFlag::LimitValid);
        const auto limit = cn4->Limit();
        EXPECT_TRUE(limit.has_value());
        EXPECT_DOUBLE_EQ(limit.value().first, 1);
        EXPECT_DOUBLE_EQ(limit.value().second, 9);

        EXPECT_TRUE(cn4->Flags() & CnFlag::ExtendedLimitValid);
        const auto ext_limit = cn4->ExtLimit();
        EXPECT_TRUE(ext_limit.has_value());
        EXPECT_DOUBLE_EQ(ext_limit.value().first, 2);
        EXPECT_DOUBLE_EQ(ext_limit.value().second, 8);

        const auto* si4 = cn4->SourceInformation();
        ASSERT_TRUE(si4 != nullptr);
        EXPECT_GT(si4->Index(), 0);
        EXPECT_STREQ(si4->BlockType().c_str(),"SI");
        EXPECT_STREQ(si4->Name().c_str(), "SI-Name");
        EXPECT_STREQ(si4->Path().c_str(), "SI-Path");
        EXPECT_STREQ(si4->Description().c_str(), "SI-Desc"); // Desc in MD block
        EXPECT_EQ(si4->Type(), SourceType::Bus);
        EXPECT_EQ(si4->Bus(), BusType::Can);
        EXPECT_EQ(si4->Flags(), 0);
        EXPECT_TRUE(si4->MetaData() != nullptr); // Desc in MD block

        const auto* cc4 = ch1->ChannelConversion();
        ASSERT_TRUE(cc4 != nullptr);
        EXPECT_GT(cc4->Index(),0);
        EXPECT_STREQ(cc4->BlockType().c_str(), "CC");
        EXPECT_STREQ(cc4->Name().c_str(),"Cc1-Name");
        EXPECT_STREQ(cc4->Description().c_str(),"Cc1-Desc");
        EXPECT_STREQ(cc4->Unit().c_str(),"Cc1-Unit");
        EXPECT_EQ(cc4->Type(),ConversionType::Linear);

        EXPECT_TRUE(cc4->Flags() & CcFlag::PrecisionValid);
        EXPECT_EQ(cc4->Decimals(),3);

        EXPECT_TRUE(cc4->Flags() & CcFlag::RangeValid);
        const auto range1 = cc4->Range();
        EXPECT_TRUE(range1.has_value());
        EXPECT_DOUBLE_EQ(range1.value().first, 0);
        EXPECT_DOUBLE_EQ(range1.value().second, 10);

        EXPECT_TRUE(cc4->Flags() & CcFlag::PrecisionValid);
      }
    }
  }
}

TEST_F(TestWrite, Mdf4Unsigned) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("unsigned.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Unsigned");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel8");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::UnsignedIntegerLe);
  ch1->DataBytes(1);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("Intel16");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::UnsignedIntegerLe);
  ch2->DataBytes(2);

  auto* ch3 = group1->CreateChannel();
  ch3->Name("Intel32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::UnsignedIntegerLe);
  ch3->DataBytes(4);

  auto* ch4 = group1->CreateChannel();
  ch4->Name("Intel64");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::UnsignedIntegerLe);
  ch4->DataBytes(8);

  auto* ch5 = group1->CreateChannel();
  ch5->Name("Motorola8");
  ch5->Type(ChannelType::FixedLength);
  ch5->Sync(ChannelSyncType::None);
  ch5->DataType(ChannelDataType::UnsignedIntegerBe);
  ch5->DataBytes(1);

  auto* ch6 = group1->CreateChannel();
  ch6->Name("Motorola16");
  ch6->Type(ChannelType::FixedLength);
  ch6->Sync(ChannelSyncType::None);
  ch6->DataType(ChannelDataType::UnsignedIntegerBe);
  ch6->DataBytes(2);

  auto* ch7 = group1->CreateChannel();
  ch7->Name("Motorola32");
  ch7->Type(ChannelType::FixedLength);
  ch7->Sync(ChannelSyncType::None);
  ch7->DataType(ChannelDataType::UnsignedIntegerBe);
  ch7->DataBytes(4);

  auto* ch8 = group1->CreateChannel();
  ch8->Name("Motorola64");
  ch8->Type(ChannelType::FixedLength);
  ch8->Sync(ChannelSyncType::None);
  ch8->DataType(ChannelDataType::UnsignedIntegerBe);
  ch8->DataBytes(8);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  uint64_t value;
  for (size_t sample = 0; sample < 100; ++sample) {
    value = static_cast<uint64_t>(sample);
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);
    ch5->SetChannelValue(value);
    ch6->SetChannelValue(value);
    ch7->SetChannelValue(value);
    ch8->SetChannelValue(value);
    writer->SaveSample(*group1,tick_time);
    tick_time += 1'000'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      uint64_t channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid) << "Sample: " << sample << ", Name: " << observer->Name();
      EXPECT_EQ(channel_value, static_cast<uint64_t>(sample))
          << observer->Name();
    }
  }

}

TEST_F(TestWrite, Mdf4Signed) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("signed.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Signed");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel8");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::SignedIntegerLe);
  ch1->DataBytes(1);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("Intel16");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::SignedIntegerLe);
  ch2->DataBytes(2);

  auto* ch3 = group1->CreateChannel();
  ch3->Name("Intel32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::SignedIntegerLe);
  ch3->DataBytes(4);

  auto* ch4 = group1->CreateChannel();
  ch4->Name("Intel64");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::SignedIntegerLe);
  ch4->DataBytes(8);

  auto* ch5 = group1->CreateChannel();
  ch5->Name("Motorola8");
  ch5->Type(ChannelType::FixedLength);
  ch5->Sync(ChannelSyncType::None);
  ch5->DataType(ChannelDataType::SignedIntegerBe);
  ch5->DataBytes(1);

  auto* ch6 = group1->CreateChannel();
  ch6->Name("Motorola16");
  ch6->Type(ChannelType::FixedLength);
  ch6->Sync(ChannelSyncType::None);
  ch6->DataType(ChannelDataType::SignedIntegerBe);
  ch6->DataBytes(2);

  auto* ch7 = group1->CreateChannel();
  ch7->Name("Motorola32");
  ch7->Type(ChannelType::FixedLength);
  ch7->Sync(ChannelSyncType::None);
  ch7->DataType(ChannelDataType::SignedIntegerBe);
  ch7->DataBytes(4);

  auto* ch8 = group1->CreateChannel();
  ch8->Name("Motorola64");
  ch8->Type(ChannelType::FixedLength);
  ch8->Sync(ChannelSyncType::None);
  ch8->DataType(ChannelDataType::SignedIntegerBe);
  ch8->DataBytes(8);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  int64_t value;
  for (int64_t sample = -99; sample < 100; ++sample) {
    value = static_cast<int64_t>(sample);
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);
    ch5->SetChannelValue(value);
    ch6->SetChannelValue(value);
    ch7->SetChannelValue(value);
    ch8->SetChannelValue(value);
    writer->SaveSample(*group1,tick_time);
    tick_time += 1'000'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 199);
    for (int64_t sample = -99; sample < 100; ++sample) {
      int64_t channel_value = 0;
      const auto valid = observer->GetChannelValue(sample + 99, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_EQ(channel_value, static_cast<int64_t>(sample))
          << observer->Name();
    }
  }
}

TEST_F(TestWrite, Mdf4Float) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("float.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Float");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel32");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::FloatLe);
  ch1->DataBytes(4);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("Intel64");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::FloatLe);
  ch2->DataBytes(8);

  auto* ch3 = group1->CreateChannel();
  ch3->Name("Motorola32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::FloatBe);
  ch3->DataBytes(4);

  auto* ch4 = group1->CreateChannel();
  ch4->Name("Motorola64");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::FloatBe);
  ch4->DataBytes(8);


  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 100; ++sample) {
    double value = static_cast<double>(sample) + 0.23;
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);

    writer->SaveSample(*group1,tick_time);
    tick_time += 1'000'000'000;
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
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
      const auto channel_list = cg4->Channels();
      EXPECT_EQ(channel_list.size(), 4);
      for (size_t index = 0; index < channel_list.size(); ++index) {
        const auto* channel = channel_list[index];
        EXPECT_TRUE(channel != nullptr);
        switch (index) {
          case 0:
            EXPECT_EQ(channel->DataBytes(), 4) << channel->Name();
            EXPECT_EQ(channel->DataType(), ChannelDataType::FloatLe);
            break;

          case 1:
            EXPECT_EQ(channel->DataBytes(), 8) << channel->Name();
            EXPECT_EQ(channel->DataType(), ChannelDataType::FloatLe);
            break;

          case 2:
            EXPECT_EQ(channel->DataBytes(), 4) << channel->Name();
            EXPECT_EQ(channel->DataType(), ChannelDataType::FloatBe);
            break;

          case 3:
            EXPECT_EQ(channel->DataBytes(), 8) << channel->Name();
            EXPECT_EQ(channel->DataType(), ChannelDataType::FloatBe);
            break;
          default:
            break;
        }
      }

    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      float channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_FLOAT_EQ(channel_value, static_cast<float>(sample + 0.23F))
          << observer->Name();
    }
  }
}
TEST_F(TestWrite, Mdf4TimeAndDate) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("time_and_date.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("TimeAndDate");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("CanDate");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::CanOpenDate);
  ch1->DataBytes(7);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("CanTime");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::CanOpenTime);
  ch2->DataBytes(6);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  const auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  auto temp_time = tick_time;

  for (size_t sample = 0; sample < 100; ++sample) {
    double value = static_cast<double>(sample) + 0.23;
    ch1->SetChannelValue(temp_time);
    ch2->SetChannelValue(temp_time);
    writer->SaveSample(*group1,temp_time);
    temp_time += 1'000'000'000;
  }
  writer->StopMeasurement(temp_time);
  writer->FinalizeMeasurement();


  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (uint64_t sample = 0; sample < 100; ++sample) {
      auto temp1_time = tick_time + (sample * 1'000'000'000);
      temp1_time /= 1'000'000; // Only ms resolution in storage
      temp1_time *= 1'000'000;
      uint64_t channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_EQ(channel_value, temp1_time) << observer->Name();
    }
  }
}

TEST_F(TestWrite, CompressData) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("compress.mf4");


  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  writer->CompressData(true);

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test compress");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Float");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel32");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::FloatLe);
  ch1->DataBytes(4);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("Intel64");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::FloatLe);
  ch2->DataBytes(8);

  auto* ch3 = group1->CreateChannel();
  ch3->Name("Motorola32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::FloatBe);
  ch3->DataBytes(4);

  auto* ch4 = group1->CreateChannel();
  ch4->Name("Motorola64");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::FloatBe);
  ch4->DataBytes(8);


  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 1'000'000; ++sample) {
    double value = static_cast<double>(sample) + 0.23;
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);

    writer->SaveSample(*group1,tick_time);
    tick_time += 100'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 1'000'000);
    for (size_t sample = 0; sample < 100; ++sample) {
      float channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_FLOAT_EQ(channel_value, static_cast<float>(sample + 0.23))
          << observer->Name();
    }
  }
}

TEST_F(TestWrite, CompressArrayData) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t nof_samples = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("compress_array.mf4");


  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  writer->CompressData(true);

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test compress");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Float");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("ByteArray");
  ch1->Type(ChannelType::VariableLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::ByteArray);
  ch1->DataBytes(8);

  const std::vector<uint8_t> data_array(32,0xFE);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < nof_samples; ++sample) {
    ch1->SetChannelValue(data_array, true);

    writer->SaveSample(*group1,tick_time);
    tick_time += 100'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), nof_samples);
    for (size_t sample = 0; sample < nof_samples; ++sample) {
      std::vector<uint8_t> channel_value;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_EQ(channel_value, data_array) << observer->Name();
    }
  }
}

TEST_F(TestWrite, StringData) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("string.mf4");


  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  writer->CompressData(false);

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test string");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("String");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("StringASCII");
  ch1->Type(ChannelType::VariableLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::StringAscii);
  ch1->DataBytes(8);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("StringUtf8");
  ch2->Type(ChannelType::VariableLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::StringUTF8);
  ch2->DataBytes(4);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 1'000; ++sample) {
    std::ostringstream value;
    value << "String "  << sample;
    ch1->SetChannelValue(value.str());
    ch2->SetChannelValue(value.str());

    writer->SaveSample(*group1,tick_time);
    tick_time += 100'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 1'000);
    for (size_t sample = 0; sample < 100; ++sample) {
      std::string channel_value;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      std::ostringstream temp;
      temp << "String "  << sample;
      EXPECT_EQ(channel_value, temp.str()) << observer->Name();
    }
  }
}

TEST_F(TestWrite, FixedStringData) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("fixed_string.mf4");


  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  writer->CompressData(false);

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test string");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("String");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("StringUTF8");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::StringAscii);
  ch1->DataBytes(20);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("StringAscii");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::StringUTF8);
  ch2->DataBytes(20);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 100; ++sample) {
    std::ostringstream value;
    value << "String "  << sample;
    ch1->SetChannelValue(value.str());
    ch2->SetChannelValue(value.str());

    writer->SaveSample(*group1,tick_time);
    tick_time += 100'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      std::string channel_value;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      std::ostringstream temp;
      temp << "String "  << sample;
      EXPECT_EQ(channel_value, temp.str()) << observer->Name();
    }
  }
}


TEST_F(TestWrite, Mdf4Invalid) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("invalid.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Float");


  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel32");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::FloatLe);
  ch1->Flags(CnFlag::InvalidValid);
  ch1->DataBytes(4);

  auto* ch2 = group1->CreateChannel();
  ch2->Name("Intel64");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::FloatLe);
  ch2->Flags(CnFlag::InvalidValid);
  ch2->DataBytes(8);

  auto* ch3 = group1->CreateChannel();
  ch3->Name("Motorola32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::FloatBe);
  ch3->Flags(CnFlag::InvalidValid);
  ch3->DataBytes(4);

  auto* ch4 = group1->CreateChannel();
  ch4->Name("Motorola64");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::FloatBe);
  ch4->Flags(CnFlag::InvalidValid);
  ch4->DataBytes(8);

  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 100; ++sample) {
    double value = static_cast<double>(sample) + 0.23;
    bool valid = (sample % 2) == 0; // Even samples are valid
    ch1->SetChannelValue(value, valid);
    ch2->SetChannelValue(value, valid);
    ch3->SetChannelValue(value, valid);
    ch4->SetChannelValue(value, valid);

    writer->SaveSample(*group1,tick_time);
    tick_time += 1'000'000'000;
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
  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 1);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      float channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      if ((sample % 2) == 0) {
        EXPECT_TRUE(valid);
      } else {
        EXPECT_FALSE(valid);
      }
      EXPECT_FLOAT_EQ(channel_value, static_cast<float>(sample + 0.23))
          << observer->Name();
    }
  }
}

TEST_F(TestWrite, Mdf4Multi) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("multi.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");
  { // Measurement 1
    auto* data_group = header->CreateDataGroup();
    auto* group1 = data_group->CreateChannelGroup();
    group1->Name("Intel");

    auto* ch1 = group1->CreateChannel();
    ch1->Name("Intel32");
    ch1->Type(ChannelType::FixedLength);
    ch1->Sync(ChannelSyncType::None);
    ch1->DataType(ChannelDataType::FloatLe);
    ch1->DataBytes(4);

    auto* ch2 = group1->CreateChannel();
    ch2->Name("Intel64");
    ch2->Type(ChannelType::FixedLength);
    ch2->Sync(ChannelSyncType::None);
    ch2->DataType(ChannelDataType::FloatLe);
    ch2->DataBytes(8);

    auto* group2 = data_group->CreateChannelGroup();
    group2->Name("Motorola");

    auto* ch3 = group2->CreateChannel();
    ch3->Name("Motorola32");
    ch3->Type(ChannelType::FixedLength);
    ch3->Sync(ChannelSyncType::None);
    ch3->DataType(ChannelDataType::FloatBe);
    ch3->DataBytes(4);

    auto* ch4 = group2->CreateChannel();
    ch4->Name("Motorola64");
    ch4->Type(ChannelType::FixedLength);
    ch4->Sync(ChannelSyncType::None);
    ch4->DataType(ChannelDataType::FloatBe);
    ch4->DataBytes(8);

    writer->PreTrigTime(0);
    writer->InitMeasurement();

    auto tick_time = TimeStampToNs();
    writer->StartMeasurement(tick_time);

    for (size_t sample = 0; sample < 100; ++sample) {
      auto value = static_cast<double>(sample) + 0.23;
      ch1->SetChannelValue(value);
      ch2->SetChannelValue(value);
      ch3->SetChannelValue(value);
      ch4->SetChannelValue(value);

      writer->SaveSample(*group1, tick_time);
      writer->SaveSample(*group2, tick_time);
      tick_time += 2'000'000'000;
    }
    writer->StopMeasurement(tick_time);
    writer->FinalizeMeasurement();
  }
  { // Measurement 2
    auto* data_group = header->CreateDataGroup();
    auto* group1 = data_group->CreateChannelGroup();
    group1->Name("Intel");

    auto* ch1 = group1->CreateChannel();
    ch1->Name("Intel32");
    ch1->Type(ChannelType::FixedLength);
    ch1->Sync(ChannelSyncType::None);
    ch1->DataType(ChannelDataType::FloatLe);
    ch1->DataBytes(4);

    auto* ch2 = group1->CreateChannel();
    ch2->Name("Intel64");
    ch2->Type(ChannelType::FixedLength);
    ch2->Sync(ChannelSyncType::None);
    ch2->DataType(ChannelDataType::FloatLe);
    ch2->DataBytes(8);

    auto* group2 = data_group->CreateChannelGroup();
    group2->Name("Motorola");

    auto* ch3 = group2->CreateChannel();
    ch3->Name("Motorola32");
    ch3->Type(ChannelType::FixedLength);
    ch3->Sync(ChannelSyncType::None);
    ch3->DataType(ChannelDataType::FloatBe);
    ch3->DataBytes(4);

    auto* ch4 = group2->CreateChannel();
    ch4->Name("Motorola64");
    ch4->Type(ChannelType::FixedLength);
    ch4->Sync(ChannelSyncType::None);
    ch4->DataType(ChannelDataType::FloatBe);
    ch4->DataBytes(8);

    writer->PreTrigTime(0);
    writer->InitMeasurement();

    auto tick_time = TimeStampToNs();
    writer->StartMeasurement(tick_time);

    for (size_t sample = 0; sample < 100; ++sample) {
      auto value = static_cast<double>(sample) + 0.23;
      ch1->SetChannelValue(value);
      ch2->SetChannelValue(value);
      ch3->SetChannelValue(value);
      ch4->SetChannelValue(value);

      writer->SaveSample(*group1, tick_time);
      writer->SaveSample(*group2, tick_time);
      tick_time += 1'000'000'000;
    }
    writer->StopMeasurement(tick_time);
    writer->FinalizeMeasurement();
  }

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 2);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 2);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }

    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    ASSERT_EQ(observer->NofSamples(), 100);
    for (size_t sample = 0; sample < 100; ++sample) {
      float channel_value = 0;
      const auto valid = observer->GetChannelValue(sample, channel_value);
      EXPECT_TRUE(valid);
      EXPECT_FLOAT_EQ(channel_value, static_cast<float>(sample) + 0.23F)
          << observer->Name();
    }
  }
}

TEST_F(TestWrite, Mdf4Master) {
  if (kSkipTest) {
    GTEST_SKIP();
  }

  path mdf_file(kTestDir);
  mdf_file.append("master.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();

  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("FloatMaster");

  auto* master1 = group1->CreateChannel();
  master1->Name("Time");
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->DataType(ChannelDataType::FloatLe);
  master1->DataBytes(4);

  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel32");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::FloatLe);
  ch1->DataBytes(4);

  auto* group2 = data_group->CreateChannelGroup();
  group1->Name("DoubleMaster");

  auto* master2 = group2->CreateChannel();
  master2->Name("Time");
  master2->Type(ChannelType::Master);
  master2->Sync(ChannelSyncType::Time);
  master2->DataType(ChannelDataType::FloatBe);
  master2->DataBytes(8);

  auto* ch2 = group2->CreateChannel();
  ch2->Name("Intel32");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::FloatLe);
  ch2->DataBytes(4);

  auto* group3 = data_group->CreateChannelGroup();
  group3->Name("Signed32Master");

  auto* master3 = group3->CreateChannel();
  master3->Name("Time");
  master3->Unit("s");
  master3->Type(ChannelType::Master);
  master3->Sync(ChannelSyncType::Time);
  master3->DataType(ChannelDataType::SignedIntegerLe);
  master3->DataBytes(4);

  auto cc3 = master3->CreateChannelConversion();
  cc3->Name("NsToS");
  cc3->Description("Nanoseconds to seconds conversion");
  cc3->Unit("s");
  cc3->Type(ConversionType::Linear);
  cc3->Parameter(0,0.0);
  cc3->Parameter(1, 1E-9);

  auto* ch3 = group3->CreateChannel();
  ch3->Name("Intel32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::FloatLe);
  ch3->DataBytes(4);

  auto* group4 = data_group->CreateChannelGroup();
  group4->Name("Signed64Master");

  auto* master4 = group4->CreateChannel();
  master4->Name("Time");
  master4->Unit("s");
  master4->Type(ChannelType::Master);
  master4->Sync(ChannelSyncType::Time);
  master4->DataType(ChannelDataType::SignedIntegerLe);
  master4->DataBytes(8);

  auto cc4 = master4->CreateChannelConversion();
  cc4->Name("NsToS");
  cc4->Description("Nanoseconds to seconds conversion");
  cc4->Unit("s");
  cc4->Type(ConversionType::Linear);
  cc4->Parameter(0,0.0);
  cc4->Parameter(1, 1E-9);

  auto* ch4 = group4->CreateChannel();
  ch4->Name("Intel32");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::FloatLe);
  ch4->DataBytes(4);

  writer->PreTrigTime(2.0); // Store one seconds samples before start
  writer->InitMeasurement(); // Start the internal cache
  auto tick_time = TimeStampToNs();

  // Fill cache with 1 second data
  uint64_t sample = 0;
  for (; sample < 1000; ++sample) {
    const auto value = static_cast<uint32_t>(sample);
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);
    writer->SaveSample(*group1, tick_time);
    writer->SaveSample(*group2, tick_time);
    writer->SaveSample(*group3, tick_time);
    writer->SaveSample(*group4, tick_time);
    tick_time += 2'000'000; // Stepping 1ms
  }

  writer->StartMeasurement(tick_time);

  // Fill with 1 seconds samples after start.
  for (; sample < 2000; ++sample) {
    const auto value = static_cast<uint32_t>(sample);
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);
    writer->SaveSample(*group1, tick_time);
    writer->SaveSample(*group2, tick_time);
    writer->SaveSample(*group3, tick_time);
    writer->SaveSample(*group4, tick_time);
    tick_time += 2'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MDF_TRACE() << "Written MDF File: " <<  writer->Name();

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
    ASSERT_EQ(observer_list.size(), 8) << "Observer List Size: " << observer_list.size();
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    const auto nof_samples = observer->NofSamples();
    EXPECT_EQ(nof_samples, 2000);
    if (observer->IsMaster()) {
      // Check that the times are stepping forward
      double previous = 0.0;
      for (sample = 0 ; sample < nof_samples; ++sample) {
        double time = 0.0;
        observer->GetEngValue(sample, time, 0);
        if (sample > 0) {
          EXPECT_GT(time, previous) << "Time/Previous: " << time << "/" << previous;
        }
        if (sample < 1000) {
          EXPECT_LE(time, 0.0) << "Time/Sample: " << time << "/" << sample;
        }
        previous = time;
      }
    }
  }
}

TEST_F(TestWrite, Mdf4Master2s) {
  if (kSkipTest) {
    GTEST_SKIP();
  }

  path mdf_file(kTestDir);
  mdf_file.append("master_2s.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test data types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();

  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("FloatMaster");

  auto* master1 = group1->CreateChannel();
  master1->Name("Time");
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->DataType(ChannelDataType::FloatLe);
  master1->DataBytes(4);

  auto* ch1 = group1->CreateChannel();
  ch1->Name("Intel32");
  ch1->Type(ChannelType::FixedLength);
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::FloatLe);
  ch1->DataBytes(4);

  auto* group2 = data_group->CreateChannelGroup();
  group1->Name("DoubleMaster");

  auto* master2 = group2->CreateChannel();
  master2->Name("Time");
  master2->Type(ChannelType::Master);
  master2->Sync(ChannelSyncType::Time);
  master2->DataType(ChannelDataType::FloatBe);
  master2->DataBytes(8);

  auto* ch2 = group2->CreateChannel();
  ch2->Name("Intel32");
  ch2->Type(ChannelType::FixedLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::FloatLe);
  ch2->DataBytes(4);

  auto* group3 = data_group->CreateChannelGroup();
  group3->Name("Signed64Master");

  auto* master3 = group3->CreateChannel();
  master3->Name("Time");
  master3->Unit("s");
  master3->Type(ChannelType::Master);
  master3->Sync(ChannelSyncType::Time);
  master3->DataType(ChannelDataType::SignedIntegerLe);
  master3->DataBytes(8);

  auto cc3 = master3->CreateChannelConversion();
  cc3->Name("NsToS");
  cc3->Description("Nanoseconds to seconds conversion");
  cc3->Unit("s");
  cc3->Type(ConversionType::Linear);
  cc3->Parameter(0,0.0);
  cc3->Parameter(1, 1E-9);

  auto* ch3 = group3->CreateChannel();
  ch3->Name("Intel32");
  ch3->Type(ChannelType::FixedLength);
  ch3->Sync(ChannelSyncType::None);
  ch3->DataType(ChannelDataType::FloatLe);
  ch3->DataBytes(4);

  auto* group4 = data_group->CreateChannelGroup();
  group4->Name("Signed64Master");

  auto* master4 = group4->CreateChannel();
  master4->Name("Time");
  master4->Unit("s");
  master4->Type(ChannelType::Master);
  master4->Sync(ChannelSyncType::Time);
  master4->DataType(ChannelDataType::SignedIntegerLe);
  master4->DataBytes(8);

  auto cc4 = master4->CreateChannelConversion();
  cc4->Name("NsToS");
  cc4->Description("Nanoseconds to seconds conversion");
  cc4->Unit("s");
  cc4->Type(ConversionType::Linear);
  cc4->Parameter(0,0.0);
  cc4->Parameter(1, 1E-9);

  auto* ch4 = group4->CreateChannel();
  ch4->Name("Intel32");
  ch4->Type(ChannelType::FixedLength);
  ch4->Sync(ChannelSyncType::None);
  ch4->DataType(ChannelDataType::FloatLe);
  ch4->DataBytes(4);

  writer->PreTrigTime(0.0); // Store one seconds samples before start
  writer->InitMeasurement(); // Start the internal cache
  auto tick_time = TimeStampToNs();

  writer->StartMeasurement(tick_time);
  size_t sample = 0;
  // Fill with 1 seconds samples after start.
  for (; sample < 2000; ++sample) {
    const auto value = static_cast<uint32_t>(sample);
    ch1->SetChannelValue(value);
    ch2->SetChannelValue(value);
    ch3->SetChannelValue(value);
    ch4->SetChannelValue(value);
    writer->SaveSample(*group1, tick_time);
    writer->SaveSample(*group2, tick_time);
    writer->SaveSample(*group3, tick_time);
    writer->SaveSample(*group4, tick_time);
    tick_time += 2'000'000'000;
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MDF_TRACE() << "Written MDF File: " <<  writer->Name();

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
    ASSERT_EQ(observer_list.size(), 8) << "Observer List Size: " << observer_list.size();
    reader.ReadData(*dg4);
  }
  reader.Close();

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    const auto nof_samples = observer->NofSamples();
    EXPECT_EQ(nof_samples, 2000);
    if (observer->IsMaster()) {
      // Check that the times are stepping forward
      double previous = 0.0;
      for (sample = 0 ; sample < nof_samples; ++sample) {
        double time = 0.0;
        observer->GetEngValue(sample, time, 0);
        if (sample > 0) {
          EXPECT_GT(time, previous) << "Time/Previous: " << time << "/" << previous;
        }
        previous = time;
      }
    }
  }
}

TEST_F(TestWrite, Mdf4Mime) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("mime.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  writer->CompressData(true);

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Test mime types");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("2.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();

  auto* group1 = data_group->CreateChannelGroup();
  group1->Name("Mime");

  auto* master1 = group1->CreateChannel();
  master1->Name("Time");
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->DataType(ChannelDataType::FloatLe);
  master1->DataBytes(4);

  auto* ch1 = group1->CreateChannel();
  ch1->Name("Pictures");
  ch1->Type(ChannelType::VariableLength); // Store in SD block
  ch1->Sync(ChannelSyncType::None);
  ch1->DataType(ChannelDataType::MimeSample);
  ch1->Unit("image/png");
  ch1->DataBytes(8); // 64-bit index

  // Not very likely that we are using the same time channel for
  // picture and video streams
  auto* ch2 = group1->CreateChannel();
  ch2->Name("Video");
  ch2->Type(ChannelType::VariableLength);
  ch2->Sync(ChannelSyncType::None);
  ch2->DataType(ChannelDataType::MimeStream);
  ch2->Unit("application/mpeg4-generic");
  ch2->DataBytes(8);

  writer->PreTrigTime(0.0);
  writer->InitMeasurement(); // The sample cache is started here

  auto tick_time = TimeStampToNs();

  writer->StartMeasurement(tick_time);

  std::vector<uint8_t> byte_array = {1,2,3,4,5}; // Not a very likely input
  constexpr size_t kSamples = 10'000;
  {
    for (size_t sample = 0; sample < kSamples; ++sample) {
      ch1->SetChannelValue(byte_array);
      ch2->SetChannelValue(byte_array);
      writer->SaveSample(*group1, tick_time);
      tick_time += 1'000'000;
    }
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();

  MdfReader reader(mdf_file.string());
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());

  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);

  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);

  const auto dg4 = header1->LastDataGroup();
  ASSERT_TRUE(dg4 != nullptr);

  const auto cg_list = dg4->ChannelGroups();
  EXPECT_EQ(cg_list.size(), 1);
  for (auto* cg4 : cg_list) {
    CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
  }
  EXPECT_TRUE(reader.ReadData(*dg4));
  reader.Close();

  EXPECT_EQ(observer_list.size(), 3);
  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), kSamples);
    for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
      if (observer->IsMaster()) {
        continue; // Skip the time channel test
      }
      std::vector<uint8_t> sample_array;
      const auto valid = observer->GetChannelValue(sample, sample_array);
      EXPECT_TRUE(valid) << "Invalid Sample: " << sample;
      EXPECT_EQ(sample_array, byte_array) << "Invalid Value: "
                                          << observer->Name()
                                          << " ,Sample: " << sample;
     }
  }
}

TEST_F(TestWrite, MultiCGWithoutMasterChannel) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("without_master.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test write without master");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  auto* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  // Note that no master channel added

  auto* group1 = last_dg->CreateChannelGroup("Group1");
  ASSERT_TRUE(group1 != nullptr);
  auto* signal1 = group1->CreateChannel("Signal1");
  ASSERT_TRUE(signal1 != nullptr);
  signal1->DataType(ChannelDataType::FloatLe);
  signal1->DataBytes(8);

  auto* group2 = last_dg->CreateChannelGroup("Group2");
  ASSERT_TRUE(group2 != nullptr);
  auto* signal2 = group2->CreateChannel("Signal2");
  ASSERT_TRUE(signal2 != nullptr);
  signal2->DataType(ChannelDataType::FloatLe);
  signal2->DataBytes(8);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 1'000; ++sample) {
    const auto value = static_cast<double>(sample);
    signal1->SetChannelValue(value, true);
    writer->SaveSample(*group1, tick_time);

    signal2->SetChannelValue(value, true);
    writer->SaveSample(*group2, tick_time);
    tick_time += 1'000'000; // 1 ms period
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
    EXPECT_EQ(cg_list.size(), 2);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 1'000);
    // Verify that the values are correct
    for (size_t sample = 0; sample < 1'000; ++sample ) {
      double value = -1.0;
      const bool valid = observer->GetChannelValue(sample, value);
      EXPECT_TRUE(valid);
      EXPECT_DOUBLE_EQ(value, static_cast<double>(sample));
    }
  }
}

TEST_F(TestWrite, MdfCompressConverter) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  const auto start_write = TimeStampToNs();
  path mdf_file(kTestDir);
  mdf_file.append("mdf_compress_converter.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test MDF Converter");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  // Note that no master channel added

  auto* group1 = last_dg->CreateChannelGroup("Group1");
  ASSERT_TRUE(group1 != nullptr);

  auto* master1 = group1->CreateChannel("Master1");
  ASSERT_TRUE(master1 != nullptr);
  master1->DataType(ChannelDataType::FloatLe);
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->Unit("s");
  master1->DataBytes(4);

  auto* signal1 = group1->CreateChannel("Signal1");
  ASSERT_TRUE(signal1 != nullptr);
  signal1->DataType(ChannelDataType::FloatLe);
  signal1->DataBytes(8);

  auto* group2 = last_dg->CreateChannelGroup("Group2");
  ASSERT_TRUE(group2 != nullptr);

  auto* master2 = group2->CreateChannel("Master2");
  ASSERT_TRUE(master2 != nullptr);
  master2->DataType(ChannelDataType::FloatLe);
  master2->Type(ChannelType::Master);
  master2->Sync(ChannelSyncType::Time);
  master2->Unit("s");
  master2->DataBytes(4);

  auto* signal2 = group2->CreateChannel("Signal2");
  ASSERT_TRUE(signal2 != nullptr);
  signal2->DataType(ChannelDataType::FloatLe);
  signal2->DataBytes(8);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 1'000'000; ++sample) {
    const auto value = static_cast<double>(sample);
    signal1->SetChannelValue(value, true);
    writer->SaveSample(*group1, tick_time);

    signal2->SetChannelValue(value, true);
    writer->SaveSample(*group2, tick_time);
    tick_time += 1'000'000; // 1 ms period
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();
  const auto stop_write = TimeStampToNs();
  std::cout << "Write Time (2MS) [s]: " <<
      static_cast<double>(stop_write - start_write) / 1'000'000'000
      << std::endl;
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
    EXPECT_EQ(cg_list.size(), 2);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 1'000'000);
    // Verify only signal data

    // Verify that the values are correct. Only checking first 10 values.
    for (size_t sample = 0; sample < 10; ++sample ) {
      if (observer->IsMaster()) {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid);
        if (sample > 0 ) {
          EXPECT_GT(value, 0.0);
        }
      } else {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid);
        EXPECT_DOUBLE_EQ(value, static_cast<double>(sample));
      }
    }
  }
}


TEST_F(TestWrite, MdfNotCompressConverter) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  const auto start_write = TimeStampToNs();
  path mdf_file(kTestDir);
  mdf_file.append("mdf_nocompress_converter.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
  writer->Init(mdf_file.string());
  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test MDF Converter");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->PreTrigTime(0.0);
  writer->CompressData(false);

  auto* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  // Note that no master channel added

  auto* group1 = last_dg->CreateChannelGroup("Group1");
  ASSERT_TRUE(group1 != nullptr);

  auto* master1 = group1->CreateChannel("Master1");
  ASSERT_TRUE(master1 != nullptr);
  master1->DataType(ChannelDataType::FloatLe);
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->Unit("s");
  master1->DataBytes(4);

  auto* signal1 = group1->CreateChannel("Signal1");
  ASSERT_TRUE(signal1 != nullptr);
  signal1->DataType(ChannelDataType::FloatLe);
  signal1->DataBytes(8);

  auto* group2 = last_dg->CreateChannelGroup("Group2");
  ASSERT_TRUE(group2 != nullptr);

  auto* master2 = group2->CreateChannel("Master2");
  ASSERT_TRUE(master2 != nullptr);
  master2->DataType(ChannelDataType::FloatLe);
  master2->Type(ChannelType::Master);
  master2->Sync(ChannelSyncType::Time);
  master2->Unit("s");
  master2->DataBytes(4);

  auto* signal2 = group2->CreateChannel("Signal2");
  ASSERT_TRUE(signal2 != nullptr);
  signal2->DataType(ChannelDataType::FloatLe);
  signal2->DataBytes(8);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 1'000'000; ++sample) {
    const auto value = static_cast<double>(sample);
    signal1->SetChannelValue(value, true);
    writer->SaveSample(*group1, tick_time);

    signal2->SetChannelValue(value, true);
    writer->SaveSample(*group2, tick_time);
    tick_time += 1'000'000; // 1 ms period
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();
  const auto stop_write = TimeStampToNs();
  std::cout << "Write Time (2MS) [s]: " <<
      static_cast<double>(stop_write - start_write) / 1'000'000'000
            << std::endl;
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
    EXPECT_EQ(cg_list.size(), 2);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 1'000'000);
    // Verify only signal data

    // Verify that the values are correct. Only checking first 10 values.
    for (size_t sample = 0; sample < 10; ++sample ) {
      if (observer->IsMaster()) {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid) << sample;
        if (sample > 0 ) {
          EXPECT_GT(value, 0.0);
        }
      } else {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid);
        EXPECT_DOUBLE_EQ(value, static_cast<double>(sample));
      }
    }
  }
}


TEST_F(TestWrite, TestStreamInterface) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path test_file(kTestDir);
  test_file.append("stream_write.mf4");

  auto buffer = std::make_shared<std::filebuf>();
  const auto* file = buffer->open(test_file.string(),
    std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
  ASSERT_TRUE(file != nullptr) << test_file;

  const auto start_write = TimeStampToNs();
  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);

  writer->Init(buffer);

  auto* header = writer->Header();
  auto* history = header->CreateFileHistory();
  history->Description("Test MDF Converter");
  history->ToolName("MdfWrite");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  writer->PreTrigTime(0.0);
  writer->CompressData(true);

  auto* last_dg = header->CreateDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  // Note that no master channel added

  auto* group1 = last_dg->CreateChannelGroup("Group1");
  ASSERT_TRUE(group1 != nullptr);

  auto* master1 = group1->CreateChannel("Master1");
  ASSERT_TRUE(master1 != nullptr);
  master1->DataType(ChannelDataType::FloatLe);
  master1->Type(ChannelType::Master);
  master1->Sync(ChannelSyncType::Time);
  master1->Unit("s");
  master1->DataBytes(4);

  auto* signal1 = group1->CreateChannel("Signal1");
  ASSERT_TRUE(signal1 != nullptr);
  signal1->DataType(ChannelDataType::FloatLe);
  signal1->DataBytes(8);

  auto* group2 = last_dg->CreateChannelGroup("Group2");
  ASSERT_TRUE(group2 != nullptr);

  auto* master2 = group2->CreateChannel("Master2");
  ASSERT_TRUE(master2 != nullptr);
  master2->DataType(ChannelDataType::FloatLe);
  master2->Type(ChannelType::Master);
  master2->Sync(ChannelSyncType::Time);
  master2->Unit("s");
  master2->DataBytes(4);

  auto* signal2 = group2->CreateChannel("Signal2");
  ASSERT_TRUE(signal2 != nullptr);
  signal2->DataType(ChannelDataType::FloatLe);
  signal2->DataBytes(8);

  writer->InitMeasurement();
  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);
  for (size_t sample = 0; sample < 1'000'000; ++sample) {
    const auto value = static_cast<double>(sample);
    signal1->SetChannelValue(value, true);
    writer->SaveSample(*group1, tick_time);

    signal2->SetChannelValue(value, true);
    writer->SaveSample(*group2, tick_time);
    tick_time += 1'000'000; // 1 ms period
  }
  writer->StopMeasurement(tick_time);
  writer->FinalizeMeasurement();
  const auto stop_write = TimeStampToNs();
  buffer->close();

  std::cout << "Write Time [ms]: " <<
      static_cast<double>(stop_write - start_write) / 1'000'000
            << std::endl;

  auto in_buffer = std::make_shared<std::filebuf>();
  const auto* in_file = in_buffer->open(test_file.string(),
             std::ios_base::in | std::ios_base::binary);
  ASSERT_TRUE( in_file != nullptr) << test_file;

  MdfReader reader( in_buffer );
  ChannelObserverList observer_list;

  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());
  const auto* file1 = reader.GetFile();
  const auto* header1 = file1->Header();
  const auto dg_list = header1->DataGroups();
  EXPECT_EQ(dg_list.size(), 1);

  for (auto* dg4 : dg_list) {
    const auto cg_list = dg4->ChannelGroups();
    EXPECT_EQ(cg_list.size(), 2);
    for (auto* cg4 : cg_list) {
      CreateChannelObserverForChannelGroup(*dg4, *cg4, observer_list);
    }
    reader.ReadData(*dg4);
  }
  reader.Close();
  in_buffer->close();

  const auto read_time = TimeStampToNs();
  std::cout << "Write Time [ms]: " <<
      static_cast<double>(read_time - stop_write) / 1'000'000
            << std::endl;

  std::set<std::string> unique_list;

  for (auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    EXPECT_EQ(observer->NofSamples(), 1'000'000);
    // Verify only signal data

    // Verify that the values are correct. Only checking first 10 values.
    for (size_t sample = 0; sample < 10; ++sample ) {
      if (observer->IsMaster()) {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid);
        if (sample > 0 ) {
          EXPECT_GT(value, 0.0);
        }
      } else {
        double value = -1.0;
        const bool valid = observer->GetChannelValue(sample, value);
        EXPECT_TRUE(valid);
        EXPECT_DOUBLE_EQ(value, static_cast<double>(sample));
      }
    }
  }
}

}  // end namespace mdf::test