/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>
#include <string_view>

#include "mdf/hdcomment.h"
#include "mdf/mdflogstream.h"
#include "mdf/mdstring.h"

#include "writexml.h"

namespace {
  constexpr std::string_view kAuthor = "author";
  constexpr std::string_view kDepartment = "department";
  constexpr std::string_view kProject = "project";
  constexpr std::string_view kSubject = "subject";
  constexpr std::string_view kMeasurementUuid = "Measurement.UUID";
  constexpr std::string_view kRecorderUuid = "Recorder.UUID";
  constexpr std::string_view kRecorderFileIndex = "Recorder.FileIndex";
}
namespace mdf {

HdComment::HdComment()
: MdComment("HD") {
}

HdComment::HdComment(std::string comment)
    : HdComment() {
  Comment(std::move(comment));
}

void HdComment::Author(std::string author) {
  AddProperty({std::string(kAuthor), std::move(author)});
}

std::string HdComment::Author() const {
  return GetPropertyValue<std::string>(std::string(kAuthor));
}

void HdComment::Department(std::string department) {
  AddProperty({std::string(kDepartment), std::move(department)});
}

std::string HdComment::Department() const {
  return GetPropertyValue<std::string>(std::string(kDepartment));
}

void HdComment::Project(std::string project) {
  AddProperty({std::string(kProject), std::move(project)});
}

std::string HdComment::Project() const {
  return GetPropertyValue<std::string>(std::string(kProject));
}

void HdComment::Subject(std::string subject) {
  AddProperty({std::string(kSubject), std::move(subject)});
}

std::string HdComment::Subject() const {
  return GetPropertyValue<std::string>(std::string(kSubject));
}

void HdComment::MeasurementUuid(std::string uuid) {
  AddProperty({std::string(kMeasurementUuid), std::move(uuid)});
}

std::string HdComment::MeasurementUuid() const {
  return GetPropertyValue<std::string>(std::string(kMeasurementUuid));
}

void HdComment::RecorderUuid(std::string uuid) {
  AddProperty({std::string(kRecorderUuid), std::move(uuid)});
}

std::string HdComment::RecorderUuid() const {
  return GetPropertyValue<std::string>(std::string(kRecorderUuid));
}

void HdComment::RecorderFileIndex(int64_t index) {
  MdProperty file_index(std::string(kRecorderFileIndex), "");
  file_index.Value(index);
  file_index.DataType(MdDataType::MdInteger);

  AddProperty(file_index);
}

int64_t HdComment::RecorderFileIndex() const {
  auto file_index = GetPropertyValue<std::string>(std::string(kRecorderFileIndex));
  if (file_index.empty()) {
    return -1;
  }
  int64_t index;
  try {
    index = std::stoll(file_index);
  } catch (const std::exception& ) {
    index = -1;
  }
  return index;
}

void HdComment::TimeSource(MdString time_source) {
  time_source_ = std::move(time_source);
}

const MdString& HdComment::TimeSource() const {
  return time_source_;
}

void HdComment::AddConstant(MdString constant, std::string expression) {

  if (constant.Text().empty()) {
    MDF_ERROR() << "An empty constant are not allowed";
    return;
  }
  if (const auto itr = constant_list_.find(constant);
      itr != constant_list_.cend() ) {
    MDF_ERROR()
        << "A duplicate constant have been found. Constant/Expression: "
        << constant.Text() << "/" << expression;
    return;
  }
  constant_list_.emplace(std::move(constant), std::move(expression));
}

const MdConstantList& HdComment::Constants() const {
  return constant_list_;
}

MdConstantList& HdComment::Constants() {
  return constant_list_;
}

const HoUnitSpecification& HdComment::UnitSpecification() const {
  return unit_spec_;
}

HoUnitSpecification& HdComment::UnitSpecification() {
  return unit_spec_;
}

std::string HdComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    const bool ho_active = unit_spec_.IsActive();
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);

    if (time_source_.IsActive()) {
      time_source_.ToXml(root_node, "time_source");
    }
    if (!constant_list_.empty()) {
      auto& constants_node = root_node.AddNode("constants");
      for (const auto& [constant, expression] : constant_list_) {
        IXmlNode& const_node = constants_node.AddNode("const");
        const_node.SetAttribute("name", constant.Text());
        // Add attributes to the node
        if (constant.HistoryIndex() > 0) {
          const_node.SetAttribute("ci", constant.HistoryIndex());
        }
        if (!constant.Language().empty()) {
          const_node.SetAttribute("xml:lang", constant.Language());
        }
        for (const auto& [key,value] : constant.CustomAttributes()) {
          const_node.SetAttribute(key, value);
        }
        const_node.Value(expression);
      }
    }
    if (ho_active) {
      unit_spec_.ToXml(root_node);
    }
    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the HD comment. Error: " << err.what();
  }
  return {};
}

void HdComment::FromXml(const std::string& xml_snippet) {
  if (xml_snippet.empty()) {
    return;
  }
  try {
    auto xml_file = CreateXmlFile("Expat");
    if (!xml_file) {
      throw std::runtime_error("Failed to create EXPAT parser object");
    }
    const bool parse = xml_file->ParseString(xml_snippet);
    if (!parse) {
      throw std::runtime_error("Failed to parse the XML string.");
    }
    const auto* root_node = xml_file->RootNode();
    if (root_node == nullptr) {
      throw std::runtime_error("There is no root node in the XML string");
    }
    MdComment::FromXml(*root_node);

    IXmlNode::ChildList node_list;
    xml_file->GetChildList(node_list);
    for (const IXmlNode* node : node_list) {
      if (node == nullptr) {
        continue;
      }
      if (node->IsTagName("time_source") ) {
        time_source_.FromXml(*node);
      } else if (node->IsTagName("constants")) {
        IXmlNode::ChildList const_list;
        node->GetChildList(const_list);
        for (const auto* const_node : const_list) {
          if (const_node == nullptr || !const_node->IsTagName("const")) {
            continue;
          }
          std::string expression = const_node->Value<std::string>();
          std::string name = const_node->Attribute<std::string>("name", "");
          MdString temp;
          temp.FromXml(*const_node);
          temp.Text(name);
          AddConstant(temp, expression);
        }
      } else if (node->IsTagName("UNIT-SPEC")) {
        unit_spec_.FromXml(*node);
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the HD comment. Error: " << err.what();
  }
}


}  // namespace mdf