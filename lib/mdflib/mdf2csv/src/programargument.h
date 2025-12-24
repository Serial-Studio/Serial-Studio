/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once
#include <string>
#include <vector>
#include <boost/program_options.hpp>

namespace mdf {

class ProgramArgument final {
 public:
  static ProgramArgument& Instance();
  void Parse(int argc, char* argv[]);
  [[nodiscard]] bool Help() const { return show_help_;}
  [[nodiscard]] bool Version() const { return show_version_;}

  void NonInteractive(bool non_active) {non_interactive_ = non_active;}
  [[nodiscard]] bool NonInteractive() const { return non_interactive_;}

  [[nodiscard]] bool DeleteConverted() const { return delete_converted_;}
  [[nodiscard]] bool NoAppendRoot() const { return no_append_root_;}

  void ForceOverwrite(bool force) { force_overwrite_ = force; }
  [[nodiscard]] bool ForceOverwrite() const { return force_overwrite_;}

  [[nodiscard]] int LogLevel() const { return log_level_;}
  [[nodiscard]] int BufferSize() const { return buffer_size_;}
  [[nodiscard]] const std::string& PasswordFile() const {
    return password_file_;
  }
  [[nodiscard]] const std::string& OutputDirectory() const {
    return output_directory_;
  }
  [[nodiscard]] const std::vector<std::string>& InputFiles() const {
    return input_list_;
  }
  void ShowHelp();
  void ShowVersion();
  void Clear();
 private:
  ProgramArgument();
  int log_level_ = -1;
  int buffer_size_ = -1;
  std::string password_file_;
  bool show_help_ = false;
  bool show_version_ = false;
  bool non_interactive_ = false;
  bool delete_converted_ = false;
  bool no_append_root_ = false;
  bool force_overwrite_ = false;
  std::vector<std::string> input_list_;
  std::string output_directory_;
  boost::program_options::options_description desc_;
  boost::program_options::positional_options_description pos_;
};

}  // namespace mdf

