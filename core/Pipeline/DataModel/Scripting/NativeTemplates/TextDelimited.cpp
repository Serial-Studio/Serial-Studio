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

#include "DataModel/Scripting/NativeTemplates/TextDelimited.h"

#include <cstring>

#include "Core/DSPSimd.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SerialStudio.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Delimited text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Separator-based field splitter with optional quoting, trimming and empty-field removal.
 */
class DelimitedTextParser final : public INativeParser {
public:
  /**
   * @brief Stores the resolved separator and field post-processing flags.
   */
  DelimitedTextParser(const QString& separator, QChar quote, bool trim, bool skip_empty)
    : m_separator(separator)
    , m_sepUtf8(separator.toUtf8())
    , m_quote(quote)
    , m_trim(trim)
    , m_skipEmpty(skip_empty)
  {
    SS_ASSERT(!m_separator.isEmpty(), m_separator = QStringLiteral(","));
    SS_ASSERT(!m_sepUtf8.isEmpty(), m_sepUtf8 = m_separator.toUtf8());
  }

  /**
   * @brief Splits the frame on the separator and applies the configured post-processing.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_separator.isEmpty(), return {});

    QStringList row;
    row.reserve(16);

    if (m_quote.isNull())
      splitPlain(frame, row);
    else
      splitQuoted(frame, row);

    if (m_trim)
      for (auto& field : row)
        field = field.trimmed();

    if (m_skipEmpty)
      row.removeAll(QString());

    return singleFrame(std::move(row));
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

  /**
   * @brief Allocation-free byte split (UTF-8 self-syncs); quote mode falls back (-1).
   */
  [[nodiscard]] qsizetype parseSpans(QByteArrayView frame,
                                     QByteArrayView* out,
                                     qsizetype maxSpans) noexcept override
  {
    SS_ASSERT(out != nullptr, return -1);
    SS_ASSERT(maxSpans > 0, return -1);

    if (!m_quote.isNull())
      return -1;

    const char* data    = frame.data();
    const qsizetype len = frame.size();

    qsizetype count = -1;
    if (m_sepUtf8.size() == 1)
      count = splitSpansSingleByte(data, len, m_sepUtf8.at(0), out, maxSpans);
    else
      count = splitSpansMultiByte(data, len, out, maxSpans);

    if (count < 0)
      return -1;

    if (m_skipEmpty) {
      qsizetype kept = 0;
      for (qsizetype i = 0; i < count; ++i)
        if (!out[i].isEmpty())
          out[kept++] = out[i];

      count = kept;
    }

    return count;
  }

private:
  /**
   * @brief One-pass single-byte split through the shared DSP::simdForEachByteMatch kernel
   *        (x86 SSE2..SSE4.2 + aarch64 NEON, scalar elsewhere). Returns the span count or -1
   *        on overflow.
   */
  [[nodiscard]] qsizetype splitSpansSingleByte(const char* data,
                                               qsizetype len,
                                               char sep,
                                               QByteArrayView* out,
                                               qsizetype maxSpans) const noexcept
  {
    SS_ASSERT(data != nullptr || len == 0, return -1);
    SS_ASSERT(maxSpans > 0, return -1);

    qsizetype count = 0;
    qsizetype start = 0;
    const bool ok   = DSP::simdForEachByteMatch(data, len, sep, [&](qsizetype pos) noexcept {
      if (count >= maxSpans)
        return false;

      out[count++] = fieldSpan(data + start, pos - start);
      start        = pos + 1;
      return true;
    });
    if (!ok)
      return -1;

    if (count >= maxSpans)
      return -1;

    out[count++] = fieldSpan(data + start, len - start);
    return count;
  }

  /**
   * @brief Multi-byte split: memchr-anchored first-byte search + memcmp verify per candidate.
   */
  [[nodiscard]] qsizetype splitSpansMultiByte(const char* data,
                                              qsizetype len,
                                              QByteArrayView* out,
                                              qsizetype maxSpans) const noexcept
  {
    SS_ASSERT(maxSpans > 0, return -1);
    SS_ASSERT(m_sepUtf8.size() > 1, return -1);

    const char sep0        = m_sepUtf8.at(0);
    const qsizetype sepLen = m_sepUtf8.size();

    qsizetype count = 0;
    qsizetype start = 0;

    for (qsizetype pass = 0; pass <= len; ++pass) {
      qsizetype end = -1;
      for (qsizetype from = start; from + sepLen <= len;) {
        const void* c = memchr(data + from, sep0, static_cast<size_t>(len - from));
        if (!c)
          break;

        const qsizetype pos = static_cast<const char*>(c) - data;
        if (pos + sepLen > len)
          break;

        if (memcmp(data + pos, m_sepUtf8.constData(), sepLen) == 0) {
          end = pos;
          break;
        }

        from = pos + 1;
      }

      const bool last      = (end < 0);
      const qsizetype stop = last ? len : end;
      if (count >= maxSpans)
        return -1;

      out[count++] = fieldSpan(data + start, stop - start);
      if (last)
        break;

      start = stop + sepLen;
    }

    return count;
  }

  /**
   * @brief Returns the field view with optional ASCII whitespace trimming applied.
   */
  [[nodiscard]] QByteArrayView fieldSpan(const char* p, qsizetype n) const noexcept
  {
    SS_ASSERT(n >= 0, n = 0);

    if (!m_trim)
      return QByteArrayView(p, n);

    const auto isSpace = [](char c) noexcept {
      return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
    };

    qsizetype b = 0;
    qsizetype e = n;
    while (b < e && isSpace(p[b]))
      ++b;

    while (e > b && isSpace(p[e - 1]))
      --e;

    return QByteArrayView(p + b, e - b);
  }

  /**
   * @brief Plain split that keeps empty fields, identical to QString::split on the separator.
   */
  void splitPlain(const QString& frame, QStringList& row) const
  {
    SS_ASSERT_LOG(row.isEmpty());

    qsizetype start = 0;
    for (int field = 0; field < kMaxFields; ++field) {
      const qsizetype idx = frame.indexOf(m_separator, start);
      if (idx < 0)
        break;

      row.append(frame.mid(start, idx - start));
      start = idx + m_separator.length();
    }

    row.append(frame.mid(start));
  }

  /**
   * @brief Quote-aware split: separators inside quotes are literal, doubled quotes escape.
   */
  void splitQuoted(const QString& frame, QStringList& row) const
  {
    SS_ASSERT_LOG(row.isEmpty());
    SS_ASSERT_LOG(!m_quote.isNull());

    QString field;
    field.reserve(32);

    bool in_quotes      = false;
    const qsizetype len = frame.length();
    qsizetype i         = 0;
    while (i < len && row.size() < kMaxFields) {
      const QChar c = frame.at(i);

      if (in_quotes && c == m_quote && i + 1 < len && frame.at(i + 1) == m_quote) {
        field.append(m_quote);
        i += 2;
        continue;
      }

      if (c == m_quote) {
        in_quotes = !in_quotes;
        ++i;
        continue;
      }

      if (!in_quotes && matchesSeparatorAt(frame, i)) {
        row.append(field);
        field.clear();
        i += m_separator.length();
        continue;
      }

      field.append(c);
      ++i;
    }

    row.append(field);
  }

  /**
   * @brief Returns true when the full separator matches the frame at position i.
   */
  [[nodiscard]] bool matchesSeparatorAt(const QString& frame, qsizetype i) const
  {
    SS_ASSERT(i >= 0, return false);
    SS_ASSERT(i < frame.length(), return false);

    if (frame.at(i) != m_separator.at(0))
      return false;

    return QStringView(frame).mid(i, m_separator.length()) == m_separator;
  }

private:
  QString m_separator;
  QByteArray m_sepUtf8;
  QChar m_quote;
  bool m_trim;
  bool m_skipEmpty;
};

/**
 * @brief Descriptor for the delimited-text template (CSV, TSV, pipe, semicolon, custom).
 */
class DelimitedTextTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("delimited"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Delimited text"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Splits each frame into fields using a configurable separator, with "
                            "optional quoting, trimming and empty-field removal.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    NativeParamSpec separator;
    separator.key          = QStringLiteral("separator");
    separator.type         = NativeParamType::String;
    separator.label        = trNativeTemplate("Separator");
    separator.description  = trNativeTemplate("Literal text between fields, e.g. \",\", \";\" or "
                                              "\"|\". Escape sequences such as \\t (tab) are "
                                              "supported.");
    separator.defaultValue = QStringLiteral(",");

    NativeParamSpec quote;
    quote.key          = QStringLiteral("quoteChar");
    quote.type         = NativeParamType::Char;
    quote.label        = trNativeTemplate("Quote character");
    quote.description  = trNativeTemplate("Fields wrapped in this character may contain the "
                                          "separator. Leave empty to disable quoting.");
    quote.defaultValue = QStringLiteral("");

    NativeParamSpec trim;
    trim.key          = QStringLiteral("trimFields");
    trim.type         = NativeParamType::Bool;
    trim.label        = trNativeTemplate("Trim whitespace");
    trim.description  = trNativeTemplate("Removes leading and trailing whitespace from every "
                                         "field.");
    trim.defaultValue = false;

    NativeParamSpec skip;
    skip.key          = QStringLiteral("skipEmpty");
    skip.type         = NativeParamType::Bool;
    skip.label        = trNativeTemplate("Skip empty fields");
    skip.description  = trNativeTemplate("Drops empty fields instead of emitting empty channels.");
    skip.defaultValue = false;

    return {separator, quote, trim, skip};
  }

  /**
   * @brief Resolves the separator text and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString separator = SerialStudio::resolveEscapeSequences(
      DataModel::nativeParamString(params, QStringLiteral("separator"), QStringLiteral(",")));

    if (separator.isEmpty()) {
      error = trNativeTemplate("The separator must not be empty.");
      return nullptr;
    }

    const QChar quote = DataModel::nativeParamChar(params, QStringLiteral("quoteChar"), QChar());
    if (!quote.isNull() && separator.contains(quote)) {
      error = trNativeTemplate("The quote character must differ from the field separator.");
      return nullptr;
    }

    const bool trim = DataModel::nativeParamBool(params, QStringLiteral("trimFields"), false);
    const bool skip = DataModel::nativeParamBool(params, QStringLiteral("skipEmpty"), false);
    return std::make_unique<DelimitedTextParser>(separator, quote, trim, skip);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide delimited-text template descriptor.
 */
const DataModel::INativeTemplate& DataModel::delimitedTextTemplate()
{
  static const DelimitedTextTemplate s_delimited;
  return s_delimited;
}
