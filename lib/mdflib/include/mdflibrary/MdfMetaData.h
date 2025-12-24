/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>

#include "MdfETag.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfMetaData {
 private:
  mdf::IMetaData* metaData;

 public:
  MdfMetaData(mdf::IMetaData* metaData) : metaData(metaData) {
    if (metaData == nullptr) throw std::runtime_error("MdfMetaData Init failed");
  }
  MdfMetaData(const mdf::IMetaData* metaData)
      : MdfMetaData(const_cast<mdf::IMetaData*>(metaData)) {}
  ~MdfMetaData() { metaData = nullptr; }
  std::string GetPropertyAsString(const char* index) {
    std::string str(MdfMetaDataGetPropertyAsString(metaData, index, nullptr) + 1, '\0');
    str.resize(MdfMetaDataGetPropertyAsString(metaData, index, str.data()));
    return str;
  }
  void SetPropertyAsString(const char* index, const char* prop) {
    MdfMetaDataSetPropertyAsString(metaData, index, prop);
  }
  double GetPropertyAsFloat(const char* index) {
    return MdfMetaDataGetPropertyAsFloat(metaData, index);
  }
  void SetPropertyAsFloat(const char* index, double prop) {
    MdfMetaDataSetPropertyAsFloat(metaData, index, prop);
  }
  std::vector<MdfETag> GetProperties() const {
    size_t count = MdfMetaDataGetProperties(metaData, nullptr);
    if (count <= 0) return {};
    auto pTags = new mdf::ETag*[count];
    MdfMetaDataGetProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(pTags[i]);
    delete[] pTags;
    return tags;
  }
  std::vector<MdfETag> GetCommonProperties() const {
    size_t count = MdfMetaDataGetCommonProperties(metaData, nullptr);
    if (count <= 0) return {};
    auto pTags = new mdf::ETag*[count];
    MdfMetaDataGetCommonProperties(metaData, pTags);
    std::vector<MdfETag> tags;
    for (size_t i = 0; i < count; i++) tags.push_back(pTags[i]);
    delete[] pTags;
    return tags;
  }
  void SetCommonProperties(const std::vector<MdfETag> pProperty, size_t count) {
    auto pTags = new const mdf::ETag*[count];
    for (size_t i = 0; i < count; i++) pTags[i] = pProperty[i].GetETag();
    MdfMetaDataSetCommonProperties(metaData, pTags, count);
    delete[] pTags;
  }
  std::string GetXmlSnippet() const {
    std::string str(MdfMetaDataGetXmlSnippet(metaData, nullptr) + 1, '\0');
    str.resize(MdfMetaDataGetXmlSnippet(metaData, str.data()));
    return str;
  }
  void SetXmlSnippet(const char* xml) {
    MdfMetaDataSetXmlSnippet(metaData, xml);
  }
  void AddCommonProperty(const MdfETag &tag) {
    MdfMetaDataAddCommonProperty(metaData, tag.GetETag());
  }
};
}  // namespace MdfLibrary