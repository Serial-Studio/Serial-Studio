/*
 * * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file iblock.h
 * \brief All MDF blocks inherits from the IBlock class. The interface class
 * is used internally in lists. The user should not use this class.
 */
#pragma once
#include <cstdint>
#include <string>


namespace mdf {
/** \brief Base class for all MDF blocks.
 *
 */
class IBlock {
 public:
  virtual ~IBlock() = default; ///< Default destructor

  /** \brief File position within the file.
   *
   * The Index() function is the blocks file position in the file. The
   * position gives each block a unique index/identity.
   *
   * If file position is 0, the block have not been written yet. This
   * is frequently used to detect if the block should be written or updated.
   * @return File position of the block.
   */
  [[nodiscard]] virtual int64_t Index() const = 0;

  /** \brief Returns the block type.
   *
   * The block type is 2 character string for example 'AT' and 'DT'. In MDF 4,
   * the block type is actually 4 characters but this function removes
   * the two first characters.
   * @return Block type (2 characters length).
   */
  [[nodiscard]] virtual std::string BlockType() const = 0;

};

}  // namespace mdf
