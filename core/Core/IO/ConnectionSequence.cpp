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

#include "Core/IO/ConnectionSequence.h"

#include <array>
#include <QJsonArray>
#include <QJsonDocument>

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

using Kind = IO::SequenceStep::Kind;
using Flow = IO::SequenceStep::Flow;
using Line = IO::SequenceStep::Line;

/**
 * @brief The JSON spelling of one step kind.
 */
struct KindSlug {
  Kind kind;
  const char* slug;
};

static constexpr std::array<KindSlug, 8> kKindSlugs{
  {{Kind::Break, "break"},
   {Kind::Wait, "wait"},
   {Kind::SetLine, "line"},
   {Kind::Send, "send"},
   {Kind::Expect, "expect"},
   {Kind::Discard, "discard"},
   {Kind::SetBaud, "baud"},
   {Kind::SetFlowControl, "flow"}}
};

static constexpr std::array<const char*, 4> kFlowSlugs{"none", "rtscts", "xonxoff", "unknown"};
static constexpr std::array<const char*, 3> kLineSlugs{"dtr", "rts", "unknown"};
static_assert(kFlowSlugs.size() == static_cast<std::size_t>(Flow::Unknown) + 1);
static_assert(kLineSlugs.size() == static_cast<std::size_t>(Line::Unknown) + 1);
static constexpr int kQuotedBytes = 32;

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the JSON slug of @p kind, "unknown" for the sentinel.
 */
static QString kindSlug(Kind kind)
{
  for (const auto& entry : kKindSlugs)
    if (entry.kind == kind)
      return QString::fromLatin1(entry.slug);

  return QStringLiteral("unknown");
}

/**
 * @brief Returns the step kind a JSON slug names, Unknown when it names none.
 */
static Kind kindFromSlug(const QString& slug)
{
  for (const auto& entry : kKindSlugs)
    if (slug == QLatin1String(entry.slug))
      return entry.kind;

  return Kind::Unknown;
}

/**
 * @brief Returns the flow-control mode a JSON value names: None when absent, Unknown when it
 *        names no mode, so validate() can report it instead of silently running without flow.
 */
static Flow flowFromSlug(const QJsonValue& value)
{
  if (value.isUndefined())
    return Flow::None;

  const auto slug = value.toString();
  for (std::size_t i = 0; i < kFlowSlugs.size(); ++i)
    if (slug == QLatin1String(kFlowSlugs[i]))
      return static_cast<Flow>(i);

  return Flow::Unknown;
}

/**
 * @brief Returns the control line a JSON value names: DTR when absent, Unknown when it names
 *        neither line, so validate() can report it instead of silently driving DTR.
 */
static Line lineFromSlug(const QJsonValue& value)
{
  if (value.isUndefined())
    return Line::Dtr;

  const auto slug = value.toString();
  for (std::size_t i = 0; i < kLineSlugs.size(); ++i)
    if (slug.compare(QLatin1String(kLineSlugs[i]), Qt::CaseInsensitive) == 0)
      return static_cast<Line>(i);

  return Line::Unknown;
}

/**
 * @brief Returns the value of one hex digit, or -1 when @p c is not one.
 */
static int hexDigit(QChar c)
{
  const char16_t u = c.toLower().unicode();
  if (u >= u'0' && u <= u'9')
    return u - u'0';

  if (u >= u'a' && u <= u'f')
    return 10 + (u - u'a');

  return -1;
}

/**
 * @brief Whether @p text is whole hex byte pairs once spaces are dropped, checked without the
 *        warning SerialStudio::hexToBytes() logs.
 */
static bool isHexBytes(const QString& text)
{
  QString compact = text;
  compact.remove(QLatin1Char(' '));
  if (compact.isEmpty() || compact.size() % 2 != 0)
    return false;

  for (const QChar c : std::as_const(compact))
    if (hexDigit(c) < 0)
      return false;

  return true;
}

//--------------------------------------------------------------------------------------------------
// Expect patterns
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses "5D ?? 7F" into bytes plus masks; a '?' nibble matches anything. Whitespace is
 *        ignored, so "5D??7F" parses the same. Fails on an odd digit count or a foreign character.
 */
bool IO::ExpectPattern::parse(const QString& text, ExpectPattern& out)
{
  QString compact = text.simplified();
  compact.remove(QLatin1Char(' '));
  if (compact.isEmpty() || compact.size() % 2 != 0)
    return false;

  ExpectPattern parsed;
  for (qsizetype i = 0; i < compact.size(); i += 2) {
    int value = 0;
    int mask  = 0;
    for (qsizetype n = 0; n < 2; ++n) {
      const QChar c   = compact.at(i + n);
      const int shift = n == 0 ? 4 : 0;
      if (c == QLatin1Char('?'))
        continue;

      const int digit = hexDigit(c);
      if (digit < 0)
        return false;

      value |= digit << shift;
      mask  |= 0x0F << shift;
    }

    parsed.value.append(static_cast<char>(value));
    parsed.mask.append(static_cast<char>(mask));
  }

  SS_ASSERT_LOG(parsed.value.size() == parsed.mask.size());
  SS_ASSERT_LOG(parsed.value.size() == compact.size() / 2);

  out = parsed;
  return true;
}

/**
 * @brief Returns the offset just past the first match of this pattern in @p buffer, or -1. The
 *        caller consumes through that offset, so a reply is never matched twice.
 */
qsizetype IO::ExpectPattern::matchIn(const QByteArray& buffer) const
{
  SS_ASSERT(value.size() == mask.size(), return -1);

  const qsizetype n = value.size();
  if (n == 0 || buffer.size() < n)
    return -1;

  for (qsizetype start = 0; start + n <= buffer.size(); ++start) {
    bool hit = true;
    for (qsizetype k = 0; k < n && hit; ++k) {
      const auto b = static_cast<quint8>(buffer.at(start + k));
      const auto m = static_cast<quint8>(mask.at(k));
      hit          = (b & m) == static_cast<quint8>(value.at(k));
    }

    if (hit)
      return start + n;
  }

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Sequence step
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the fields this step's kind reads.
 */
QJsonObject IO::SequenceStep::toJson() const
{
  QJsonObject obj;
  obj.insert(QStringLiteral("kind"), kindSlug(kind));
  switch (kind) {
    case Kind::Break:
    case Kind::Wait:
      obj.insert(QStringLiteral("ms"), ms);
      break;
    case Kind::SetLine:
      obj.insert(QStringLiteral("line"), kLineSlugs[static_cast<std::size_t>(line)]);
      obj.insert(QStringLiteral("high"), high);
      break;
    case Kind::Send:
      obj.insert(QStringLiteral("data"), data);
      obj.insert(QStringLiteral("hex"), hex);
      obj.insert(QStringLiteral("eol"), eol);
      obj.insert(QStringLiteral("checksum"), checksum);
      break;
    case Kind::Expect:
      obj.insert(QStringLiteral("patterns"), QJsonArray::fromStringList(patterns));
      obj.insert(QStringLiteral("timeoutMs"), timeoutMs);
      obj.insert(QStringLiteral("optional"), optional);
      break;
    case Kind::SetBaud:
      obj.insert(QStringLiteral("rate"), rate);
      break;
    case Kind::SetFlowControl:
      obj.insert(QStringLiteral("mode"), kFlowSlugs[static_cast<std::size_t>(flow)]);
      break;
    case Kind::Discard:
    case Kind::Unknown:
      break;
  }

  return obj;
}

/**
 * @brief Reads one step; an unknown kind is kept as Unknown so validate() can name it.
 */
IO::SequenceStep IO::SequenceStep::fromJson(const QJsonObject& obj)
{
  SequenceStep step;
  step.kind      = kindFromSlug(obj.value(QStringLiteral("kind")).toString());
  step.ms        = obj.value(QStringLiteral("ms")).toInt(0);
  step.timeoutMs = obj.value(QStringLiteral("timeoutMs")).toInt(1000);
  step.rate      = obj.value(QStringLiteral("rate")).toInt(9600);
  step.high      = obj.value(QStringLiteral("high")).toBool(true);
  step.hex       = obj.value(QStringLiteral("hex")).toBool(false);
  step.optional  = obj.value(QStringLiteral("optional")).toBool(false);
  step.data      = obj.value(QStringLiteral("data")).toString();
  step.eol       = obj.value(QStringLiteral("eol")).toString();
  step.checksum  = obj.value(QStringLiteral("checksum")).toString();
  step.flow      = flowFromSlug(obj.value(QStringLiteral("mode")));
  step.line      = lineFromSlug(obj.value(QStringLiteral("line")));

  const auto array = obj.value(QStringLiteral("patterns")).toArray();
  for (const auto& item : array)
    step.patterns.append(item.toString());

  return step;
}

/**
 * @brief The bytes a send step puts on the wire: the payload (hex pairs, or text with escapes as
 *        UTF-8), then the end-of-line escape text, then the named checksum over both, the order
 *        an action uses.
 */
QByteArray IO::SequenceStep::sendBytes() const
{
  SS_ASSERT(kind == Kind::Send, return {});

  QByteArray bytes =
    hex ? SerialStudio::hexToBytes(data) : SerialStudio::resolveEscapeSequences(data).toUtf8();
  bytes.append(SerialStudio::resolveEscapeSequences(eol).toUtf8());
  if (!checksum.isEmpty())
    bytes.append(IO::checksum(checksum, bytes));

  return bytes;
}

/**
 * @brief Parses this step's patterns, skipping any that do not parse (validate() reports them).
 */
QList<IO::ExpectPattern> IO::SequenceStep::expectPatterns() const
{
  QList<ExpectPattern> out;
  for (const auto& text : patterns) {
    ExpectPattern pattern;
    if (ExpectPattern::parse(text, pattern))
      out.append(pattern);
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Connection sequence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether the sequence has no step, which is the default and today's behaviour.
 */
bool IO::ConnectionSequence::isEmpty() const noexcept
{
  return steps.isEmpty();
}

/**
 * @brief Serializes the sequence as the source's connection settings store it.
 */
QJsonObject IO::ConnectionSequence::toJson() const
{
  QJsonArray array;
  for (const auto& step : steps)
    array.append(step.toJson());

  QJsonObject obj;
  obj.insert(QStringLiteral("budgetMs"), budgetMs);
  obj.insert(QStringLiteral("steps"), array);
  return obj;
}

/**
 * @brief Reads a sequence from its JSON object; an absent or malformed object is empty.
 */
IO::ConnectionSequence IO::ConnectionSequence::fromJson(const QJsonObject& obj)
{
  ConnectionSequence sequence;
  sequence.budgetMs = obj.value(QStringLiteral("budgetMs")).toInt(kDefaultSequenceBudgetMs);

  const auto array = obj.value(QStringLiteral("steps")).toArray();
  for (const auto& item : array)
    sequence.steps.append(SequenceStep::fromJson(item.toObject()));

  return sequence;
}

/**
 * @brief Reads a sequence from the shapes a driver property arrives in: a QJsonObject (the UI
 *        driver's own property), a QVariantMap (applyConnectionSettings) or JSON text (QML).
 */
IO::ConnectionSequence IO::ConnectionSequence::fromVariant(const QVariant& value)
{
  const int type = value.typeId();
  if (type == QMetaType::QJsonObject)
    return fromJson(value.toJsonObject());

  if (type == QMetaType::QJsonValue)
    return fromJson(value.toJsonValue().toObject());

  if (type == QMetaType::QVariantMap)
    return fromJson(QJsonObject::fromVariantMap(value.toMap()));

  if (type == QMetaType::QString || type == QMetaType::QByteArray)
    return fromJson(QJsonDocument::fromJson(value.toByteArray()).object());

  return {};
}

/**
 * @brief Returns the first problem in the sequence, or an empty message when it is valid. Fixed
 *        waits (BREAK and Wait steps) are summed as they go, so a budget too small to cover them
 *        is attributed to the step that crossed it.
 */
IO::SequenceError IO::ConnectionSequence::validate() const
{
  if (budgetMs < 1 || budgetMs > kMaxSequenceBudgetMs)
    return {-1, tr("The time budget must be between 1 and %1 ms").arg(kMaxSequenceBudgetMs)};

  if (steps.size() > kMaxSequenceSteps)
    return {-1, tr("A Connection Sequence holds at most %1 steps").arg(kMaxSequenceSteps)};

  qint64 fixed = 0;
  for (qsizetype i = 0; i < steps.size(); ++i) {
    const auto& step = steps.at(i);
    const auto index = static_cast<int>(i);

    const auto problem = stepProblem(step);
    if (!problem.isEmpty())
      return {index, problem};

    if (step.kind != Kind::Break && step.kind != Kind::Wait)
      continue;

    SS_ASSERT_LOG(step.ms >= 0);
    fixed += step.ms;
    if (fixed > budgetMs)
      return {index,
              tr("The fixed waits up to this step (%1 ms) exceed the %2 ms time budget")
                .arg(QString::number(fixed), QString::number(budgetMs))};
  }

  SS_ASSERT_LOG(fixed >= 0 && fixed <= budgetMs);
  return {};
}

/**
 * @brief Returns the problem with one step considered alone, or an empty string.
 */
QString IO::ConnectionSequence::stepProblem(const SequenceStep& step)
{
  switch (step.kind) {
    case Kind::Break:
      if (step.ms < 1 || step.ms > kMaxBreakMs)
        return tr("A BREAK lasts 1 to %1 ms").arg(kMaxBreakMs);

      return {};
    case Kind::Wait:
      if (step.ms < 0 || step.ms > kMaxSequenceBudgetMs)
        return tr("A wait lasts 0 to %1 ms").arg(kMaxSequenceBudgetMs);

      return {};
    case Kind::Send:
      return sendProblem(step);
    case Kind::Expect:
      return expectProblem(step);
    case Kind::SetBaud:
      if (step.rate < 1 || step.rate > kMaxSequenceBaudRate)
        return tr("The baud rate must be between 1 and %1").arg(kMaxSequenceBaudRate);

      return {};
    case Kind::SetLine:
      return step.line == Line::Unknown ? tr("Unknown control line: use DTR or RTS") : QString();
    case Kind::SetFlowControl:
      return step.flow == Flow::Unknown ? tr("Unknown flow control mode") : QString();
    case Kind::Discard:
      return {};
    case Kind::Unknown:
      break;
  }

  return tr("Unknown step kind");
}

/**
 * @brief Returns the problem with a send step: no data, malformed hex or an unknown checksum.
 */
QString IO::ConnectionSequence::sendProblem(const SequenceStep& step)
{
  SS_ASSERT_LOG(step.kind == Kind::Send);

  if (step.data.isEmpty())
    return tr("A send step needs data");

  if (step.hex && !isHexBytes(step.data))
    return tr("\"%1\" is not valid hexadecimal: use byte pairs such as 15 15").arg(step.data);

  if (!IO::availableChecksums().contains(step.checksum))
    return tr("Unknown checksum \"%1\"").arg(step.checksum);

  return {};
}

/**
 * @brief Returns the problem with an expect step: no pattern, a malformed or oversized pattern,
 *        or a timeout out of range.
 */
QString IO::ConnectionSequence::expectProblem(const SequenceStep& step)
{
  SS_ASSERT_LOG(step.kind == Kind::Expect);

  if (step.patterns.isEmpty())
    return tr("An expect step needs at least one pattern");

  for (const auto& text : step.patterns) {
    ExpectPattern pattern;
    if (!ExpectPattern::parse(text, pattern))
      return tr("Pattern \"%1\" is not valid: use hex bytes, with ?? for any byte").arg(text);

    if (pattern.value.size() > kExpectBufferBytes)
      return tr("Pattern \"%1\" is longer than the %2-byte receive buffer")
        .arg(text, QString::number(kExpectBufferBytes));
  }

  if (step.timeoutMs < 1 || step.timeoutMs > kMaxSequenceBudgetMs)
    return tr("The reply timeout must be between 1 and %1 ms").arg(kMaxSequenceBudgetMs);

  return {};
}

/**
 * @brief The console line for step @p index: its position and what it does.
 */
QString IO::ConnectionSequence::stepText(int index) const
{
  SS_ASSERT(index >= 0 && index < steps.size(), return {});

  return tr("Step %1 of %2: %3")
    .arg(QString::number(index + 1), QString::number(steps.size()), stepSummary(steps.at(index)));
}

/**
 * @brief The user-facing name of a step kind, as the editor's kind list shows it.
 */
QString IO::ConnectionSequence::kindLabel(SequenceStep::Kind kind)
{
  switch (kind) {
    case Kind::Break:
      return tr("Send BREAK");
    case Kind::Wait:
      return tr("Wait");
    case Kind::SetLine:
      return tr("Set Line");
    case Kind::Send:
      return tr("Send Data");
    case Kind::Expect:
      return tr("Expect Reply");
    case Kind::Discard:
      return tr("Discard Input");
    case Kind::SetBaud:
      return tr("Set Baud Rate");
    case Kind::SetFlowControl:
      return tr("Set Flow Control");
    case Kind::Unknown:
      break;
  }

  return tr("Unknown");
}

/**
 * @brief Renders bytes as spaced upper-case hex, the first 32 only, for quoting a reply.
 */
QString IO::ConnectionSequence::hexText(const QByteArray& bytes)
{
  if (bytes.isEmpty())
    return tr("nothing");

  auto text = QString::fromLatin1(bytes.left(kQuotedBytes).toHex(' ')).toUpper();
  if (bytes.size() > kQuotedBytes)
    text.append(QStringLiteral(" ..."));

  return text;
}

/**
 * @brief One sentence naming what @p step does, the body of its console line.
 */
QString IO::ConnectionSequence::stepSummary(const SequenceStep& step)
{
  const auto patterns = step.patterns.join(QStringLiteral(" | "));
  const auto lineName = step.line == Line::Rts ? QStringLiteral("RTS") : QStringLiteral("DTR");
  switch (step.kind) {
    case Kind::Break:
      return tr("Send BREAK for %1 ms").arg(step.ms);
    case Kind::Wait:
      return tr("Wait %1 ms").arg(step.ms);
    case Kind::SetLine:
      return step.high ? tr("Set %1 high").arg(lineName) : tr("Set %1 low").arg(lineName);
    case Kind::Send:
      return step.hex ? tr("Send %1").arg(step.data.toUpper()) : tr("Send \"%1\"").arg(step.data);
    case Kind::Expect:
      if (step.optional)
        return tr("Expect %1 within %2 ms (optional)")
          .arg(patterns, QString::number(step.timeoutMs));

      return tr("Expect %1 within %2 ms").arg(patterns, QString::number(step.timeoutMs));
    case Kind::Discard:
      return tr("Discard pending input");
    case Kind::SetBaud:
      return tr("Set baud rate to %1").arg(step.rate);
    case Kind::SetFlowControl:
      if (step.flow == Flow::RtsCts)
        return tr("Set flow control to RTS/CTS");

      return step.flow == Flow::XonXoff ? tr("Set flow control to XON/XOFF")
                                        : tr("Set flow control to None");
    case Kind::Unknown:
      break;
  }

  return kindLabel(step.kind);
}
