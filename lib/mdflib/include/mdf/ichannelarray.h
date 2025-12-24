/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelarray.h
 * \brief Defines a channel array (CA) block.
 *
 * The interface class define a channel array definition. The channel array (CA)
 * is appended to an channel block (CN). The channel shall now be treated an
 * array channel.
 *
 * Note that only simple single arrays are supported.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "mdf/iblock.h"

namespace mdf {

class IChannel;
class IChannelConversion;
class IChannelGroup;
class IDataGroup;

/** \brief Type of array.
 *
 */
enum class ArrayType : uint8_t {
  Array = 0, ///< Simple array without attributes
  ScalingAxis = 1, ///< Scaling axis.
  LookUp = 2, ///< Lookup array.
  IntervalAxis = 3, ///< Interval axis.
  ClassificationResult = 4 ///< Classification result.
};

/** \brief Type of storage. */
enum class ArrayStorage : uint8_t {
  CnTemplate = 0, ///< Channel template.
  CgTemplate = 1, ///< Channel group template.
  DgTemplate = 2  ///< Data group template.
};

/** \brief Channel array (CA) block flags. */
namespace CaFlag {
constexpr uint32_t DynamicSize = 0x0001; ///< Dynamic size
constexpr uint32_t InputQuantity = 0x0002; ///< Input quantity.
constexpr uint32_t OutputQuantity = 0x0004; ///< Output quantity.
constexpr uint32_t ComparisonQuantity = 0x0008; ///< Comparison quantity.
constexpr uint32_t Axis = 0x0010; ///< Axis
constexpr uint32_t FixedAxis = 0x0020; ///< Fixed axis.
constexpr uint32_t InverseLayout = 0x0040; ///< Inverse layout.
constexpr uint32_t LeftOpenInterval = 0x0080; ///< Left-over interval.
constexpr uint32_t StandardAxis = 0x0100; ///< Standard axis.
}  // namespace CaFlag

/** \brief Structure that defines the channel array (CA) blocks references.*/
struct  CaTripleReference {
  const mdf::IDataGroup* DataGroup = nullptr; ///< Pointer to the data group (DG) block
  const mdf::IChannelGroup* ChannelGroup = nullptr; ///< Pointer to the channel group (CG) block.
  const mdf::IChannel* Channel = nullptr; ///< Pointer to the channel (CN) block.
};


/** \brief Defines an interface against an array (CA) block.
 *
 * This interface class defines an array of channel values. A channel (CN)
 * block is defined as an array by appending a channel array (CA) block. The
 * CN block defines an array value while the CA block defines the array
 * dimension and its size(s).
 */
class IChannelArray : public IBlock {
 public:

  virtual void Type(ArrayType type) = 0; ///< Sets the type of array..
  [[nodiscard]] virtual ArrayType Type() const = 0; ///< Type of array.

  virtual void Storage(ArrayStorage storage) = 0; ///< Storage type
  [[nodiscard]] virtual ArrayStorage Storage() const = 0; ///< Storage type

  virtual void Flags(uint32_t flags) = 0; ///< Flags
  [[nodiscard]] virtual uint32_t Flags() const = 0; ///< Flags

  [[nodiscard]] virtual size_t Dimensions() const = 0; ///< Number of dimensions.

  /** \brief Sets the dimension and each dimension size.
   *
   * The function sets the number of dimensions and each dimension size.
   * @param dim_sizes Array of dimension sizes.
   */
  virtual void Shape(const std::vector<uint64_t>& dim_sizes) = 0;

  /** \brief Returns the dimension and each dimension size.
   *
   * @return Array of dimension sizes.
   */
  [[nodiscard]] virtual const std::vector<uint64_t>& Shape() const = 0;

  /** \brief Returns a dimension size. */
  [[nodiscard]] virtual uint64_t DimensionSize(size_t dimension) const = 0;

  /** \brief Returns the fixed axis value list for reading only. */
  [[nodiscard]] virtual const std::vector<double>&  AxisValues() const = 0;

  /** \brief Returns the fixed axis value list for write. */
  [[nodiscard]] virtual std::vector<double>&  AxisValues() = 0;

  /** \brief Returns a list of cycle counts. */
  [[nodiscard]] virtual const std::vector<uint64_t>& CycleCounts() const = 0;

  /** \brief Returns a list of cycle counts. */
  [[nodiscard]] virtual std::vector<uint64_t>& CycleCounts() = 0;

  /** \brief Returns the number of values in the array.
   *
   * @return Number of values in the array
   */
  [[nodiscard]] uint64_t NofArrayValues() const;

  /** \brief Returns the array dimensions as string ("N[2][3]")
   *
   * @return Array dimension as a string.
   */
  [[nodiscard]] std::string DimensionAsString() const;

  /** \brief Converts the array link indexes into pointers.
   *
   * This function can be called before setting any of the
   * link lists on the array object. It must be called later after the
   * block have been configured i.e. sized and all flags have been set.
   *
   * Note that this is a convenient function. It is possible to do it manual
   * as well.
   */
  virtual void ResizeArrays();

  /** \brief Returns a list of data kink indexes. */
  [[nodiscard]] const std::vector<int64_t>& DataLinks() const {
    return data_links_;
  }
  /** \brief Returns a list of data kink indexes. */
  [[nodiscard]] std::vector<int64_t>& DataLinks();

  /** \brief Returns the dynamic size list. */
  [[nodiscard]] const std::vector<CaTripleReference>& DynamicSizeList() const {
    return dynamic_size_list_;
  }
  /** \brief Returns the dynamic size list. */
  [[nodiscard]] std::vector<CaTripleReference>& DynamicSizeList();

  /** \brief Returns the input quantity list. */
  [[nodiscard]] const std::vector<CaTripleReference>& InputQuantityList() const {
    return input_quantity_list_;
  }
  /** \brief Returns the input quantity list. */
  [[nodiscard]] std::vector<CaTripleReference>& InputQuantityList();

  /** \brief Returns the output quantity reference if it exist. */
  [[nodiscard]] const CaTripleReference& OutputQuantity() const {
    return output_quantity_;
  }
  /** \brief Returns the output quantity reference if it exist. */
  [[nodiscard]] CaTripleReference& OutputQuantity() {
    return output_quantity_;
  }

  /** \brief Returns the comparison quantity list. */
  [[nodiscard]] const CaTripleReference& ComparisonQuantity() const {
    return comparison_quantity_;
  }
  /** \brief Returns the comparison quantity list. */
  [[nodiscard]] CaTripleReference& ComparisonQuantity()  {
    return comparison_quantity_;
  }

  /** \brief Returns the axis conversion list. */
  [[nodiscard]] const std::vector<const IChannelConversion*>& AxisConversionList() const {
    return axis_conversion_list_;
  }
  /** \brief Returns the axis conversion list. */
  [[nodiscard]] std::vector<const IChannelConversion*>& AxisConversionList();

  /** \brief Returns the axis list. */
  [[nodiscard]] const std::vector<CaTripleReference>& AxisList() const {
    return axis_list_;
  }
  /** \brief Returns the axis list. */
  [[nodiscard]] std::vector<CaTripleReference>& AxisList();

  /** \brief Summation of all array dimensions.
   *
   * Support function that summarizes the dimension sizes. This
   * function is mainly used internally to size
   * @return
   */
  [[nodiscard]] virtual uint64_t SumOfArray() const = 0;

  /** \brief Calculates the product of array dimension sizes.
   *
   * Returns the product of array sizes. This defines number of
   * values in an array sample.
   * @return Number of array values in a sample.
   */
  [[nodiscard]] virtual uint64_t ProductOfArray() const = 0;
 protected:
  std::vector<int64_t> data_links_; ///< List of index to data blocks.
  std::vector<CaTripleReference> dynamic_size_list_; ///< Dynamic size list.
  std::vector<CaTripleReference> input_quantity_list_; ///< Input quantity list.
  CaTripleReference output_quantity_; ///< Output quantity list.
  CaTripleReference comparison_quantity_; ///< Comparison quantity list.
  std::vector<const IChannelConversion*> axis_conversion_list_; ///< Axis_conversion list.
  std::vector<CaTripleReference> axis_list_; ///< Axis list.


};

}  // namespace mdf
