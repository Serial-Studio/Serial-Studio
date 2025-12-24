/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "testread.h"

#include <chrono>
#include <filesystem>
#include <map>
#include <string>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "mdf/mdfreader.h"
#include "mdf/mdflogstream.h"

#include "mdf3file.h"
#include "mdf4file.h"

#include <util/logconfig.h>
#include <util/logstream.h>
#include <util/stringutil.h>

using namespace std::filesystem;
using namespace util::string;
using namespace mdf;
using namespace mdf::detail;
using namespace util::log;
using namespace std::chrono_literals;
using namespace std::chrono;
using namespace boost::iostreams;

namespace {
const std::string mdf_source_dir =
    "k:/test/mdf";  // Where all source MDF files exist

constexpr std::string_view  kLargeLocalSsdFile =
    "c:/temp/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";
constexpr std::string_view  kLargeLocalDriveFile =
    "o:/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";
constexpr std::string_view  kLargeNasFile =
    "k:/test/mdf/Zeekr_PR60126_BX1E_583_R08A04_dia_left_5kph_0.5m_NO_08_01_psd_1_2023_12_21_041519#CANape_log_056_TDA4GMSL.mf4";

constexpr std::string_view kLargeTime = "t";
constexpr std::string_view kLargeFrameCounter = "FrameCounter_VC0";
constexpr std::string_view kLargeFrameData = "VideoRawdata_VC0";
constexpr std::string_view kBenchMarkFile = "K:/test/mdf/net/test.mf4";
constexpr std::string_view kCssTestFile = "css_test";
constexpr std::string_view kCanFdFile = "test_canfd";

using MdfList = std::map<std::string, std::string, util::string::IgnoreCase>;
MdfList kMdfList;

std::string GetMdfFile(const std::string &name) {
  auto itr = kMdfList.find(name);
  return itr == kMdfList.cend() ? std::string() : itr->second;
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

void MdfLogFunction(const MdfLocation& location, MdfLogSeverity severity,
                    const std::string& text) {
  LogStringEx(location.line, location.column, location.file, location.function,
              static_cast<LogSeverity>(severity), text);
}

}  // namespace

namespace mdf::test {

void TestRead::SetUpTestSuite() {
  // Set up the log file to log to console
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.Type(util::log::LogType::LogToConsole);
  log_config.CreateDefaultLogger();

  MdfLogStream::SetLogFunction1(MdfLogFunction);

  kMdfList.clear();
  try {
    for (const auto &entry : recursive_directory_iterator(mdf_source_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &fullname = entry.path();
      const auto stem = fullname.stem().string();
      if (kMdfList.find(stem) == kMdfList.cend() && IsMdfFile(fullname.string())) {
        std::cout << "Found MDF file. File: " << stem << std::endl;
        kMdfList.emplace(stem, fullname.string());
      }
    }
  } catch (const std::exception &error) {
    std::cout << "Failed to fetch the MDF test files. Error: "
                << error.what()  << std::endl;
  }
}

void TestRead::TearDownTestSuite() {
  MdfLogStream::SetLogFunction1(nullptr);
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestRead, Read)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found." );
  }

  for (const auto& [name, filename] : kMdfList) {
    const auto start1 = steady_clock::now();

    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.IsOk()) << name;
    const auto* mdf_file = oRead.GetFile();

    EXPECT_TRUE(mdf_file != nullptr) << name;
    EXPECT_TRUE(oRead.ReadEverythingButData()) << name;

    const auto stop1 = steady_clock::now();
    const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

    auto* dg_group = oRead.GetDataGroup(0);
    if (dg_group == nullptr) {
      continue;
    }
    const auto start2 = steady_clock::now();
    ISampleObserver observer(*dg_group);
    oRead.ReadData(*dg_group);

    const auto stop2 = steady_clock::now();
    const auto diff2 = duration_cast<milliseconds>(stop2 - start2);
    std::cout << "File: " << name << " (" << diff1.count()
              << "/" << diff2.count() << " ms)" << std::endl;
  }
}

TEST_F(TestRead, MdfFile)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }
  for (const auto &[name, filename] : kMdfList) {
    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << name;
    const auto *mdf_file = oRead.GetFile();
    ASSERT_TRUE(mdf_file != nullptr) << name;
    if (mdf_file->IsMdf4()) {
      const auto *file4 = dynamic_cast<const Mdf4File *>(mdf_file);
      ASSERT_TRUE(file4 != nullptr);
      const auto &hd4 = file4->Hd();
      const auto &dg_list = hd4.Dg4();
      LOG_INFO() << " File: " << name
                 << ", Nof Measurement: " << dg_list.size();
      EXPECT_FALSE(dg_list.empty()) << name;
    } else {
      const auto *file3 = dynamic_cast<const Mdf3File *>(mdf_file);
      EXPECT_TRUE(file3 != nullptr);
    }
  }
}

TEST_F(TestRead, IdBlock)  // NOLINT
{
  const std::string file = GetMdfFile("Vector_CustomExtensions_CNcomment");
  // Check that the file exists. If not, skip the test
  if (file.empty()) {
    GTEST_SKIP_("Comment file doesn't exist.");
  }
  MdfReader oRead(file);
  EXPECT_TRUE(oRead.IsOk()) << file;

  const auto *f = dynamic_cast<const Mdf4File *>(oRead.GetFile());
  EXPECT_TRUE(f != nullptr) << file;

  const auto &id = f->Id();
  EXPECT_STREQ(id.FileId().c_str(), "MDF");
  EXPECT_STREQ(id.VersionString().c_str(), "4.10");
  EXPECT_STREQ(id.ProgramId().c_str(), "MCD11.01");
  EXPECT_EQ(id.Version(), 410);
}

TEST_F(TestRead, HeaderBlock)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP();
  }
  int count = 0;
  for (const auto &[name, filename] : kMdfList) {
    MdfReader oRead(filename);
    EXPECT_TRUE(oRead.ReadEverythingButData()) << name;

    const auto *mdf_file = oRead.GetFile();
    if (!mdf_file->IsMdf4()) {
      continue;
    }
    const auto *mdf4_file = dynamic_cast<const Mdf4File *>(mdf_file);
    const auto &hd4 = mdf4_file->Hd();
    EXPECT_EQ(&hd4, hd4.HeaderBlock());

    for (const auto& dg4 : hd4.Dg4()) {
      EXPECT_EQ(&hd4, dg4->HeaderBlock());
      for (const auto& cg4 : dg4->Cg4()) {
        EXPECT_EQ(&hd4, cg4->HeaderBlock());
        for (const auto& cn4 : cg4->Cn4()) {
          EXPECT_EQ(&hd4, cn4->HeaderBlock());
        }
      }
    }
    ++count;
    // Only need to check 5 files
    if (count > 5) {
      break;
    }
  }
}

TEST_F(TestRead, Benchmark) {
  try {
    if (!exists(kBenchMarkFile)) {
      GTEST_SKIP();
    }
  } catch (const std::exception& ) {
    GTEST_SKIP();
  }

  {
    MdfReader oRead(kBenchMarkFile.data());
    const auto start = steady_clock::now();
    oRead.ReadMeasurementInfo();
    const auto stop = steady_clock::now();
    std::chrono::duration<double> diff = duration_cast<milliseconds>(stop - start);
    std::cout << "Read Measurement Info TrueNas: " << diff.count()
              << " ms" << std::endl;
  }
  {
    const auto start = steady_clock::now();
    MdfReader oRead(kBenchMarkFile.data());
    oRead.ReadEverythingButData();
    const auto *file = oRead.GetFile();
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    for (auto *dg : dg_list) {
      ChannelObserverList observer_list;
      auto cg_list = dg->ChannelGroups();
      for (const auto *cg : cg_list) {
        CreateChannelObserverForChannelGroup(*dg, *cg, observer_list);
      }
      oRead.ReadData(*dg);

      double eng_value = 0;
      bool valid = true;
      for (const auto &channel : observer_list) {
        size_t samples = channel->NofSamples();
        for (size_t sample = 0; sample < samples; ++sample) {
          valid = channel->GetEngValue(sample, eng_value);
        }
      }

    }

    const auto stop = steady_clock::now();
    std::chrono::duration<double> diff = duration_cast<milliseconds>(stop - start);
    std::cout << "Everything + Conversion (TrueNas): " << diff.count()
              << " ms" << std::endl;
  }
}

TEST_F(TestRead, TestLargeFile) {

  std::array<std::string_view, 3> file_list = {
      kLargeLocalSsdFile,
      kLargeLocalDriveFile,
      kLargeNasFile};
  std::array<std::string_view, 3> storage_list = {
      "Local SSD Drive",
      "Local Mechanical Drive",
      "NAS"
  };

  for (size_t file = 0; file < file_list.size(); ++file) {
    const std::string test_file(file_list[file]);
    const std::string storage_type(storage_list[file]);

    try {
      const auto full_path = path(test_file);
      if (!exists(full_path)) {
        throw std::runtime_error("File doesn't exist");
      }
    } catch (const std::exception&) {
      continue;
    }

    {
      std::cout << "BENCHMARK Large (" << storage_type << ") Storage ReadData(without VLSD data)" << std::endl;

      const auto start_open = MdfHelper::NowNs();
      MdfReader oRead(test_file);
      const auto stop_open = MdfHelper::NowNs();
      std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

      const auto start_info = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
      const auto stop_info = MdfHelper::NowNs();
      std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

      const auto *header = oRead.GetHeader();
      ASSERT_TRUE(header != nullptr);

      auto *last_dg = header->LastDataGroup();
      ASSERT_TRUE(last_dg != nullptr);

      auto channel_groups = last_dg->ChannelGroups();
      ASSERT_FALSE(channel_groups.empty());

      auto *channel_group = channel_groups[0];
      ASSERT_TRUE(channel_group != nullptr);

      auto *time_channel = channel_group->GetChannel(kLargeTime);
      ASSERT_TRUE(time_channel != nullptr);

      auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
      ASSERT_TRUE(counter_channel != nullptr);

      auto time_observer = CreateChannelObserver(*last_dg,
                                                 *channel_group, *time_channel);
      auto counter_observer = CreateChannelObserver(*last_dg,
                                                    *channel_group, *counter_channel);

      const auto start_data = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadData(*last_dg)) << oRead.ShortName();
      const auto stop_data = MdfHelper::NowNs();
      std::cout << "Read Data (s): " << ConvertToSec(stop_data - start_data) << std::endl;

      last_dg->ClearData();
      time_observer.reset();
      counter_observer.reset();
      std::cout << std::endl;
    }

    {
      std::cout << "BENCHMARK Large (" << storage_type << ") Storage ReadData(with VLSD data)" << std::endl;

      const auto start_open = MdfHelper::NowNs();
      MdfReader oRead(test_file);
      const auto stop_open = MdfHelper::NowNs();
      std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open) << std::endl;

      const auto start_info = MdfHelper::NowNs();
      EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
      const auto stop_info = MdfHelper::NowNs();
      std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info) << std::endl;

      const auto *header = oRead.GetHeader();
      ASSERT_TRUE(header != nullptr);

      auto *last_dg = header->LastDataGroup();
      ASSERT_TRUE(last_dg != nullptr);

      auto channel_groups = last_dg->ChannelGroups();
      ASSERT_FALSE(channel_groups.empty());

      auto *channel_group = channel_groups[0];
      ASSERT_TRUE(channel_group != nullptr);

      auto *time_channel = channel_group->GetChannel(kLargeTime);
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
      EXPECT_TRUE(oRead.ReadData(*last_dg)) << oRead.ShortName();
      auto offset_list = frame_observer->GetOffsetList();
      EXPECT_FALSE(offset_list.empty());
      const auto stop_data = MdfHelper::NowNs();
      std::cout << "Read Offset Data (s): " << ConvertToSec(stop_data - start_data) << std::endl;

//      size_t sample_count = 0;
      std::function callback = [&](uint64_t offset, const std::vector<uint8_t> &buffer) {
        const bool offset_exist = std::any_of(offset_list.cbegin(),offset_list.cend(),
                                              [&] (uint64_t off) {
          return off == offset;
        });
        EXPECT_TRUE(offset_exist) << "Offset: " << offset << std::endl;

//        std::cout << "Offset: " << offset << "/" << offset_list[sample_count]
//                  << ", Size: " << buffer.size() << std::endl;
//        ++sample_count;
      };
      const auto start_vlsd = MdfHelper::NowNs();
      while (offset_list.size() > 100) {
        offset_list.pop_back();
      }
      EXPECT_TRUE(oRead.ReadVlsdData(*last_dg, *frame_channel, offset_list, callback)) << oRead.ShortName();
      const auto stop_vlsd = MdfHelper::NowNs();

      std::cout << "Read 100 VLSD Data (s): " << ConvertToSec(stop_vlsd - start_vlsd) << std::endl;


      last_dg->ClearData();
      time_observer.reset();
      counter_observer.reset();

      std::cout << std::endl;
    }
  }
}

TEST_F(TestRead, TestExampleCrash) {
    const std::string crash_file = GetMdfFile("crashfile");
    if (crash_file.empty()) {
      GTEST_SKIP();
    }

    MdfReader reader(crash_file);  // Note the file is now open.

    // Read all blocks but not the raw data and attachments.
    // This reads in the block information into memory.
    EXPECT_TRUE(reader.ReadEverythingButData());

    const auto* mdf_file = reader.GetFile(); // Get the file interface.
    DataGroupList dg_list;                   // Get all measurements.
    mdf_file->DataGroups(dg_list);
    EXPECT_EQ(dg_list.size(), 2);

    // In this example, we read in all sample data and fetch all values.
    for (auto* dg4 : dg_list) {
      // Subscribers hold the sample data for a channel.
      // You should normally only subscribe on some channels.
      // We need a list to hold them.
      ChannelObserverList subscriber_list;
      const auto cg_list = dg4->ChannelGroups();
      for (const auto* cg4 : cg_list ) {
        const auto cn_list = cg4->Channels();
        for (const auto* cn4 : cn_list) {
          // Create a subscriber and add it to the temporary list
          auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
          EXPECT_TRUE(sub);
          subscriber_list.push_back(std::move(sub));
        }
      }

      // Now it is time to read in all samples
      EXPECT_TRUE(reader.ReadData(*dg4)); // Read raw data from the file.
      std::string channel_value; // Channel value (no scaling)
      std::string eng_value; // Engineering value
      for (auto& obs : subscriber_list) {
        EXPECT_GT(obs->NofSamples(), 0);
        std::cout << obs->Name() << " Samples: " << obs->NofSamples() << std::endl;
        for (size_t sample = 0; sample < obs->NofSamples(); ++sample) {
          const auto channel_valid = obs->GetChannelValue(sample, channel_value);
          const auto eng_valid = obs->GetEngValue(sample, eng_value);
          // You should do something with data here
          std::cout << sample << ": " << (eng_valid ? eng_value : "*") << std::endl;
        }
        std::cout << std::endl;
      }

      dg4->ClearData();
    }
    reader.Close(); // Close the file

}

TEST_F(TestRead, TestPartialRead) {
  try {
    const auto full_path = path(kLargeNasFile);
    if (!exists(full_path)) {
      throw std::runtime_error("File doesn't exist");
    }
  } catch (const std::exception &) {
    GTEST_SKIP();
  }

  std::cout << "Start reading large 22GB file (NAS storage." << std::endl;

  const auto start_open = MdfHelper::NowNs();
  MdfReader oRead(kLargeNasFile.data());
  const auto stop_open = MdfHelper::NowNs();
  std::cout << "Open File (ms): " << ConvertToMs(stop_open - start_open)
            << std::endl;

  const auto start_info = MdfHelper::NowNs();
  EXPECT_TRUE(oRead.ReadEverythingButData()) << oRead.ShortName();
  const auto stop_info = MdfHelper::NowNs();
  std::cout << "Read Info (ms): " << ConvertToMs(stop_info - start_info)
            << std::endl;

  const auto *header = oRead.GetHeader();
  auto *last_dg = header->LastDataGroup();
  auto channel_groups = last_dg->ChannelGroups();
  auto *channel_group = channel_groups[0];
  auto *time_channel = channel_group->GetChannel(kLargeTime);
  auto *counter_channel = channel_group->GetChannel(kLargeFrameCounter);
  auto *data_channel = channel_group->GetChannel(kLargeFrameData);

  auto time_observer =
      CreateChannelObserver(*last_dg, *channel_group, *time_channel);
  auto counter_observer =
      CreateChannelObserver(*last_dg, *channel_group, *counter_channel);
  auto data_observer =
      CreateChannelObserver(*last_dg, *channel_group, *data_channel);

  const auto start_data = MdfHelper::NowNs();
  EXPECT_TRUE(oRead.ReadPartialData(*last_dg, 0, 1)) << oRead.ShortName();
  const auto stop_data = MdfHelper::NowNs();
  std::cout << "Read 1 sample data (ms): "
            << ConvertToMs(stop_data - start_data) << std::endl;

  for (size_t sample = 0; sample < time_observer->NofSamples(); ++sample) {
    double time = 0;
    bool valid = time_observer->GetEngValue(sample, time);
    if (sample == 0) {
      EXPECT_TRUE(valid);
    } else {
      EXPECT_FALSE(valid);
    }
    std::vector<uint8_t> data;
    valid = data_observer->GetEngValue(sample, data);
    if (sample == 0) {
      EXPECT_GT(data.size(), 0);
      EXPECT_TRUE(valid);
    } else {
      EXPECT_FALSE(valid);
      EXPECT_TRUE(data.empty());
    }
  }

  last_dg->ClearData();
  time_observer.reset();
  counter_observer.reset();
  data_observer.reset();
}

TEST_F(TestRead, TestNotFinalized) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }

  // Find all files that are not finalized.
  MdfList test_list;
  for (const auto& [name, filename] : kMdfList ) {
    MdfReader reader(filename);
    if (!reader.IsFinalized()) {
      test_list.emplace(name, filename);
    }
  }
  EXPECT_FALSE(test_list.empty());
  for (const auto& [name1, filename1] : test_list) {
    MdfReader reader(filename1);
    EXPECT_TRUE(reader.ReadEverythingButData());
    EXPECT_TRUE(reader.IsFinalized()) << name1;
    std::cout << "Finalized File: " << name1 << std::endl;

  }

}

TEST_F(TestRead, TestCssChannelObservers) {
  const std::string& test_file = GetMdfFile(kCssTestFile.data());
  const auto find_test_file = kMdfList.find(kCssTestFile.data());
  if (find_test_file == kMdfList.cend()) {
    GTEST_SKIP_("No CSS test file found.");
  }

  const auto start1 = steady_clock::now();

  MdfReader reader(test_file);
  ASSERT_TRUE(reader.IsOk());
  reader.ReadEverythingButData();
  const auto stop1 = steady_clock::now();
  const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

  const auto* mdf_file = reader.GetFile();
  ASSERT_TRUE(mdf_file != nullptr);
  DataGroupList dg_list;
  mdf_file->DataGroups(dg_list);
  for (IDataGroup* data_group : dg_list) {
    if (data_group == nullptr) {
      continue;
    }
    ChannelObserverList observer_list;
    CreateChannelObserverForDataGroup(*data_group, observer_list);
    reader.ReadData(*data_group);
    data_group->ClearData();
  }
  reader.Close();

  const auto stop2 = steady_clock::now();
  const auto diff2 = duration_cast<milliseconds>(stop2 - stop1);

  std:: cout << "File: " << kCssTestFile << " (" << diff1.count()
            << "/" << diff2.count() << " ms)" << std::endl;

}

TEST_F(TestRead, TestCssSampleObserver) {
  const std::string& test_file = GetMdfFile(kCssTestFile.data());
  const auto find_test_file = kMdfList.find(kCssTestFile.data());
  if (find_test_file == kMdfList.cend()) {
    GTEST_SKIP_("No CSS test file found.");
  }

  const auto start1 = steady_clock::now();

  MdfReader reader(test_file);
  ASSERT_TRUE(reader.IsOk());
  reader.ReadEverythingButData();
  const auto stop1 = steady_clock::now();
  const auto diff1 = duration_cast<milliseconds>(stop1 - start1);

  const auto* mdf_file = reader.GetFile();
  ASSERT_TRUE(mdf_file != nullptr);
  DataGroupList dg_list;
  mdf_file->DataGroups(dg_list);
  for (IDataGroup* data_group : dg_list) {
    if (data_group == nullptr) {
      continue;
    }
    ISampleObserver observer(*data_group);

    reader.ReadData(*data_group);
  }
  reader.Close();

  const auto stop2 = steady_clock::now();
  const auto diff2 = duration_cast<milliseconds>(stop2 - stop1);

  std:: cout << "File: " << kCssTestFile << " (" << diff1.count()
            << "/" << diff2.count() << " ms)" << std::endl;

}


TEST_F(TestRead, TestStreamInterface)  // NOLINT
{
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files found.");
  }
  // Read into memory and then parse the memory.

  for (const auto &[name, filename] : kMdfList) {
    uint64_t file_size = 0;
    // Read in file into memory

    ASSERT_TRUE(IsMdfFile(filename));

    try {
      const path fullname(filename);
      file_size = std::filesystem::file_size(fullname);
     } catch (std::exception& err) {
      ADD_FAILURE() << err.what();
      continue;
    }

    if (file_size > 1'000'000'000) {
      // Skip large files.
      continue;
    }
    std::vector<char> file_array;
    file_array.reserve(file_size);

    back_insert_device<std::vector<char>> array_device(file_array);
    stream_buffer< back_insert_device<std::vector<char> >> out_buffer(array_device);
    std::ostream out_array(&out_buffer);

    std::ifstream in_file(filename, std::ios_base::in | std::ios_base::binary);
    for ( int input = in_file.get(); !in_file.eof(); input = in_file.get()  ) {
      if (input>= 0) {
        out_array.put(static_cast<char>(input));
      }
    }
    in_file.close();
    out_buffer.pubsync(); // Doing a flush of buffer to vector
    out_buffer.close();

    if (file_array.empty()) {
      ADD_FAILURE() << "File is empty. File: " << name;
      continue;
    }

    std::shared_ptr<std::streambuf> input_buffer =
        std::make_shared< stream_buffer<array_source> >(file_array.data(), file_array.size());

    ASSERT_TRUE(IsMdfFile(*input_buffer));

    MdfReader oRead(input_buffer);
    EXPECT_TRUE(oRead.ReadMeasurementInfo()) << name;
    const auto *mdf_file = oRead.GetFile();
    ASSERT_TRUE(mdf_file != nullptr) << name;
    if (mdf_file->IsMdf4()) {
      const auto *file4 = dynamic_cast<const Mdf4File *>(mdf_file);
      ASSERT_TRUE(file4 != nullptr) << name;
      const auto &hd4 = file4->Hd();
      const auto &dg_list = hd4.Dg4();
      std::cout <<"File: " << name << " (" << file_size / 1'000
                << " kB) , Nof Measurement: " << dg_list.size() << std::endl;
      EXPECT_FALSE(dg_list.empty()) << name;
    } else {
      const auto *file3 = dynamic_cast<const Mdf3File *>(mdf_file);
      EXPECT_TRUE(file3 != nullptr);
      ASSERT_TRUE(file3 != nullptr) << name;
      const auto &hd3 = file3->Hd();
      const auto &dg_list = hd3.Dg3();
      std::cout <<"File: " << name << " (" << file_size / 1'000
                << " kB) , Nof Measurement: " << dg_list.size() << std::endl;
      EXPECT_FALSE(dg_list.empty()) << name;
    }
  }
}

TEST_F(TestRead, TestCanFd) {
  std::string test_file = GetMdfFile(kCanFdFile.data());
  if (test_file.empty()) {
    GTEST_SKIP_("CAN FD file not found");
  }

  MdfReader reader(test_file);
  const bool read_config = reader.ReadEverythingButData();
  ASSERT_TRUE(read_config);

  const MdfFile* mf4_file = reader.GetFile();
  ASSERT_TRUE(mf4_file != nullptr);

  std::vector<IDataGroup*> dg_list;
  mf4_file->DataGroups(dg_list);
  EXPECT_EQ(dg_list.size(), 3);

  for (IDataGroup* data_group : dg_list ) {
    ASSERT_TRUE(data_group != nullptr);

    // Subscribe on all channels
    ChannelObserverList observer_list;
    CreateChannelObserverForDataGroup(*data_group, observer_list);
    EXPECT_FALSE(observer_list.empty());

    const bool read = reader.ReadData(*data_group);
    EXPECT_TRUE(read);

    for (uint64_t sample = 0; sample < 5; ++sample) {

      bool valid = false;
      uint16_t channel = 0;
      std::string direction;
      uint32_t can_id = 0;
      std::vector<uint8_t> data_bytes;

      for (const auto &observer : observer_list) {
        if (observer->Name() == "CAN_DataFrame.BusChannel") {
          valid = observer->GetEngValue(sample, channel);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.Dir") {
          valid = observer->GetEngValue(sample, direction);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.ID") {
          valid = observer->GetEngValue(sample, can_id);
          EXPECT_TRUE(valid);
        } else if (observer->Name() == "CAN_DataFrame.DataBytes") {
          valid = observer->GetEngValue(sample, data_bytes);
          EXPECT_TRUE(valid);
        }
      }
      if (valid) {
        std::cout << "Sample: " << sample
                  << " Channel: " << channel
                  << " Direction: " << direction
                  << " CAN ID: " << can_id
                  << " Data: ";
        std::ostringstream hex; // Needs temporary stream as I change format.
        for (uint8_t data : data_bytes) {
          hex << std::hex << std::uppercase << std::setw(2)
                    << std::setfill('0') << static_cast<int>(data) << " ";
        }
        std::cout << hex.str() << std::endl;

      }
    }

  }
}

TEST_F(TestRead, TestReadData) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No MDF files.");
  }
  size_t read_count = 0;
  for (const auto& [name, test_file]: kMdfList) {
    MdfReader reader(test_file);
    const bool read_config = reader.ReadEverythingButData();
    ASSERT_TRUE(read_config) << name;

    const MdfFile* mf4_file = reader.GetFile();
    ASSERT_TRUE(mf4_file != nullptr);

    std::vector<IDataGroup*> dg_list;
    mf4_file->DataGroups(dg_list);
    if (dg_list.empty()) {
      LOG_TRACE() << "No data groups in the test file. File: " << test_file;
      continue;
    }
    const auto test_file_size = file_size(test_file);
    if (test_file_size > 10'000'000) {
     // LOG_TRACE() << "Skipped file due to size. File: " << name
     //     << ", Size: " << static_cast<double>(test_file_size)/ 1E6 << "MB";
      continue;
    }
    for (IDataGroup* data_group : dg_list) {
      ASSERT_TRUE(data_group != nullptr);
      ChannelObserverList observers;
      CreateChannelObserverForDataGroup(*data_group, observers);
      EXPECT_TRUE(reader.ReadData(*data_group)) << test_file;
    }
    ++read_count;
  }
  std::cout << "Tested " << read_count << " files." << std::endl;
}

}  // namespace mdf::test
