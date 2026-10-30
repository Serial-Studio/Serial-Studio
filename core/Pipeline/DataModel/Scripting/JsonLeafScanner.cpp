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

#include "DataModel/Scripting/JsonLeafScanner.h"

#include <array>
#include <cstring>
#include <system_error>

#include "Core/SSAssert.h"
#include "Core/ThirdParty/fast_float.h"

//--------------------------------------------------------------------------------------------------
// Scanner state
//--------------------------------------------------------------------------------------------------

namespace JsonScanDetail {

/**
 * @brief The scan cursor: the unread bytes, the fields being filled, and the path of the value
 *        under the cursor as a working buffer that leaves are copied out of.
 */
struct Cursor {
  const char* p;
  const char* end;
  DataModel::LineFields* out;
  std::array<char, DataModel::kMaxJsonPathBytes> path;
  qsizetype pathLen;
  int depth;
  bool ok;
};

}  // namespace JsonScanDetail

using JsonScanDetail::Cursor;

[[nodiscard]] static bool scanValue(Cursor& cur) noexcept;

/**
 * @brief JSON insignificant whitespace.
 */
[[nodiscard]] static bool isJsonSpace(const char c) noexcept
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/**
 * @brief ASCII decimal digit.
 */
[[nodiscard]] static bool isJsonDigit(const char c) noexcept
{
  return c >= '0' && c <= '9';
}

/**
 * @brief Advances past insignificant whitespace.
 */
static void skipSpace(Cursor& cur) noexcept
{
  SS_ASSERT(cur.p != nullptr, return);
  SS_ASSERT(cur.end >= cur.p, return);

  for (; cur.p < cur.end && isJsonSpace(*cur.p); ++cur.p)
    continue;
}

/**
 * @brief Consumes @p c when it is the next byte.
 */
[[nodiscard]] static bool accept(Cursor& cur, const char c) noexcept
{
  SS_ASSERT(cur.p != nullptr, return false);
  if (cur.p >= cur.end || *cur.p != c)
    return false;

  ++cur.p;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Path buffer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends @p bytes to the working path; an overflow fails the whole scan.
 */
[[nodiscard]] static bool pushPath(Cursor& cur, QByteArrayView bytes) noexcept
{
  SS_ASSERT(cur.pathLen >= 0, return false);
  SS_ASSERT(DataModel::kMaxJsonPathBytes >= cur.pathLen, return false);
  if (cur.pathLen + bytes.size() > DataModel::kMaxJsonPathBytes) {
    cur.ok = false;
    return false;
  }

  std::memcpy(cur.path.data() + cur.pathLen, bytes.data(), static_cast<size_t>(bytes.size()));
  cur.pathLen += bytes.size();
  return true;
}

/**
 * @brief Appends "[index]" to the working path.
 */
[[nodiscard]] static bool pushIndex(Cursor& cur, const qsizetype index) noexcept
{
  SS_ASSERT(index >= 0, return false);
  SS_ASSERT(DataModel::kMaxLineFields >= index, return false);

  std::array<char, 16> digits{};
  qsizetype n       = 0;
  qsizetype value   = index;
  digits[n++]       = '[';
  const qsizetype s = n;
  do {
    digits[n++]  = static_cast<char>('0' + (value % 10));
    value       /= 10;
  } while (value > 0 && n < 14);

  for (qsizetype i = s, j = n - 1; i < j; ++i, --j) {
    const char t = digits[i];
    digits[i]    = digits[j];
    digits[j]    = t;
  }

  digits[n++] = ']';
  return pushPath(cur, QByteArrayView(digits.data(), n));
}

/**
 * @brief Copies the working path into the arena and appends a numeric field for it.
 */
static void emitLeaf(Cursor& cur, QByteArrayView valueText, const double number) noexcept
{
  SS_ASSERT(cur.out != nullptr, return);
  SS_ASSERT(DataModel::kMaxJsonPathBytes >= cur.pathLen, return);
  auto& out = *cur.out;
  if (out.count >= DataModel::kMaxLineFields
      || out.arenaUsed + cur.pathLen > DataModel::kLinePathArenaBytes) {
    cur.ok = false;
    return;
  }

  char* dst = out.pathArena.data() + out.arenaUsed;
  std::memcpy(dst, cur.path.data(), static_cast<size_t>(cur.pathLen));
  out.arenaUsed += cur.pathLen;

  auto& field   = out.items[out.count++];
  field         = DataModel::LineField();
  field.token   = valueText;
  field.name    = QByteArrayView(dst, cur.pathLen);
  field.value   = valueText;
  field.number  = number;
  field.numeric = true;
}

//--------------------------------------------------------------------------------------------------
// Grammar
//--------------------------------------------------------------------------------------------------

/**
 * @brief Skips a string token (escapes honored) and returns its raw bytes without the quotes.
 */
[[nodiscard]] static bool scanString(Cursor& cur, QByteArrayView& raw) noexcept
{
  SS_ASSERT(cur.p != nullptr, return false);
  SS_ASSERT(cur.end >= cur.p, return false);
  if (!accept(cur, '"'))
    return false;

  const char* start = cur.p;
  for (; cur.p < cur.end && *cur.p != '"'; ++cur.p) {
    if (*cur.p != '\\')
      continue;

    if (cur.end - cur.p < 2)
      return false;

    ++cur.p;
  }

  if (cur.p >= cur.end)
    return false;

  raw = QByteArrayView(start, cur.p - start);
  ++cur.p;
  return true;
}

/**
 * @brief A number token becomes a leaf; fast_float decides where it ends.
 */
[[nodiscard]] static bool scanNumber(Cursor& cur) noexcept
{
  SS_ASSERT(cur.p != nullptr, return false);
  SS_ASSERT(cur.end > cur.p, return false);

  double number = 0.0;
  const auto result =
    fast_float::from_chars(cur.p, cur.end, number, fast_float::chars_format::general);
  if (result.ec != std::errc() || result.ptr == cur.p)
    return false;

  const QByteArrayView text(cur.p, result.ptr - cur.p);
  cur.p = result.ptr;
  emitLeaf(cur, text, number);
  return cur.ok;
}

/**
 * @brief Consumes one of the bare literals; none of them yields a field.
 */
[[nodiscard]] static bool scanLiteral(Cursor& cur, const char* word) noexcept
{
  SS_ASSERT(word != nullptr, return false);
  const auto n = static_cast<qsizetype>(std::strlen(word));
  SS_ASSERT(n > 0, return false);
  if (cur.end - cur.p < n || std::memcmp(cur.p, word, static_cast<size_t>(n)) != 0)
    return false;

  cur.p += n;
  return true;
}

/**
 * @brief An object: each member pushes its key onto the path (dotted below the root), scans its
 *        value, then restores the path. Bounded by kMaxJsonDepth and the member cap.
 */
[[nodiscard]] static bool scanObject(Cursor& cur) noexcept
{
  SS_ASSERT(cur.depth >= 0, return false);
  SS_ASSERT(cur.end >= cur.p, return false);
  if (!accept(cur, '{') || ++cur.depth > DataModel::kMaxJsonDepth)
    return false;

  skipSpace(cur);
  if (accept(cur, '}')) {
    --cur.depth;
    return true;
  }

  const qsizetype base = cur.pathLen;
  for (qsizetype member = 0; member <= DataModel::kMaxLineFields; ++member) {
    skipSpace(cur);
    QByteArrayView key;
    if (!scanString(cur, key) || key.indexOf(',') >= 0)
      return false;

    skipSpace(cur);
    if (!accept(cur, ':'))
      return false;

    if ((base > 0 && !pushPath(cur, QByteArrayView(".", 1))) || !pushPath(cur, key))
      return false;

    skipSpace(cur);
    if (!scanValue(cur))
      return false;

    cur.pathLen = base;
    skipSpace(cur);
    if (accept(cur, '}')) {
      --cur.depth;
      return true;
    }

    if (!accept(cur, ','))
      return false;
  }

  return false;
}

/**
 * @brief An array: each element pushes "[index]" onto the path, scans, then restores the path.
 */
[[nodiscard]] static bool scanArray(Cursor& cur) noexcept
{
  SS_ASSERT(cur.depth >= 0, return false);
  SS_ASSERT(cur.end >= cur.p, return false);
  if (!accept(cur, '[') || ++cur.depth > DataModel::kMaxJsonDepth)
    return false;

  skipSpace(cur);
  if (accept(cur, ']')) {
    --cur.depth;
    return true;
  }

  const qsizetype base = cur.pathLen;
  for (qsizetype index = 0; index <= DataModel::kMaxLineFields; ++index) {
    if (!pushIndex(cur, index))
      return false;

    skipSpace(cur);
    if (!scanValue(cur))
      return false;

    cur.pathLen = base;
    skipSpace(cur);
    if (accept(cur, ']')) {
      --cur.depth;
      return true;
    }

    if (!accept(cur, ','))
      return false;
  }

  return false;
}

/**
 * @brief Dispatches on the first byte of a value; only numbers produce fields.
 */
[[nodiscard]] static bool scanValue(Cursor& cur) noexcept
{
  SS_ASSERT(cur.p != nullptr, return false);
  if (cur.p >= cur.end)
    return false;

  const char c = *cur.p;
  if (c == '{')
    return scanObject(cur);

  if (c == '[')
    return scanArray(cur);

  if (c == '"') {
    QByteArrayView ignored;
    return scanString(cur, ignored);
  }

  if (c == '-' || isJsonDigit(c))
    return scanNumber(cur);

  return scanLiteral(cur, "true") || scanLiteral(cur, "false") || scanLiteral(cur, "null");
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scans one line holding a JSON object into numeric leaves in text order; any failure
 *        leaves @p out empty so a caller never sees a partial line.
 */
bool DataModel::scanJsonLeaves(QByteArrayView text, LineFields& out) noexcept
{
  out.count     = 0;
  out.arenaUsed = 0;
  SS_ASSERT(text.data() != nullptr || text.isEmpty(), return false);
  if (text.isEmpty())
    return false;

  Cursor cur{};
  cur.p       = text.data();
  cur.end     = text.data() + text.size();
  cur.out     = &out;
  cur.pathLen = 0;
  cur.depth   = 0;
  cur.ok      = true;

  skipSpace(cur);
  const bool parsed = cur.end > cur.p && *cur.p == '{' && scanObject(cur);
  if (parsed)
    skipSpace(cur);

  SS_ASSERT(kMaxLineFields >= out.count, out.count = kMaxLineFields);
  if (!parsed || !cur.ok || cur.p != cur.end || out.count == 0) {
    out.count     = 0;
    out.arenaUsed = 0;
    return false;
  }

  return true;
}
