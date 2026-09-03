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

#include "DataModel/ExportStructure.h"

using DataModel::ExportStructure;

/**
 * @brief The schema half the CSV, MDF4 and Sessions export workers share (spec 0075 B11): the two
 *        ways a template frame is adopted, and the title scrub every export path names its folder
 *        with. Three copies of both drifted apart; this pins the one that replaced them.
 */
class TstExportStructure : public QObject {
  Q_OBJECT

private slots:
  void anEmptyFrameNeverWipesAnAdoptedTemplate();
  void aPublishedStructureOnlyFillsAnEmptySlot();
  void anOpenFileKeepsItsSchema();
  void aTitleCannotEscapeTheWorkspace();
  void aTitleThatScrubsAwayFallsBack();
};

/**
 * @brief Builds a frame with @p groupCount groups, which is what hasStructure() reads.
 */
[[nodiscard]] static DataModel::Frame frameWithGroups(int groupCount, const QString& title)
{
  DataModel::Frame frame;
  frame.title = title;
  for (int i = 0; i < groupCount; ++i)
    frame.groups.push_back(DataModel::Group());

  return frame;
}

/**
 * @brief Structure arrives asynchronously and QuickPlot has none until its first frame, so an
 *        empty payload landing second must be ignored: adopting it would wipe the schema and the
 *        recording would never create a file.
 */
void TstExportStructure::anEmptyFrameNeverWipesAnAdoptedTemplate()
{
  ExportStructure structure;
  QVERIFY(!structure.hasStructure());

  structure.setTemplateFrame(frameWithGroups(2, QStringLiteral("Real")));
  QVERIFY(structure.hasStructure());
  QCOMPARE(structure.templateFrame().title, QStringLiteral("Real"));

  structure.setTemplateFrame(DataModel::Frame());
  QVERIFY(structure.hasStructure());
  QCOMPARE(structure.templateFrame().title, QStringLiteral("Real"));

  structure.clear();
  QVERIFY(!structure.hasStructure());
}

/**
 * @brief The pipeline's published structure is the QuickPlot fallback for a connect-time fetch
 *        that came back empty; it must never overwrite a schema the fetch already supplied.
 */
void TstExportStructure::aPublishedStructureOnlyFillsAnEmptySlot()
{
  ExportStructure structure;
  structure.applyPublishedStructure(frameWithGroups(1, QStringLiteral("Published")), false);
  QVERIFY(structure.hasStructure());
  QCOMPARE(structure.templateFrame().title, QStringLiteral("Published"));

  structure.applyPublishedStructure(frameWithGroups(1, QStringLiteral("Later")), false);
  QCOMPARE(structure.templateFrame().title, QStringLiteral("Published"));
}

/**
 * @brief An open file's columns are fixed for its lifetime, so a structure landing after the file
 *        exists is dropped rather than rewriting the schema its rows are already keyed on.
 */
void TstExportStructure::anOpenFileKeepsItsSchema()
{
  ExportStructure structure;
  structure.applyPublishedStructure(frameWithGroups(1, QStringLiteral("Late")), true);
  QVERIFY(!structure.hasStructure());
}

/**
 * @brief Every export lane names a folder from the project title, so a title carrying separators
 *        or parent-directory hops must not be able to write outside the workspace.
 */
void TstExportStructure::aTitleCannotEscapeTheWorkspace()
{
  const QString fallback = QStringLiteral("Untitled");

  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("../../etc/passwd"), fallback),
           QStringLiteral("etcpasswd"));
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("a/b\\c:d*e?f\"g<h>i|j"), fallback),
           QStringLiteral("abcdefghij"));
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("Trailing dots..."), fallback),
           QStringLiteral("Trailing dots"));
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("  spaced   out  "), fallback),
           QStringLiteral("spaced out"));
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("Normal Title"), fallback),
           QStringLiteral("Normal Title"));
}

/**
 * @brief A title made entirely of forbidden characters must still name a folder; each lane picks
 *        its own fallback and the sanitizer never returns an empty component.
 */
void TstExportStructure::aTitleThatScrubsAwayFallsBack()
{
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral("///"), QStringLiteral("Untitled")),
           QStringLiteral("Untitled"));
  QCOMPARE(ExportStructure::sanitizeTitle(QString(), QStringLiteral("SerialStudio")),
           QStringLiteral("SerialStudio"));
  QCOMPARE(ExportStructure::sanitizeTitle(QStringLiteral(".. .. .."), QStringLiteral("Untitled")),
           QStringLiteral("Untitled"));
}

QTEST_GUILESS_MAIN(TstExportStructure)

#include "tst_export_structure.moc"
