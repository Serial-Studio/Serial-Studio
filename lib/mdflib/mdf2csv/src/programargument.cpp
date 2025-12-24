/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "programargument.h"
#include <iostream>
#include <string_view>
#include <boost/program_options/variables_map.hpp>

using namespace boost::program_options;

namespace {
constexpr std::string_view kToolAndVersionText =
    "MDF to CSV file converter. Version 1.0";
constexpr std::string_view kCopyrightText =
    "Copyright 2024 Ingemar Hedvall";
constexpr std::string_view kLicenseText =
    "SPDX-License-Identifier: MIT";

void strip_path(std::string& file_path) {
  while (!file_path.empty() && file_path[0] == '"') {
    file_path = file_path.substr(1);
    file_path.pop_back();
  }
}
}

namespace mdf {

ProgramArgument::ProgramArgument() :
desc_("Available Options") {
  desc_.add_options()
  ("help,h", "Display all available command line options and then exit.")
  ("version,V", "Displays the tool version and then exit")
  ("verbosity,v", value(&log_level_), "Sets the log level.")
  ("non-interactive","Does not print any progress output.")
  ("quiet,q","Does not print any progress output.")
  ("silent","Does not print any progress output.")
  ("force","Overwrite existing CSV files.")
  ("delete-converted,d", "Delete original files after they are converted")
  ("no-append-root,X", "Do not append \"_out\" when converting folders.")
  ("buffer,b", value(&buffer_size_), "Buffer size in bytes. 0 disables and -1 takes entire file.")
  ("password-file,p", value(&password_file_), "Path to password file.")
  ("output-directory,O",value(&output_directory_), "Output directory path.")
  ("input-files,i",value(&input_list_)->multitoken(), "List of input file(s)/folder(s).");

  pos_.add("input-files",-1);
}

ProgramArgument& ProgramArgument::Instance() {
  static ProgramArgument instance;
  return instance;
}

void ProgramArgument::Parse(int argc, char *argv[]) {
  Clear();
  // Parse the command line
  try {
    command_line_parser parser(argc, argv);
    parser.options(desc_);
    parser.positional(pos_);
    const auto opt = parser.run();
    variables_map vm;
    store(opt, vm);
    notify(vm);
    if (vm.empty()) {
      show_help_ = true;
      return;
    }
    show_help_ = vm.contains("help");
    show_version_ = vm.contains("version");
    if (vm.contains("non-interactive")) {
        non_interactive_ = true;
    }
    if (vm.contains("quiet")) {
      non_interactive_ = true;
    }
    if (vm.contains("silent")) {
      non_interactive_ = true;
    }
    delete_converted_ = vm.contains("delete-converted");
    no_append_root_ = vm.contains("no-append-root");
    force_overwrite_ = vm.contains("force");

    strip_path(password_file_);
    strip_path(output_directory_);
    for (auto& input : input_list_) {
      strip_path(input);
    }
  } catch (const std::exception& err) {
    std::cerr << "Invalid input argument. Error: " << err.what();
    show_help_ = true;
  }
}

void ProgramArgument::ShowHelp() {
  std::cout << std::endl;
  std::cout << kToolAndVersionText << std::endl;
  std::cout << kCopyrightText << std::endl;
  std::cout << kLicenseText << std::endl;
  std::cout << std::endl;
  std::cout << desc_ << std::endl;
}

void ProgramArgument::ShowVersion() {
  std::cout << std::endl;
  std::cout << kToolAndVersionText << std::endl;
  std::cout << kCopyrightText << std::endl;
  std::cout << kLicenseText << std::endl;
  std::cout << std::endl;
}

void ProgramArgument::Clear() {
  log_level_ = -1;
  buffer_size_ = -1;
  password_file_.clear();
  show_help_ = false;
  show_version_ = false;
  non_interactive_ = false;
  delete_converted_ = false;
  no_append_root_ = false;
  force_overwrite_ = false;
  input_list_.clear();
  output_directory_.clear();
}

}  // namespace mdf