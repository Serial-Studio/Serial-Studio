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
#include <QRegularExpression>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"
#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Base.h"
#endif
#include "ProjectApiSupport.h"

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
  registerHistoryCommands();
}

/**
 * @brief Register project.undo / project.redo commands.
 */
void API::Handlers::ProjectHandler::registerHistoryCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.undo"),
    QStringLiteral("Undo the most recent project-document mutation (from any surface: "
                   "editor UI, API, or batch -- one shared history). A batch or cascade "
                   "reverts atomically as one step. Returns {performed:false, reason} "
                   "when history is empty; never an error. History clears on project "
                   "load/new. Not for workspace or widget-layout edits (those are "
                   "outside undo history)."),
    empty,
    &projectUndo);

  registry.registerCommand(
    QStringLiteral("project.redo"),
    QStringLiteral("Replay the most recently undone project-document step. Returns "
                   "{performed:false, reason} when there is nothing to redo. Any new "
                   "mutation after an undo discards the redo tail."),
    empty,
    &projectRedo);
}

/**
 * @brief Register project new/open/save/loadJson/setTitle commands.
 */
void API::Handlers::ProjectHandler::registerFileLifecycleCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

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
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

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
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

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
  registerDatasetMarkerCommands();
}

/**
 * @brief Registers project.dataset.add and project.dataset.addMany.
 */
void API::Handlers::ProjectHandler::registerDatasetCreateCommands()
{
  static auto& registry = CommandRegistry::instance();

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
  static auto& registry = CommandRegistry::instance();

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
  static auto& registry = CommandRegistry::instance();

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
  static auto& registry = CommandRegistry::instance();

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
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.getAlarmBands"),
    QStringLiteral("Returns the dataset's coloured alarm bands as a JSON array. Each entry: "
                   "{min, max, severity (0=Info, 1=OK, 2=Warning, 3=Critical), color "
                   "(\"#rrggbb\" override or empty for severity default), label, blink}. Also "
                   "returns rangeMin/rangeMax (the dataset's wgtMin/wgtMax) so callers can "
                   "validate band ranges before writing back. An empty array means no alarms "
                   "configured. Applies to bar / gauge / meter widgets and LED-panel datasets; "
                   "calling on other widget types succeeds with an empty array."),
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
                   "(surfaces in band-edge notifications). blink is optional (boolean; LED "
                   "panels flash the LED while the band is active). Bands may have gaps and "
                   "may overlap; rendering paints them in array order behind the value "
                   "indicator. Severity >= Warning triggers a notification when the value "
                   "enters the band, even when no widget is visible (3-second per-dataset "
                   "cooldown suppresses oscillation spam).\n"
                   "Pass an empty array to clear all alarms."),
    makeSchema({
      {                     QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                             },
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
       "notifications)")}}},
       {QStringLiteral("blink"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")},
       {QStringLiteral("description"),
       QStringLiteral("Optional; LED panels flash the LED "
       "while this band is active")}}}}},
       {QStringLiteral("required"),
       QJsonArray{
       QStringLiteral("min"), QStringLiteral("max"), QStringLiteral("severity")}}}
      )
  }),
    &datasetSetAlarmBands);
}

/**
 * @brief Register dataset FFT frequency-marker getter / setter commands.
 */
void API::Handlers::ProjectHandler::registerDatasetMarkerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.getFFTMarkers"),
    QStringLiteral("Returns the dataset's FFT frequency markers as a JSON array. Each entry: "
                   "{freq (Hz), endFreq (optional; present only for band markers), label, "
                   "color (\"#rrggbb\" override or empty for the theme accent), warningDb, "
                   "alarmDb (optional display-dB thresholds; omitted when unset)}. Also "
                   "returns nyquist (fftSamplingRate / 2) so callers can validate frequencies "
                   "before writing back. An empty array means no markers configured."),
    makeSchema({
      {  QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                  },
      {QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")}
  }),
    &datasetGetFFTMarkers);

  registry.registerCommand(
    QStringLiteral("project.dataset.setFFTMarkers"),
    QStringLiteral("Atomically replaces the dataset's fftMarkers array (FFT plot + waterfall "
                   "frequency markers). Each entry must provide a finite freq > 0 in Hz; "
                   "invalid entries are silently dropped and counted in result.droppedInvalid. "
                   "endFreq is optional (> freq turns the marker into a band; otherwise it is "
                   "a point marker). label and color (\"#rrggbb\") are optional. warningDb and "
                   "alarmDb are optional display-dB thresholds evaluated against the rendered "
                   "spectrum; when both are present and reversed they are swapped. Pass an "
                   "empty array to clear all markers."),
    makeSchema({
      {                     QString(Keys::GroupId),QStringLiteral("integer"),QStringLiteral("Owning group id")                                     },
      {                          QString(Keys::DatasetId),
       QStringLiteral("integer"),
       QStringLiteral("Dataset id within the group")                   },
      arrayProp(
        QString(Keys::FFTMarkers),
        QStringLiteral("Full replacement marker list (empty = no markers)."),
        QJsonObject{
       {QStringLiteral("type"), QStringLiteral("object")},
       {QStringLiteral("properties"),
       QJsonObject{{QStringLiteral("freq"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Marker frequency in Hz (> 0)")}}},
       {QStringLiteral("endFreq"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional band end in Hz (> freq)")}}},
       {QStringLiteral("label"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional marker name shown on the chip")}}},
       {QStringLiteral("color"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
       {QStringLiteral("description"),
       QStringLiteral("Optional \"#rrggbb\" override; empty = "
       "theme accent")}}},
       {QStringLiteral("warningDb"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional warning level in display dB")}}},
       {QStringLiteral("alarmDb"),
       QJsonObject{{QStringLiteral("type"), QStringLiteral("number")},
       {QStringLiteral("description"),
       QStringLiteral("Optional alarm level in display dB")}}}}},
       {QStringLiteral("required"), QJsonArray{QStringLiteral("freq")}}}
      )
  }),
    &datasetSetFFTMarkers);
}

/**
 * @brief Register action CRUD commands.
 */
void API::Handlers::ProjectHandler::registerActionCommands()
{
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

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
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

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
  registerParserTemplateCommands();
  registerParserConfigCommands();
}

/**
 * @brief Register frame-parser code and language commands (set/get code, set/get language).
 */
void API::Handlers::ProjectHandler::registerParserCodeCommands()
{
  static auto& registry = CommandRegistry::instance();

  const auto setCodeSchema = makeSchema(
    {
      {QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Frame parser script code (JS or Lua), or for the Built-In language the "
                      "JSON descriptor {\"template\": id, \"params\": {...}}")}
  },
    {{QString(Keys::SourceId),
      QStringLiteral("integer"),
      QStringLiteral("Source index (default 0)")},
     {QStringLiteral("language"),
      QStringLiteral("integer"),
      QStringLiteral("Optional: 0 = JavaScript, 1 = Lua, 2 = Built-In (parametrized C++ "
                     "template). When supplied, the source language is flipped before the "
                     "code is validated and script errors are returned as API errors.")}});

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
                                          "and the tableGet/tableSet API. For Built-In (2), "
                                          "prefer project.frameParser.setTemplate; passing "
                                          "the JSON descriptor as `code` also works."),
                           setCodeSchema,
                           &parserSetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.getCode"),
                           QStringLiteral("Read the current frame parser source for a "
                                          "given data source. Returns {code, language}; "
                                          "Built-In sources also return {template, params} "
                                          "and `code` carries the JSON descriptor. "
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
                           QStringLiteral("Switch a source between JavaScript, Lua and Built-In "
                                          "frame parsers. WARNING: for JS/Lua this WIPES any "
                                          "existing frameParser code for that source -- the "
                                          "loaded default template for the new language "
                                          "replaces it. If you want to preserve+translate, "
                                          "frameParser.getCode first, switch, then "
                                          "frameParser.setCode with the translated source. "
                                          "Switching to Built-In (2) seeds the default "
                                          "'delimited' template and leaves JS/Lua code "
                                          "intact for round-trips."),
                           makeSchema(
                             {
                               {QStringLiteral("language"),
                                QStringLiteral("integer"),
                                QStringLiteral("Script language: 0 = JavaScript, 1 = Lua, "
                                               "2 = Built-In (parametrized C++ template)")}
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
 * @brief Register Native frame-parser template discovery / configuration commands.
 */
void API::Handlers::ProjectHandler::registerParserTemplateCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.frameParser.listTemplates"),
    QStringLiteral("List the Built-In (C++) frame parser templates. Returns {templates: "
                   "[{id, name, description}], count}. Built-In templates parse without "
                   "user code and are the fastest option; configure one with "
                   "project.frameParser.setTemplate after inspecting its parameters via "
                   "project.frameParser.getTemplateSchema."),
    emptySchema(),
    &parserListTemplates);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getTemplateSchema"),
    QStringLiteral("Get the parameter schema of a Built-In frame parser template. Returns "
                   "{id, name, description, params: [{key, type, label, description, "
                   "default, options?, min?, max?}]}. Param types: string, char, int, "
                   "float, bool, enum (enum values come from options[].value)."),
    makeSchema({
      {QStringLiteral("template"),
       QStringLiteral("string"),
       QStringLiteral("Template id from project.frameParser.listTemplates")}
  }),
    &parserGetTemplateSchema);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getTemplate"),
    QStringLiteral("Get the Built-In frame parser configuration for a source. Returns "
                   "{sourceId, language, template, params}; template is empty when the "
                   "source never used the Built-In language."),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")}}),
    &parserGetTemplate);

  registry.registerCommand(
    QStringLiteral("project.frameParser.setTemplate"),
    QStringLiteral("Select a Built-In frame parser template for a source and switch the "
                   "source to the Built-In language. Params are validated against the "
                   "template schema; omitted params use the schema defaults. Use "
                   "project.frameParser.dryRun (language 2, descriptor as code) to "
                   "preview the output before or after applying."),
    makeSchema(
      {
        {QStringLiteral("template"),
         QStringLiteral("string"),
         QStringLiteral("Template id from project.frameParser.listTemplates")}
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")},
       {QStringLiteral("params"),
        QStringLiteral("object"),
        QStringLiteral("Template parameters (see getTemplateSchema); omitted keys use "
                       "schema defaults")}}),
    &parserSetTemplate);
}

/**
 * @brief Register frame-parser configuration commands (delimiters, checksum, mode, getConfig).
 */
void API::Handlers::ProjectHandler::registerParserConfigCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.frameParser.update"),
    QStringLiteral("Configure frame extraction settings on a source (params: startSequence, "
                   "endSequence, checksumAlgorithm, frameDetection, decoderMethod, "
                   "hexadecimalDelimiters, operationMode). This is the persist path for "
                   "delimiters -- parser code alone (setCode) never changes them."),
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
       {QString(Keys::DecoderMethod),
        QStringLiteral("integer"),
        QStringLiteral("Decoder method (0=PlainText, 1=Hex, 2=Base64, 3=Binary)")},
       {QString(Keys::HexadecimalDelimiters),
        QStringLiteral("boolean"),
        QStringLiteral("Treat startSequence/endSequence as hex byte sequences")},
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
  static auto& registry = CommandRegistry::instance();
  const auto empty      = emptySchema();

  registry.registerCommand(
    QStringLiteral("project.group.list"),
    QStringLiteral("List all groups with dataset counts. Large projects: pass offset/limit to "
                   "page (reply carries window/nextOffset/projectEpoch); groupCount always "
                   "reflects the whole project. Prefer project.search to find a group by name "
                   "and project.group.get to inspect one group."),
    makeSchema(
      {
  },
      {{QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First group index to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max groups to return; omit or <=0 for all.")}}),
    &groupsList);
  registry.registerCommand(
    QStringLiteral("project.dataset.list"),
    QStringLiteral("List all datasets across all groups. Large projects: pass offset/limit to "
                   "page the flattened dataset array (reply carries window/nextOffset/"
                   "projectEpoch); datasetCount always reflects the whole project. Prefer "
                   "project.search to find datasets by partial name."),
    makeSchema(
      {
  },
      {{QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First dataset index to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max datasets to return; omit or <=0 for all.")}}),
    &datasetsList);
  registry.registerCommand(
    QStringLiteral("project.action.list"), QStringLiteral("List all actions"), empty, &actionsList);

  registerResolverCommands();
  registerDiscoveryCommands();
  registerSnapshotAndMoveCommands();
}

/**
 * @brief Register the discovery commands (project.search, project.group.get).
 */
void API::Handlers::ProjectHandler::registerDiscoveryCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.search"),
    QStringLiteral("Case-insensitive substring search across every project entity type: "
                   "datasets (title, alias, units), groups, actions, sources, workspaces, and "
                   "data tables (title/name). Returns compact typed rows -- never full objects "
                   "-- with matchCount, window/nextOffset paging (default limit 20, max 100), "
                   "and projectEpoch. THE starting point for finding anything in a large "
                   "project; drill into hits with project.dataset.getByUniqueId / getByPath or "
                   "project.group.get."),
    makeSchema(
      {
        {QStringLiteral("query"),
         QStringLiteral("string"),
         QStringLiteral("Substring to find (case-insensitive); must be non-empty.")}
  },
      {{QStringLiteral("type"),
        QStringLiteral("string|array"),
        QStringLiteral("Restrict to one or more of: dataset, group, action, source, "
                       "workspace, table.")},
       {QString(Keys::GroupId),
        QStringLiteral("integer"),
        QStringLiteral("Only datasets in this positional groupId.")},
       {QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Only entities bound to this sourceId.")},
       {QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First match to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max rows to return (default 20, max 100).")}}),
    &projectSearch);

  registry.registerCommand(
    QStringLiteral("project.group.get"),
    QStringLiteral("Read one group plus a compact dataset summary (datasetId, uniqueId, index, "
                   "title, units) without pulling the whole project. Address it by EITHER the "
                   "positional groupId (what group.update / dataset CRUD take) OR the stable "
                   "uniqueId (what workspace widget refs carry under the key 'groupId'); the "
                   "response returns both ids. The summary pages via offset/limit (default 50, "
                   "max 200)."),
    makeSchema(
      {
  },
      {{QString(Keys::GroupId),
        QStringLiteral("integer"),
        QStringLiteral("Positional group id (0..N-1, shifts on reorder). Mutually exclusive "
                       "with uniqueId.")},
       {QString(Keys::UniqueId),
        QStringLiteral("integer"),
        QStringLiteral("Stable persisted group id (sparse counter; what workspace refs "
                       "carry). Mutually exclusive with groupId.")},
       {QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First dataset-summary row to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max dataset-summary rows (default 50, max 200).")}}),
    &groupGet);
}

/**
 * @brief Register dataset resolver commands (getByUniqueId, getByTitle, getByPath,
 * getExecutionOrder).
 */
void API::Handlers::ProjectHandler::registerResolverCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dataset.getByUniqueId"),
    QStringLiteral("Resolve a dataset by its uniqueId (number) or alias (string). Returns the "
                   "same shape as the elements of project.dataset.list."),
    makeSchema({
      {QString(Keys::UniqueId),
       QStringLiteral("integer|string"),
       QStringLiteral(
         "Dataset selector: an integer uniqueId (opaque persisted handle allocated at dataset "
         "creation, stable across reorders; read it from list/snapshot, never compute it) OR a "
         "string alias assigned in the editor. A string is always an alias, never a uniqueId.")}
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
                   "values only for datasets earlier in this list). Large projects: pass "
                   "offset/limit to page (each row carries its absolute `position`; reply "
                   "carries window/nextOffset/projectEpoch)."),
    makeSchema(
      {
  },
      {{QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("First execution position to return (default 0).")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max rows to return; omit or <=0 for all.")}}),
    &datasetGetExecutionOrder);
}

/**
 * @brief Register the project.snapshot composite read and dataset/group move commands.
 */
void API::Handlers::ProjectHandler::registerSnapshotAndMoveCommands()
{
  static auto& registry = CommandRegistry::instance();

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
                   "so you can refetch before acting on now-shifted uniqueIds.\n"
                   "Large projects: `sections` picks which blocks to include, and "
                   "offset/limit windows the groups array (reply carries nextOffset); "
                   "groupCount/datasetCount always reflect the whole project."),
    makeSchema(
      {
  },
      {{QStringLiteral("verbose"),
        QStringLiteral("boolean"),
        QStringLiteral("Include frame parser source and source-level frame settings.")},
       typedArrayProp(QStringLiteral("sections"),
                      QStringLiteral("Sections to include: any of \"sources\", \"groups\", "
                                     "\"workspaces\", \"dataTables\" (default: all)."),
                      QStringLiteral("string")),
       {QStringLiteral("offset"),
        QStringLiteral("integer"),
        QStringLiteral("Skip this many groups before returning results (default 0); pass the "
                       "nextOffset from a previous reply to page through large projects.")},
       {QStringLiteral("limit"),
        QStringLiteral("integer"),
        QStringLiteral("Max groups to return (default 0 = all).")}}),
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
         QStringLiteral("integer|string"),
         QStringLiteral("Dataset selector to move: integer uniqueId or string alias.")   },
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
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.painter.setCode"),
    QStringLiteral("Set the painter widget code for a group (params: groupId, code). "
                   "**JavaScript only** -- painter scripts run in QJSEngine, not Lua. "
                   "Available globals: ctx (2D canvas context, QPainter-like), w, h "
                   "(canvas dimensions), datasetGetFinal(uid)/datasetGetRaw(uid). The "
                   "entry point is paint(ctx, w, h) and an optional zero-arg onFrame() "
                   "callback. "
                   "Validate with project.painter.dryRun before setCode. **Always call "
                   "meta.fetchScriptingDocs{kind:'painter_js'} first** for the full API "
                   "surface and worked examples -- don't invent canvas methods from JS "
                   "DOM Canvas, the surface is QPainter-shaped."),
    makeSchema({
      {QStringLiteral("groupId"),
       QStringLiteral("integer"),
       QStringLiteral("Target group id (from project.group.list)")              },
      {   QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Painter widget JS source. Must define paint(ctx, w, h) and may "
       "define a zero-arg onFrame(). Replaces any existing code for the group.")}
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
 * @brief Register patching update commands for groups/datasets/actions/outputs.
 */
/**
 * @brief Registers project.group.update and project.dataset.update.
 */
void API::Handlers::ProjectHandler::registerEntityUpdateCommands()
{
  static auto& registry = CommandRegistry::instance();

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

/**
 * @brief Registers the project.batch multi-op endpoint.
 */
void API::Handlers::ProjectHandler::registerBatchCommand()
{
  static auto& registry = CommandRegistry::instance();

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
  registerFrameParserDryRunCommands();
  registerScriptDryRunCommands();
}

/**
 * @brief Register frame-parser dryRun + dryCompile endpoints.
 */
void API::Handlers::ProjectHandler::registerFrameParserDryRunCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.frameParser.dryRun"),
    QStringLiteral(
      "Compile and execute frame parser code against raw stream bytes WITHOUT touching the live "
      "project. Drives the full pipeline: extraction (delimiters / detection) -> decoder switch "
      "-> parse(). Identical code path to the live FrameBuilder and the frame parser test "
      "dialog. "
      "Required: code, language, inputBytesHex (preferred; binary-safe) or inputBytes (UTF-8 "
      "text). Recommended: decoderMethod + frameDetection + frameStart / frameEnd + "
      "checksumAlgorithm, otherwise sensible defaults apply (PlainText, EndDelimiterOnly, no "
      "delimiters, no checksum). Returns per-frame raw / decoder / rows plus extractedCount / "
      "consumedBytes / remainingBytes / droppedFrames. For binary protocols (COBS, Modbus, "
      "custom binary) pick decoderMethod=3 (Binary); the text decoders run through "
      "QString::fromUtf8 and corrupt non-ASCII bytes."),
    makeSchema(
      {
        {    QStringLiteral("code"),QStringLiteral("string"),QStringLiteral("Frame parser source")            },
        {QStringLiteral("language"),
         QStringLiteral("integer"),
         QStringLiteral("0 = JavaScript, 1 = Lua")}
  },
      {{QStringLiteral("inputBytes"),
        QStringLiteral("string"),
        QStringLiteral("Raw stream bytes as UTF-8 text. Lossy for binary payloads -- prefer "
                       "inputBytesHex for COBS / Modbus / non-ASCII. One of inputBytes / "
                       "inputBytesHex must be a non-empty string.")},
       {QStringLiteral("inputBytesHex"),
        QStringLiteral("string"),
        QStringLiteral("Raw stream bytes as a hex string (space-tolerant). Binary-safe; use "
                       "this for COBS or any non-ASCII protocol. One of inputBytes / "
                       "inputBytesHex must be a non-empty string.")},
       {QString(Keys::DecoderMethod),
        QStringLiteral("integer"),
        QStringLiteral("0=PlainText (default; UTF-8 -> QString, mojibakes binary), "
                       "1=Hexadecimal (toHex -> QString), 2=Base64 (toBase64 -> QString), "
                       "3=Binary (raw QByteArray, only mode that's safe for binary "
                       "protocols).")},
       {QString(Keys::FrameDetection),
        QStringLiteral("integer"),
        QStringLiteral("0=EndDelimiterOnly (default), 1=StartAndEndDelimiter, 2=NoDelimiters, "
                       "3=StartDelimiterOnly.")},
       {QString(Keys::FrameStart),
        QStringLiteral("string"),
        QStringLiteral("Start delimiter. Hex when hexadecimalDelimiters is true. Default: "
                       "none.")},
       {QString(Keys::FrameEnd),
        QStringLiteral("string"),
        QStringLiteral("End delimiter. Hex when hexadecimalDelimiters is true. Default: "
                       "none.")},
       {QString(Keys::HexadecimalDelimiters),
        QStringLiteral("boolean"),
        QStringLiteral("When true, frameStart / frameEnd are parsed as hex bytes. Default "
                       "false.")},
       {QString(Keys::ChecksumAlgorithm),
        QStringLiteral("string"),
        QStringLiteral("Checksum name to validate trailing bytes. Empty (default) = none.")},
       {QStringLiteral("operationMode"),
        QStringLiteral("integer"),
        QStringLiteral("0=ProjectFile (default; runs decoder + parser), 2=QuickPlot (line "
                       "extractor, comma-split, parser is bypassed). 1=ConsoleOnly is invalid "
                       "for dryRun.")}}),
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
}

/**
 * @brief Register dataset-transform and painter dryRun endpoints.
 */
void API::Handlers::ProjectHandler::registerScriptDryRunCommands()
{
  static auto& registry = CommandRegistry::instance();

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

  registry.registerCommand(
    QStringLiteral("project.outputWidget.dryRun"),
    QStringLiteral("Compile an output-widget transmit function WITHOUT touching the live "
                   "project. Verifies the script compiles and defines transmit(value); "
                   "returns ok / compileError + line. The transmitFunction is **JavaScript "
                   "only** and runs with the same injected Modbus/CAN helper globals + table "
                   "API as the live widget. "
                   "Pass inputValue (and hex:true for hex byte input) to also execute it once "
                   "and return the produced bytes (outputHex + byteCount). Validate here "
                   "BEFORE project.outputWidget.update."),
    makeSchema(
      {
        {QStringLiteral("code"),
         QStringLiteral("string"),
         QStringLiteral("Transmit source. Must define transmit(value)")}
  },
      {{QStringLiteral("inputValue"),
        QStringLiteral("string"),
        QStringLiteral("Optional sample value to run transmit() against")},
       {QStringLiteral("hex"),
        QStringLiteral("boolean"),
        QStringLiteral("Treat inputValue as space-separated hex bytes. Default false.")}}),
    &outputWidgetDryRun);
}

/**
 * @brief Register the end-to-end dryRun endpoint (parser + transforms in throwaway engines).
 */
void API::Handlers::ProjectHandler::registerEndToEndDryRunCommand()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.dryRun.endToEnd"),
    QStringLiteral("End-to-end dry run: takes a sample frame body, runs the project's "
                   "frame parser, then applies every dataset's transform in execution "
                   "order, and returns the final per-dataset values WITHOUT touching "
                   "live state. Required: sampleFrame (single body) OR sampleFrames "
                   "(array of bodies); everything else defaults to the live project. "
                   "Use this to verify the full parse->transform pipeline "
                   "before issuing setCode/setTransformCode. Note: the table API "
                   "(tableGet/tableSet/datasetGetRaw/datasetGetFinal) is NOT injected; "
                   "transforms that depend on it should be tested with "
                   "project.dataset.transform.dryRun individually."),
    makeSchema(
      {
  },
      {{QStringLiteral("sampleFrame"),
        QStringLiteral("string"),
        QStringLiteral("Single frame body (without delimiters). Use sampleFrames for an "
                       "array. One of sampleFrame / sampleFrames is required.")},
       typedArrayProp(
         QStringLiteral("sampleFrames"),
         QStringLiteral("Array of frame bodies; runs sequentially in one parser engine "
                        "instance. One of sampleFrame / sampleFrames is required."),
         QStringLiteral("string")),
       {QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index to use for parser code + dataset transforms (default 0)")},
       {QStringLiteral("code"),
        QStringLiteral("string"),
        QStringLiteral("Optional override for the frame parser source (default: use live "
                       "project)")},
       {QStringLiteral("language"),
        QStringLiteral("integer"),
        QStringLiteral("Optional override: 0 = JavaScript, 1 = Lua (default: live source "
                       "language)")},
       {QStringLiteral("verbose"),
        QStringLiteral("boolean"),
        QStringLiteral("Include raw cell values alongside final transformed values (default "
                       "false)")}}),
    &endToEndDryRun);
}

/**
 * @brief Register project.template.* commands.
 */
void API::Handlers::ProjectHandler::registerTemplateCommands()
{
  static auto& registry = CommandRegistry::instance();

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
                   "  telemetry_udp: CSV body parser with 5 generic value channels, "
                   "intended for UDP telemetry. The template does NOT configure the "
                   "connection -- set up the UDP listener via io.network.* after "
                   "applying.\n"
                   "  mqtt_subscriber (Pro): MQTT subscriber-mode skeleton. After "
                   "applying, configure the broker via project.mqtt.subscriber.setConfig.\n"
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
