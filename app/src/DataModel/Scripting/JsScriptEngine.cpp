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

#include "DataModel/Scripting/JsScriptEngine.h"

#include <QMessageBox>
#include <QRegularExpression>

#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/DashboardApi.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Multi-frame parsing helper functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Classifies JavaScript return-value shapes from the parse function.
 */
enum class ArrayType {
  Scalar,
  Array1D,
  Array2D,
  ArrayMixed
};

/**
 * @brief Converts a JavaScript array to a QStringList.
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
 * @brief Classifies the JS value as scalar, 1D, 2D, or mixed array.
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
 * @brief Converts a pure 2D JavaScript array to one frame per row.
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
 * @brief Unzips a mixed scalar/vector JS array into one frame per vector index.
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
 */
static QList<QStringList> convertJsResult(const QJSValue& jsResult)
{
  static const QString kLength = QStringLiteral("length");

  // Non-array (or empty) returns the legacy single empty-or-scalar frame
  if (!jsResult.isArray()) {
    QList<QStringList> results;
    results.append(jsArrayToStringList(jsResult));
    return results;
  }

  // Fused single-pass 1D conversion; bail to the classifier on the first nested array
  const int length = jsResult.property(kLength).toInt();

  QStringList scalars;
  scalars.reserve(length);
  for (int i = 0; i < length; ++i) {
    const auto element = jsResult.property(static_cast<quint32>(i));

    // A nested array means 2D or mixed; only a full re-scan can tell them apart
    if (element.isArray()) [[unlikely]] {
      return detectArrayType(jsResult) == ArrayType::Array2D ? convert2DArray(jsResult)
                                                             : convertMixedArray(jsResult);
    }

    scalars.append(element.toString());
  }

  QList<QStringList> results;
  results.append(scalars);
  return results;
}

//--------------------------------------------------------------------------------------------------
// JsScriptEngine implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the JS engine with safe extensions and the runtime watchdog.
 */
DataModel::JsScriptEngine::JsScriptEngine()
  : m_watchdog(&m_engine, kRuntimeWatchdogMs, QStringLiteral("JsScriptEngine"))
  , m_disabled(false)
  , m_sourceId(0)
  , m_consecutiveTimeouts(0)
{
  m_engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  // Expose notify* + level constants (gated on the active license tier)
  DataModel::NotificationCenter::installScriptApi(&m_engine);

  // Expose tableGet / tableSet / datasetGetRaw / datasetGetFinal
  DataModel::FrameBuilder::instance().injectTableApiJS(&m_engine);

  // Expose deviceWrite(data, sourceId?) bound to source 0 until loadScript() updates it
  DataModel::DeviceWriteApi::installJS(&m_engine, m_sourceId);

  // Expose actionFire(actionId) for firing dashboard actions from the parser
  DataModel::ActionFireApi::installJS(&m_engine);

  // Expose dashboard.* helpers (clearPlots, setPlotPoints, UI toggles, setActiveWorkspace)
  DataModel::DashboardApi::installJS(&m_engine);

  // Expose apiCall(method, params?): gated to a read-only allow-list by default
  DataModel::ScriptApiCall::installJS(&m_engine, m_sourceId);
}

/**
 * @brief Records a watchdog timeout and disables the parser after too many in a row.
 */
bool DataModel::JsScriptEngine::noteTimeoutAndCheckDisabled(int sourceId)
{
  ++m_consecutiveTimeouts;
  if (m_consecutiveTimeouts < kMaxConsecutiveTimeouts)
    return false;

  m_disabled = true;
  qWarning() << "[JsScriptEngine] Source" << sourceId << "disabled after" << kMaxConsecutiveTimeouts
             << "consecutive watchdog timeouts.";
  Misc::Utilities::showMessageBox(
    QObject::tr("Frame Parser Disabled"),
    QObject::tr("The JavaScript frame parser for source %1 timed out %2 frames "
                "in a row and has been disabled to keep Serial Studio responsive.\n\n"
                "Most likely cause: an infinite loop or extremely slow operation "
                "in the script body. Fix the script and reload the project to "
                "re-enable parsing.")
      .arg(sourceId)
      .arg(kMaxConsecutiveTimeouts),
    QMessageBox::Critical);
  return true;
}

/**
 * @brief Clears the consecutive-timeout counter after a successful parse.
 */
void DataModel::JsScriptEngine::resetTimeoutCounter() noexcept
{
  m_consecutiveTimeouts = 0;
}

/**
 * @brief Returns true once a parse() function has been compiled into the engine.
 */
bool DataModel::JsScriptEngine::isLoaded() const noexcept
{
  return m_parseFunction.isCallable();
}

/**
 * @brief Returns the ScriptLanguage implemented by this engine.
 */
int DataModel::JsScriptEngine::language() const noexcept
{
  return SerialStudio::JavaScript;
}

/**
 * @brief Runs one cycle of QJSEngine garbage collection.
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
  m_parseFunction       = QJSValue();
  m_hexToArray          = QJSValue();
  m_disabled            = false;
  m_consecutiveTimeouts = 0;
}

/**
 * @brief Calls the parse function under a runtime watchdog timer.
 */
QJSValue DataModel::JsScriptEngine::guardedCall(QJSValueList& args)
{
  Q_ASSERT(m_parseFunction.isCallable());
  Q_ASSERT(!args.isEmpty());

  return m_watchdog.call(m_parseFunction, args);
}

/**
 * @brief Runs the JS parse function over a text frame.
 */
QList<QStringList> DataModel::JsScriptEngine::parseString(const QString& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parseFunction.isCallable() || m_disabled)
    return {};

  QJSValueList args;
  args << frame;
  const auto jsResult = guardedCall(args);

  if (m_watchdog.lastCallTimedOut()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] parse() timed out after" << kRuntimeWatchdogMs << "ms";
    (void)noteTimeoutAndCheckDisabled(0);
    return {};
  }

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] JS error:" << jsResult.property("message").toString();
    return {};
  }

  resetTimeoutCounter();
  return convertJsResult(jsResult);
}

/**
 * @brief Runs the JS parse function over a binary frame.
 */
QList<QStringList> DataModel::JsScriptEngine::parseBinary(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parseFunction.isCallable() || m_disabled)
    return {};

  // Convert binary data to JS array via hex helper or manual loop
  QJSValue jsArray;
  if (m_hexToArray.isCallable()) [[likely]] {
    QJSValueList hexArgs;
    hexArgs << QString::fromLatin1(frame.toHex());
    jsArray = m_hexToArray.call(hexArgs);

    // Fall back to manual loop if the hex helper failed
    if (jsArray.isError()) [[unlikely]] {
      qWarning() << "[JsScriptEngine] hex helper error:" << jsArray.property("message").toString();
      jsArray = QJSValue();
    }
  }

  if (!jsArray.isArray()) {
    jsArray          = m_engine.newArray(frame.size());
    const auto* data = reinterpret_cast<const quint8*>(frame.constData());
    for (int i = 0; i < frame.size(); ++i)
      jsArray.setProperty(i, data[i]);
  }

  QJSValueList args;
  args << jsArray;
  const auto jsResult = guardedCall(args);

  if (m_watchdog.lastCallTimedOut()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] parse() timed out after" << kRuntimeWatchdogMs << "ms";
    (void)noteTimeoutAndCheckDisabled(0);
    return {};
  }

  if (jsResult.isError()) [[unlikely]] {
    qWarning() << "[JsScriptEngine] JS error:" << jsResult.property("message").toString();
    return {};
  }

  resetTimeoutCounter();
  return convertJsResult(jsResult);
}

/**
 * @brief Evaluates the script and reports any syntax errors.
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

  // Arm the watchdog around evaluate() so a top-level infinite loop is interrupted off-thread.
  QStringList exceptionStackTrace;
  m_watchdog.arm();
  auto result = m_engine.evaluate(
    script, QStringLiteral("parser_%1.js").arg(sourceId), 1, &exceptionStackTrace);
  m_watchdog.disarm();

  // Report a watchdog interruption (top-level evaluation that exceeded the time budget)
  if (m_engine.isInterrupted()) {
    m_engine.setInterrupted(false);
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("JavaScript Timed Out"),
        QObject::tr("The parser code did not finish evaluating within %1 ms and was "
                    "interrupted.\n\nMost likely cause: an infinite loop at the top level "
                    "of the script.")
          .arg(kRuntimeWatchdogMs),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Source" << sourceId << "evaluation timed out after"
                 << kRuntimeWatchdogMs << "ms -- interrupted";
    }
    return false;
  }

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
 * @brief Returns the global 'parse' function, or a null QJSValue on failure.
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
 * @brief Probes parse() with sample inputs under the runtime watchdog.
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

  // Install candidate as m_parseFunction so guardedCall protects each probe
  QJSValue savedParseFn = m_parseFunction;
  m_parseFunction       = parseFunction;

  for (const auto& input : probeInputs) {
    QJSValueList probeArgs;
    probeArgs << input;
    const auto probeResult = guardedCall(probeArgs);
    if (!probeResult.isError()) {
      probeOk = true;
      break;
    }

    lastError = probeResult;
  }

  m_parseFunction = savedParseFn;

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
 */
bool DataModel::JsScriptEngine::loadScript(const QString& script,
                                           int sourceId,
                                           bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

  QJSValue prevParseFn  = m_parseFunction;
  QJSValue prevHexToArr = m_hexToArray;

  auto restorePrevious = [&]() {
    m_parseFunction = prevParseFn;
    m_hexToArray    = prevHexToArr;
  };

  if (!validateScriptSyntax(script, sourceId, showMessageBoxes)) {
    restorePrevious();
    return false;
  }

  auto parseFunction = validateParseFunction(sourceId, showMessageBoxes);
  if (parseFunction.isNull()) {
    restorePrevious();
    return false;
  }

  if (sourceId == 0) {
    if (!validateParseSignature(script, showMessageBoxes)) {
      restorePrevious();
      return false;
    }

    if (!probeParseFunction(parseFunction, sourceId, showMessageBoxes)) {
      restorePrevious();
      return false;
    }
  }

  m_parseFunction = parseFunction;

  m_engine.evaluate(QStringLiteral("function __ss_internal_hex_to_array__(h){"
                                   "var n=h.length>>1,a=new Array(n);"
                                   "for(var i=0,j=0;i<h.length;i+=2,j++)"
                                   "a[j]=parseInt(h.substr(i,2),16);return a;}"));
  m_hexToArray = m_engine.globalObject().property(QStringLiteral("__ss_internal_hex_to_array__"));

  // Re-bind deviceWrite() to this source so the optional sourceId argument defaults correctly
  m_sourceId = sourceId;
  DataModel::DeviceWriteApi::installJS(&m_engine, m_sourceId);

  // Re-arm the circuit breaker on fresh script
  m_disabled            = false;
  m_consecutiveTimeouts = 0;

  return true;
}

/**
 * @brief Verifies the script defines a callable parse() function with a usable arity.
 */
bool DataModel::JsScriptEngine::validateParseSignature(const QString& script, bool showMessageBoxes)
{
  static QRegularExpression legacyTwoArgRegex(
    R"(\bparse\b\s*(?:=\s*)?(?:function)?\s*\(\s*([a-zA-Z_$][\w$]*)\s*,\s*([a-zA-Z_$][\w$]*)\s*\))");
  if (const auto legacy = legacyTwoArgRegex.match(script); legacy.hasMatch()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("Deprecated Function Signature"),
        QObject::tr("The 'parse' function uses the old two-parameter "
                    "format: parse(%1, %2)\n\n"
                    "This format is no longer supported. Please update "
                    "to the new single-parameter format:\n"
                    "function parse(%1) { ... }\n\n"
                    "The separator parameter is no longer needed.")
          .arg(legacy.captured(1), legacy.captured(2)),
        QMessageBox::Warning);
    } else {
      qWarning() << "[JsScriptEngine] Deprecated two-parameter parse function";
    }
    return false;
  }

  // Engine-side check: parse must exist as a callable
  const auto parseFn = m_engine.globalObject().property(QStringLiteral("parse"));
  if (!parseFn.isCallable()) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(QObject::tr("Invalid Function Declaration"),
                                      QObject::tr("No callable 'parse' export found.\n\n"
                                                  "Define one of:\n"
                                                  "  function parse(frame) { ... }\n"
                                                  "  const parse = (frame) => { ... }"),
                                      QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] No callable parse function found";
    }
    return false;
  }

  const int arity = parseFn.property(QStringLiteral("length")).toInt();
  if (arity < 1) {
    if (showMessageBoxes) {
      Misc::Utilities::showMessageBox(
        QObject::tr("Invalid Function Parameter"),
        QObject::tr("The 'parse' function must accept at least one parameter (the frame "
                    "payload)."),
        QMessageBox::Critical);
    } else {
      qWarning() << "[JsScriptEngine] Parse function must take at least one parameter";
    }
    return false;
  }

  return true;
}
