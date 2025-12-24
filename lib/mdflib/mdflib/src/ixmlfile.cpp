/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ixmlfile.h"

#include <fstream>

#include "expatxml.h"
#include "mdf/mdflogstream.h"
#include "writexml.h"
#include "xmlnode.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace mdf {

std::string IXmlFile::FileNameWithoutPath() const {
  try {
    auto filename = fs::u8path(filename_).stem().u8string();
    return std::string(filename.begin(), filename.end());
  } catch (const std::exception &error) {
    MDF_ERROR() << "Invalid path. File: " << filename_
                << ", Error: " << error.what();
  }
  return filename_;
}

bool IXmlFile::ParseFile() { return false; }

bool IXmlFile::ParseString(const std::string &input) { return false; }

void IXmlFile::Reset() {
  root_node_.reset();
  version_ = "1.0";
  encoding_ = "UTF-8";
  standalone_ = true;
}

void IXmlFile::GetChildList(IXmlNode::ChildList &child_list) const {
  if (root_node_) {
    root_node_->GetChildList(child_list);
  }
}

const IXmlNode *IXmlFile::GetNode(const std::string &tag) const {
  return root_node_ ? root_node_->GetNode(tag) : nullptr;
}

IXmlNode &IXmlFile::RootName(const std::string &name) {
  if (!root_node_) {
    root_node_ = CreateNode(name);
  } else {
    root_node_->TagName(name);
  }
  return *root_node_;
}

std::unique_ptr<IXmlNode> IXmlFile::CreateNode(const std::string &name) {
  return std::make_unique<XmlNode>(name);
}

bool IXmlFile::WriteFile() {
  if (filename_.empty()) {
    MDF_ERROR() << "No file name defined";
    return false;
  }
  try {
    std::ofstream file(fs::u8path(filename_),
                       std::ofstream::out | std::ofstream::trunc);
    if (!file.is_open()) {
      MDF_ERROR() << "Couldn't open file for writing. File: " << filename_;
      return false;
    }

    // Start with byte order characters if the encoding is UTF-8
    if (Platform::stricmp(encoding_.c_str(), "UTF-8") == 0) {
      file << "\xEF\xBB\xBF";
    }
    WriteRoot(file, false);

    file.close();

  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to write the file. Error: " << err.what()
                << ", File: " << FileName();
    return false;
  }
  return true;
}

std::string IXmlFile::WriteString(bool skip_header) {
  std::ostringstream temp;
  WriteRoot(temp, skip_header);
  return temp.str();
}

void IXmlFile::WriteRoot(std::ostream &dest, bool skip_header) {
  if (!skip_header) {
    dest << "<?xml version='" << Version() << "' encoding='" << Encoding()
         << "'";
    if (Standalone()) {
      dest << " standalone='yes'";
    }
    dest << "?>" << std::endl;

    if (!style_sheet_.empty()) {
      dest << "<?xml-stylesheet type='text/xsl' href='" << StyleSheet() << "'?>"
           << std::endl;
    }
  }
  if (root_node_) {
    root_node_->Write(dest, 0);
  }
}

void IXmlFile::FileName(const std::string &filename) { filename_ = filename; }

std::unique_ptr<IXmlFile> CreateXmlFile(const std::string &type) {
  if (Platform::stricmp("FileWriter", type.c_str()) == 0) {
    return std::make_unique<WriteXml>();
  }
  return std::make_unique<ExpatXml>();
}

}  // namespace mdf
