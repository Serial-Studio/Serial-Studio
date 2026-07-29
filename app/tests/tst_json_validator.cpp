/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>

#include "Misc/JsonValidator.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a JSON object nested exactly @a depth levels deep, innermost value a number.
 */
static QByteArray nestedObjectJson(int depth)
{
  QByteArray open;
  QByteArray close;

  for (int i = 0; i < depth; ++i) {
    open  += "{\"a\":";
    close += "}";
  }

  return open + "1" + close;
}

/**
 * @brief Builds a JSON array nested exactly @a depth levels deep, innermost value a number.
 */
static QByteArray nestedArrayJson(int depth)
{
  QByteArray open;
  QByteArray close;

  for (int i = 0; i < depth; ++i) {
    open  += "[";
    close += "]";
  }

  return open + "1" + close;
}

/**
 * @brief Builds a top-level JSON array literal with @a count integer elements.
 */
static QByteArray arrayOfSize(int count)
{
  QByteArray json = "[";

  for (int i = 0; i < count; ++i) {
    if (i > 0)
      json += ",";
    json += "0";
  }

  json += "]";
  return json;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

/**
 * @brief Byte-level contract of Misc::JsonValidator's security bounds: file size, nesting depth
 *        and array size limits, plus the parse-error and default-limits plumbing around them.
 */
class TstJsonValidator : public QObject {
  Q_OBJECT

private slots:
  void validSmallObject();
  void validTopLevelArray();
  void emptyDataIsInvalid();
  void malformedJsonReportsOffset();

  void fileSizeExactLimitPasses();
  void fileSizeOverLimitFails();

  void depthExactLimitPasses();
  void depthOverLimitObjectFails();
  void depthOverLimitArrayFails();

  void arraySizeExactLimitPasses();
  void arraySizeOverLimitFails();

  void customDepthLimitRejectsDeeper();
  void customDepthLimitAcceptsExact();
  void customArraySizeLimitRejectsLarger();

  void mixedArrayOfObjectsOneOverDepthFails();

  void defaultOverloadMatchesExplicitDefaultLimits();
  void defaultLimitsCarryDocumentedValues();
};

//--------------------------------------------------------------------------------------------------
// Basic acceptance and rejection
//--------------------------------------------------------------------------------------------------

/**
 * @brief A small well-formed object parses successfully and the document round-trips.
 */
void TstJsonValidator::validSmallObject()
{
  const QByteArray data = "{\"name\":\"probe\",\"value\":42}";
  const auto result     = Misc::JsonValidator::parseAndValidate(data);

  QVERIFY(result.valid);
  QVERIFY(result.errorMessage.isEmpty());
  QVERIFY(result.document.isObject());
  QCOMPARE(result.document.object().value("name").toString(), QStringLiteral("probe"));
  QCOMPARE(result.document.object().value("value").toInt(), 42);
}

/**
 * @brief A top-level array is a valid document shape, not just objects.
 */
void TstJsonValidator::validTopLevelArray()
{
  const QByteArray data = "[1,2,3]";
  const auto result     = Misc::JsonValidator::parseAndValidate(data);

  QVERIFY(result.valid);
  QVERIFY(result.document.isArray());
  QCOMPARE(result.document.array().size(), 3);
}

/**
 * @brief An empty byte array is rejected before parsing, with a message naming the reason.
 */
void TstJsonValidator::emptyDataIsInvalid()
{
  const auto result = Misc::JsonValidator::parseAndValidate(QByteArray());

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("empty", Qt::CaseInsensitive));
}

/**
 * @brief Malformed JSON fails at the parse stage and the error names the byte offset.
 */
void TstJsonValidator::malformedJsonReportsOffset()
{
  const QByteArray data = "{\"a\":}";
  const auto result     = Misc::JsonValidator::parseAndValidate(data);

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("offset", Qt::CaseInsensitive));
}

//--------------------------------------------------------------------------------------------------
// File size limit (boundary is a strict '>')
//--------------------------------------------------------------------------------------------------

/**
 * @brief Data whose size exactly equals maxFileSize passes the size gate.
 */
void TstJsonValidator::fileSizeExactLimitPasses()
{
  const QByteArray data = "{\"probe\":12345678}";

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize = data.size();

  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(result.valid);
}

/**
 * @brief Data one byte over maxFileSize fails, naming the MB-scaled limit in the message.
 */
void TstJsonValidator::fileSizeOverLimitFails()
{
  const QByteArray data = "{\"probe\":12345678}";

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize = data.size() - 1;

  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("MB"));
}

//--------------------------------------------------------------------------------------------------
// Depth limit (boundary is a strict '>')
//--------------------------------------------------------------------------------------------------

/**
 * @brief Nesting exactly maxDepth levels deep passes structural validation.
 */
void TstJsonValidator::depthExactLimitPasses()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 5;

  const auto data   = nestedObjectJson(limits.maxDepth);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(result.valid);
}

/**
 * @brief Nesting maxDepth + 1 object levels deep fails structural validation.
 */
void TstJsonValidator::depthOverLimitObjectFails()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 5;

  const auto data   = nestedObjectJson(limits.maxDepth + 1);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("depth"));
}

/**
 * @brief Nesting maxDepth + 1 array levels deep fails structural validation identically to
 *        the object case.
 */
void TstJsonValidator::depthOverLimitArrayFails()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 5;

  const auto data   = nestedArrayJson(limits.maxDepth + 1);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("depth"));
}

//--------------------------------------------------------------------------------------------------
// Array size limit (boundary is a strict '>')
//--------------------------------------------------------------------------------------------------

/**
 * @brief An array with exactly maxArraySize elements passes structural validation.
 */
void TstJsonValidator::arraySizeExactLimitPasses()
{
  Misc::JsonValidator::Limits limits;
  limits.maxArraySize = 5;

  const auto data   = arrayOfSize(limits.maxArraySize);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(result.valid);
}

/**
 * @brief An array with maxArraySize + 1 elements fails structural validation.
 */
void TstJsonValidator::arraySizeOverLimitFails()
{
  Misc::JsonValidator::Limits limits;
  limits.maxArraySize = 5;

  const auto data   = arrayOfSize(limits.maxArraySize + 1);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
  QVERIFY(result.errorMessage.contains("array size"));
}

//--------------------------------------------------------------------------------------------------
// Custom limits applied directly through validateStructure()
//--------------------------------------------------------------------------------------------------

/**
 * @brief A custom maxDepth of 2 rejects a 3-level nested object.
 */
void TstJsonValidator::customDepthLimitRejectsDeeper()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 2;

  const auto data   = nestedObjectJson(3);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
}

/**
 * @brief The same custom maxDepth of 2 accepts a 2-level nested object.
 */
void TstJsonValidator::customDepthLimitAcceptsExact()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 2;

  const auto data   = nestedObjectJson(2);
  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(result.valid);
}

/**
 * @brief A custom maxArraySize of 1 rejects a two-element array.
 */
void TstJsonValidator::customArraySizeLimitRejectsLarger()
{
  Misc::JsonValidator::Limits limits;
  limits.maxArraySize = 1;

  const QByteArray data = "[1,2]";
  const auto result     = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
}

//--------------------------------------------------------------------------------------------------
// Mixed structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief An array of near-limit-depth objects fails overall as soon as one element exceeds the
 *        depth limit, even though the sibling elements stay within bounds.
 */
void TstJsonValidator::mixedArrayOfObjectsOneOverDepthFails()
{
  Misc::JsonValidator::Limits limits;
  limits.maxDepth = 4;

  const auto within     = nestedObjectJson(limits.maxDepth - 1);
  const auto over       = nestedObjectJson(limits.maxDepth + 1);
  const QByteArray data = "[" + within + "," + over + "]";

  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);

  QVERIFY(!result.valid);
}

//--------------------------------------------------------------------------------------------------
// Default-limits plumbing
//--------------------------------------------------------------------------------------------------

/**
 * @brief The single-argument overload is exactly parseAndValidate(data, Limits{}).
 */
void TstJsonValidator::defaultOverloadMatchesExplicitDefaultLimits()
{
  const QByteArray data = "{\"probe\":true}";

  const auto viaDefault = Misc::JsonValidator::parseAndValidate(data);
  const auto viaExplicit =
    Misc::JsonValidator::parseAndValidate(data, Misc::JsonValidator::Limits());

  QCOMPARE(viaDefault.valid, viaExplicit.valid);
  QCOMPARE(viaDefault.errorMessage, viaExplicit.errorMessage);
  QCOMPARE(viaDefault.document, viaExplicit.document);
}

/**
 * @brief The documented security defaults are 10 MB, depth 128 and array size 10000.
 */
void TstJsonValidator::defaultLimitsCarryDocumentedValues()
{
  const Misc::JsonValidator::Limits limits;

  QCOMPARE(limits.maxFileSize, qsizetype(10 * 1024 * 1024));
  QCOMPARE(limits.maxDepth, 128);
  QCOMPARE(limits.maxArraySize, 10000);
}

QTEST_APPLESS_MAIN(TstJsonValidator)

#include "tst_json_validator.moc"
