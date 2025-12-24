/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>
#include <array>
#include <sstream>

#include <gtest/gtest.h>

#include "mdf/hdcomment.h"
#include "mdf/mdfhelper.h"

namespace mdf::test {

TEST(TestHdComment, Properties) {
  HdComment comment;

  constexpr std::string_view kAuthor = "Road Runner";
  comment.Author(std::string(kAuthor));
  EXPECT_EQ(comment.Author(), kAuthor);

  constexpr std::string_view kDepartment = "ACME Ltd";
  comment.Department(std::string(kDepartment));
  EXPECT_EQ(comment.Department(), kDepartment);

  constexpr std::string_view kProject = "Exploding MDF";
  comment.Project(std::string(kProject));
  EXPECT_EQ(comment.Project(), kProject);

  constexpr std::string_view kSubject = "Instant Success";
  comment.Subject(std::string(kSubject));
  EXPECT_EQ(comment.Subject(), kSubject);

  constexpr std::string_view kMeasUuid = "&789";
  comment.MeasurementUuid(std::string(kMeasUuid));
  EXPECT_EQ(comment.MeasurementUuid(), kMeasUuid);

  constexpr std::string_view kRecorderUuid = "1234";
  comment.RecorderUuid(std::string(kRecorderUuid));
  EXPECT_EQ(comment.RecorderUuid(), kRecorderUuid);

  constexpr int64_t kRecorderFileIndex = 112;
  comment.RecorderFileIndex(kRecorderFileIndex);
  EXPECT_EQ(comment.RecorderFileIndex(), kRecorderFileIndex);

  constexpr std::string_view kTimeSource = "PC Computer";
  comment.TimeSource(MdString(kTimeSource));
  EXPECT_EQ(comment.TimeSource(), kTimeSource);

  constexpr double kPi = 3.14;
  comment.AddConstant(MdString("PI"), std::to_string(kPi) );
  EXPECT_EQ(comment.Constants().size(), 1);

  auto& unit_spec = comment.UnitSpecification();
  constexpr std::string_view kKiloMeter = "km";
  HoUnit unit;
  unit.Identity(std::string(kKiloMeter));
  unit.ShortName(std::string(kKiloMeter));
  unit_spec.AddUnit(unit);

  std::string xml_snippet = comment.ToXml();

  HdComment comment1;
  comment1.FromXml(xml_snippet);

  EXPECT_EQ(comment1.TimeSource(), comment.TimeSource());
  EXPECT_EQ(comment1.Constants().size(), 1);
  for ( const auto& [name, expression] : comment1.Constants()) {
    EXPECT_EQ(name.Text(), std::string("PI"));
    EXPECT_DOUBLE_EQ(std::stod(expression), 3.14);
  }

  const auto& unit_spec1 = comment1.UnitSpecification();
  EXPECT_EQ(unit_spec1.Units().size(), 1);

  std::cout << xml_snippet << std::endl;
}

TEST(TestHdComment, AdminData) {
  HdComment comment;
  auto& unit_spec = comment.UnitSpecification();

  auto& admin_data = unit_spec.AdminData();
  EXPECT_FALSE(admin_data.IsActive());

  HoRevision revision;
  constexpr std::string_view kTeamMember = "Wile E. Coyote";
  revision.TeamMemberReference(std::string(kTeamMember));

  constexpr std::string_view kLabel = "1.2";
  revision.RevisionLabel(std::string(kLabel));

  constexpr std::string_view kState = "Init";
  revision.State(std::string(kState));

  uint64_t now = MdfHelper::NowNs(); // Normalize to seconds
  now /= 1'000'000'000;
  now *= 1'000'000'000;
  revision.DateNs(now);

  HoCompanyRevision company_revision;
  constexpr std::string_view kDataRef = "ACME Corporation";
  company_revision.DataReference = kDataRef;
  company_revision.Label = kLabel;
  company_revision.State = kState;

  revision.AddCompanyRevision(company_revision);

  HoModification modification;
  constexpr std::string_view kChange = "Exploding Seeds";
  modification.Change = kChange;
  constexpr std::string_view kReason = "Catch Road Runner";
  modification.Reason = kReason;
  revision.AddModification(modification);

  admin_data.AddRevision(revision);

  EXPECT_TRUE(admin_data.IsActive());

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);

  const auto& unit_spec1  = comment1.UnitSpecification();
  const auto& admin_data1 = unit_spec1.AdminData();

  EXPECT_TRUE(admin_data1.IsActive());
  ASSERT_EQ(admin_data1.Revisions().size(), 1);
  const auto& revision1 = admin_data1.Revisions()[0];

  EXPECT_EQ(revision1.TeamMemberReference(), kTeamMember);
  EXPECT_EQ(revision1.RevisionLabel(), kLabel);
  EXPECT_EQ(revision1.State(), kState);
  EXPECT_EQ(revision1.DateNs(), now);

  ASSERT_EQ(revision1.CompanyRevisions().size(),1);
  const auto& company_revision1 = revision1.CompanyRevisions()[0];

  EXPECT_EQ(company_revision1.DataReference, kDataRef);
  EXPECT_EQ(company_revision1.Label, kLabel);
  EXPECT_EQ(company_revision1.State, kState);

  ASSERT_EQ(revision1.Modifications().size(),1);
  const auto& modification1 = revision1.Modifications()[0];

  EXPECT_EQ(modification1.Change, kChange);
  EXPECT_EQ(modification1.Reason, kReason);

}

TEST(TestHdComment, PhysicalDimension) {
  HdComment comment;
  auto& unit_spec = comment.UnitSpecification();
  EXPECT_FALSE(unit_spec.IsActive());

  HoPhysicalDimension dimension;

  constexpr std::string_view kShortName = "RR";
  dimension.ShortName(std::string(kShortName));
  EXPECT_EQ(dimension.ShortName(), kShortName);

  constexpr std::string_view kLongName = "Road Runner";
  constexpr uint64_t kCi = 22;
  const std::string_view kLang = "eng";
  MdString long_name(std::string(kLongName), kCi, std::string(kLang) );
  dimension.AddLongName(long_name);

  constexpr std::string_view kDesc = "Nemesis of Coyote";
  MdString description(std::string(kDesc), kCi, std::string(kLang) );
  dimension.AddDescription(description);

  const std::string_view kMeter = "m";
  dimension.Identity(std::string(kMeter));
  dimension.LengthExponent(1);

  unit_spec.AddPhysicalDimension(dimension);
  EXPECT_EQ(unit_spec.PhysicalDimensions().size(), 1);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);

  const auto& unit_spec1 = comment1.UnitSpecification();
  ASSERT_EQ(unit_spec1.PhysicalDimensions().size(), 1);

  for (const auto& [name, dim] : unit_spec1.PhysicalDimensions()) {
    EXPECT_EQ(name, kMeter);
    EXPECT_EQ(dim.Identity(), kMeter);
    EXPECT_DOUBLE_EQ(dim.LengthExponent(), 1);
    EXPECT_DOUBLE_EQ(dim.MassExponent(), 0);
  }
}

TEST(TestHdComment, UnitGroup) {
  HdComment comment;
  auto& unit_spec = comment.UnitSpecification();
  EXPECT_FALSE(unit_spec.IsActive());

  HoUnitGroup group;
  constexpr std::string_view kGroupName = "Length";
  group.ShortName(std::string(kGroupName));
  group.Category(HoUnitGroupCategory::EquivUnits);

  constexpr std::string_view kUnitRef = "km";
  group.AddUnitReference(std::string(kUnitRef));

  unit_spec.AddUnitGroup(group);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);

  auto& unit_spec1 = comment1.UnitSpecification();
  EXPECT_TRUE(unit_spec1.IsActive());
  EXPECT_EQ(unit_spec1.UnitGroups().size(), 1);

  for (const auto& [name, group1] : unit_spec1.UnitGroups()) {
    EXPECT_EQ(name, kGroupName);
    EXPECT_EQ(group1.ShortName(), kGroupName);
    EXPECT_EQ(group1.Category(), HoUnitGroupCategory::EquivUnits);
    EXPECT_EQ(group1.UnitRefs().size(), 1);
  }
}

TEST(TestHdComment, Unit) {
  HdComment comment;
  auto& unit_spec = comment.UnitSpecification();
  EXPECT_FALSE(unit_spec.IsActive());

  HoUnit unit;
  constexpr std::string_view kUnit = "km";
  unit.Identity(std::string(kUnit));
  unit.DisplayName(std::string(kUnit));
  unit.Factor(1000);
  unit.Offset(0);

  constexpr std::string_view kPhysDim = "m";
  unit.PhysicalDimensionRef(std::string(kPhysDim));

  unit_spec.AddUnit(unit);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);

  auto& unit_spec1 = comment1.UnitSpecification();
  EXPECT_TRUE(unit_spec1.IsActive());
  EXPECT_EQ(unit_spec1.Units().size(), 1);

  for (const auto& [name, unit1] : unit_spec1.Units()) {
    EXPECT_EQ(name, kUnit);
    EXPECT_EQ(unit1.DisplayName(), kUnit);
    EXPECT_DOUBLE_EQ(unit1.Factor(), 1000);
    EXPECT_DOUBLE_EQ(unit1.Offset(), 0);
    EXPECT_EQ(unit1.PhysicalDimensionRef(), kPhysDim);
  }
}

TEST(TestHdComment, MdComment) {
  HdComment comment;

  const std::string_view kTx = "Header Test";
  comment.Comment(MdString(kTx));
  EXPECT_EQ(comment.Comment().Text(), kTx);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment().Text(), kTx);

}

TEST(TestHdComment, Property) {
  HdComment comment;

  const std::string_view kTx = "Property Test";
  comment.Comment(MdString(kTx));
  EXPECT_EQ(comment.Comment().Text(), kTx);

  MdProperty property;

  constexpr std::string_view kName = "TntBalls";
  property.Name(std::string(kName));
  EXPECT_EQ(property.Name(), kName);

  constexpr std::string_view kDesc = "Exploding tennis balls";
  property.Description(std::string(kDesc));
  EXPECT_EQ(property.Description(), kDesc);

  constexpr std::string_view kUnit = "kilotons";
  property.Unit(std::string(kUnit));
  EXPECT_EQ(property.Unit(), kUnit);

  constexpr std::string_view kUnitRef = "energy";
  property.UnitReference(std::string(kUnitRef));
  EXPECT_EQ(property.UnitReference(), kUnitRef);

  constexpr MdDataType kDataType = MdDataType::MdInteger;
  property.DataType(kDataType);
  EXPECT_EQ(property.DataType(), kDataType);

  constexpr bool kReadOnly = true;
  property.ReadOnly(kReadOnly);
  EXPECT_EQ(property.ReadOnly(), kReadOnly);

  constexpr int64_t kValue = 123;
  property.Value(kValue);
  EXPECT_EQ(property.Value<int64_t>(), kValue);

  comment.AddProperty(property);

  constexpr std::string_view kStringProperty = "StringProperty";
  constexpr std::string_view kStringValue = "StringValue";
  {
    MdProperty string_property((std::string(kStringProperty)),
                               std::string(kStringValue));
    comment.AddProperty(string_property);
  }

  constexpr std::string_view kDecimalProperty = "DecimalProperty";
  constexpr float kDecimalValue = 1.23F;
  {
    MdProperty decimal_property(std::string(kDecimalProperty), "");
    decimal_property.DataType(MdDataType::MdDecimal);
    decimal_property.Value(kDecimalValue);
    comment.AddProperty(decimal_property);
  }

  constexpr std::string_view kIntegerProperty = "IntegerProperty";
  constexpr int kIntegerValue = -123;
  {
    MdProperty integer_property(std::string(kIntegerProperty), "");
    integer_property.DataType(MdDataType::MdInteger);
    integer_property.Value(kIntegerValue);
    comment.AddProperty(integer_property);
  }

  constexpr std::string_view kFloatProperty = "FloatProperty";
  constexpr double kFloatValue = -1.23;
  {
    MdProperty float_property(std::string(kFloatProperty), "");
    float_property.DataType(MdDataType::MdFloat);
    float_property.Value(kFloatValue);
    comment.AddProperty(float_property);
  }

  constexpr std::string_view kBooleanProperty = "BooleanProperty";
  constexpr bool kBooleanValue = true;
  {
    MdProperty boolean_property(std::string(kBooleanProperty), "");
    boolean_property.DataType(MdDataType::MdBoolean);
    boolean_property.Value(kBooleanValue);
    comment.AddProperty(boolean_property);
  }

  const uint64_t kDateValue =
      (MdfHelper::NowNs() / 1'000'000'000) * 1'000'000'000;
  constexpr std::string_view kDateProperty = "DateProperty";
  {
    MdProperty date_property(std::string(kDateProperty), "");
    date_property.DataType(MdDataType::MdDate);
    date_property.Value(kDateValue);
    comment.AddProperty(date_property);
  }

  constexpr std::string_view kTimeProperty = "TimeProperty";
  {
    MdProperty time_property(std::string(kTimeProperty), "");
    time_property.DataType(MdDataType::MdTime);
    time_property.Value(kDateValue);
    comment.AddProperty(time_property);
  }

  constexpr std::string_view kDateTimeProperty = "DateTimeProperty";
  {
    MdProperty date_time_property(std::string(kDateTimeProperty), "");
    date_time_property.DataType(MdDataType::MdDateTime);
    date_time_property.Value(kDateValue);
    comment.AddProperty(date_time_property);
  }

  constexpr std::string_view kHexProperty = "HexProperty";
  constexpr uint64_t kHexValue = 0x1234;
  {
    MdProperty hex_property(std::string(kHexProperty), "");
    hex_property.DataType(MdDataType::MdHex);
    hex_property.Value(kHexValue);
    comment.AddProperty(hex_property);
  }

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment().Text(), kTx);
  EXPECT_EQ(comment1.Properties().size(), 10);

  const MdProperty* property1 = comment1.GetProperty(std::string(kName));
  ASSERT_TRUE(property1 != nullptr);
  EXPECT_EQ(property1->Name(), kName);
  EXPECT_EQ(property1->Description(), kDesc);
  EXPECT_EQ(property1->Unit(), kUnit);
  EXPECT_EQ(property1->UnitReference(), kUnitRef);
  EXPECT_EQ(property1->DataType(), kDataType);
  EXPECT_EQ(property1->ReadOnly(), kReadOnly);
  EXPECT_EQ(property1->Value<int64_t>(), kValue);
  {
    const MdProperty* string_property1 =
        comment1.GetProperty(std::string(kStringProperty));
    ASSERT_TRUE(string_property1 != nullptr);
    EXPECT_EQ(string_property1->Value<std::string>(), kStringValue);
  }
  {
    const MdProperty* decimal_property1 =
        comment1.GetProperty(std::string(kDecimalProperty));
    ASSERT_TRUE(decimal_property1 != nullptr);
    EXPECT_FLOAT_EQ(decimal_property1->Value<float>(), kDecimalValue);
  }
  {
    const MdProperty* integer_property1 =
        comment1.GetProperty(std::string(kIntegerProperty));
    ASSERT_TRUE(integer_property1 != nullptr);
    EXPECT_EQ(integer_property1->Value<int64_t>(), kIntegerValue);
  }
  {
    const MdProperty* float_property1 =
        comment1.GetProperty(std::string(kFloatProperty));
    ASSERT_TRUE(float_property1 != nullptr);
    EXPECT_DOUBLE_EQ(float_property1->Value<double>(), kFloatValue);
  }
  {
    const MdProperty* boolean_property1 =
        comment1.GetProperty(std::string(kBooleanProperty));
    ASSERT_TRUE(boolean_property1 != nullptr);
    EXPECT_EQ(boolean_property1->Value<bool>(), kBooleanValue);
  }
  {
    const MdProperty* date_property1 =
        comment1.GetProperty(std::string(kDateProperty));
    ASSERT_TRUE(date_property1 != nullptr);
    EXPECT_FALSE(date_property1->Value<std::string>().empty());
  }
  {
    const MdProperty* time_property1 =
        comment1.GetProperty(std::string(kTimeProperty));
    ASSERT_TRUE(time_property1 != nullptr);
    EXPECT_FALSE(time_property1->Value<std::string>().empty());
  }
  {
    const MdProperty* date_time_property1 =
        comment1.GetProperty(std::string(kDateTimeProperty));
    ASSERT_TRUE(date_time_property1 != nullptr);
    EXPECT_EQ(date_time_property1->Value<uint64_t>(), kDateValue);
  }
  {
    const MdProperty* hex_property1 =
        comment1.GetProperty(std::string(kHexProperty));
    ASSERT_TRUE(hex_property1 != nullptr);
    EXPECT_EQ(hex_property1->Value<uint64_t>(), kHexValue);
  }
}

TEST(TestHdComment, Tree) {
  HdComment comment;

  constexpr std::string_view kTx = "Tree Test";
  comment.Comment(MdString(kTx));
  EXPECT_EQ(comment.Comment().Text(), kTx);

  MdList tree;
  const std::string_view kTreeName = "Engine Data";
  tree.Name(std::string(kTreeName));

  MdProperty property;
  constexpr std::string_view kNofCyl = "NofCyl";
  constexpr std::string_view kNofCylDesc = "Number of cylinders";
  constexpr int kNofCylValue = 4;
  property.Name(std::string(kNofCyl));
  property.Description(std::string(kNofCylDesc));
  property.DataType(MdDataType::MdInteger);
  property.Value(kNofCylValue);
  tree.AddProperty(property);

  comment.AddTree(tree);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment().Text(), kTx);
  EXPECT_EQ(comment1.Properties().size(), 0);
  EXPECT_EQ(comment1.Trees().size(), 1);

  const MdProperty* property1 = comment1.GetProperty(std::string(kNofCyl));
  ASSERT_TRUE(property1 != nullptr);
  EXPECT_EQ(property1->Name(), kNofCyl);
  EXPECT_EQ(property1->Description(), kNofCylDesc);
  EXPECT_EQ(property1->DataType(), MdDataType::MdInteger);
  EXPECT_EQ(property1->Value<int>(), kNofCylValue);

  std::ostringstream prop_name;
  prop_name << kTreeName << "/" << kNofCyl;
  const MdProperty* property2 = comment1.GetProperty(prop_name.str());
  EXPECT_EQ(property1, property2);

}

TEST(TestHdComment, List) {
  HdComment comment;

  constexpr std::string_view kTx = "List Test";
  comment.Comment(MdString(kTx));
  EXPECT_EQ(comment.Comment().Text(), kTx);

  MdList list;
  const std::string_view kListName = "Engine Data";
  list.Name(std::string(kListName));

  MdProperty property;
  constexpr std::string_view kNofCyl = "NofCyl";
  constexpr std::string_view kNofCylDesc = "Number of cylinders";
  constexpr int kNofCylValue = 4;
  property.Name(std::string(kNofCyl));
  property.Description(std::string(kNofCylDesc));
  property.DataType(MdDataType::MdInteger);
  property.Value(kNofCylValue);
  list.AddProperty(property);

  comment.AddList(list);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment().Text(), kTx);
  EXPECT_EQ(comment1.Properties().size(), 0);
  EXPECT_EQ(comment1.Lists().size(), 1);

  const MdProperty* property1 = comment1.GetProperty(std::string(kNofCyl));
  ASSERT_TRUE(property1 != nullptr);
  EXPECT_EQ(property1->Name(), kNofCyl);
  EXPECT_EQ(property1->Description(), kNofCylDesc);
  EXPECT_EQ(property1->DataType(), MdDataType::MdInteger);
  EXPECT_EQ(property1->Value<int>(), kNofCylValue);

  std::ostringstream prop_name;
  prop_name << kListName << "/" << kNofCyl;
  const MdProperty* property2 = comment1.GetProperty(prop_name.str());
  EXPECT_EQ(property1, property2);

}

TEST(TestHdComment, Enumerate) {
  HdComment comment;

  constexpr std::string_view kTx = "Enumerate Test";
  comment.Comment(MdString(kTx));
  EXPECT_EQ(comment.Comment().Text(), kTx);

  MdEnumerate enumerate;
  constexpr std::string_view kEnumerate = "Gear";
  constexpr std::array<std::string_view, 5> kEnumerateList = {
    "P", "R", "N", "D", "L"
  };
  enumerate.Name(std::string(kEnumerate));
  for (const auto& text : kEnumerateList) {
    enumerate.AddValue(MdString(text));
  }
  comment.AddEnumerate(enumerate);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  HdComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment().Text(), kTx);
  EXPECT_EQ(comment1.Properties().size(), 0);
  EXPECT_EQ(comment1.Enumerates().size(), 1);

  const MdEnumerate* enumerate1 = comment1.GetEnumerate(std::string(kEnumerate));
  ASSERT_TRUE(enumerate1 != nullptr);
  EXPECT_EQ(enumerate1->Name(), kEnumerate);
  const auto& enum_list = enumerate1->ValueList();
  ASSERT_EQ(kEnumerateList.size(), enum_list.size());
  for (size_t index = 0; index < kEnumerateList.size(); ++index) {
    EXPECT_EQ(kEnumerateList[index], enum_list[index]);
  }
}

} // end namespace