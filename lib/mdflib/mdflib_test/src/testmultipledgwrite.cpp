/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "testmultipledgwrite.h"
#include <chrono>
#include <filesystem>
#include <thread>

#include "util/logconfig.h"
#include "util/logstream.h"

#include "mdf/mdflogstream.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannel.h"
#include "mdf/mdfreader.h"

using namespace util::log;
using namespace std::filesystem;
using namespace std::chrono_literals;
using namespace std::this_thread;

namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/write;
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

  // Copy MdfLocation to std::source_location is still a problem.
  // Adding it to the text for the time being.
  std::ostringstream temp;
  temp << text << " [" << location.file
       << "::" << location.function
       << ":" << location.line << "]";
  LogMessage message;
  message.message = temp.str();
  message.severity = static_cast<LogSeverity>(severity);

  log_config.AddLogMessage(message);
}
/**
 * Function that stops logging of MDF logs
 */
void NoLog(const MdfLocation& , mdf::MdfLogSeverity,
           const std::string& ) {
}

}  // namespace


namespace mdf::test {
void TestMultipleDgWrite::SetUpTestSuite() {


  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at the restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to a file as this file is used as an attachment file
    auto& log_config = LogConfig::Instance();
    log_config.RootDir(kTestRootDir);
    log_config.BaseName("multi_dg_write");
    log_config.Type(LogType::LogToFile);
    log_config.CreateDefaultLogger();

    remove(log_config.GetLogFile()); // Remove any old log files

    // Connect the MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    // Add console logging
    log_config.AddLogger("Console",LogType::LogToConsole, {});
    LOG_TRACE() << "Log file created. File: " << log_config.GetLogFile();

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after.
    temp_dir.append("mdf");
    temp_dir.append("multiple_dg");
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

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestMultipleDgWrite::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestMultipleDgWrite, Mdf3WriteSingleDG) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  size_t max_sample = 10'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("single_dg.mf3");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("ACME Road Runner Company");
    header->Description("Testing Single DG Measurement");
    header->Project("MDF3 Write Multiple (Sorted) File");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    auto old_f_size = file_size(mdf_file);
    size_t file_save = 0;
    size_t sample;
    for (sample = 0; sample < max_sample && file_save <= 1; ++sample) {
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value =  static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            if (channel->Type() == ChannelType::Master) {
              continue;
            }
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*channel_group, MdfHelper::NowNs());
        }
        yield();
        // sleep_for(2ms);
        auto f_size = file_size(mdf_file);
        if (f_size >old_f_size ) {
          ++file_save;
          old_f_size = f_size;
        }
      }
    }
    max_sample = sample;
    EXPECT_GT(file_save, 1);
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    LOG_TRACE() << "File Saved: " << file_save << " times.";
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 1);

    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GT(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            auto expected_value = static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }

    }
  }

}


TEST_F(TestMultipleDgWrite, Mdf3WriteMultiDG) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 150;
  path mdf_file(kTestDir);
  mdf_file.append("multi_dg.mf3");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF3 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
        yield();
        //sleep_for(100ms);
      }
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GT(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4WriteMultiDG) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("multi_dg.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF4 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
        yield();
        //sleep_for(0ms);
      }
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4ConverterMultiDGCompress) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("converter_multi_dg_compress.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    ASSERT_TRUE(writer != nullptr);

    writer->PreTrigTime(0.0);
    writer->CompressData(true);
    writer->SavePeriodic(true);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement");
    header->Project("MDF4 Write Multiple DG");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
      }
      yield();
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    const uint64_t write_time = (stop_time - start_time)/ 1'000'000;
    LOG_TRACE() << "Converter Write Time [ms]: " << write_time;
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}

TEST_F(TestMultipleDgWrite, Mdf4ConverterMultiNoCompressDG) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr size_t max_sample = 1'000'000;
  path mdf_file(kTestDir);
  mdf_file.append("converter_multi_dg_no_compress.mf4");
  {
    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::MdfConverter);
    ASSERT_TRUE(writer != nullptr);

    writer->PreTrigTime(0.0);
    writer->CompressData(false);
    writer->SavePeriodic(false);

    writer->Init(mdf_file.string());

    auto* header = writer->Header();
    ASSERT_TRUE(header != nullptr);
    header->Author("Ingemar Hedvall");
    header->Department("Home Alone");
    header->Description("Testing Multiple DG Measurement with no Compression");
    header->Project("MDF4 Write Multiple DG No Compress");

    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(8);
      master->Unit("s");

      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatLe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }
    {
      auto* data_group = writer->CreateDataGroup();
      ASSERT_TRUE(data_group != nullptr);

      auto* channel_group = data_group->CreateChannelGroup();
      ASSERT_TRUE(channel_group != nullptr);

      auto* master = channel_group->CreateChannel();
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");

      for (size_t cn_index = 10; cn_index < 13; ++cn_index) {
        auto* channel = channel_group->CreateChannel();
        ASSERT_TRUE(channel != nullptr);
        std::ostringstream name;
        name << "Channel_" << cn_index + 1;
        channel->Name(name.str());
        channel->Description("Channel description");
        channel->Type(ChannelType::FixedLength);
        channel->DataType(ChannelDataType::FloatBe);
        channel->DataBytes(4);
        channel->Unit("s");
      }
    }

    writer->InitMeasurement();
    const auto start_time = MdfHelper::NowNs();

    writer->StartMeasurement(start_time);
    for (size_t sample = 0; sample < max_sample; ++sample) {
      uint32_t dg_count = 0;
      for (auto* data_group : header->DataGroups()) {
        for (auto* channel_group : data_group->ChannelGroups()) {
          auto value = (10.0 * dg_count) + static_cast<double>(sample);
          for (auto* channel : channel_group->Channels()) {
            channel->SetChannelValue(value);
          }
          writer->SaveSample(*data_group, *channel_group, MdfHelper::NowNs());
          ++dg_count;
        }
      }
      yield();
    }
    auto stop_time = MdfHelper::NowNs();
    writer->StopMeasurement(stop_time);
    writer->FinalizeMeasurement();

    const uint64_t write_time = (stop_time - start_time)/ 1'000'000;
    LOG_TRACE() << "Converter Write Time [ms]: " << write_time;
  }
  {
    MdfReader reader(mdf_file.string());
    EXPECT_TRUE(reader.IsOk());
    EXPECT_TRUE(reader.IsFinalized());

    EXPECT_TRUE(reader.ReadEverythingButData());
    const auto* header = reader.GetHeader();
    ASSERT_TRUE(header != nullptr);


    const auto dg_list = header->DataGroups();
    EXPECT_EQ(dg_list.size(), 2);
    uint32_t dg_count = 0;
    for (auto* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);

      ChannelObserverList observer_list;
      CreateChannelObserverForDataGroup(*data_group, observer_list);
      EXPECT_EQ(observer_list.size(), 4);
      EXPECT_TRUE(reader.ReadData(*data_group));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        EXPECT_EQ(observer->NofSamples(), max_sample);
        double old_time = -1.0;
        for (size_t sample = 0; sample < observer->NofSamples() && sample < 10; ++sample) {
          if (observer->IsMaster()) {
            double sample_time = 0.0;
            const bool valid = observer->GetEngValue(sample, sample_time);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_GE(sample_time, old_time) << "Sample: " << sample;
            old_time = sample_time;
          } else {
            double expected_value =
                (10.0 * dg_count) + static_cast<double>(sample);
            double value = 0.0;
            const bool valid = observer->GetEngValue(sample, value);
            EXPECT_TRUE(valid) << "Sample: " << sample;
            EXPECT_DOUBLE_EQ(expected_value, value) << "Sample: " << sample;
          }
        }
      }
      ++dg_count;
    }
  }

}


}  // namespace mdf::test