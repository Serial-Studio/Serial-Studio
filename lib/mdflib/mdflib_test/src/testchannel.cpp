/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>
#include "cn4block.h"
#include <string_view>
#include <boost/endian.hpp>

using namespace mdf::detail;
using namespace boost::endian;

namespace {
constexpr std::string_view kUtf8Chinese = "中文";
constexpr std::string_view kUtf8SuperHero = "超级英雄";
constexpr std::string_view kUnit = "km/h";
} // end namespace empty namespace

namespace mdf::test {

TEST(Cn4Block, TestProperties) {
  Cn4Block channel;

  EXPECT_EQ(channel.Index(), 0);
  EXPECT_STREQ(channel.BlockType().c_str(), "CN");

  EXPECT_EQ(channel.MetaData(), nullptr);
  auto* meta = channel.CreateMetaData();
  EXPECT_EQ(channel.MetaData(), meta);

  channel.Name(kUtf8Chinese.data());
  std::cout << "Name: " << channel.Name() << std::endl;
  EXPECT_STREQ(channel.Name().c_str(), kUtf8Chinese.data() );

  channel.Description(kUtf8SuperHero.data());
  std::cout << "Description: " << channel.Description() << std::endl;
  meta = channel.MetaData();
  std::cout << meta->XmlSnippet() << std::endl;
  EXPECT_STREQ(channel.Description().c_str(), kUtf8SuperHero.data() );

  EXPECT_FALSE(channel.IsUnitValid()); // Test default
  channel.Unit(kUnit.data());
  EXPECT_STREQ(channel.Unit().c_str(), kUnit.data() );
  EXPECT_TRUE(channel.IsUnitValid());

  EXPECT_EQ(channel.Flags(), 0); // Test default
  channel.Flags(0xAAAA);
  EXPECT_EQ(channel.Flags(), 0xAAAA);
  channel.Flags(0); // Reset flags for the time being

  EXPECT_EQ(channel.Type(), ChannelType::FixedLength); // Test default
  for (uint8_t type = 0; type <= static_cast<uint8_t>(ChannelType::VirtualData); ++type) {
    channel.Type(static_cast<ChannelType>(type));
    EXPECT_EQ(channel.Type(), static_cast<ChannelType>(type));
  }

  EXPECT_EQ(channel.Sync(), ChannelSyncType::None); // Test default
  for (uint8_t sync = 0; sync <= static_cast<uint8_t>(ChannelSyncType::Index); ++sync) {
    channel.Sync(static_cast<ChannelSyncType>(sync));
    EXPECT_EQ(channel.Sync(), static_cast<ChannelSyncType>(sync));
  }

  EXPECT_EQ(channel.DataType(), ChannelDataType::UnsignedIntegerLe); // Test default.
  for (uint8_t data = 0; data <= static_cast<uint8_t>(ChannelDataType::ComplexBe); ++data) {
    channel.DataType(static_cast<ChannelDataType>(data));
    EXPECT_EQ(channel.DataType(), static_cast<ChannelDataType>(data));
  }

  // EXPECT_EQ(channel.DataBytes(), 0); // Test default. Destroyed by previous loop
  for (uint64_t nof_bytes = 0; nof_bytes < 64; ++nof_bytes) {
    channel.DataBytes(nof_bytes);
    EXPECT_EQ(channel.DataBytes(), nof_bytes) << "Expected: " << nof_bytes;
  }

  EXPECT_FALSE(channel.IsDecimalUsed()); // Default
  channel.Decimals(8);
  EXPECT_EQ(channel.Decimals(),8);
  EXPECT_TRUE(channel.IsDecimalUsed());

  EXPECT_FALSE(channel.Range().has_value()); // Default
  channel.Range(-11, 11);
  EXPECT_TRUE(channel.Range().has_value());
  EXPECT_EQ(channel.Range()->first, -11);
  EXPECT_EQ(channel.Range()->second, 11);

  EXPECT_FALSE(channel.Limit().has_value()); // Default
  channel.Limit(-12, 12);
  EXPECT_TRUE(channel.Limit().has_value());
  EXPECT_EQ(channel.Limit()->first, -12);
  EXPECT_EQ(channel.Limit()->second, 12);

  EXPECT_FALSE(channel.ExtLimit().has_value()); // Default
  channel.ExtLimit(-10, 10);
  EXPECT_TRUE(channel.ExtLimit().has_value());
  EXPECT_EQ(channel.ExtLimit()->first, -10);
  EXPECT_EQ(channel.ExtLimit()->second, 10);

  EXPECT_EQ(channel.SourceInformation(), nullptr);
  auto* source = channel.CreateSourceInformation();
  EXPECT_EQ(channel.SourceInformation(), source);

  EXPECT_TRUE(channel.ChannelArrays().empty());
  const auto* array = channel.CreateChannelArray();
  EXPECT_EQ(channel.ChannelArray(0),array);

  // Do number test before CC test
  channel.DataType(ChannelDataType::FloatBe);
  EXPECT_TRUE(channel.IsNumber());
  channel.DataType(ChannelDataType::MimeSample);
  EXPECT_FALSE(channel.IsNumber());

  EXPECT_EQ(channel.ChannelConversion(), nullptr);
  auto* conversion = channel.CreateChannelConversion();
  EXPECT_EQ(channel.ChannelConversion(), conversion);

  EXPECT_EQ(channel.ChannelCompositions().size(), 0);
  channel.CreateChannelComposition();
  EXPECT_EQ(channel.ChannelCompositions().size(), 1);

  EXPECT_EQ(channel.VlsdRecordId(), 0);
  channel.VlsdRecordId(1234);
  EXPECT_EQ(channel.VlsdRecordId(), 1234);
}
TEST(Cn4Block, TestDescription) {
  Cn4Block channel;

  channel.Description(kUtf8SuperHero.data());
  std::cout << "Description: " << channel.Description() << std::endl;
  EXPECT_STREQ(channel.Description().c_str(), kUtf8SuperHero.data() );
}

TEST(Cn4Block, TestUnsignedLe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::UnsignedIntegerLe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::UnsignedIntegerLe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  for (uint32_t bit_count = 1; bit_count <= 64; ++bit_count) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    uint64_t simple_value = 0;
    uint64_t offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_EQ(simple_value,offset_value);

    // Validate with the boost endian
    switch (bit_count) {
      case 16: {
        auto expected = load_little_u16(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 24: {
        auto expected = load_little_u24(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 32: {
        auto expected = load_little_u32(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 40: {
        auto expected = load_little_u40(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 48: {
        auto expected = load_little_u48(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 56: {
        auto expected = load_little_u56(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 64: {
        auto expected = load_little_u64(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }

      default:
        break;
    }

    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
      << (simple_valid ? "True" : "False") << std::endl;
  }
}

TEST(Cn4Block, TestSignedLe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::SignedIntegerLe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::SignedIntegerLe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  for (uint32_t bit_count = 1; bit_count <= 64; ++bit_count) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    int64_t simple_value = 0;
    int64_t offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_EQ(simple_value,offset_value);

    // Validate with the boost endian
    switch (bit_count) {
      case 16: {
        auto expected = load_little_s16(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 24: {
        auto expected = load_little_s24(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 32: {
        auto expected = load_little_s32(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 40: {
        auto expected = load_little_s40(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 48: {
        auto expected = load_little_s48(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 56: {
        auto expected = load_little_s56(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 64: {
        auto expected = load_little_s64(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }

      default:
        break;
    }
    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
              << (simple_valid ? "True" : "False") << std::endl;
  }
}

TEST(Cn4Block, TestUnsignedBe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::UnsignedIntegerBe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::UnsignedIntegerBe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  for (uint32_t bit_count = 1; bit_count <= 64; ++bit_count) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    uint64_t simple_value = 0;
    uint64_t offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_EQ(simple_value,offset_value);

    // Validate with the boost endian
    switch (bit_count) {
      case 16: {
        auto expected = load_big_u16(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 24: {
        auto expected = load_big_u24(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 32: {
        auto expected = load_big_u32(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 40: {
        auto expected = load_big_u40(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 48: {
        auto expected = load_big_u48(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 56: {
        auto expected = load_big_u56(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 64: {
        auto expected = load_big_u64(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }

      default:
        break;
    }
    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
              << (simple_valid ? "True" : "False") << std::endl;
  }
}

TEST(Cn4Block, TestSignedBe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::SignedIntegerBe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::SignedIntegerBe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  for (uint32_t bit_count = 1; bit_count <= 64; ++bit_count) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    int64_t simple_value = 0;
    int64_t offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_EQ(simple_value,offset_value);

    // Validate with the boost endian
    switch (bit_count) {
      case 16: {
        auto expected = load_big_s16(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 24: {
        auto expected = load_big_s24(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 32: {
        auto expected = load_big_s32(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 40: {
        auto expected = load_big_s40(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 48: {
        auto expected = load_big_s48(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 56: {
        auto expected = load_big_s56(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }
      case 64: {
        auto expected = load_big_s64(record_buffer.data());
        EXPECT_EQ(simple_value,expected);
        break;
      }

      default:
        break;
    }
    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
              << (simple_valid ? "True" : "False") << std::endl;
  }
}

TEST(Cn4Block, TestFloatLe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::FloatLe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::FloatLe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  constexpr std::array<uint32_t,3> count_list = {16,32,64};
  for (const auto bit_count : count_list) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    double simple_value = 0;
    double offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_DOUBLE_EQ(simple_value,offset_value);

    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
              << (simple_valid ? "True" : "False") << std::endl;
  }
}

TEST(Cn4Block, TestFloatBe) {
  const std::vector<uint8_t> record_buffer(10, 0xAA);

  Cn4Block simple;
  simple.DataType(ChannelDataType::FloatBe);
  simple.BitOffset(0); // Offset 0..7
  simple.ByteOffset(0);
  EXPECT_EQ(simple.BitOffset(), 0);
  EXPECT_EQ(simple.ByteOffset(), 0);

  Cn4Block offset;
  offset.DataType(ChannelDataType::FloatBe);
  offset.BitOffset(4); // Offset 0..7
  offset.ByteOffset(1);
  EXPECT_EQ(offset.BitOffset(), 4);
  EXPECT_EQ(offset.ByteOffset(), 1);

  // First verify conversion against the boost endian
  constexpr std::array<uint32_t,3> count_list = {16,32,64};
  for (const auto bit_count : count_list) {
    simple.BitCount(bit_count);
    offset.BitCount(bit_count);

    EXPECT_EQ(simple.BitCount(), bit_count);
    EXPECT_EQ(offset.BitCount(), bit_count);

    double simple_value = 0;
    double offset_value = 0;
    bool simple_valid;
    bool offset_valid;
    try {
      simple_valid = simple.GetChannelValue(record_buffer, simple_value,0);
      offset_valid = offset.GetChannelValue(record_buffer, offset_value,0);
    } catch (const std::exception& ) {
      simple_valid = false;
      offset_valid = false;
    }
    EXPECT_TRUE(simple_valid);
    EXPECT_TRUE(offset_valid);
    EXPECT_DOUBLE_EQ(simple_value,offset_value);

    std::cout << "Value[" << bit_count << "]: " << simple_value << "/"
              << (simple_valid ? "True" : "False") << std::endl;
  }
}



} // End namespace mdf::test