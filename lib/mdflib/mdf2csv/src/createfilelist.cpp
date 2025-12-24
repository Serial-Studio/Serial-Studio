/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include "createfilelist.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <cctype>
#include <array>
#include <mdf/mdfreader.h>
#include <mdf/mdflogstream.h>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include "csvwriter.h"
#include "programargument.h"


using namespace std::filesystem;

namespace mdf {
void CreateFileList::MakeFileList(const std::vector<std::string>& input_files) {
  // Parse out the directories first
  try {
    files_ok_ = true;
    for (const auto& input : input_files) {
      path input_path(input);
      if (!exists(input_path)) {
        MDF_ERROR() << "The input path/file doesn't exist. File: " << input;
        files_ok_ = false;
      } else if (is_directory(input_path)) {
        directories_.emplace_back(input_path.string());
      } else if (is_regular_file(input_path)) {
        const auto file = input_path.string();
        const bool mdf = IsMdfFile(file);
        if (!mdf) {
          MDF_ERROR() << "The file isn't an MDF file. File: " << file;
          files_ok_ = false;
          continue;
        }

        files_.emplace_back(file);
      }
    }

    for (const auto& dir : directories_) {
      path input_dir(dir);
      for (const directory_entry& dir_entry :
           recursive_directory_iterator(input_dir)) {
        if (!dir_entry.is_regular_file()) {
          continue;
        }
        const auto file = dir_entry.path().string();
        const bool mdf = IsMdfFile(file);
        if (!mdf) {
          // This is not considered an error as above.
          continue;
        }

        files_.emplace_back(file);
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "File/Directory syntax error. Error: " << err.what();
    files_ok_ = false;
  }
}

void CreateFileList::ConvertMdfFile(const std::string& file) {
  const auto& arguments = ProgramArgument::Instance();
  std::string filename;
  std::string stem;

  try {
    path fullname(file);
    filename = fullname.filename().string();
    stem = fullname.stem().string();
    if (output_path_.empty()) {
      output_path_ = fullname.parent_path().string();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Invalid file path. Error :" << err.what();
    return;
  }

  MdfReader reader(file);
  if (!reader.IsOk()) {
    MDF_INFO() << filename << ". Skipping. Invalid MDF file.";
    return;
  }

  const bool read = reader.ReadEverythingButData();
  if (!read) {
    MDF_INFO() << filename << ". Skipping. Read failure.";
    return;
  }

  const IHeader* header = reader.GetHeader();
  if (header == nullptr) {
    MDF_INFO() << ". Skipping. No file header.";
    return;
  }
  const auto dg_list = header->DataGroups();
  if (dg_list.empty()) {
    // Nothing to convert
    MDF_INFO() << ". Skipping. No data groups.";
    return;
  }

  size_t measure_no = 0; // Used to set a unique CSV name.
  for (auto* dg_group : dg_list) {
    if (dg_group == nullptr) {
      continue;
    }
    ++measure_no;
    size_t group_no = 0; // Used if the CG doesn't have a name.
    std::vector<IChannelGroup*> cg_list;
    const auto temp_list = dg_group->ChannelGroups();
    for (auto* temp_group : temp_list) {
      if (temp_group == nullptr) {
        continue;
      }
      if ((temp_group->Flags() & CgFlag::VlsdChannel) != 0) {
        continue;
      }
      if (temp_group->NofSamples() == 0) {
        continue;
      }
      cg_list.emplace_back(temp_group);
    }

    for (auto* cg_group : cg_list) {
      ++group_no;

      // Create a CSV name
      std::ostringstream csv_name;
      csv_name << stem;
      if (dg_list.size() > 1) {
        csv_name << "_DG_" << measure_no;
      }
      const auto cg_name = cg_group->Name();
      if (cg_list.size() > 1 && cg_name.empty()) {
        csv_name << "_CG_" << group_no;
      } else if (cg_list.size() > 1) {
        csv_name << "_" << cg_name;
      }

      bool existing; // Check if CSV already exist
      std::string csv_file;
      try {
        path fullname(output_path_);
        fullname.append(StripName(csv_name.str()));
        fullname.concat(".csv");
        csv_file = fullname.string();
        existing = exists(csv_file);
      } catch (const std::exception& err) {
        MDF_ERROR() << "CSV path error. Error: " << err.what();
        return;
      }

      if (existing && !arguments.ForceOverwrite()) {
        MDF_INFO() << filename << ". Skipping. CSV file exist.";
        continue;
      }

      // Check if it is a bus_log_file
      const bool bus_log_file = (cg_group->Flags() & CgFlag::BusEvent) != 0;

      ChannelObserverList observer_list;
      CreateChannelObserverForChannelGroup(*dg_group,
                                           *cg_group, observer_list);
      if (bus_log_file) {
        constexpr std::array<std::string_view, 4> exclude_list = {
          "CAN_DataFrame","CAN_RemoteFrame", "CAN_ErrorFrame", "CAN_OverloadFrame"
        };

        for (auto itr = observer_list.begin();
             itr != observer_list.end();  /* No ++itr here */ ) {
          if (itr->get() == nullptr) {
            itr = observer_list.erase(itr);
            continue;
          }

          const auto& channel = itr->get()->Channel();
          const std::string channel_name = channel.Name();
          const bool exclude = std::any_of(exclude_list.cbegin(),
                                           exclude_list.cend(),
                                           [&] (const std::string_view& name) ->bool {
                                             return channel_name == name;
                                           });
          if (exclude) {
            itr = observer_list.erase(itr);
          } else {
            ++itr;
          }
        }
      }

      if (observer_list.empty()) {
        continue;
      }

      const bool data = reader.ReadData(*dg_group);
      if (!data) {
        MDF_INFO() << " .Skipping. Read failure";
        continue;
      }

      MDF_INFO() << filename << " -> " << csv_name.str() << ".csv";
      CsvWriter writer(csv_file);
      if (!writer.IsOk()) {
        continue;
      }
      writer.Convert(observer_list);
    }
  }

}

std::string CreateFileList::StripName(const std::string& text) {
  std::ostringstream output;
  for (char input : text ) {
    switch (input) {
      case '_':
      case '-':
        output << input;
        break;

      default:
        if (!std::isspace(input) && !std::ispunct(input)) {
          output << input;
        }
        break;
    }
  }
  return output.str();
}

}  // namespace mdf