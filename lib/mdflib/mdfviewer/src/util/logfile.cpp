/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "logfile.h"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>

#include "logconfig.h"
#include "logging.h"
#include "logstream.h"
#include "timestamp.h"

using namespace std::filesystem;
using namespace std::chrono_literals;

namespace {

std::string GetStem(const std::string &file) {
  try {
    std::filesystem::path p(file);
    return p.stem().string();
  } catch (const std::exception &) {
  }
  return file;
}

/**
 * Returns the preferred program data root dir. In windows this is
 * 'c:\programdata'.
 * @return empty string or the preferred application data directory.
 */

std::string FindLogPath(const std::string &base_name) {
  try {
    auto &log_config = util::log::LogConfig::Instance();

    path filename(base_name.empty() ? log_config.BaseName() : base_name);

    // Add an extension if it is missing
    if (!filename.has_extension()) {
      filename.replace_extension(".log");
    }

    // If the user supply a full path, then use the path as it is
    if (filename.has_root_path()) {
      return filename.string();
    }

    path root_dir(log_config.RootDir());
    // Try program data path
    if (root_dir.empty()) {
      root_dir = util::log::ProgramDataPath();
    }
    // Still empty. Use temp path
    if (root_dir.empty()) {
      root_dir = temp_directory_path();
    }
    std::string sub_dir = log_config.SubDir();
    if (!sub_dir.empty()) {
      root_dir.append(sub_dir);
    }
    create_directories(root_dir);

    root_dir.append(filename.string());
    return root_dir.string();
  } catch (const std::exception &error) {
    std::cerr << "FindLogPath(). Error: " << error.what() << std::endl;
    throw error;  // No meaning to log the error to file
  }
}
}  // namespace

namespace util::log::detail {

std::string LogFile::Filename() const { return filename_; }

LogFile::LogFile() noexcept {
  EnableSeverityLevel(LogSeverity::kTrace,
                      false);  // Turn of trace level by default
  InitLogFile("");
}

LogFile::LogFile(const std::string &base_name) {
  EnableSeverityLevel(LogSeverity::kTrace,
                      false);  // Turn of trace level by default
  InitLogFile(base_name);
}

LogFile::~LogFile() {
  stop_thread_ = true;
  if (worker_thread_.joinable()) {
    condition_.notify_one();
    worker_thread_.join();
  }
  stop_thread_ = false;
  if (file_ != nullptr) {
    fclose(file_);
  }
}

void LogFile::StartWorkerThread() {
  stop_thread_ = false;
  worker_thread_ = std::thread(&LogFile::WorkerThread, this);
}

void LogFile::WorkerThread() {
  try {
    BackupFiles(filename_);
  } catch (const std::exception &error) {
    LOG_ERROR() << "Failed to backup log files at start. Error: "
                << error.what();
  }

  do {
    std::unique_lock<std::mutex> lock(locker_);
    condition_.wait_for(lock, 1000ms, [&] { return stop_thread_.load(); });

    int message_count = 0;
    for (; !message_list_.empty() && message_count <= 10000; ++message_count) {
      LogMessage m = message_list_.front();
      message_list_.pop();
      lock.unlock();
      HandleMessage(m);
      lock.lock();
    }
    if (file_ != nullptr) {
      std::fclose(file_);
      file_ = nullptr;
    }
    try {
      if (message_count > 0) {
        path p(filename_);
        if (file_ == nullptr && exists(p) && file_size(p) > 10'000'000) {
          BackupFiles(filename_);
        }
      }
    } catch (const std::exception &error) {
      LOG_ERROR() << "Failed to backup log files. Error: " << error.what();
    }
  } while (!stop_thread_);

  while (!message_list_.empty()) {
    LogMessage m = message_list_.front();
    message_list_.pop();
    HandleMessage(m);
  }

  if (file_ != nullptr) {
    std::fclose(file_);
    file_ = nullptr;
  }
}

void LogFile::HandleMessage(const LogMessage &m) {
  if (file_ == nullptr) {
    file_ = fopen(filename_.c_str(), "at");
  }
  if (file_ == nullptr) {
    return;
  }
  if (m.message.empty()) {
    return;
  }
  const char last = m.message.back();
  const bool has_newline = last == '\n' || last == '\r';
  const std::string time = time::GetLocalTimestampWithMs(m.timestamp);
  const std::string severity = GetSeverityString(m.severity);

  std::ostringstream temp;
  temp << "[" << time << "] " << severity << " ";
  if (has_newline) {
    std::string text = m.message;
    text.pop_back();
    temp << text << " ";
  } else {
    temp << m.message << " ";
  }
  if (ShowLocation()) {
    temp << "    [" << GetStem(m.location.file_name()) << ":"
         << m.location.function_name() << ":" << m.location.line() << "]";
  }
  temp << std::endl;
  std::fwrite(temp.str().data(), 1, temp.str().size(), file_);
}

/**
 * Adds a log message to the internal message queue. The queue is saved to a
 * file by a worker thread.
 * @param [in] message Message to handle.
 */
void LogFile::AddLogMessage(const LogMessage &message) {
  if (stop_thread_ || !IsSeverityLevelEnabled(message.severity)) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(locker_);
    if (stop_thread_) {
      return;
    }
    message_list_.push(message);
  }
  condition_.notify_one();
}

/**
 * Stops the working thread. This means that all messages in the queue is saved
 * to the file.
 */
void LogFile::Stop() {
  stop_thread_ = true;
  if (worker_thread_.joinable()) {
    condition_.notify_one();
    worker_thread_.join();
  }
  stop_thread_ = false;
  if (file_ != nullptr) {
    fclose(file_);
  }
}

void LogFile::InitLogFile(const std::string &base_name) {
  try {
    stop_thread_ = true;
    if (worker_thread_.joinable()) {
      condition_.notify_one();
      worker_thread_.join();
    }
    stop_thread_ = false;
    if (file_ != nullptr) {
      fclose(file_);
      file_ = nullptr;
    }
    filename_ = {};
    path p = FindLogPath(base_name);
    if (p.empty()) {
      throw std::ios_base::failure(std::string("Path is empty. Path: ") +
                                   p.string());
    }
    if (!exists(p.parent_path())) {
      throw std::ios_base::failure(std::string("Path does not exist Path: ") +
                                   p.string());
    }
    filename_ = p.string();
    StartWorkerThread();
  } catch (const std::exception &error) {
    std::cerr << "Couldn't initiate a log file. Error: " << error.what()
              << std::endl;
  }
}

bool LogFile::HasLogFile() const { return !filename_.empty(); }

}  // namespace util::log::detail
