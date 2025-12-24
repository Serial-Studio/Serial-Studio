/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "expatxml.h"

#include <array>

#include "mdf/mdflogstream.h"
#include "platform.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace {
void StartElementHandler(void *userData, const XML_Char *name,
                         const XML_Char **attributes) {
  auto parser = static_cast<mdf::ExpatXml *>(userData);
  if (parser != nullptr) {
    parser->StartElement(name, attributes);
  }
}

void EndElementHandler(void *userData, const XML_Char *name) {
  auto parser = static_cast<mdf::ExpatXml *>(userData);
  if (parser != nullptr) {
    parser->EndElement(name);
  }
}

void CharacterDataHandler(void *userData, const XML_Char *s, int len) {
  auto parser = static_cast<mdf::ExpatXml *>(userData);
  if (parser != nullptr) {
    parser->CharacterData(s, len);
  }
}

void XmlDeclHandler(void *userData, const XML_Char *version,
                    const XML_Char *encoding, int standalone) {
  auto parser = static_cast<mdf::ExpatXml *>(userData);
  if (parser != nullptr) {
    parser->XmlDecl(version, encoding, standalone);
  }
}

class ExpatParser final {
 public:
  explicit ExpatParser(mdf::ExpatXml *dest)
      : parser_(XML_ParserCreate("UTF-8")) {
    XML_SetUserData(parser_, dest);
    XML_SetElementHandler(parser_, StartElementHandler, EndElementHandler);
    XML_SetCharacterDataHandler(parser_, CharacterDataHandler);
    XML_SetXmlDeclHandler(parser_, XmlDeclHandler);
  }
  ~ExpatParser() { XML_ParserFree(parser_); }

  ExpatParser() = delete;

  enum XML_Status Parse(const char *buffer, size_t len, bool isFinal) const {
    return XML_Parse(parser_, buffer, static_cast<int>(len), isFinal);
  }
  XML_Parser parser_ = nullptr;
};

}  // end namespace

namespace mdf {

void ExpatXml::StartElement(const XML_Char *fullname,
                            const XML_Char **attributes) {
  std::unique_ptr<XmlNode> p = std::make_unique<XmlNode>(fullname);
  p->SetAttribute(attributes);
  auto *current = node_stack_.empty() ? nullptr : node_stack_.top();
  node_stack_.push(p.get());
  if (!root_node_) {
    root_node_ = std::move(p);
  } else if (current != nullptr) {
    current->AddNode(std::move(p));
  }
}

void ExpatXml::EndElement(const XML_Char *) {
  if (!node_stack_.empty()) {
    node_stack_.pop();
  }
}

void ExpatXml::CharacterData(const char *buffer, int len) {
  auto *current = node_stack_.empty() ? nullptr : node_stack_.top();
  if (current != nullptr) {
    current->AppendData(buffer, len);
  }
}

void ExpatXml::XmlDecl(const XML_Char *version, const XML_Char *encoding,
                       int standalone) {
  if (version != nullptr) {
    version_ = version;
  }
  if (encoding != nullptr) {
    encoding_ = encoding;
  }
  standalone_ =
      standalone != 0;  // Note that not included is -1, 0 == false, 1 = true
}

bool ExpatXml::ParseFile() {
  Reset();

  std::FILE *file = nullptr;
  Platform::fileopen(&file, filename_.c_str(), "r");
  if (file == nullptr) {
    return false;
  }

  ExpatParser p(this);
  std::array<char, 1024> buffer{};
  bool ok = true;
  for (size_t reads = fread(buffer.data(), 1, buffer.size(), file); reads > 0;
       reads = fread(buffer.data(), 1, buffer.size(), file)) {
    const auto ret = p.Parse(buffer.data(), reads, reads == 0);
    if (ret != XML_STATUS_OK) {
      const auto line = XML_GetCurrentLineNumber(p.parser_);
      const auto column = XML_GetCurrentColumnNumber(p.parser_);
      const auto error = XML_GetErrorCode(p.parser_);
      const auto *error_text = XML_ErrorString(error);
      MDF_ERROR() << "XML parser error.  Line: " << line
                  << ", Column: " << column << ", Error: " << error_text
                  << ", File: " << filename_;
      ok = false;
      break;
    }
  }
  if (ok && !std::feof(file)) {
    MDF_ERROR() << "XML file error.  Error: Not entire file was read. File: "
                << filename_;
    ok = false;
  }

  std::fclose(file);
  return ok;
}

bool ExpatXml::ParseString(const std::string &input) {
  Reset();
  ExpatParser p(this);
  bool ok = true;
  auto ret = p.Parse(input.data(), input.size(), true);
  if (ret != XML_STATUS_OK) {
    const auto line = XML_GetCurrentLineNumber(p.parser_);
    const auto column = XML_GetCurrentColumnNumber(p.parser_);
    const auto error = XML_GetErrorCode(p.parser_);
    const auto *error_text = XML_ErrorString(error);
    MDF_ERROR() << "XML parser error.  Line: " << line << ", Column: " << column
                << ", Error: " << error_text << ", String: " << input;
    ok = false;
  }
  return ok;
}

void ExpatXml::Reset() {
  IXmlFile::Reset();
  node_stack_ = {};
}

}  // namespace mdf
