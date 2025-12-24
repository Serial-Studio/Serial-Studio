/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelhierarchy.h
 * \brief The channel hierarchy (CH) block defines channel dependencies.
 */
#pragma once
#include <cstdint>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/iblock.h"
#include "mdf/chcomment.h"

namespace mdf {

/** \brief Support structure that holds a DG/CG/CN relation. */
struct ElementLink {
  const IDataGroup* data_group = nullptr; ///< Pointer to a DG block.
  const IChannelGroup* channel_group = nullptr; ///< Pointer to a CG block.
  const IChannel* channel = nullptr; ///< Pointer to a CN block.
};

/** \brief Block type */
enum class ChType : uint8_t {
  Group = 0, ///< Define a group.
  Function = 1, ///< Define a function.
  Structure = 2, ///< Define a structure.
  MapList = 3, ///< Define a map list.
  InputVariable = 4, ///< Define input variables.
  OutputVariable = 5, ///< Define output variables.
  LocalVariable = 6, ///< Define local variables.
  CalibrationDefinition = 7, ///< Define calibration define objects.
  CalibrationObject = 8 ///< Define calibration reference objects.
};

/** \brief Channel hierarchy (CH) block. */
class IChannelHierarchy : public IBlock {
 public:

  virtual void Name(const std::string& name) = 0; ///< Sets name.
  [[nodiscard]] virtual const std::string& Name() const = 0; ///< Name.

  virtual void Type(ChType type) = 0; ///< Sets the block type.
  [[nodiscard]] virtual ChType Type() const = 0; ///< Block type.
  [[nodiscard]] std::string TypeToString() const; ///< Block type as text.

  virtual void Description(const std::string& description)
      = 0; ///< Sets the descriptive text.
  [[nodiscard]] virtual std::string Description() const = 0; ///< Description.

  /** \brief Returns an interface against an MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual IMetaData* CreateMetaData() = 0;

  /** \brief Returns an constant interface against a MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  /** \brief Adds an element link. */
  virtual void AddElementLink(const ElementLink& element) = 0;

  /** \brief Returns a list of element links. */
  [[nodiscard]] virtual const std::vector<ElementLink>& ElementLinks()
      const = 0;

  /** \brief Create a CH block. */
  [[nodiscard]] virtual IChannelHierarchy* CreateChannelHierarchy() = 0;

  /** \brief Returns a list of CH blocks. */
  [[nodiscard]] virtual std::vector<IChannelHierarchy*> ChannelHierarchies()
      const = 0;

  void SetChComment(const ChComment& ch_comment);
  void GetChComment(ChComment& ch_comment) const;
};
}  // namespace mdf