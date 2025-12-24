/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/mdffile.h"

#include <string>

#include "at4block.h"
#include "mdf/mdflogstream.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace mdf {

int MdfFile::MainVersion() const {
  const auto version = Version();
  int main_version = 0;
  try {
    const auto main = version.substr(0, 1);
    main_version = std::stoi(main);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failure to finding MDF main version. Error: " << err.what();
  }
  return main_version;
}

int MdfFile::MinorVersion() const {
  const auto version = Version();
  int minor_version = 0;

  try {
    const auto minor = version.substr(2);
    minor_version = std::stoi(minor);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failure to finding MDF minor version. Error: "
                << err.what();
  }
  return minor_version;
}

IAttachment* MdfFile::CreateAttachment() { return nullptr; }

void MdfFile::FileName(const std::string& filename) {
  filename_ = filename;
  if (name_.empty()) {
    try {
      auto name = fs::u8path(filename).stem().u8string();
      name_ = std::string(name.begin(), name.end());
    } catch (const std::exception& err) {
      MDF_ERROR() << "Invalid file name detected. Error: " << err.what()
                  << ", File: " << filename;
    }
  }
}
bool MdfFile::IsFinalizedDone() const { return false; }

}  // namespace mdf
