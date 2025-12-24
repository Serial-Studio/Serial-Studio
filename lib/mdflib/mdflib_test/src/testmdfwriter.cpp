/*
* Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <filesystem>
#include <iostream>

#include <gtest/gtest.h>
#include <mdf/mdfwriter.h>
#include "mdf/mdffactory.h"

using namespace std::filesystem;

namespace {
  std::string CreateTestFile(const std::string& filename) {
    try {
      path fullname = temp_directory_path();
      fullname.append("test");
      fullname.append("mdf");
      fullname.append("writer");
      create_directories(fullname);
      fullname.append(filename);
      remove(fullname);
      return fullname.string();
    } catch (const std::exception& err) {
      std::cout << "Filename error. File: " << filename
        << ", Error: " << err.what() << std::endl;

    }
    return {};
  }
}

namespace mdf::test {
TEST(MdfWriter, TestProperties) {
  auto writer = MdfFactory::CreateMdfWriter(
    MdfWriterType::MdfConverter);
  ASSERT_TRUE(writer);
  const std::string test_file = CreateTestFile("mdfwriter_prop.mf4");

  writer->PreTrigTime(1.23);
  EXPECT_FLOAT_EQ(writer->PreTrigTime(), 1.23);

  writer->PreTrigTime(1.0);
  EXPECT_EQ(writer->PreTrigTimeNs(), 1'000'000'000);

  writer->BusType(MdfBusType::FlexRay);
  EXPECT_EQ(writer->BusType(), MdfBusType::FlexRay);

  writer->StorageType(MdfStorageType::MlsdStorage);
  EXPECT_EQ(writer->StorageType(), MdfStorageType::MlsdStorage);

  writer->MaxLength(11);
  EXPECT_EQ(writer->MaxLength(), 11);

  writer->CompressData(true);
  EXPECT_EQ(writer->CompressData(), true);
  writer->CompressData(false);
  EXPECT_EQ(writer->CompressData(), false);

  writer->SavePeriodic(false);
  EXPECT_FALSE(writer->IsSavePeriodic());
  writer->SavePeriodic(true);
  EXPECT_TRUE(writer->IsSavePeriodic());

  writer->MandatoryMembersOnly(true);
  EXPECT_TRUE(writer->MandatoryMembersOnly());
  writer->MandatoryMembersOnly(false);
  EXPECT_FALSE(writer->MandatoryMembersOnly());

  ASSERT_TRUE(writer->Init(test_file));
  EXPECT_EQ(writer->Name(), "mdfwriter_prop");
  EXPECT_EQ(writer->TypeOfWriter(), MdfWriterType::MdfConverter);
  EXPECT_EQ(writer->State(), WriteState::Create);
  EXPECT_TRUE(writer->IsFileNew());

  EXPECT_EQ(writer->StartTime(), 0);
  EXPECT_EQ(writer->StopTime(), 0);

  EXPECT_TRUE(writer->GetFile() != nullptr);
  EXPECT_TRUE(writer->Header() != nullptr);


}

}