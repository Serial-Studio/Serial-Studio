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

#include "DataModel/Scripting/ScriptCells.h"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif
#include <QStringConverter>

#include "Core/SSAssert.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr qsizetype kInitialCellReserve = 64;
static constexpr qsizetype kInitialByteReserve = 1024;
static constexpr qsizetype kShrinkAboveCells   = 1 << 18;
static constexpr qsizetype kShrinkAboveBytes   = 16 << 20;
static constexpr int kJsShortestDigitsMax      = 17;
static constexpr int kJsFixedUpperExponent     = 21;
static constexpr int kJsFixedLowerExponent     = -6;

//--------------------------------------------------------------------------------------------------
// ScriptCellRows
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts empty with a small capacity; the first wide frame grows it once.
 */
ScriptCellRows::ScriptCellRows()
{
  reserve(kInitialCellReserve, kInitialByteReserve);
}

/**
 * @brief Number of rows the last result produced.
 */
qsizetype ScriptCellRows::rowCount() const noexcept
{
  return static_cast<qsizetype>(m_rowStarts.size());
}

/**
 * @brief Number of cells across every row.
 */
qsizetype ScriptCellRows::cellCount() const noexcept
{
  return static_cast<qsizetype>(m_cells.size());
}

/**
 * @brief Cells of @p row and how many there are; null with count 0 for an out-of-range row.
 */
const ScriptCell* ScriptCellRows::rowCells(qsizetype row, qsizetype& count) const noexcept
{
  count = 0;
  if (row < 0 || row >= rowCount())
    return nullptr;

  const auto begin = static_cast<std::size_t>(m_rowStarts[static_cast<std::size_t>(row)]);
  const auto end =
    row + 1 < rowCount() ? static_cast<std::size_t>(m_rowStarts[row + 1]) : m_cells.size();
  SS_ASSERT_LOG(begin <= end);
  count = static_cast<qsizetype>(end - begin);
  return count > 0 ? &m_cells[begin] : nullptr;
}

/**
 * @brief The cell's text bytes, valid until the next clear().
 */
QByteArrayView ScriptCellRows::view(const ScriptCell& cell) const noexcept
{
  SS_ASSERT_LOG(cell.offset + cell.length <= m_scratch.size());
  return QByteArrayView(m_scratch.constData() + cell.offset, cell.length);
}

/**
 * @brief Forgets every row and cell but keeps the storage (resize(0), never clear(), on the
 *        scratch: QByteArray::clear() would release the buffer). One pathological result is not
 *        allowed to pin its storage for the session: capacity past the high-water marks is
 *        released back to the initial reserve.
 */
void ScriptCellRows::clear() noexcept
{
  m_cells.clear();
  m_rowStarts.clear();
  m_scratch.resize(0);

  const bool cells_wide = static_cast<qsizetype>(m_cells.capacity()) > kShrinkAboveCells;
  const bool bytes_wide = m_scratch.capacity() > kShrinkAboveBytes;
  if (!cells_wide && !bytes_wide) [[likely]]
    return;

  std::vector<ScriptCell>().swap(m_cells);
  std::vector<qsizetype>().swap(m_rowStarts);
  m_scratch = QByteArray();
  reserve(kInitialCellReserve, kInitialByteReserve);
}

/**
 * @brief Grows the three containers up front so a whole result appends without reallocating.
 */
void ScriptCellRows::reserve(qsizetype cells, qsizetype bytes)
{
  SS_ASSERT(cells >= 0 && bytes >= 0, return);

  m_cells.reserve(static_cast<std::size_t>(cells));
  m_rowStarts.reserve(static_cast<std::size_t>(cells));
  m_scratch.reserve(bytes);
}

/**
 * @brief Opens a new row; the cells appended next belong to it.
 */
void ScriptCellRows::beginRow()
{
  m_rowStarts.push_back(cellCount());
}

/**
 * @brief Appends a text cell, copying its bytes into the scratch.
 */
void ScriptCellRows::appendText(const char* bytes, qsizetype length)
{
  SS_ASSERT_LOG(bytes != nullptr || length == 0);
  SS_ASSERT_LOG(!m_rowStarts.empty());

  const qsizetype offset = m_scratch.size();
  if (length > 0)
    m_scratch.append(bytes, length);

  m_cells.push_back(ScriptCell{offset, length, 0.0, CellKind::Text});
}

/**
 * @brief Appends a UTF-16 text cell, encoding it straight into the scratch tail (no QByteArray
 *        temporary): the tail is grown to the encoder's worst case, then trimmed to what it wrote.
 */
void ScriptCellRows::appendUtf16(QStringView text)
{
  SS_ASSERT_LOG(!m_rowStarts.empty());

  QStringEncoder encoder(QStringConverter::Utf8);
  const qsizetype offset = m_scratch.size();
  const qsizetype room   = encoder.requiredSpace(text.size());
  SS_ASSERT_LOG(room >= 0);
  m_scratch.resize(offset + room);

  char* const begin     = m_scratch.data() + offset;
  const char* const end = encoder.appendToBuffer(begin, text);
  const auto length     = static_cast<qsizetype>(end - begin);
  m_scratch.resize(offset + length);
  m_cells.push_back(ScriptCell{offset, length, 0.0, CellKind::Text});
}

/**
 * @brief Appends a number cell together with the text its engine would have shown for it.
 */
void ScriptCellRows::appendNumber(double value, const char* text, qsizetype length)
{
  SS_ASSERT_LOG(text != nullptr && length > 0);
  SS_ASSERT_LOG(!m_rowStarts.empty());

  const qsizetype offset = m_scratch.size();
  m_scratch.append(text, length);
  m_cells.push_back(ScriptCell{offset, length, value, CellKind::Number});
}

//--------------------------------------------------------------------------------------------------
// Number formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies a literal into @p out; 0 when it does not fit.
 */
[[nodiscard]] static qsizetype putLiteral(const char* literal,
                                          char* out,
                                          qsizetype capacity) noexcept
{
  const auto n = static_cast<qsizetype>(std::strlen(literal));
  if (n > capacity)
    return 0;

  std::memcpy(out, literal, static_cast<std::size_t>(n));
  return n;
}

/**
 * @brief Lua's list-path formatting: %lld for integers, %.15g (C locale) otherwise, with the
 *        non-finite spellings QString::number used ("nan" unsigned, "inf", "-inf").
 */
qsizetype formatLuaNumber(double value, bool isInteger, char* out, qsizetype capacity) noexcept
{
  SS_ASSERT(out != nullptr && capacity > 0, return 0);

  if (std::isnan(value))
    return putLiteral("nan", out, capacity);

  if (std::isinf(value))
    return putLiteral(value < 0.0 ? "-inf" : "inf", out, capacity);

  if (isInteger) {
    const int len =
      std::snprintf(out, static_cast<std::size_t>(capacity), "%lld", static_cast<long long>(value));
    return (len > 0 && len < capacity) ? len : 0;
  }

#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const int len = snprintf_l(out, static_cast<std::size_t>(capacity), nullptr, "%.15g", value);
  return (len > 0 && len < capacity) ? len : 0;
#else
  const auto res = std::to_chars(out, out + capacity, value, std::chars_format::general, 15);
  return res.ec == std::errc() ? static_cast<qsizetype>(res.ptr - out) : 0;
#endif
}

/**
 * @brief Shortest round-trip digits of a positive finite double as "d[.ddd]e[+-]XX" bytes.
 */
[[nodiscard]] static qsizetype shortestScientific(double magnitude,
                                                  char* out,
                                                  qsizetype capacity) noexcept
{
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  for (int precision = 0; precision <= kJsShortestDigitsMax - 1; ++precision) {
    const int len =
      snprintf_l(out, static_cast<std::size_t>(capacity), nullptr, "%.*e", precision, magnitude);
    if (len <= 0 || len >= capacity)
      return 0;

    if (strtod_l(out, nullptr, nullptr) == magnitude)
      return len;
  }

  return 0;
#else
  const auto res = std::to_chars(out, out + capacity, magnitude, std::chars_format::scientific);
  return res.ec == std::errc() ? static_cast<qsizetype>(res.ptr - out) : 0;
#endif
}

/**
 * @brief Splits "d.ddde+XX" into its digit string and decimal exponent (the ECMA "n" is exp + 1).
 *        The exponent is read within [0, length): the buffer is not NUL-terminated.
 */
[[nodiscard]] static bool splitScientific(
  const char* text, qsizetype length, char* digits, int& digitCount, int& exponent) noexcept
{
  SS_ASSERT(text != nullptr && digits != nullptr, return false);
  SS_ASSERT(length > 0 && length <= 40, return false);

  digitCount  = 0;
  qsizetype i = 0;
  for (; i < length && text[i] != 'e' && text[i] != 'E'; ++i) {
    if (text[i] == '.')
      continue;

    if (digitCount >= kJsShortestDigitsMax)
      return false;

    digits[digitCount++] = text[i];
  }

  if (i >= length || digitCount == 0)
    return false;

  qsizetype j = i + 1;
  int sign    = 1;
  if (j < length && (text[j] == '+' || text[j] == '-')) {
    sign = text[j] == '-' ? -1 : 1;
    ++j;
  }

  const qsizetype first_digit = j;
  int magnitude               = 0;
  for (; j < length && text[j] >= '0' && text[j] <= '9'; ++j)
    magnitude = magnitude * 10 + (text[j] - '0');

  exponent = sign * magnitude;
  return j > first_digit;
}

/**
 * @brief Bounded byte sink for the layout steps: counts past the capacity so overflow is one check.
 */
struct LayoutSink {
  char* out;
  qsizetype capacity;
  qsizetype pos;

  /**
   * @brief Writes one byte when it fits, always advancing the length.
   */
  void put(char c) noexcept
  {
    if (pos < capacity)
      out[pos] = c;

    ++pos;
  }

  /**
   * @brief Writes s[from, to).
   */
  void putRange(const char* s, int from, int to) noexcept
  {
    for (int i = from; i < to; ++i)
      put(s[i]);
  }

  /**
   * @brief Writes @p count '0' bytes.
   */
  void putZeros(int count) noexcept
  {
    for (int i = 0; i < count; ++i)
      put('0');
  }
};

/**
 * @brief ECMA-262 step 10 (exponent form): "d[.ddd]e[+-]N" with N = n - 1 and no zero padding.
 */
static void layoutJsExponent(LayoutSink& sink, const char* digits, int k, int n) noexcept
{
  sink.put(digits[0]);
  if (k > 1) {
    sink.put('.');
    sink.putRange(digits, 1, k);
  }

  sink.put('e');
  const int e = n - 1;
  sink.put(e < 0 ? '-' : '+');

  char expText[16];
  const int expLen = std::snprintf(expText, sizeof(expText), "%d", e < 0 ? -e : e);
  sink.putRange(expText, 0, expLen);
}

/**
 * @brief Lays out @p digits per ECMA-262 Number::toString for a positive value: fixed notation for
 *        1e-6 <= x < 1e21, exponent notation outside, exactly as QJSValue::toString() prints.
 */
[[nodiscard]] static qsizetype layoutJsNumber(
  const char* digits, int k, int n, char* out, qsizetype capacity) noexcept
{
  SS_ASSERT(digits != nullptr && out != nullptr, return 0);
  SS_ASSERT(k > 0 && k <= kJsShortestDigitsMax, return 0);

  LayoutSink sink{out, capacity, 0};

  if (k <= n && n <= kJsFixedUpperExponent) {
    sink.putRange(digits, 0, k);
    sink.putZeros(n - k);
  } else if (0 < n && n <= kJsFixedUpperExponent) {
    sink.putRange(digits, 0, n);
    sink.put('.');
    sink.putRange(digits, n, k);
  } else if (kJsFixedLowerExponent < n && n <= 0) {
    sink.put('0');
    sink.put('.');
    sink.putZeros(-n);
    sink.putRange(digits, 0, k);
  } else {
    layoutJsExponent(sink, digits, k, n);
  }

  return sink.pos <= capacity ? sink.pos : 0;
}

/**
 * @brief ECMAScript Number::toString: the special values first, then shortest digits laid out.
 */
qsizetype formatJsNumber(double value, char* out, qsizetype capacity) noexcept
{
  SS_ASSERT(out != nullptr && capacity > 0, return 0);

  if (std::isnan(value))
    return putLiteral("NaN", out, capacity);

  if (std::isinf(value))
    return putLiteral(value < 0.0 ? "-Infinity" : "Infinity", out, capacity);

  if (value == 0.0)
    return putLiteral("0", out, capacity);

  const bool negative    = value < 0.0;
  const double magnitude = negative ? -value : value;

  char scientific[40];
  const qsizetype sciLen = shortestScientific(magnitude, scientific, sizeof(scientific));
  if (sciLen <= 0)
    return 0;

  char digits[kJsShortestDigitsMax + 1];
  int digitCount = 0;
  int exponent   = 0;
  if (!splitScientific(scientific, sciLen, digits, digitCount, exponent))
    return 0;

  qsizetype pos = 0;
  if (negative)
    out[pos++] = '-';

  const qsizetype body =
    layoutJsNumber(digits, digitCount, exponent + 1, out + pos, capacity - pos);
  return body > 0 ? pos + body : 0;
}

}  // namespace DataModel
