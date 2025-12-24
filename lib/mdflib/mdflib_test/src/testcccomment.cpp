/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>

#include <gtest/gtest.h>

#include "mdf/cccomment.h"
#include "mdf/ccunit.h"

namespace mdf::test {

TEST(TestCcComment, Properties) {
  CcComment comment;

  constexpr std::string_view kTx = "CC Test Properties";
  comment.Comment(MdString(kTx));

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  CcComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);
}

TEST(TestCcComment, CompuMethod) {
  CcComment comment;

  constexpr std::string_view kTx = "CC Test CompuMethod";
  comment.Comment(MdString(kTx));

  auto& compu_method = comment.CompuMethod();
  EXPECT_FALSE(compu_method.IsActive());

  constexpr std::string_view kShortName = "Coyote";
  compu_method.ShortName(std::string(kShortName));
  // The rest tested in HD comment test

  constexpr HoComputationMethodCategory kCategory =
      HoComputationMethodCategory::Formula;
  compu_method.Category(kCategory);

  constexpr std::string_view kUnitRef = "Animal";
  compu_method.UnitReference(std::string(kUnitRef));

  HoScaleConstraint phys_constraint;
  EXPECT_FALSE(phys_constraint.IsActive());

  HoInterval lower_limit;
  EXPECT_FALSE(lower_limit.IsActive());
  constexpr double kLowerLimit = -1.23;
  lower_limit.Limit(kLowerLimit);
  constexpr HoIntervalType kLowerType = HoIntervalType::Open;
  lower_limit.Type(kLowerType);
  EXPECT_TRUE(lower_limit.IsActive());
  phys_constraint.LowerLimit(lower_limit);

  HoInterval upper_limit;
  EXPECT_FALSE(upper_limit.IsActive());
  constexpr double kUpperLimit = 1.23;
  upper_limit.Limit(kUpperLimit);
  constexpr HoIntervalType kUpperType = HoIntervalType::Infinite;
  upper_limit.Type(kUpperType);
  EXPECT_TRUE(upper_limit.IsActive());
  phys_constraint.UpperLimit(upper_limit);

  constexpr HoValidity kValidity = HoValidity::NotValid;
  phys_constraint.Validity(kValidity);

  EXPECT_TRUE(phys_constraint.IsActive());
  compu_method.AddPhysicalConstraint(phys_constraint);
  compu_method.AddInternalConstraint(phys_constraint);

  HoCompuScale scale;
  constexpr std::string_view kDesc = "Coyote always lose";
  scale.AddDescription(MdString(kDesc));

  constexpr double kFloatValue = 12.3;
  scale.ConstFloatValue(kFloatValue);

  constexpr std::string_view kTextValue = "ACME";
  scale.ConstTextValue(std::string(kTextValue));

  constexpr double kNumerator = 11.3;
  scale.AddNumerator(kNumerator);

  constexpr double kDenominator = 66.3;
  scale.AddDenominator(kDenominator);

  constexpr std::string_view kMath = "Math Genius";
  scale.GenericMath(std::string(kMath));

  compu_method.AddCompuScale(scale);
  EXPECT_EQ(compu_method.CompuScales().size(), 1);

  compu_method.DefaultFloatValue(kFloatValue);
  compu_method.DefaultTextValue(std::string(kTextValue));

  EXPECT_TRUE(compu_method.IsActive());
  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  CcComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  const auto& compu_method1 = comment1.CompuMethod();
  EXPECT_TRUE(compu_method1.IsActive());
  EXPECT_EQ(compu_method1.Category(), kCategory);
  EXPECT_EQ(compu_method1.UnitReference(), kUnitRef);

  ASSERT_EQ(compu_method1.PhysicalConstraints().size(), 1);
  const HoScaleConstraint& phys_constraint1 =
      compu_method1.PhysicalConstraints()[0];
  EXPECT_TRUE(phys_constraint1.IsActive());
  EXPECT_EQ(phys_constraint1.LowerLimit().Limit(), kLowerLimit);
  EXPECT_EQ(phys_constraint1.LowerLimit().Type(), kLowerType);
  EXPECT_EQ(phys_constraint1.Validity(), kValidity);

  ASSERT_EQ(compu_method1.InternalConstraints().size(), 1);

  ASSERT_EQ(compu_method1.CompuScales().size(), 1);
  const HoCompuScale& scale1 =
      compu_method1.CompuScales()[0];
  ASSERT_EQ(scale1.Descriptions().size(), 1);
  EXPECT_EQ(scale1.Descriptions()[0], kDesc);

  EXPECT_EQ(scale1.ConstFloatValue(), kFloatValue);
  EXPECT_EQ(scale1.ConstTextValue(), kTextValue);

  ASSERT_EQ(scale1.Numerators().size(), 1);
  EXPECT_EQ(scale1.Numerators()[0], kNumerator);

  ASSERT_EQ(scale1.Denominators().size(), 1);
  EXPECT_EQ(scale1.Denominators()[0], kDenominator);

  EXPECT_EQ(scale1.GenericMath(), kMath);

  EXPECT_EQ(compu_method1.DefaultFloatValue(), kFloatValue);
  EXPECT_EQ(compu_method1.DefaultTextValue(), kTextValue);
}

TEST(TestCcComment, CcUnit) {
  CcUnit unit;

  constexpr std::string_view kTx = "Test CC Unit Test";
  unit.Comment(MdString(kTx));

  constexpr std::string_view kUnitRef = "km";
  unit.UnitRef(MdString(kUnitRef));

  const std::string xml_snippet = unit.ToXml();
  std::cout << xml_snippet << std::endl;

  CcUnit unit1;
  unit1.FromXml(xml_snippet);
  EXPECT_EQ(unit1.Comment(), kTx);
  EXPECT_EQ(unit1.UnitRef(), kUnitRef);

}
} // end namespace mdf::test