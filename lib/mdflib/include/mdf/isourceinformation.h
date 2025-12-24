/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file isourceinformation.h
 * \brief Interface against a source information (SI) block.
 *
 */
#pragma once
#include <cstdint>
#include <string>

#include "mdf/imetadata.h"
#include "mdf/iblock.h"
#include "mdf/sicomment.h"

namespace mdf {

/** \brief Type of source information. */
enum class SourceType : uint8_t {
  Other = 0, ///< Unknown source type.
  Ecu = 1, ///< ECU.
  Bus = 2, ///< Bus.
  IoDevice = 3, ///< I/O device.
  Tool = 4, ///< Tool.
  User = 5 ///< User.
};

/** \brief Type of bus. */
enum class BusType : uint8_t {
  None = 0, ///< No bus (default).
  Other = 1, ///< Unknown bus type.
  Can = 2, ///< CAN bus.
  Lin = 3, ///< LIN bus.
  Most = 4, ///< MOST bus.
  FlexRay = 5, ///< FlexRay bus.
  Kline = 6, ///< KLINE bus.
  Ethernet = 7, ///< EtherNet bus.
  Usb = 8 ///< USB bus.
};

/** \brief Source information flags. */
namespace SiFlag {
constexpr uint8_t Simulated = 0x01; ///< Simulated device.
}

/** \brief Interface to a source information (SI) block.
 *
 * The source information (SI) describe the test equipment or its environment.
 */
class ISourceInformation : public IBlock {
 public:
  virtual void Name(const std::string& name) = 0; ///< Sets the name.
  [[nodiscard]] virtual const std::string& Name() const = 0; ///< Name.

  virtual void Path(const std::string& path) = 0; ///< Sets the path.
  [[nodiscard]] virtual const std::string& Path() const = 0; ///< Path.

  /** \brief Sets the descriptive text. */
  virtual void Description(const std::string& desc) = 0;
  /** \brief Return the descriptive text. */
  [[nodiscard]] virtual std::string Description() const = 0;

  virtual void Type(SourceType type) = 0; ///< Sets the type of source.
  [[nodiscard]] virtual SourceType Type() const = 0; ///< Type of source.

  virtual void Bus(BusType type) = 0; ///< Set the type of bus.
  [[nodiscard]] virtual BusType Bus() const = 0; ///< Type of bus.

  virtual void Flags(uint8_t flags) = 0; ///< Set the SiFlag.
  [[nodiscard]] virtual uint8_t Flags() const = 0; ///< Returns SiFlag.

  /** \brief Creates a meta-data (MD) block. */
  [[nodiscard]] virtual IMetaData* CreateMetaData() = 0;
  /** \brief Returns an existing meta-data (MD) block. */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  void SetSiComment(const SiComment& si_comment);
  void GetSiComment(SiComment& si_comment) const;
};

}  // end namespace mdf