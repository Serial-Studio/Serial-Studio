/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file ixmlnode.h
 * Interface class that defines a XML node (tag).
 */
#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

#include "mdf/mdfhelper.h"
#include "platform.h"

namespace mdf {

/** \class IXmlNode ixmlnode.h "xmlnode.h"
 * \brief Interface class against a XML tag (node) in a XML file.
 *
 * The class is the main interface when an XML file is more than one level deep.
 * Simple configuration files normally only have a root tag including one or
 * more value tags (properties).
 *
 * Each tag have a tag name and none or more attributes. If the tag name is
 * unique within the file, there is no need for attributes but if many tags with
 * the same name, it's recommended to set a unique attribute on each tag.
 *
 */
class IXmlNode {
 public:
  using KeyValueList = std::map<std::string, std::string>;

  explicit IXmlNode(const std::string &tag_name);  ///< Creates a XML tag.
  virtual ~IXmlNode() = default;                   ///< Destructor

  IXmlNode() = delete;

  /** \brief Returns the nodes tag name.
   *
   * Return the XML nodes tag name.
   * @return Teturns the tag name.
   */
  [[nodiscard]] std::string TagName() const { return tag_name_; }

  /** \brief Sets the tag name.
   *
   * Sets the node tag name.
   * @param name The new tag name.
   */
  void TagName(const std::string &name) { tag_name_ = name; }

  /** \brief Returns true if the tag name match.
   * Check if this is a specific tag name. The check ignore case and namespaces.
   *
   * Example: If Tag name is '<ns:Foo>', IsTagName("foo") -> true
   * @param [in] tag Tag name
   * @return True if the tag name match.
   */
  [[nodiscard]] bool IsTagName(const std::string &tag) const;

  /** \brief Checks if an attribute exist
   *
   * Tests if an attribute exist.
   * @param key Attribute name
   * @param value Attribute value
   * @return True if the attribute exist
   */
  [[nodiscard]] bool IsAttribute(const std::string &key,
                                 const std::string &value) const;

  /** \brief Returns an attribute.
   *
   * Returns an attribute
   * @tparam T Type of value. Can be string, numbers or boolean.
   * @param [in] key Attribute name.
   * @param [in] def Default value if attribute is missing.
   * @return Returns the attribute value or default value..
   */
  template <typename T>
  [[nodiscard]] T Attribute(const std::string &key, const T &def = {}) const;

  /** \brief Sets an attribute
   *
   * @tparam T Type of value
   * @param key Attribute name
   * @param value Attribute value
   */
  template <typename T>
  void SetAttribute(const std::string &key, const T &value);

  /** \brief Returns a value.
   *
   * Returns the tag value
   * @tparam T Type of value. Can be string, numbers or boolean.
   * @return Returns the tag value
   */
  template <typename T>
  [[nodiscard]] T Value() const; 

  /** \brief Sets a tag value
   *
   *  Sets a tag value.
   * @tparam T Type of value
   * @param value Value
   */
  template <typename T>
  void Value(const T &value);

  /** \brief Returns a specific tag value.
   *
   * Return a child tag value. Note that the Value() function returns this tags
   * value.
   * @tparam T Type of value. Can be string, numbers or boolean.
   * @param [in] key Child tag name.
   * @param [in] def Default value.
   * @return The child tags value.
   */
  template <typename T>
  [[nodiscard]] T Property(const std::string &key, const T &def = {}) const {
    const IXmlNode *node = GetNode(key);
    return node != nullptr ? node->Value<T>() : def;
  }

  /** \brief Sets a property
   *
   * Sets a property
   * @tparam T  Type of value
   * @param key Tag name
   * @param value Value.
   */
  template <typename T>
  void SetProperty(const std::string &key, const T &value) {
    auto &node = AddUniqueNode(key);
    node.Value(value);
  }

  /** \typedef ChildList
   * \brief List of pointer to child nodes.
   */
  using ChildList = std::vector<const IXmlNode *>;

  virtual void GetChildList(
      ChildList &child_list) const;  ///< Returns the child nodes.
  [[nodiscard]] virtual const IXmlNode *GetNode(
      const std::string &tag) const;  ///< Returns a node if it exist.
  [[nodiscard]] virtual const IXmlNode *GetNode(const std::string &tag,
                                                const std::string &key,
                                                const std::string &attr)
      const;  ///< Returns a node with a specific attribute.

  /** \brief Returns true if the named property exist.
   *
   * Returns true if the named property exists. This function is normally used
   * when deciding if an attribute or a property key/value should be used.
   * @param tag The name of the key (tag).
   * @return True if it exist
   */
  [[nodiscard]] bool ExistProperty(const std::string &tag) const {
    return GetNode(tag) != nullptr;
  }

  virtual IXmlNode &AddNode(const std::string &name);  ///< Adds a new tag
  virtual IXmlNode &AddUniqueNode(
      const std::string &name);  ///< Adds a unique tag
  virtual IXmlNode &AddUniqueNode(
      const std::string &name, const std::string &key,
      const std::string &attr);  ///< Adds a unique tag with an attribute

  virtual void AddNode(std::unique_ptr<IXmlNode> p);  ///< Adds a node

  void DeleteNode(const std::string& name); ///< Deletes all nodes with name
  void DeleteNode(const IXmlNode* node); ///< Deletes a specific node

  virtual void Write(std::ostream &dest,
                     size_t level);  ///< Write the node to the stream
  [[nodiscard]] bool HasChildren() const;

  [[nodiscard]] KeyValueList GetAttributeList() const;

 protected:
  std::string tag_name_;  ///< Name of this tag.
  std::string value_;     ///< String value of this tag.

  /** \typedef AttributeList
   * \brief Indexed list of to key, attribute value pair.
   */
  using AttributeList = std::unordered_map<std::string, std::string>;
  AttributeList attribute_list_;  ///< Indexed list of attribute

  /** \typedef NodeList
   * \brief  List of nodes
   */
  using NodeList = std::vector<std::unique_ptr<IXmlNode>>;
  NodeList node_list_;  ///< List of nodes

  [[nodiscard]] virtual std::unique_ptr<IXmlNode> CreateNode(
      const std::string &name) const;  ///< Create a node
};

template <typename T>
void IXmlNode::Value(const T &value) {
  //  value_ = std::to_string(value);
  std::ostringstream temp;
  temp << value;
  value_ = temp.str();
}

/** \brief Adds a boolean value
 *
 * @param value Boolean value
 */
template <>
void IXmlNode::Value(const bool &value);

/** \brief Adds a string value
 *
 * @param value String value
 */
template <>
void IXmlNode::Value(const std::string &value);

template <typename T>
T IXmlNode::Attribute(const std::string& key, const T& def) const {
  for (const auto &p : attribute_list_) {
    if (Platform::stricmp(p.first.c_str(), key.c_str()) == 0) {
      std::string v(p.second);
      mdf::MdfHelper::Trim(v);
      std::istringstream temp(v);
      T out = {};
      temp >> out;
      return out;  
    }
  }
  return def;
}

template <>
inline bool IXmlNode::Attribute(const std::string& key, const bool& def) const {
  for (const auto &p : attribute_list_) {
    if (Platform::stricmp(p.first.c_str(), key.c_str()) == 0) {
      std::string v(p.second);
      mdf::MdfHelper::Trim(v);

      return Platform::stricmp(v.c_str(), "1") == 0 ||
             Platform::stricmp(v.c_str(), "ON") == 0 ||
             Platform::strnicmp(v.c_str(), "T", 1) == 0      // True/False
             || Platform::strnicmp(v.c_str(), "Y", 1) == 0;  // Yes/No
    }
  }
  return def;
}

template <>
inline std::string IXmlNode::Attribute(const std::string& key,
  const std::string& def) const {
  for (const auto &p : attribute_list_) {
    if (Platform::stricmp(p.first.c_str(), key.c_str()) == 0) {
      std::string v(p.second);
      mdf::MdfHelper::Trim(v);
      return v;
    }
  }
  return def;
}
template <typename T>
void IXmlNode::SetAttribute(const std::string &key, const T &value) {
  std::ostringstream temp;
  temp << value;
  attribute_list_.insert({key, temp.str()});
}

template <typename T>
T IXmlNode::Value() const {
  std::string v(value_);
  mdf::MdfHelper::Trim(v);
  std::istringstream temp(v);
  T out = {};
  temp >> out;
  return out;  
}

template <>
inline bool IXmlNode::Value() const {
  std::string v(value_);
  mdf::MdfHelper::Trim(v);

  return Platform::stricmp(v.c_str(), "1") == 0 ||
         Platform::stricmp(v.c_str(), "ON") == 0 ||
         Platform::strnicmp(v.c_str(), "T", 1) == 0      // True/False
         || Platform::strnicmp(v.c_str(), "Y", 1) == 0;  // Yes/No 
}

template <>
inline std::string IXmlNode::Value() const {
  std::string v(value_);
  mdf::MdfHelper::Trim(v);
  return v;
}
/** \brief Sets a boolean attribute
 *
 * @param key Attribute name
 * @param value  Attribute boolean value
 */
template <>
void IXmlNode::SetAttribute(const std::string &key, const bool &value);

}  // namespace mdf
