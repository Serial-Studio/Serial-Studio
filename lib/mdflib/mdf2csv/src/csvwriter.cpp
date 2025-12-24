/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "csvwriter.h"
#include <filesystem>
#include <iostream>
#include <mdf/mdflogstream.h>

using namespace std::filesystem;

namespace mdf {

CsvWriter::CsvWriter(const std::string& filename)
: dest_filename_(filename) {
  try {
    path fullname(filename);
    path parent_path = fullname.parent_path();
    parent_path_ = parent_path.string();
    name_ = fullname.filename().string();

    path temp_path = temp_directory_path();
    temp_path.append(name_);
    temp_filename_ = temp_path.string();

    if (!exists(parent_path_)) {
      create_directories(parent_path_);
    }

    is_ok_ = true;
  } catch (const std::exception& err) {
    MDF_ERROR() << "CSV file path error. Error: " << err.what();
  }
}

CsvWriter::~CsvWriter() {
  try {
    if (file_.is_open()) {
      file_.close();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Fail to close file: Error: " << err.what();
  }
}

void CsvWriter::Convert(ChannelObserverList& observer_list) {
  std::vector<const IChannelObserver*> obs_list;

  // Add master channel(s) first in the list
  for (const auto& itr : observer_list) {
    const IChannelObserver* obs = itr.get();
    if (obs == nullptr) {
      continue;
    }
    if (obs->IsMaster()) {
      obs_list.emplace_back(obs);
      // Should be a break here as it should only exist one master
      // but some files has marked more channels as master as it
      // may exist in a real application.
    }
  }

  // Append non-master channels to the list
  for (const auto& itr : observer_list) {
    const IChannelObserver* obs = itr.get();
    if (obs == nullptr) {
      continue;
    }
    if (!obs->IsMaster()) {
      obs_list.emplace_back(obs);
    }
  }

  if (obs_list.empty()) {
    return;
  }

  size_t nof_samples = observer_list[0]->NofSamples();
  if (nof_samples == 0) {
    return;
  }

  try {
    file_.open(temp_filename_, std::ios::out | std::ios::trunc);
    // HEADER LINE
    size_t count = 0;
    for (const auto* header : obs_list) {
      const IChannel& channel = header->Channel();
      const auto unit = channel.Unit();
      if (count > 0) {
        file_ << delimiter_;
      }
      file_ << "\"" << channel.Name();
      if (!unit.empty()) {
        file_ << " [" << unit << "]";
      }
      file_ << "\"";
      ++count;
    }
    file_ << std::endl;

    // ROW LINE
    for (size_t sample = 0; sample < nof_samples; ++sample) {
      count = 0;
      for (const IChannelObserver* obs : obs_list) {
        std::string value;
        bool valid = obs->GetEngValue(sample, value);
        if (count > 0) {
          file_ << delimiter_;
        }
        if (valid) {
          file_ << CsvText(value);
        }
        ++count;
      }
      file_ << std::endl;
    }
    file_.close();
    std::filesystem::rename(temp_filename_, dest_filename_);
  } catch (const std::exception& err) {
    MDF_ERROR() << "File save error. Error: " << err.what();
  }

  try {
    if (file_.is_open()) {
      file_.close();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "File close error. Error: " << err.what();
  }
}

std::string CsvWriter::CsvText(std::string& csv_text) const {
  for (char input : csv_text) {
    if (input == '\"' || input == delimiter_ || isspace(input)) {
      std::ostringstream temp;
      temp << "\"" << csv_text << "\"";
      return temp.str();
    }
  }

  return csv_text;
}

} // namespace mdf