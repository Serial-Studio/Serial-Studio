/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file imetadata.h
 *
 * Support class for storing meta data within an MDF block. Many MDF4 blocks
 * store its properties in an embedded XML file or more correctly an XML
 * snippets which just includes the root tag and its content.
 */
#pragma once
#include <string>
#include <vector>

#include "mdf/etag.h"
#include "mdf/iblock.h"

namespace mdf {

/** \class IMetaData imetadata.h "mdf/imetadata.h"
 * \brief Interface against an meta data block (MD) in a MDF4 file.
 *
 * Interface class against an meta data block in a MDF4 file. In reality is the
 * block just an interface against an XML snippets
 */
class IMetaData : public IBlock {
 public:
  /** \brief Initiate the MD block.
   *
   * Initiate the MD block with a root name. Note that this function
   * should not be called by the end-user as it automatically created by the
   * owner block.
   * @param root_name Root tag name of the XML snippet.
   */
  void InitMd(const std::string& root_name);

  /** \brief Sets a string property in the block.
   *
   * @param tag Tag name.
   * @param value Tag value.
   */
  void StringProperty(const std::string& tag, const std::string& value);

  /** \brief Returns a specific tag value.
   *
   * @param tag Tag name.
   * @return  Tag value.
   */
  [[nodiscard]] std::string StringProperty(const std::string& tag) const;

  /** \brief Sets a float property in the block.
   *
   * @param tag Tag name.
   * @param value Tag value.
   */
  void FloatProperty(const std::string& tag, double value);

  /** \brief Returns a specific tag value.
   *
   * @param tag Tag name.
   * @return  Tag value.
   */
  [[nodiscard]] double FloatProperty(const std::string& tag) const;

  /** \brief Sets a common property.
   *
   * @param e_tag Property to set.
   */
  void CommonProperty(const ETag& e_tag);

  /** \brief Return a common property
   *
   * @param name Property name.
   * @return Common property.
   */
  [[nodiscard]] ETag CommonProperty(const std::string& name) const;

  /** \brief Sets a number of common properties.
   *
   * @param tag_list List of common properties.
   */
  void CommonProperties(const std::vector<ETag>& tag_list);

  /** \brief Returns all common properties
   *
   * @return All common properties.
   */
  [[nodiscard]] std::vector<ETag> CommonProperties() const;
  
  /** \brief Returns all properties without children
   *
   * @return All properties.
   */
  [[nodiscard]] std::vector<ETag> Properties() const;
  
  /** \brief Stores the XML as a string..
   *
   * @param text The XML string.
   */
  virtual void XmlSnippet(const std::string& text) = 0;

  /** \brief Returns the XML string.
   *
   * @return The XML as a string.
   */
  [[nodiscard]] virtual const std::string& XmlSnippet() const = 0;
};

}  // namespace mdf