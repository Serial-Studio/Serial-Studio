/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "csvwriter.h"
#include "logstream.h"

#include <algorithm>
#include <filesystem>

#include <boost/lexical_cast.hpp>


using namespace util::log;

namespace {
std::string kEmptyString;

/**
 * Scan through the text and checks if it include CRLF or any comma (','). If so
 * the text needs to be inside ditto marks ("text").
 * @param text
 */
void CheckIfDittoMarkNeeded(std::string &text) {
  auto need_ditto_mark = std::any_of(text.cbegin(), text.cend(), [](const char in) {
    const auto byte = static_cast<unsigned char>(in);
    if (std::isspace(byte) || !std::isprint(byte) || in == ',') {
      return true;
    }
    return false;
  });

  if (need_ditto_mark) {
    std::ostringstream s;
    s << "\"" << text << "\"";
    text = s.str();
  }
}

}  // namespace

namespace util::plot {

CsvWriter::CsvWriter(const std::string &filename) : filename_(filename) {
  output_ = &text_;
  try {
    file_.open(filename_.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (file_.is_open()) {
      output_ = &file_;
    } else {
      LOG_ERROR() << "Failed to open the CSV file. File: " << filename_;
    }
  } catch (const std::exception &) {
    LOG_ERROR() << "Failed to open the CSV file. File: " << filename_;
  }
}
CsvWriter::CsvWriter() { output_ = &text_; }

CsvWriter::~CsvWriter() {
  if (file_.is_open()) {
    file_.close();
  }
}

std::string CsvWriter::FileName() const {
  std::string name;
  try {
    std::filesystem::path temp(filename_);
    name = temp.filename().string();
  } catch (const std::exception &err) {
    LOG_ERROR()
        << "Failed to retrieve the filename out of the full path. Error: "
        << err.what() << ", File Name: " << filename_;
  }
  return name;
}

bool CsvWriter::IsOk() const { return file_.is_open(); }

void CsvWriter::AddColumnHeader(const std::string &name,
                                const std::string &unit, bool valid) {
  if (row_count_ != 0) {
    LOG_ERROR() << "Column headers shall be added to first row. File: "
                << filename_;
    return;
  }
  header_list_.emplace_back(Header{name, unit, valid});
  std::ostringstream header;
  header << name;
  if (!unit.empty()) {
    header << " [" << unit << "]";
  }
  SaveText(header.str());
  max_columns_ = std::max(column_count_, max_columns_);
}

void CsvWriter::SaveText(const std::string &text) {
  if (output_ == nullptr) {
    LOG_ERROR() << "File is not open. File: " << filename_;
    return;
  }

  // If temp includes a '"' it needs to be replaced by a '""'=
  std::string temp = [&text] {
    std::ostringstream temp;
    for (auto i : text) {
      if (i == '"') {
        temp << "\"\"";
      } else {
        temp << i;
      }
    };
    return temp.str();
  }();

  CheckIfDittoMarkNeeded(
      temp);  // Check if the text needs '"' at start and end.
  if (column_count_ > 0) {
    *output_ << ",";
  }
  *output_ << temp;
  ++column_count_;
}

template <>
void CsvWriter::AddColumnValue(const std::string &value) {
  if (row_count_ > 0 && column_count_ >= max_columns_) {
    AddRow();
  }
  SaveText(value);
  if (row_count_ == 0) {
    max_columns_ = std::max(column_count_, max_columns_);
  }
}

template <>
void CsvWriter::AddColumnValue(const float &value) {
  const auto temp = boost::lexical_cast<std::string>(value);
  AddColumnValue(temp);
}

template <>
void CsvWriter::AddColumnValue(const double &value) {
  const auto temp = boost::lexical_cast<std::string>(value);
  AddColumnValue(temp);
}

template <>
void CsvWriter::AddColumnValue(const bool &value) {
  const std::string temp = value ? "1" : "0";
  AddColumnValue(temp);
}

void CsvWriter::AddRow() {
  if (column_count_ == 0 && row_count_ == 0) {
    return;  // No empty lines allowed
  }
  if (output_ != nullptr) {
    if (column_count_ < max_columns_) {
      for (auto ii = column_count_; ii < max_columns_; ++ii) {
        if (ii > 0) {
          *output_ << ",";
        }
      }
    }
    *output_ << std::endl;
  }
  ++row_count_;
  column_count_ = 0;
}

void CsvWriter::CloseFile() {
  if (file_.is_open()) {
    file_.close();
  }
  output_ = &text_;
}

void CsvWriter::OpenFile(const std::string &filename) {
  filename_ = filename;
  output_ = &text_;
  try {
    file_.open(filename_.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (file_.is_open()) {
      output_ = &file_;
    } else {
      LOG_ERROR() << "Failed to open the CSV file. File: " << filename_;
    }
  } catch (const std::exception &) {
    LOG_ERROR() << "Failed to open the CSV file. File: " << filename_;
  }
}

const std::string CsvWriter::Label(size_t index) const {
  std::ostringstream temp;
  temp << Name(index);
  const auto unit = Unit(index);
  if (!unit.empty()) {
    temp << " [" << unit << "]";
  }
  return temp.str();
}

const std::string &CsvWriter::Name(size_t index) const {
  return index < header_list_.size() ? header_list_[index].name : kEmptyString;
}

const std::string &CsvWriter::Unit(size_t index) const {
  return index < header_list_.size() ? header_list_[index].unit : kEmptyString;
}

void CsvWriter::Reset() {
  column_count_ = 0;
  max_columns_ = 0;
  row_count_ = 0;
  header_list_.clear();
  if (file_.is_open()) {
    file_.close();
  }
  text_.str("");
  text_.clear();
  output_ = &text_;
}

void CsvWriter::SetColumnValid(size_t column, bool valid) {
  if (column < header_list_.size()) {
    header_list_[column].valid = valid;
  }
}

bool CsvWriter::IsColumnValid(size_t column) const {
  return column < header_list_.size() ? header_list_[column].valid : false;
}

}  // namespace util::plot
