/*
* Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <filesystem>
#include <chrono>
#include <string>
#include <iostream>
#include <thread>

#include "testwrite3.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfhelper.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdfreader.h"
using namespace std::filesystem;
using namespace std::chrono_literals;
using namespace mdf;

namespace {
std::string kTestDir; ///<  <Test Root Dir>/mdf/write3";
bool kSkipTest = false;

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogToConsole(const MdfLocation& location, mdf::MdfLogSeverity severity,
             const std::string& text) {
  std::cout << "[" << static_cast<int>(severity) << "] " << text << std::endl;
}

} // end namespace

namespace mdf::test {

void TestWrite3::SetUpTestSuite() {
  MdfLogStream::SetLogFunction1(LogToConsole);

  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    temp_dir.append("mdf");
    temp_dir.append("write3");
    kTestDir = temp_dir.string();
    // Create the test directory. Note that this directory is deleted before
    // running the test, not after.
    std::error_code err;
    remove_all(temp_dir, err);
    create_directories(temp_dir);

    MDF_TRACE() << "Created the test directory. Dir: " << kTestDir;
    kSkipTest = false;

  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the test directories. Error: "
      << err.what();
    kSkipTest = true;
  }

}

void TestWrite3::TearDownTestSuite() {
  MdfLogStream::SetLogFunction1(nullptr);
}

TEST_F(TestWrite3, Mdf3WriteHD) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("hd3.mf3");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf3Basic);

  writer->Init(mdf_file.string());

  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->Author("Ingemar Hedvall");
  header->Department("Home Alone");
  header->Description("Testing Header");
  header->Project("Mdf3WriteHD");
  header->StartTime(MdfHelper::NowNs());
  header->Subject("PXY");

  for (size_t dg_index = 0; dg_index < 2; ++dg_index) {
    auto* dg3 = writer->CreateDataGroup();
    ASSERT_TRUE(dg3 != nullptr);
    dg3->Description("First Measurement");
    for (size_t cg_index = 0; cg_index < 2; ++cg_index) {
      auto* cg3 = mdf::MdfWriter::CreateChannelGroup(dg3);
      ASSERT_TRUE(cg3 != nullptr);
      cg3->Description("CG description");
      for (size_t cn_index = 0; cn_index < 3; ++cn_index) {
        auto* cn3 = mdf::MdfWriter::CreateChannel(cg3);
        ASSERT_TRUE(cn3 != nullptr);
        std::ostringstream name;
        name << "Channel" << cn_index;
        cn3->Name(name.str());
        cn3->Description("Channel description");
        cn3->Type(cn_index == 0 ? ChannelType::Master
                                : ChannelType::FixedLength);
        cn3->DataType(ChannelDataType::FloatBe);
        cn3->DataBytes(4);
        cn3->Unit("s");
      }
    }
  }
  writer->InitMeasurement();
  writer->FinalizeMeasurement();
}

TEST_F(TestWrite3, TestManyChannel) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  constexpr uint16_t channelNum = 900;

  path test_file(kTestDir);
  test_file.append("test_many_channels.dat");
  auto writer = MdfFactory::CreateMdfWriterEx(MdfWriterType::Mdf3Basic);
  writer->Init(test_file.string());

  auto *header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto *dg3 = writer->CreateDataGroup();
  ASSERT_TRUE(dg3 != nullptr);

  auto *cg3 = mdf::MdfWriter::CreateChannelGroup(dg3);
  ASSERT_TRUE(cg3 != nullptr);
  {
      auto *master = mdf::MdfWriter::CreateChannel(cg3);
      master->Name("Time");
      master->Description("Time channel");
      master->Type(ChannelType::Master);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(4);
      master->Unit("s");
  }
  for (uint32_t index = 0; index < channelNum; ++index) {
    auto *channel = MdfWriter::CreateChannel(cg3);
    ASSERT_TRUE(channel != nullptr);
    channel->Name("channel_double_" + std::to_string(index));
    channel->Description("float");
    channel->Type(ChannelType::FixedLength);
    channel->DataType(ChannelDataType::FloatLe);
    channel->DataBytes(4);
  }

  for (uint32_t index = 0; index < channelNum; ++index) {
      auto *channel = MdfWriter::CreateChannel(cg3);
      channel->Name("channel_string_" + std::to_string(index));
      channel->Description("string");
      channel->Type(ChannelType::FixedLength);
      channel->DataType(ChannelDataType::StringAscii);
      channel->DataBytes(64);
  }


  uint64_t sample_time = MdfHelper::NowNs();
  ASSERT_TRUE(writer->InitMeasurement());
  writer->StartMeasurement(sample_time);
  auto cn_list = cg3->Channels();
  for (size_t sample = 0; sample < 10; ++sample) {

      // Master channel value set by the SaveSample function
      // cn_list[0]->SetChannelValue(0.01 * static_cast<double>(sample));
      for (uint32_t index = 0; index < channelNum; ++index) {
        cn_list[index + 1]->SetChannelValue(11.1F * static_cast<float>(sample));
      }

      for (uint32_t index = 0; index < channelNum; ++index) {
        cn_list[index + 1 + channelNum]->SetChannelValue(std::to_string(sample));
      }

      sample_time = MdfHelper::NowNs();

      writer->SaveSample(*cg3, sample_time);
      std::this_thread::sleep_for(10ms);
  }
  writer->StopMeasurement(MdfHelper::NowNs());
  writer->FinalizeMeasurement();

  MdfReader oRead(test_file.string());
  oRead.ReadEverythingButData();
  const auto *file = oRead.GetFile();
  ASSERT_TRUE(file != nullptr);

  DataGroupList dg_list;
  file->DataGroups(dg_list);
  for (auto *dg: dg_list) {
      ChannelObserverList observer_list;
      auto cg_list = dg->ChannelGroups();
      for (const auto *cg: cg_list) {
        EXPECT_EQ(cg->NofSamples(), 10);
        CreateChannelObserverForChannelGroup(*dg, *cg, observer_list);
      }
      oRead.ReadData(*dg);

      double value = 0;
      uint32_t count = 0;

      std::vector<double> values_exp{};
      std::vector<double> values_read{};
      for (const auto &channel: observer_list) {
          if (count == 0) {
              count++;
              continue;
          } else if (count < channelNum + 1) {
              size_t samples = channel->NofSamples();
              values_exp.clear();
              values_read.clear();
              bool fail = false;
              for (size_t sample = 0; sample < samples; ++sample) {
                  values_exp.push_back(11.1 * static_cast<double>(sample));
                  channel->GetEngValue(sample, value);
                  values_read.push_back(value);
                  if(11.1 * static_cast<double>(sample) - value > 0.00001)
                      fail = true;
              }
              if(fail)
                  EXPECT_EQ(values_read, values_exp);

          } else {
              size_t samples = channel->NofSamples();
              values_exp.clear();
              values_read.clear();
              bool fail = false;
              for (size_t sample = 0; sample < samples; ++sample) {
                  values_exp.push_back(static_cast<double>(sample));
                  channel->GetEngValue(sample, value);
                  values_read.push_back(value);
                  if (value == static_cast<double>(sample))
                      fail = true;
              }
              if (fail)
                  EXPECT_EQ(values_read, values_exp);
          }
          count++;
      }
  }
}
}  // namespace mdf