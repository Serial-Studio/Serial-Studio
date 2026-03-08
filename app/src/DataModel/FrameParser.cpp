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
enum class ArrayType
{
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
static QStringList jsArrayToStringList(const QJSValue &jsValue)
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
static ArrayType detectArrayType(const QJSValue &jsValue)
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
static QList<QStringList> convert2DArray(const QJSValue &jsValue)
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
static QList<QStringList> convertMixedArray(const QJSValue &jsValue)
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

  for (auto &vec : vectors) {
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

    for (const auto &vec : std::as_const(vectors)) {
      if (i < vec.size())
        frame.append(vec[i]);
    }

    results.append(frame);
  }

  return results;
}

//--------------------------------------------------------------------------------------------------
// FrameParser singleton implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initialises the JS engine and loads template names.
 *
 * Cross-singleton connections to ProjectModel are deferred to
 * @c setupExternalConnections() to avoid static-init-order deadlocks.
 */
DataModel::FrameParser::FrameParser()
  : m_templateIdx(-1)
  , m_suppressMessageBoxes(false)
{
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Collect JS garbage at 1 Hz
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameParser::collectGarbage);

  // ProjectModel connections are deferred to setupExternalConnections() to
  // avoid a static-init-order deadlock (ProjectModel calls FrameBuilder which
  // calls FrameParser before ProjectModel finishes constructing).

  // Reload template names when the UI language changes
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DataModel::FrameParser::loadTemplateNames);

  loadTemplateNames();
}

/**
 * @brief Returns the singleton instance.
 */
DataModel::FrameParser &DataModel::FrameParser::instance()
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
 * @brief Returns the JavaScript source of the currently selected template.
 *
 * @return Template source code, or an empty string if the index is invalid.
 */
QString DataModel::FrameParser::templateCode() const
{
  if (m_templateIdx < 0 || m_templateIdx >= m_templateFiles.count())
    return {};

  QString code;
  const auto path =
    QStringLiteral(":/rcc/scripts/%1").arg(m_templateFiles.at(m_templateIdx));

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
const QStringList &DataModel::FrameParser::templateNames() const
{
  return m_templateNames;
}

/**
 * @brief Returns the list of template file basenames.
 */
const QStringList &DataModel::FrameParser::templateFiles() const
{
  return m_templateFiles;
}

/**
 * @brief Executes the current frame parser function over UTF-8 text data.
 *
 * @param frame Decoded UTF-8 string frame.
 * @return An array of strings returned by the JS parser.
 */
QStringList DataModel::FrameParser::parse(const QString &frame)
{
  QJSValueList args;
  args << frame;

  return m_parseFunction.call(args).toVariant().toStringList();
}

/**
 * @brief Executes the current frame parser function over binary data.
 *
 * @param frame Binary frame data.
 * @return Parsed string fields returned by the JS frame parser.
 */
QStringList DataModel::FrameParser::parse(const QByteArray &frame)
{
  QJSValue jsArray;
  if (m_hexToArray.isCallable()) [[likely]] {
    QJSValueList hexArgs;
    hexArgs << QString::fromLatin1(frame.toHex());
    jsArray = m_hexToArray.call(hexArgs);
  } else {
    jsArray = m_engine.newArray(frame.size());
    const auto *data = reinterpret_cast<const quint8 *>(frame.constData());
    for (int i = 0; i < frame.size(); ++i)
      jsArray.setProperty(i, data[i]);
  }

  QJSValueList args;
  args << jsArray;

  return m_parseFunction.call(args).toVariant().toStringList();
}

/**
 * @brief Executes the frame parser function over text data, returning one or
 * more frames.
 *
 * Supports flat 1D arrays (single frame), pure 2D arrays (one frame per row),
 * and mixed scalar/vector arrays (scalars repeated across frames).
 *
 * @param frame Decoded UTF-8 string frame.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QString &frame)
{
  QList<QStringList> results;

  QJSValueList args;
  args << frame;
  const auto jsResult = m_parseFunction.call(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[FrameParser] JS error:" << jsResult.property("message").toString();
    return results;
  }

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
 * @brief Executes the frame parser function over binary data, returning one or
 * more frames.
 *
 * @param frame Binary frame data.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QByteArray &frame)
{
  QList<QStringList> results;

  QJSValue jsArray;
  if (m_hexToArray.isCallable()) [[likely]] {
    QJSValueList hexArgs;
    hexArgs << QString::fromLatin1(frame.toHex());
    jsArray = m_hexToArray.call(hexArgs);
  } else {
    jsArray = m_engine.newArray(frame.size());
    const auto *data = reinterpret_cast<const quint8 *>(frame.constData());
    for (int i = 0; i < frame.size(); ++i)
      jsArray.setProperty(i, data[i]);
  }

  QJSValueList args;
  args << jsArray;
  const auto jsResult = m_parseFunction.call(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[FrameParser] JS error:" << jsResult.property("message").toString();
    return results;
  }

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
 * @brief Validates and loads a JavaScript frame parser script into the engine.
 *
 * Checks for syntax errors, verifies that a callable single-parameter
 * @c parse() function exists, probes it with representative inputs, and
 * installs an optimised hex-to-byte-array helper on success.
 *
 * @param script           The JavaScript source to validate and load.
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true on success, @c false if validation failed.
 */
bool DataModel::FrameParser::loadScript(const QString &script,
                                        const bool showMessageBoxes)
{
  m_parseFunction = QJSValue();
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Check for JavaScript syntax errors
  QStringList exceptionStackTrace;
  auto result = m_engine.evaluate(script, QStringLiteral("parser.js"), 1,
                                  &exceptionStackTrace);
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
      qWarning() << "[FrameParser] JavaScript syntax error at line" << lineNumber
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
      qWarning() << "[FrameParser] JavaScript exceptions:"
                 << exceptionStackTrace.join(QStringLiteral(", "));
    }
    return false;
  }

  // Verify the 'parse' function exists and is callable
  auto parseFunction = m_engine.globalObject().property(QStringLiteral("parse"));
  if (parseFunction.isNull()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("Missing Parse Function"),
        tr("The 'parse' function is not defined in the script.\n\n"
           "Please ensure your code includes:\nfunction parse(frame) { ... }"),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Missing parse function in script";
    }
    return false;
  }

  if (!parseFunction.isCallable()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("Invalid Parse Function"),
        tr("The 'parse' property exists but is not a callable function.\n\n"
           "Please ensure 'parse' is declared as a function."),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Parse property is not a callable function";
    }
    return false;
  }

  // Validate function signature
  static QRegularExpression functionRegex(
    R"(\bfunction\s+parse\s*\(\s*([a-zA-Z_$][a-zA-Z0-9_$]*)(\s*,\s*([a-zA-Z_$][a-zA-Z0-9_$]*))?\s*\))");
  const auto match = functionRegex.match(script);
  if (!match.hasMatch()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("Invalid Function Declaration"),
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
           "format:\nfunction parse(%1) { ... }\n\nThe separator parameter is no longer needed.")
          .arg(firstArg, secondArg),
        QMessageBox::Warning);
    } else {
      qWarning() << "[FrameParser] Parse function uses deprecated two-parameter format";
    }
    return false;
  }

  // Probe the parse function with three representative input types.
  //
  // A single empty-string probe produces false positives for JSON parsers
  // because JSON.parse("") always throws a SyntaxError. Instead we try:
  //
  //   1. "0"  — non-empty string. JSON.parse("0") returns the number 0, so
  //             simple property accesses like data.values return undefined
  //             rather than throwing, making this probe pass for most
  //             JSON parsers.
  //   2. [0]  — minimal one-byte array. Covers binary frame parsers that
  //             expect a JS byte array rather than a string.
  //   3. ""   — empty string. Succeeds for parsers that explicitly guard
  //             against empty input with if (frame.length > 0).
  //
  // A genuine code defect (e.g. calling an undefined function) throws on
  // every probe, so real bugs are still caught and reported.
  //
  // A watchdog thread interrupts the engine after 500 ms to guard against
  // infinite loops and heap-exhaustion scripts.
  bool probeOk = false;
  QJSValue lastError;
  auto byteProbe = m_engine.newArray(1);
  byteProbe.setProperty(0, 0);
  const QJSValue probeInputs[] = {QJSValue("0"), byteProbe, QJSValue("")};
  {
    QAtomicInt probeDone(0);
    auto *watchdog = QThread::create([this, &probeDone]() {
      constexpr int kTimeoutMs = 500;
      constexpr int kSliceMs   = 20;
      for (int t = 0; t < kTimeoutMs && !probeDone.loadAcquire(); t += kSliceMs)
        QThread::msleep(kSliceMs);

      if (!probeDone.loadAcquire())
        m_engine.setInterrupted(true);
    });
    watchdog->start();

    for (const auto &input : probeInputs) {
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
    m_engine.setInterrupted(false);
    watchdog->wait();
    watchdog->deleteLater();
  }

  if (!probeOk) {
    const QString errorMsg = lastError.property("message").toString();
    const int lineNumber   = lastError.property("lineNumber").toInt();
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        tr("Parse Function Runtime Error"),
        tr("The parse function contains an error at line %1:\n\n"
           "%2\n\n"
           "Please fix the error in the function body.")
          .arg(lineNumber)
          .arg(errorMsg),
        QMessageBox::Critical);
    } else {
      qWarning() << "[FrameParser] Parse function runtime error at line" << lineNumber
                 << ":" << errorMsg;
    }
    return false;
  }

  m_parseFunction = parseFunction;

  // Install optimised hex-to-byte-array helper for binary frame parsing
  m_engine.evaluate(QStringLiteral(
    "function __ss_internal_hex_to_array__(h){"
    "var n=h.length>>1,a=new Array(n);"
    "for(var i=0,j=0;i<h.length;i+=2,j++)"
    "a[j]=parseInt(h.substr(i,2),16);return a;}"));
  m_hexToArray =
    m_engine.globalObject().property(QStringLiteral("__ss_internal_hex_to_array__"));

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
 * @brief Loads the code stored in the project model into the JS engine.
 */
void DataModel::FrameParser::readCode()
{
  const auto &model  = ProjectModel::instance();
  const auto code    = model.frameParserCode();
  const bool suppress =
    m_suppressMessageBoxes || model.suppressMessageBoxes();

  (void)loadScript(code, !suppress);

  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the JS execution context by re-loading the current code.
 */
void DataModel::FrameParser::clearContext()
{
  (void)loadScript(ProjectModel::instance().frameParserCode(),
                   !m_suppressMessageBoxes);
}

/**
 * @brief Runs one cycle of JS garbage collection.
 */
void DataModel::FrameParser::collectGarbage()
{
  m_engine.collectGarbage();
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
 * Loads the template source, evaluates it in the engine, and persists it via
 * @c ProjectModel::setFrameParserCode(). The @c JsCodeEditor picks up the
 * change through the @c frameParserCodeChanged signal.
 *
 * @param idx Template index into @c m_templateFiles.
 */
void DataModel::FrameParser::setTemplateIdx(const int idx)
{
  if (idx < 0 || idx >= m_templateFiles.size())
    return;

  m_templateIdx = idx;
  const QString code = templateCode();

  if (loadScript(code, !m_suppressMessageBoxes)) {
    DataModel::ProjectModel::instance().setFrameParserCode(code);
    DataModel::ProjectModel::instance().setModified(true);
  }
}

/**
 * @brief Loads the default comma-separated-values template.
 *
 * @param guiTrigger When @c false, clears the project's modified flag after
 *                   loading (silent default reset).
 */
void DataModel::FrameParser::loadDefaultTemplate(const bool guiTrigger)
{
  const auto idx = m_templateFiles.indexOf(QStringLiteral("comma_separated.js"));
  setTemplateIdx(idx);

  if (!guiTrigger)
    DataModel::ProjectModel::instance().setModified(false);
}
