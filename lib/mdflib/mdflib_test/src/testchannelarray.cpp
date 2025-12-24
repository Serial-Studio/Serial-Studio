/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include <gtest/gtest.h>
#include "testchannelarray.h"
#include "ca4block.h"

#include <filesystem>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/mdflogstream.h"
#include "mdf/mdffactory.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdfreader.h"
#include "mdf/ifilehistory.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannel.h"

using namespace std::this_thread;
using namespace std::chrono_literals;
using namespace util::log;
using namespace util::time;
using namespace std::filesystem;


namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/array";
constexpr std::string_view kFixedAxisFile =
    "E:/test_dir/mdf/mdf4_2/Arrays/Simple/Vector_ArrayWithFixedAxes.MF4";
constexpr std::string_view kArrayInArrayFile =
    "E:/test_dir/mdf/mdf4_2/Arrays/Simple/Vector_MeasurementArrays.mf4";
bool kSkipTest = false;

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
void NoLog(const MdfLocation& location, mdf::MdfLogSeverity severity,
           const std::string& text) {
}

}  // namespace

namespace mdf::test {

void TestChannelArray::SetUpTestSuite() {


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
    log_config.BaseName("mdf_channel_array");
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
    temp_dir.append("channel_array");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      LOG_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();

  } catch (const std::exception& err) {
    LOG_ERROR() << "Failed to create test directories. Error: " << err.what();
    kSkipTest = true;
  }
}

void TestChannelArray::TearDownTestSuite() {
  LOG_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();

}


TEST_F(TestChannelArray, TestProperties) {
  mdf::detail::Ca4Block array;
  EXPECT_EQ(array.Index(), 0);
  EXPECT_STREQ(array.BlockType().c_str(), "CA");

  array.Type(ArrayType::LookUp);
  EXPECT_EQ(array.Type(), ArrayType::LookUp);

  array.Storage(ArrayStorage::CgTemplate);
  EXPECT_EQ(array.Storage(), ArrayStorage::CgTemplate);

  EXPECT_EQ(array.Flags(), 0);
  array.Flags(CaFlag::ComparisonQuantity);
  EXPECT_EQ(array.Flags(), CaFlag::ComparisonQuantity);

  EXPECT_EQ(array.Dimensions(), 0);
  const std::vector<uint64_t> shape = {1,2,3};
  array.Shape(shape);
  EXPECT_EQ(array.Dimensions(), 3);
  const auto& shape1 = array.Shape();
  EXPECT_EQ(shape, shape1);
  EXPECT_EQ(array.DimensionSize(0), 1);
  EXPECT_EQ(array.DimensionSize(1), 2);
  EXPECT_EQ(array.DimensionSize(2), 3);
  EXPECT_EQ(array.SumOfArray(), 6);
  EXPECT_EQ(array.ProductOfArray(), 6);
  EXPECT_EQ(array.NofArrayValues(), 6);
  EXPECT_FALSE(array.DimensionAsString().empty());

  array.Storage(ArrayStorage::DgTemplate);
  array.ResizeArrays();
  EXPECT_EQ(array.DataLinks().size(), array.ProductOfArray());
  EXPECT_EQ(array.CycleCounts().size(), array.ProductOfArray());

  array.Flags(CaFlag::DynamicSize);
  array.ResizeArrays();
  EXPECT_EQ(array.DynamicSizeList().size(), array.Dimensions());

  array.Flags(CaFlag::InputQuantity);
  array.ResizeArrays();
  EXPECT_EQ(array.InputQuantityList().size(), array.Dimensions());

  array.Flags(CaFlag::Axis);
  array.ResizeArrays();
  EXPECT_EQ(array.AxisConversionList().size(), array.Dimensions());
  EXPECT_EQ(array.AxisList().size(), array.Dimensions());

  array.Flags(CaFlag::FixedAxis);
  array.ResizeArrays();
  EXPECT_EQ(array.AxisValues().size(), array.SumOfArray());

}

TEST_F(TestChannelArray, Mdf4WriteCA) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("ca4.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfChannelArray Unit Test");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  ASSERT_TRUE(data_group != nullptr);

  // Test CG4 block
  auto* channel_group = data_group->CreateChannelGroup();
  ASSERT_TRUE(channel_group != nullptr);
  channel_group->Name("Group1");

  // Add master
  auto* master = channel_group->CreateChannel();
  ASSERT_TRUE(master != nullptr);

  // Add time master channel
  master->Name("Time");
  master->Unit("km/h");
  master->Type(ChannelType::Master);
  master->Sync(ChannelSyncType::Time);
  master->DataType(ChannelDataType::FloatLe);
  master->DataBytes(8);

  // Add channel
  auto* channel = channel_group->CreateChannel();
  ASSERT_TRUE(channel != nullptr);

  channel->Name("Array Channel");
  channel->DisplayName("Test channel");
  channel->Description("Unit test of CA block");
  channel->Unit("km/h");
  channel->Flags(CnFlag::InvalidValid);
  channel->Type(ChannelType::FixedLength);
  channel->Sync(ChannelSyncType::None);
  channel->DataType(ChannelDataType::FloatLe);
  channel->DataBytes(8);

  auto* channel1 = channel_group->CreateChannel();
  ASSERT_TRUE(channel1 != nullptr);
  channel1->Name("Normal Channel");
  channel1->DisplayName("Test channel");
  channel1->Description("Unit test of CA block");
  channel1->Unit("kPa");
  channel1->Flags(CnFlag::InvalidValid);
  channel1->Type(ChannelType::FixedLength);
  channel1->Sync(ChannelSyncType::None);
  channel1->DataType(ChannelDataType::FloatLe);
  channel1->DataBytes(8);

  // Add Channel Array
  auto* array = channel->CreateChannelArray();
  ASSERT_TRUE(array != nullptr);

  array->Type(ArrayType::Array);
  array->Shape({3,4});
  const uint64_t nof_array_values = array->NofArrayValues();

  // Create some dummy samples
  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  for (size_t sample = 0; sample < 10; ++sample) {
    for (uint64_t index = 0; index < nof_array_values; ++index) {
      bool valid = (index % 2) == 0;
      channel->SetChannelValue(index, valid, index);
    }
    writer->SaveSample(*channel_group,tick_time);
    tick_time += 1'000'000'000; // 1 second tick
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
      EXPECT_EQ(cn_list.size(),3);

      bool master_channel = false;
      for (auto* cn4 : cn_list) {
        ASSERT_TRUE(cn4 != nullptr);
        EXPECT_GT(cn4->Index(),0);
        if (cn4->Type() == ChannelType::Master) {
          master_channel = true;
        }
        const auto* ca4 = cn4->ChannelArray(0);
        if (ca4 != nullptr) {
          EXPECT_GT(ca4->Index(), 0);
          EXPECT_EQ(ca4->Type(), ArrayType::Array);
          ASSERT_EQ(ca4->Dimensions(), 2);
          ASSERT_EQ(ca4->SumOfArray(), 3 + 4);
          ASSERT_EQ(ca4->ProductOfArray(), 3 * 4);
        }
      }
      EXPECT_TRUE(master_channel);

      // Set up subscription and read in data
      ChannelObserverList  observer_list;
      CreateChannelObserverForChannelGroup(*dg4, *cg4,
                                           observer_list);
      EXPECT_TRUE(reader.ReadData(*dg4));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        const auto* array1 = observer->Channel().ChannelArray(0);
        if (array1 == nullptr) {
          continue;
        }

        for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
          for (uint64_t index = 0; index < array1->NofArrayValues(); ++index) {
            uint64_t value = 0;
            const bool valid = observer->GetChannelValue(sample, value, index);
            EXPECT_EQ(valid, (index % 2) == 0) << "Sample/Index: "
                                               << sample << "/" << index;
            EXPECT_EQ(value, index) << "Sample/Index: " << sample
                << "/" << index
                << ", Value: " << value << "(" << index << ")";
          }
        }
      }
    }
  }
}

TEST_F(TestChannelArray, TestArrayInsertAndRead) {
  if (kSkipTest) {
    GTEST_SKIP();
  }
  path mdf_file(kTestDir);
  mdf_file.append("array.mf4");

  auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
  ASSERT_TRUE(writer->Init(mdf_file.string()));
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);

  auto* history = header->CreateFileHistory();
  ASSERT_TRUE(history != nullptr);
  history->Description("Created");
  history->ToolName("MdfChannelArray Unit Test");
  history->ToolVendor("ACME Road Runner Company");
  history->ToolVersion("1.0");
  history->UserName("Ingemar Hedvall");

  auto* data_group = header->CreateDataGroup();
  ASSERT_TRUE(data_group != nullptr);

  // Test CG4 block
  auto* channel_group = data_group->CreateChannelGroup();
  ASSERT_TRUE(channel_group != nullptr);
  channel_group->Name("Group1");

  // Add master
  auto* master = channel_group->CreateChannel();
  ASSERT_TRUE(master != nullptr);

  // Add time master channel
  master->Name("Time");
  master->Unit("km/h");
  master->Type(ChannelType::Master);
  master->Sync(ChannelSyncType::Time);
  master->DataType(ChannelDataType::FloatLe);
  master->DataBytes(8);

  // Add channel
  auto* channel = channel_group->CreateChannel();
  ASSERT_TRUE(channel != nullptr);

  channel->Name("Array Channel");
  channel->DisplayName("Test channel");
  channel->Description("Unit test of CA block");
  channel->Unit("km/h");
  channel->Flags(CnFlag::InvalidValid);
  channel->Type(ChannelType::FixedLength);
  channel->Sync(ChannelSyncType::None);
  channel->DataType(ChannelDataType::FloatLe);
  channel->DataBytes(8);

  // Add Channel Array
  auto* array = channel->CreateChannelArray();
  ASSERT_TRUE(array != nullptr);

  array->Type(ArrayType::Array);
  array->Shape({3,4});
  const uint64_t nof_array_values = array->NofArrayValues();

  // Create some dummy samples
  writer->PreTrigTime(0);
  writer->InitMeasurement();

  auto tick_time = TimeStampToNs();
  writer->StartMeasurement(tick_time);

  std::vector<double> values(nof_array_values);
  for (uint64_t index = 0; index < nof_array_values; ++index) {
    values[index] = static_cast<double>(index);
  }

  for (size_t sample = 0; sample < 10; ++sample) {
    channel->SetChannelValues(values);
    writer->SaveSample(*channel_group,tick_time);
    tick_time += 1'000'000'000; // 1 second tick
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
      EXPECT_EQ(cn_list.size(),2);

      bool master_channel = false;
      for (auto* cn4 : cn_list) {
        ASSERT_TRUE(cn4 != nullptr);
        EXPECT_GT(cn4->Index(),0);
        if (cn4->Type() == ChannelType::Master) {
          master_channel = true;
        }
        const auto* ca4 = cn4->ChannelArray(0);
        if (ca4 != nullptr) {
          EXPECT_GT(ca4->Index(), 0);
          EXPECT_EQ(ca4->Type(), ArrayType::Array);
          ASSERT_EQ(ca4->Dimensions(), 2);
          ASSERT_EQ(ca4->SumOfArray(), 3 + 4);
          ASSERT_EQ(ca4->ProductOfArray(), 3 * 4);
        }
      }
      EXPECT_TRUE(master_channel);

      // Set up subscription and read in data
      ChannelObserverList  observer_list;
      CreateChannelObserverForChannelGroup(*dg4, *cg4,
                                           observer_list);
      EXPECT_TRUE(reader.ReadData(*dg4));

      for (const auto& observer : observer_list) {
        ASSERT_TRUE(observer);
        if (!observer->IsArray()) {
          const auto nof_samples = observer->NofSamples();
          std::vector<double> channel_values;
          observer->GetChannelSamples(channel_values);
          EXPECT_EQ(channel_values.size(),nof_samples);

          std::vector<double> eng_values;
          observer->GetEngSamples(eng_values);
          EXPECT_EQ(eng_values.size(),nof_samples);
        } else {
          const auto array_size = observer->ArraySize();
          std::vector<uint64_t> value_array;
          for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
            observer->GetEngValues(sample, value_array);
            EXPECT_EQ(value_array.size(), array_size);
            uint64_t expected = 0;
            for (const uint64_t value : value_array) {
              EXPECT_EQ(value, expected)
                  << "Sample: " << sample << ", Value: " << value << "("
                  << expected << ")";
              ++expected;
            }
          }
        }
      }
    }
  }
}

TEST_F(TestChannelArray, TestArrayInArray) {
  try {
    path filename(kArrayInArrayFile);
    if (!exists(filename)) {
      throw std::runtime_error("File does not exist. File:" + filename.string());
    }
  } catch (const std::exception& err) {
    GTEST_SKIP_(err.what());
  }
  const std::string test_file(kArrayInArrayFile);
  MdfReader reader(test_file);
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadEverythingButData());

  const auto* file = reader.GetFile();
  ASSERT_TRUE(file != nullptr);

  const auto* header = file->Header();
  ASSERT_TRUE(header != nullptr);

  const auto dg_list = header->DataGroups();
  ASSERT_EQ(dg_list.size(), 11);

  auto* data_group = dg_list[8];
  ASSERT_TRUE(data_group != nullptr);

  const auto cg_list = data_group->ChannelGroups();
  EXPECT_EQ(cg_list.size(), 1);

  const auto* channel_group = cg_list[0];
  ASSERT_TRUE(channel_group != nullptr);

  EXPECT_EQ(channel_group->NofSamples(),4);
  EXPECT_EQ(channel_group->RecordId(),1);

  const auto cn_list = channel_group->Channels();
  ASSERT_EQ(cn_list.size(),2);

  const auto* master = channel_group->GetMasterChannel();
  ASSERT_TRUE(master != nullptr);

  const auto* channel = cn_list[1];
  ASSERT_TRUE(channel != nullptr);

  const auto ca_list = channel->ChannelArrays();
  ASSERT_EQ(ca_list.size(),2);

  const auto* array1 = ca_list[0];
  ASSERT_TRUE(array1 != nullptr);
  EXPECT_EQ(array1->Type(), ArrayType::LookUp);
  EXPECT_EQ(array1->Storage(), ArrayStorage::CnTemplate);
  EXPECT_EQ(array1->Dimensions(), 1);
  EXPECT_EQ(array1->DimensionSize(0), 6);

  const auto* array2 = ca_list[1];
  ASSERT_TRUE(array2 != nullptr);
  EXPECT_EQ(array2->Type(), ArrayType::LookUp);
  EXPECT_EQ(array2->Storage(), ArrayStorage::CnTemplate);
  EXPECT_EQ(array2->Dimensions(), 1);
  EXPECT_EQ(array2->DimensionSize(0), 8);

  EXPECT_EQ(channel->ArraySize(), 6*8);

  // Set up subscription and read in data
  ChannelObserverList  observer_list;
  CreateChannelObserverForChannelGroup(*data_group, *channel_group,
                                           observer_list);
  EXPECT_TRUE(reader.ReadData(*data_group));

  for (const auto& observer : observer_list) {
    ASSERT_TRUE(observer);
    if (!observer->IsArray()) {
      continue;
    }
    const auto array_size = observer->ArraySize();
    std::vector<uint64_t> value_array;
    for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
      const auto valid_list = observer->GetEngValues(sample,
        value_array);
      EXPECT_EQ(valid_list.size(), array_size);
      EXPECT_EQ(value_array.size(), array_size);
      uint64_t main_index = 0;
      std::cout << "Sample: " << sample << std::endl;
      std::cout << main_index << ": ";
      for (uint64_t index = 0; index < array_size; ++index) {
        if (main_index != (index / 8)) {
          main_index = index / 8;
          std::cout << std::endl << main_index << ": ";
        }
        EXPECT_TRUE(valid_list[index]);
        std::cout << value_array[index] << "\t";
      }
      std::cout << std::endl << std::endl;
    }
  }

}

}