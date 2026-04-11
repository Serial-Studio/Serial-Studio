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

#include "DataModel/JsScriptEngine.h"

#include <QAtomicInt>
#include <QMessageBox>
#include <QRegularExpression>
#include <QThread>

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
  static const QString kLength = QStringLiteral("length");

  QStringList result;
  const int length = jsValue.property(kLength).toInt();
  result.reserve(length);

  for (int i = 0; i < length; ++i)
    result.append(jsValue.property(static_cast<quint32>(i)).toString());

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
  // Reject non-arrays and empty arrays as scalars
  if (!jsValue.isArray())
    return ArrayType::Scalar;

  static const QString kLength = QStringLiteral("length");
  const int length             = jsValue.property(kLength).toInt();
  if (length == 0)
    return ArrayType::Scalar;

  // Scan elements to classify; early-exit once type is determined
  bool hasArray  = false;
  bool hasScalar = false;

  for (int i = 0; i < length; ++i) {
    if (jsValue.property(static_cast<quint32>(i)).isArray())
      hasArray = true;
    else
      hasScalar = true;

    if (hasArray && hasScalar)
      return ArrayType::ArrayMixed;
  }

  if (hasArray)
    return ArrayType::Array2D;

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
  static const QString kLength = QStringLiteral("length");

  QList<QStringList> results;
  const int rowCount = jsValue.property(kLength).toInt();
  results.reserve(rowCount);

  for (int row = 0; row < rowCount; ++row) {
    const auto rowArray = jsValue.property(static_cast<quint32>(row));

    if (!rowArray.isArray()) [[unlikely]] {
      qWarning() << "[JsScriptEngine] Row" << row << "is not an array, skipping";
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
  static const QString kLength = QStringLiteral("length");
  const int elementCount       = jsValue.property(kLength).toInt();

  constexpr int kMaxElements     = 10000;
  constexpr qsizetype kMaxVecLen = 10000;

  QStringList scalars;
  QList<QStringList> vectors;
  qsizetype maxVectorLength = 0;

  for (int i = 0; i < qMin(elementCount, kMaxElements); ++i) {
    const auto element = jsValue.property(static_cast<quint32>(i));

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

  // Cap to prevent OOM from malformed JS arrays
  maxVectorLength = qMin(maxVectorLength, kMaxVecLen);

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

/**
 * @brief Classifies the JS result and converts it to a list of frames.
 *
 * @param jsResult The value returned by the parse function.
 * @return List of QStringList, each representing one frame.
 */
static QList<QStringList> convertJsResult(const QJSValue& jsResult)
{
  QList<QStringList> results;
  switch (detectArrayType(jsResult)) {
    case ArrayType::Array2D:
      return convert2DArray(jsResult);
    case ArrayType::ArrayMixed:
      return convertMixedArray(jsResult);
    case ArrayType::Array1D:
    case ArrayType::Scalar:
    default:
      results.append(jsArrayToStringList(jsResult));
      return results;
  }
}

//--------------------------------------------------------------------------------------------------
// JsScriptEngine implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the JS engine and configures the runtime watchdog timer.
 */
DataModel::JsScriptEngine::JsScriptEngine()
{
  m_engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  m_watchdog.setSingleShot(true);
  m_watchdog.setInterval(kRuntimeWatchdogMs);
  QObject::connect(&m_watchdog, &QTimer::timeout, [this]() { m_engine.setInterrupted(true); });
}

/**
 * @brief Returns @c true when a callable parse function is loaded.
 */
bool DataModel::JsScriptEngine::isLoaded() const noexcept
{
  return m_parseFunction.isCallable();
}

/**
 * @brief Runs one cycle of JS garbage collection.
 */
void DataModel::JsScriptEngine::collectGarbage()
{
  m_engine.collectGarbage();
}

/**
 * @brief Resets the engine to an unloaded state without destroying it.
 */
void DataModel::JsScriptEngine::reset()
{
  m_parseFunction = QJSValue();
  m_hexToArray    = QJSValue();
}

/**
 * @brief Calls the parse function with a runtime watchdog timer.
 *
 * Starts a single-shot timer before the JS call. If the script exceeds
 * kRuntimeWatchdogMs, the engine is interrupted and an error is returned.
 *
 * @param args Arguments to pass to the parse function.
 * @return The JS result, or an error value if interrupted.
 */
QJSValue DataModel::JsScriptEngine::guardedCall(QJSValueList& args)
{
  Q_ASSERT(m_parseFunction.isCallable());
  Q_ASSERT(!args.isEmpty());

  m_engine.setInterrupted(false);
  m_watchdog.start();
  const auto result = m_parseFunction.call(args);
  m_watchdog.stop();

  if (m_engine.isInterrupted()) [[unlikely]] {
    m_engine.setInterrupted(false);
    qWarning() << "[JsScriptEngine] Script execution timed out after" << kRuntimeWatchdogMs
               << "ms — interrupted";
  }

  return result;
}

/**
 * @brief Executes the JS parse function over text data, returning one or more
 * frames.
 *
 * @param frame Decoded UTF-8 string frame.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::JsScriptEngine::parseString(const QString& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parseFunction.isCallable())
    return {};

  QJSValueList args;
  args << frame;
  const auto jsResult = guardedCall(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] JS error:" << jsResult.property("message").toString();
    return {};
  }

  return convertJsResult(jsResult);
}

/**
 * @brief Executes the JS parse function over binary data, returning one or
 * more frames.
 *
 * @param frame Binary frame data.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::JsScriptEngine::parseBinary(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parseFunction.isCallable())
    return {};

  // Convert binary data to JS array via hex helper or manual loop
  QJSValue jsArray;
  if (m_hexToArray.isCallable()) [[likely]] {
    QJSValueList hexArgs;
    hexArgs << QString::fromLatin1(frame.toHex());
    jsArray = m_hexToArray.call(hexArgs);
  } else {
    jsArray          = m_engine.newArray(frame.size());
    const auto* data = reinterpret_cast<const quint8*>(frame.constData());
    for (int i = 0; i < frame.size(); ++i)
      jsArray.setProperty(i, data[i]);
  }

  QJSValueList args;
  args << jsArray;
  const auto jsResult = guardedCall(args);

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] JS error:" << jsResult.property("message").toString();
    return {};
  }

  return convertJsResult(jsResult);
}

/**
 * @brief Evaluates a script and checks for syntax errors.
 *
 * @param script          The JavaScript source to evaluate.
 * @param sourceId        Source identifier (for diagnostics).
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true if the script evaluated without errors.
 */
bool DataModel::JsScriptEngine::validateScriptSyntax(const QString& script,
                                                     int sourceId,
                                                     bool showMessageBoxes)
{
  Q_ASSERT(!script.isEmpty());
  Q_ASSERT(sourceId >= 0);

  // Reset the engine and evaluate the script
  m_parseFunction = QJSValue();
  m_engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  QStringList exceptionStackTrace;
  auto result = m_engine.evaluate(
    script, QStringLiteral("parser_%1.js").arg(sourceId), 1, &exceptionStackTrace);

  // Report syntax errors
  if (result.isError()) {
    const QString errorMsg = result.property("message").toString();
    const int lineNumber   = result.property("lineNumber").toInt();
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("JavaScript Syntax Error"),
        QObject::tr("The parser code contains a syntax error at line %1:\n\n%2")
          .arg(lineNumber)
          .arg(errorMsg),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Source" << sourceId << "syntax error at line" << lineNumber
                 << ":" << errorMsg;
    }
    return false;
  }

  // Report exceptions raised during evaluation
  if (!exceptionStackTrace.isEmpty()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("JavaScript Exception Occurred"),
        QObject::tr("The parser code triggered the following exceptions:\n\n%1")
          .arg(exceptionStackTrace.join(QStringLiteral("\n"))),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Source" << sourceId
                 << "exceptions:" << exceptionStackTrace.join(QStringLiteral(", "));
    }
    return false;
  }

  return true;
}

/**
 * @brief Validates that the engine's global scope contains a callable 'parse'
 * function.
 *
 * @param sourceId        Source identifier (for diagnostics).
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return The parse function on success, or a null QJSValue on failure.
 */
QJSValue DataModel::JsScriptEngine::validateParseFunction(int sourceId, bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!m_engine.isInterrupted());

  auto parseFunction = m_engine.globalObject().property(QStringLiteral("parse"));
  if (parseFunction.isNull() || !parseFunction.isCallable()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("Missing Parse Function"),
        QObject::tr("The 'parse' function is not defined in the script.\n\n"
                    "Please ensure your code includes:\nfunction parse(frame) { ... }"),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Source" << sourceId << "missing or non-callable parse()";
    }
    return QJSValue();
  }

  return parseFunction;
}

/**
 * @brief Probes the parse function with representative inputs under a watchdog
 * timer to detect runtime errors and infinite loops.
 *
 * @param parseFunction   The callable parse function to test.
 * @param sourceId        Source identifier (for diagnostics).
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true if at least one probe input succeeded.
 */
bool DataModel::JsScriptEngine::probeParseFunction(const QJSValue& parseFunction,
                                                   int sourceId,
                                                   bool showMessageBoxes)
{
  Q_ASSERT(parseFunction.isCallable());
  Q_ASSERT(sourceId >= 0);

  // Prepare three representative probe inputs
  bool probeOk = false;
  QJSValue lastError;
  auto byteProbe = m_engine.newArray(1);
  byteProbe.setProperty(0, 0);
  const QJSValue probeInputs[] = {QJSValue("0"), byteProbe, QJSValue("")};

  // Run probes under a watchdog thread
  {
    QAtomicInt probeDone(0);
    auto* watchdog = QThread::create([this, &probeDone]() {
      constexpr int kTimeoutMs = 500;
      constexpr int kSliceMs   = 20;
      for (int t = 0; t < kTimeoutMs && !probeDone.loadAcquire(); t += kSliceMs)
        QThread::msleep(kSliceMs);

      if (!probeDone.loadAcquire())
        m_engine.setInterrupted(true);
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
    m_engine.setInterrupted(false);
    watchdog->wait();
    watchdog->deleteLater();
  }

  // Report probe failure
  if (!probeOk) {
    const QString errorMsg = lastError.property("message").toString();
    const int lineNumber   = lastError.property("lineNumber").toInt();
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("Parse Function Runtime Error"),
        QObject::tr("The parse function contains an error at line %1:\n\n"
                    "%2\n\n"
                    "Please fix the error in the function body.")
          .arg(lineNumber)
          .arg(errorMsg),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Runtime error at line" << lineNumber << ":" << errorMsg;
    }
  }

  return probeOk;
}

/**
 * @brief Validates and loads a JavaScript frame parser script.
 *
 * For source 0, performs full validation: syntax check, function-signature
 * check, and a probe with a watchdog timer. For all other sources, performs
 * lightweight validation (syntax + callable parse function) without the probe.
 *
 * @param script           The JavaScript source to validate and load.
 * @param sourceId         Target source engine (0 = global).
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true on success, @c false if validation failed.
 */
bool DataModel::JsScriptEngine::loadScript(const QString& script,
                                           int sourceId,
                                           bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

  // Save existing parse function in case validation fails
  QJSValue prevParseFn  = m_parseFunction;
  QJSValue prevHexToArr = m_hexToArray;

  auto restorePrevious = [&]() {
    m_parseFunction = prevParseFn;
    m_hexToArray    = prevHexToArr;
  };

  // Syntax check
  if (!validateScriptSyntax(script, sourceId, showMessageBoxes)) {
    restorePrevious();
    return false;
  }

  // Verify the 'parse' function exists and has a valid signature
  auto parseFunction = validateParseFunction(sourceId, showMessageBoxes);
  if (parseFunction.isNull()) {
    restorePrevious();
    return false;
  }

  // Full signature + probe validation only for source 0
  if (sourceId == 0) {
    static QRegularExpression functionRegex(
      R"(\bfunction\s+parse\s*\(\s*([a-zA-Z_$][a-zA-Z0-9_$]*)(\s*,\s*([a-zA-Z_$][a-zA-Z0-9_$]*))?\s*\))");
    const auto match = functionRegex.match(script);
    if (!match.hasMatch()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          QObject::tr("Invalid Function Declaration"),
          QObject::tr("No valid 'parse' function declaration found.\n\n"
                      "Expected format:\nfunction parse(frame) { ... }"),
          QMessageBox::Critical);
      } else {
        qWarning() << "[JsScriptEngine] No valid 'parse' function "
                      "declaration found";
      }
      restorePrevious();
      return false;
    }

    const QString firstArg  = match.captured(1);
    const QString secondArg = match.captured(3);

    if (firstArg.isEmpty()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          QObject::tr("Invalid Function Parameter"),
          QObject::tr("The 'parse' function must have at least one parameter."
                      "\n\nExpected format:\n"
                      "function parse(frame) { ... }"),
          QMessageBox::Critical);
      } else {
        qWarning() << "[JsScriptEngine] Parse function must have at "
                      "least one parameter";
      }
      restorePrevious();
      return false;
    }

    if (!secondArg.isEmpty()) {
      if (showMessageBoxes) {
        Misc::Utilities::showMessageBox(
          QObject::tr("Deprecated Function Signature"),
          QObject::tr("The 'parse' function uses the old two-parameter "
                      "format: parse(%1, %2)\n\n"
                      "This format is no longer supported. Please update "
                      "to the new single-parameter format:\n"
                      "function parse(%1) { ... }\n\n"
                      "The separator parameter is no longer needed.")
            .arg(firstArg, secondArg),
          QMessageBox::Warning);
      } else {
        qWarning() << "[JsScriptEngine] Deprecated two-parameter "
                      "parse function";
      }
      restorePrevious();
      return false;
    }

    if (!probeParseFunction(parseFunction, sourceId, showMessageBoxes)) {
      restorePrevious();
      return false;
    }
  }

  m_parseFunction = parseFunction;

  // Hex-to-byte-array helper for binary frame parsing
  m_engine.evaluate(QStringLiteral("function __ss_internal_hex_to_array__(h){"
                                   "var n=h.length>>1,a=new Array(n);"
                                   "for(var i=0,j=0;i<h.length;i+=2,j++)"
                                   "a[j]=parseInt(h.substr(i,2),16);return a;}"));
  m_hexToArray = m_engine.globalObject().property(QStringLiteral("__ss_internal_hex_to_array__"));

  return true;
}
