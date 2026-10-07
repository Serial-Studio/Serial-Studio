/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QTest>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameSupport.h"

/**
 * @brief Contract of the Pro-content rules the gate reads (spec 0094): only non-blank transform
 *        code and user tables count as Pro content (R1), and only writing non-blank code counts as
 *        authoring a transform, never clearing it (R7, R9).
 */
class TstProContent : public QObject {
  Q_OBJECT

private slots:
  void emptyProjectIsFree();
  void blankTransformCodeDoesNotCount();
  void nonBlankTransformCodeCounts();
  void userTablesCount();
  void countsAcrossGroups();
  void negativeTableCountClampsToZero();
  void clearingNeverAuthors();
  void unchangedTransformDoesNotAuthor();
  void newCodeAuthors();
  void languageOrParamsChangeAuthors();
};

/**
 * @brief Builds a group holding one dataset per entry of @p codes.
 */
[[nodiscard]] static DataModel::Group makeGroup(const QStringList& codes)
{
  DataModel::Group group;
  for (const auto& code : codes) {
    DataModel::Dataset dataset;
    dataset.transformCode = code;
    group.datasets.push_back(dataset);
  }

  return group;
}

/**
 * @brief A project with no groups and no tables needs no entitlement.
 */
void TstProContent::emptyProjectIsFree()
{
  const std::vector<DataModel::Group> groups;
  const auto summary = SerialStudio::proContentSummary(groups, 0);
  QCOMPARE(summary.transforms, 0);
  QCOMPARE(summary.tables, 0);
}

/**
 * @brief Empty and whitespace-only transform code is the absence of a transform.
 */
void TstProContent::blankTransformCodeDoesNotCount()
{
  const std::vector<DataModel::Group> groups = {
    makeGroup({QString(), QStringLiteral("   "), QStringLiteral("\n\t \r\n")})};
  const auto summary = SerialStudio::proContentSummary(groups, 0);
  QCOMPARE(summary.transforms, 0);
  QCOMPARE(summary.tables, 0);
}

/**
 * @brief Any non-blank transform code counts once per dataset, padding included.
 */
void TstProContent::nonBlankTransformCodeCounts()
{
  const std::vector<DataModel::Group> groups = {
    makeGroup({QStringLiteral("function transform(value) return value * 2 end"),
               QStringLiteral("  x  "),
               QString()})};
  const auto summary = SerialStudio::proContentSummary(groups, 0);
  QCOMPARE(summary.transforms, 2);
  QCOMPARE(summary.tables, 0);
}

/**
 * @brief The caller's user-table count passes through untouched by the datasets.
 */
void TstProContent::userTablesCount()
{
  const std::vector<DataModel::Group> groups = {makeGroup({QString()})};
  const auto summary                         = SerialStudio::proContentSummary(groups, 1);
  QCOMPARE(summary.transforms, 0);
  QCOMPARE(summary.tables, 1);
}

/**
 * @brief Transforms are counted over every group.
 */
void TstProContent::countsAcrossGroups()
{
  const std::vector<DataModel::Group> groups = {
    makeGroup({QStringLiteral("a"), QString()}),
    makeGroup({}),
    makeGroup({QStringLiteral("b"), QStringLiteral("c")})};
  const auto summary = SerialStudio::proContentSummary(groups, 3);
  QCOMPARE(summary.transforms, 3);
  QCOMPARE(summary.tables, 3);
}

/**
 * @brief A nonsensical negative count never reads as "no content plus something".
 */
void TstProContent::negativeTableCountClampsToZero()
{
  const std::vector<DataModel::Group> groups;
  const auto summary = SerialStudio::proContentSummary(groups, -4);
  QCOMPARE(summary.tables, 0);
}

/**
 * @brief Builds a dataset carrying @p code in @p language.
 */
[[nodiscard]] static DataModel::Dataset makeDataset(const QString& code, const int language)
{
  DataModel::Dataset dataset;
  dataset.transformCode     = code;
  dataset.transformLanguage = language;
  return dataset;
}

/**
 * @brief Removing a transform, or leaving only whitespace, is never authoring one.
 */
void TstProContent::clearingNeverAuthors()
{
  const auto before = makeDataset(QStringLiteral("function transform(v) return v end"), 1);
  QVERIFY(!SerialStudio::authorsTransform(before, makeDataset(QString(), -1)));
  QVERIFY(!SerialStudio::authorsTransform(before, makeDataset(QStringLiteral("  \n "), 1)));
}

/**
 * @brief Writing back the same code, language and parameters changes nothing.
 */
void TstProContent::unchangedTransformDoesNotAuthor()
{
  const auto before = makeDataset(QStringLiteral("function transform(v) return v end"), 1);
  QVERIFY(!SerialStudio::authorsTransform(before, before));
}

/**
 * @brief New or different code is authoring, whether or not a transform was there.
 */
void TstProContent::newCodeAuthors()
{
  const auto code = QStringLiteral("function transform(v) return v * 2 end");
  QVERIFY(SerialStudio::authorsTransform(makeDataset(QString(), -1), makeDataset(code, 1)));
  QVERIFY(
    SerialStudio::authorsTransform(makeDataset(QStringLiteral("x"), 1), makeDataset(code, 1)));
}

/**
 * @brief Changing an existing transform's language or parameters is editing it.
 */
void TstProContent::languageOrParamsChangeAuthors()
{
  const auto before          = makeDataset(QStringLiteral("function transform(v) return v end"), 1);
  auto language              = before;
  language.transformLanguage = 0;
  QVERIFY(SerialStudio::authorsTransform(before, language));

  auto params = before;
  params.transformParams.insert(QStringLiteral("gain"), 2.0);
  QVERIFY(SerialStudio::authorsTransform(before, params));
}

QTEST_GUILESS_MAIN(TstProContent)
#include "tst_pro_content.moc"
