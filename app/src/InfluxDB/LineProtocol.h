/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <charconv>
#include <cmath>
#include <cstdio>
#include <QByteArray>
#include <QByteArrayView>
#include <QString>
#include <QStringView>
#include <QtGlobal>

#if defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) \
  && __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 130300
#  define SS_APPLE_NO_FLOAT_TO_CHARS 1
#  include <xlocale.h>
#endif

namespace InfluxDB {

/**
 * @brief InfluxDB 2.x line-protocol writer. Header-only and Qt-Core-only so the ctest tier can
 *        prove the escaping and the batch boundary without a network stack or a licensed build.
 *
 * One line is <tt>measurement[,tag=value...] field=value[,...] timestamp</tt>, where the escape
 * set differs per position (v2 spec): a measurement escapes comma and space, a tag key, a tag
 * value and a field key escape comma, equals and space, a string field value escapes the double
 * quote, and the backslash escapes itself everywhere. Timestamps are nanoseconds, which is what
 * the sink asks the server for with <tt>precision=ns</tt>.
 */

// A batch is considered full at this size; well under InfluxDB's documented per-write ceiling
inline constexpr qsizetype kDefaultBatchBytes = 512 * 1024;

// Escape sets per position; the backslash escapes itself everywhere (E7)
inline constexpr QByteArrayView kMeasurementSpecials = ", \\";
inline constexpr QByteArrayView kKeySpecials         = ",= \\";
inline constexpr QByteArrayView kStringSpecials      = "\"\\";

/**
 * @brief Returns whether @p byte is one of @p specials, without the terminator match a plain
 *        strchr() would report for an embedded NUL.
 */
[[nodiscard]] inline bool isLineSpecial(char byte, QByteArrayView specials) noexcept
{
  for (const char candidate : specials)
    if (candidate == byte)
      return true;

  return false;
}

/**
 * @brief Appends @p text as UTF-8 with every byte of @p specials backslash-escaped. A line break
 *        is not escapable in line protocol, so it degrades to a space (itself escaped where a
 *        literal space would be) and a NUL is dropped. ASCII runs escape in place; only a
 *        non-ASCII run pays a UTF-8 conversion, whose bytes never collide with a line special.
 */
inline void appendEscaped(QByteArray& out, QStringView text, QByteArrayView specials)
{
  const qsizetype n = text.size();
  out.reserve(out.size() + n);

  qsizetype i = 0;
  while (i < n) {
    const char16_t unit = text[i].unicode();
    if (unit >= 0x80) {
      const qsizetype start = i;
      while (i < n && text[i].unicode() >= 0x80)
        ++i;

      out.append(QStringView(text.data() + start, i - start).toUtf8());
      continue;
    }

    ++i;
    if (unit == u'\0')
      continue;

    char byte = static_cast<char>(unit);
    if (byte == '\n' || byte == '\r' || byte == '\t')
      byte = ' ';

    if (isLineSpecial(byte, specials))
      out.append('\\');

    out.append(byte);
  }
}

/**
 * @brief Appends @p value in the shortest decimal form that reads back bit-identically, straight
 *        into @p out with no intermediate allocation. Apple ships float std::to_chars only from
 *        macOS 13.3, so older targets emit 17 significant digits, which always round-trips.
 */
inline void appendDouble(QByteArray& out, double value)
{
  char buf[32];
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  const int len = snprintf_l(buf, sizeof(buf), nullptr, "%.17g", value);
  if (len > 0 && static_cast<std::size_t>(len) < sizeof(buf))
    out.append(buf, static_cast<qsizetype>(len));
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value);
  out.append(buf, static_cast<qsizetype>(res.ptr - buf));
#endif
}

/**
 * @brief Renders @p value with the shortest decimal form that reads back bit-identically, so a
 *        stored point is the value the pipeline produced and not a rounded twin.
 */
[[nodiscard]] inline QByteArray formatDouble(double value)
{
  QByteArray text;
  appendDouble(text, value);
  return text;
}

/**
 * @brief Accumulates rendered points until it reports itself full, at which point the owner
 *        POSTs @c payload() and clears it. A point with no field is invalid line protocol, so
 *        @c endPoint() rolls it back rather than emitting a line the server would reject.
 */
class LineBatch {
public:
  /**
   * @brief Builds an empty batch that reports full at @p flushBytes.
   */
  explicit LineBatch(qsizetype flushBytes = kDefaultBatchBytes)
    : m_flushBytes(flushBytes > 0 ? flushBytes : kDefaultBatchBytes)
    , m_pointStart(0)
    , m_fields(0)
    , m_points(0)
    , m_skippedFields(0)
    , m_open(false)
  {}

  /**
   * @brief Opens a point on @p measurement; an unfinished previous point is discarded.
   */
  void beginPoint(const QString& measurement)
  {
    if (m_open)
      m_payload.truncate(m_pointStart);

    m_pointStart = m_payload.size();
    m_fields     = 0;
    m_open       = true;
    appendEscaped(m_payload, measurement, kMeasurementSpecials);
  }

  /**
   * @brief Opens a point on a pre-escaped @c measurement,source=<id> head, so a caller that holds
   *        the head constant for a block writes it once per bind instead of re-escaping per sample.
   */
  void beginPointRaw(QByteArrayView head)
  {
    if (m_open)
      m_payload.truncate(m_pointStart);

    m_pointStart = m_payload.size();
    m_fields     = 0;
    m_open       = true;
    m_payload.append(head);
  }

  /**
   * @brief Appends one tag to the open point. Tags precede fields on the line, so a tag added
   *        after the first field would corrupt it and is refused.
   */
  void addTag(const QString& key, const QString& value)
  {
    if (!m_open || m_fields > 0 || key.isEmpty() || value.isEmpty())
      return;

    m_payload.append(',');
    appendEscaped(m_payload, key, kKeySpecials);
    m_payload.append('=');
    appendEscaped(m_payload, value, kKeySpecials);
  }

  /**
   * @brief Appends a float field. NaN and the infinities have no line-protocol spelling, so they
   *        are skipped and counted instead of poisoning the whole batch with a rejected line.
   */
  [[nodiscard]] bool addFieldFloat(const QString& key, double value)
  {
    if (!std::isfinite(value)) {
      ++m_skippedFields;
      return false;
    }

    if (!openField(key))
      return false;

    appendDouble(m_payload, value);
    return true;
  }

  /**
   * @brief Appends a float field on a pre-escaped key; the non-finite skip is unchanged.
   */
  [[nodiscard]] bool addFieldFloatRaw(QByteArrayView key, double value)
  {
    if (!std::isfinite(value)) {
      ++m_skippedFields;
      return false;
    }

    if (!openFieldRaw(key))
      return false;

    appendDouble(m_payload, value);
    return true;
  }

  /**
   * @brief Appends a signed-integer field, which line protocol marks with a trailing @c i.
   */
  [[nodiscard]] bool addFieldInteger(const QString& key, qint64 value)
  {
    if (!openField(key))
      return false;

    m_payload.append(QByteArray::number(value));
    m_payload.append('i');
    return true;
  }

  /**
   * @brief Appends a boolean field.
   */
  [[nodiscard]] bool addFieldBoolean(const QString& key, bool value)
  {
    if (!openField(key))
      return false;

    m_payload.append(value ? 't' : 'f');
    return true;
  }

  /**
   * @brief Appends a quoted string field.
   */
  [[nodiscard]] bool addFieldString(const QString& key, const QString& value)
  {
    if (!openField(key))
      return false;

    m_payload.append('"');
    appendEscaped(m_payload, value, kStringSpecials);
    m_payload.append('"');
    return true;
  }

  /**
   * @brief Appends a quoted string field on a pre-escaped key.
   */
  [[nodiscard]] bool addFieldStringRaw(QByteArrayView key, const QString& value)
  {
    if (!openFieldRaw(key))
      return false;

    m_payload.append('"');
    appendEscaped(m_payload, value, kStringSpecials);
    m_payload.append('"');
    return true;
  }

  /**
   * @brief Closes the open point at @p timestampNs, or rolls it back when it carries no field.
   */
  [[nodiscard]] bool endPoint(qint64 timestampNs)
  {
    if (!m_open)
      return false;

    m_open = false;
    if (m_fields == 0) {
      m_payload.truncate(m_pointStart);
      return false;
    }

    m_payload.append(' ');
    m_payload.append(QByteArray::number(timestampNs));
    m_payload.append('\n');
    ++m_points;
    return true;
  }

  /**
   * @brief Returns whether the accumulated payload reached the flush boundary.
   */
  [[nodiscard]] bool full() const noexcept { return m_payload.size() >= m_flushBytes; }

  /**
   * @brief Returns whether no complete point has been accumulated.
   */
  [[nodiscard]] bool isEmpty() const noexcept { return m_points == 0; }

  /**
   * @brief Returns how many complete points the payload carries.
   */
  [[nodiscard]] qsizetype points() const noexcept { return m_points; }

  /**
   * @brief Returns how many field values were skipped as unrepresentable since construction.
   */
  [[nodiscard]] qsizetype skippedFields() const noexcept { return m_skippedFields; }

  /**
   * @brief Returns the rendered batch body, ready to POST.
   */
  [[nodiscard]] const QByteArray& payload() const noexcept { return m_payload; }

  /**
   * @brief Drops the accumulated payload and its point count; the skip tally is cumulative and
   *        survives, because the owner reports it as a session statistic.
   */
  void clear() noexcept
  {
    m_payload.clear();
    m_pointStart = 0;
    m_fields     = 0;
    m_points     = 0;
    m_open       = false;
  }

  /**
   * @brief Reserves the accumulation buffer once, so a batch fills without the 0 -> flush-size
   *        regrowth a freshly cleared QByteArray would pay every cycle.
   */
  void reserve(qsizetype bytes) { m_payload.reserve(bytes); }

  /**
   * @brief Swaps the filled payload into @p out and adopts @p out's buffer as the next (emptied)
   *        accumulator, so the owner double-buffers: the POSTed bytes are never touched in place
   *        and neither array's capacity is released between cycles.
   */
  void takePayload(QByteArray& out) noexcept
  {
    m_payload.swap(out);
    m_payload.resize(0);
    m_pointStart = 0;
    m_fields     = 0;
    m_points     = 0;
    m_open       = false;
  }

private:
  /**
   * @brief Writes the separator and key of the next field, reporting whether one may follow.
   */
  [[nodiscard]] bool openField(const QString& key)
  {
    if (!m_open || key.isEmpty())
      return false;

    m_payload.append(m_fields == 0 ? ' ' : ',');
    appendEscaped(m_payload, key, kKeySpecials);
    m_payload.append('=');
    ++m_fields;
    return true;
  }

  /**
   * @brief Writes the separator and a pre-escaped key of the next field.
   */
  [[nodiscard]] bool openFieldRaw(QByteArrayView key)
  {
    if (!m_open || key.isEmpty())
      return false;

    m_payload.append(m_fields == 0 ? ' ' : ',');
    m_payload.append(key);
    m_payload.append('=');
    ++m_fields;
    return true;
  }

private:
  QByteArray m_payload;
  qsizetype m_flushBytes;
  qsizetype m_pointStart;
  qsizetype m_fields;
  qsizetype m_points;
  qsizetype m_skippedFields;
  bool m_open;
};

}  // namespace InfluxDB
