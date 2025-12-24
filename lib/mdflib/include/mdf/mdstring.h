/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>

#include "mdf/mdstandardattribute.h"

namespace mdf {

class MdString;
class IXmlNode;

using MdStringList = std::vector<MdString>;

class MdString : public MdStandardAttribute {
  public:

   MdString() = default;
   explicit MdString(const char* text, uint64_t history_index = 0,
                     std::string language = {});

   explicit MdString(const std::string_view& text, uint64_t history_index = 0,
                     std::string language = {});

   explicit MdString(std::string text, uint64_t history_index = 0,
                     std::string language = {});

   operator const std::string&() const; // NOLINT(*-explicit-constructor)

   [[nodiscard]] bool operator < (const MdString& text) const;

   [[nodiscard]] bool operator == (const MdString& text) const;

   [[nodiscard]] bool operator == (const std::string& text) const;

   [[nodiscard]] bool operator == (const std::string_view& text) const;

   [[nodiscard]] bool IsActive() const override;

   void Text(std::string text);
   [[nodiscard]] const std::string& Text() const;

   void Offset(uint64_t offset);
   [[nodiscard]] uint64_t Offset() const;

   virtual void ToXml(IXmlNode& root_node, const std::string& tag_name) const;
   void FromXml(const IXmlNode& node) override;
  private:
   std::string text_;
   std::optional<uint64_t> offset_;

};

}  // namespace mdf

