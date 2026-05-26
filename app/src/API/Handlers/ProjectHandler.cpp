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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ProjectHandler.h"

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <QFile>
#include <QHash>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "IO/ConnectionManager.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Legacy alarm-field synthesis for MCP input compatibility
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies simple-mode alarmEnabled / alarmLow / alarmHigh fields to a dataset's alarmBands.
 */
static void applySimpleAlarmFields(DataModel::Dataset& d,
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
 * @brief Appends a dataset's compatible DashboardWidget enums to compat (deduped).
 */
static void appendDatasetWidgetTypes(const DataModel::Dataset& ds, QJsonArray& compat)
{
  for (const auto w : SerialStudio::getDashboardWidgets(ds)) {
    const auto v = static_cast<int>(w);
    if (!compat.contains(v))
      compat.append(v);
  }
}

/**
 * @brief Returns the per-dataset bitflag summary (mirrors enabled visualization options).
 */
static int datasetOptionsBitflag(const DataModel::Dataset& d);

/**
 * @brief Builds the canonical dataset response object used by list / get* / snapshot.
 */
static QJsonObject buildDatasetObject(const DataModel::Dataset& dataset,
                                      const DataModel::Group& group)
{
  QJsonObject dataset_obj = DataModel::serialize(dataset);

  dataset_obj[QStringLiteral("groupId")]    = group.groupId;
  dataset_obj[QStringLiteral("groupTitle")] = group.title;
  dataset_obj[Keys::SourceId]               = dataset.sourceId;
  dataset_obj[Keys::UniqueId]               = dataset.uniqueId;

  QStringList enabled;
  if (dataset.plt)
    enabled.append(QStringLiteral("plot"));

  if (dataset.fft)
    enabled.append(QStringLiteral("FFT"));

  if (dataset.led)
    enabled.append(QStringLiteral("LED"));

  if (dataset.log)
    enabled.append(QStringLiteral("log"));

  if (dataset.waterfall)
    enabled.append(QStringLiteral("waterfall"));

  if (!dataset.alarmBands.empty())
    enabled.append(QStringLiteral("alarm"));

  if (!dataset.widget.isEmpty())
    enabled.append(dataset.widget);

  dataset_obj[QStringLiteral("enabledFeatures")] =
    enabled.isEmpty() ? QStringLiteral("plain numeric") : enabled.join(QStringLiteral(", "));

  const int optionsBits                         = datasetOptionsBitflag(dataset);
  dataset_obj[QStringLiteral("enabledOptions")] = optionsBits;

  QJsonArray optionSlugs;
  for (const auto& slug : API::EnumLabels::datasetOptionsBitsToSlugs(optionsBits))
    optionSlugs.append(slug);

  dataset_obj[QStringLiteral("enabledOptionsSlugs")] = optionSlugs;

  QJsonArray ds_widget_types;
  appendDatasetWidgetTypes(dataset, ds_widget_types);
  dataset_obj[QStringLiteral("enabledWidgetTypes")] = ds_widget_types;

  QJsonArray ds_widget_type_slugs;
  for (const auto w : SerialStudio::getDashboardWidgets(dataset)) {
    const auto slug = API::EnumLabels::dashboardWidgetSlug(static_cast<int>(w));
    if (!ds_widget_type_slugs.contains(slug))
      ds_widget_type_slugs.append(slug);
  }
  dataset_obj[QStringLiteral("enabledWidgetTypesSlugs")] = ds_widget_type_slugs;

  dataset_obj[QStringLiteral("hasTransform")] = !dataset.transformCode.isEmpty();
  dataset_obj[QStringLiteral("isVirtual")]    = dataset.virtual_;

  // Prose explanations block -- saves the caller from translating raw enums and bitflags
  QJsonObject explanations;
  explanations[QStringLiteral("enabledOptions")] =
    QStringLiteral("%1 (bitflag %2)")
      .arg(API::EnumLabels::datasetOptionsLabel(optionsBits))
      .arg(optionsBits);

  if (!dataset.transformCode.isEmpty())
    explanations[QStringLiteral("transformLanguage")] =
      QStringLiteral("Transform script in %1 (%2 bytes)")
        .arg(API::EnumLabels::scriptLanguageLabel(dataset.transformLanguage))
        .arg(dataset.transformCode.size());

  if (dataset.virtual_)
    explanations[QStringLiteral("virtual")] =
      QStringLiteral("Virtual dataset: value comes from transform(), not a parser-output slot");

  if (dataset.index == 0 && !dataset.virtual_)
    explanations[QStringLiteral("index")] =
      QStringLiteral("index=0 means the dataset is unassigned -- it will read nothing unless "
                     "you set index>=1 (parser-output slot) or virtual=true");

  dataset_obj[QStringLiteral("_explanations")] = explanations;
  return dataset_obj;
}

/**
 * @brief Adds projectEpoch (monotonic mutation counter) to @p result.
 */
static void attachProjectEpoch(QJsonObject& result)
{
  result[QStringLiteral("projectEpoch")] =
    static_cast<qint64>(DataModel::ProjectModel::instance().mutationEpoch());
}

/**
 * @brief Snapshot of the project epoch before a mutating handler runs.
 */
[[nodiscard]] static qint64 captureProjectEpoch()
{
  return DataModel::ProjectModel::instance().mutationEpoch();
}

/**
 * @brief Append a stale_project warning when the caller's expectedProjectEpoch is stale.
 */
static void appendStaleProjectWarning(QJsonObject& result,
                                      const QJsonObject& params,
                                      qint64 preMutationEpoch)
{
  if (!params.contains(QStringLiteral("expectedProjectEpoch")))
    return;

  const qint64 expected =
    params.value(QStringLiteral("expectedProjectEpoch")).toVariant().toLongLong();
  if (preMutationEpoch == expected)
    return;

  QJsonArray warnings;
  if (result.contains(QStringLiteral("warnings")))
    warnings = result.value(QStringLiteral("warnings")).toArray();

  QJsonObject w;
  w[QStringLiteral("code")]                 = QStringLiteral("stale_project");
  w[QStringLiteral("expectedProjectEpoch")] = expected;
  w[QStringLiteral("currentProjectEpoch")]  = preMutationEpoch;
  w[QStringLiteral("message")] =
    QStringLiteral("Project was mutated %1 time(s) between the epoch you supplied and "
                   "this call. uniqueIds derived from the older snapshot may now point "
                   "at a different group/dataset -- refetch via project.snapshot or "
                   "resolve by path with project.dataset.getByPath before mutating "
                   "further. The current call still executed as requested.")
      .arg(preMutationEpoch - expected);
  warnings.append(w);
  result[QStringLiteral("warnings")] = warnings;
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
                   "Call project.describeCommand for the list of writable fields, "
                   "or check your spelling.")
      .arg(command);
  warnings.append(w);

  result[QStringLiteral("warnings")] = warnings;
}

/**
 * @brief Flags obvious language/syntax mismatches; returns a short warning or empty.
 */
[[nodiscard]] static QString detectLanguageMismatch(const QString& code, int language)
{
  static const QString kJsHallmarks[] = {
    QStringLiteral("\nvar "),
    QStringLiteral("\nlet "),
    QStringLiteral("\nconst "),
    QStringLiteral(" => "),
    QStringLiteral("=== "),
    QStringLiteral("!== "),
    QStringLiteral(") {"),
    QStringLiteral("} else"),
  };
  static const QString kLuaHallmarks[] = {
    QStringLiteral("\nlocal "),
    QStringLiteral("\nfunction "),
    QStringLiteral(" then\n"),
    QStringLiteral(" do\n"),
    QStringLiteral("\nend\n"),
    QStringLiteral("\nelseif "),
    QStringLiteral("--[["),
  };

  bool looksJs  = false;
  bool looksLua = false;
  for (const auto& m : kJsHallmarks)
    if (code.contains(m)) {
      looksJs = true;
      break;
    }
  for (const auto& m : kLuaHallmarks)
    if (code.contains(m)) {
      looksLua = true;
      break;
    }

  if (language == SerialStudio::JavaScript && looksLua && !looksJs)
    return QStringLiteral("language=0 (JavaScript) but the code contains Lua-only "
                          "syntax (e.g. 'local', 'end', 'then'). The script will "
                          "fail to compile silently. Either pass language=1 (Lua) "
                          "or rewrite the code in JavaScript.");

  if (language == SerialStudio::Lua && looksJs && !looksLua)
    return QStringLiteral("language=1 (Lua) but the code contains JS-only syntax "
                          "(e.g. 'var', 'let', 'const', '=>'). The script will "
                          "fail to compile silently. Either pass language=0 "
                          "(JavaScript) or rewrite the code in Lua.");

  return QString();
}

/**
 * @brief Returns the DatasetOption bitflag value of @a ds (1=Plot, 2=FFT, ...).
 */
static int datasetOptionsBitflag(const DataModel::Dataset& ds)
{
  int flags = SerialStudio::DatasetGeneric;
  if (ds.plt)
    flags |= SerialStudio::DatasetPlot;

  if (ds.fft)
    flags |= SerialStudio::DatasetFFT;

  if (ds.led)
    flags |= SerialStudio::DatasetLED;

  if (ds.waterfall)
    flags |= SerialStudio::DatasetWaterfall;

  static const QHash<QString, int> kWidgetFlags = {
    {    QStringLiteral("bar"),     SerialStudio::DatasetBar},
    {  QStringLiteral("gauge"),   SerialStudio::DatasetGauge},
    {QStringLiteral("compass"), SerialStudio::DatasetCompass},
    {  QStringLiteral("meter"),   SerialStudio::DatasetMeter},
  };
  flags |= kWidgetFlags.value(ds.widget, 0);

  return flags;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Project commands with the registry
 */
void API::Handlers::ProjectHandler::registerCommands()
{
  registerFileCommands();
  registerGroupCommands();
  registerDatasetCommands();
  registerActionCommands();
  registerOutputWidgetCommands();
  registerParserCommands();
  registerPainterCommands();
  registerListCommands();
  registerTemplateCommands();
}

/**
 * @brief Register file lifecycle commands (new/open/save/load/getStatus).
 */
void API::Handlers::ProjectHandler::registerFileCommands()
{
  registerFileLifecycleCommands();
  registerFileMetadataCommands();
}

/**
 * @brief Register project new/open/save/loadJson/setTitle commands.
 */
void API::Handlers::ProjectHandler::registerFileLifecycleCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.new"),
    QStringLiteral("Reset to a blank project (one default UART source, no groups, no "
                   "datasets, no actions, no workspaces). DESTRUCTIVE: discards the "
                   "loaded project. Use only when the user explicitly says \"start "
                   "over\" or \"new project\". For starting a TYPED project, prefer "
                   "project.template.apply -- it gives the user a useful skeleton "
                   "instead of an empty canvas. Pass dryRun:true to see what the "
                   "current project (wouldDiscard) looks like before wiping it."),
    makeSchema(
      {
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return wouldDiscard + wouldCreate summary without "
                       "committing.")}}),
    &fileNew);

  registry.registerCommand(
    QStringLiteral("project.setTitle"),
    QStringLiteral("Rename the project (only the in-app title). Does not move or rename "
                   "the .ssproj file on disk; auto-save still writes to the existing "
                   "file path. To save to a different file, use project.save{filePath}."),
    makeSchema({
      {QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("Project title")}
  }),
    &setTitle);

  registry.registerCommand(
    QStringLiteral("project.open"),
    QStringLiteral("Open a .ssproj or .json project file. Replaces the current project. "
                   "Auto-switches operationMode to ProjectFile if it was QuickPlot or "
                   "ConsoleOnly. Path must be absolute. Pass dryRun:true to read the "
                   "file and return wouldDiscard + wouldApply summaries without loading."),
    makeSchema(
      {
        {QStringLiteral("filePath"),
         QStringLiteral("string"),
         QStringLiteral("Absolute path to project file (.json or .ssproj)")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, parse the file and return its summary without "
                       "loading.")}}),
    &fileOpen);

  registry.registerCommand(
    QStringLiteral("project.save"),
    QStringLiteral("Write the current project to disk. Note: the AI runtime auto-saves "
                   "after every successful mutating tool call within ~1 second, so you "
                   "do NOT need to call this explicitly when editing. Call it ONLY "
                   "when the user explicitly says \"save\" or wants a different file "
                   "path -- in which case pass {filePath: \"/abs/path\"} for headless "
                   "save-as."),
    makeSchema(
      {
  },
      {{QStringLiteral("filePath"),
        QStringLiteral("string"),
        QStringLiteral("Absolute path to save to (headless save-as). Omit to save to "
                       "the project's existing file path.")},
       {QStringLiteral("askPath"),
        QStringLiteral("boolean"),
        QStringLiteral("Show native save dialog. Default false. AI runtime should "
                       "almost always leave false -- the user already approved.")}}),
    &fileSave);

  registry.registerCommand(
    QStringLiteral("project.loadJson"),
    QStringLiteral("Replace the current project with a JSON object IN MEMORY (no file "
                   "association). Use when you have a project shape ready to install -- "
                   "e.g. building a project from scratch in one shot, or implementing a "
                   "custom template. The JSON shape must match the .ssproj schema "
                   "(top-level: title, frameStart, frameEnd, frameDetection, decoder, "
                   "frameParser, groups, actions, ...). Prefer project.template.apply "
                   "for canned starters. Pass dryRun:true to return wouldDiscard + "
                   "wouldApply summaries without loading."),
    makeSchema(
      {
        {QStringLiteral("config"),
         QStringLiteral("object"),
         QStringLiteral("Full project JSON document")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, summarize the config without loading it.")}}),
    &loadFromJSON);
}

/**
 * @brief Register project getStatus/validate/exportJson/activate commands.
 */
void API::Handlers::ProjectHandler::registerFileMetadataCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.getStatus"),
    QStringLiteral("Returns top-level project state: title, file path, modified flag, "
                   "operation mode (ProjectFile/ConsoleOnly/QuickPlot), counts of "
                   "groups/datasets/sources/actions/workspaces. Useful as a sanity "
                   "check before destructive operations."),
    empty,
    &getStatus);

  registry.registerCommand(
    QStringLiteral("project.validate"),
    QStringLiteral("Walk the loaded project and report inconsistencies (missing source "
                   "references, parser compile errors, empty groups, duplicate dataset "
                   "indexes, etc.). Returns {ok, issues:[{level, location, message}], "
                   "issueCount, groupCount, sourceCount, actionCount}. Call before "
                   "project.save when building a project programmatically."),
    empty,
    &validate);

  registry.registerCommand(QStringLiteral("project.exportJson"),
                           QStringLiteral("Export project as JSON"),
                           empty,
                           &exportJson);

  registry.registerCommand(QStringLiteral("project.activate"),
                           QStringLiteral("Load current project into FrameBuilder"),
                           empty,
                           &loadIntoFrameBuilder);
}

/**
 * @brief Register group CRUD commands.
 */
void API::Handlers::ProjectHandler::registerGroupCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.group.add"),
    QStringLiteral("Create a new visualization group. Pick widgetType by data shape:\n"
                   "  - 5 (NoGroupWidget): just hold related datasets together; per-"
                   "dataset widgets render individually. Default for arbitrary "
                   "scalar data.\n"
                   "  - 4 (MultiPlot): N values plotted on a shared time axis. The "
                   "right choice for correlated signals (sensor-array, multi-channel "
                   "ADC).\n"
                   "  - 0 (DataGrid): tabular numeric readout. Good for long lists "
                   "of scalars where graphing isn't useful.\n"
                   "  - 1/2/3 (Accelerometer / Gyroscope / GPS): typed 3-axis IMU or "
                   "GPS group. Datasets must follow conventional widget tags "
                   "(\"x\", \"y\", \"z\" for IMUs; \"lat\", \"lon\", \"alt\" for GPS).\n"
                   "  - 6 (Plot3D, Pro): 3D point trail from three datasets.\n"
                   "  - 7 (ImageView, Pro): displays an embedded JPEG/PNG stream.\n"
                   "  - 8 (Painter, Pro): user-scripted JS canvas. Group can be "
                   "EMPTY (no datasets) and read peer datasets via "
                   "datasetGetFinal(uniqueId). See meta.howTo('add_painter').\n"
                   "Don't pick 0 / DataGrid as a default -- it makes a forgettable "
                   "table. Match the user's data."),
    makeSchema({
      {     QStringLiteral("title"),
       QStringLiteral("string"),
       QStringLiteral("Group title shown in dashboard headers and the Project Editor tree")},
      {QStringLiteral("widgetType"),
       QStringLiteral("integer"),
       QStringLiteral("GroupWidget enum -- see command description for decision "
       "guidance. 0=DataGrid, 1=Accelerometer, 2=Gyroscope, 3=GPS, "
       "4=MultiPlot, 5=NoGroupWidget, 6=Plot3D, 7=ImageView, 8=Painter")                   }
  }),
    &groupAdd);

  registry.registerCommand(
    QStringLiteral("project.group.delete"),
    QStringLiteral("Delete a group by id. Pass dryRun:true to preview what would change "
                   "without committing -- the response contains the same {deleted, "
                   "renumbered, warnings} fields as a real call, plus a top-level "
                   "dryRun:true flag. Always preview before committing when the user "
                   "doesn't have a backup workflow."),
    makeSchema(
      {
        {QStringLiteral("groupId"),
         QStringLiteral("integer"),
         QStringLiteral("Group id to delete")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &groupDelete);

  registry.registerCommand(QStringLiteral("project.group.duplicate"),
                           QStringLiteral("Duplicate a group by id (params: groupId)"),
                           makeSchema({
                             {QStringLiteral("groupId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Group id to duplicate")}
  }),
                           &groupDuplicate);
}

/**
 * @brief Register dataset CRUD and field-setter commands.
 */
void API::Handlers::ProjectHandler::registerDatasetCommands()
{
  registerDatasetCrudCommands();
  registerDatasetFieldCommands();
  registerDatasetAlarmCommands();
}

/**
 * @brief Registers project.dataset.add and project.dataset.addMany.
 */
void API::Handlers::ProjectHandler::registerDatasetCreateCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.add"),
    QStringLiteral("Add a dataset to a group. A dataset is one channel of incoming "
                   "data: it gets one column in CSV/MDF4 export, one entry in the "
                   "frame parser's output array (its `index` field), and zero or "
                   "more dashboard widgets driven by the `options` bitflags.\n"
                   "Pass `options` as the bitwise OR of the visualizations you want:\n"
                   "  1 = Plot (time-series line)\n"
                   "  2 = FFT (frequency-domain plot)\n"
                   "  4 = Bar\n"
                   "  8 = Gauge (radial dial; needs widgetMin/widgetMax)\n"
                   "  16 = Compass (heading 0-360)\n"
                   "  32 = LED (binary indicator with ledHigh threshold)\n"
                   "  64 = Waterfall (Pro; spectrogram, FFT-driven)\n"
                   "  0 = no widget (raw column for export only)\n"
                   "Combine: 1|8 = 9 = plot AND gauge. After creation, set title, "
                   "units, ranges, and transformCode via project.dataset.update."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Group to attach the dataset to")                                 },
      {QStringLiteral("options"),
       QStringLiteral("integer"),
       QStringLiteral("Visualization bit flags. See description for decision guidance.")}
  }),
    &datasetAdd);

  registry.registerCommand(
    QStringLiteral("project.dataset.addMany"),
    QStringLiteral("Bulk-create N datasets in one call -- the right tool whenever you "
                   "would otherwise loop project.dataset.add. Avoids per-call overhead "
                   "and the autosave-debounce churn that comes with rapid mutation "
                   "bursts. After creation, individual datasets can still be patched "
                   "with project.dataset.update or another project.batch round-trip.\n"
                   "  count          -- how many datasets to create (1..1024).\n"
                   "  options        -- visualization bitfield (same as project.dataset.add).\n"
                   "  titlePattern   -- optional, e.g. 'LED {n}'. {n} is replaced with "
                   "the running counter (startNumber + i), {i} with the zero-based index. "
                   "Omit to keep the auto-generated title from project.dataset.add.\n"
                   "  startNumber    -- optional, default 1; first {n} value.\n"
                   "  startIndex     -- optional, default -1 (auto-assign next free "
                   "parser slot). Pass 0 to leave index unset, or 1+ to assign "
                   "consecutive parser slots starting from there.\n"
                   "Returns {count, created: [{datasetId, title, index, uniqueId}...]}."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Group to attach the datasets to")                                    },
      {  QStringLiteral("count"),
       QStringLiteral("integer"),
       QStringLiteral("How many datasets to create (1..1024)")                              },
      {QStringLiteral("options"),
       QStringLiteral("integer"),
       QStringLiteral("Visualization bit flags. See project.dataset.add for the bit table.")}
  }),
    &datasetAddMany);
}

/**
 * @brief Registers project.dataset.delete/duplicate (single-target lifecycle ops).
 */
void API::Handlers::ProjectHandler::registerDatasetLifecycleCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("Delete a dataset by id. Pass dryRun:true to preview the renumbering "
                   "without committing -- the response carries the same {deleted, "
                   "renumbered, warnings} shape as a real call, plus a top-level "
                   "dryRun:true flag. Always preview before committing when destructive "
                   "intent is even slightly uncertain."),
    makeSchema(
      {
        {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                    },
        {          Keys::DatasetId,
         QStringLiteral("integer"),
         QStringLiteral("Dataset id within the group")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &datasetDelete);

  registry.registerCommand(
    QStringLiteral("project.dataset.duplicate"),
    QStringLiteral("Duplicate a dataset by id (params: groupId, datasetId)"),
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"),             QStringLiteral("Owning group id")},
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")}
  }),
    &datasetDuplicate);
}

/**
 * @brief Registers project.dataset.setOption/setOptions option-bitfield commands.
 */
void API::Handlers::ProjectHandler::registerDatasetOptionCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.setOption"),
    QStringLiteral("DEPRECATED -- prefer project.dataset.setOptions, which takes the "
                   "full bitfield in one call and removes the singular/plural ambiguity. "
                   "Kept for backward compatibility with existing scripts. Toggles one "
                   "DatasetOption flag on a dataset and updates the group's "
                   "compatibleWidgetTypes immediately. option values (DatasetOption "
                   "bitflag): 1=Plot, 2=FFT, 4=Bar, 8=Gauge, 16=Compass, 32=LED, "
                   "64=Waterfall (Pro). Bar/Gauge/Compass are mutually exclusive. "
                   "NOTE: this is a DatasetOption bitflag, NOT a DashboardWidget enum -- "
                   "the numbers do not line up with project.workspace.addWidget's "
                   "widgetType."),
    makeSchema({
      {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                                                            },
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")},
      { QStringLiteral("option"),
       QStringLiteral("string|integer"),
       QStringLiteral("PREFERRED: a slug -- 'plot', 'fft', 'bar', 'gauge', 'compass', "
       "'led', 'waterfall'. Integer DatasetOption bitflag still accepted "
       "(1, 2, 4, 8, 16, 32, 64).")                                                                       },
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("Whether to enable or disable the option")                                          }
  }),
    &datasetSetOption);

  registry.registerCommand(
    QStringLiteral("project.dataset.setOptions"),
    QStringLiteral("Apply several DatasetOption flags at once (plural form of "
                   "project.dataset.setOption). Pass `options` as the bitwise OR of the "
                   "flags you want enabled; any flag NOT set in the value is disabled. "
                   "Bits: 1=Plot, 2=FFT, 4=Bar, 8=Gauge, 16=Compass, 32=LED, "
                   "64=Waterfall (Pro). Bar/Gauge/Compass are mutually exclusive -- if "
                   "more than one is set, the highest bit wins. Updates the group's "
                   "compatibleWidgetTypes immediately. NOTE: these are DatasetOption "
                   "bitflags, NOT DashboardWidget enum values -- the numbers do not "
                   "line up with project.workspace.addWidget's widgetType."),
    makeSchema({
      {QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                                                            },
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Dataset id within the group")},
      {QStringLiteral("options"),
       QStringLiteral("array|integer"),
       QStringLiteral("PREFERRED: an array of slugs (e.g. ['plot','fft','waterfall']). "
       "Integer bitflag still accepted (Plot=1, FFT=2, Bar=4, Gauge=8, "
       "Compass=16, LED=32, Waterfall=64).")                                                              }
  }),
    &datasetSetOptions);
}

/**
 * @brief Register dataset add/delete/duplicate and option-bitfield commands.
 */
void API::Handlers::ProjectHandler::registerDatasetCrudCommands()
{
  registerDatasetCreateCommands();
  registerDatasetLifecycleCommands();
  registerDatasetOptionCommands();
}

/**
 * @brief Register dataset field setters (virtual flag, transform code).
 */
void API::Handlers::ProjectHandler::registerDatasetFieldCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.setVirtual"),
    QStringLiteral("Toggle the virtual flag on a dataset (params: groupId, datasetId, virtual)"),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                           },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                    },
      {  QString(Keys::Virtual),
       QStringLiteral("boolean"),
       QStringLiteral("Mark dataset as virtual (computed by transform)")}
  }),
    &datasetSetVirtual);

  registry.registerCommand(
    QStringLiteral("project.dataset.setTransformCode"),
    QStringLiteral("Set dataset transformCode. Empty clears. Pass `language` whenever "
                   "you author code so the dataset's transformLanguage matches the "
                   "syntax you wrote -- mismatches compile-fail silently. Lua (1) is "
                   "the recommended default; it's measurably faster than JavaScript "
                   "on hot transforms. If this dataset is compute-only (no slot in "
                   "the parser output array), also set virtual=true via "
                   "project.dataset.setVirtual or project.dataset.update. Validate with "
                   "project.dataset.transform.dryRun before setting. **Call "
                   "meta.fetchScriptingDocs{kind: 'transform_lua' | 'transform_js'} "
                   "first** for the transform(value) signature, table API "
                   "(tableGet/tableSet/datasetGetRaw/datasetGetFinal), and "
                   "execution-order rules -- a transform may read RAW values from any "
                   "dataset but only FINAL values of datasets earlier in "
                   "project.dataset.getExecutionOrder."),
    makeSchema(
      {
        {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                           },
        {QString(Keys::DatasetId),
         QStringLiteral("integer"),
         QStringLiteral("Dataset id within the group")                        },
        {  QStringLiteral("code"),
         QStringLiteral("string"),
         QStringLiteral("Transform source (Lua or JS, must match `language`)")}
  },
      {{QStringLiteral("language"),
        QStringLiteral("integer"),
        QStringLiteral("Optional: 0=JavaScript, 1=Lua (recommended). If omitted, "
                       "the dataset inherits the source's frameParserLanguage.")}}),
    &datasetSetTransformCode);
}

/**
 * @brief Register dataset alarm-band getter / setter commands.
 */
void API::Handlers::ProjectHandler::registerDatasetAlarmCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.getAlarmBands"),
    QStringLiteral("Returns the dataset's coloured alarm bands as a JSON array. Each entry: "
                   "{min, max, severity (0=Info, 1=OK, 2=Warning, 3=Critical), color "
                   "(\"#rrggbb\" override or empty for severity default), label}. Also returns "
                   "rangeMin/rangeMax (the dataset's wgtMin/wgtMax) so callers can validate "
                   "band ranges before writing back. An empty array means no alarms "
                   "configured. Applies only to bar / gauge / meter widgets; calling on other "
                   "widget types succeeds with an empty array."),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                  },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")}
  }),
    &datasetGetAlarmBands);

  registry.registerCommand(
    QStringLiteral("project.dataset.setAlarmBands"),
    QStringLiteral("Atomically replaces the dataset's alarmBands array. Each band entry must "
                   "provide numeric min and max (max>min; bands with max<=min are silently "
                   "dropped and counted in result.droppedInvalid), and an integer severity "
                   "(0=Info, 1=OK, 2=Warning, 3=Critical). color is optional (\"#rrggbb\" "
                   "override; empty/missing = severity's theme colour). label is optional "
                   "(surfaces in band-edge notifications). Bands may have gaps and may "
                   "overlap; rendering paints them in array order behind the value indicator. "
                   "Severity >= Warning triggers a notification when the value enters the "
                   "band (3-second per-widget cooldown suppresses oscillation spam).\n"
                   "Pass an empty array to clear all alarms."),
    makeSchema({
      {                     QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                     },
      {                          QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                             },
      arrayProp(
        QString(Keys::AlarmBands),
        QStringLiteral("Full replacement band list (empty = no alarms)."),
        QJsonObject{
       {QStringLiteral("type"), QStringLiteral("object")},
       {QStringLiteral("properties"),
       QJsonObject{{QStringLiteral("min"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Lower bound (inclusive)")}}},
       {QStringLiteral("max"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Upper bound (exclusive at top of range)")}}},
       {QStringLiteral("severity"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")},
       {QStringLiteral("description"),
       QStringLiteral("0=Info, 1=OK, 2=Warning, 3=Critical")}}},
       {QStringLiteral("color"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional \"#rrggbb\" override; empty = "
       "use severity's theme colour")}}},
       {QStringLiteral("label"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional band name (shown in "
       "notifications)")}}}}},
       {QStringLiteral("required"),
       QJsonArray{
       QStringLiteral("min"), QStringLiteral("max"), QStringLiteral("severity")}}}
      )
  }),
    &datasetSetAlarmBands);
}

/**
 * @brief Register action CRUD commands.
 */
void API::Handlers::ProjectHandler::registerActionCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.action.add"),
    QStringLiteral("Create a new outgoing-action button shown on the toolbar. Actions "
                   "transmit a configurable payload (text or binary, with optional "
                   "EOL sequence) to the device on click, or repeat on a timer. After "
                   "creation, populate it with project.action.update {actionId, "
                   "title, txData, eolSequence, timerMode (0=Off, 1=AutoStart, "
                   "2=ToggleOnTrigger), timerIntervalMs, repeatCount, icon}. Common "
                   "uses: 'send AT command', 'request telemetry', 'reset device'."),
    empty,
    &actionAdd);
  registry.registerCommand(QStringLiteral("project.action.delete"),
                           QStringLiteral("Delete an action by id (params: actionId)"),
                           makeSchema({
                             {QStringLiteral("actionId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Action id to delete")}
  }),
                           &actionDelete);
  registry.registerCommand(QStringLiteral("project.action.duplicate"),
                           QStringLiteral("Duplicate an action by id (params: actionId)"),
                           makeSchema({
                             {QStringLiteral("actionId"),
                              QStringLiteral("integer"),
                              QStringLiteral("Action id to duplicate")}
  }),
                           &actionDuplicate);
}

/**
 * @brief Register output-widget CRUD commands.
 */
void API::Handlers::ProjectHandler::registerOutputWidgetCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.outputWidget.add"),
    QStringLiteral("Add an output widget to a group (params: groupId, type)"),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Group id to add the widget to. Output widgets attach to a group's "
       "output panel.")                                                           },
      {   QStringLiteral("type"),
       QStringLiteral("integer"),
       QStringLiteral(
       "OutputWidgetType enum: 0=Button, 1=Slider, 2=Toggle, 3=TextField, 4=Knob")}
  }),
    &outputWidgetAdd);

  registry.registerCommand(
    QStringLiteral("project.outputWidget.delete"),
    QStringLiteral("Delete an output widget by id (params: groupId, widgetId)"),
    makeSchema({
      { QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                 },
      {QStringLiteral("widgetId"),
       QStringLiteral("integer"),
       QStringLiteral("Widget id within the group")}
  }),
    &outputWidgetDelete);

  registry.registerCommand(
    QStringLiteral("project.outputWidget.duplicate"),
    QStringLiteral("Duplicate an output widget by id (params: groupId, widgetId)"),
    makeSchema({
      { QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                 },
      {QStringLiteral("widgetId"),
       QStringLiteral("integer"),
       QStringLiteral("Widget id within the group")}
  }),
    &outputWidgetDuplicate);

  registry.registerCommand(
    QStringLiteral("project.outputWidget.get"),
    QStringLiteral("Read the current configuration of an output widget "
                   "(params: groupId, widgetId). Returns title, icon, type, "
                   "min/max/step/initialValue, transmitFunction. Use BEFORE "
                   "rewriting the transmitFunction so you preserve the user's "
                   "current ranges and labels."),
    makeSchema({
      { QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Owning group id")                 },
      {QStringLiteral("widgetId"),
       QStringLiteral("integer"),
       QStringLiteral("Widget id within the group")}
  }),
    &outputWidgetGet);
}

/**
 * @brief Register frame-parser commands (code/language/configuration).
 */
void API::Handlers::ProjectHandler::registerParserCommands()
{
  registerParserCodeCommands();
  registerParserConfigCommands();
}

/**
 * @brief Register frame-parser code and language commands (set/get code, set/get language).
 */
void API::Handlers::ProjectHandler::registerParserCodeCommands()
{
  auto& registry = CommandRegistry::instance();

  const auto setCodeSchema = makeSchema(
    {
      {QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Frame parser script code (JS or Lua)")}
  },
    {{QString(Keys::SourceId),
      QStringLiteral("integer"),
      QStringLiteral("Source index (default 0)")},
     {QStringLiteral("language"),
      QStringLiteral("integer"),
      QStringLiteral("Optional: 0 = JavaScript, 1 = Lua. When supplied, the source language "
                     "is flipped before the code is validated and script errors are returned "
                     "as API errors.")}});

  registry.registerCommand(QStringLiteral("project.frameParser.setCode"),
                           QStringLiteral("Set frame parser code (params: code, "
                                          "optional sourceId, optional language). "
                                          "Always pass `language` when authoring code "
                                          "to lock in the runtime engine -- mismatch = "
                                          "silent compile failure. Lua (1) is the "
                                          "recommended default; it's faster than "
                                          "JavaScript on the hotpath at typical "
                                          "telemetry rates. Use JavaScript only when "
                                          "you need a JS-specific library or feature. "
                                          "Validate with project.frameParser.dryRun (or "
                                          "dryCompile for a syntax-only check) before "
                                          "setCode. **Call meta.fetchScriptingDocs{kind: "
                                          "'frame_parser_lua' | 'frame_parser_js'} first** "
                                          "for the parse() signature, return-shape rules, "
                                          "and the tableGet/tableSet API."),
                           setCodeSchema,
                           &parserSetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.getCode"),
                           QStringLiteral("Read the current frame parser source for a "
                                          "given data source. Returns {code, language}. "
                                          "Always read BEFORE rewriting -- preserve the "
                                          "user's existing structure where reasonable."),
                           makeSchema(
                             {
  },
                             {{QString(Keys::SourceId),
                               QStringLiteral("integer"),
                               QStringLiteral("Source index (default 0)")}}),
                           &parserGetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.setLanguage"),
                           QStringLiteral("Switch a source between JavaScript and Lua frame "
                                          "parsers. WARNING: this WIPES any existing "
                                          "frameParser code for that source -- the loaded "
                                          "default template for the new language replaces "
                                          "it. If you want to preserve+translate, "
                                          "frameParser.getCode first, switch, then "
                                          "frameParser.setCode with the translated source."),
                           makeSchema(
                             {
                               {QStringLiteral("language"),
                                QStringLiteral("integer"),
                                QStringLiteral("Script language: 0 = JavaScript, 1 = Lua")}
  },
                             {{QString(Keys::SourceId),
                               QStringLiteral("integer"),
                               QStringLiteral("Source identifier (default 0)")}}),
                           &parserSetLanguage);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getLanguage"),
    QStringLiteral("Get the script language used by the frame parser for a given source"),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source identifier (default 0)")}}),
    &parserGetLanguage);
}

/**
 * @brief Register frame-parser configuration commands (delimiters, checksum, mode, getConfig).
 */
void API::Handlers::ProjectHandler::registerParserConfigCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.frameParser.update"),
    QStringLiteral("Configure frame parser settings (params: startSequence, endSequence, "
                   "checksumAlgorithm, frameDetection, operationMode)"),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")},
       {QStringLiteral("startSequence"),
        QStringLiteral("string"),
        QStringLiteral("Frame start delimiter")},
       {QStringLiteral("endSequence"),
        QStringLiteral("string"),
        QStringLiteral("Frame end delimiter")},
       {QString(Keys::ChecksumAlgorithm),
        QStringLiteral("string"),
        QStringLiteral("Checksum algorithm name")},
       {QString(Keys::FrameDetection),
        QStringLiteral("integer"),
        QStringLiteral("Frame detection mode (0-3)")},
       {QStringLiteral("operationMode"),
        QStringLiteral("integer"),
        QStringLiteral("Operation mode (0-2)")}}),
    &frameParserConfigure);

  registry.registerCommand(QStringLiteral("project.frameParser.getConfig"),
                           QStringLiteral("Get frame parser configuration"),
                           emptySchema(),
                           &frameParserGetConfig);
}

/**
 * @brief Register list / enumeration commands.
 */
void API::Handlers::ProjectHandler::registerListCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(QStringLiteral("project.group.list"),
                           QStringLiteral("List all groups with dataset counts"),
                           empty,
                           &groupsList);
  registry.registerCommand(QStringLiteral("project.dataset.list"),
                           QStringLiteral("List all datasets across all groups"),
                           empty,
                           &datasetsList);
  registry.registerCommand(
    QStringLiteral("project.action.list"), QStringLiteral("List all actions"), empty, &actionsList);

  registerResolverCommands();
  registerSnapshotAndMoveCommands();
}

/**
 * @brief Register dataset resolver commands (getByUniqueId, getByTitle, getByPath,
 * getExecutionOrder).
 */
void API::Handlers::ProjectHandler::registerResolverCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.dataset.getByUniqueId"),
    QStringLiteral("Resolve a dataset by its uniqueId. Returns the same shape as the "
                   "elements of project.dataset.list."),
    makeSchema({
      {QString(Keys::UniqueId),
       QStringLiteral("integer"),
       QStringLiteral(
         "Computed as sourceId*1000000 + groupId*10000 + datasetId. Treat as opaque.")}
  }),
    &datasetGetByUniqueId);

  registry.registerCommand(
    QStringLiteral("project.dataset.getByTitle"),
    QStringLiteral("Resolve a dataset by exact title. Pass sourceId / groupId to "
                   "disambiguate when titles repeat across groups."),
    makeSchema(
      {
        {QString(Keys::Title),
         QStringLiteral("string"),
         QStringLiteral("Dataset title (exact match).")}
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Optional sourceId filter.")},
       {QString(Keys::GroupId),
        QStringLiteral("integer"),
        QStringLiteral("Optional groupId filter.")}}),
    &datasetGetByTitle);

  registry.registerCommand(
    QStringLiteral("project.dataset.getByPath"),
    QStringLiteral("Resolve a dataset by title path. Preferred for human-readable "
                   "addressing -- survives uniqueId reordering."),
    makeSchema({
      {QStringLiteral("path"),
       QStringLiteral("string"),
       QStringLiteral("'Group/Dataset' or 'Source/Group/Dataset' (titles, '/'-separated).")}
  }),
    &datasetGetByPath);

  registry.registerCommand(
    QStringLiteral("project.dataset.getExecutionOrder"),
    QStringLiteral("Returns the order datasets execute in during transform processing. "
                   "Useful for debugging cross-dataset transforms (a transform reads final "
                   "values only for datasets earlier in this list)."),
    empty,
    &datasetGetExecutionOrder);
}

/**
 * @brief Register the project.snapshot composite read and dataset/group move commands.
 */
void API::Handlers::ProjectHandler::registerSnapshotAndMoveCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.snapshot"),
    QStringLiteral("Composite read of the active project: title, sources, groups + "
                   "datasets, workspaces summary, and data tables summary -- in one "
                   "round trip. Pass verbose=true for source-level frame settings and "
                   "frame parser source. Prefer this over chaining list/get calls.\n"
                   "Every level (top, source, group, dataset) carries an `_explanations` "
                   "object with prose translations of enum/bitflag fields "
                   "(operationMode, busType, frameDetection, decoderMethod, "
                   "frameParserLanguage, enabledOptions, transformLanguage) and a one-line "
                   "summary of the project's operating shape -- read those first; the "
                   "raw int fields stay alongside for machine processing.\n"
                   "Response includes `projectEpoch`, a monotonic counter that bumps on "
                   "every structural mutation (group/dataset add, delete, move, source "
                   "add/delete). Cache this value alongside any uniqueIds you pull from "
                   "the snapshot, then pass `expectedProjectEpoch` to any later mutating "
                   "command (dataset.update/move/delete, group.move/delete) -- the response "
                   "will carry a stale_project warning if the project changed under you, "
                   "so you can refetch before acting on now-shifted uniqueIds."),
    makeSchema(
      {
  },
      {{QStringLiteral("verbose"),
        QStringLiteral("boolean"),
        QStringLiteral("Include frame parser source and source-level frame settings.")}}),
    &projectSnapshot);

  registry.registerCommand(
    QStringLiteral("project.dataset.move"),
    QStringLiteral("Reorder a dataset within its group. Changes datasetId (and therefore "
                   "uniqueId) for the moved dataset and any it crossed; workspace refs "
                   "re-anchor automatically. Scripts that pinned a uniqueId must be "
                   "updated -- prefer dataset.getByPath in scripts. Pass dryRun:true to "
                   "preview the renumbering without committing."),
    makeSchema(
      {
        {      QString(Keys::UniqueId),
         QStringLiteral("integer"),
         QStringLiteral("Dataset uniqueId to move.")                                     },
        {QStringLiteral("newPosition"),
         QStringLiteral("integer"),
         QStringLiteral("New 0-based position within the group; clamped to valid range.")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &datasetMove);

  registry.registerCommand(
    QStringLiteral("project.group.move"),
    QStringLiteral("Reorder a group within the project. Changes groupId for the moved "
                   "group and any it crossed (which propagates to dataset uniqueIds). "
                   "Workspace refs re-anchor automatically; scripts pinning a uniqueId "
                   "must be updated. Pass dryRun:true to preview the renumbering."),
    makeSchema(
      {
        {       QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Group id to move.")                           },
        {QStringLiteral("newPosition"),
         QStringLiteral("integer"),
         QStringLiteral("New 0-based position; clamped to valid range.")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, return the affected entities without committing. Auto-runs "
                       "without an approval card.")}}),
    &groupMove);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Summarise a serialised project JSON object for dryRun replies (no model mutation).
 */
static QJsonObject summarizeProjectJson(const QJsonObject& project)
{
  QJsonObject out;
  out[QStringLiteral("title")] = project.value(QStringLiteral("title")).toString();

  const auto groups                 = project.value(QStringLiteral("groups")).toArray();
  out[QStringLiteral("groupCount")] = groups.size();

  int datasetCount = 0;
  QJsonArray groupTitles;
  for (const auto& gv : groups) {
    const auto g  = gv.toObject();
    datasetCount += g.value(QStringLiteral("datasets")).toArray().size();
    if (groupTitles.size() < 32)
      groupTitles.append(g.value(QStringLiteral("title")).toString());
  }
  out[QStringLiteral("datasetCount")] = datasetCount;
  out[QStringLiteral("groupTitles")]  = groupTitles;

  const auto sources = project.value(QStringLiteral("sources")).toArray();
  out[QStringLiteral("sourceCount")] =
    sources.isEmpty() ? 1 : sources.size();  // legacy projects have no `sources` array
  return out;
}

/**
 * @brief Summarise the current ProjectModel for the `wouldDiscard` half of a dryRun reply.
 */
static QJsonObject summarizeCurrentProject()
{
  const auto& pm = DataModel::ProjectModel::instance();
  QJsonObject out;
  out[QStringLiteral("title")]        = pm.title();
  out[QStringLiteral("groupCount")]   = pm.groupCount();
  out[QStringLiteral("datasetCount")] = pm.datasetCount();
  out[QStringLiteral("sourceCount")]  = pm.sourceCount();
  out[QStringLiteral("filePath")]     = pm.jsonFilePath();

  QJsonArray groupTitles;
  for (const auto& g : pm.groups())
    if (groupTitles.size() < 32)
      groupTitles.append(g.title);

  out[QStringLiteral("groupTitles")] = groupTitles;
  return out;
}

/**
 * @brief Reset the active project to a blank slate.
 */
API::CommandResponse API::Handlers::ProjectHandler::fileNew(const QString& id,
                                                            const QJsonObject& params)
{
  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    QJsonObject wouldCreate;
    wouldCreate[QStringLiteral("title")]        = QStringLiteral("Untitled Project");
    wouldCreate[QStringLiteral("groupCount")]   = 0;
    wouldCreate[QStringLiteral("datasetCount")] = 0;
    wouldCreate[QStringLiteral("sourceCount")]  = 1;

    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldCreate")]  = wouldCreate;
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was reset. The current project (wouldDiscard) "
                     "would be replaced by a blank one. Confirm before re-issuing without "
                     "dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  DataModel::ProjectModel::instance().newJsonFile();
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  QJsonObject result;
  result[QStringLiteral("created")] = true;
  result[QStringLiteral("title")]   = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set project title
 */
API::CommandResponse API::Handlers::ProjectHandler::setTitle(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("title"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));
  }

  DataModel::ProjectModel::instance().setTitle(title);

  QJsonObject result;
  result[QStringLiteral("title")] = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Open project file
 */
API::CommandResponse API::Handlers::ProjectHandler::fileOpen(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("filePath"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: filePath"));
  }

  const QString file_path = params.value(QStringLiteral("filePath")).toString();
  if (file_path.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath cannot be empty"));
  }

  if (!API::isPathAllowed(file_path)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
  }

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    QFile f(file_path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
      return CommandResponse::makeError(
        id,
        ErrorCode::OperationFailed,
        QStringLiteral("DRY RUN: cannot read filePath '%1'").arg(file_path));

    QJsonParseError err{};
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject())
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("DRY RUN: filePath does not contain a valid project JSON object."));

    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("filePath")]     = file_path;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(doc.object());
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was loaded. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  const bool ok = DataModel::ProjectModel::instance().openJsonFile(file_path);
  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  if (!ok) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Failed to open project file (validation or I/O error)"));
  }

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);

  QJsonObject result;
  result[QStringLiteral("filePath")] = DataModel::ProjectModel::instance().jsonFilePath();
  result[QStringLiteral("title")]    = DataModel::ProjectModel::instance().title();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Save project
 */
API::CommandResponse API::Handlers::ProjectHandler::fileSave(const QString& id,
                                                             const QJsonObject& params)
{
  // When filePath is provided, save directly to that path (headless save-as)
  const QString explicit_path = params.value(QStringLiteral("filePath")).toString();

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(true);
  bool success = false;
  if (!explicit_path.isEmpty()) {
    if (!API::isPathAllowed(explicit_path)) {
      DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("filePath is not allowed"));
    }

    success = DataModel::ProjectModel::instance().apiSaveJsonFile(explicit_path);
  } else {
    const bool ask_path = params.value(QStringLiteral("askPath")).toBool(false);
    success             = DataModel::ProjectModel::instance().saveJsonFile(ask_path);
  }

  DataModel::ProjectModel::instance().setSuppressMessageBoxes(false);

  if (!success) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("Failed to save project"));
  }

  QJsonObject result;
  result[QStringLiteral("saved")]    = true;
  result[QStringLiteral("filePath")] = DataModel::ProjectModel::instance().jsonFilePath();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get project status
 */
API::CommandResponse API::Handlers::ProjectHandler::getStatus(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();

  QJsonObject result;
  result[QStringLiteral("title")]                      = project.title();
  result[QStringLiteral("filePath")]                   = project.jsonFilePath();
  result[QStringLiteral("modified")]                   = project.modified();
  result[QStringLiteral("groupCount")]                 = project.groupCount();
  result[QStringLiteral("datasetCount")]               = project.datasetCount();
  result[QStringLiteral("actionCount")]                = static_cast<int>(project.actions().size());
  result[QStringLiteral("containsCommercialFeatures")] = project.containsCommercialFeatures();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add group
 */
API::CommandResponse API::Handlers::ProjectHandler::groupAdd(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("title"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));
  }

  if (!params.contains(QStringLiteral("widgetType"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetType"));
  }

  const QString title = params.value(QStringLiteral("title")).toString();
  if (title.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));
  }

  const int widget_type = params.value(QStringLiteral("widgetType")).toInt();
  if (widget_type < 0 || widget_type > static_cast<int>(SerialStudio::Painter)) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid widgetType: must be 0..%1 "
                     "(see GroupWidget enum: 0=DataGrid, 1=Accelerometer, "
                     "2=Gyroscope, 3=GPS, 4=MultiPlot, 5=NoGroupWidget, "
                     "6=Plot3D, 7=ImageView, 8=Painter)")
        .arg(static_cast<int>(SerialStudio::Painter)));
  }

  const auto widget = static_cast<SerialStudio::GroupWidget>(widget_type);
  DataModel::ProjectModel::instance().addGroup(title, widget);

  QJsonObject result;
  result[QStringLiteral("title")]      = title;
  result[QStringLiteral("widgetType")] = widget_type;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete a group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDelete(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  // Capture target group + every dataset inside before mutation
  const auto& targetGroup = groups[groupId];
  QJsonArray childDatasets;
  for (const auto& d : targetGroup.datasets) {
    QJsonObject row;
    row[Keys::DatasetId]         = d.datasetId;
    row[Keys::UniqueId]          = d.uniqueId;
    row[QStringLiteral("title")] = d.title;
    childDatasets.append(row);
  }

  QJsonObject deleted;
  deleted[QStringLiteral("groupId")]      = groupId;
  deleted[QStringLiteral("title")]        = targetGroup.title;
  deleted[QStringLiteral("widget")]       = targetGroup.widget;
  deleted[QStringLiteral("datasetCount")] = childDatasets.size();
  deleted[QStringLiteral("datasets")]     = childDatasets;

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  // Groups above target compact by 1; uniqueId derives from group/dataset IDs, so they go stale.
  QJsonArray renumbered;
  for (const auto& g : groups) {
    if (g.groupId <= groupId)
      continue;

    QJsonObject row;
    row[QStringLiteral("oldGroupId")]   = g.groupId;
    row[QStringLiteral("newGroupId")]   = g.groupId - 1;
    row[QStringLiteral("title")]        = g.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(g.datasets.size());
    renumbered.append(row);
  }

  QString backupPath;
  qint64 preEpoch = 0;
  if (!isDryRun) {
    backupPath = Misc::BackupManager::instance().snapshot(QStringLiteral("pre-groupDelete"));
    preEpoch   = captureProjectEpoch();
    project.deleteGroup(groupId);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("deleted")]    = deleted;
  result[QStringLiteral("renumbered")] = renumbered;
  if (!backupPath.isEmpty())
    result[QStringLiteral("backupPath")] = backupPath;

  QJsonArray warnings;
  if (isDryRun)
    warnings.append(QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows groupId values that WOULD shift; every dataset in those "
      "groups would have its uniqueId invalidated."));

  else if (!renumbered.isEmpty())
    warnings.append(QStringLiteral(
      "groupId values shifted after deletion; uniqueIds of every dataset in renumbered "
      "groups are now stale -- re-read project state before further mutations."));

  if (!isDryRun && !backupPath.isEmpty())
    warnings.append(QStringLiteral(
      "Pre-mutation snapshot saved at backupPath; pass it to assistant.restore to undo."));

  if (!warnings.isEmpty())
    result[QStringLiteral("warnings")] = warnings;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate a group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::groupDuplicate(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  project.duplicateGroup(groupId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the widget string matching a one-of DatasetOption bit.
 */
QString API::Handlers::ProjectHandler::widgetForDatasetOptions(int options)
{
  if (options & SerialStudio::DatasetCompass)
    return QStringLiteral("compass");

  if (options & SerialStudio::DatasetMeter)
    return QStringLiteral("meter");

  if (options & SerialStudio::DatasetGauge)
    return QStringLiteral("gauge");

  if (options & SerialStudio::DatasetBar)
    return QStringLiteral("bar");

  return QString();
}

/**
 * @brief Sets plt/fft/led/waterfall/widget on @p d from a DatasetOption bitfield.
 */
void API::Handlers::ProjectHandler::applyDatasetVisualizationFlags(DataModel::Dataset& d,
                                                                   int options)
{
  if (options & SerialStudio::DatasetPlot)
    d.plt = true;

  if (options & SerialStudio::DatasetFFT)
    d.fft = true;

  if (options & SerialStudio::DatasetLED)
    d.led = true;

  if (options & SerialStudio::DatasetWaterfall)
    d.waterfall = true;

  const QString widget = widgetForDatasetOptions(options);
  if (!widget.isEmpty())
    d.widget = widget;
}

/**
 * @brief Add a dataset to a specific group by id
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetAdd(const QString& id,
                                                               const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("options")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: options"));

  const int groupId = params.value(QStringLiteral("groupId")).toInt();
  const int options = params.value(QStringLiteral("options")).toInt();
  if (options < 0 || options > 0b11111111)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid options: must be 0-255 (bit flags)"));

  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  // Headline flag drives the auto-generated title; remaining bits are applied below
  SerialStudio::DatasetOption headline = SerialStudio::DatasetGeneric;
  for (const auto cand : {SerialStudio::DatasetPlot,
                          SerialStudio::DatasetFFT,
                          SerialStudio::DatasetBar,
                          SerialStudio::DatasetGauge,
                          SerialStudio::DatasetCompass,
                          SerialStudio::DatasetLED,
                          SerialStudio::DatasetWaterfall,
                          SerialStudio::DatasetMeter}) {
    if (options & cand) {
      headline = cand;
      break;
    }
  }

  project.setSelectedGroup(groups[groupId]);
  project.addDataset(headline);

  // Apply any remaining bits on top of the headline-seeded dataset
  const int remaining = options & ~static_cast<int>(headline);
  const auto& post    = project.groups();
  const int newIndex  = static_cast<int>(post[groupId].datasets.size()) - 1;
  if (remaining != 0 && newIndex >= 0) {
    DataModel::Dataset d = post[groupId].datasets[newIndex];
    applyDatasetVisualizationFlags(d, remaining);
    project.updateDataset(groupId, newIndex, d, /*rebuildTree=*/true);
  }

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("options")] = options;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add several datasets to a group in one call (bulk creation).
 */
/**
 * @brief Picks the headline visualization flag from an options bitfield.
 */
static SerialStudio::DatasetOption pickHeadlineDatasetOption(int options)
{
  for (const auto cand : {SerialStudio::DatasetPlot,
                          SerialStudio::DatasetFFT,
                          SerialStudio::DatasetBar,
                          SerialStudio::DatasetGauge,
                          SerialStudio::DatasetCompass,
                          SerialStudio::DatasetLED,
                          SerialStudio::DatasetWaterfall,
                          SerialStudio::DatasetMeter}) {
    if (options & cand)
      return cand;
  }
  return SerialStudio::DatasetGeneric;
}

/**
 * @brief Bulk-creates N datasets in one call with optional title/index patterns.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetAddMany(const QString& id,
                                                                   const QJsonObject& params)
{
  constexpr int kMaxAddManyCount = 1024;

  for (const auto& key :
       {QStringLiteral("groupId"), QStringLiteral("count"), QStringLiteral("options")}) {
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));
  }

  const int groupId = params.value(QStringLiteral("groupId")).toInt();
  const int count   = params.value(QStringLiteral("count")).toInt();
  const int options = params.value(QStringLiteral("options")).toInt();

  if (count <= 0 || count > kMaxAddManyCount)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid count: must be 1..%1 (got %2)")
        .arg(QString::number(kMaxAddManyCount), QString::number(count)));

  if (options < 0 || options > 0b111111111)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid options: must be 0-511 (bit flags)"));

  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const QString titlePattern = params.value(QStringLiteral("titlePattern")).toString();
  const int startNumber      = params.contains(QStringLiteral("startNumber"))
                               ? params.value(QStringLiteral("startNumber")).toInt()
                               : 1;
  const int startIndex       = params.contains(QStringLiteral("startIndex"))
                               ? params.value(QStringLiteral("startIndex")).toInt()
                               : -1;

  if (startIndex < -1)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid startIndex: must be -1 (auto), 0, or 1+"));

  const auto headline     = pickHeadlineDatasetOption(options);
  const int remainingBits = options & ~static_cast<int>(headline);

  // Suspend autosave so the whole burst lands as a single save at the end
  project.setAutoSaveSuspended(true);
  project.setSelectedGroup(groups[groupId]);

  QJsonArray created;
  for (int i = 0; i < count; ++i) {
    project.addDataset(headline);

    const auto& post   = project.groups();
    const int newIndex = static_cast<int>(post[groupId].datasets.size()) - 1;
    if (newIndex < 0)
      continue;

    DataModel::Dataset d = post[groupId].datasets[newIndex];
    if (remainingBits != 0)
      applyDatasetVisualizationFlags(d, remainingBits);

    if (!titlePattern.isEmpty()) {
      QString title = titlePattern;
      title.replace(QStringLiteral("{n}"), QString::number(startNumber + i));
      title.replace(QStringLiteral("{i}"), QString::number(i));
      d.title = title;
    }

    if (startIndex >= 0)
      d.index = startIndex + i;

    project.updateDataset(groupId, newIndex, d, /*rebuildTree=*/true);

    QJsonObject entry;
    entry[QStringLiteral("groupId")] = groupId;
    entry[Keys::DatasetId]           = d.datasetId;
    entry[Keys::Title]               = d.title;
    entry[QStringLiteral("index")]   = d.index;
    entry[Keys::UniqueId]            = d.uniqueId;
    created.append(entry);
  }

  project.setAutoSaveSuspended(false);
  project.flushAutoSave();

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("count")]   = created.size();
  result[QStringLiteral("created")] = created;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Build the warnings array for a dataset-delete response.
 */
static QJsonArray buildDatasetDeleteWarnings(int groupId,
                                             bool isDryRun,
                                             const QJsonArray& renumbered,
                                             const QString& backupPath)
{
  QJsonArray warnings;
  if (isDryRun)
    warnings.append(
      QStringLiteral(
        "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
        "renumbered[] array shows datasetId/uniqueId values that WOULD shift in groupId=%1.")
        .arg(groupId));

  else if (!renumbered.isEmpty())
    warnings.append(
      QStringLiteral(
        "datasetId values in groupId=%1 were renumbered; cached uniqueIds for the affected "
        "datasets are now stale -- re-read project state before further mutations.")
        .arg(groupId));

  if (!isDryRun && !backupPath.isEmpty())
    warnings.append(QStringLiteral(
      "Pre-mutation snapshot saved at backupPath; pass it to assistant.restore to undo."));

  return warnings;
}

/**
 * @brief Delete a dataset by id, returning the deleted entity + any renumbered peers.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDelete(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& groups  = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  const auto& targetGroup   = groups[groupId];
  const auto& targetDataset = targetGroup.datasets[datasetId];

  QJsonObject deleted;
  deleted[QStringLiteral("groupId")]    = groupId;
  deleted[QStringLiteral("groupTitle")] = targetGroup.title;
  deleted[Keys::DatasetId]              = datasetId;
  deleted[Keys::UniqueId]               = targetDataset.uniqueId;
  deleted[QStringLiteral("title")]      = targetDataset.title;
  if (!targetDataset.units.isEmpty())
    deleted[QStringLiteral("units")] = targetDataset.units;

  if (!targetDataset.transformCode.isEmpty()) {
    deleted[QStringLiteral("hadTransform")]       = true;
    deleted[QStringLiteral("transformByteCount")] = targetDataset.transformCode.size();
  }

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);

  // Peers above target compact down by 1 (matches ProjectModel::deleteDataset semantics).
  QJsonArray renumbered;
  for (const auto& d : targetGroup.datasets) {
    if (d.datasetId <= datasetId)
      continue;

    QJsonObject row;
    row[QStringLiteral("groupId")]      = groupId;
    row[QStringLiteral("oldDatasetId")] = d.datasetId;
    row[QStringLiteral("newDatasetId")] = d.datasetId - 1;
    row[Keys::UniqueId]                 = d.uniqueId;
    row[QStringLiteral("title")]        = d.title;
    renumbered.append(row);
  }

  QString backupPath;
  qint64 preEpoch = 0;
  if (!isDryRun) {
    backupPath = Misc::BackupManager::instance().snapshot(QStringLiteral("pre-datasetDelete"));
    preEpoch   = captureProjectEpoch();
    project.deleteDataset(groupId, datasetId);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("deleted")]    = deleted;
  result[QStringLiteral("renumbered")] = renumbered;
  if (!backupPath.isEmpty())
    result[QStringLiteral("backupPath")] = backupPath;

  const auto warnings = buildDatasetDeleteWarnings(groupId, isDryRun, renumbered, backupPath);
  if (!warnings.isEmpty())
    result[QStringLiteral("warnings")] = warnings;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate a dataset by id
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetDuplicate(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& groups  = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  project.duplicateDataset(groupId, datasetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle a dataset option by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetOption(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  if (!params.contains(QStringLiteral("option")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: option"));

  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const bool enabled  = params.value(QStringLiteral("enabled")).toBool();

  // option accepts a string slug ('plot', 'fft', ...) or the DatasetOption integer
  int option                  = 0;
  const QJsonValue optionJson = params.value(QStringLiteral("option"));
  if (optionJson.isString()) {
    option = API::EnumLabels::datasetOptionFromSlug(optionJson.toString());
    if (option == 0)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Unknown option slug '%1'. Use one of: plot, fft, bar, gauge, "
                       "compass, led, waterfall.")
          .arg(optionJson.toString()));
  } else {
    option = optionJson.toInt();
  }

  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  // changeDatasetOption() reads m_selectedDataset; select first.
  project.setSelectedDataset(groups[groupId].datasets[datasetId]);
  project.changeDatasetOption(static_cast<SerialStudio::DatasetOption>(option), enabled);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("option")]     = option;
  result[QStringLiteral("optionSlug")] = API::EnumLabels::datasetOptionSlug(option);
  result[QStringLiteral("enabled")]    = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Apply a bitmask of DatasetOption flags in one call.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetOptions(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  if (!params.contains(QStringLiteral("options")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: options"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  // options accepts an integer bitflag OR an array of slug strings
  int options                  = 0;
  const QJsonValue optionsJson = params.value(QStringLiteral("options"));
  if (optionsJson.isArray()) {
    QStringList slugs;
    for (const auto& v : optionsJson.toArray())
      slugs.append(v.toString());

    options = API::EnumLabels::datasetOptionsSlugsToBits(slugs);
  } else {
    options = optionsJson.toInt();
  }

  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (datasetId < 0 || static_cast<size_t>(datasetId) >= groups[groupId].datasets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found: %1 in group %2")
                                        .arg(QString::number(datasetId), QString::number(groupId)));

  DataModel::Dataset d = groups[groupId].datasets[datasetId];
  d.plt                = (options & SerialStudio::DatasetPlot) != 0;
  d.fft                = (options & SerialStudio::DatasetFFT) != 0;
  d.led                = (options & SerialStudio::DatasetLED) != 0;
  d.waterfall          = (options & SerialStudio::DatasetWaterfall) != 0;

  // widget string is a one-of group; clear if caller did not request bar/gauge/compass
  const QString chosen = widgetForDatasetOptions(options);
  const bool wasOneOf  = d.widget == QStringLiteral("bar") || d.widget == QStringLiteral("gauge")
                     || d.widget == QStringLiteral("compass");
  if (!chosen.isEmpty())
    d.widget = chosen;
  else if (wasOneOf)
    d.widget = QString();

  project.updateDataset(groupId, datasetId, d, /*rebuildTree=*/true);

  QJsonArray slugs;
  for (const auto& s : API::EnumLabels::datasetOptionsBitsToSlugs(options))
    slugs.append(s);

  QJsonObject result;
  result[QStringLiteral("groupId")]      = groupId;
  result[Keys::DatasetId]                = datasetId;
  result[QStringLiteral("options")]      = options;
  result[QStringLiteral("optionsSlugs")] = slugs;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Dataset field setters (v3.3)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggle the @c virtual_ flag on the dataset identified by (groupId, datasetId).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetVirtual(const QString& id,
                                                                      const QJsonObject& params)
{
  // Validate required parameters
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QString(Keys::Virtual),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const bool isVirt   = params.value(Keys::Virtual).toBool();

  // Locate the group by logical id, then the dataset within it
  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  // Apply the flag -- updateDataset validates, emits signals, and rebuilds the tree
  DataModel::Dataset updated = *dit;
  updated.virtual_           = isVirt;
  pm.updateDataset(groupId, datasetId, updated, true);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[Keys::Virtual]             = isVirt;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the per-dataset transform code (Lua or JS; language matches the dataset's owning
 * source).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetTransformCode(
  const QString& id, const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId),
    QString(Keys::DatasetId),
    QStringLiteral("code"),
  };

  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const QString code  = params.value(QStringLiteral("code")).toString();

  // Locate the group by logical id, then the dataset within it
  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.transformCode      = code;

  bool languageInherited = false;
  if (params.contains(QStringLiteral("language"))) {
    const int lang = params.value(QStringLiteral("language")).toInt();
    if (lang != SerialStudio::JavaScript && lang != SerialStudio::Lua)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

    updated.transformLanguage = lang;
  } else if (!code.isEmpty() && updated.transformLanguage < 0) {
    // No explicit language with non-empty code: resolve immediately from owning source
    const auto& srcs = pm.sources();
    const auto sit   = std::find_if(
      srcs.begin(), srcs.end(), [&](const auto& s) { return s.sourceId == updated.sourceId; });
    updated.transformLanguage = (sit != srcs.end()) ? sit->frameParserLanguage : 0;
    languageInherited         = true;
  }

  pm.updateDataset(groupId, datasetId, updated, false);

  QJsonObject result;
  result[Keys::GroupId]                = groupId;
  result[Keys::DatasetId]              = datasetId;
  result[QStringLiteral("codeLength")] = code.size();
  result[QStringLiteral("language")]   = updated.transformLanguage;
  result[QStringLiteral("updated")]    = true;

  if (languageInherited) {
    result[QStringLiteral("languageInherited")] = true;
    result[QStringLiteral("inheritNotice")] =
      QStringLiteral("language was not provided; inherited from source "
                     "frameParserLanguage (%1). Pass language explicitly "
                     "to silence this notice.")
        .arg(updated.transformLanguage == 1 ? QStringLiteral("Lua") : QStringLiteral("JavaScript"));
  }

  if (!code.isEmpty() && updated.transformLanguage != -1) {
    const auto warning = detectLanguageMismatch(code, updated.transformLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  if (!code.isEmpty() && !updated.virtual_ && updated.index <= 0)
    result[QStringLiteral("hint")] =
      QStringLiteral("transformCode set but virtual=false and index<=0; if this "
                     "dataset has no slot in the parser output array, set "
                     "virtual=true via project.dataset.update{virtual:true} "
                     "or the dataset will read empty channel data.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the dataset's alarm bands as a JSON array.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetAlarmBands(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{QString(Keys::GroupId), QString(Keys::DatasetId)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  QJsonArray bands;
  for (const auto& b : dit->alarmBands)
    bands.append(DataModel::serialize(b));

  QJsonObject result;
  result[Keys::GroupId]              = groupId;
  result[Keys::DatasetId]            = datasetId;
  result[Keys::AlarmBands]           = bands;
  result[QStringLiteral("count")]    = bands.size();
  result[QStringLiteral("rangeMin")] = qMin(dit->wgtMin, dit->wgtMax);
  result[QStringLiteral("rangeMax")] = qMax(dit->wgtMin, dit->wgtMax);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Atomic write of the full alarmBands array onto a dataset.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetSetAlarmBands(const QString& id,
                                                                         const QJsonObject& params)
{
  const QStringList required{
    QString(Keys::GroupId), QString(Keys::DatasetId), QString(Keys::AlarmBands)};
  for (const auto& key : required)
    if (!params.contains(key))
      return CommandResponse::makeError(
        id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: %1").arg(key));

  const int groupId   = params.value(Keys::GroupId).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  const auto arr      = params.value(Keys::AlarmBands).toArray();

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  const auto git     = std::find_if(
    groups.begin(), groups.end(), [groupId](const auto& g) { return g.groupId == groupId; });

  if (git == groups.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& datasets = git->datasets;
  const auto dit       = std::find_if(datasets.begin(), datasets.end(), [datasetId](const auto& d) {
    return d.datasetId == datasetId;
  });

  if (dit == datasets.end())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Dataset id not found in group: %1/%2")
                                        .arg(QString::number(groupId), QString::number(datasetId)));

  DataModel::Dataset updated = *dit;
  updated.alarmBands.clear();
  updated.alarmBands.reserve(arr.size());

  int dropped = 0;
  for (const auto& v : arr) {
    DataModel::AlarmBand b;
    if (DataModel::read(b, v.toObject()))
      updated.alarmBands.push_back(std::move(b));
    else
      ++dropped;
  }

  pm.updateDataset(groupId, datasetId, updated, false);

  QJsonObject result;
  result[Keys::GroupId]             = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("count")]   = static_cast<int>(updated.alarmBands.size());
  result[QStringLiteral("updated")] = true;
  if (dropped > 0)
    result[QStringLiteral("droppedInvalid")] = dropped;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Add action
 */
API::CommandResponse API::Handlers::ProjectHandler::actionAdd(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  DataModel::ProjectModel::instance().addAction();

  QJsonObject result;
  result[QStringLiteral("added")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an action by id
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDelete(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId  = params.value(QStringLiteral("actionId")).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& actions = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.deleteAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")] = actionId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an action by id
 */
API::CommandResponse API::Handlers::ProjectHandler::actionDuplicate(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId  = params.value(QStringLiteral("actionId")).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& actions = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  project.duplicateAction(actionId);

  QJsonObject result;
  result[QStringLiteral("actionId")]   = actionId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Output widget commands
//--------------------------------------------------------------------------------------------------

/**
 * @brief Add an output widget to the specified group.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetAdd(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const int type     = params.value(QStringLiteral("type")).toInt(0);
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  // addOutputControl() reads m_selectedGroup; select the target first.
  project.setSelectedGroup(groups[groupId]);
  project.addOutputControl(static_cast<SerialStudio::OutputWidgetType>(
    qBound(0, type, static_cast<int>(SerialStudio::OutputKnob))));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("type")]    = type;
  result[QStringLiteral("added")]   = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Delete an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDelete(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId = params.value(QStringLiteral("widgetId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.deleteOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("deleted")]  = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Duplicate an output widget by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDuplicate(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId = params.value(QStringLiteral("widgetId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  if (widgetId < 0 || static_cast<size_t>(widgetId) >= groups[groupId].outputWidgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  project.duplicateOutputWidget(groupId, widgetId);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("widgetId")]   = widgetId;
  result[QStringLiteral("duplicated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the project configuration of one output widget.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetGet(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId = params.value(QStringLiteral("widgetId")).toInt();
  const auto& groups = DataModel::ProjectModel::instance().groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  const auto& widgets = groups[groupId].outputWidgets;
  if (widgetId < 0 || static_cast<size_t>(widgetId) >= widgets.size())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Widget id not found: %1 in group %2")
                                        .arg(QString::number(widgetId), QString::number(groupId)));

  const auto& w = widgets[widgetId];
  QJsonObject result;
  result[QStringLiteral("groupId")]          = groupId;
  result[QStringLiteral("widgetId")]         = w.widgetId;
  result[QStringLiteral("type")]             = static_cast<int>(w.type);
  result[QStringLiteral("title")]            = w.title;
  result[QStringLiteral("icon")]             = w.icon;
  result[QStringLiteral("monoIcon")]         = w.monoIcon;
  result[QStringLiteral("minValue")]         = w.minValue;
  result[QStringLiteral("maxValue")]         = w.maxValue;
  result[QStringLiteral("stepSize")]         = w.stepSize;
  result[QStringLiteral("initialValue")]     = w.initialValue;
  result[Keys::SourceId]                     = w.sourceId;
  result[QStringLiteral("txEncoding")]       = w.txEncoding;
  result[QStringLiteral("transmitFunction")] = w.transmitFunction;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set frame parser code for a source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  // Validate required "code" parameter
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  // Resolve code and sourceId, validate bounds
  const QString code = params.value(QStringLiteral("code")).toString();
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  auto& model        = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  // Optional language flip -- validate under the new engine and roll back if invalid.
  const bool hasLanguage = params.contains(QStringLiteral("language"));
  int savedLanguage      = 0;
  if (hasLanguage) {
    const int language = params.value(QStringLiteral("language")).toInt();
    if (language != SerialStudio::JavaScript && language != SerialStudio::Lua)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

    // Snapshot prior language for rollback on validation failure.
    savedLanguage = model.frameParserLanguage(sourceId);

    // Flip the language so the engine dispatch picks the right implementation.
    model.updateSourceFrameParserLanguage(sourceId, language);

    // Suppress modal dialogs during headless API-driven validation.
    auto& parser            = DataModel::FrameParser::instance();
    const bool prevSuppress = model.suppressMessageBoxes();
    model.setSuppressMessageBoxes(true);
    parser.setSuppressMessageBoxes(true);

    // Validate + load the script under the new engine.
    const bool ok = parser.loadScript(sourceId, code, false);

    parser.setSuppressMessageBoxes(prevSuppress);
    model.setSuppressMessageBoxes(prevSuppress);

    // Roll back the language flip on validation failure.
    if (!ok) {
      model.updateSourceFrameParserLanguage(sourceId, savedLanguage);
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Script engine rejected the parser code (check logs)"));
    }
  }

  // Persist: source 0 -> setFrameParserCode; source >0 -> updateSourceFrameParser.
  if (sourceId == 0)
    model.setFrameParserCode(code);
  else
    model.updateSourceFrameParser(sourceId, code);

  const int effectiveLanguage = model.frameParserLanguage(sourceId);

  QJsonObject result;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("codeLength")] = code.length();
  result[QStringLiteral("language")]   = effectiveLanguage;

  if (!code.isEmpty()) {
    const auto warning = detectLanguageMismatch(code, effectiveLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get frame parser code for a source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  const auto& model  = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  const QString code =
    sourceId == 0 ? model.frameParserCode() : model.sources()[sourceId].frameParserCode;

  QJsonObject result;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("code")]       = code;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the scripting language for a frame parser source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  // Validate required "language" parameter
  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  // Resolve sourceId (logical, default 0)
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  // Validate language value against the SerialStudio::ScriptLanguage enum
  const int language = params.value(QStringLiteral("language")).toInt();
  if (language != SerialStudio::JavaScript && language != SerialStudio::Lua)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

  // Locate the source by logical ID (not by vector index)
  auto& model         = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  // Reject unknown source IDs
  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  model.updateSourceFrameParserLanguage(sourceId, language);

  // Reset parser code to the default template for the new language.
  DataModel::FrameParser::instance().loadDefaultTemplate(sourceId, true);

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = language;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the scripting language for a frame parser source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  // Resolve sourceId (logical, default 0)
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  // Verify the source exists before reporting a language
  const auto& model   = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  // Reject unknown source IDs
  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = it->frameParserLanguage;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all groups with basic info
 */
API::CommandResponse API::Handlers::ProjectHandler::groupsList(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray groups_array;
  for (const auto& group : groups) {
    QJsonObject obj = DataModel::serialize(group);

    obj[QStringLiteral("datasetCount")] = static_cast<int>(group.datasets.size());

    // Dataset titles + units to disambiguate groups beyond "groupId N"
    QJsonArray ds_summary;
    for (const auto& ds : group.datasets) {
      QJsonObject d;
      d[Keys::DatasetId]         = ds.datasetId;
      d[Keys::UniqueId]          = ds.uniqueId;
      d[QStringLiteral("index")] = ds.index;
      d[QStringLiteral("title")] = ds.title;
      if (!ds.units.isEmpty())
        d[QStringLiteral("units")] = ds.units;

      d[QStringLiteral("enabledOptions")] = datasetOptionsBitflag(ds);

      QJsonArray ds_compat;
      appendDatasetWidgetTypes(ds, ds_compat);
      d[QStringLiteral("enabledWidgetTypes")] = ds_compat;

      ds_summary.append(d);
    }
    obj[QStringLiteral("datasetSummary")] = ds_summary;

    // Precomputed DashboardWidget enums accepted by workspace-add for this group
    QJsonArray compat;
    const auto group_w = static_cast<int>(SerialStudio::getDashboardWidget(group));
    if (group_w != SerialStudio::DashboardNoWidget)
      compat.append(group_w);

    for (const auto& ds : group.datasets)
      appendDatasetWidgetTypes(ds, compat);

    obj[QStringLiteral("compatibleWidgetTypes")] = compat;

    QJsonArray compatSlugs;
    for (const auto& v : compat) {
      const auto slug = API::EnumLabels::dashboardWidgetSlug(v.toInt());
      if (!compatSlugs.contains(slug))
        compatSlugs.append(slug);
    }
    obj[QStringLiteral("compatibleWidgetTypeSlugs")] = compatSlugs;

    groups_array.append(obj);
  }

  QString summary;
  if (groups.empty()) {
    summary = QStringLiteral("No groups configured.");
  } else {
    QStringList names;
    for (const auto& g : groups) {
      const auto widgetStr = g.widget.simplified();
      names.append(
        QStringLiteral("\"%1\"%2")
          .arg(g.title)
          .arg(widgetStr.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(widgetStr)));
    }
    summary = QStringLiteral("%1 group%2: %3.")
                .arg(groups.size())
                .arg(groups.size() == 1 ? QString() : QStringLiteral("s"))
                .arg(names.join(QStringLiteral(", ")));
  }

  int totalDatasets = 0;
  for (const auto& g : groups)
    totalDatasets += static_cast<int>(g.datasets.size());

  QJsonObject result;
  result[QStringLiteral("_summary")]   = summary;
  result[QStringLiteral("groups")]     = groups_array;
  result[QStringLiteral("groupCount")] = static_cast<int>(groups.size());
  if (groups.size() >= 5 || totalDatasets >= 10)
    result[QStringLiteral("_hint")] =
      QStringLiteral("%1 groups / %2 datasets present. For bulk edits across many "
                     "groups/datasets use project.batch instead of looping per-item updates "
                     "(see meta.describeCommand{name:\"project.batch\"}). For creating arrays "
                     "of similar datasets, use project.dataset.addMany.")
        .arg(groups.size())
        .arg(totalDatasets);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all datasets across all groups
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetsList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray datasets_array;
  int total_datasets = 0;

  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      datasets_array.append(buildDatasetObject(dataset, group));
      ++total_datasets;
    }
  }

  QString summary;
  if (total_datasets == 0) {
    summary = QStringLiteral("No datasets configured.");
  } else {
    summary = QStringLiteral("%1 datasets across %2 group%3.")
                .arg(total_datasets)
                .arg(groups.size())
                .arg(groups.size() == 1 ? QString() : QStringLiteral("s"));
  }

  QJsonObject result;
  result[QStringLiteral("_summary")]     = summary;
  result[QStringLiteral("datasets")]     = datasets_array;
  result[QStringLiteral("datasetCount")] = total_datasets;
  if (total_datasets >= 10)
    result[QStringLiteral("_hint")] =
      QStringLiteral("Bulk edits across %1 datasets: use project.batch (rename/retitle/reindex/"
                     "update many at once) or project.dataset.addMany (create N similar). "
                     "Looping single project.dataset.update calls costs N round-trips and N "
                     "autosave-debounce restarts. See meta.describeCommand{name:"
                     "\"project.batch\"} for the {ops:[{command,params},...]} shape.")
        .arg(total_datasets);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Find a dataset by its uniqueId across all groups.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByUniqueId(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(Keys::UniqueId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: uniqueId"));

  const int uniqueId = params.value(Keys::UniqueId).toInt();
  const auto& groups = DataModel::ProjectModel::instance().groups();

  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets)
      if (dataset.uniqueId == uniqueId)
        return CommandResponse::makeSuccess(id, buildDatasetObject(dataset, group));
  }

  return CommandResponse::makeError(
    id, ErrorCode::InvalidParam, QStringLiteral("Dataset not found for uniqueId %1").arg(uniqueId));
}

/**
 * @brief Find a dataset by title (optionally narrowed by sourceId / groupId).
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByTitle(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(Keys::Title))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: title"));

  const QString title        = params.value(Keys::Title).toString();
  const bool hasSourceFilter = params.contains(Keys::SourceId);
  const bool hasGroupFilter  = params.contains(Keys::GroupId);
  const int filterSourceId   = hasSourceFilter ? params.value(Keys::SourceId).toInt() : 0;
  const int filterGroupId    = hasGroupFilter ? params.value(Keys::GroupId).toInt() : 0;

  if (title.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("title cannot be empty"));

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray matches;
  for (const auto& group : groups) {
    if (hasGroupFilter && group.groupId != filterGroupId)
      continue;

    for (const auto& dataset : group.datasets) {
      if (hasSourceFilter && dataset.sourceId != filterSourceId)
        continue;

      if (dataset.title == title)
        matches.append(buildDatasetObject(dataset, group));
    }
  }

  if (matches.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("No dataset matched title '%1'").arg(title));

  if (matches.size() > 1) {
    QJsonObject extra;
    extra[QStringLiteral("matches")] = matches;
    extra[QStringLiteral("hint")] =
      QStringLiteral("Multiple datasets match this title. Pass sourceId or groupId to "
                     "disambiguate, or call project.dataset.getByPath.");
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Ambiguous title '%1' (%2 matches)")
                                        .arg(title, QString::number(matches.size())),
                                      extra);
  }

  return CommandResponse::makeSuccess(id, matches.first().toObject());
}

/**
 * @brief Find a dataset by 'Group/Dataset' or 'Source/Group/Dataset' path.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetByPath(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("path")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: path"));

  const QString path  = params.value(QStringLiteral("path")).toString();
  const auto segments = path.split(QChar('/'), Qt::SkipEmptyParts);

  if (segments.size() != 2 && segments.size() != 3)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("path must be 'Group/Dataset' or 'Source/Group/Dataset'"));

  const QString sourceTitle  = segments.size() == 3 ? segments.at(0) : QString();
  const QString groupTitle   = segments.size() == 3 ? segments.at(1) : segments.at(0);
  const QString datasetTitle = segments.last();

  const auto& pm      = DataModel::ProjectModel::instance();
  const auto& sources = pm.sources();
  const auto& groups  = pm.groups();

  int sourceFilterId = -1;
  if (!sourceTitle.isEmpty()) {
    for (const auto& src : sources)
      if (src.title == sourceTitle) {
        sourceFilterId = src.sourceId;
        break;
      }

    if (sourceFilterId < 0)
      return CommandResponse::makeError(
        id, ErrorCode::InvalidParam, QStringLiteral("Source not found: '%1'").arg(sourceTitle));
  }

  for (const auto& group : groups) {
    if (group.title != groupTitle)
      continue;

    for (const auto& dataset : group.datasets) {
      if (sourceFilterId >= 0 && dataset.sourceId != sourceFilterId)
        continue;

      if (dataset.title == datasetTitle)
        return CommandResponse::makeSuccess(id, buildDatasetObject(dataset, group));
    }
  }

  return CommandResponse::makeError(
    id, ErrorCode::InvalidParam, QStringLiteral("No dataset matched path '%1'").arg(path));
}

/**
 * @brief Compute the new ordinal for an item after a list move; mirrors std::vector reorder.
 */
static int projectedAfterMove(int oldIndex, int from, int to)
{
  if (oldIndex == from)
    return to;

  if (from < to && oldIndex > from && oldIndex <= to)
    return oldIndex - 1;

  if (from > to && oldIndex >= to && oldIndex < from)
    return oldIndex + 1;

  return oldIndex;
}

/**
 * @brief Moves a dataset to a new position within its group.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetMove(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(Keys::UniqueId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: uniqueId"));

  if (!params.contains(QStringLiteral("newPosition")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newPosition"));

  const int uniqueId    = params.value(Keys::UniqueId).toInt();
  const int newPosition = params.value(QStringLiteral("newPosition")).toInt();
  const bool isDryRun   = params.value(QStringLiteral("dryRun")).toBool(false);

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();

  const DataModel::Group* matchGroup     = nullptr;
  const DataModel::Dataset* matchDataset = nullptr;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (dataset.uniqueId == uniqueId) {
        matchGroup   = &group;
        matchDataset = &dataset;
        break;
      }
    }
    if (matchDataset)
      break;
  }

  if (!matchDataset)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Dataset not found for uniqueId %1").arg(uniqueId));

  const int oldPosition = matchDataset->datasetId;
  const int clampedNewId =
    std::clamp(newPosition, 0, static_cast<int>(matchGroup->datasets.size()) - 1);

  // Compute the renumbering map; uniqueId stays stable across reorders.
  QJsonArray renumbered;
  for (const auto& d : matchGroup->datasets) {
    const int newId = projectedAfterMove(d.datasetId, oldPosition, clampedNewId);
    if (newId == d.datasetId)
      continue;

    QJsonObject row;
    row[QStringLiteral("groupId")]      = matchGroup->groupId;
    row[QStringLiteral("oldDatasetId")] = d.datasetId;
    row[QStringLiteral("newDatasetId")] = newId;
    row[Keys::UniqueId]                 = d.uniqueId;
    row[QStringLiteral("title")]        = d.title;
    renumbered.append(row);
  }

  qint64 preEpoch = 0;
  if (!isDryRun) {
    preEpoch = captureProjectEpoch();
    pm.moveDataset(matchGroup->groupId, oldPosition, newPosition);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[Keys::UniqueId]                = uniqueId;
  result[Keys::GroupId]                 = matchGroup->groupId;
  result[QStringLiteral("oldPosition")] = oldPosition;
  result[QStringLiteral("newPosition")] = clampedNewId;
  result[QStringLiteral("moved")]       = !isDryRun;
  result[QStringLiteral("renumbered")]  = renumbered;

  QString warning;
  if (isDryRun)
    warning = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows the new datasetId for each affected dataset (uniqueId "
      "is stable across reorders).");

  else
    warning = QStringLiteral(
      "Dataset reorder renumbers datasetId within the group. uniqueId stays stable "
      "across reorders, so workspace refs and xAxisId references survive untouched.");

  result[QStringLiteral("warning")] = warning;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Moves a group to a new position in the project.
 */
API::CommandResponse API::Handlers::ProjectHandler::groupMove(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(Keys::GroupId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("newPosition")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: newPosition"));

  const int groupId     = params.value(Keys::GroupId).toInt();
  const int newPosition = params.value(QStringLiteral("newPosition")).toInt();
  const bool isDryRun   = params.value(QStringLiteral("dryRun")).toBool(false);

  auto& pm           = DataModel::ProjectModel::instance();
  const auto& groups = pm.groups();
  if (groupId < 0 || groupId >= static_cast<int>(groups.size()))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id out of range: %1").arg(groupId));

  const int clampedNew = std::clamp(newPosition, 0, static_cast<int>(groups.size()) - 1);

  // Compute renumbering map.
  QJsonArray renumbered;
  for (const auto& g : groups) {
    const int newId = projectedAfterMove(g.groupId, groupId, clampedNew);
    if (newId == g.groupId)
      continue;

    QJsonObject row;
    row[QStringLiteral("oldGroupId")]   = g.groupId;
    row[QStringLiteral("newGroupId")]   = newId;
    row[QStringLiteral("title")]        = g.title;
    row[QStringLiteral("datasetCount")] = static_cast<int>(g.datasets.size());
    renumbered.append(row);
  }

  qint64 preEpoch = 0;
  if (!isDryRun) {
    preEpoch = captureProjectEpoch();
    pm.moveGroup(groupId, newPosition);
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("oldPosition")] = groupId;
  result[QStringLiteral("newPosition")] = clampedNew;
  result[QStringLiteral("moved")]       = !isDryRun;
  result[QStringLiteral("renumbered")]  = renumbered;

  QString warning;
  if (isDryRun)
    warning = QStringLiteral(
      "DRY RUN: no changes were written. Re-call without dryRun:true to commit. The "
      "renumbered[] array shows the new groupId for each affected group; uniqueIds stay "
      "stable across reorders.");

  else
    warning = QStringLiteral(
      "Group reorder renumbers groupId. Dataset uniqueIds and Group.uniqueId stay stable, "
      "so workspace refs and xAxisId references survive untouched.");

  result[QStringLiteral("warning")] = warning;

  if (!isDryRun)
    appendStaleProjectWarning(result, params, preEpoch);

  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns a composite read of the entire project state.
 */
/**
 * @brief Builds the prose _explanations block for a single Source.
 */
static QJsonObject buildSourceExplanations(const DataModel::Source& src, bool verbose)
{
  QJsonObject ex;
  ex[Keys::BusType]             = API::EnumLabels::busTypeLabel(src.busType);
  ex[Keys::FrameParserLanguage] = API::EnumLabels::scriptLanguageLabel(src.frameParserLanguage);

  if (verbose) {
    ex[Keys::FrameDetection] = API::EnumLabels::frameDetectionLabel(src.frameDetection);
    ex[Keys::DecoderMethod]  = API::EnumLabels::decoderMethodLabel(src.decoderMethod);
  }

  const QString summary =
    QStringLiteral("%1 source running a %2 parser; frames split by %3.")
      .arg(API::EnumLabels::busTypeLabel(src.busType),
           API::EnumLabels::scriptLanguageLabel(src.frameParserLanguage),
           API::EnumLabels::frameDetectionLabel(src.frameDetection).toLower());
  ex[QStringLiteral("summary")] = summary;
  return ex;
}

/**
 * @brief Builds the per-source array used by project.snapshot.
 */
static QJsonArray buildSnapshotSources(const DataModel::ProjectModel& pm, bool verbose)
{
  QJsonArray sources;
  for (const auto& src : pm.sources()) {
    QJsonObject s;
    s[Keys::SourceId]                    = src.sourceId;
    s[Keys::Title]                       = src.title;
    s[Keys::BusType]                     = src.busType;
    s[Keys::FrameParserLanguage]         = src.frameParserLanguage;
    s[QStringLiteral("frameParserSize")] = src.frameParserCode.size();

    if (verbose) {
      s[Keys::FrameStart]        = src.frameStart;
      s[Keys::FrameEnd]          = src.frameEnd;
      s[Keys::ChecksumAlgorithm] = src.checksumAlgorithm;
      s[Keys::FrameDetection]    = src.frameDetection;
      s[Keys::FrameParserCode]   = src.frameParserCode;
    }

    s[QStringLiteral("_explanations")] = buildSourceExplanations(src, verbose);
    sources.append(s);
  }
  return sources;
}

/**
 * @brief Builds the prose _explanations block for a single Group.
 */
static QJsonObject buildGroupExplanations(const DataModel::Group& group)
{
  QJsonObject ex;
  const QString widgetSlug     = group.widget.simplified().toLower();
  ex[QStringLiteral("widget")] = widgetSlug.isEmpty()
                                 ? QStringLiteral("No group widget (datasets render independently)")
                                 : QStringLiteral("Group widget: %1").arg(widgetSlug);

  if (!group.painterCode.isEmpty())
    ex[QStringLiteral("painterCode")] =
      QStringLiteral("Group has %1 bytes of painter JS (paint(ctx) entry point).")
        .arg(group.painterCode.size());

  return ex;
}

/**
 * @brief Builds the groups+datasets array; writes total dataset count to @a totalDatasets.
 */
static QJsonArray buildSnapshotGroups(const DataModel::ProjectModel& pm, int& totalDatasets)
{
  QJsonArray groups;
  totalDatasets = 0;
  for (const auto& group : pm.groups()) {
    QJsonObject g;
    g[Keys::GroupId]                  = group.groupId;
    g[Keys::Title]                    = group.title;
    g[QStringLiteral("widget")]       = group.widget;
    g[QStringLiteral("datasetCount")] = static_cast<int>(group.datasets.size());

    QJsonArray ds;
    for (const auto& dataset : group.datasets) {
      ds.append(buildDatasetObject(dataset, group));
      ++totalDatasets;
    }
    g[QStringLiteral("datasets")]      = ds;
    g[QStringLiteral("_explanations")] = buildGroupExplanations(group);
    groups.append(g);
  }
  return groups;
}

/**
 * @brief Builds the workspace summary array used by project.snapshot.
 */
static QJsonArray buildSnapshotWorkspaces(const DataModel::ProjectModel& pm)
{
  QJsonArray workspaces;
  for (const auto& ws : pm.editorWorkspaces()) {
    QJsonObject w;
    w[Keys::WorkspaceId]             = ws.workspaceId;
    w[Keys::Title]                   = ws.title;
    w[QStringLiteral("widgetCount")] = static_cast<int>(ws.widgetRefs.size());
    workspaces.append(w);
  }
  return workspaces;
}

/**
 * @brief Builds the data-tables summary array used by project.snapshot.
 */
static QJsonArray buildSnapshotTables(const DataModel::ProjectModel& pm)
{
  QJsonArray tables;
  for (const auto& t : pm.tables()) {
    QJsonObject tbl;
    tbl[Keys::Title]                     = t.name;
    tbl[QStringLiteral("registerCount")] = static_cast<int>(t.registers.size());

    const auto constants = std::count_if(t.registers.begin(), t.registers.end(), [](const auto& r) {
      return r.type == DataModel::RegisterType::Constant;
    });
    const auto computed  = std::count_if(t.registers.begin(), t.registers.end(), [](const auto& r) {
      return r.type == DataModel::RegisterType::Computed;
    });

    tbl[QStringLiteral("constantCount")] = static_cast<int>(constants);
    tbl[QStringLiteral("computedCount")] = static_cast<int>(computed);
    tables.append(tbl);
  }
  return tables;
}

/**
 * @brief Returns project.snapshot's caller hint, with a bulk-edit nudge above 10 datasets.
 */
static QString buildSnapshotHint(int totalDatasets)
{
  QString hint =
    QStringLiteral("Pass verbose=true to include frame parser source and source-level frame "
                   "settings. For per-table register details, call project.dataTable.get with "
                   "the table name.");
  if (totalDatasets >= 10)
    hint += QStringLiteral(" %1 datasets present -- bulk edits should go through project.batch "
                           "(ops: [{command, params}, ...]) instead of looping individual updates; "
                           "project.dataset.addMany handles 'create N similar' patterns.")
              .arg(totalDatasets);

  return hint;
}

/**
 * @brief Builds the top-level _explanations object summarizing the project's operating shape.
 */
static QJsonObject buildSnapshotExplanations(const DataModel::ProjectModel& pm,
                                             int operationMode,
                                             int totalDatasets)
{
  QJsonObject ex;
  ex[QStringLiteral("operationMode")] = API::EnumLabels::operationModeLabel(operationMode);

  const int sourceCount    = static_cast<int>(pm.sources().size());
  const int groupCount     = static_cast<int>(pm.groups().size());
  const int workspaceCount = static_cast<int>(pm.editorWorkspaces().size());
  const int tableCount     = static_cast<int>(pm.tables().size());

  QString summary;
  if (operationMode == SerialStudio::ConsoleOnly) {
    summary = QStringLiteral("Console-only mode: raw bytes go to the terminal, no parsing, "
                             "no dashboard. Dataset/group config is inert until you switch to "
                             "ProjectFile or QuickPlot via project.frameParser.update.");
  } else if (operationMode == SerialStudio::QuickPlot) {
    summary = QStringLiteral("Quick Plot mode: auto CSV plotting on CR/LF/CRLF delimiters. "
                             "Frame parser code and most project config are bypassed -- "
                             "Quick Plot generates datasets from comma-separated fields on "
                             "each line.");
  } else {
    summary =
      QStringLiteral("Project File mode: %1 source(s), %2 group(s), %3 dataset(s), "
                     "%4 workspace(s), %5 data table(s). Frame parser code is authoritative.")
        .arg(sourceCount)
        .arg(groupCount)
        .arg(totalDatasets)
        .arg(workspaceCount)
        .arg(tableCount);
  }

  if (totalDatasets >= 3)
    summary += QStringLiteral(" Loops touching multiple datasets should use project.batch.");

  ex[QStringLiteral("summary")] = summary;
  return ex;
}

/**
 * @brief Returns a structured snapshot of the active project (sources, groups, tables, hint).
 */
API::CommandResponse API::Handlers::ProjectHandler::projectSnapshot(const QString& id,
                                                                    const QJsonObject& params)
{
  const bool verbose = params.value(QStringLiteral("verbose")).toBool(false);
  const auto& pm     = DataModel::ProjectModel::instance();

  QJsonObject snapshot;
  snapshot[Keys::Title]                = pm.title();
  snapshot[Keys::PointCount]           = pm.pointCount();
  snapshot[QStringLiteral("filePath")] = pm.jsonFilePath();
  snapshot[QStringLiteral("modified")] = pm.modified();
  snapshot[QStringLiteral("sources")]  = buildSnapshotSources(pm, verbose);

  int totalDatasets                        = 0;
  const QJsonArray groups                  = buildSnapshotGroups(pm, totalDatasets);
  snapshot[QStringLiteral("groups")]       = groups;
  snapshot[QStringLiteral("groupCount")]   = groups.size();
  snapshot[QStringLiteral("datasetCount")] = totalDatasets;
  snapshot[QStringLiteral("workspaces")]   = buildSnapshotWorkspaces(pm);
  snapshot[QStringLiteral("dataTables")]   = buildSnapshotTables(pm);

  const int operationMode = static_cast<int>(AppState::instance().operationMode());
  snapshot[QStringLiteral("operationMode")] = operationMode;
  snapshot[QStringLiteral("_explanations")] =
    buildSnapshotExplanations(pm, operationMode, totalDatasets);

  QJsonObject result;
  result[QStringLiteral("snapshot")] = snapshot;
  result[QStringLiteral("hint")]     = buildSnapshotHint(totalDatasets);
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns datasets in the order FrameBuilder traverses them.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetGetExecutionOrder(
  const QString& id, const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& groups = DataModel::ProjectModel::instance().groups();

  QJsonArray order;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      QJsonObject entry;
      entry[Keys::UniqueId]                      = dataset.uniqueId;
      entry[Keys::Title]                         = dataset.title;
      entry[Keys::SourceId]                      = dataset.sourceId;
      entry[Keys::GroupId]                       = group.groupId;
      entry[Keys::DatasetId]                     = dataset.datasetId;
      entry[QStringLiteral("hasTransform")]      = !dataset.transformCode.isEmpty();
      entry[QStringLiteral("isVirtual")]         = dataset.virtual_;
      entry[QStringLiteral("transformLanguage")] = dataset.transformLanguage;
      order.append(entry);
    }
  }

  QJsonObject result;
  result[QStringLiteral("order")] = order;
  result[QStringLiteral("count")] = order.size();

  QJsonObject ex;
  ex[QStringLiteral("summary")] =
    QStringLiteral("Datasets execute in (group-array, dataset-array) order. A transform "
                   "may read raw values of ALL datasets via datasetGetRaw(uid), but only "
                   "final values of datasets EARLIER in this list via datasetGetFinal(uid).");
  result[QStringLiteral("_explanations")] = ex;
  attachProjectEpoch(result);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List all actions
 */
API::CommandResponse API::Handlers::ProjectHandler::actionsList(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& actions = DataModel::ProjectModel::instance().actions();

  QJsonArray actions_array;
  for (const auto& action : actions) {
    QJsonObject obj;
    obj[QStringLiteral("actionId")] = action.actionId;
    obj[QStringLiteral("title")]    = action.title;
    obj[QStringLiteral("icon")]     = action.icon;
    obj[QStringLiteral("txData")]   = action.txData;
    actions_array.append(obj);
  }

  QJsonObject result;
  result[QStringLiteral("actions")]     = actions_array;
  result[QStringLiteral("actionCount")] = static_cast<int>(actions.size());

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load project configuration from JSON object
 */
API::CommandResponse API::Handlers::ProjectHandler::loadFromJSON(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("config"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: config"));
  }

  const QJsonObject config = params.value(QStringLiteral("config")).toObject();
  if (config.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("config cannot be empty"));
  }

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  if (isDryRun) {
    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(config);
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no project was loaded. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  // In-memory load -- no temp file, no file association
  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  const bool ok = project.loadFromJsonDocument(QJsonDocument(config));
  project.setSuppressMessageBoxes(false);

  if (!ok) {
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("Failed to load project from JSON (validation error)"));
  }

  QJsonObject result;
  result[QStringLiteral("loaded")]       = true;
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Configure frame parser settings for a specific source.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserConfigure(const QString& id,
                                                                         const QJsonObject& params)
{
  // Obtain instance and validate state
  auto& model   = DataModel::ProjectModel::instance();
  auto& manager = IO::ConnectionManager::instance();
  bool updated  = false;

  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || (!model.sources().empty() && sourceId >= srcCount))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  if (params.contains(QStringLiteral("operationMode"))) {
    const int modeIdx = params.value(QStringLiteral("operationMode")).toInt();
    if (modeIdx >= 0 && modeIdx <= 2) {
      AppState::instance().setOperationMode(static_cast<SerialStudio::OperationMode>(modeIdx));
      updated = true;
    }
  }

  if (sourceId == 0) {
    // Route through top-level setters -- they sync into source[0] and update the FrameReader.
    if (params.contains(QStringLiteral("startSequence"))) {
      const QString start = params.value(QStringLiteral("startSequence")).toString();
      manager.setStartSequence(start.toUtf8());
      model.setFrameStartSequence(start);
      updated = true;
    }

    if (params.contains(QStringLiteral("endSequence"))) {
      const QString end = params.value(QStringLiteral("endSequence")).toString();
      manager.setFinishSequence(end.toUtf8());
      model.setFrameEndSequence(end);
      updated = true;
    }

    if (params.contains(Keys::ChecksumAlgorithm)) {
      const QString checksumName = params.value(Keys::ChecksumAlgorithm).toString();
      manager.setChecksumAlgorithm(checksumName);
      model.setChecksumAlgorithm(checksumName);
      updated = true;
    }

    if (params.contains(Keys::FrameDetection)) {
      const int detectionIdx = params.value(Keys::FrameDetection).toInt();
      if (detectionIdx >= 0 && detectionIdx <= 3) {
        model.setFrameDetection(static_cast<SerialStudio::FrameDetection>(detectionIdx));
        updated = true;
      }
    }
  } else {
    DataModel::Source src = model.sources()[sourceId];

    if (params.contains(QStringLiteral("startSequence"))) {
      src.frameStart = params.value(QStringLiteral("startSequence")).toString();
      updated        = true;
    }

    if (params.contains(QStringLiteral("endSequence"))) {
      src.frameEnd = params.value(QStringLiteral("endSequence")).toString();
      updated      = true;
    }

    if (params.contains(Keys::ChecksumAlgorithm)) {
      src.checksumAlgorithm = params.value(Keys::ChecksumAlgorithm).toString();
      updated               = true;
    }

    if (params.contains(Keys::FrameDetection)) {
      const int detectionIdx = params.value(Keys::FrameDetection).toInt();
      if (detectionIdx >= 0 && detectionIdx <= 3) {
        src.frameDetection = detectionIdx;
        updated            = true;
      }
    }

    if (updated)
      model.updateSource(sourceId, src);
  }

  if (updated && sourceId == 0)
    manager.resetFrameReader();

  QJsonObject result;
  result[QStringLiteral("updated")] = updated;
  result[Keys::SourceId]            = sourceId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current frame parser configuration
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserGetConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& cfg = AppState::instance().frameConfig();

  // Build the multi-source arrays
  QJsonArray startArr, endArr;
  for (const auto& s : cfg.startSequences)
    startArr.append(QString::fromUtf8(s));

  for (const auto& f : cfg.finishSequences)
    endArr.append(QString::fromUtf8(f));

  // Primary (source[0]) delimiters as singular scalars -- single-source clients
  const QString primaryStart =
    cfg.startSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.startSequences.first());
  const QString primaryEnd =
    cfg.finishSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.finishSequences.first());

  // Assemble the response
  QJsonObject result;
  result[QStringLiteral("startSequence")]  = primaryStart;
  result[QStringLiteral("endSequence")]    = primaryEnd;
  result[QStringLiteral("startSequences")] = startArr;
  result[QStringLiteral("endSequences")]   = endArr;
  result[Keys::ChecksumAlgorithm]          = cfg.checksumAlgorithm;
  result[QStringLiteral("operationMode")]  = static_cast<int>(cfg.operationMode);
  result[Keys::FrameDetection]             = static_cast<int>(cfg.frameDetection);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Export current project configuration as JSON
 */
API::CommandResponse API::Handlers::ProjectHandler::exportJson(const QString& id,
                                                               const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project          = DataModel::ProjectModel::instance();
  const QJsonObject json = project.serializeToJson();

  QJsonObject result;
  result[QStringLiteral("config")] = json;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Load current project JSON into FrameBuilder
 */
API::CommandResponse API::Handlers::ProjectHandler::loadIntoFrameBuilder(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  auto& project = DataModel::ProjectModel::instance();
  auto& builder = DataModel::FrameBuilder::instance();

  const bool hasDatasetlessGroup =
    std::any_of(project.groups().begin(), project.groups().end(), [](const DataModel::Group& g) {
      return g.widget == QLatin1String("image") || g.widget == QLatin1String("painter");
    });

  if (project.groupCount() == 0 || (project.datasetCount() == 0 && !hasDatasetlessGroup)) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Project has no groups or datasets"));
  }

  builder.syncFromProjectModel();

  QJsonObject result;
  result[QStringLiteral("loaded")]       = true;
  result[QStringLiteral("source")]       = QStringLiteral("API");
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();

  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Painter (group widget JS) command surface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register painter setCode/getCode commands.
 */
void API::Handlers::ProjectHandler::registerPainterCommands()
{
  registerPainterCodeCommands();
  registerUpdateCommands();
  registerDryRunCommands();
  registerEndToEndDryRunCommand();
}

/**
 * @brief Register painter widget JS setCode/getCode commands.
 */
void API::Handlers::ProjectHandler::registerPainterCodeCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.painter.setCode"),
    QStringLiteral("Set the painter widget code for a group (params: groupId, code). "
                   "**JavaScript only** -- painter scripts run in QJSEngine, not Lua. "
                   "Available globals: ctx (2D canvas context, QPainter-like), w, h "
                   "(canvas dimensions), datasetGetFinal(uid)/datasetGetRaw(uid). The "
                   "entry point is paint(ctx) and an optional onFrame(value) callback. "
                   "Validate with project.painter.dryRun before setCode. **Always call "
                   "meta.fetchScriptingDocs{kind:'painter_js'} first** for the full API "
                   "surface and worked examples -- don't invent canvas methods from JS "
                   "DOM Canvas, the surface is QPainter-shaped."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Target group id (from project.group.list)") },
      {   QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Painter widget JS source. Must define paint(ctx) and may define "
       "onFrame(value). Replaces any existing code for the group.")}
  }),
    &painterSetCode);

  registry.registerCommand(
    QStringLiteral("project.painter.getCode"),
    QStringLiteral("Get the painter widget JS for a group "
                   "(params: groupId)"),
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"), QStringLiteral("Target group id")}
  }),
    &painterGetCode);
}

/**
 * @brief Register patching update commands for groups/datasets/actions/outputs.
 */
/**
 * @brief Registers project.group.update and project.dataset.update.
 */
void API::Handlers::ProjectHandler::registerEntityUpdateCommands()
{
  auto& registry = CommandRegistry::instance();

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
    QStringLiteral("Patch dataset fields by id (params: groupId, datasetId, plus any of "
                   "title, units, widget, index, sourceId, graph, fft, led, waterfall, "
                   "log, overviewDisplay, hideOnDashboard, xAxisId, "
                   "waterfallYAxis, fftSamples, fftSamplingRate, fftMin, fftMax, "
                   "pltMin, pltMax, wgtMin, wgtMax, ledHigh, "
                   "alarmBands (array of {min,max,severity,color,label} objects, "
                   "severity=0..3 for Info/Ok/Warning/Critical), "
                   "or legacy alarmLow/alarmHigh/alarmEnabled for 2-band simple mode, "
                   "displayTickCount, displayFormat, "
                   "transformCode, transformLanguage, virtual). The boolean fields "
                   "graph/fft/led/waterfall toggle the same flags as "
                   "project.dataset.setOption -- use them here when patching multiple "
                   "fields at once.\n"
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
    makeSchema({
      {QStringLiteral("groupId"), QStringLiteral("integer"),   QStringLiteral("Target group id")},
      {          Keys::DatasetId, QStringLiteral("integer"), QStringLiteral("Target dataset id")}
  }),
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
                   "returning a Uint8Array / string), available globals, and the protocol "
                   "helper APIs (CRC, NMEA, Modbus, SLCAN, GRBL, GCode, SCPI, binary "
                   "packet)."),
    makeSchema({
      { QStringLiteral("groupId"),QStringLiteral("integer"),QStringLiteral("Target group id")                           },
      {QStringLiteral("widgetId"),
       QStringLiteral("integer"),
       QStringLiteral("Target widget index within the group")}
  }),
    &outputWidgetUpdate);
}

/**
 * @brief Registers the project.batch multi-op endpoint.
 */
void API::Handlers::ProjectHandler::registerBatchCommand()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.batch"),
    QStringLiteral(
      "Run several project mutations atomically WITH RESPECT TO AUTOSAVE -- "
      "all ops execute sequentially under one suspended-autosave window, "
      "and a single save is flushed at the end. Use this whenever you would "
      "otherwise issue more than ~5 sequential mutations (renames, retypes, "
      "reindexes), since N round-trips cost N times the latency and N times "
      "the autosave/tree-rebuild churn.\n"
      "Each op is {command: '<registered command name>', params: {...}}; "
      "results are returned in the same order with per-op success/error "
      "fields. Set stopOnError:true to abort the batch on the first failure "
      "(default false: best-effort, all ops attempted).\n"
      "Pass dryRun:true at the top level to preview every op without "
      "committing. Each op's per-result still carries the affected-entity "
      "payload as if it had run. Rejected when any op is not in the "
      "dryRun-aware command set -- mixing previewable and non-previewable "
      "ops would leave a partial mutation, which is worse than no preview "
      "at all.\n"
      "Note: NOT transactional. Already-applied ops are NOT rolled back on "
      "later failures -- this is a save-suspend wrapper, not a database "
      "transaction. Nested project.batch calls are rejected. Hard cap of "
      "1024 ops per call.\n"
      "Example: rename 40 datasets in one round-trip:\n"
      "  ops: [\n"
      "    {command:'project.dataset.update', params:{groupId:0, datasetId:0, title:'LED 1', index:1}},\n"
      "    {command:'project.dataset.update', params:{groupId:0, datasetId:1, title:'LED 2', index:2}},\n"
      "    ...\n"
      "  ]"),
    makeSchema({
      arrayProp(
        QStringLiteral("ops"),
        QStringLiteral("Array of {command, params} ops to execute sequentially. Max 1024."),
        QJsonObject{
                    {QStringLiteral("type"), QStringLiteral("object")},
                    {QStringLiteral("properties"),
           QJsonObject{
             {QStringLiteral("command"),
              QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                          {QStringLiteral("description"),
                           QStringLiteral("Registered command name (e.g. "
                                          "'project.dataset.update'). Not "
                                          "'project.batch' -- nested batches are rejected.")}}},
             {QStringLiteral("params"),
              QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
                          {QStringLiteral("description"),
                           QStringLiteral(
                             "Arguments object for the command, exactly "
                             "what you would pass at the top level if calling it directly.")}}}}},
                    {QStringLiteral("required"),
           QJsonArray{QStringLiteral("command"), QStringLiteral("params")}}}
        )
  }),
    &projectBatch);
}

/**
 * @brief Register patch endpoints for groups/datasets/actions/widgets and project.batch.
 */
void API::Handlers::ProjectHandler::registerUpdateCommands()
{
  registerEntityUpdateCommands();
  registerBatchCommand();
}

/**
 * @brief Register dryRun endpoints (compile + execute in throwaway engines).
 */
void API::Handlers::ProjectHandler::registerDryRunCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.frameParser.dryRun"),
    QStringLiteral("Compile and execute frame parser code against one or more sample "
                   "frames WITHOUT touching the live project. Returns parsed values "
                   "per frame, or compile/runtime errors. Pass either sampleFrame "
                   "(single string) or sampleFrames (array of strings) -- the array "
                   "form runs sequentially through one engine, so stateful parsers "
                   "(top-level closures, EMA-style state) reveal their behavior. Use "
                   "this to iterate before project.frameParser.setCode."),
    makeSchema({
      {       QStringLiteral("code"),QStringLiteral("string"),      QStringLiteral("Frame parser source")                          },
      {   QStringLiteral("language"),
       QStringLiteral("integer"),
       QStringLiteral("0 = JavaScript, 1 = Lua")                                               },
      {QStringLiteral("sampleFrame"),
       QStringLiteral("string"),
       QStringLiteral("Single frame body (without delimiters). Use sampleFrames for an array.")},
      typedArrayProp(
        QStringLiteral("sampleFrames"),
        QStringLiteral("Array of frame bodies; runs sequentially in one engine instance."),
        QStringLiteral("string"))
  }),
    &frameParserDryRun);

  registry.registerCommand(
    QStringLiteral("project.frameParser.dryCompile"),
    QStringLiteral("Compile-only check for a frame parser. Catches syntax errors and the "
                   "'wrong-language' silent failure (e.g. Lua code passed with language=0). "
                   "Returns {ok, error?, warning?} without executing the parser. Cheap; use "
                   "before frameParser.setCode when authoring."),
    makeSchema({
      {    QStringLiteral("code"),QStringLiteral("string"),QStringLiteral("Frame parser source")          },
      {QStringLiteral("language"),
       QStringLiteral("integer"),
       QStringLiteral("0 = JavaScript, 1 = Lua")}
  }),
    &frameParserDryCompile);

  registry.registerCommand(
    QStringLiteral("project.dataset.transform.dryRun"),
    QStringLiteral("Compile and execute a value-transform script against one or "
                   "more sample inputs WITHOUT touching the live project. Returns "
                   "the per-input transform output or compile errors. Params: "
                   "code (string), language (0=JS, 1=Lua), values (array of "
                   "numbers or strings)."),
    makeSchema({
      {QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Transform source. Must define transform(value).")},
      {QStringLiteral("language"),
       QStringLiteral("integer"),
       QStringLiteral("0 = JavaScript, 1 = Lua")},
      arrayProp(QStringLiteral("values"),
                QStringLiteral("Sample values to pass through transform(). Each entry may be a "
                               "number or a string -- the dispatcher coerces as needed."),
                QJsonObject{{QStringLiteral("type"),
                             QJsonArray{QStringLiteral("number"), QStringLiteral("string")}}}
      )
  }),
    &transformDryRun);

  registry.registerCommand(
    QStringLiteral("project.painter.dryRun"),
    QStringLiteral("Compile a painter program WITHOUT touching the live project. "
                   "Verifies that paint(ctx, w, h) exists and that the script "
                   "compiles cleanly; does NOT actually render to a canvas. "
                   "Returns ok / lastError. Params: code (string)."),
    makeSchema({
      {QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Painter source. Must define paint(ctx, w, h)")}
  }),
    &painterDryRun);
}

/**
 * @brief Register the end-to-end dryRun endpoint (parser + transforms in throwaway engines).
 */
void API::Handlers::ProjectHandler::registerEndToEndDryRunCommand()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dryRun.endToEnd"),
    QStringLiteral("End-to-end dry run: takes a sample frame body, runs the project's "
                   "frame parser, then applies every dataset's transform in execution "
                   "order, and returns the final per-dataset values WITHOUT touching "
                   "live state. Use this to verify the full parse->transform pipeline "
                   "before issuing setCode/setTransformCode. Note: the table API "
                   "(tableGet/tableSet/datasetGetRaw/datasetGetFinal) is NOT injected; "
                   "transforms that depend on it should be tested with "
                   "project.dataset.transform.dryRun individually."),
    makeSchema({
      {QStringLiteral("sampleFrame"),
       QStringLiteral("string"),
       QStringLiteral("Single frame body (without delimiters). Use sampleFrames for an array.")   },
      typedArrayProp(
        QStringLiteral("sampleFrames"),
        QStringLiteral("Array of frame bodies; runs sequentially in one parser engine instance."),
        QStringLiteral("string")),
      {      QString(Keys::SourceId),
       QStringLiteral("integer"),
       QStringLiteral("Source index to use for parser code + dataset transforms (default 0)")     },
      {       QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Optional override for the frame parser source (default: use live project)")},
      {   QStringLiteral("language"),
       QStringLiteral("integer"),
       QStringLiteral(
       "Optional override: 0 = JavaScript, 1 = Lua (default: live source language)")              },
      {    QStringLiteral("verbose"),
       QStringLiteral("boolean"),
       QStringLiteral(
       "Include raw cell values alongside final transformed values (default false)")              }
  }),
    &endToEndDryRun);
}

/**
 * @brief Set painter code for a specific group by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::painterSetCode(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const QString code = params.value(QStringLiteral("code")).toString();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  g.painterCode      = code;
  project.updateGroup(groupId, g, /*rebuildTree=*/false);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("codeLength")] = code.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the painter code for a group.
 */
API::CommandResponse API::Handlers::ProjectHandler::painterGetCode(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const auto& groups = DataModel::ProjectModel::instance().groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("code")]    = groups[groupId].painterCode;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Bulk update mutators -- stateless, id required, PATCH semantics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Patch any subset of group fields by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::groupUpdate(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  bool rebuildTree   = false;
  QSet<QString> consumed{QStringLiteral("groupId")};

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

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("updated")] = true;
  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.group.update"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Patch optional dataset fields onto @p d; returns non-empty error string on failure.
 */
/**
 * @brief Returns true if @a params has @a key; records the key into @a consumed.
 */
static bool takeParam(const QJsonObject& params, QSet<QString>& consumed, const QString& key)
{
  if (!params.contains(key))
    return false;

  consumed.insert(key);
  return true;
}

/**
 * @brief Applies title/units/widget text fields and visualization toggles to @a d.
 */
static void applyDatasetTextAndToggleFields(DataModel::Dataset& d,
                                            const QJsonObject& params,
                                            bool& rebuildTree,
                                            QSet<QString>& consumed)
{
  if (takeParam(params, consumed, QStringLiteral("title"))) {
    d.title     = params.value(QStringLiteral("title")).toString();
    rebuildTree = true;
  }
  if (takeParam(params, consumed, QStringLiteral("units")))
    d.units = params.value(QStringLiteral("units")).toString();

  if (takeParam(params, consumed, QStringLiteral("widget"))) {
    d.widget    = params.value(QStringLiteral("widget")).toString();
    rebuildTree = true;
  }

  // Visualization booleans share the project JSON keys (graph/fft/led/waterfall)
  if (takeParam(params, consumed, Keys::Graph)) {
    d.plt       = params.value(Keys::Graph).toBool();
    rebuildTree = true;
  }
  if (takeParam(params, consumed, Keys::FFT)) {
    d.fft       = params.value(Keys::FFT).toBool();
    rebuildTree = true;
  }
  if (takeParam(params, consumed, Keys::LED)) {
    d.led       = params.value(Keys::LED).toBool();
    rebuildTree = true;
  }
  if (takeParam(params, consumed, Keys::Waterfall)) {
    d.waterfall = params.value(Keys::Waterfall).toBool();
    rebuildTree = true;
  }
}

/**
 * @brief Applies index/sourceId/numeric range fields; returns error string on bad index.
 */
static QString applyDatasetNumericFields(DataModel::Dataset& d,
                                         const QJsonObject& params,
                                         bool& rebuildTree,
                                         QSet<QString>& consumed)
{
  if (takeParam(params, consumed, QStringLiteral("index"))) {
    const int idx = params.value(QStringLiteral("index")).toInt();
    if (idx < 0)
      return QStringLiteral("Invalid index: must be >= 0 (0 = unassigned, 1+ = parser slot)");

    d.index     = idx;
    rebuildTree = true;
  }

  if (takeParam(params, consumed, QStringLiteral("xAxisId")))
    d.xAxisId = params.value(QStringLiteral("xAxisId")).toInt();

  if (takeParam(params, consumed, QStringLiteral("waterfallYAxis")))
    d.waterfallYAxis = params.value(QStringLiteral("waterfallYAxis")).toInt();

  if (takeParam(params, consumed, Keys::SourceId))
    d.sourceId = params.value(Keys::SourceId).toInt();

  if (takeParam(params, consumed, QStringLiteral("fftSamples")))
    d.fftSamples = params.value(QStringLiteral("fftSamples")).toInt();

  if (takeParam(params, consumed, QStringLiteral("fftSamplingRate")))
    d.fftSamplingRate = params.value(QStringLiteral("fftSamplingRate")).toInt();

  if (takeParam(params, consumed, QStringLiteral("fftMin")))
    d.fftMin = params.value(QStringLiteral("fftMin")).toDouble();

  if (takeParam(params, consumed, QStringLiteral("fftMax")))
    d.fftMax = params.value(QStringLiteral("fftMax")).toDouble();

  if (takeParam(params, consumed, QStringLiteral("pltMin")))
    d.pltMin = params.value(QStringLiteral("pltMin")).toDouble();

  if (takeParam(params, consumed, QStringLiteral("pltMax")))
    d.pltMax = params.value(QStringLiteral("pltMax")).toDouble();

  if (takeParam(params, consumed, QStringLiteral("wgtMin")))
    d.wgtMin = params.value(QStringLiteral("wgtMin")).toDouble();

  if (takeParam(params, consumed, QStringLiteral("wgtMax")))
    d.wgtMax = params.value(QStringLiteral("wgtMax")).toDouble();

  const bool hasAlarmBands = takeParam(params, consumed, Keys::AlarmBands);
  const bool hasAlarmLow   = takeParam(params, consumed, QStringLiteral("alarmLow"));
  const bool hasAlarmHigh  = takeParam(params, consumed, QStringLiteral("alarmHigh"));
  const bool hasAlarmEnab  = takeParam(params, consumed, QStringLiteral("alarmEnabled"));

  if (hasAlarmBands) {
    d.alarmBands.clear();
    const auto arr = params.value(Keys::AlarmBands).toArray();
    d.alarmBands.reserve(arr.size());
    for (const auto& v : arr) {
      DataModel::AlarmBand b;
      if (DataModel::read(b, v.toObject()))
        d.alarmBands.push_back(std::move(b));
    }
  }

  else if (hasAlarmLow || hasAlarmHigh || hasAlarmEnab) {
    std::optional<bool> en;
    std::optional<double> lo;
    std::optional<double> hi;
    if (hasAlarmEnab)
      en = params.value(QStringLiteral("alarmEnabled")).toBool();

    if (hasAlarmLow)
      lo = params.value(QStringLiteral("alarmLow")).toDouble();

    if (hasAlarmHigh)
      hi = params.value(QStringLiteral("alarmHigh")).toDouble();

    applySimpleAlarmFields(d, en, lo, hi);
  }

  if (takeParam(params, consumed, Keys::DisplayTickCount))
    d.displayTickCount = qMax(0, params.value(Keys::DisplayTickCount).toInt());

  if (takeParam(params, consumed, Keys::DisplayFormat))
    d.displayFormat = params.value(Keys::DisplayFormat).toString();

  if (takeParam(params, consumed, QStringLiteral("ledHigh")))
    d.ledHigh = params.value(QStringLiteral("ledHigh")).toDouble();

  return QString();
}

/**
 * @brief Applies log/display/transform fields; returns error string on bad transformLanguage.
 */
static QString applyDatasetDisplayAndTransformFields(DataModel::Dataset& d,
                                                     const QJsonObject& params,
                                                     bool& rebuildTree,
                                                     QSet<QString>& consumed)
{
  if (takeParam(params, consumed, QStringLiteral("log")))
    d.log = params.value(QStringLiteral("log")).toBool();

  if (takeParam(params, consumed, QStringLiteral("overviewDisplay"))) {
    d.overviewDisplay = params.value(QStringLiteral("overviewDisplay")).toBool();
    rebuildTree       = true;
  }

  if (takeParam(params, consumed, QStringLiteral("hideOnDashboard"))) {
    d.hideOnDashboard = params.value(QStringLiteral("hideOnDashboard")).toBool();
    rebuildTree       = true;
  }

  if (takeParam(params, consumed, QStringLiteral("transformCode")))
    d.transformCode = params.value(QStringLiteral("transformCode")).toString();

  if (takeParam(params, consumed, Keys::TransformLanguage)) {
    const int lang = params.value(Keys::TransformLanguage).toInt();
    if (lang != -1 && lang != SerialStudio::JavaScript && lang != SerialStudio::Lua)
      return QStringLiteral("Invalid transformLanguage: must be -1 (inherit), 0 (JS), or 1 (Lua)");

    d.transformLanguage = lang;
  }

  if (takeParam(params, consumed, Keys::Virtual))
    d.virtual_ = params.value(Keys::Virtual).toBool();

  return QString();
}

/**
 * @brief Patches dataset fields from a generic params object; returns error string on failure.
 */
QString API::Handlers::ProjectHandler::applyDatasetUpdateParams(DataModel::Dataset& d,
                                                                const QJsonObject& params,
                                                                bool& rebuildTree,
                                                                QSet<QString>& consumed)
{
  applyDatasetTextAndToggleFields(d, params, rebuildTree, consumed);

  if (auto err = applyDatasetNumericFields(d, params, rebuildTree, consumed); !err.isEmpty())
    return err;

  if (auto err = applyDatasetDisplayAndTransformFields(d, params, rebuildTree, consumed);
      !err.isEmpty())
    return err;

  return QString();
}

/**
 * @brief Patch any subset of dataset fields by groupId + datasetId.
 */
API::CommandResponse API::Handlers::ProjectHandler::datasetUpdate(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(Keys::DatasetId))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: datasetId"));

  const int groupId   = params.value(QStringLiteral("groupId")).toInt();
  const int datasetId = params.value(Keys::DatasetId).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& groups  = project.groups();
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
  const QString err = applyDatasetUpdateParams(d, params, rebuildTree, consumed);
  if (!err.isEmpty())
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, err);

  const auto preEpoch = captureProjectEpoch();
  project.updateDataset(groupId, datasetId, d, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[Keys::DatasetId]           = datasetId;
  result[QStringLiteral("updated")] = true;

  appendUnknownFieldsWarning(result, params, consumed, QStringLiteral("project.dataset.update"));
  appendStaleProjectWarning(result, params, preEpoch);
  attachProjectEpoch(result);

  if (!d.transformCode.isEmpty() && d.transformLanguage != -1) {
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
API::CommandResponse API::Handlers::ProjectHandler::actionUpdate(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("actionId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: actionId"));

  const int actionId  = params.value(QStringLiteral("actionId")).toInt();
  auto& project       = DataModel::ProjectModel::instance();
  const auto& actions = project.actions();
  if (actionId < 0 || static_cast<size_t>(actionId) >= actions.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Action id not found: %1").arg(actionId));

  DataModel::Action a = actions[actionId];
  bool rebuildTree    = false;
  QSet<QString> consumed{QStringLiteral("actionId")};

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
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetUpdate(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  const int widgetId = params.value(QStringLiteral("widgetId")).toInt();
  auto& project      = DataModel::ProjectModel::instance();
  const auto& groups = project.groups();
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
    w.minValue = params.value(QStringLiteral("minValue")).toDouble();

  if (take(QStringLiteral("maxValue")))
    w.maxValue = params.value(QStringLiteral("maxValue")).toDouble();

  if (take(QStringLiteral("stepSize")))
    w.stepSize = params.value(QStringLiteral("stepSize")).toDouble();

  if (take(QStringLiteral("initialValue")))
    w.initialValue = params.value(QStringLiteral("initialValue")).toDouble();

  project.updateGroup(groupId, g, rebuildTree);

  QJsonObject result;
  result[QStringLiteral("groupId")]  = groupId;
  result[QStringLiteral("widgetId")] = widgetId;
  result[QStringLiteral("updated")]  = true;
  appendUnknownFieldsWarning(
    result, params, consumed, QStringLiteral("project.outputWidget.update"));
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Project batch -- run a sequence of commands under one autosave window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs an array of project mutations sequentially under a suspended autosave.
 */
/**
 * @brief Returns the schema-hint object attached to every project.batch validation error.
 */
static QJsonObject buildBatchSchemaHint()
{
  QJsonObject example;
  QJsonArray exampleOps;
  QJsonObject op0;
  op0[QStringLiteral("command")] = QStringLiteral("project.dataset.update");
  QJsonObject p0;
  p0[QStringLiteral("groupId")] = 0;
  p0[Keys::DatasetId]           = 0;
  p0[QStringLiteral("title")]   = QStringLiteral("LED 1");
  p0[QStringLiteral("index")]   = 1;
  op0[QStringLiteral("params")] = p0;
  exampleOps.append(op0);
  example[QStringLiteral("ops")]         = exampleOps;
  example[QStringLiteral("stopOnError")] = false;

  QJsonObject hint;
  hint[QStringLiteral("expected")] =
    QStringLiteral("{ ops: Array<{command: string, params: object}>, stopOnError?: boolean }");
  hint[QStringLiteral("opShape")] =
    QStringLiteral("Each op MUST be {command: '<registered name>', params: {...}}. Per-call "
                   "args go INSIDE params, not at the top of the op object.");
  hint[QStringLiteral("limits")] =
    QStringLiteral("1 <= ops.length <= 1024. Nested project.batch is rejected.");
  hint[QStringLiteral("example")] = example;
  return hint;
}

/**
 * @brief Builds a per-op result entry for a validation failure inside project.batch.
 */
static QJsonObject buildBatchErrorEntry(int index,
                                        const QString& command,
                                        const QString& code,
                                        const QString& message,
                                        const QJsonObject& data)
{
  QJsonObject entry;
  entry[QStringLiteral("index")] = index;
  if (!command.isEmpty())
    entry[QStringLiteral("command")] = command;

  entry[QStringLiteral("success")] = false;
  QJsonObject err;
  err[QStringLiteral("code")]    = code;
  err[QStringLiteral("message")] = message;
  if (!data.isEmpty())
    err[QStringLiteral("data")] = data;

  entry[QStringLiteral("error")] = err;
  return entry;
}

/**
 * @brief Commands that honour `dryRun:true` -- used to validate batch previews.
 */
static const QSet<QString>& dryRunAwareCommands()
{
  static const QSet<QString> kSet = {
    QStringLiteral("project.dataset.delete"),
    QStringLiteral("project.group.delete"),
    QStringLiteral("project.dataset.move"),
    QStringLiteral("project.group.move"),
    QStringLiteral("project.workspace.delete"),
    QStringLiteral("project.workspace.clearAll"),
    QStringLiteral("project.new"),
    QStringLiteral("project.open"),
    QStringLiteral("project.loadJson"),
    QStringLiteral("project.template.apply"),
    QStringLiteral("project.batch"),
    QStringLiteral("assistant.project.bulkApply"),
  };
  return kSet;
}

/**
 * @brief Validates and executes a single project.batch op; returns the per-op result entry.
 */
static QJsonObject executeBatchOp(int index, const QJsonObject& op, bool dryRun, bool& success)
{
  success = false;

  if (op.isEmpty()) {
    return buildBatchErrorEntry(
      index,
      QString(),
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops[%1] must be an object of shape {command: string, params: object}")
        .arg(index),
      buildBatchSchemaHint());
  }

  const auto command = op.value(QStringLiteral("command")).toString();
  auto opParams      = op.value(QStringLiteral("params")).toObject();

  if (command.isEmpty()) {
    return buildBatchErrorEntry(index,
                                QString(),
                                API::ErrorCode::MissingParam,
                                QStringLiteral("ops[%1].command is required (each op is "
                                               "{command: '<registered name>', params: {...}})")
                                  .arg(index),
                                buildBatchSchemaHint());
  }

  if (command == QStringLiteral("project.batch")) {
    return buildBatchErrorEntry(index,
                                command,
                                API::ErrorCode::InvalidParam,
                                QStringLiteral("project.batch cannot be nested"),
                                QJsonObject());
  }

  // Propagate dryRun into every op so the inner handler short-circuits without mutating.
  if (dryRun)
    opParams.insert(QStringLiteral("dryRun"), true);

  const auto response =
    API::CommandRegistry::instance().execute(command, QString::number(index), opParams);

  QJsonObject entry;
  entry[QStringLiteral("index")]   = index;
  entry[QStringLiteral("command")] = command;
  entry[QStringLiteral("success")] = response.success;
  if (response.success) {
    if (!response.result.isEmpty())
      entry[QStringLiteral("result")] = response.result;

    success = true;
  } else {
    QJsonObject err;
    err[QStringLiteral("code")]    = response.errorCode;
    err[QStringLiteral("message")] = response.errorMessage;
    if (!response.errorData.isEmpty())
      err[QStringLiteral("data")] = response.errorData;

    entry[QStringLiteral("error")] = err;
  }
  return entry;
}

/**
 * @brief Validate the `ops` array against the batch handler's preconditions; returns an error
 * response when invalid.
 */
static std::optional<API::CommandResponse> validateBatchOps(const QString& id,
                                                            const QJsonObject& params,
                                                            QJsonArray& outOps)
{
  constexpr int kMaxBatchOps = 1024;

  if (!params.contains(QStringLiteral("ops")))
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::MissingParam,
                                           QStringLiteral("Missing required parameter: ops"),
                                           buildBatchSchemaHint());

  if (!params.value(QStringLiteral("ops")).isArray())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops must be an array"),
                                           buildBatchSchemaHint());

  outOps = params.value(QStringLiteral("ops")).toArray();
  if (outOps.isEmpty())
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("ops array must not be empty"),
                                           buildBatchSchemaHint());

  if (outOps.size() > kMaxBatchOps)
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::InvalidParam,
      QStringLiteral("ops array exceeds limit of %1 (got %2)")
        .arg(QString::number(kMaxBatchOps), QString::number(outOps.size())),
      buildBatchSchemaHint());

  return std::nullopt;
}

/**
 * @brief When dryRun is set, ensure every op supports dryRun; returns an error response if any does
 * not.
 */
static std::optional<API::CommandResponse> ensureBatchDryRunCompatible(const QString& id,
                                                                       const QJsonArray& ops)
{
  QJsonArray unsupported;
  for (int i = 0; i < ops.size(); ++i) {
    const auto cmd = ops.at(i).toObject().value(QStringLiteral("command")).toString();
    if (!dryRunAwareCommands().contains(cmd)) {
      QJsonObject row;
      row[QStringLiteral("index")]   = i;
      row[QStringLiteral("command")] = cmd;
      unsupported.append(row);
    }
  }
  if (unsupported.isEmpty())
    return std::nullopt;

  QJsonObject data;
  data[QStringLiteral("unsupportedOps")] = unsupported;
  data[QStringLiteral("supportedSet")] = QJsonArray::fromStringList(dryRunAwareCommands().values());
  return API::CommandResponse::makeError(
    id,
    API::ErrorCode::InvalidParam,
    QStringLiteral("dryRun rejected: %1 op(s) do not support dryRun. Either drop dryRun "
                   "for the whole batch or split the un-previewable ops out into a "
                   "separate batch.")
      .arg(unsupported.size()),
    data);
}

/**
 * @brief Runs an array of project mutations under a single suspended-autosave window.
 */
API::CommandResponse API::Handlers::ProjectHandler::projectBatch(const QString& id,
                                                                 const QJsonObject& params)
{
  QJsonArray ops;
  if (const auto invalid = validateBatchOps(id, params, ops); invalid)
    return *invalid;

  const bool stopOnError = params.value(QStringLiteral("stopOnError")).toBool(false);
  const bool isDryRun    = params.value(QStringLiteral("dryRun")).toBool(false);

  if (isDryRun) {
    if (const auto invalid = ensureBatchDryRunCompatible(id, ops); invalid)
      return *invalid;
  }

  auto& project = DataModel::ProjectModel::instance();
  if (!isDryRun)
    project.setAutoSaveSuspended(true);

  QJsonArray results;
  int successCount = 0;
  int failureCount = 0;
  bool aborted     = false;

  for (int i = 0; i < ops.size(); ++i) {
    bool opSucceeded  = false;
    QJsonObject entry = executeBatchOp(i, ops.at(i).toObject(), isDryRun, opSucceeded);
    results.append(entry);
    if (opSucceeded)
      ++successCount;
    else
      ++failureCount;

    if (!opSucceeded && stopOnError) {
      aborted = true;
      break;
    }
  }

  if (!isDryRun) {
    project.setAutoSaveSuspended(false);
    project.flushAutoSave();
  }

  QJsonObject result;
  if (isDryRun)
    result[QStringLiteral("dryRun")] = true;

  result[QStringLiteral("results")]   = results;
  result[QStringLiteral("total")]     = ops.size();
  result[QStringLiteral("succeeded")] = successCount;
  result[QStringLiteral("failed")]    = failureCount;
  result[QStringLiteral("aborted")]   = aborted;
  result[QStringLiteral("autoSaveMode")] =
    isDryRun ? QStringLiteral("none") : QStringLiteral("flushed");
  if (isDryRun)
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: no ops were committed. Each op's per-result still carries the "
                     "affected-entity payload as if it had run.");

  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Project templates -- starter projects loaded from rcc:/ai/templates/*
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the templates manifest as a QJsonDocument, or empty on error.
 */
static QJsonDocument loadTemplateManifest()
{
  QFile f(QStringLiteral(":/ai/templates/manifest.json"));
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return {};

  return QJsonDocument::fromJson(f.readAll());
}

/**
 * @brief Loads a template body file by id. Returns empty doc when not found.
 */
static QJsonDocument loadTemplateBodyById(const QString& templateId)
{
  const auto manifest = loadTemplateManifest().object();
  const auto entries  = manifest.value(QStringLiteral("templates")).toArray();
  for (const auto& v : entries) {
    const auto entry = v.toObject();
    if (entry.value(QStringLiteral("id")).toString() != templateId)
      continue;

    const auto file = entry.value(QStringLiteral("file")).toString();
    QFile body(QStringLiteral(":/ai/templates/") + file);
    if (!body.open(QIODevice::ReadOnly | QIODevice::Text))
      return {};

    return QJsonDocument::fromJson(body.readAll());
  }
  return {};
}

/**
 * @brief Register project.template.* commands.
 */
void API::Handlers::ProjectHandler::registerTemplateCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.template.list"),
    QStringLiteral("List built-in starter project templates (id, title, description). "
                   "Use this BEFORE project.template.apply when the user asks for a "
                   "starter (\"set me up an IMU project\", \"give me a GPS template\")."),
    emptySchema(),
    &templateList);

  registry.registerCommand(
    QStringLiteral("project.template.apply"),
    QStringLiteral("Replace the current project with a starter template -- the right "
                   "first move when the user says \"set me up an X project\" rather "
                   "than \"add an X to my existing project\". DESTRUCTIVE: discards "
                   "any unsaved current state; auto-save will write the new template "
                   "to disk shortly after. After applying, narrate what landed (groups, "
                   "datasets) so the user knows what they got, then proceed with any "
                   "follow-up edits the user requested. Available templates and their "
                   "best fit:\n"
                   "  blank: empty project, one default UART source. Use when the user "
                   "wants to build everything themselves.\n"
                   "  imu_uart: 9-DOF IMU (accelerometer X/Y/Z + gyro X/Y/Z + mag "
                   "X/Y/Z) over UART, comma-separated, $ start / ; end. Replace the "
                   "frame parser if your device frames differ.\n"
                   "  gps_uart_nmea: GPS over UART using NMEA 0183 ($GPGGA). "
                   "Lat/lon/altitude/satellites/HDOP/fixQuality. Map widget configured.\n"
                   "  scope_multichannel_uart: 8 generic channels over UART, "
                   "comma-separated, all plot-enabled. Adapt for ADC streams or sensor "
                   "arrays.\n"
                   "  telemetry_udp: UDP listener on port 1234, CSV body parser, 5 "
                   "generic value channels. Adapt port + parser to your protocol.\n"
                   "  mqtt_subscriber (Pro): MQTT subscriber-mode skeleton. After "
                   "applying, configure broker via mqtt.set* commands.\n"
                   "Pass dryRun:true to return wouldDiscard + wouldApply summaries "
                   "without applying. Useful when the user is choosing between two "
                   "templates."),
    makeSchema(
      {
        {QStringLiteral("templateId"),
         QStringLiteral("string"),
         QStringLiteral("Template id from project.template.list (blank, imu_uart, "
                        "gps_uart_nmea, scope_multichannel_uart, telemetry_udp, "
                        "mqtt_subscriber)")}
  },
      {{QStringLiteral("dryRun"),
        QStringLiteral("boolean"),
        QStringLiteral("If true, summarize the template without applying.")}}),
    &templateApply);
}

/**
 * @brief Returns the templates manifest.
 */
API::CommandResponse API::Handlers::ProjectHandler::templateList(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto manifest = loadTemplateManifest();
  if (manifest.isNull())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Failed to load templates manifest"));

  QJsonObject result;
  result[QStringLiteral("templates")] = manifest.object().value(QStringLiteral("templates"));
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Loads a template into the project model.
 */
API::CommandResponse API::Handlers::ProjectHandler::templateApply(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("templateId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: templateId"));

  const auto templateId = params.value(QStringLiteral("templateId")).toString();
  const auto body       = loadTemplateBodyById(templateId);
  if (body.isNull())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Template not found: %1. Call project.template.list for the "
                     "available ids.")
        .arg(templateId));

  const bool isDryRun = params.value(QStringLiteral("dryRun")).toBool(false);
  if (isDryRun) {
    QJsonObject result;
    result[QStringLiteral("dryRun")]       = true;
    result[QStringLiteral("templateId")]   = templateId;
    result[QStringLiteral("wouldDiscard")] = summarizeCurrentProject();
    result[QStringLiteral("wouldApply")]   = summarizeProjectJson(body.object());
    result[QStringLiteral("warning")] =
      QStringLiteral("DRY RUN: template not applied. wouldDiscard shows what would be lost; "
                     "wouldApply shows what would replace it. Confirm before re-issuing "
                     "without dryRun:true.");
    return CommandResponse::makeSuccess(id, result);
  }

  auto& project = DataModel::ProjectModel::instance();
  project.setSuppressMessageBoxes(true);
  const bool ok = project.loadFromJsonDocument(body);
  project.setSuppressMessageBoxes(false);

  if (!ok)
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Template applied but project failed to validate"));

  QJsonObject result;
  result[QStringLiteral("templateId")]   = templateId;
  result[QStringLiteral("title")]        = project.title();
  result[QStringLiteral("groupCount")]   = project.groupCount();
  result[QStringLiteral("datasetCount")] = project.datasetCount();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Project consistency validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a structured issue row to the validate() result array.
 */
static void addIssue(QJsonArray& issues,
                     const QString& level,
                     const QString& location,
                     const QString& message)
{
  QJsonObject row;
  row[QStringLiteral("level")]    = level;
  row[QStringLiteral("location")] = location;
  row[QStringLiteral("message")]  = message;
  issues.append(row);
}

/**
 * @brief Validates per-source frame parsers (compiles each, flags failures).
 */
static void validateSources(const std::vector<DataModel::Source>& sources,
                            QJsonArray& issues,
                            bool& ok)
{
  auto& parser = DataModel::FrameParser::instance();
  for (const auto& src : sources) {
    if (src.frameParserCode.isEmpty()) {
      addIssue(issues,
               QStringLiteral("info"),
               QStringLiteral("source[%1]").arg(src.sourceId),
               QStringLiteral("Source has no frame parser; raw frames will be dropped"));
      continue;
    }

    const bool compiled = parser.loadScript(src.sourceId, src.frameParserCode, false);
    if (!compiled) {
      ok = false;
      addIssue(issues,
               QStringLiteral("error"),
               QStringLiteral("source[%1].frameParser").arg(src.sourceId),
               QStringLiteral("Frame parser failed to compile or define parse()"));
    }
  }
}

/**
 * @brief Validates a single dataset's references and per-field invariants.
 */
static void validateDataset(const DataModel::Dataset& d,
                            const DataModel::Group& g,
                            const QSet<int>& sourceIds,
                            QSet<int>& datasetIndexes,
                            QJsonArray& issues,
                            bool& ok)
{
  const QString dloc = QStringLiteral("group[%1].dataset[%2]")
                         .arg(QString::number(g.groupId), QString::number(d.datasetId));

  if (!sourceIds.contains(d.sourceId)) {
    ok = false;
    addIssue(issues,
             QStringLiteral("error"),
             dloc,
             QStringLiteral("Dataset references missing source id %1").arg(d.sourceId));
  }

  if (d.title.trimmed().isEmpty())
    addIssue(issues, QStringLiteral("warning"), dloc, QStringLiteral("Dataset has no title"));

  if (!d.virtual_ && d.index <= 0)
    addIssue(issues,
             QStringLiteral("warning"),
             dloc,
             QStringLiteral("Non-virtual dataset has index 0; nothing maps to it"));

  if (!d.virtual_ && datasetIndexes.contains(d.index)) {
    ok = false;
    addIssue(issues,
             QStringLiteral("error"),
             dloc,
             QStringLiteral("Dataset index %1 is already used in this group").arg(d.index));
  }
  datasetIndexes.insert(d.index);

  if (d.fft && d.fftSamples <= 0) {
    ok = false;
    addIssue(
      issues, QStringLiteral("error"), dloc, QStringLiteral("FFT enabled but fftSamples is 0"));
  }
}

/**
 * @brief Validates each group and its datasets against the source-id set.
 */
static void validateGroups(const std::vector<DataModel::Group>& groups,
                           const QSet<int>& sourceIds,
                           QJsonArray& issues,
                           bool& ok)
{
  for (const auto& g : groups) {
    const QString gloc = QStringLiteral("group[%1]").arg(g.groupId);

    if (g.title.trimmed().isEmpty())
      addIssue(issues, QStringLiteral("warning"), gloc, QStringLiteral("Group has no title"));

    if (g.datasets.empty())
      addIssue(issues,
               QStringLiteral("warning"),
               gloc,
               QStringLiteral("Group has no datasets; nothing to display"));

    if (!sourceIds.contains(g.sourceId)) {
      ok = false;
      addIssue(issues,
               QStringLiteral("error"),
               gloc,
               QStringLiteral("Group references missing source id %1").arg(g.sourceId));
    }

    QSet<int> datasetIndexes;
    for (const auto& d : g.datasets)
      validateDataset(d, g, sourceIds, datasetIndexes, issues, ok);
  }
}

/**
 * @brief Validates actions (non-blocking sanity checks: titles, payloads).
 */
static void validateActions(const std::vector<DataModel::Action>& actions, QJsonArray& issues)
{
  for (const auto& a : actions) {
    const QString aloc = QStringLiteral("action[%1]").arg(a.actionId);
    if (a.title.trimmed().isEmpty())
      addIssue(issues, QStringLiteral("warning"), aloc, QStringLiteral("Action has no title"));

    if (a.txData.isEmpty())
      addIssue(
        issues, QStringLiteral("warning"), aloc, QStringLiteral("Action has no payload (txData)"));
  }
}

/**
 * @brief Walks the project model, reports inconsistencies.
 */
API::CommandResponse API::Handlers::ProjectHandler::validate(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& project = DataModel::ProjectModel::instance();
  const auto& groups  = project.groups();
  const auto& sources = project.sources();
  const auto& actions = project.actions();

  QJsonArray issues;
  bool ok = true;

  // Build set of valid source ids for fast lookup.
  QSet<int> sourceIds;
  for (const auto& src : sources)
    sourceIds.insert(src.sourceId);

  // Empty-project guard rail.
  if (groups.empty()) {
    addIssue(issues,
             QStringLiteral("warning"),
             QStringLiteral("project"),
             QStringLiteral("Project has no groups; no data will be visualized"));
  }

  validateSources(sources, issues, ok);
  validateGroups(groups, sourceIds, issues, ok);
  validateActions(actions, issues);

  QJsonObject result;
  result[QStringLiteral("ok")]          = ok;
  result[QStringLiteral("issues")]      = issues;
  result[QStringLiteral("groupCount")]  = static_cast<int>(groups.size());
  result[QStringLiteral("sourceCount")] = static_cast<int>(sources.size());
  result[QStringLiteral("actionCount")] = static_cast<int>(actions.size());
  result[QStringLiteral("issueCount")]  = issues.size();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Script dry-run helpers (compile + run in throwaway engines, never touch project)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the right script engine for a language tag (0=JS, 1=Lua).
 */
static std::unique_ptr<DataModel::IScriptEngine> makeScriptEngine(int language)
{
  if (language == 1)
    return std::make_unique<DataModel::LuaScriptEngine>();

  return std::make_unique<DataModel::JsScriptEngine>();
}

/**
 * @brief Frame parser dry-run: compile + parse a sample frame.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserDryRun(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  const bool hasFrame  = params.contains(QStringLiteral("sampleFrame"));
  const bool hasFrames = params.contains(QStringLiteral("sampleFrames"));
  if (!hasFrame && !hasFrames)
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: sampleFrame (string) or sampleFrames (array)"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  // Collect frame samples in execution order
  QStringList frames;
  if (hasFrames)
    for (const auto& v : params.value(QStringLiteral("sampleFrames")).toArray())
      frames.append(v.toString());
  else
    frames.append(params.value(QStringLiteral("sampleFrame")).toString());

  auto engine = makeScriptEngine(language);
  if (!engine->loadScript(code, /*sourceId=*/0, /*showMessageBoxes=*/false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Frame parser failed to compile or define parse(frame)"));

  // Run all frames sequentially through the same engine to expose stateful parsers
  QJsonArray frameResults;
  int totalRows = 0;
  for (const auto& sample : frames) {
    const auto parsed = engine->parseString(sample);

    QJsonArray rows;
    for (const auto& row : parsed) {
      QJsonArray cells;
      for (const auto& cell : row)
        cells.append(cell);

      rows.append(cells);
    }

    QJsonObject perFrame;
    perFrame[QStringLiteral("rows")]     = rows;
    perFrame[QStringLiteral("rowCount")] = rows.size();
    frameResults.append(perFrame);
    totalRows += rows.size();
  }

  QJsonObject result;
  result[QStringLiteral("ok")]         = true;
  result[QStringLiteral("frames")]     = frameResults;
  result[QStringLiteral("frameCount")] = frameResults.size();
  result[QStringLiteral("totalRows")]  = totalRows;

  // Back-compat: when caller used sampleFrame (string), also return flat rows[]
  if (hasFrame && !hasFrames && !frameResults.isEmpty()) {
    result[QStringLiteral("rows")]     = frameResults.first().toObject().value("rows");
    result[QStringLiteral("rowCount")] = frameResults.first().toObject().value("rowCount");
  }

  result[QStringLiteral("hint")] =
    QStringLiteral("Frames run sequentially through one engine instance, exposing state in "
                   "stateful parsers. row[i] in the live builder maps to dataset.index = i+1.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Compile-only check for a frame parser; surfaces syntax errors without running.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserDryCompile(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  if (language != 0 && language != 1)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript) or 1 (Lua)"));

  auto engine   = makeScriptEngine(language);
  const bool ok = engine->loadScript(code, /*sourceId=*/0, /*showMessageBoxes=*/false);

  QJsonObject result;
  result[QStringLiteral("ok")] = ok;
  result[QStringLiteral("language")] =
    (language == 1 ? QStringLiteral("lua") : QStringLiteral("javascript"));

  if (!ok) {
    result[QStringLiteral("error")] =
      QStringLiteral("Compile failed or parse(frame) is not defined.");

    // Surface a heuristic language-mismatch nudge when applicable
    const auto warning = detectLanguageMismatch(code, language);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Transform dry-run: compile + apply transform() to a list of values.
 */
API::CommandResponse API::Handlers::ProjectHandler::transformDryRun(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  if (!params.contains(QStringLiteral("values")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: values"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();
  const auto values   = params.value(QStringLiteral("values")).toArray();

  // Wrap user transform() in a parser-shaped parse(frame) for either language
  QString wrapped;
  if (language == 1) {
    wrapped = code
            + QStringLiteral("\n\nfunction parse(frame)\n"
                             "  local v = tonumber(frame)\n"
                             "  if v == nil then v = frame end\n"
                             "  local out = transform(v)\n"
                             "  return { tostring(out) }\n"
                             "end\n");
  } else {
    wrapped = code
            + QStringLiteral("\n\nfunction parse(frame) {\n"
                             "  var v = parseFloat(frame);\n"
                             "  if (isNaN(v)) v = frame;\n"
                             "  return [String(transform(v))];\n"
                             "}\n");
  }

  auto engine = makeScriptEngine(language);
  if (!engine->loadScript(wrapped, /*sourceId=*/0, /*showMessageBoxes=*/false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Transform failed to compile or define transform(value)"));

  QJsonArray outputs;
  for (const auto& v : values) {
    QString sample;
    if (v.isDouble())
      sample = QString::number(v.toDouble(), 'g', 17);
    else
      sample = v.toString();

    const auto rows = engine->parseString(sample);
    if (rows.isEmpty() || rows.first().isEmpty()) {
      outputs.append(QJsonValue::Null);
      continue;
    }

    const auto cell = rows.first().first();
    bool isNum      = false;
    const auto num  = cell.toDouble(&isNum);
    if (isNum)
      outputs.append(num);
    else
      outputs.append(cell);
  }

  QJsonObject result;
  result[QStringLiteral("ok")]      = true;
  result[QStringLiteral("outputs")] = outputs;
  result[QStringLiteral("hint")] =
    QStringLiteral("outputs[i] is the result of transform(values[i]). null means transform "
                   "returned a non-finite value -- the live runtime falls back to the raw "
                   "value in that case.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Painter dry-run: verify the script compiles and exposes paint().
 */
API::CommandResponse API::Handlers::ProjectHandler::painterDryRun(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto code = params.value(QStringLiteral("code")).toString();

  // Throwaway engine for syntax + paint() definition check
  QJSEngine engine;
  engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  // Stub the bridge globals so top-level reads don't ReferenceError.
  auto stub = engine.evaluate(
    QStringLiteral("var datasets = []; datasets.length = 0;"
                   "var group = { id: 0, title: '', columns: 0, sourceId: 0 };"
                   "var frame = { number: 0, timestampMs: 0 };"
                   "var theme = new Proxy({}, { get: function() { return '#000000'; } });"
                   "function tableGet() { return 0; }"
                   "function tableSet() {}"
                   "function datasetGetRaw() { return 0; }"
                   "function datasetGetFinal() { return 0; }"));
  if (stub.isError())
    return CommandResponse::makeError(id,
                                      ErrorCode::ExecutionError,
                                      QStringLiteral("Painter dry-run bootstrap failed: %1")
                                        .arg(stub.property(QStringLiteral("message")).toString()));

  const auto compiled = engine.evaluate(code, QStringLiteral("painter_dryrun.js"));
  if (compiled.isError()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      compiled.property(QStringLiteral("message")).toString();
    result[QStringLiteral("line")] = compiled.property(QStringLiteral("lineNumber")).toInt();
    return CommandResponse::makeSuccess(id, result);
  }

  const auto paintFn = engine.globalObject().property(QStringLiteral("paint"));
  if (!paintFn.isCallable()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      QStringLiteral("Script compiled but did not define paint(ctx, w, h). Painter scripts "
                     "MUST define `function paint(ctx, w, h)`. The function is named "
                     "`paint`, not `draw` or `render`.");
    return CommandResponse::makeSuccess(id, result);
  }

  const auto onFrameFn = engine.globalObject().property(QStringLiteral("onFrame"));
  QJsonObject result;
  result[QStringLiteral("ok")]         = true;
  result[QStringLiteral("hasPaint")]   = true;
  result[QStringLiteral("hasOnFrame")] = onFrameFn.isCallable();
  result[QStringLiteral("hint")] =
    QStringLiteral("Compile + paint() lookup succeeded. Note: dry-run does NOT actually "
                   "render -- runtime errors inside paint() (out-of-bounds reads, missing "
                   "moveTo before arc, etc.) only surface when the live widget mounts.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Wraps a transform() script so it can be driven via IScriptEngine::parseString.
 */
static QString wrapTransformForParser(const QString& code, int language)
{
  if (language == 1)
    return code
         + QStringLiteral("\n\nfunction parse(frame)\n"
                          "  local v = tonumber(frame)\n"
                          "  if v == nil then v = frame end\n"
                          "  local out = transform(v)\n"
                          "  return { tostring(out) }\n"
                          "end\n");

  return code
       + QStringLiteral("\n\nfunction parse(frame) {\n"
                        "  var v = parseFloat(frame);\n"
                        "  if (isNaN(v)) v = frame;\n"
                        "  return [String(transform(v))];\n"
                        "}\n");
}

/**
 * @brief Applies a single dataset's transform to a raw cell value via a cached engine.
 */
static QJsonValue applyTransformForDryRun(
  const DataModel::Dataset& dataset,
  int defaultLanguage,
  const QString& rawCell,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& engines,
  std::map<int, bool>& engineOk)
{
  const int datasetKey = dataset.uniqueId;
  const int language =
    (dataset.transformLanguage == -1) ? defaultLanguage : dataset.transformLanguage;

  auto it = engines.find(datasetKey);
  if (it == engines.end()) {
    auto engine    = makeScriptEngine(language);
    const auto src = wrapTransformForParser(dataset.transformCode, language);
    const bool ok  = engine->loadScript(src, dataset.sourceId, /*showMessageBoxes=*/false);

    engineOk[datasetKey] = ok;
    engines[datasetKey]  = std::move(engine);
    it                   = engines.find(datasetKey);
  }

  if (!engineOk[datasetKey])
    return QJsonValue::Null;

  const auto rows = it->second->parseString(rawCell);
  if (rows.isEmpty() || rows.first().isEmpty())
    return QJsonValue::Null;

  const auto cell = rows.first().first();
  bool isNum      = false;
  const auto num  = cell.toDouble(&isNum);
  if (isNum)
    return num;

  return cell;
}

/**
 * @brief Build a single dataset entry for an endToEndDryRun row.
 */
static QJsonObject buildDryRunDatasetEntry(
  const DataModel::Dataset& dataset,
  int groupId,
  const QStringList& row,
  int language,
  bool verbose,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& transformEngines,
  std::map<int, bool>& transformEngineOk)
{
  const int idx = dataset.index;
  QString rawCell;
  if (idx >= 1 && idx <= row.size())
    rawCell = row.at(idx - 1);

  QJsonObject entry;
  entry[Keys::UniqueId]              = dataset.uniqueId;
  entry[Keys::Title]                 = dataset.title;
  entry[Keys::GroupId]               = groupId;
  entry[Keys::DatasetId]             = dataset.datasetId;
  entry[Keys::Index]                 = idx;
  entry[QStringLiteral("isVirtual")] = dataset.virtual_;

  if (verbose)
    entry[QStringLiteral("raw")] = rawCell;

  if (!dataset.transformCode.isEmpty()) {
    entry[QStringLiteral("final")] =
      applyTransformForDryRun(dataset, language, rawCell, transformEngines, transformEngineOk);
    entry[QStringLiteral("transformApplied")] = true;
    return entry;
  }

  bool isNum                                = false;
  const auto num                            = rawCell.toDouble(&isNum);
  entry[QStringLiteral("final")]            = isNum ? QJsonValue(num) : QJsonValue(rawCell);
  entry[QStringLiteral("transformApplied")] = false;
  return entry;
}

/**
 * @brief Build a single parsed-row payload for an endToEndDryRun frame.
 */
static QJsonObject buildDryRunRow(
  const QStringList& row,
  int sourceId,
  const std::vector<DataModel::Group>& groups,
  int language,
  bool verbose,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& transformEngines,
  std::map<int, bool>& transformEngineOk)
{
  QJsonArray datasetResults;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (dataset.sourceId != sourceId)
        continue;

      datasetResults.append(buildDryRunDatasetEntry(
        dataset, group.groupId, row, language, verbose, transformEngines, transformEngineOk));
    }
  }

  QJsonArray rawCells;
  for (const auto& cell : row)
    rawCells.append(cell);

  QJsonObject rowOut;
  rowOut[QStringLiteral("rawCells")] = rawCells;
  rowOut[QStringLiteral("datasets")] = datasetResults;
  return rowOut;
}

/**
 * @brief End-to-end dry-run: parser + all dataset transforms applied to a sample frame.
 */
API::CommandResponse API::Handlers::ProjectHandler::endToEndDryRun(const QString& id,
                                                                   const QJsonObject& params)
{
  // Resolve source + parser code (override or live project)
  auto& pm           = DataModel::ProjectModel::instance();
  const auto sources = pm.sources();
  const int sourceId = params.value(Keys::SourceId).toInt(0);
  if (sources.empty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Project has no sources to dry-run against"));

  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Source id out of range: %1").arg(sourceId));

  const auto& source = sources[sourceId];

  // Collect sample frames
  const bool hasFrame  = params.contains(QStringLiteral("sampleFrame"));
  const bool hasFrames = params.contains(QStringLiteral("sampleFrames"));
  if (!hasFrame && !hasFrames)
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: sampleFrame (string) or sampleFrames (array)"));

  QStringList frames;
  if (hasFrames)
    for (const auto& v : params.value(QStringLiteral("sampleFrames")).toArray())
      frames.append(v.toString());
  else
    frames.append(params.value(QStringLiteral("sampleFrame")).toString());

  const bool verbose = params.value(QStringLiteral("verbose")).toBool(false);
  const QString code = params.contains(QStringLiteral("code"))
                       ? params.value(QStringLiteral("code")).toString()
                       : source.frameParserCode;
  const int language = params.contains(QStringLiteral("language"))
                       ? params.value(QStringLiteral("language")).toInt()
                       : source.frameParserLanguage;

  // Compile parser
  auto parser = makeScriptEngine(language);
  if (!parser->loadScript(code, sourceId, /*showMessageBoxes=*/false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Frame parser failed to compile or define parse(frame)"));

  // Cache transform engines across frames so stateful transforms reveal behavior
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>> transformEngines;
  std::map<int, bool> transformEngineOk;
  const auto& groups = pm.groups();

  QJsonArray frameResults;
  for (const auto& sample : frames) {
    const auto parsed = parser->parseString(sample);

    QJsonArray rowResults;
    for (const auto& row : parsed)
      rowResults.append(buildDryRunRow(
        row, sourceId, groups, language, verbose, transformEngines, transformEngineOk));

    QJsonObject perFrame;
    perFrame[QStringLiteral("rows")]     = rowResults;
    perFrame[QStringLiteral("rowCount")] = rowResults.size();
    frameResults.append(perFrame);
  }

  // Surface which transforms failed to compile so the caller can fix them
  QJsonArray failedTransforms;
  for (const auto& [uid, ok] : transformEngineOk)
    if (!ok)
      failedTransforms.append(uid);

  QJsonObject result;
  result[QStringLiteral("ok")]         = true;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("frames")]     = frameResults;
  result[QStringLiteral("frameCount")] = frameResults.size();

  if (!failedTransforms.isEmpty()) {
    result[QStringLiteral("transformCompileFailures")] = failedTransforms;
    result[QStringLiteral("warning")] =
      QStringLiteral("One or more dataset transforms failed to compile. Their `final` "
                     "values are null. Iterate the failing transforms via "
                     "project.dataset.transform.dryRun, then setTransformCode.");
  }

  result[QStringLiteral("hint")] =
    QStringLiteral("rawCells[i] maps to dataset.index = i+1. The table API "
                   "(tableGet/tableSet/datasetGetRaw/datasetGetFinal) is NOT injected in "
                   "this dry-run -- transforms that read other datasets will see 0/null. "
                   "Virtual datasets show their transform applied to the raw cell at "
                   "their index (which is normally 0/unset for virtual entries).");
  return CommandResponse::makeSuccess(id, result);
}
