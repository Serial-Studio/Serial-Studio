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

#include "Misc/Problems/ProjectCheckers.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QString>

#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Misc/ProblemCenter.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Finding  = Misc::ProblemCenter::Finding;
using Severity = Misc::ProblemCenter::Severity;

static constexpr int kMaxFindings = 50;

static const QString kJumpGroup   = QStringLiteral("group");
static const QString kJumpAction  = QStringLiteral("action");
static const QString kJumpDataset = QStringLiteral("dataset");

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated text for the shared "Problems" translation context.
 */
[[nodiscard]] static QString trProblem(const char* text)
{
  return QCoreApplication::translate("Problems", text);
}

/**
 * @brief Assembles one finding; the checker id is stamped by the problem center after the run.
 */
[[nodiscard]] static Finding makeFinding(Severity severity,
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
 * @brief Truncates an oversized slice and appends the trailing "and N more" summary, so one broken
 *        project cannot flood the panel with a finding per dataset.
 */
static void capFindings(QList<Finding>& out)
{
  if (out.size() <= kMaxFindings)
    return;

  const int hidden = out.size() - (kMaxFindings - 1);
  out.resize(kMaxFindings - 1);
  out.append(makeFinding(Misc::ProblemCenter::Info,
                         "and-more",
                         trProblem("More problems of this kind"),
                         trProblem("%1 further problems of this kind were found and are not "
                                   "listed individually.")
                           .arg(hidden),
                         trProblem("Fix the listed problems and run the checks again."),
                         -1,
                         QString()));
}

/**
 * @brief Reports whether a project document exists at all; QuickPlot and Console-only builds
 *        carry none, and an empty project is not a broken project.
 */
[[nodiscard]] static bool projectAvailable()
{
  static auto& state   = AppState::instance();
  static auto& project = DataModel::ProjectModel::instance();

  if (state.operationMode() != SerialStudio::ProjectFile)
    return false;

  return project.groupCount() > 0;
}

/**
 * @brief Returns a printable name for a dataset, falling back to its identity when untitled.
 */
[[nodiscard]] static QString datasetLabel(const DataModel::Dataset& dataset)
{
  if (!dataset.title.isEmpty())
    return dataset.title;

  return trProblem("Dataset %1").arg(dataset.uniqueId);
}

/**
 * @brief Returns a printable name for a group, falling back to its identity when untitled.
 */
[[nodiscard]] static QString groupLabel(const DataModel::Group& group)
{
  if (!group.title.isEmpty())
    return group.title;

  return trProblem("Group %1").arg(group.uniqueId);
}

/**
 * @brief Reports whether a dataset participates in frame building at all; disabled and virtual
 *        datasets never read a frame position, so index rules do not apply to them.
 */
[[nodiscard]] static bool datasetReadsFrame(const DataModel::Group& group,
                                            const DataModel::Dataset& dataset)
{
  return group.enabled && dataset.enabled && !dataset.virtual_;
}

//--------------------------------------------------------------------------------------------------
// Frame index checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flags a dataset whose frame position can never be filled, or that collides with an
 *        earlier dataset in the same source; indices are only unique per source, so the key
 *        carries the source id.
 */
static void inspectDatasetIndex(const DataModel::Group& group,
                                const DataModel::Dataset& dataset,
                                QHash<quint64, QString>& seen,
                                QList<Finding>& out)
{
  if (!datasetReadsFrame(group, dataset))
    return;

  if (dataset.index <= 0) {
    out.append(makeFinding(Misc::ProblemCenter::Error,
                           "unreachable-frame-index",
                           trProblem("Dataset reads no frame position"),
                           trProblem("\"%1\" has frame index %2, but frame positions start at 1, "
                                     "so no incoming frame can ever supply a value for it.")
                             .arg(datasetLabel(dataset))
                             .arg(dataset.index),
                           trProblem("Set the dataset's index to the position of its value in the "
                                     "frame, or mark the dataset as virtual."),
                           dataset.uniqueId,
                           kJumpDataset));
    return;
  }

  const auto source = static_cast<quint64>(static_cast<quint32>(group.sourceId));
  const auto index  = static_cast<quint64>(static_cast<quint32>(dataset.index));
  const quint64 key = (source << 32) | index;

  const auto it = seen.constFind(key);
  if (it == seen.constEnd()) {
    seen.insert(key, datasetLabel(dataset));
    return;
  }

  out.append(makeFinding(Misc::ProblemCenter::Warning,
                         "duplicate-frame-index",
                         trProblem("Two datasets share a frame index"),
                         trProblem("\"%1\" and \"%2\" both read frame index %3 of the same source, "
                                   "so they will always show the same value.")
                           .arg(it.value(), datasetLabel(dataset))
                           .arg(dataset.index),
                         trProblem("Give one of them the frame index of its own value, or delete "
                                   "the duplicate if the repetition is intentional."),
                         dataset.uniqueId,
                         kJumpDataset));
}

/**
 * @brief Checks every dataset's frame index for reachability and per-source collisions.
 */
static void checkFrameIndices(QList<Finding>& out)
{
  if (!projectAvailable())
    return;

  static auto& project = DataModel::ProjectModel::instance();

  QHash<quint64, QString> seen;
  const auto& groups = project.groups();
  for (const auto& group : groups)
    for (const auto& dataset : group.datasets)
      inspectDatasetIndex(group, dataset, seen, out);

  capFindings(out);
}

//--------------------------------------------------------------------------------------------------
// Empty group check
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flags groups that render nothing: no datasets, no output widgets, and a widget type that
 *        does not draw from its own code (painter and web view do).
 */
static void checkEmptyGroups(QList<Finding>& out)
{
  if (!projectAvailable())
    return;

  static auto& project = DataModel::ProjectModel::instance();

  const auto& groups = project.groups();
  for (const auto& group : groups) {
    const bool selfDrawing =
      group.widget == QStringLiteral("painter") || group.widget == QStringLiteral("webview");
    if (selfDrawing || !group.datasets.empty() || !group.outputWidgets.empty())
      continue;

    out.append(makeFinding(Misc::ProblemCenter::Warning,
                           "empty-group",
                           trProblem("Group contains nothing to show"),
                           trProblem("\"%1\" has no datasets and no output widgets, so it renders "
                                     "as an empty panel on the dashboard.")
                             .arg(groupLabel(group)),
                           trProblem("Add a dataset or an output widget to the group, or delete "
                                     "the group."),
                           group.uniqueId,
                           kJumpGroup));
  }

  capFindings(out);
}

//--------------------------------------------------------------------------------------------------
// Dangling reference checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flags a dataset whose X-axis or waterfall Y-axis points at a dataset that no longer
 *        exists; the widget then plots against nothing and comes up empty.
 */
static void inspectDatasetReferences(const DataModel::Dataset& dataset,
                                     const QSet<int>& datasetIds,
                                     QList<Finding>& out)
{
  if (dataset.xAxisId > 0 && !datasetIds.contains(dataset.xAxisId))
    out.append(makeFinding(Misc::ProblemCenter::Error,
                           "dangling-x-axis",
                           trProblem("Plot X-axis points at a deleted dataset"),
                           trProblem("\"%1\" plots against dataset %2, which is not part of this "
                                     "project any more.")
                             .arg(datasetLabel(dataset))
                             .arg(dataset.xAxisId),
                           trProblem("Pick an existing dataset as the X-axis, or switch the axis "
                                     "back to Time or Samples."),
                           dataset.uniqueId,
                           kJumpDataset));

  if (dataset.waterfallYAxis > 0 && !datasetIds.contains(dataset.waterfallYAxis))
    out.append(makeFinding(Misc::ProblemCenter::Error,
                           "dangling-waterfall-axis",
                           trProblem("Waterfall Y-axis points at a deleted dataset"),
                           trProblem("\"%1\" reads its waterfall Y-axis from dataset %2, which is "
                                     "not part of this project any more.")
                             .arg(datasetLabel(dataset))
                             .arg(dataset.waterfallYAxis),
                           trProblem("Pick an existing dataset as the Y-axis, or switch it back "
                                     "to Time."),
                           dataset.uniqueId,
                           kJumpDataset));
}

/**
 * @brief Flags workspace tiles that reference a group which has since been deleted, so the
 *        workspace opens with a missing widget. The tile itself is not an editor entity, so the
 *        finding carries no jump target.
 */
static void inspectWorkspaceReferences(const DataModel::Workspace& workspace,
                                       const QSet<int>& groupIds,
                                       QList<Finding>& out)
{
  for (const auto& ref : workspace.widgetRefs) {
    if (ref.groupUniqueId < 0 || groupIds.contains(ref.groupUniqueId))
      continue;

    out.append(makeFinding(Misc::ProblemCenter::Warning,
                           "dangling-workspace-widget",
                           trProblem("Workspace shows a deleted widget"),
                           trProblem("Workspace \"%1\" places a widget from group %2, which is not "
                                     "part of this project any more.")
                             .arg(workspace.title)
                             .arg(ref.groupUniqueId),
                           trProblem("Open the workspace editor and remove the missing tile."),
                           -1,
                           QString()));
  }
}

/**
 * @brief Flags actions and output widgets that transmit to a source the project no longer
 *        defines; the command is then sent nowhere. A project that declares no sources at all
 *        still resolves everything to the implicit default, so it is left alone.
 */
static void inspectSourceReferences(const QSet<int>& sourceIds, QList<Finding>& out)
{
  if (sourceIds.isEmpty())
    return;

  static auto& project = DataModel::ProjectModel::instance();
  for (const auto& action : project.actions()) {
    if (sourceIds.contains(action.sourceId))
      continue;

    out.append(makeFinding(Misc::ProblemCenter::Error,
                           "dangling-action-source",
                           trProblem("Action targets a missing source"),
                           trProblem("Action \"%1\" transmits to source %2, which is not defined "
                                     "in this project.")
                             .arg(action.title)
                             .arg(action.sourceId),
                           trProblem("Point the action at one of the project's sources."),
                           action.actionId,
                           kJumpAction));
  }

  for (const auto& group : project.groups()) {
    for (const auto& widget : group.outputWidgets) {
      if (sourceIds.contains(widget.sourceId))
        continue;

      out.append(makeFinding(Misc::ProblemCenter::Error,
                             "dangling-output-source",
                             trProblem("Output widget targets a missing source"),
                             trProblem("\"%1\" in group \"%2\" transmits to source %3, which is "
                                       "not defined in this project.")
                               .arg(widget.title, groupLabel(group))
                               .arg(widget.sourceId),
                             trProblem("Point the output widget at one of the project's sources."),
                             group.uniqueId,
                             kJumpGroup));
    }
  }
}

/**
 * @brief Runs every reference check against the identities the project currently defines.
 */
static void checkDanglingReferences(QList<Finding>& out)
{
  if (!projectAvailable())
    return;

  static auto& project = DataModel::ProjectModel::instance();

  QSet<int> groupIds;
  QSet<int> datasetIds;
  for (const auto& group : project.groups()) {
    groupIds.insert(group.uniqueId);
    for (const auto& dataset : group.datasets)
      datasetIds.insert(dataset.uniqueId);
  }

  QSet<int> sourceIds;
  for (const auto& source : project.sources())
    sourceIds.insert(source.sourceId);

  for (const auto& group : project.groups())
    for (const auto& dataset : group.datasets)
      inspectDatasetReferences(dataset, datasetIds, out);

  for (const auto& workspace : project.editorWorkspaces())
    inspectWorkspaceReferences(workspace, groupIds, out);

  inspectSourceReferences(sourceIds, out);
  capFindings(out);
}

//--------------------------------------------------------------------------------------------------
// Numeric range checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies a min/max pair; a pair left at 0/0 is the "unset, scale automatically"
 *        default and is never a problem.
 */
[[nodiscard]] static bool rangeIsBroken(double min, double max)
{
  if (min == 0.0 && max == 0.0)
    return false;

  return min >= max;
}

/**
 * @brief Flags a plot whose configured Y range cannot be drawn.
 */
static void checkPlotRange(const DataModel::Dataset& dataset, QList<Finding>& out)
{
  if (!dataset.plt || !rangeIsBroken(dataset.pltMin, dataset.pltMax))
    return;

  out.append(makeFinding(Misc::ProblemCenter::Warning,
                         "inverted-plot-range",
                         trProblem("Plot range is inverted or empty"),
                         trProblem("\"%1\" has a plot minimum of %2 and a maximum of %3, so the "
                                   "curve has no room to be drawn.")
                           .arg(datasetLabel(dataset))
                           .arg(dataset.pltMin)
                           .arg(dataset.pltMax),
                         trProblem("Set the plot maximum above the plot minimum, or set both to "
                                   "zero to scale the axis automatically."),
                         dataset.uniqueId,
                         kJumpDataset));
}

/**
 * @brief Flags an analog widget whose configured value range cannot be drawn.
 */
static void checkWidgetRange(const DataModel::Dataset& dataset, QList<Finding>& out)
{
  const auto widget = SerialStudio::datasetWidgetFromId(dataset.widget);
  const bool scaled =
    widget == SerialStudio::Bar || widget == SerialStudio::Gauge || widget == SerialStudio::Meter;
  if (!scaled || !rangeIsBroken(dataset.wgtMin, dataset.wgtMax))
    return;

  out.append(makeFinding(Misc::ProblemCenter::Warning,
                         "inverted-widget-range",
                         trProblem("Widget range is inverted or empty"),
                         trProblem("\"%1\" has a widget minimum of %2 and a maximum of %3, so the "
                                   "indicator has no scale to move along.")
                           .arg(datasetLabel(dataset))
                           .arg(dataset.wgtMin)
                           .arg(dataset.wgtMax),
                         trProblem("Set the widget maximum above the widget minimum."),
                         dataset.uniqueId,
                         kJumpDataset));
}

/**
 * @brief Flags an FFT plot whose configured magnitude range cannot be drawn.
 */
static void checkFftRange(const DataModel::Dataset& dataset, QList<Finding>& out)
{
  if (!dataset.fft || !rangeIsBroken(dataset.fftMin, dataset.fftMax))
    return;

  out.append(makeFinding(Misc::ProblemCenter::Warning,
                         "inverted-fft-range",
                         trProblem("FFT range is inverted or empty"),
                         trProblem("\"%1\" has an FFT minimum of %2 and a maximum of %3, so the "
                                   "spectrum has no room to be drawn.")
                           .arg(datasetLabel(dataset))
                           .arg(dataset.fftMin)
                           .arg(dataset.fftMax),
                         trProblem("Set the FFT maximum above the FFT minimum, or set both to "
                                   "zero to scale the axis automatically."),
                         dataset.uniqueId,
                         kJumpDataset));
}

/**
 * @brief Flags an LED threshold that sits outside the dataset's own value range, which leaves the
 *        indicator permanently on or permanently off. Datasets with alarm bands are exempt: the
 *        LED panel drives its states from the bands and ledHigh is dead data (the form hides it).
 */
static void checkLedThreshold(const DataModel::Dataset& dataset, QList<Finding>& out)
{
  if (!dataset.led || dataset.wgtMin >= dataset.wgtMax || !dataset.alarmBands.empty())
    return;

  if (dataset.ledHigh >= dataset.wgtMin && dataset.ledHigh <= dataset.wgtMax)
    return;

  out.append(makeFinding(Misc::ProblemCenter::Warning,
                         "led-threshold-outside-range",
                         trProblem("LED threshold is outside the value range"),
                         trProblem("\"%1\" lights its LED at %2, which lies outside its value "
                                   "range of %3 to %4.")
                           .arg(datasetLabel(dataset))
                           .arg(dataset.ledHigh)
                           .arg(dataset.wgtMin)
                           .arg(dataset.wgtMax),
                         trProblem("Move the LED threshold inside the dataset's value range."),
                         dataset.uniqueId,
                         kJumpDataset));
}

/**
 * @brief Flags alarm bands that are inverted, so the band never matches a value.
 */
static void checkAlarmBands(const DataModel::Dataset& dataset, QList<Finding>& out)
{
  for (const auto& band : dataset.alarmBands) {
    if (!rangeIsBroken(band.min, band.max))
      continue;

    out.append(makeFinding(Misc::ProblemCenter::Warning,
                           "inverted-alarm-band",
                           trProblem("Alarm band is inverted or empty"),
                           trProblem("An alarm band on \"%1\" runs from %2 to %3, so no value can "
                                     "ever fall inside it.")
                             .arg(datasetLabel(dataset))
                             .arg(band.min)
                             .arg(band.max),
                           trProblem("Set the band's upper bound above its lower bound."),
                           dataset.uniqueId,
                           kJumpDataset));
    return;
  }
}

/**
 * @brief Runs every numeric range check over the project's datasets.
 */
static void checkNumericRanges(QList<Finding>& out)
{
  if (!projectAvailable())
    return;

  static auto& project = DataModel::ProjectModel::instance();

  const auto& groups = project.groups();
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      checkPlotRange(dataset, out);
      checkWidgetRange(dataset, out);
      checkFftRange(dataset, out);
      checkLedThreshold(dataset, out);
      checkAlarmBands(dataset, out);
    }
  }

  capFindings(out);
}

//--------------------------------------------------------------------------------------------------
// Duplicate alias check
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flags datasets that claim an alias another dataset already uses; scripts and the API
 *        resolve an alias to exactly one dataset, so the second one is unreachable by name.
 */
static void checkDuplicateAliases(QList<Finding>& out)
{
  if (!projectAvailable())
    return;

  static auto& project = DataModel::ProjectModel::instance();

  QHash<QString, QString> seen;
  const auto& groups = project.groups();
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (dataset.alias.isEmpty())
        continue;

      const auto it = seen.constFind(dataset.alias);
      if (it == seen.constEnd()) {
        seen.insert(dataset.alias, datasetLabel(dataset));
        continue;
      }

      out.append(makeFinding(Misc::ProblemCenter::Warning,
                             "duplicate-dataset-alias",
                             trProblem("Two datasets share an alias"),
                             trProblem("\"%1\" and \"%2\" both use the alias \"%3\", so scripts "
                                       "and the API can only reach one of them by name.")
                               .arg(it.value(), datasetLabel(dataset), dataset.alias),
                             trProblem("Give one of the datasets a different alias."),
                             dataset.uniqueId,
                             kJumpDataset));
    }
  }

  capFindings(out);
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers the project-schema checkers, all of which run on a project change and on an
 *        explicit re-run request.
 */
void Misc::ProjectCheckers::registerAll()
{
  static auto& center   = Misc::ProblemCenter::instance();
  const quint8 triggers = Misc::ProblemCenter::ProjectChanged | Misc::ProblemCenter::OnDemand;

  center.registerChecker(QStringLiteral("project.frame-index"), triggers, checkFrameIndices);
  center.registerChecker(QStringLiteral("project.empty-group"), triggers, checkEmptyGroups);
  center.registerChecker(QStringLiteral("project.reference"), triggers, checkDanglingReferences);
  center.registerChecker(QStringLiteral("project.numeric-range"), triggers, checkNumericRanges);
  center.registerChecker(QStringLiteral("project.alias"), triggers, checkDuplicateAliases);
}
