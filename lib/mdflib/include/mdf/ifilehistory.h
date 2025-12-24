/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ifilehistory.h
 * Defines an interface against a FH block in an MDF4 file. MDF3 file doesn't
 * support file history.
 */
#pragma once
#include <cstdint>
#include <string>

#include "imdftimestamp.h"
#include "itimestamp.h"
#include "mdf/iblock.h"
#include "mdf/imetadata.h"
#include "mdf/fhcomment.h"

namespace mdf {

/** \class IFileHistory ifilehistory.h "mdf/ifilehistory.h"
 * \brief Interface class against an MDF FH block.
 *
 * Interface against an MDF4 file history block. An MDF4 file must have at least
 * one history block that defines it creation. Each time the file is modified,
 * there should be a new history block appended.
 */
class IFileHistory : public IBlock {
 public:
  /** \brief Sets the time when the history block was created.
   *
   * Sets the absolute start time for the measurement file. It is default
   * set by the application to now.
   * @param ns_since_1970 Nanoseconds since 1970.
   */
  virtual void Time(uint64_t ns_since_1970) = 0;

  /**
   * \brief Sets the time using an ITimestamp object.
   *
   * This function sets the time for the history block using an ITimestamp
   * object.
   * @param timestamp An ITimestamp object representing the time.
   */
  virtual void Time(ITimestamp& timestamp) = 0;

  /** \brief Returns the time for the history block.
   *
   * Sets the time the history block was created.
   * @return Nanoseconds since 1970.
   */
  [[nodiscard]] virtual uint64_t Time() const = 0;

  /** \brief Returns the start timestamp of the measurement.
   *
   * This function returns the start timestamp of the measurement as a pointer
   * to an IMdfTimestamp object.
   * @return Pointer to an IMdfTimestamp object representing the start
   * timestamp.
   */
  [[nodiscard]] virtual const mdf::IMdfTimestamp* StartTimestamp() const = 0;

  /** \brief Returns an interface against a MD4 block
   *
   * @return Pointer to an meta data block.
   */
  [[nodiscard]] virtual IMetaData* CreateMetaData() = 0;

  /** \brief Returns an constant interface against a MD4 block
   *
   * @return Pointer to an meta data block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  /** \brief Sets the description for the history block.
   *
   * Sets the description of the history block. The value is stored
   * int the TX tag in the meta data block.
   * @param description Description of history block.
   */
  void Description(const std::string& description) {
    auto* md4 = CreateMetaData();
    if (md4 != nullptr) {
      md4->StringProperty("TX", description);
    }
  }

  /** \brief Returns the description.
   *
   * Returns the description.
   * @return Description of the history block.
   */
  [[nodiscard]] std::string Description() const {
    const auto* md4 = MetaData();
    return md4 != nullptr ? md4->StringProperty("TX") : std::string();
  }

  /** \brief Sets the tool name.
   *
   * Mandatory text that identify the tool name.
   * @param tool_name Name of the tool.
   */
  void ToolName(const std::string& tool_name) {
    auto* md4 = CreateMetaData();
    if (md4 != nullptr) {
      md4->StringProperty("tool_id", tool_name);
    }
  }

  /** \brief Returns the tool name.
   *
   * @return Tool name.
   */
  [[nodiscard]] std::string ToolName() const {
    const auto* md4 = MetaData();
    return md4 != nullptr ? md4->StringProperty("tool_id") : std::string();
  }

  /** \brief Sets the tool vendor.
   *
   * Mandatory text that identify the tool vendor.
   * @param tool_vendor Vendor name.
   */
  void ToolVendor(const std::string& tool_vendor) {
    auto* md4 = CreateMetaData();
    if (md4 != nullptr) {
      md4->StringProperty("tool_vendor", tool_vendor);
    }
  }

  /** \brief Returns the tool vendor.
   *
   * @return Tool vendor.
   */
  [[nodiscard]] std::string ToolVendor() const {
    const auto* md4 = MetaData();
    return md4 != nullptr ? md4->StringProperty("tool_vendor") : std::string();
  }

  /** \brief Sets the tool version.
   *
   * Mandatory text that identify the tool version.
   * @param tool_version Version string.
   */
  void ToolVersion(const std::string& tool_version) {
    auto* md4 = CreateMetaData();
    if (md4 != nullptr) {
      md4->StringProperty("tool_version", tool_version);
    }
  }

  /** \brief Returns the tool version.
   *
   * @return Tool version.
   */
  [[nodiscard]] std::string ToolVersion() const {
    const auto* md4 = MetaData();
    return md4 != nullptr ? md4->StringProperty("tool_version") : std::string();
  }

  /** \brief Sets the user name.
   *
   * Optional text that identify the user.
   * @param user User name.
   */
  void UserName(const std::string& user) {
    auto* md4 = CreateMetaData();
    if (md4 != nullptr) {
      md4->StringProperty("user_name", user);
    }
  }

  /** \brief Returns the user name.
   *
   * @return User name.
   */
  [[nodiscard]] std::string UserName() const {
    const auto* md4 = MetaData();
    return md4 != nullptr ? md4->StringProperty("user_name") : std::string();
  }

  void SetFhComment(const FhComment& fh_comment);
  void GetFhComment(FhComment& fh_comment) const;
};

}  // namespace mdf