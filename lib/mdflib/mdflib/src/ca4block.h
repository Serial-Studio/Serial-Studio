/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/ichannelarray.h"
#include "mdfblock.h"
namespace mdf::detail {

class Cn4Block;

class Ca4Block : public MdfBlock, public IChannelArray {
 public:
  using Cx4List = std::vector<std::unique_ptr<MdfBlock>>;
  Ca4Block();

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Type(ArrayType type) override;
  [[nodiscard]] ArrayType Type() const override;
  [[nodiscard]] std::string TypeAsString() const;

  void Storage(ArrayStorage storage) override;
  [[nodiscard]] ArrayStorage Storage() const override;
  [[nodiscard]] std::string StorageAsString() const;

  void Flags(uint32_t flags) override;
  [[nodiscard]] uint32_t Flags() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;

  [[nodiscard]] size_t Dimensions() const override; ///< Number of dimensions

  void Shape(const std::vector<uint64_t>& dim_sizes) override;
  [[nodiscard]] const std::vector<uint64_t>& Shape() const override;

  [[nodiscard]] uint64_t DimensionSize(size_t dimension) const override; ///< Size for a dimension

  /** Returns the summation of the dimension sizes. */
  [[nodiscard]] uint64_t SumOfArray() const override;

  /** Returns the product of the dimension sizes. */
  [[nodiscard]] uint64_t ProductOfArray() const override;

  /** \brief Returns a list of axis values.
   *
   * @return List of axis values.
   */
  [[nodiscard]] const std::vector<double>& AxisValues() const override;

  /** \brief Returns a list of axis values.
   *
   * @return List of axis values.
   */
  [[nodiscard]] std::vector<double>& AxisValues() override;

  [[nodiscard]] const std::vector<uint64_t>& CycleCounts() const override;
  [[nodiscard]] std::vector<uint64_t>& CycleCounts() override;

  uint64_t Read(std::streambuf& buffer) override;
  void FindAllReferences(std::streambuf& buffer);

  uint64_t Write(std::streambuf& buffer) override;

  void SetParentChannel(const Cn4Block* parent);
  void PrepareForWriting();
 private:
  uint8_t type_ = 0;
  uint8_t storage_ = 0;
  uint16_t dimensions_ = 0; // Number of dimensions (N)
  uint32_t flags_ = 0;
  int32_t byte_offset_base_ = 0;
  uint32_t invalid_bit_pos_base_ = 0;
  const Cn4Block* parent_channel_ = nullptr;

  // Store number of items in each dimension (d).
  std::vector<uint64_t> dim_size_list_; ///< Dimension size for each dimension
  // Axis values
  std::vector<double> axis_value_list_;
  std::vector<uint64_t> cycle_count_list_;

  [[nodiscard]] CaTripleReference ReadReference(size_t index) const;

  void WriteReference(std::streambuf& buffer,
                      const CaTripleReference& ref, size_t start_index);
  size_t WriteReferences(std::streambuf& buffer,
                       const std::vector<CaTripleReference>& list,
                       size_t max_fill,
                       size_t start_index);
};
}  // namespace mdf::detail
