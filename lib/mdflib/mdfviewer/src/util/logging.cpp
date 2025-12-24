/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <filesystem>
#include <vector>

#include "logconfig.h"
#include "logging.h"
#include "logstream.h"

namespace {

void SendLogMessage(const util::log::LogMessage &m) {
  const auto &log_config = util::log::LogConfig::Instance();
  log_config.AddLogMessage(m);
}

}  // namespace
namespace util::log {

void LogDebug(const Loc &loc, const char *fmt, ...) {
  if (fmt == nullptr) {
    return;
  }
  char buffer[5000]{};
  std::va_list lsArg;
  va_start(lsArg, fmt);
  std::vsnprintf(buffer, 5000, fmt, lsArg);
  va_end(lsArg);
  buffer[4999] = '\0';

  LogString(loc, LogSeverity::kDebug, buffer);
}

void LogInfo(const Loc &loc, const char *fmt, ...) {
  if (fmt == nullptr) {
    return;
  }
  char buffer[5000]{};
  std::va_list lsArg;
  va_start(lsArg, fmt);
  std::vsnprintf(buffer, 5000, fmt, lsArg);
  va_end(lsArg);
  buffer[4999] = '\0';

  LogString(loc, LogSeverity::kInfo, buffer);
}

void LogError(const Loc &loc, const char *fmt, ...) {
  if (fmt == nullptr) {
    return;
  }
  char buffer[5000]{};
  std::va_list lsArg;
  va_start(lsArg, fmt);
  std::vsnprintf(buffer, 5000, fmt, lsArg);
  va_end(lsArg);
  buffer[4999] = '\0';

  LogString(loc, LogSeverity::kError, buffer);
}

void LogString(const Loc &loc, LogSeverity severity,
               const std::string &message) {
  LogMessage m;
  m.message = message;
  m.location = loc;
  m.severity = severity;

  SendLogMessage(m);
}

std::string FindNotepad() {
  std::string note;
  // 1. Find the path to the 'notepad++.exe'
  try {
    auto notepad = boost::process::environment::find_executable("notepad++");
    if (!notepad.string().empty()) {
      note = notepad.string();
    }
  } catch (const std::exception &) {
    note.clear();
  }

  if (!note.empty()) {
    return note;
  }

  // 2. Find the path to the 'notepad.exe'
  try {
    auto notepad = boost::process::environment::find_executable("notepad");
    if (!notepad.string().empty()) {
      note = notepad.string();
    }
  } catch (const std::exception &) {
    note.clear();
  }

  if (!note.empty()) {
    return note;
  }

  // 3. Find the path to the 'gedit' GNOME editor
  try {
    auto notepad = boost::process::environment::find_executable("gedit");
    if (!notepad.string().empty()) {
      note = notepad.string();
    }
  } catch (const std::exception &) {
    note.clear();
  }
  return note;
}

bool BackupFiles(const std::string &filename, bool remove_file) {
  if (filename.empty()) {
    LOG_ERROR() << "File name is empty. Illegal use of function";
    return false;
  }

  try {
    const std::filesystem::path full(filename);
    const std::filesystem::path parent(full.parent_path());
    const std::filesystem::path stem(full.stem());
    const std::filesystem::path ext(full.extension());
    if (!std::filesystem::exists(full)) {
      return true;  // No meaning to back up if original doesn't exist.
    }
    // shift all file xxx_N -> xxx_N-1 and last xxx -> xxx_0
    for (int ii = 9; ii >= 0; --ii) {
      std::ostringstream temp1;
      temp1 << stem.string() << "_" << ii << ext.string();

      std::filesystem::path file1(parent);
      file1.append(temp1.str());
      if (std::filesystem::exists(file1) && ii == 9) {
        std::filesystem::remove(file1);
      }
      if (ii == 0) {
        if (remove_file) {
          std::filesystem::rename(full, file1);
        } else {
          std::filesystem::copy(full, file1);
        }
      } else {
        std::ostringstream temp2;
        temp2 << stem.string() << "_" << ii - 1 << ext.string();
        std::filesystem::path file2(parent);
        file2.append(temp2.str());
        if (std::filesystem::exists(file2)) {
          std::filesystem::rename(file2, file1);
        }
      }
    }
  } catch (const std::exception &error) {
    LOG_ERROR() << "Backup of file failed. Error: " << error.what()
                << ", File: " << filename;
    return false;
  }
  return true;
}

}  // namespace util::log
