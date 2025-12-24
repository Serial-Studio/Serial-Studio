/*
 * Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testreadvlsd.h"

#include <filesystem>
#include <string_view>

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"

using namespace std::filesystem;


namespace {

constexpr std::string_view kVeryLargeFile =
   "k:/test/mdf/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";
constexpr std::string_view kLargeFrameCounter = "FrameCounter_VC0";
constexpr std::string_view kLargeFrameData = "VideoRawdata_VC0";

constexpr std::string_view kSdList = "k:/test/mdf/mdf4_2/DataList/SD_List/Vector_SD_List.MF4";
constexpr std::string_view kWriteSignalName = "Write";
void LogToConsole(const MdfLocation& location, mdf::MdfLogSeverity severity,
             const std::string& text) {
  std::cout << "[" << static_cast<int>(severity) << "] " << text << std::endl;
}

double ConvertToMs(uint64_t diff) {
  auto temp = static_cast<double>(diff);
  temp /= 1'000'000;
  return temp;
}

double ConvertToSec(uint64_t diff) {
  auto temp = static_cast<double>(diff);
  temp /= 1'000'000'000;
  return temp;
}

}

namespace mdf::test {


void TestReadVlsd::SetUpTestSuite() {
 MdfLogStream::SetLogFunction1(LogToConsole);
}

void TestReadVlsd::TearDownTestSuite() {
  MdfLogStream::SetLogFunction1(nullptr);
}

TEST_F(TestReadVlsd, TestVeryLargeFile) {
  try {
    if (!exists(kVeryLargeFile)) {
      GTEST_SKIP_("The test file doesn't exist");
    }
  } catch (const std::exception& err) {
    ASSERT_TRUE(false) << err.what();
  }
  const auto start_open = MdfHelper::NowNs();
  MdfReader reader((kVeryLargeFile.data()));
  const auto stop_open = MdfHelper::NowNs();
  std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

  const auto start_info = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadEverythingButData());
  const auto stop_info = MdfHelper::NowNs();
  std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

  const auto *header = reader.GetHeader();
  ASSERT_TRUE(header != nullptr);

  auto *last_dg = header->LastDataGroup();
  ASSERT_TRUE(last_dg != nullptr);

  auto channel_groups = last_dg->ChannelGroups();
  ASSERT_FALSE(channel_groups.empty());

  auto *channel_group = channel_groups[0];
  ASSERT_TRUE(channel_group != nullptr);

  auto *time_channel = channel_group->GetMasterChannel();;
  ASSERT_TRUE(time_channel != nullptr);

  auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
  ASSERT_TRUE(counter_channel != nullptr);

  auto *frame_channel = channel_group->GetChannel(kLargeFrameData);
  ASSERT_TRUE(frame_channel != nullptr);

  auto time_observer = CreateChannelObserver(*last_dg,
                                             *channel_group, *time_channel);
  auto counter_observer = CreateChannelObserver(*last_dg,
                                                    *channel_group, *counter_channel);
  auto frame_observer = CreateChannelObserver(*last_dg,
                                               *channel_group, *frame_channel);
  frame_observer->ReadVlsdData(false);

  const auto start_data = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadData(*last_dg)) << reader.ShortName();

  auto offset_list = frame_observer->GetOffsetList();
  EXPECT_FALSE(offset_list.empty());

  const auto stop_data = MdfHelper::NowNs();
  std::cout << "Read Offset Data (s): " << ConvertToSec(stop_data - start_data) << std::endl;

  size_t sample_count = 0;
  size_t byte_count = 0;
  std::function callback = [&](uint64_t offset, const std::vector<uint8_t> &buffer) {
    const bool offset_exist = std::ranges::any_of(std::as_const(offset_list),
                                          [&] (uint64_t off) {
      return off == offset;
    });
    EXPECT_TRUE(offset_exist) << "Offset: " << offset << std::endl;
    ++sample_count;
    byte_count += buffer.size();
  };

  const auto start_vlsd = MdfHelper::NowNs();
  // Limit the read to 100 samples
  while (offset_list.size() > 100) {
    offset_list.pop_back();
  }

  EXPECT_TRUE(reader.ReadVlsdData(*last_dg, *frame_channel, offset_list, callback))
    << reader.ShortName();
  const auto stop_vlsd = MdfHelper::NowNs();
  EXPECT_EQ(sample_count, 100);

  std::cout << "Read 100 VLSD Data (s): "
    << ConvertToSec(stop_vlsd - start_vlsd) << std::endl;
  std::cout << "Byte Count [MB]: "
    << static_cast<double>(byte_count) / 1'000'000 << std::endl;

  last_dg->ClearData();
  time_observer.reset();
  counter_observer.reset();
  frame_observer.reset();
  std::cout << std::endl;
}

TEST_F(TestReadVlsd, TestSdList) {
  try {
    if (!exists(kSdList)) {
      GTEST_SKIP_("The test file doesn't exist");
    }
  } catch (const std::exception& err) {
    ASSERT_TRUE(false) << err.what();
  }

  const auto start_open = MdfHelper::NowNs();
  MdfReader reader((kSdList.data()));
  const auto stop_open = MdfHelper::NowNs();
  std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

  const auto start_info = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadEverythingButData());
  const auto stop_info = MdfHelper::NowNs();
  std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

  const auto *header = reader.GetHeader();
  ASSERT_TRUE(header != nullptr);

  auto data_group_list = header->DataGroups();
  ASSERT_EQ(data_group_list.size(), 2);

  auto *first_dg = data_group_list[0];
  ASSERT_TRUE(first_dg != nullptr);

  auto channel_groups = first_dg->ChannelGroups();
  ASSERT_FALSE(channel_groups.empty());

  auto *channel_group = channel_groups[0];
  ASSERT_TRUE(channel_group != nullptr);

  auto *time_channel = channel_group->GetMasterChannel();;
  ASSERT_TRUE(time_channel != nullptr);

  auto *write_channel = channel_group->GetChannel(kWriteSignalName);
  ASSERT_TRUE(write_channel != nullptr);

  auto time_observer = CreateChannelObserver(*first_dg,
                                             *channel_group, *time_channel);
  ASSERT_TRUE(time_observer);

  auto write_observer = CreateChannelObserver(*first_dg,
                                                    *channel_group, *write_channel);
  ASSERT_TRUE(write_observer);
  write_observer->ReadVlsdData(false);

  const auto start_data = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadData(*first_dg)) << reader.ShortName();

  const auto& offset_list = write_observer->GetOffsetList();
  const auto nof_samples = write_observer->NofSamples();
  EXPECT_EQ(offset_list.size(), nof_samples);

  const auto stop_data = MdfHelper::NowNs();
  std::cout << "Read Offset Data [ms]: "
   << ConvertToMs(stop_data - start_data) << std::endl;

  size_t sample_count = 0;
  size_t byte_count = 0;
  std::function callback = [&](uint64_t offset, const std::vector<uint8_t> &buffer) {
    ++sample_count;
    EXPECT_GT(buffer.size(), 1);
    byte_count += buffer.size();
    for (size_t index = 0; sample_count < 10 && index < buffer.size(); ++index) {
      const char input = static_cast<const char>(buffer[index]);
      if (input != '\0') {
        std::cout << static_cast<char>(buffer[index]);
      }
    }
    if (sample_count < 10) {
      std::cout << std::endl;
    }
  };

  const auto start_vlsd = MdfHelper::NowNs();

  EXPECT_TRUE(reader.ReadVlsdData(*first_dg, *write_channel, offset_list, callback))
    << reader.ShortName();
  const auto stop_vlsd = MdfHelper::NowNs();
  EXPECT_EQ(sample_count, nof_samples);

  std::cout << "Read VLSD Data [ms]: "
    << ConvertToMs(stop_vlsd - start_vlsd) << std::endl;
  std::cout << "Byte Count [kB]: "
    << static_cast<double>(byte_count) / 1'000 << std::endl;

  first_dg->ClearData();
  time_observer.reset();
  write_observer.reset();
  std::cout << std::endl;
}

TEST_F(TestReadVlsd, TestRead10) {
  try {
    if (!exists(kSdList)) {
      GTEST_SKIP_("The test file doesn't exist");
    }
  } catch (const std::exception& err) {
    ASSERT_TRUE(false) << err.what();
  }

  const auto start_open = MdfHelper::NowNs();
  MdfReader reader((kSdList.data()));
  const auto stop_open = MdfHelper::NowNs();
  std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

  const auto start_info = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadEverythingButData());
  const auto stop_info = MdfHelper::NowNs();
  std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

  const auto *header = reader.GetHeader();
  ASSERT_TRUE(header != nullptr);

  auto data_group_list = header->DataGroups();
  ASSERT_EQ(data_group_list.size(), 2);

  auto *first_dg = data_group_list[0];
  ASSERT_TRUE(first_dg != nullptr);

  auto channel_groups = first_dg->ChannelGroups();
  ASSERT_FALSE(channel_groups.empty());

  auto *channel_group = channel_groups[0];
  ASSERT_TRUE(channel_group != nullptr);

  auto *time_channel = channel_group->GetMasterChannel();;
  ASSERT_TRUE(time_channel != nullptr);

  auto *write_channel = channel_group->GetChannel(kWriteSignalName);
  ASSERT_TRUE(write_channel != nullptr);

  auto time_observer = CreateChannelObserver(*first_dg,
                                             *channel_group, *time_channel);
  ASSERT_TRUE(time_observer);

  auto write_observer = CreateChannelObserver(*first_dg,
                                                    *channel_group, *write_channel);
  ASSERT_TRUE(write_observer);
  write_observer->ReadVlsdData(false);

  const auto start_data = MdfHelper::NowNs();
  EXPECT_TRUE(reader.ReadData(*first_dg)) << reader.ShortName();

  auto offset_list = write_observer->GetOffsetList();
  // Remove the first offset in the list and keep the last 10 offsets.
  while (offset_list.size() > 10) {
    offset_list.erase(offset_list.begin());
  }
  const auto nof_samples = offset_list.size();

  const auto stop_data = MdfHelper::NowNs();
  std::cout << "Read Offset Data [ms]: "
   << ConvertToMs(stop_data - start_data) << std::endl;

  size_t sample_count = 0;
  size_t byte_count = 0;
  std::function callback = [&](uint64_t offset, const std::vector<uint8_t> &buffer) {
    ++sample_count;
    EXPECT_GT(buffer.size(), 1);
    byte_count += buffer.size();
    for (size_t index = 0; sample_count < 10 && index < buffer.size(); ++index) {
      const char input = static_cast<const char>(buffer[index]);
      if (input != '\0') {
        std::cout << static_cast<char>(buffer[index]);
      }
    }
    if (sample_count < 10) {
      std::cout << std::endl;
    }
  };

  const auto start_vlsd = MdfHelper::NowNs();

  EXPECT_TRUE(reader.ReadVlsdData(*first_dg, *write_channel, offset_list, callback))
    << reader.ShortName();
  const auto stop_vlsd = MdfHelper::NowNs();
  EXPECT_EQ(sample_count, nof_samples);

  std::cout << "Read VLSD Data [ms]: "
    << ConvertToMs(stop_vlsd - start_vlsd) << std::endl;
  std::cout << "Byte Count [kB]: "
    << static_cast<double>(byte_count) / 1'000 << std::endl;

  first_dg->ClearData();
  time_observer.reset();
  write_observer.reset();
  std::cout << std::endl;
}

}  // namespace mdf::test