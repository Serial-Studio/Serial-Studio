/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "testcanbusobserver.h"

#include <filesystem>
#include <vector>
#include <memory>

#include "util/logconfig.h"
#include "util/logstream.h"

#include <mdf/mdfreader.h>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include <mdf/canbusobserver.h>
#include <mdf/mdflogstream.h>

using namespace std::filesystem;
using namespace util::log;
using namespace mdf;

namespace {

const std::string mdf_source_dir = "k:/test/mdf";
using MdfList = std::map<std::string, std::string>;
MdfList kMdfList;
size_t kNofMessages = 0;

bool TestBusType(const std::string& filename) {
  MdfReader reader(filename);
  if (!reader.IsOk()) {
    return false;
  }
  if (!reader.ReadEverythingButData() ) {
    return false;
  }
  const auto* mdf_file = reader.GetFile();
  if (mdf_file == nullptr) {
    return false;
  }

  DataGroupList dg_list;
  mdf_file->DataGroups(dg_list);
  for (const auto* data_group : dg_list) {
    if (data_group == nullptr) {
      continue;
    }
    for (const auto* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr) {
        continue;
      }
      if (channel_group->GetBusType() == BusType::Can) {
        return true;
      }
    }
  }
  return false;
}

void LogMdf(const MdfLocation &, MdfLogSeverity severity,
            const std::string &text) {
  if  (severity <= mdf::MdfLogSeverity::kTrace ) {
    return;
  }
  LogStream(Loc::current(), static_cast<LogSeverity>(severity)) << text;
}

void NoLogMdf(const MdfLocation &, MdfLogSeverity ,
              const std::string &) {

}

bool OnSampleCallback(uint64_t , const CanMessage& ) {
  ++kNofMessages;
  return true;
}

} // end namespace

namespace mdf::test {

void TestCanBusObserver::SetUpTestSuite() {
  // Set up the log file to log to console
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.Type(util::log::LogType::LogToConsole);
  log_config.CreateDefaultLogger();

  MdfLogStream::SetLogFunction1(LogMdf);


  kMdfList.clear();
  try {
    for (const auto &entry : recursive_directory_iterator(mdf_source_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      const auto &fullname = entry.path();
      const auto stem = fullname.stem().string();
      const auto filename = fullname.string();
      if (kMdfList.find(stem) == kMdfList.cend() &&
          IsMdfFile(fullname.string()) &&
          TestBusType(filename) ) {
        std::cout << "Found a CAN MDF file. File: " << stem << std::endl;
        kMdfList.emplace(stem, filename);
      }
    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "Didn't read the CAN MDF test files. Error: "
              << error.what()  << std::endl;
  }
}

void TestCanBusObserver::TearDownTestSuite() {
  MdfLogStream::SetLogFunction1(NoLogMdf);
  util::log::LogConfig &log_config = util::log::LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestCanBusObserver, TestBusType) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No CAN MDF files found.");
  }
  std::cout << "Found " << kMdfList.size() << " CAN bus files" << std::endl;
}

TEST_F(TestCanBusObserver, TestCanBusObserver_Callback) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No CAN MDF files found.");
  }
  for (const auto& [stem, test_file] : kMdfList) {
    MdfReader reader(test_file);
    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData() );
    const auto* mdf_file = reader.GetFile();
    ASSERT_TRUE(mdf_file != nullptr);
    DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);

    for (auto* data_group : dg_list) {
      if (data_group == nullptr) {
        continue;
      }
      size_t expected_nof_messages = 0;
      kNofMessages = 0;
      std::vector<std::unique_ptr<CanBusObserver>> observer_list;
      for (const auto* channel_group : data_group->ChannelGroups()) {
        if (channel_group == nullptr) {
          continue;
        }
        if (channel_group->GetBusType() == BusType::Can &&
            channel_group->NofSamples() > 0 &&
            (channel_group->Flags() & CgFlag::VlsdChannel) == 0) {
           auto observer = std::make_unique<CanBusObserver>(*data_group,
                                                           *channel_group);

           if (observer->NofSamples() == 0) {
             // No payload channel found.
             continue;
           }
           observer->OnCanMessage = OnSampleCallback;
           expected_nof_messages += observer->NofSamples();
           observer_list.push_back(std::move(observer));
        }
      }

      if (observer_list.empty()) {
        continue;
      }

      EXPECT_TRUE( reader.ReadData(*data_group) );
      std::cout << "Reading data. File: " << stem
                << ", Messages: " << kNofMessages
                << " (" << test_file << ")" << std::endl;
      EXPECT_EQ(kNofMessages, expected_nof_messages) << test_file;
    }
  }
}

TEST_F(TestCanBusObserver, TestCanBusObserver_NoCallback) {
  if (kMdfList.empty()) {
    GTEST_SKIP_("No CAN MDF files found.");
  }
  for (const auto& [stem, test_file] : kMdfList) {
    MdfReader reader(test_file);
    ASSERT_TRUE(reader.IsOk());
    ASSERT_TRUE(reader.ReadEverythingButData() );
    const auto* mdf_file = reader.GetFile();
    ASSERT_TRUE(mdf_file != nullptr);
    DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);

    for (auto* data_group : dg_list) {
      if (data_group == nullptr) {
        continue;
      }
      size_t expected_nof_messages = 0;
      kNofMessages = 0;
      std::vector<std::unique_ptr<CanBusObserver>> observer_list;
      for (const auto* channel_group : data_group->ChannelGroups()) {
        if (channel_group == nullptr) {
          continue;
        }
        if (channel_group->GetBusType() == BusType::Can &&
            channel_group->NofSamples() > 0 &&
            (channel_group->Flags() & CgFlag::VlsdChannel) == 0) {
          auto observer = std::make_unique<CanBusObserver>(*data_group,
                                                           *channel_group);

          if (observer->NofSamples() == 0) {
            // No payload channel found.
            continue;
          }
          expected_nof_messages += observer->NofSamples();
          observer_list.push_back(std::move(observer));
        }
      }

      if (observer_list.empty()) {
        continue;
      }

      EXPECT_TRUE( reader.ReadData(*data_group) );
      for (const auto& observer : observer_list) {
        for (uint64_t sample = 0; sample < observer->NofSamples(); ++sample) {
          if (const CanMessage* msg = observer->GetCanMessage(sample);
              msg != nullptr) {
            ++kNofMessages;
          }
        }
      }
      std::cout << "Reading data. File: " << stem
                << ", Messages: " << kNofMessages
                << " (" << test_file << ")" << std::endl;
      EXPECT_EQ(kNofMessages, expected_nof_messages) << test_file;
    }
  }
}

}  // namespace mdf::test