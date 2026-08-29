/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "API/Handlers/ProjectUpdateCommands.h"

#include <cmath>
#include <limits>
#include <optional>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

#include "API/Handlers/ProjectApiSupport.h"
#include "API/Handlers/ProjectHandler.h"
#include "API/SchemaBuilder.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "SerialStudio.h"

using namespace API::Handlers::ProjectApiSupport;

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Legacy alarm-field synthesis for MCP input compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies simple-mode alarmEnabled / alarmLow / alarmHigh fields to a dataset's alarmBands.
 *        Non-static: the generated dataset field appliers call it for the v3.3 legacy inputs.
 */
void applySimpleAlarmFields(DataModel::Dataset& d,
                            std::optional<bool> enabled,
                            std::optional<double> low,
                            std::optional<double> high)
{
  bool curEnabled       = !d.alarmBands.empty();
  double curLow         = std::numeric_limits<double>::quiet_NaN();
  double curHigh        = std::numeric_limits<double>::quiet_NaN();
  const double rangeMin = qMin(d.wgtMin, d.wgtMax);
  const double rangeMax = qMax(d.wgtMin, d.wgtMax);
  for (const auto& b : d.alarmBands) {
    if (static_cast<int>(b.severity) < 2)
      continue;

    const double bLo = qMin(b.min, b.max);
    const double bHi = qMax(b.min, b.max);
    if (qFuzzyCompare(1.0 + bLo, 1.0 + rangeMin) && bHi < rangeMax)
      curLow = bHi;
    else if (bLo > rangeMin && qFuzzyCompare(1.0 + bHi, 1.0 + rangeMax))
      curHigh = bLo;
  }

  const bool useEnabled = enabled.value_or(curEnabled);
  const double useLow   = low.value_or(curLow);
  const double useHigh  = high.value_or(curHigh);

  d.alarmBands.clear();
  if (!useEnabled)
    return;

  const double range = rangeMax - rangeMin;
  if (range <= 0)
    return;

  const double lo = std::isnan(useLow) ? rangeMin + range * 0.20 : useLow;
  const double hi = std::isnan(useHigh) ? rangeMin + range * 0.80 : useHigh;
  if (lo > rangeMin && lo < rangeMax) {
    DataModel::AlarmBand band;
    band.min      = rangeMin;
    band.max      = lo;
    band.severity = DataModel::AlarmSeverity::Warning;
    d.alarmBands.push_back(band);
  }

  if (hi > rangeMin && hi < rangeMax && hi > lo) {
    DataModel::AlarmBand band;
    band.min      = hi;
    band.max      = rangeMax;
    band.severity = DataModel::AlarmSeverity::Warning;
    d.alarmBands.push_back(band);
  }
}

/**
 * @brief Appends an unknown_field warning to @p result when @p params has unconsumed keys.
 */
static void appendUnknownFieldsWarning(QJsonObject& result,
                                       const QJsonObject& params,
                                       const QSet<QString>& consumed,
                                       const QString& command)
{
  QJsonArray unknownFields;
  for (const auto& key : params.keys())
    if (!consumed.contains(key))
      unknownFields.append(key);

  if (unknownFields.isEmpty())
    return;

  QJsonArray warnings;
  if (result.contains(QStringLiteral("warnings")))
    warnings = result.value(QStringLiteral("warnings")).toArray();

  QJsonObject w;
  w[QStringLiteral("code")]   = QStringLiteral("unknown_field");
  w[QStringLiteral("fields")] = unknownFields;
  w[QStringLiteral("message")] =
    QStringLiteral("These fields were ignored because they are not patchable via %1. "
                   "Call meta.describeCommand for the list of writable fields, "
                   "or check your spelling.")
      .arg(command);
  warnings.append(w);

  result[QStringLiteral("warnings")] = warnings;
}

}  // namespace API::Handlers

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the project.dataset.update schema: the two identity params plus every typed field
 *        the property registry declares, so callers read the field list instead of parsing prose.
 */
[[nodiscard]] static QJsonObject datasetUpdateSchema()
{
  auto schema = API::makeSchema({
    {QStringLiteral("groupId"), QStringLiteral("integer"),   QStringLiteral("Target group id")},
    {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Target dataset id")}
  });

  auto properties   = schema.value(QStringLiteral("properties")).toObject();
  const auto fields = API::Handlers::datasetFieldSchema();
  for (auto it = fields.constBegin(); it != fields.constEnd(); ++it)
    properties.insert(it.key(), it.value());

  schema.insert(QStringLiteral("properties"), properties);
  return schema;
}

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectUpdateCommands::ProjectUpdateCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Registers project.group.update, project.dataset.update, project.action.update and
 *        project.outputWidget.update.
 */
void API::Handlers::ProjectUpdateCommands::registerCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.group.update"),
    QStringLiteral("Patch group fields by id (params: groupId, plus any of title, "
                   "widget, columns, sourceId, painterCode). Unknown fields are "
                   "accepted but ignored, and surfaced in result.warnings[].fields "
                   "with code 'unknown_field'."),
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"), QStringLiteral("Target group id")}
  }),
    &groupUpdate);

  registry.registerCommand(
    QStringLiteral("project.dataset.update"),
    QStringLiteral("Patch dataset fields by id: params are groupId, datasetId, plus any of "
                   "the writable fields declared in this command's schema -- read the typed "
                   "'properties' block for each field's type, description and enum domain "
                   "instead of guessing from prose.\n"
                   "Fields whose project-file spelling differs from the API name (plotMin, "
                   "plotMax, widgetMin, widgetMax, xAxis, datasetSourceId) are accepted as "
                   "aliases, so an object read back from the API can be written straight "
                   "through without loss.\n"
                   "Nested collections: alarmBands takes an array of "
                   "{min,max,severity,color,label,blink} objects (severity=0..3 for "
                   "Info/Ok/Warning/Critical), and legacy alarmLow/alarmHigh/alarmEnabled "
                   "still drive the 2-band simple mode; fftMarkers takes an array of "
                   "{freq,endFreq,label,color,warningDb,alarmDb} objects (see "
                   "project.dataset.setFFTMarkers for field semantics).\n"
                   "The boolean fields graph/fft/led/waterfall toggle the same flags as "
                   "project.dataset.setOption -- use them here when patching multiple "
                   "fields at once. Multiplot groups follow the group's FIRST dataset for "
                   "plotLogX/plotLogY, so set those flags on every member to keep the group "
                   "coherent.\n"
                   "Unknown fields are accepted but ignored, and surfaced in "
                   "result.warnings[].fields with code 'unknown_field' so the caller "
                   "can self-correct on the next turn instead of silently mis-applying "
                   "a typo.\n"
                   "REMINDERS for compute-only datasets:\n"
                   "  - Set virtual=true when the dataset's value comes from "
                   "transformCode rather than from a slot in the parser output. "
                   "Without virtual=true the dataset still tries to read "
                   "channels[index-1] and ends up empty.\n"
                   "  - `index` is the 1-based parser-output slot (0 = unassigned). "
                   "Patchable via this call; bulk-renumbering 40 datasets is best "
                   "done through project.batch to avoid round-trip overhead.\n"
                   "  - Set transformLanguage explicitly (0=JavaScript, 1=Lua, "
                   "-1=inherit from source). Mismatched language vs code = silent "
                   "compile failure. Lua is the recommended default; it's faster "
                   "on the hotpath."),
    datasetUpdateSchema(),
    &datasetUpdate);

  registry.registerCommand(
    QStringLiteral("project.action.update"),
    QStringLiteral("Patch action fields by id (params: actionId, plus any of title, icon, "
                   "txData, eolSequence, timerMode, timerIntervalMs, repeatCount, "
                   "sourceId, txEncoding, binaryData, autoExecuteOnConnect). Unknown "
                   "fields are accepted but ignored, and surfaced in "
                   "result.warnings[].fields with code 'unknown_field'."),
    makeSchema({
      {QStringLiteral("actionId"), QStringLiteral("integer"), QStringLiteral("Target action id")}
  }),
    &actionUpdate);

  registry.registerCommand(
    QStringLiteral("project.outputWidget.update"),
    QStringLiteral("Patch output-widget fields by id (params: groupId, widgetId, plus any "
                   "of title, icon, transmitFunction, sourceId, txEncoding, monoIcon, "
                   "minValue, maxValue, stepSize, initialValue). The transmitFunction is "
                   "**JavaScript only** -- runs in QJSEngine to convert UI state into "
                   "device bytes. **Call meta.fetchScriptingDocs{kind:'output_widget_js'} "
                   "before authoring** for the function signature (transmit(value) "
                   "returning a Uint8Array / string), the per-widget value semantics, and "
                   "the injected Modbus/CAN helper globals (modbusWriteRegister / "
                   "modbusWriteCoil / modbusWriteFloat / canSendFrame / canSendValue). "
                   "Validate first with project.outputWidget.dryRun."),
    makeSchema({
      { QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Target group id")                           },
      {QStringLiteral("widgetId"),
       QStringLiteral("integer"),
       QStringLiteral("Target widget index within the group")}
  }),
    &outputWidgetUpdate);
}

//--------------------------------------------------------------------------------------------------
// Bulk update mutators: stateless, id required, PATCH semantics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Patch any subset of group fields by id.
 */
API::CommandResponse API::Handlers::ProjectUpdateCommands::groupUpdate(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  bool rebuildTree   = false;
  QSet<QString> consumed{QStringLiteral("groupId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    g.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("widget"))) {
    g.widget    = params.value(QStringLiteral("widget")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("columns")))
    g.columns = params.value(QStringLiteral("columns")).toInt();

  if (take(Keys::SourceId))
    g.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("painterCode")))
    g.painterCode = params.value(QStringLiteral("painterCode")).toString();

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("updated")] = true;
  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.group.update"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of dataset fields by groupId + datasetId; any applied field forces
 *        the tree rebuild so the epoch-gated dashboard apply and editor reload fire.
 */
API::CommandResponse API::Handlers::ProjectUpdateCommands::datasetUpdate(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId  = params.value(Keys::DatasetId).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = groups[groupId].datasets;
  if (datasetId < 0 || static_cast<size_t>(datasetId) >= datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  DataModel::Dataset d = datasets[datasetId];
  bool rebuildTree     = false;
  QSet<QString> consumed{
    QStringLiteral("groupId"), Keys::DatasetId, QStringLiteral("expectedProjectEpoch")};
  const auto identityKeyCount = consumed.size();
  const QString err = ProjectHandler::applyDatasetUpdateParams(d, params, rebuildTree, consumed);
  if (!err.isEmpty())
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, err);

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  const auto preEpoch = captureProjectEpoch();
  project.updateDataset(groupId, datasetId, d, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("updated")] = true;

  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.dataset.update"));
  appendStaleProjectWarning(result, params, preEpoch);
  attachProjectEpoch(result);

  if (!d.transformCode.isEmpty() && d.transformLanguage != -1
      && d.transformLanguage != SerialStudio::Expression) {
    const auto warning = detectLanguageMismatch(d.transformCode, d.transformLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  if (!d.transformCode.isEmpty() && !d.virtual_ && d.index <= 0)
    result[QStringLiteral("hint")] =
      QStringLiteral("transformCode set but virtual=false and index<=0; if this "
                     "dataset has no slot in the parser output array, set "
                     "virtual=true (next call: project.dataset.update{virtual:true}).");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of action fields by id.
 */
API::CommandResponse API::Handlers::ProjectUpdateCommands::actionUpdate(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId   = params.value(QStringLiteral("actionId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& actions  = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  DataModel::Action a = actions[actionId];
  bool rebuildTree    = false;
  QSet<QString> consumed{QStringLiteral("actionId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    a.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("icon"))) {
    a.icon      = params.value(QStringLiteral("icon")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("txData")))
    a.txData = params.value(QStringLiteral("txData")).toString();

  if (take(QStringLiteral("eolSequence")))
    a.eolSequence = params.value(QStringLiteral("eolSequence")).toString();

  if (take(QStringLiteral("timerMode")))
    a.timerMode =
      static_cast<DataModel::TimerMode>(params.value(QStringLiteral("timerMode")).toInt());

  if (take(QStringLiteral("timerIntervalMs")))
    a.timerIntervalMs = params.value(QStringLiteral("timerIntervalMs")).toInt();

  if (take(QStringLiteral("repeatCount")))
    a.repeatCount = params.value(QStringLiteral("repeatCount")).toInt();

  if (take(Keys::SourceId))
    a.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("txEncoding")))
    a.txEncoding = params.value(QStringLiteral("txEncoding")).toInt();

  if (take(QStringLiteral("binaryData")))
    a.binaryData = params.value(QStringLiteral("binaryData")).toBool();

  if (take(QStringLiteral("autoExecuteOnConnect")))
    a.autoExecuteOnConnect = params.value(QStringLiteral("autoExecuteOnConnect")).toBool();

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateAction(actionId, a, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("actionId")] = actionId;
  result[QStringLiteral("updated")]  = true;
  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.action.update"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch any subset of output-widget fields by groupId + widgetId.
 */
API::CommandResponse API::Handlers::ProjectUpdateCommands::outputWidgetUpdate(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId   = params.value(QStringLiteral("widgetId")).toInt();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= g.outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  DataModel::OutputWidget& w = g.outputWidgets[widgetId];
  bool rebuildTree           = false;
  QSet<QString> consumed{QStringLiteral("groupId"), QStringLiteral("widgetId")};
  const auto identityKeyCount = consumed.size();

  const auto take = [&](const QString& key) -> bool {
    if (!params.contains(key))
      return false;

    consumed.insert(key);
    return true;
  };

  if (take(QStringLiteral("title"))) {
    w.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("icon"))) {
    w.icon      = params.value(QStringLiteral("icon")).toString();
    rebuildTree = true;
  }
  if (take(QStringLiteral("transmitFunction")))
    w.transmitFunction = params.value(QStringLiteral("transmitFunction")).toString();

  if (take(Keys::SourceId))
    w.sourceId = params.value(Keys::SourceId).toInt();

  if (take(QStringLiteral("txEncoding")))
    w.txEncoding = params.value(QStringLiteral("txEncoding")).toInt();

  if (take(QStringLiteral("monoIcon"))) {
    w.monoIcon  = params.value(QStringLiteral("monoIcon")).toBool();
    rebuildTree = true;
  }

  if (take(QStringLiteral("minValue")))
    w.minValue = SerialStudio::toDouble(params.value(QStringLiteral("minValue")));

  if (take(QStringLiteral("maxValue")))
    w.maxValue = SerialStudio::toDouble(params.value(QStringLiteral("maxValue")));

  if (take(QStringLiteral("stepSize")))
    w.stepSize = SerialStudio::toDouble(params.value(QStringLiteral("stepSize")));

  if (take(QStringLiteral("initialValue")))
    w.initialValue = SerialStudio::toDouble(params.value(QStringLiteral("initialValue")));

  if (consumed.size() > identityKeyCount)
    rebuildTree = true;

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("updated")]  = true;
  appendUnknownFieldsWarning(
    result, params, consumed, QStringLiteral("project.outputWidget.update"));
  return CommandResponse::makeSuccess(id, result);
}
