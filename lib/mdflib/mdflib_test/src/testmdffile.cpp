/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>

#include <string_view>

#include "mdf/imetadata.h"
#include "mdf/mdffactory.h"
#include "mdf/mdffile.h"

namespace {
constexpr std::string_view kFileName = "c:/olle.dat";
}

namespace mdf::test {

TEST(TestMdf, Mdf3File) {  // NOLINT
  auto mdf = MdfFactory::CreateMdfFile(MdfFileType::Mdf3FileType);
  ASSERT_TRUE(mdf);
  std::cout << "MDF Version: " << mdf->Version() << std::endl;
  EXPECT_TRUE(mdf->Version() > "3.00" && mdf->Version() < "4.00");
  EXPECT_EQ(mdf->MainVersion(), 3);
  EXPECT_TRUE(mdf->MinorVersion() >= 0 && mdf->MinorVersion() <= 30);

  mdf->MinorVersion(-10);
  EXPECT_TRUE(mdf->Version() == "3.00");

  mdf->MinorVersion(0);
  EXPECT_TRUE(mdf->Version() == "3.00");

  mdf->MinorVersion(1);
  EXPECT_TRUE(mdf->Version() == "3.10");

  mdf->MinorVersion(10);
  EXPECT_TRUE(mdf->Version() == "3.10");

  mdf->MinorVersion(330);
  EXPECT_TRUE(mdf->Version() == "3.30");

  std::cout << "Default ProgramId: " << mdf->ProgramId() << std::endl;
  mdf->ProgramId("Olle43");
  EXPECT_TRUE(mdf->ProgramId() == "Olle43");

  EXPECT_TRUE(mdf->Header() != nullptr);

  AttachmentList attachment_list;
  mdf->Attachments(attachment_list);
  EXPECT_TRUE(attachment_list.empty());
  auto* at_group = mdf->CreateAttachment();
  EXPECT_TRUE(at_group == nullptr);
  attachment_list.clear();
  mdf->Attachments(attachment_list);
  EXPECT_TRUE(attachment_list.empty());

  DataGroupList meas_list;
  mdf->DataGroups(meas_list);
  EXPECT_TRUE(meas_list.empty());
  auto* dg_group = mdf->CreateDataGroup();
  EXPECT_TRUE(dg_group != nullptr);
  meas_list.clear();
  mdf->DataGroups(meas_list);
  EXPECT_EQ(meas_list.size(), 1);

  EXPECT_FALSE(mdf->IsMdf4());

  EXPECT_TRUE(mdf->Name().empty());
  mdf->Name("Olle");
  EXPECT_TRUE(mdf->Name() == "Olle");
  mdf->FileName(kFileName.data());
  EXPECT_TRUE(mdf->Name() == "Olle");

  mdf->Name("");
  mdf->FileName(kFileName.data());
  EXPECT_TRUE(!mdf->Name().empty());
  EXPECT_TRUE(mdf->FileName() == kFileName);

  std::cout << "File Name: " << mdf->FileName() << std::endl;
  std::cout << "Name: " << mdf->Name() << std::endl;
/*
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  EXPECT_TRUE(mdf->IsFinalized(standard_flags, custom_flags));
  mdf->IsFinalized(false, nullptr, 1, 2);
  EXPECT_FALSE(mdf->IsFinalized(standard_flags, custom_flags));
  EXPECT_EQ(standard_flags, 1);
  EXPECT_EQ(custom_flags, 2);
  mdf->IsFinalized(true, nullptr, 0, 0);
  EXPECT_TRUE(mdf->IsFinalized(standard_flags, custom_flags));
  EXPECT_EQ(standard_flags, 0);
  EXPECT_EQ(custom_flags, 0);
  */
}

TEST(TestMdf, Mdf4File) {  // NOLINT
  auto mdf = MdfFactory::CreateMdfFile(MdfFileType::Mdf4FileType);
  ASSERT_TRUE(mdf);
  std::cout << "MDF Version: " << mdf->Version() << std::endl;
  EXPECT_TRUE(mdf->Version() > "4.00" && mdf->Version() < "5.00");
  EXPECT_EQ(mdf->MainVersion(), 4);
  EXPECT_TRUE(mdf->MinorVersion() >= 0 && mdf->MinorVersion() <= 99);

  mdf->MinorVersion(-10);
  EXPECT_TRUE(mdf->Version() == "4.00");

  mdf->MinorVersion(0);
  EXPECT_TRUE(mdf->Version() == "4.00");

  mdf->MinorVersion(1);
  EXPECT_TRUE(mdf->Version() == "4.10");

  mdf->MinorVersion(10);
  EXPECT_TRUE(mdf->Version() == "4.10");

  mdf->MinorVersion(430);
  EXPECT_TRUE(mdf->Version() == "4.30");

  std::cout << "Default ProgramId: " << mdf->ProgramId() << std::endl;
  mdf->ProgramId("Olle43");
  EXPECT_TRUE(mdf->ProgramId() == "Olle43");

  EXPECT_TRUE(mdf->Header() != nullptr);

  AttachmentList attachment_list;
  mdf->Attachments(attachment_list);
  EXPECT_TRUE(attachment_list.empty());
  auto* at_group = mdf->CreateAttachment();
  EXPECT_TRUE(at_group != nullptr);
  attachment_list.clear();
  mdf->Attachments(attachment_list);
  EXPECT_FALSE(attachment_list.empty());

  DataGroupList meas_list;
  mdf->DataGroups(meas_list);
  EXPECT_TRUE(meas_list.empty());
  auto* dg_group = mdf->CreateDataGroup();
  EXPECT_TRUE(dg_group != nullptr);
  meas_list.clear();
  mdf->DataGroups(meas_list);
  EXPECT_EQ(meas_list.size(), 1);

  EXPECT_TRUE(mdf->IsMdf4());

  EXPECT_TRUE(mdf->Name().empty());
  mdf->Name("Olle");
  EXPECT_TRUE(mdf->Name() == "Olle");
  mdf->FileName(kFileName.data());
  EXPECT_TRUE(mdf->Name() == "Olle");

  mdf->Name("");
  mdf->FileName(kFileName.data());
  EXPECT_TRUE(!mdf->Name().empty());
  EXPECT_TRUE(mdf->FileName() == kFileName);

  std::cout << "File Name: " << mdf->FileName() << std::endl;
  std::cout << "Name: " << mdf->Name() << std::endl;
/*
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  EXPECT_TRUE(mdf->IsFinalized(standard_flags, custom_flags));
  mdf->IsFinalized(false, nullptr, 1, 2);
  EXPECT_FALSE(mdf->IsFinalized(standard_flags, custom_flags));
  EXPECT_EQ(standard_flags, 1);
  EXPECT_EQ(custom_flags, 2);
  mdf->IsFinalized(true, nullptr, 0, 0);
  EXPECT_TRUE(mdf->IsFinalized(standard_flags, custom_flags));
  EXPECT_EQ(standard_flags, 0);
  EXPECT_EQ(custom_flags, 0);
*/
  auto* header = mdf->Header();
  ASSERT_TRUE(header != nullptr);

}

}  // namespace mdf::test
