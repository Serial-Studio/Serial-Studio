/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <fstream>
#include <mdf/mdfreader.h>

namespace mdf {

class CsvWriter final {
 public:
  CsvWriter() = delete;
  explicit CsvWriter(const std::string& filename);
  ~CsvWriter();
  [[nodiscard]] bool IsOk() const { return is_ok_; }

  void Convert( mdf::ChannelObserverList& observer_list );

 protected:
  std::string dest_filename_;
  std::string temp_filename_;
  std::string parent_path_;
  std::string name_;         ///< File name without path
  bool is_ok_ = false;
  std::ofstream file_;
  char delimiter_ = ';';

  std::string CsvText(std::string& csv_text) const;
};

}  // namespace mdf


