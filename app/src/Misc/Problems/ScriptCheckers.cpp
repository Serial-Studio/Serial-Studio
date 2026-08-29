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

#include "Misc/Problems/ScriptCheckers.h"

#include <QCoreApplication>
#include <QString>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "Misc/ProblemCenter.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Finding  = Misc::ProblemCenter::Finding;
using Severity = Misc::ProblemCenter::Severity;

static const QString kScriptJumpDataset = QStringLiteral("dataset");

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated text for the shared "Problems" translation context.
 */
[[nodiscard]] static QString trScriptProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one finding; the checker id is stamped by the problem center after the run.
 */
[[nodiscard]] static Finding makeScriptFinding(Severity severity,
                                               const char* code,
                                               const QString& title,
                                               const QString& explanation,
                                               const QString& remedy,
                                               int entityUniqueId,
                                               const QString& jump)
{
  Finding finding;
  finding.severity       = severity;
  finding.entityUniqueId = entityUniqueId;
  finding.code           = QString::fromLatin1(code);
  finding.jump           = jump;
  finding.title          = title;
  finding.remedy         = remedy;
  finding.explanation    = explanation;
  return finding;
}

/**
 * @brief The project document the label helpers read, resolved once: the checkers are free
 *        functions with no constructor to capture in, so the singleton reach is funnelled through
 *        one accessor instead of a per-helper static.
 */
[[nodiscard]] static DataModel::ProjectModel& projectModel()
{
  static auto& model = DataModel::ProjectModel::instance();
  return model;
}

/**
 * @brief Maps a live error counter onto a decade bucket, so the finding text stays identical while
 *        the failure keeps repeating and the panel is not reset once per second.
 */
[[nodiscard]] static QString scriptBucketLabel(quint64 value)
{
  if (value >= 1000000)
    return trScriptProblem("more than a million times");

  if (value >= 100000)
    return trScriptProblem("more than 100,000 times");

  if (value >= 10000)
    return trScriptProblem("more than 10,000 times");

  if (value >= 1000)
    return trScriptProblem("more than 1,000 times");

  if (value >= 100)
    return trScriptProblem("more than 100 times");

  if (value >= 10)
    return trScriptProblem("more than 10 times");

  if (value > 1)
    return trScriptProblem("a few times");

  return trScriptProblem("once");
}

/**
 * @brief Returns a printable name for a source, falling back to its identity when untitled.
 */
[[nodiscard]] static QString scriptSourceLabel(int sourceId)
{
  const auto& sources = projectModel().sources();
  for (const auto& source : sources)
    if (source.sourceId == sourceId && !source.title.isEmpty())
      return source.title;

  return trScriptProblem("Source %1").arg(sourceId);
}

/**
 * @brief Returns a printable name for the dataset that owns a failing transform, falling back to
 *        its identity when the dataset is untitled or already gone.
 */
[[nodiscard]] static QString scriptDatasetLabel(int uniqueId)
{
  const auto& groups = projectModel().groups();
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets)
      if (dataset.uniqueId == uniqueId && !dataset.title.isEmpty())
        return dataset.title;
  }

  return trScriptProblem("Dataset %1").arg(uniqueId);
}

//--------------------------------------------------------------------------------------------------
// Parser checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a parser engine the watchdog switched off after repeated frame-budget overruns;
 *        the source then produces no data at all until the script is fixed and reloaded.
 */
static void reportDisabledEngine(const DataModel::ScriptStat& stat, QList<Finding>& out)
{
  out.append(makeScriptFinding(
    Misc::ProblemCenter::Error,
    "parser-disabled",
    trScriptProblem("Frame parser was switched off"),
    trScriptProblem("The frame parser of \"%1\" exceeded its time budget on too "
                    "many consecutive frames and was switched off. Last error: %2")
      .arg(scriptSourceLabel(stat.sourceId), stat.lastError),
    trScriptProblem("Simplify the parser so it returns within the frame budget, "
                    "then reload the script to re-enable it."),
    -1,
    QString()));
}

/**
 * @brief Reports a parser that keeps throwing, carrying the retained message and a bucketed count.
 */
static void reportParserErrors(const DataModel::ScriptStat& stat, QList<Finding>& out)
{
  out.append(makeScriptFinding(
    Misc::ProblemCenter::Warning,
    "parser-errors",
    trScriptProblem("Frame parser is failing"),
    trScriptProblem("The frame parser of \"%1\" has failed %2. Last error: %3")
      .arg(scriptSourceLabel(stat.sourceId), scriptBucketLabel(stat.errorCount), stat.lastError),
    trScriptProblem("Open the frame parser and fix the reported error; frames that "
                    "fail to parse produce no dataset values."),
    -1,
    QString()));
}

/**
 * @brief Walks the per-source parser engines and reports the ones that are disabled or failing.
 */
static void checkParserErrors(QList<Finding>& out)
{
  static auto& parser = DataModel::FrameParser::instance();

  const auto stats = parser.scriptStats();
  for (const auto& stat : stats) {
    if (stat.disabled) {
      reportDisabledEngine(stat, out);
      continue;
    }

    if (stat.errorCount > 0)
      reportParserErrors(stat, out);
  }
}

//--------------------------------------------------------------------------------------------------
// Transform checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports per-dataset value transforms that keep throwing; the dataset silently falls back
 *        to its raw value, which looks like a wrong reading rather than a broken script.
 */
static void checkTransformErrors(QList<Finding>& out)
{
  static auto& builder = DataModel::FrameBuilder::instance();

  const quint64 fails = builder.transformErrorCount();
  if (fails == 0)
    return;

  int uniqueId = -1;
  QString lastError;
  builder.invokeOnBuilderThreadBlocking([&] {
    uniqueId  = builder.lastTransformDataset();
    lastError = builder.lastTransformError();
  });

  out.append(
    makeScriptFinding(Misc::ProblemCenter::Warning,
                      "transform-errors",
                      trScriptProblem("A value transform is failing"),
                      trScriptProblem("The transform of \"%1\" has failed %2, so the dataset shows "
                                      "its untransformed value. Last error: %3")
                        .arg(scriptDatasetLabel(uniqueId), scriptBucketLabel(fails), lastError),
                      trScriptProblem("Open the dataset's transform and fix the reported error."),
                      uniqueId,
                      kScriptJumpDataset));
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the script checkers, which poll the engine counters on the shared 1 Hz sample
 *        tick and on an explicit re-run request.
 */
void Misc::ScriptCheckers::registerAll()
{
  static auto& center   = Misc::ProblemCenter::instance();
  const quint8 triggers = Misc::ProblemCenter::LinkSample | Misc::ProblemCenter::OnDemand;

  center.registerChecker(QStringLiteral("script.parser"), triggers, checkParserErrors);
  center.registerChecker(QStringLiteral("script.transform"), triggers, checkTransformErrors);
}
