/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include <string>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "programargument.h"
#include "createfilelist.h"
#include "logtoconsole.h"

using namespace mdf;


int main(int argc, char* argv[]) {
  ProgramArgument& arguments = ProgramArgument::Instance();

  arguments.Parse(argc,argv);

  if (arguments.Help()) {
    arguments.ShowHelp();
    return EXIT_SUCCESS;
  }

  if (arguments.Version()) {
    arguments.ShowVersion();
    return EXIT_SUCCESS;
  }

  LogToConsole logger;

  const auto& input_files = arguments.InputFiles();
  const auto& output_dir = arguments.OutputDirectory();

  CreateFileList creator;
  creator.OutputPath(output_dir);
  creator.MakeFileList(input_files);

  if (creator.Files().empty()) {
    MDF_ERROR() << "There is no MDF files to convert.";
    return EXIT_FAILURE;
  }

  for (const auto& file : creator.Files()) {
    creator.ConvertMdfFile(file);
  }

  return EXIT_SUCCESS;
}