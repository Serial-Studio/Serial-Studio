/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>

#include <gtest/gtest.h>

#include "mdf/cncomment.h"
#include "mdf/cnunit.h"

namespace mdf::test {

TEST(TestCnComment, Properties) {
  CnComment comment;

  constexpr std::string_view kTx = "CN Test Properties";
  comment.Comment(MdString(kTx));

  constexpr std::string_view kLinkerName = "Germany";
  comment.LinkerName(MdString(kLinkerName));

  constexpr uint64_t kLinkerAddress = 0x123456;
  comment.LinkerAddress(MdNumber(kLinkerAddress, MdDataType::MdHex));

  constexpr MdMonotony kMonotony = MdMonotony::Monotonous;
  comment.AxisMonotony(kMonotony);

  constexpr double kRaster = 1.23;
  comment.Raster(MdNumber(kRaster));

  constexpr uint64_t kAddress = 0xABCDEF;
  comment.Address(MdNumber(kAddress, MdDataType::MdHex));

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  CnComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  EXPECT_EQ(comment1.LinkerName(), kLinkerName);
  EXPECT_EQ(comment1.LinkerAddress(), kLinkerAddress);
  EXPECT_EQ(comment1.AxisMonotony(), kMonotony);
  EXPECT_EQ(comment1.Raster(), kRaster);
  EXPECT_EQ(comment1.Address(), kAddress);
}

TEST(TestCnComment, MdNumber) {
  CnComment comment;

  constexpr std::string_view kTx = "CN Test MdNumber";
  comment.Comment(MdString(kTx));


  MdNumber linker_address;
  EXPECT_FALSE(linker_address.IsActive());

  linker_address.DataType(MdDataType::MdHex);
  EXPECT_EQ(linker_address.DataType(), MdDataType::MdHex);

  constexpr uint64_t kByteCount = 4;
  linker_address.ByteCount(kByteCount);
  EXPECT_EQ(linker_address.ByteCount(), kByteCount);

  constexpr uint64_t kBitMask = 0xFFFF;
  linker_address.BitMask(kBitMask);
  EXPECT_EQ(linker_address.BitMask(), kBitMask);

  const MdByteOrder kByteOrder = MdByteOrder::BigEndian;
  linker_address.ByteOrder(kByteOrder);
  EXPECT_EQ(linker_address.ByteOrder(), kByteOrder);

  constexpr uint64_t kLinkerAddress = 0x123456;
  linker_address.Number(kLinkerAddress);
  EXPECT_EQ(linker_address.Number<uint64_t>(), kLinkerAddress);
  EXPECT_TRUE(linker_address.IsActive());

  comment.LinkerAddress(linker_address);


  MdNumber raster;

  constexpr double kMin = -12.3;
  EXPECT_EQ(raster.Min(), std::nullopt);
  raster.Min(kMin);
  EXPECT_DOUBLE_EQ(raster.Min().value(), kMin);

  constexpr double kMax = 12.3;
  raster.Max(kMax);
  EXPECT_DOUBLE_EQ(raster.Max().value(), kMax);

  constexpr double kAverage = -1.23;
  raster.Average(kAverage);
  EXPECT_EQ(raster.Average(), kAverage);

  constexpr std::string_view kUnit = "m";
  raster.Unit(std::string(kUnit));
  EXPECT_EQ(raster.Unit(), kUnit);

  constexpr std::string_view kUnitRef = "Yard";
  raster.UnitRef(std::string(kUnitRef));
  EXPECT_EQ(raster.UnitRef(), kUnitRef);

  constexpr double kRaster = 1.23;
  raster.Number(kRaster);
  EXPECT_EQ(raster.Number<double>(), kRaster);

  comment.Raster(raster);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  CnComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  const auto& linker_address1 = comment1.LinkerAddress();
  EXPECT_EQ(linker_address1.Number<uint64_t>(), kLinkerAddress);
  EXPECT_EQ(linker_address1.ByteCount(), kByteCount);
  EXPECT_EQ(linker_address1.BitMask(), kBitMask);
  EXPECT_EQ(linker_address1.ByteOrder(), kByteOrder);

 const auto& raster1 = comment1.Raster();
 EXPECT_DOUBLE_EQ(raster1.Number<double>(), kRaster);
 EXPECT_DOUBLE_EQ(raster1.Min().value(), kMin);
 EXPECT_DOUBLE_EQ(raster1.Max().value(), kMax);
 EXPECT_DOUBLE_EQ(raster1.Average().value(), kAverage);
 EXPECT_EQ(raster1.Unit(), kUnit);
 EXPECT_EQ(raster1.UnitRef(), kUnitRef);
}

TEST(TestCnComment, CnUnit) {
  CnUnit unit;

  constexpr std::string_view kUnit = "miles";
  unit.Unit(MdString(kUnit));

  constexpr std::string_view kUnitRef = "km";
  unit.UnitRef(MdString(kUnitRef));

  const std::string xml_snippet = unit.ToXml();
  std::cout << xml_snippet << std::endl;

  CnUnit unit1;
  unit1.FromXml(xml_snippet);
  EXPECT_EQ(unit1.Comment(), kUnit);
  EXPECT_EQ(unit1.Unit(), kUnit);
  EXPECT_EQ(unit1.UnitRef(), kUnitRef);

}
} // end namespace
