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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/FrameParser.h"

#include <QAtomicInt>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QThread>

#include "DataModel/ProjectModel.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Multi-frame parsing helper functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enum to classify JavaScript return value types from the parse
 * function.
 */
enum class ArrayType {
  Scalar,
  Array1D,
  Array2D,
  ArrayMixed
};

/**
 * @brief Converts a JavaScript array to a QStringList.
 *
 * @param jsValue JavaScript array to convert.
 * @return QStringList containing string representation of each element.
 */
static QStringList jsArrayToStringList(const QJSValue& jsValue)
{
  QStringList result;
  const int length = jsValue.property("length").toInt();
  result.reserve(length);

  for (int i = 0; i < length; ++i)
    result.append(jsValue.property(i).toString());

  return result;
}

/**
 * @brief Detects the type of JavaScript array returned by the parse function.
 *
 * Classifies the JavaScript value as:
 * - Scalar: Not an array or empty
 * - Array1D: Flat scalar array [1, 2, 3]
 * - Array2D: Pure 2D array [[1,2], [3,4]]
 * - ArrayMixed: Mixed scalar/vector [1, [2,3], [4,5]]
 *
 * @param jsValue JavaScript value to classify.
 * @return ArrayType classification.
 */
static ArrayType detectArrayType(const QJSValue& jsValue)
{
  if (!jsValue.isArray())
    return ArrayType::Scalar;

  const int length = jsValue.property("length").toInt();
  if (length == 0)
    return ArrayType::Scalar;

  bool hasArray  = false;
  bool hasScalar = false;

  for (int i = 0; i < length; ++i) {
    const auto element = jsValue.property(i);
    if (element.isArray())
      hasArray = true;
    else
      hasScalar = true;
  }

  if (hasArray && hasScalar)
    return ArrayType::ArrayMixed;
  else if (hasArray && !hasScalar)
    return ArrayType::Array2D;
  else
    return ArrayType::Array1D;
}

/**
 * @brief Converts a pure 2D JavaScript array to multiple frames.
 *
 * Processes arrays like [[1,2,3], [4,5,6]] and generates one frame per row.
 * Invalid rows (non-arrays) are skipped with a warning.
 *
 * @param jsValue JavaScript 2D array.
 * @return List of QStringList, one per row/frame.
 */
static QList<QStringList> convert2DArray(const QJSValue& jsValue)
{
  QList<QStringList> results;
  const int rowCount = jsValue.property("length").toInt();
  results.reserve(rowCount);

  for (int row = 0; row < rowCount; ++row) {
    const auto rowArray = jsValue.property(row);

    if (!rowArray.isArray()) [[unlikely]] {
      qWarning() << "[FrameParser] Row" << row << "is not an array, skipping";
      continue;
    }

    results.append(jsArrayToStringList(rowArray));
  }

  return results;
}

/**
 * @brief Converts a mixed scalar/vector JavaScript array to multiple frames.
 *
 * Processes arrays like [scalar1, scalar2, [vec1, vec2, vec3]] and "unzips"
 * vectors while repeating scalars across frames.
 *
 * Example:
 *   Input:  [25.5, 60.0, [1.1, 2.2, 3.3]]
 *   Output: [[25.5, 60.0, 1.1], [25.5, 60.0, 2.2], [25.5, 60.0, 3.3]]
 *
 * If multiple vectors have different lengths, shorter vectors are extended
 * by repeating their last value.
 *
 * @param jsValue JavaScript mixed array.
 * @return List of QStringList, one per frame.
 */
static QList<QStringList> convertMixedArray(const QJSValue& jsValue)
{
  const int elementCount = jsValue.property("length").toInt();

  QStringList scalars;
  QList<QStringList> vectors;
  qsizetype maxVectorLength = 0;

  for (int i = 0; i < elementCount; ++i) {
    const auto element = jsValue.property(i);

    if (element.isArray()) {
      const auto vec = jsArrayToStringList(element);
      if (!vec.isEmpty()) {
        vectors.append(vec);
        maxVectorLength = std::max(maxVectorLength, vec.size());
      }
    } else {
      scalars.append(element.toString());
    }
  }

  if (vectors.isEmpty()) [[unlikely]] {
    QList<QStringList> results;
    results.append(scalars);
    return results;
  }

  for (auto& vec : vectors) {
    if (!vec.isEmpty() && vec.size() < maxVectorLength) {
      const QString lastValue = vec.last();
      while (vec.size() < maxVectorLength)
        vec.append(lastValue);
    }
  }

  QList<QStringList> results;
  results.reserve(maxVectorLength);

  for (int i = 0; i < maxVectorLength; ++i) {
    QStringList frame;
    frame.reserve(scalars.size() + vectors.size());

    frame.append(scalars);

    for (const auto& vec : std::as_const(vectors))
      if (i < vec.size())
        frame.append(vec[i]);

    results.append(frame);
  }

  return results;
}

//--------------------------------------------------------------------------------------------------
// FrameParser singleton implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initialises the engine map (source 0 = global) and loads template names.
 *
 * Cross-singleton connections to ProjectModel are deferred to
 * @c setupExternalConnections() to avoid static-init-order deadlocks.
 */
DataModel::FrameParser::FrameParser() : m_suppressMessageBoxes(false)
{
  (void)engineForSource(0);

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameParser::collectGarbage);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DataModel::FrameParser::loadTemplateNames);

  loadTemplateNames();
}

/**
 * @brief Returns the singleton instance.
 */
DataModel::FrameParser& DataModel::FrameParser::instance()
{
  static FrameParser singleton;
  return singleton;
}

/**
 * @brief Wires cross-singleton connections and performs the initial script load.
 *
 * Called by ModuleManager after all singletons are constructed so that
 * accessing ProjectModel::instance() here cannot deadlock.
 */
void DataModel::FrameParser::setupExternalConnections()
{
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::FrameParser::readCode);

  readCode();
}

/**
 * @brief Returns the source code of the default frame parser template.
 *
 * @return The default template source code, or an empty string if the
 *         resource could not be read.
 */
QString DataModel::FrameParser::defaultTemplateCode()
{
  QString code;
  QFile file(QStringLiteral(":/rcc/scripts/comma_separated.js"));
  if (file.open(QFile::ReadOnly)) {
    code = QString::fromUtf8(file.readAll());
    file.close();
  }

  return code;
}

/**
 * @brief Returns the JavaScript source of the currently selected template for
 * @p sourceId.
 *
 * @param sourceId Source whose stored template index is used (default: 0).
 * @return Template source code, or an empty string if no template is selected.
 */
QString DataModel::FrameParser::templateCode(int sourceId) const
{
  auto it       = m_engines.find(sourceId);
  const int idx = (it != m_engines.end()) ? it.value()->templateIdx : -1;

  if (idx < 0 || idx >= m_templateFiles.count())
    return {};

  QString code;
  const auto path = QStringLiteral(":/rcc/scripts/%1").arg(m_templateFiles.at(idx));

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    code = QString::fromUtf8(file.readAll());
    file.close();
  }

  return code;
}

/**
 * @brief Returns the list of localised template display names.
 */
const QStringList& DataModel::FrameParser::templateNames() const
{
  return m_templateNames;
}

/**
 * @brief Returns the list of template file basenames.
 */
const QStringList& DataModel::FrameParser::templateFiles() const
{
  return m_templateFiles;
}

/**
 * @brief Returns (or lazily creates) the JS engine for @p sourceId.
 *
 * Source 0 is the global engine shared by all single-source projects.
 * Higher IDs get their own isolated engine.
 *
 * @param sourceId Source identifier (0 = global).
 * @return Reference to the SourceEngine for this source.
 */
DataModel::FrameParser::SourceEngine& DataModel::FrameParser::engineForSource(int sourceId)
{
  auto it = m_engines.find(sourceId);
  if (it != m_engines.end())
    return *it.value();

  auto* se = new SourceEngine();
  se->engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  m_engines.insert(sourceId, se);
  return *se;
}

/**
 * @brief Sets per-source JavaScript code, loading it into the source engine.
 * @param sourceId Source identifier (0 = global).
 * @param code     JavaScript source code.
 */
void DataModel::FrameParser::setSourceCode(int sourceId, const QString& code)
{
  if (code.isEmpty()) {
    clearSourceEngine(sourceId);
    return;
  }

  (void)loadScript(sourceId, code, false);
}

/**
 * @brief Removes and destroys the JS engine for @p sourceId.
 *
 * For source 0 this resets the global engine to a clean, unloaded state
 * rather than removing it from the map (source 0 is always present).
 *
 * @param sourceId Source identifier (0 = global).
 */
void DataModel::FrameParser::clearSourceEngine(int sourceId)
{
  auto it = m_engines.find(sourceId);
  if (it == m_engines.end())
    return;

  if (sourceId == 0) {
    it.value()->parseFunction = QJSValue();
    it.value()->hexToArray    = QJSValue();
    return;
  }

  delete it.value();
  m_engines.erase(it);
}

/**
 * @brief Executes the JS engine for @p sourceId over text data, returning one
 * or more frames.
 *
 * Source 0 uses the global engine. For other IDs, falls back to source 0 when
 * the source has no dedicated engine loaded.
 *
 * @param frame    Decoded UTF-8 string frame.
 * @param sourceId Source identifier whose engine should be used.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QString& frame, int sourceId)
{
  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it.value()->parseFunction.isCallable()) {
    if (sourceId == 0)
      return {};

    return parseMultiFrame(frame, 0);
  }

  auto& se = *it.value();

  QJSValueList args;
  args << frame;
  const auto jsResult = se.parseFunction.call(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[FrameParser] Source" << sourceId
               << "JS error:" << jsResult.property("message").toString();
    return {};
  }

  QList<QStringList> results;
  switch (detectArrayType(jsResult)) {
    case ArrayType::Array2D:
      return convert2DArray(jsResult);
    case ArrayType::ArrayMixed:
      return convertMixedArray(jsResult);
    case ArrayType::Array1D:
    case ArrayType::Scalar:
    default:
      results.append(jsResult.toVariant().toStringList());
      return results;
  }
}

/**
 * @brief Executes the JS engine for @p sourceId over binary data, returning
 * one or more frames.
 *
 * Source 0 uses the global engine. For other IDs, falls back to source 0 when
 * the source has no dedicated engine loaded.
 *
 * @param frame    Binary frame data.
 * @param sourceId Source identifier whose engine should be used.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QByteArray& frame, int sourceId)
{
  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it.value()->parseFunction.isCallable()) {
    if (sourceId == 0)
      return {};

    return parseMultiFrame(frame, 0);
  }

  auto& se = *it.value();

  QJSValue jsArray;
  if (se.hexToArray.isCallable()) [[likely]] {
    QJSValueList hexArgs;
    hexArgs << QString::fromLatin1(frame.toHex());
    jsArray = se.hexToArray.call(hexArgs);
  } else {
    jsArray          = se.engine.newArray(frame.size());
    const auto* data = reinterpret_cast<const quint8*>(frame.constData());
    for (int i = 0; i < frame.size(); ++i)
      jsArray.setProperty(i, data[i]);
  }

  QJSValueList args;
  args << jsArray;
  const auto jsResult = se.parseFunction.call(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[FrameParser] Source" << sourceId
               << "JS error:" << jsResult.property("message").toString();
    return {};
  }

  QList<QStringList> results;
  switch (detectArrayType(jsResult)) {
    case ArrayType::Array2D:
      return convert2DArray(jsResult);
    case ArrayType::ArrayMixed:
      return convertMixedArray(jsResult);
    case ArrayType::Array1D:
    case ArrayType::Scalar:
    default:
      results.append(jsResult.toVariant().toStringList());
      return results;
  }
}

/**
 * @brief Validates and loads a JavaScript frame parser script into the engine
 * for @p sourceId.
 *
 * For source 0, performs full validation: syntax check, function-signature
 * check, and a probe with a watchdog timer. For all other sources, performs
 * lightweight validation (syntax + callable parse function) without the probe,
 * so per-source scripts load without GUI message boxes.
 *
 * @param sourceId         Target source engine (0 = global).
 * @param script           The JavaScript source to validate and load.
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true on success, @c false if validation failed.
 */
bool DataModel::FrameParser::loadScript(int sourceId, const QString& script, bool showMessageBoxes)
{
  auto& se         = engineForSource(sourceId);
  se.parseFunction = QJSValue();
  se.engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  // Syntax check
  QStringList exceptionStackTrace;
  auto result = se.engine.evaluate(
    script, QStringLiteral("parser_%1.js").arg(sourceId), 1, &exceptionStackTrace);

  if (result.isError()) {
    const QString errorMsg = result.property("message").toString();
    const int lineNumber   = result.property("lineNumber").toInt();
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("JavaScript Syntax Error"),
        tr("The parser code contains a syntax error at line %1:\n\n%2")
          .arg(lineNumber)
          .arg(errorMsg),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Source" << sourceId << "syntax error at line" << lineNumber
                 << ":" << errorMsg;
    }
    return false;
  }

  if (!exceptionStackTrace.isEmpty()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("JavaScript Exception Occurred"),
        tr("The parser code triggered the following exceptions:\n\n%1")
          .arg(exceptionStackTrace.join(QStringLiteral("\n"))),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Source" << sourceId
                 << "exceptions:" << exceptionStackTrace.join(QStringLiteral(", "));
    }
    return false;
  }

  // Verify the 'parse' function exists and is callable
  auto parseFunction = se.engine.globalObject().property(QStringLiteral("parse"));
  if (parseFunction.isNull() || !parseFunction.isCallable()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("Missing Parse Function"),
        tr("The 'parse' function is not defined in the script.\n\n"
           "Please ensure your code includes:\nfunction parse(frame) { ... }"),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Source" << sourceId << "missing or non-callable parse()";
    }
    return false;
  }

  // Full signature + probe validation only for source 0 (the global / primary engine)
  if (sourceId == 0) {
    static QRegularExpression functionRegex(
      R"(\bfunction\s+parse\s*\(\s*([a-zA-Z_$][a-zA-Z0-9_$]*)(\s*,\s*([a-zA-Z_$][a-zA-Z0-9_$]*))?\s*\))");
    const auto match = functionRegex.match(script);
    if (!match.hasMatch()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(tr("Invalid Function Declaration"),
                                        tr("No valid 'parse' function declaration found.\n\n"
                                           "Expected format:\nfunction parse(frame) { ... }"),
                                        QMessageBox::Critical);
      } else {
        qWarning() << "[FrameParser] No valid 'parse' function declaration found";
      }
      return false;
    }

    const QString firstArg  = match.captured(1);
    const QString secondArg = match.captured(3);

    if (firstArg.isEmpty()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          tr("Invalid Function Parameter"),
          tr("The 'parse' function must have at least one parameter.\n\n"
             "Expected format:\nfunction parse(frame) { ... }"),
          QMessageBox::Critical);
      } else {
        qWarning() << "[FrameParser] Parse function must have at least one parameter";
      }
      return false;
    }

    if (!secondArg.isEmpty()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          tr("Deprecated Function Signature"),
          tr("The 'parse' function uses the old two-parameter format: parse(%1, %2)\n\n"
             "This format is no longer supported. Please update to the new single-parameter "
             "format:\nfunction parse(%1) { ... }\n\nThe separator parameter is no longer "
             "needed.")
            .arg(firstArg, secondArg),
          QMessageBox::Warning);
      } else {
        qWarning() << "[FrameParser] Deprecated two-parameter parse function";
      }
      return false;
    }

    // Probe with three representative inputs; watchdog kills infinite loops after 500 ms
    bool probeOk = false;
    QJSValue lastError;
    auto byteProbe = se.engine.newArray(1);
    byteProbe.setProperty(0, 0);
    const QJSValue probeInputs[] = {QJSValue("0"), byteProbe, QJSValue("")};
    {
      QAtomicInt probeDone(0);
      auto* watchdog = QThread::create([&se, &probeDone]() {
        constexpr int kTimeoutMs = 500;
        constexpr int kSliceMs   = 20;
        for (int t = 0; t < kTimeoutMs && !probeDone.loadAcquire(); t += kSliceMs)
          QThread::msleep(kSliceMs);

        if (!probeDone.loadAcquire())
          se.engine.setInterrupted(true);
      });
      watchdog->start();

      for (const auto& input : probeInputs) {
        QJSValueList probeArgs;
        probeArgs << input;
        const auto probeResult = parseFunction.call(probeArgs);
        if (!probeResult.isError()) {
          probeOk = true;
          break;
        }

        lastError = probeResult;
      }

      probeDone.storeRelease(1);
      se.engine.setInterrupted(false);
      watchdog->wait();
      watchdog->deleteLater();
    }

    if (!probeOk) {
      const QString errorMsg = lastError.property("message").toString();
      const int lineNumber   = lastError.property("lineNumber").toInt();
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(tr("Parse Function Runtime Error"),
                                        tr("The parse function contains an error at line %1:\n\n"
                                           "%2\n\n"
                                           "Please fix the error in the function body.")
                                          .arg(lineNumber)
                                          .arg(errorMsg),
                                        QMessageBox::Critical);
      } else {
        qWarning() << "[FrameParser] Runtime error at line" << lineNumber << ":" << errorMsg;
      }
      return false;
    }
  }

  se.parseFunction = parseFunction;

  // Hex-to-byte-array helper for binary frame parsing
  se.engine.evaluate(QStringLiteral("function __ss_internal_hex_to_array__(h){"
                                    "var n=h.length>>1,a=new Array(n);"
                                    "for(var i=0,j=0;i<h.length;i+=2,j++)"
                                    "a[j]=parseInt(h.substr(i,2),16);return a;}"));
  se.hexToArray = se.engine.globalObject().property(QStringLiteral("__ss_internal_hex_to_array__"));

  return true;
}

/**
 * @brief Enables or disables UI message boxes (suppressed during API calls).
 */
void DataModel::FrameParser::setSuppressMessageBoxes(const bool suppress)
{
  m_suppressMessageBoxes = suppress;
}

/**
 * @brief Loads the code stored in the project model into all engines.
 *
 * Destroys all engines except source 0 (which is reset in-place), then
 * reloads source 0 from the project and lazily loads each per-source engine
 * from its stored parser code.
 */
void DataModel::FrameParser::readCode()
{
  QList<int> keysToRemove;
  for (auto it = m_engines.begin(); it != m_engines.end(); ++it) {
    if (it.key() != 0) {
      delete it.value();
      keysToRemove.append(it.key());
    }
  }

  for (int k : keysToRemove)
    m_engines.remove(k);

  const auto& model   = ProjectModel::instance();
  const bool suppress = m_suppressMessageBoxes || model.suppressMessageBoxes();

  const auto& sources = model.sources();
  const QString code  = sources.empty() ? QString() : sources[0].frameParserCode;

  (void)loadScript(0, code, !suppress);

  for (const auto& src : sources)
    if (src.sourceId > 0 && !src.frameParserCode.isEmpty())
      (void)loadScript(src.sourceId, src.frameParserCode, false);

  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the JS execution context by re-loading the current code.
 *
 * Destroys all per-source engines (source 0 is reset in-place), then reloads
 * all scripts from the project model.
 */
void DataModel::FrameParser::clearContext()
{
  QList<int> keysToRemove;
  for (auto it = m_engines.begin(); it != m_engines.end(); ++it) {
    if (it.key() != 0) {
      delete it.value();
      keysToRemove.append(it.key());
    }
  }

  for (int k : keysToRemove)
    m_engines.remove(k);

  const auto& sources = ProjectModel::instance().sources();
  const QString code  = sources.empty() ? QString() : sources[0].frameParserCode;

  (void)loadScript(0, code, !m_suppressMessageBoxes);

  for (const auto& src : sources)
    if (src.sourceId > 0 && !src.frameParserCode.isEmpty())
      (void)loadScript(src.sourceId, src.frameParserCode, false);
}

/**
 * @brief Runs one cycle of JS garbage collection on all engines.
 */
void DataModel::FrameParser::collectGarbage()
{
  for (auto it = m_engines.begin(); it != m_engines.end(); ++it)
    it.value()->engine.collectGarbage();
}

/**
 * @brief Rebuilds the list of available JS template files and display names.
 */
void DataModel::FrameParser::loadTemplateNames()
{
  m_templateFiles = {QStringLiteral("at_commands.js"),
                     QStringLiteral("base64_encoded.js"),
                     QStringLiteral("batched_sensor_data.js"),
                     QStringLiteral("binary_tlv.js"),
                     QStringLiteral("cobs_encoded.js"),
                     QStringLiteral("comma_separated.js"),
                     QStringLiteral("fixed_width_fields.js"),
                     QStringLiteral("hexadecimal_bytes.js"),
                     QStringLiteral("ini_config.js"),
                     QStringLiteral("json_data.js"),
                     QStringLiteral("key_value_pairs.js"),
                     QStringLiteral("mavlink.js"),
                     QStringLiteral("messagepack.js"),
                     QStringLiteral("modbus.js"),
                     QStringLiteral("nmea_0183.js"),
                     QStringLiteral("nmea_2000.js"),
                     QStringLiteral("pipe_delimited.js"),
                     QStringLiteral("raw_bytes.js"),
                     QStringLiteral("rtcm_corrections.js"),
                     QStringLiteral("semicolon_separated.js"),
                     QStringLiteral("sirf_binary.js"),
                     QStringLiteral("slip_encoded.js"),
                     QStringLiteral("tab_separated.js"),
                     QStringLiteral("time_series_2d.js"),
                     QStringLiteral("ubx_ublox.js"),
                     QStringLiteral("url_encoded.js"),
                     QStringLiteral("xml_data.js"),
                     QStringLiteral("yaml_data.js")};

  m_templateNames = {tr("AT command responses"),
                     tr("Base64-encoded data"),
                     tr("Batched sensor data (multi-frame)"),
                     tr("Binary TLV (Tag-Length-Value)"),
                     tr("COBS-encoded frames"),
                     tr("Comma-separated data"),
                     tr("Fixed-width fields"),
                     tr("Hexadecimal bytes"),
                     tr("INI/config format"),
                     tr("JSON data"),
                     tr("Key-value pairs"),
                     tr("MAVLink messages"),
                     tr("MessagePack data"),
                     tr("Modbus frames"),
                     tr("NMEA 0183 sentences"),
                     tr("NMEA 2000 messages"),
                     tr("Pipe-delimited data"),
                     tr("Raw bytes"),
                     tr("RTCM corrections"),
                     tr("Semicolon-separated data"),
                     tr("SiRF binary protocol"),
                     tr("SLIP-encoded frames"),
                     tr("Tab-separated data"),
                     tr("Time-series 2D arrays (multi-frame)"),
                     tr("UBX protocol (u-blox)"),
                     tr("URL-encoded data"),
                     tr("XML data"),
                     tr("YAML data")};

  Q_EMIT templateNamesChanged();
}

/**
 * @brief Sets the active template index and saves the resulting code to the
 * project model.
 *
 * Loads the template source, evaluates it in the engine for @p sourceId, and
 * persists it. For source 0, also calls @c ProjectModel::setFrameParserCode().
 * For other sources, calls @c ProjectModel::updateSourceFrameParser().
 *
 * @param sourceId Target source engine (0 = global).
 * @param idx      Template index into @c m_templateFiles.
 */
void DataModel::FrameParser::setTemplateIdx(int sourceId, int idx)
{
  if (idx < 0 || idx >= m_templateFiles.size())
    return;

  engineForSource(sourceId).templateIdx = idx;
  const QString code                    = templateCode(sourceId);

  if (loadScript(sourceId, code, !m_suppressMessageBoxes)) {
    auto& model = DataModel::ProjectModel::instance();
    if (sourceId == 0)
      model.setFrameParserCode(code);
    else
      model.updateSourceFrameParser(sourceId, code);

    model.setModified(true);
  }
}

/**
 * @brief Loads the default comma-separated-values template for @p sourceId.
 *
 * @param sourceId   Target source engine (0 = global).
 * @param guiTrigger When @c false, clears the project's modified flag after
 *                   loading (silent default reset).
 */
void DataModel::FrameParser::loadDefaultTemplate(int sourceId, bool guiTrigger)
{
  const auto idx = m_templateFiles.indexOf(QStringLiteral("comma_separated.js"));
  setTemplateIdx(sourceId, idx);

  if (!guiTrigger)
    DataModel::ProjectModel::instance().setModified(false);
}
