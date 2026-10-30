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

#include "DataModel/Scripting/LineFormat.h"

#include <cstring>
#include <system_error>

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "Core/ThirdParty/fast_float.h"
#include "DataModel/Scripting/JsonLeafScanner.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

namespace LineFormatDetail {

struct SeparatorCandidate {
  char separator;
  bool decimalComma;
};

/**
 * @brief Candidate order from spec 0106 R9: semicolon and space admit a decimal comma, comma and
 *        tab do not, and a single field is the last resort.
 */
inline constexpr std::array<SeparatorCandidate, 5> kSeparatorCandidates{
  {{';', true}, {'\t', false}, {',', false}, {' ', true}, {'\0', true}}
};

}  // namespace LineFormatDetail

//--------------------------------------------------------------------------------------------------
// Character classes
//--------------------------------------------------------------------------------------------------

/**
 * @brief ASCII space, tab, CR or LF.
 */
[[nodiscard]] static bool isLineSpace(const char c) noexcept
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/**
 * @brief ASCII decimal digit.
 */
[[nodiscard]] static bool isDigit(const char c) noexcept
{
  return c >= '0' && c <= '9';
}

/**
 * @brief First byte a field name may start with.
 */
[[nodiscard]] static bool isNameStart(const char c) noexcept
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

/**
 * @brief Byte a field name may continue with; inner spaces allow "Temp Sensor".
 */
[[nodiscard]] static bool isNameChar(const char c) noexcept
{
  return isNameStart(c) || isDigit(c) || c == ' ';
}

/**
 * @brief The two name delimiters of a named field.
 */
[[nodiscard]] static bool isNameDelimiter(const char c) noexcept
{
  return c == ':' || c == '=';
}

/**
 * @brief A field name starts with a letter or underscore and continues with letters, digits,
 *        underscores or inner spaces.
 */
[[nodiscard]] static bool isFieldName(QByteArrayView text) noexcept
{
  if (text.isEmpty() || !isNameStart(text.front()))
    return false;

  for (qsizetype i = 1; i < text.size(); ++i)
    if (!isNameChar(text.at(i)))
      return false;

  return true;
}

/**
 * @brief A unit is 1 to kMaxUnitBytes bytes with no digit, whitespace or field punctuation, so
 *        ":30:45", ",hum=60" and "x1F" are rejected while "hPa", "%" and UTF-8 degree signs pass.
 */
[[nodiscard]] static bool isUnitSuffix(QByteArrayView text) noexcept
{
  if (text.isEmpty() || text.size() > DataModel::kMaxUnitBytes)
    return false;

  for (const char c : text)
    if (isDigit(c) || isLineSpace(c) || c == ',' || c == ';' || isNameDelimiter(c))
      return false;

  return true;
}

/**
 * @brief Longest numeric prefix of [first, last) via fast_float; nullptr when none.
 */
[[nodiscard]] static const char* parseNumberPrefix(const char* first,
                                                   const char* last,
                                                   double& number) noexcept
{
  SS_ASSERT(first != nullptr, return nullptr);
  SS_ASSERT(last >= first, return nullptr);

  constexpr auto kFormat =
    fast_float::chars_format::general | fast_float::chars_format::allow_leading_plus;
  const auto result = fast_float::from_chars(first, last, number, kFormat);
  if (result.ec != std::errc() || result.ptr == first)
    return nullptr;

  return result.ptr;
}

//--------------------------------------------------------------------------------------------------
// Trimming and wrapper
//--------------------------------------------------------------------------------------------------

/**
 * @brief Trims ASCII space, tab, CR and LF from both ends.
 */
QByteArrayView DataModel::trimLineAscii(QByteArrayView text) noexcept
{
  qsizetype s = 0;
  qsizetype e = text.size();
  while (s < e && isLineSpace(text.at(s)))
    ++s;

  while (e > s && isLineSpace(text.at(e - 1)))
    --e;

  SS_ASSERT(e >= s, return QByteArrayView());
  SS_ASSERT(text.size() >= e, return QByteArrayView());
  return text.sliced(s, e - s);
}

/**
 * @brief Trims the line and removes a surrounding slash-star comment pair.
 */
QByteArrayView DataModel::stripLineWrapper(QByteArrayView line, bool& wrapped) noexcept
{
  const auto text = trimLineAscii(line);
  wrapped         = text.size() >= 4 && text.startsWith("/*") && text.endsWith("*/");
  if (!wrapped)
    return text;

  SS_ASSERT(text.size() >= 4, return text);
  return trimLineAscii(text.sliced(2, text.size() - 4));
}

//--------------------------------------------------------------------------------------------------
// Numbers and fields
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number with an optional unit suffix; a decimal comma is rewritten on the stack because
 *        fast_float only knows the point, and the unit span still points into the caller's text.
 */
bool DataModel::parseLineNumber(QByteArrayView text,
                                const bool decimalComma,
                                double& number,
                                QByteArrayView& unit) noexcept
{
  unit             = QByteArrayView();
  number           = 0.0;
  const auto token = trimLineAscii(text);
  if (token.isEmpty())
    return false;

  const char lead = token.front();
  if (!isDigit(lead) && lead != '+' && lead != '-' && lead != '.')
    return false;

  char rewritten[kMaxNumberTokenBytes];
  const char* first     = token.data();
  const char* last      = first + token.size();
  const qsizetype comma = decimalComma ? token.indexOf(',') : -1;
  if (comma >= 0) {
    if (token.size() > kMaxNumberTokenBytes || token.indexOf('.') >= 0)
      return false;

    SS_ASSERT(kMaxNumberTokenBytes > comma, return false);
    std::memcpy(rewritten, token.data(), static_cast<size_t>(token.size()));
    rewritten[comma] = '.';
    first            = rewritten;
    last             = rewritten + token.size();
  }

  const char* end = parseNumberPrefix(first, last, number);
  if (end == nullptr)
    return false;

  SS_ASSERT(end > first && last >= end, return false);
  const auto rest = trimLineAscii(token.sliced(end - first));
  if (rest.isEmpty())
    return true;

  if (!isUnitSuffix(rest)) {
    number = 0.0;
    return false;
  }

  unit = rest;
  return true;
}

/**
 * @brief One token as a named pair when its prefix is a well-formed name, positional otherwise.
 */
bool DataModel::parseLineField(QByteArrayView token,
                               const bool decimalComma,
                               LineField& out) noexcept
{
  out             = LineField();
  const auto text = trimLineAscii(token);
  out.token       = text;
  out.value       = text;
  if (text.isEmpty())
    return false;

  qsizetype delimiter = -1;
  for (qsizetype i = 0; i < text.size() && delimiter < 0; ++i)
    if (isNameDelimiter(text.at(i)))
      delimiter = i;

  SS_ASSERT(text.size() > delimiter, return false);
  if (delimiter > 0) {
    const auto name = trimLineAscii(text.first(delimiter));
    if (isFieldName(name)) {
      out.name  = name;
      out.value = trimLineAscii(text.sliced(delimiter + 1));
    }
  }

  out.numeric = parseLineNumber(out.value, decimalComma, out.number, out.unit);
  return out.numeric;
}

//--------------------------------------------------------------------------------------------------
// Splitting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fills a named field from a "name:" token and the value token that followed it.
 */
static void setNamedField(DataModel::LineField& field,
                          QByteArrayView name,
                          QByteArrayView value,
                          const bool decimalComma) noexcept
{
  field         = DataModel::LineField();
  field.name    = name;
  field.value   = DataModel::trimLineAscii(value);
  field.numeric = DataModel::parseLineNumber(field.value, decimalComma, field.number, field.unit);

  const char* tokenEnd = field.value.data() + field.value.size();
  SS_ASSERT(tokenEnd >= name.data(), return);
  field.token = QByteArrayView(name.data(), tokenEnd - name.data());
}

/**
 * @brief The whole line as one field.
 */
static void splitLineWhole(QByteArrayView text,
                           const bool decimalComma,
                           DataModel::LineFields& out) noexcept
{
  SS_ASSERT(out.count == 0, out.count = 0);
  SS_ASSERT(!text.isEmpty(), return);

  (void)DataModel::parseLineField(text, decimalComma, out.items[0]);
  out.count = 1;
}

/**
 * @brief Splits on one separator byte through the shared byte-match kernel. A line wider than the
 *        field cap keeps the fields that fit and drops the tail.
 */
static void splitLineOnByte(QByteArrayView text,
                            const char separator,
                            const bool decimalComma,
                            DataModel::LineFields& out) noexcept
{
  const char* data    = text.data();
  const qsizetype len = text.size();
  SS_ASSERT(data != nullptr || len == 0, return);
  SS_ASSERT(out.count == 0, out.count = 0);

  qsizetype start = 0;
  const bool ok   = DSP::simdForEachByteMatch(data, len, separator, [&](qsizetype pos) noexcept {
    if (out.count >= DataModel::kMaxLineFields)
      return false;

    (void)DataModel::parseLineField(
      QByteArrayView(data + start, pos - start), decimalComma, out.items[out.count]);
    ++out.count;
    start = pos + 1;
    return true;
  });
  if (!ok || out.count >= DataModel::kMaxLineFields)
    return;

  (void)DataModel::parseLineField(
    QByteArrayView(data + start, len - start), decimalComma, out.items[out.count]);
  ++out.count;
}

/**
 * @brief Splits on runs of spaces, then merges a token ending in ':' or '=' with the value token
 *        that follows it, so "temp: 23.5 hum: 60" yields two named fields. Raw spans are staged
 *        in the value slots and compacted in place; the write index never passes the read index.
 */
static void splitLineOnSpaces(QByteArrayView text,
                              const bool decimalComma,
                              DataModel::LineFields& out) noexcept
{
  const char* data    = text.data();
  const qsizetype len = text.size();
  SS_ASSERT(data != nullptr || len == 0, return);
  SS_ASSERT(out.count == 0, out.count = 0);

  qsizetype rawCount = 0;
  qsizetype i        = 0;
  while (i < len && rawCount < DataModel::kMaxLineFields) {
    while (i < len && data[i] == ' ')
      ++i;

    const qsizetype start = i;
    while (i < len && data[i] != ' ')
      ++i;

    if (i > start)
      out.items[rawCount++].value = QByteArrayView(data + start, i - start);
  }

  qsizetype r = 0;
  while (r < rawCount) {
    const auto token = out.items[r].value;
    const bool merge = (r + 1 < rawCount) && token.size() > 1 && isNameDelimiter(token.back())
                    && isFieldName(DataModel::trimLineAscii(token.chopped(1)));
    if (merge) {
      const auto value = out.items[r + 1].value;
      setNamedField(
        out.items[out.count], DataModel::trimLineAscii(token.chopped(1)), value, decimalComma);
      r += 2;
    } else {
      (void)DataModel::parseLineField(token, decimalComma, out.items[out.count]);
      r += 1;
    }

    ++out.count;
  }
}

/**
 * @brief Splits one line under a fixed format; a JSON format hands the line to the leaf scanner.
 */
qsizetype DataModel::tokenizeLine(QByteArrayView line,
                                  const LineFormat& format,
                                  LineFields& out) noexcept
{
  out.count       = 0;
  out.arenaUsed   = 0;
  bool wrapped    = false;
  const auto text = stripLineWrapper(line, wrapped);
  if (text.isEmpty())
    return 0;

  SS_ASSERT(format.kind == LineFormatKind::Delimited || format.kind == LineFormatKind::Json,
            return 0);
  if (format.kind == LineFormatKind::Json)
    return scanJsonLeaves(text, out) ? out.count : 0;

  if (format.separator == ' ')
    splitLineOnSpaces(text, format.decimalComma, out);
  else if (format.separator != '\0')
    splitLineOnByte(text, format.separator, format.decimalComma, out);
  else
    splitLineWhole(text, format.decimalComma, out);

  SS_ASSERT(kMaxLineFields >= out.count, out.count = kMaxLineFields);
  return out.count;
}

//--------------------------------------------------------------------------------------------------
// Detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fields that parsed as numbers.
 */
qsizetype DataModel::numericLineFieldCount(const LineFields& fields) noexcept
{
  SS_ASSERT(fields.count >= 0, return 0);
  SS_ASSERT(kMaxLineFields >= fields.count, return 0);

  qsizetype numeric = 0;
  for (qsizetype i = 0; i < fields.count; ++i)
    numeric += fields.items[i].numeric ? 1 : 0;

  return numeric;
}

/**
 * @brief True when at least one field carries a name.
 */
[[nodiscard]] static bool anyNamedField(const DataModel::LineFields& fields) noexcept
{
  SS_ASSERT(fields.count >= 0, return false);
  SS_ASSERT(DataModel::kMaxLineFields >= fields.count, return false);

  for (qsizetype i = 0; i < fields.count; ++i)
    if (!fields.items[i].name.isEmpty())
      return true;

  return false;
}

/**
 * @brief Scores the separator candidates against one line (spec 0106 R9/R10): a JSON object wins
 *        outright, else the first candidate whose fields are all numeric, else the best one with
 *        at least half numeric.
 */
bool DataModel::detectLineFormat(QByteArrayView line,
                                 LineFormat& format,
                                 LineFields& fields) noexcept
{
  bool wrapped    = false;
  const auto text = stripLineWrapper(line, wrapped);
  fields.count    = 0;
  if (text.isEmpty())
    return false;

  if (text.front() == '{' && scanJsonLeaves(text, fields)) {
    format.kind         = LineFormatKind::Json;
    format.separator    = '\0';
    format.decimalComma = false;
    format.named        = true;
    format.wrapper      = wrapped;
    return true;
  }

  LineFormat best;
  double bestScore    = 0.0;
  bool found          = false;
  bool fieldsHoldBest = false;
  for (const auto& candidate : LineFormatDetail::kSeparatorCandidates) {
    if (candidate.separator != '\0' && text.indexOf(candidate.separator) < 0)
      continue;

    LineFormat trial;
    trial.separator    = candidate.separator;
    trial.decimalComma = candidate.decimalComma;
    trial.wrapper      = wrapped;

    const qsizetype count = tokenizeLine(line, trial, fields);
    fieldsHoldBest        = false;
    if (count <= 0)
      continue;

    const double numeric = static_cast<double>(numericLineFieldCount(fields));
    const double score   = numeric / static_cast<double>(count);
    if (score >= 1.0) {
      best           = trial;
      found          = true;
      fieldsHoldBest = true;
      break;
    }

    if (score >= 0.5 && (!found || score > bestScore)) {
      best           = trial;
      bestScore      = score;
      found          = true;
      fieldsHoldBest = true;
    }
  }

  if (!found)
    return false;

  if (!fieldsHoldBest)
    (void)tokenizeLine(line, best, fields);

  SS_ASSERT(fields.count > 0, return false);
  SS_ASSERT(kMaxLineFields >= fields.count, return false);
  best.named = anyNamedField(fields);
  format     = best;
  return true;
}
