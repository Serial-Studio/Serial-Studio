/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
#include <vector>
namespace mdf {

class CreateFileList final {
 public:

  void MakeFileList(const std::vector<std::string>& input_files);
  [[nodiscard]] bool FilesOk() const { return files_ok_; }
  [[nodiscard]] const std::vector<std::string>& Directories() const {
    return directories_;
  }
  [[nodiscard]] const std::vector<std::string>& Files() const {
    return files_;
  }

  void OutputPath(const std::string& output_path) {
    output_path_ = output_path;
  }
  [[nodiscard]] const std::string& OutputPath() const {
    return output_path_;
  }


  void ConvertMdfFile(const std::string& file);
  /** \brief Removes all characters that can cause problem when using
   * it in a filename.
   * @param text Input text
   * @return Stripped text safe to use in a file path.
   */
  static std::string StripName(const std::string& text);
 private:
  bool files_ok_ = false;
  std::vector<std::string> directories_;
  std::vector<std::string> files_;
  std::string output_path_;


};

}  // namespace mdf


