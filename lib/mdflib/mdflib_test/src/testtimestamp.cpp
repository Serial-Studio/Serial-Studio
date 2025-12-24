/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "testtimestamp.h"

#include <string>
#include <filesystem>

#include "util/logconfig.h"
#include "util/logstream.h"
#include "util/timestamp.h"

#include "mdf/mdflogstream.h"
#include "mdf/mdfreader.h"

using namespace util::log;
using namespace util::time;
using namespace std::filesystem;
using namespace mdf;

namespace {

std::string kTestRootDir; ///< Test root directory
std::string kTestDir; ///<  <Test Root Dir>/mdf/write";

constexpr uint64_t kNanosecondsPerSecond = 1'000'000'000;
constexpr uint64_t kNanosecondsPerMinute = 60 * kNanosecondsPerSecond;
constexpr uint64_t kNanosecondsPerHour = 60 * kNanosecondsPerMinute;
constexpr uint32_t kSecondsPerMinute = 60;
constexpr uint32_t kSecondsPerHour = 60 * kSecondsPerMinute;

/**
 * Function that connect the MDF library to the UTIL library log functionality.
 * @param location Source file and line location
 * @param severity Severity code
 * @param text Log text
 */
void LogFunc(const MdfLocation& , MdfLogSeverity severity,
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
void NoLog(const MdfLocation& , MdfLogSeverity ,
           const std::string& ) {
}

void CreateMdfWithTime(const std::string& filepath, MdfWriterType writerType,
                       ITimestamp& timestamp) {
  auto writer = MdfFactory::CreateMdfWriter(writerType);
  writer->Init(filepath);
  auto* header = writer->Header();
  ASSERT_TRUE(header != nullptr);
  header->StartTime(timestamp);
  writer->InitMeasurement();
  writer->StartMeasurement(timestamp);
  writer->StopMeasurement(timestamp);
  writer->FinalizeMeasurement();
}

void TestMdf3Time(const std::string& filepath, uint64_t time) {
  MdfReader reader(filepath);
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file1 = reader.GetFile();
  ASSERT_TRUE(file1 != nullptr);
  const auto* header1 = file1->Header();
  ASSERT_TRUE(header1 != nullptr);

  EXPECT_EQ(header1->StartTime(), time) << filepath;
}

void TestMdf4Time(const std::string& filepath, uint64_t time,
                  uint16_t tz_offset_min, uint16_t dst_offset_min) {
  MdfReader reader(filepath);
  ASSERT_TRUE(reader.IsOk());
  ASSERT_TRUE(reader.ReadHeader());
  const auto* file = reader.GetFile();
  const auto* header = file->Header();
  ASSERT_TRUE(header != nullptr);
  const auto* local_ts = header->StartTimestamp();
  EXPECT_EQ(local_ts->GetTimeNs(), time);
  EXPECT_EQ(local_ts->GetDstOffsetMin(), dst_offset_min);
  EXPECT_EQ(local_ts->GetTzOffsetMin(), tz_offset_min);
}

}  // namespace

namespace mdf::test {
void TestTimestamp::SetUpTestSuite() {
  try {
    // Create the root asn log directory. Note that this directory
    // exists in the temp dir of the operating system and is not
    // deleted by this test program. May be deleted at restart
    // of the operating system.
    auto temp_dir = temp_directory_path();
    temp_dir.append("test");
    kTestRootDir = temp_dir.string();
    create_directories(temp_dir); // Not deleted


    // Log to the console only
    auto& log_config = LogConfig::Instance();
    log_config.Type(LogType::LogToConsole);
    log_config.CreateDefaultLogger();

     // Connect MDF library to the util logging functionality
    MdfLogStream::SetLogFunction1(LogFunc);

    // Create the test directory. Note that this directory is deleted before
    // running the test, not after. This give the
    temp_dir.append("mdf");
    temp_dir.append("timestamp");
    std::error_code err;
    remove_all(temp_dir, err);
    if (err) {
      MDF_TRACE() << "Remove error. Message: " << err.message();
    }
    create_directories(temp_dir);
    kTestDir = temp_dir.string();
    MDF_TRACE() << "Created the test directory. Dir: " << kTestDir;

  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create test directories. Error: " << err.what();
  }
}

void TestTimestamp::TearDownTestSuite() {
  MDF_TRACE() << "Tear down the test suite.";
  MdfLogStream::SetLogFunction1(NoLog);
  LogConfig& log_config = LogConfig::Instance();
  log_config.DeleteLogChain();
}

TEST_F(TestTimestamp, GmtOffsetNs) {
  int64_t offset = MdfHelper::GmtOffsetNs();
  std::cout << "GOT Offset (h): "
            << offset / 3'600'000'000'000 << std::endl;
  int64_t dst_offset = MdfHelper::GmtOffsetNs();
  std::cout << "DST Offset (h): "
            << dst_offset / 3'600'000'000'000 << std::endl;
}

TEST_F(TestTimestamp, UtcTimestamp) {
  UtcTimestamp now;
  EXPECT_GT(now.GetTimeNs(), 0);
  EXPECT_EQ(now.GetDstMin(), 0);
  EXPECT_EQ(now.GetTimezoneMin(), 0);
  EXPECT_EQ(now.GetUtcTimeNs(), now.GetTimeNs());

  const auto utc_now = now.GetUtcTimeNs();
  UtcTimestamp now1(utc_now);
  EXPECT_EQ(now1.GetTimeNs(), utc_now);
  EXPECT_EQ(now1.GetDstMin(), 0);
  EXPECT_EQ(now1.GetTimezoneMin(), 0);
  EXPECT_EQ(now1.GetUtcTimeNs(), utc_now);

  const auto iso_date = now.ToIsoDate();
  std::cout << "ISO Date: " << iso_date << std::endl;
  EXPECT_EQ(iso_date.size(), 11);

  const auto iso_time = now.ToIsoTime(false);
  std::cout << "ISO Time: " << iso_time << std::endl;
  EXPECT_EQ(iso_time.size(), 9);

  const auto iso_date_time = now.ToIsoDateTime(false);
  std::cout << "ISO Date Time: " << iso_date_time << std::endl;
  EXPECT_EQ(iso_date_time.size(), 20);

  const auto iso_time_ms = now.ToIsoTime(true);
  std::cout << "ISO Time: " << iso_time_ms << std::endl;

  const auto iso_date_time_ms = now.ToIsoDateTime(true);
  std::cout << "ISO Date Time: " << iso_date_time_ms << std::endl;
}

TEST_F(TestTimestamp, IsoTime) {
  UtcTimestamp now;
  EXPECT_GT(now.GetTimeNs(), 0);
  EXPECT_EQ(now.GetUtcTimeNs(), now.GetTimeNs());
  const uint64_t utc_now = now.GetUtcTimeNs();
  const std::string iso_now = now.ToIsoDateTime(true);

  std::cout << "ISO Date/Time: " << iso_now << std::endl;
  UtcTimestamp now1;
  now1.FromIsoDateTime(iso_now);

  std::cout << "Diff (ms): "
    << (int) (now1.GetUtcTimeNs() / 1'000'000)
                                - (int) (utc_now / 1'000'000) << std::endl;

  EXPECT_EQ(now1.GetUtcTimeNs() / 1'000'000, utc_now / 1'000'000);
  EXPECT_EQ(iso_now, now1.ToIsoDateTime(true));

  UtcTimestamp now2;
  now2.FromIsoDateTime("19700102Z");
  EXPECT_EQ(now2.GetUtcTimeNs() / 1'000'000'000, 24*3600);

  now2.FromIsoDateTime("1970-01-02 00:00:00+01:00");
  EXPECT_EQ(now2.GetUtcTimeNs() / 1'000'000'000, 23*3600);
}

TEST_F(TestTimestamp, Mdf3TimeStamp) {

  const int64_t now = static_cast<int64_t >(time(nullptr)) * 1'000'000'000;

  const int64_t local_time = now + MdfHelper::GmtOffsetNs()
                              + MdfHelper::DstOffsetNs();
  const int64_t mdf3_time = now + MdfHelper::GmtOffsetNs();

  std::cout << "Offset (h): " <<
      (local_time - now) / 3'600'000'000'000 << std::endl;

  // local time
  path local_path(kTestDir);
  local_path.append("mf3_local.mf3");
  LocalTimestamp local_timestamp(local_time);
  CreateMdfWithTime(local_path.string(), MdfWriterType::Mdf3Basic, local_timestamp);
  TestMdf3Time(local_path.string(), mdf3_time );

  // utc time
  path utc_path(kTestDir);
  utc_path.append("mf3_utc.mf3");
  UtcTimestamp utc_time(now);
  CreateMdfWithTime(utc_path.string(), MdfWriterType::Mdf3Basic, utc_time);
  TestMdf3Time(utc_path.string(), mdf3_time );

  // timezone time
  auto tz_offset_min = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                            kNanosecondsPerMinute);
  auto dst_offset_min = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                            kNanosecondsPerMinute);
  path tz_path(kTestDir);
  tz_path.append("mf3_tz.mf3");
  TimezoneTimestamp timezone_timestamp(now * 1'000'000'000, tz_offset_min,
                                       dst_offset_min);
  CreateMdfWithTime(tz_path.string(), MdfWriterType::Mdf3Basic, local_timestamp);
  TestMdf3Time(tz_path.string(), mdf3_time);
}

TEST_F(TestTimestamp, Mdf4TimeStamp) {
  const auto start_time = TimeStampToNs();

  // local time
  path local_path(kTestDir);
  local_path.append("mf4_local.mf4");
  LocalTimestamp local_timestamp(start_time);
  CreateMdfWithTime(local_path.string(), MdfWriterType::Mdf4Basic,
                    local_timestamp);
  TestMdf4Time(local_path.string(), start_time, 0, 0);

  // utc time
  path utc_path(kTestDir);
  utc_path.append("mf4_utc.mf4");
  UtcTimestamp utc_time(start_time);
  CreateMdfWithTime(utc_path.string(), MdfWriterType::Mdf4Basic, utc_time);
  MdfReader utc_reader(utc_path.string());
  TestMdf4Time(utc_path.string(), start_time, 0, 0);


  // timezone time
  auto tz_offset_min = static_cast<int16_t>(MdfHelper::GmtOffsetNs() /
                                            kNanosecondsPerMinute);
  auto dst_offset_min = static_cast<int16_t>(MdfHelper::DstOffsetNs() /
                                             kNanosecondsPerMinute);
  path tz_path(kTestDir);
  tz_path.append("mf4_tz.mf4");
  TimezoneTimestamp timezone_timestamp(start_time, tz_offset_min,
                                       dst_offset_min);
  CreateMdfWithTime(tz_path.string(), MdfWriterType::Mdf4Basic,
                    timezone_timestamp);
  TestMdf4Time(tz_path.string(), start_time, tz_offset_min, dst_offset_min);
}

}  // namespace mdf