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

#include "API/Handlers/ProjectDryRunCommands.h"

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

#include "API/Handlers/ProjectApiSupport.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "Core/DataModel/Frame.h"
#include "Core/EnumLabels.h"
#include "Core/SerialStudio.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "DataModel/Scripting/ScriptDryRun.h"
#ifdef BUILD_COMMERCIAL
#endif

using namespace API::Handlers::ProjectApiSupport;

namespace API::Handlers {

/**
 * @brief Builds the likely-cause suffix for a frame-parser compile failure (legacy two-parameter
 *        parse signature or a language/syntax mismatch); returns an empty string when neither
 *        heuristic fires.
 */
[[nodiscard]] static QString frameParserCompileHint(const QString& code, int language)
{
  static const QRegularExpression kTwoArgParse(QStringLiteral(
    R"(\bparse\b\s*(?:=\s*)?(?:function)?\s*\(\s*[a-zA-Z_$][\w$]*\s*,\s*[a-zA-Z_$][\w$]*\s*\))"));
  if (kTwoArgParse.match(code).hasMatch())
    return QStringLiteral(" Likely cause: the code defines the legacy two-parameter "
                          "parse(frame, separator) signature, which is rejected. Define "
                          "parse(frame) with a single parameter and split the frame yourself.");

  const auto mismatch = detectLanguageMismatch(code, language);
  if (!mismatch.isEmpty())
    return QStringLiteral(" Likely cause: ") + mismatch;

  return QString();
}

}  // namespace API::Handlers

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectDryRunCommands::ProjectDryRunCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register every dryRun endpoint (compile + execute in throwaway engines).
 */
void API::Handlers::ProjectDryRunCommands::registerCommands()
{
  registerFrameParserDryRunCommands();
  registerScriptDryRunCommands();
  registerEndToEndDryRunCommand();
}

/**
 * @brief Register frame-parser dryRun + dryCompile endpoints.
 */
void API::Handlers::ProjectDryRunCommands::registerFrameParserDryRunCommands()
{
  auto& registry = m_registry;

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
        {    QStringLiteral("code"),QStringLiteral("string"),QStringLiteral("Frame parser source")                          },
        {QStringLiteral("language"),
         QStringLiteral("integer"),
         QStringLiteral("0 = JavaScript, 1 = Lua, 2 = Built-In (descriptor JSON)")}
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
void API::Handlers::ProjectDryRunCommands::registerScriptDryRunCommands()
{
  auto& registry = m_registry;

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
void API::Handlers::ProjectDryRunCommands::registerEndToEndDryRunCommand()
{
  auto& registry = m_registry;

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

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Script dry-run helpers (compile + run in throwaway engines, never touch project)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the right script engine for a language tag (0=JS, 1=Lua, 2=Native).
 */
static std::unique_ptr<DataModel::IScriptEngine> makeScriptEngine(int language)
{
  if (language == SerialStudio::Native)
    return std::make_unique<DataModel::CFrameParser>();

  if (language == SerialStudio::Lua)
    return std::make_unique<DataModel::LuaScriptEngine>();

  return std::make_unique<DataModel::JsScriptEngine>();
}

/**
 * @brief Parses a delimiter field (frameStart / frameEnd) honoring the hexadecimalDelimiters flag.
 */
static QByteArray dryRunDelimiter(const QJsonObject& params, const QString& key, bool hex)
{
  const auto raw = params.value(key).toString();
  if (raw.isEmpty())
    return {};

  if (hex) {
    const auto resolved = SerialStudio::resolveEscapeSequences(raw);
    return QByteArray::fromHex(QString(resolved).remove(' ').toUtf8());
  }

  return SerialStudio::resolveEscapeSequences(raw).toUtf8();
}

/**
 * @brief Decodes the caller-supplied raw stream bytes; non-empty inputBytesHex wins, an empty
 *        string in either field counts as absent so the other field can still supply the data.
 */
static QByteArray dryRunInputBytes(const QJsonObject& params)
{
  const auto hex = params.value(QStringLiteral("inputBytesHex")).toString();
  if (!hex.trimmed().isEmpty())
    return SerialStudio::hexToBytes(hex);

  return params.value(QStringLiteral("inputBytes")).toString().toUtf8();
}

/**
 * @brief Serializes a single pipeline frame into the dryRun response shape.
 */
static QJsonObject dryRunSerializeFrame(const DataModel::PipelineFrame& frame)
{
  QJsonArray rows;
  for (const auto& row : frame.rows) {
    QJsonArray cells;
    for (const auto& cell : row)
      cells.append(cell);

    rows.append(cells);
  }

  QJsonObject obj;
  obj[QStringLiteral("rawHex")]          = QString::fromLatin1(frame.rawBytes.toHex(' '));
  obj[QStringLiteral("rawByteCount")]    = static_cast<int>(frame.rawBytes.size());
  obj[QStringLiteral("decoderOutput")]   = frame.decoderOutput;
  obj[QStringLiteral("decoderIsBinary")] = frame.decoderProducedBinary;
  obj[QStringLiteral("rows")]            = rows;
  obj[QStringLiteral("rowCount")]        = rows.size();
  return obj;
}

}  // namespace API::Handlers

/**
 * @brief Frame parser dry-run: drives extraction + decoder + parser against caller bytes.
 */
API::CommandResponse API::Handlers::ProjectDryRunCommands::frameParserDryRun(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  if (!params.contains(QStringLiteral("inputBytes"))
      && !params.contains(QStringLiteral("inputBytesHex")))
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: inputBytesHex (preferred, binary-safe) or "
                     "inputBytes (UTF-8 text)."));

  const auto bytes = dryRunInputBytes(params);
  if (bytes.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("inputBytes / inputBytesHex were provided but decoded to zero bytes. "
                     "Pass the raw stream in exactly one of them: inputBytesHex as hex digits "
                     "(e.g. '61 2C 62') or inputBytes as UTF-8 text. Empty strings count as "
                     "absent."));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  DataModel::PipelineSpec spec;
  spec.operationMode = static_cast<SerialStudio::OperationMode>(
    params.value(QStringLiteral("operationMode")).toInt(int(SerialStudio::ProjectFile)));
  spec.frameDetection = static_cast<SerialStudio::FrameDetection>(
    params.value(Keys::FrameDetection).toInt(int(SerialStudio::EndDelimiterOnly)));
  spec.decoderMethod = static_cast<SerialStudio::DecoderMethod>(
    params.value(Keys::DecoderMethod).toInt(int(SerialStudio::PlainText)));
  spec.checksumAlgorithm = params.value(Keys::ChecksumAlgorithm).toString();

  const bool hexDelims = params.value(Keys::HexadecimalDelimiters).toBool(false);
  const auto start     = dryRunDelimiter(params, QString(Keys::FrameStart), hexDelims);
  const auto end       = dryRunDelimiter(params, QString(Keys::FrameEnd), hexDelims);
  if (!start.isEmpty())
    spec.startSequences.append(start);

  if (!end.isEmpty())
    spec.finishSequences.append(end);

  if (spec.operationMode == SerialStudio::QuickPlot && spec.finishSequences.isEmpty()) {
    spec.finishSequences = {QByteArray("\n"), QByteArray("\r\n"), QByteArray("\r")};
    spec.frameDetection  = SerialStudio::EndDelimiterOnly;
  }

  const auto run = DataModel::runFrameParserPipelineWithCode(bytes, spec, code, language);
  if (!run.stageError.isEmpty()) {
    QString message = run.stageError;
    if (run.stageWhere == QStringLiteral("compile"))
      message += frameParserCompileHint(code, language);

    return CommandResponse::makeError(id, ErrorCode::ExecutionError, message);
  }

  QJsonArray frameResults;
  int totalRows = 0;
  for (const auto& f : run.frames) {
    frameResults.append(dryRunSerializeFrame(f));
    totalRows += f.rows.size();
  }

  QJsonObject result;
  result[QStringLiteral("ok")]             = true;
  result[QStringLiteral("frames")]         = frameResults;
  result[QStringLiteral("frameCount")]     = frameResults.size();
  result[QStringLiteral("extractedCount")] = run.extractedCount;
  result[QStringLiteral("consumedBytes")]  = static_cast<int>(run.consumedBytes);
  result[QStringLiteral("remainingBytes")] = static_cast<int>(run.remainingBytes);
  result[QStringLiteral("droppedFrames")]  = static_cast<qint64>(run.droppedFrames);
  result[QStringLiteral("totalRows")]      = totalRows;
  result[QStringLiteral("hint")]           = QStringLiteral(
    "Bytes flow through extraction (delimiters / detection) -> decoder method -> parser, the "
              "same path the live FrameBuilder uses. Pick the Binary decoder for non-text streams "
              "(COBS, Modbus, custom binary) -- PlainText / Hex / Base64 route through "
              "QString::fromUtf8 and mojibake non-ASCII bytes.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Compile-only check for a frame parser; surfaces syntax errors without running.
 */
API::CommandResponse API::Handlers::ProjectDryRunCommands::frameParserDryCompile(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  if (language != SerialStudio::JavaScript && language != SerialStudio::Lua
      && language != SerialStudio::Native)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript), 1 (Lua) or 2 (Built-In)"));

  auto engine   = makeScriptEngine(language);
  const bool ok = engine->loadScript(code, 0, false);

  QJsonObject result;
  result[QStringLiteral("ok")] = ok;
  if (language == SerialStudio::Native)
    result[QStringLiteral("language")] = QStringLiteral("native");
  else
    result[QStringLiteral("language")] =
      (language == SerialStudio::Lua ? QStringLiteral("lua") : QStringLiteral("javascript"));

  if (!ok) {
    if (language == SerialStudio::Native)
      result[QStringLiteral("error")] = QStringLiteral(
        "Invalid native parser descriptor: expected {\"template\": id, \"params\": {...}} "
        "with a known template id and valid params.");
    else
      result[QStringLiteral("error")] =
        QStringLiteral("Compile failed or parse(frame) is not defined.");

    if (language != SerialStudio::Native) {
      const auto warning = detectLanguageMismatch(code, language);
      if (!warning.isEmpty())
        result[QStringLiteral("warning")] = warning;
    }
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Transform dry-run: compile + apply transform() to a list of values.
 */
API::CommandResponse API::Handlers::ProjectDryRunCommands::transformDryRun(
  const QString& id, const QJsonObject& params)
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
  if (!engine->loadScript(wrapped, 0, false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Transform failed to compile or define transform(value)"));

  QJsonArray outputs;
  for (const auto& v : values) {
    QString sample;
    if (v.isDouble())
      sample = QString::number(SerialStudio::toDouble(v), 'g', 17);
    else
      sample = v.toString();

    const auto rows = engine->parseString(sample);
    if (rows.isEmpty() || rows.first().isEmpty()) {
      outputs.append(QJsonValue::Null);
      continue;
    }

    const auto cell = rows.first().first();
    bool isNum      = false;
    const auto num  = SerialStudio::toDouble(cell, &isNum);
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
API::CommandResponse API::Handlers::ProjectDryRunCommands::painterDryRun(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto code = params.value(QStringLiteral("code")).toString();

  DataModel::ScriptDryRun session(DataModel::ScriptDryRun::Language::JavaScript,
                                  DataModel::kScriptDryRunBudgetMs,
                                  "painter.dryRun");
  if (!session.valid())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Failed to create the dry-run engine"));

  auto& engine = *session.jsEngine();
  engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  auto stub = session.evaluate(
    QStringLiteral("var datasets = []; datasets.length = 0;"
                   "var group = { id: 0, title: '', columns: 0, sourceId: 0 };"
                   "var frame = { number: 0, timestampMs: 0 };"
                   "var theme = new Proxy({}, { get: function() { return '#000000'; } });"
                   "function tableGet() { return 0; }"
                   "function tableSet() {}"
                   "function datasetGetRaw() { return 0; }"
                   "function datasetGetFinal() { return 0; }"),
    QStringLiteral("painter_stub.js"));
  if (stub.isError())
    return CommandResponse::makeError(id,
                                      ErrorCode::ExecutionError,
                                      QStringLiteral("Painter dry-run bootstrap failed: %1")
                                        .arg(stub.property(QStringLiteral("message")).toString()));

  const auto compiled = session.evaluate(code, QStringLiteral("painter_dryrun.js"));
  if (session.timedOut())
    return CommandResponse::makeError(
      id,
      ErrorCode::ScriptTimeout,
      QStringLiteral("Painter code did not finish evaluating within %1 ms (infinite loop at the "
                     "top level?)")
        .arg(session.budgetMs()));

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
      QStringLiteral("Script compiled but did not define paint(ctx, w, h). Canvas scripts "
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

namespace API::Handlers {

/**
 * @brief Runs a compiled transmit() against one sample value under the session deadline, reporting
 *        bytes, a runtime error, or a timeout.
 */
static QJsonObject runOutputWidgetSample(DataModel::ScriptDryRun& session,
                                         QJSValue& transmitFn,
                                         const QJsonValue& inputValue,
                                         bool hex)
{
  QJSEngine& engine = *session.jsEngine();

  QJSValue jsValue;
  if (hex)
    jsValue =
      engine.toScriptValue(QString::fromLatin1(SerialStudio::hexToBytes(inputValue.toString())));
  else if (inputValue.isDouble())
    jsValue = engine.toScriptValue(SerialStudio::toDouble(inputValue));
  else {
    const auto text = inputValue.toString();
    bool numeric    = false;
    const auto num  = SerialStudio::toDouble(text, &numeric);
    jsValue         = numeric ? engine.toScriptValue(num) : engine.toScriptValue(text);
  }

  const auto called = session.call(transmitFn, QJSValueList{jsValue});
  QJsonObject out;
  if (session.timedOut()) {
    out[QStringLiteral("ok")]       = false;
    out[QStringLiteral("timedOut")] = true;
    out[QStringLiteral("runtimeError")] =
      QStringLiteral("transmit() did not return within %1 ms").arg(session.budgetMs());
    return out;
  }

  if (called.isError()) {
    out[QStringLiteral("ok")]           = false;
    out[QStringLiteral("runtimeError")] = called.toString();
    out[QStringLiteral("line")]         = called.property(QStringLiteral("lineNumber")).toInt();
    return out;
  }

  const QByteArray payload =
    called.isString() ? called.toString().toLatin1() : called.toVariant().toByteArray();
  out[QStringLiteral("ok")]        = true;
  out[QStringLiteral("byteCount")] = payload.size();
  out[QStringLiteral("outputHex")] = QString::fromLatin1(payload.toHex(' ')).toUpper();
  return out;
}

}  // namespace API::Handlers

/**
 * @brief Compiles an output-widget transmit() in the live helper environment without applying.
 */
API::CommandResponse API::Handlers::ProjectDryRunCommands::outputWidgetDryRun(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto code    = params.value(QStringLiteral("code")).toString();
  auto& frameBuilder = DataModel::pipelineModules().frameBuilder;
  frameBuilder.refreshTableStoreFromProjectModel();

  DataModel::ScriptDryRun session(DataModel::ScriptDryRun::Language::JavaScript,
                                  DataModel::kScriptDryRunBudgetMs,
                                  "outputWidget.dryRun");
  if (!session.valid())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Failed to create the dry-run engine"));

  QJSEngine& engine = *session.jsEngine();
  engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
#ifdef BUILD_COMMERCIAL
  DataModel::ScriptApiCall::installAll(&engine, 0);
#endif
  frameBuilder.injectTableApiJS(&engine);

  const auto wrapped =
    QStringLiteral("(function() { %1\n"
                   "return typeof transmit === 'function' ? transmit : undefined; })()")
      .arg(code);
  auto transmitFn = session.evaluate(wrapped, QStringLiteral("output_widget_dryrun.js"));
  if (session.timedOut())
    return CommandResponse::makeError(
      id,
      ErrorCode::ScriptTimeout,
      QStringLiteral("Transmit code did not finish evaluating within %1 ms (infinite loop at the "
                     "top level?)")
        .arg(session.budgetMs()));

  if (transmitFn.isError()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      transmitFn.property(QStringLiteral("message")).toString();
    result[QStringLiteral("line")] = transmitFn.property(QStringLiteral("lineNumber")).toInt();
    return CommandResponse::makeSuccess(id, result);
  }

  if (!transmitFn.isCallable()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      QStringLiteral("Script compiled but did not define transmit(value). Output-widget "
                     "transmit scripts MUST define `function transmit(value)` returning the "
                     "bytes to send (a Uint8Array, a byte array, or a string). The function "
                     "is named `transmit`, not `output` or `send`.");
    return CommandResponse::makeSuccess(id, result);
  }

  QJsonObject result;
  result[QStringLiteral("ok")]          = true;
  result[QStringLiteral("hasTransmit")] = true;
  if (params.contains(QStringLiteral("inputValue")))
    result[QStringLiteral("sampleRun")] =
      runOutputWidgetSample(session,
                            transmitFn,
                            params.value(QStringLiteral("inputValue")),
                            params.value(QStringLiteral("hex")).toBool());
  else
    result[QStringLiteral("hint")] =
      QStringLiteral("Compiled and transmit(value) is defined. Pass inputValue (and hex:true "
                     "for hex byte input) to also execute it and see the produced bytes.");

  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

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
    const bool ok  = engine->loadScript(src, dataset.sourceId, false);

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
  const auto num  = SerialStudio::toDouble(cell, &isNum);
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
  const auto num                            = SerialStudio::toDouble(rawCell, &isNum);
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

}  // namespace API::Handlers

/**
 * @brief End-to-end dry-run: parser + all dataset transforms applied to a sample frame.
 */
API::CommandResponse API::Handlers::ProjectDryRunCommands::endToEndDryRun(const QString& id,
                                                                          const QJsonObject& params)
{
  auto& pm           = DataModel::pipelineModules().projectModel;
  const auto sources = pm.sources();
  const int sourceId = params.value(Keys::SourceId).toInt(0);
  if (sources.empty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Project has no sources to dry-run against"));

  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Source id out of range: %1").arg(sourceId));

  const auto& source = sources[sourceId];

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

  auto parser = makeScriptEngine(language);
  if (!parser->loadScript(code, sourceId, false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Frame parser failed to compile or define parse(frame).")
        + frameParserCompileHint(code, language));

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
                   "Computed datasets show their transform applied to the raw cell at "
                   "their index (which is normally 0/unset for computed entries).");
  return CommandResponse::makeSuccess(id, result);
}
