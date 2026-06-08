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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <bit>
#include <cstring>
#include <QCoreApplication>
#include <QHash>
#include <QJsonDocument>
#include <QRegularExpression>

#if defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64)
#  define SS_TEXT_SPLIT_SSE2 1
#  include <emmintrin.h>
#endif

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "SerialStudio.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;

static constexpr int kMaxFields = 10000;

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
[[nodiscard]] static QString trNative(const char* text)
{
  return QCoreApplication::translate("NativeTemplates", text);
}

/**
 * @brief Wraps a single channel row as the one-frame multi-frame result.
 */
[[nodiscard]] static QList<QStringList> singleFrame(QStringList&& row)
{
  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

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
    Q_ASSERT(!m_separator.isEmpty());
    Q_ASSERT(!m_sepUtf8.isEmpty());
  }

  /**
   * @brief Splits the frame on the separator and applies the configured post-processing.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_separator.isEmpty());

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
    Q_ASSERT(out != nullptr);
    Q_ASSERT(maxSpans > 0);

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
   * @brief One-pass single-byte split: SIMD compare + bit scan on x86-64 (SSE2 is inside the
   *        x86-64-v2 baseline), scalar elsewhere. Returns the span count or -1 on overflow.
   */
  [[nodiscard]] qsizetype splitSpansSingleByte(const char* data,
                                               qsizetype len,
                                               char sep,
                                               QByteArrayView* out,
                                               qsizetype maxSpans) const noexcept
  {
    Q_ASSERT(data != nullptr || len == 0);
    Q_ASSERT(maxSpans > 0);

    qsizetype count = 0;
    qsizetype start = 0;
    qsizetype i     = 0;

#ifdef SS_TEXT_SPLIT_SSE2
    const __m128i needle = _mm_set1_epi8(sep);
    for (; i + 16 <= len; i += 16) {
      const __m128i block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + i));
      unsigned mask       = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(block, needle)));

      for (int b = 0; b < 16 && mask != 0; ++b) {
        const qsizetype pos = i + std::countr_zero(mask);
        if (count >= maxSpans)
          return -1;

        out[count++]  = fieldSpan(data + start, pos - start);
        start         = pos + 1;
        mask         &= mask - 1;
      }
    }
#endif

    for (; i < len; ++i) {
      if (data[i] == sep) {
        if (count >= maxSpans)
          return -1;

        out[count++] = fieldSpan(data + start, i - start);
        start        = i + 1;
      }
    }

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
    Q_ASSERT(maxSpans > 0);
    Q_ASSERT(m_sepUtf8.size() > 1);

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
    Q_ASSERT(n >= 0);

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
    Q_ASSERT(row.isEmpty());

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
    Q_ASSERT(row.isEmpty());
    Q_ASSERT(!m_quote.isNull());

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
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < frame.length());

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
  [[nodiscard]] QString name() const override { return trNative("Delimited text"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Splits each frame into fields using a configurable separator, with "
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
    separator.label        = trNative("Separator");
    separator.description  = trNative("Literal text between fields, e.g. \",\", \";\" or \"|\". "
                                      "Escape sequences such as \\t (tab) are supported.");
    separator.defaultValue = QStringLiteral(",");

    NativeParamSpec quote;
    quote.key          = QStringLiteral("quoteChar");
    quote.type         = NativeParamType::Char;
    quote.label        = trNative("Quote character");
    quote.description  = trNative("Fields wrapped in this character may contain the separator. "
                                  "Leave empty to disable quoting.");
    quote.defaultValue = QStringLiteral("");

    NativeParamSpec trim;
    trim.key          = QStringLiteral("trimFields");
    trim.type         = NativeParamType::Bool;
    trim.label        = trNative("Trim whitespace");
    trim.description  = trNative("Removes leading and trailing whitespace from every field.");
    trim.defaultValue = false;

    NativeParamSpec skip;
    skip.key          = QStringLiteral("skipEmpty");
    skip.type         = NativeParamType::Bool;
    skip.label        = trNative("Skip empty fields");
    skip.description  = trNative("Drops empty fields instead of emitting empty channels.");
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
      error = trNative("The separator must not be empty.");
      return nullptr;
    }

    const QChar quote = DataModel::nativeParamChar(params, QStringLiteral("quoteChar"), QChar());
    if (!quote.isNull() && separator.contains(quote)) {
      error = trNative("The quote character must differ from the field separator.");
      return nullptr;
    }

    const bool trim = DataModel::nativeParamBool(params, QStringLiteral("trimFields"), false);
    const bool skip = DataModel::nativeParamBool(params, QStringLiteral("skipEmpty"), false);
    return std::make_unique<DelimitedTextParser>(separator, quote, trim, skip);
  }
};

//--------------------------------------------------------------------------------------------------
// Shared text helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the trimmed text parses fully as a number.
 */
[[nodiscard]] static bool isNumericValue(QStringView text)
{
  if (text.isEmpty())
    return false;

  bool ok = false;
  (void)SerialStudio::toDouble(text, &ok);
  return ok;
}

/**
 * @brief Builds the key -> channel-index lookup shared by the latching key/value templates.
 */
[[nodiscard]] static QHash<QString, int> buildKeyIndex(const QStringList& keys)
{
  QHash<QString, int> index;
  index.reserve(keys.size());
  for (qsizetype i = 0; i < keys.size(); ++i)
    index.insert(keys.at(i), static_cast<int>(i));

  return index;
}

/**
 * @brief Decodes a percent-encoded string ('+' becomes space, %XX becomes a character).
 */
[[nodiscard]] static QString percentDecode(QStringView text)
{
  if (!text.contains(QLatin1Char('%')) && !text.contains(QLatin1Char('+')))
    return text.toString();

  QString result;
  result.reserve(text.length());

  qsizetype i         = 0;
  const qsizetype len = text.length();
  while (i < len) {
    const QChar c = text.at(i);

    if (c == QLatin1Char('%') && i + 2 < len) {
      bool ok        = false;
      const int code = text.mid(i + 1, 2).toInt(&ok, 16);
      result.append(ok ? QChar(code) : c);
      i += ok ? 3 : 1;
      continue;
    }

    result.append(c == QLatin1Char('+') ? QChar(QLatin1Char(' ')) : c);
    ++i;
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Fixed-width fields
//--------------------------------------------------------------------------------------------------

/**
 * @brief Splits frames on whitespace runs, or slices fixed column widths when configured.
 */
class FixedWidthParser final : public INativeParser {
public:
  /**
   * @brief Stores the column widths (empty = whitespace split) and trim flag.
   */
  FixedWidthParser(const QList<int>& widths, bool trim) : m_widths(widths), m_trim(trim)
  {
    Q_ASSERT(m_widths.size() <= kMaxFields);
  }

  /**
   * @brief Extracts fields by width table or whitespace splitting.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    if (m_widths.isEmpty()) {
      static const QRegularExpression kWhitespace(QStringLiteral("\\s+"));
      auto row = frame.split(kWhitespace, Qt::SkipEmptyParts);
      return singleFrame(std::move(row));
    }

    QStringList row;
    row.reserve(m_widths.size());

    qsizetype pos = 0;
    for (const int width : m_widths) {
      Q_ASSERT(width > 0);
      const QString field = frame.mid(pos, width);
      row.append(m_trim ? field.trimmed() : field);
      pos += width;
    }

    return singleFrame(std::move(row));
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QList<int> m_widths;
  bool m_trim;
};

/**
 * @brief Descriptor for the fixed-width fields template.
 */
class FixedWidthTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("fixed_width"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("Fixed-width fields"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Splits each frame on whitespace runs, or slices fixed column widths when "
                    "a width list is configured.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto widths = DataModel::nativeParam(
      "widths",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Column widths"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated character counts per field. Leave empty to split on "
                        "whitespace."),
      QStringLiteral(""));

    auto trim = DataModel::nativeParam(
      "trimFields",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Trim whitespace"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Removes padding around every sliced field."),
      true);

    return {widths, trim};
  }

  /**
   * @brief Validates the width list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto entries = DataModel::nativeKeyList(params, QStringLiteral("widths"), QString());

    QList<int> widths;
    widths.reserve(entries.size());
    for (const auto& entry : entries) {
      bool ok         = false;
      const int width = entry.toInt(&ok);
      if (!ok || width <= 0) {
        error = trNative("Column widths must be positive integers.");
        return nullptr;
      }

      widths.append(width);
    }

    const bool trim = DataModel::nativeParamBool(params, QStringLiteral("trimFields"), true);
    return std::make_unique<FixedWidthParser>(widths, trim);
  }
};

//--------------------------------------------------------------------------------------------------
// Key-value pairs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching key=value extractor with configurable separators and key order.
 */
class KeyValueParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the separators and ordered key list.
   */
  KeyValueParser(QChar pairSeparator, QChar kvSeparator, const QStringList& keys, bool numericOnly)
    : NativeLatchParser(static_cast<int>(keys.size()))
    , m_pairSeparator(pairSeparator)
    , m_kvSeparator(kvSeparator)
    , m_keys(keys)
    , m_keyIndex(buildKeyIndex(keys))
    , m_numericOnly(numericOnly)
  {
    Q_ASSERT(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value pair in the frame.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_keys.isEmpty());

    const auto pairs = QStringView(frame).split(m_pairSeparator);
    for (const auto pair : pairs) {
      const qsizetype split = pair.indexOf(m_kvSeparator);
      if (split < 0)
        continue;

      const QStringView key   = pair.left(split).trimmed();
      const QStringView value = pair.mid(split + 1).trimmed();
      if (m_numericOnly && !isNumericValue(value))
        continue;

      const auto it = m_keyIndex.constFind(key.toString());
      if (it != m_keyIndex.constEnd())
        storeAt(it.value(), value.toString());
    }

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QChar m_pairSeparator;
  QChar m_kvSeparator;
  QStringList m_keys;
  QHash<QString, int> m_keyIndex;
  bool m_numericOnly;
};

/**
 * @brief Descriptor for the key-value pairs template.
 */
class KeyValueTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("key_value"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("Key-value pairs"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Extracts key=value pairs into a fixed channel order. Missing keys keep "
                    "their previous values between frames.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto keys = DataModel::nativeParam(
      "keys",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Keys (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated key names. The position of each key sets its channel "
                        "index."),
      QStringLiteral("temperature,humidity,pressure"));

    auto pair_sep = DataModel::nativeParam(
      "pairSeparator",
      NativeParamType::Char,
      QT_TRANSLATE_NOOP("NativeTemplates", "Pair separator"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Character between key=value pairs."),
      QStringLiteral(","));

    auto kv_sep = DataModel::nativeParam(
      "kvSeparator",
      NativeParamType::Char,
      QT_TRANSLATE_NOOP("NativeTemplates", "Key-value separator"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Character between a key and its value."),
      QStringLiteral("="));

    auto numeric = DataModel::nativeParam(
      "numericOnly",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Numeric values only"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Ignores pairs whose value is not a number."),
      true);

    return {keys, pair_sep, kv_sep, numeric};
  }

  /**
   * @brief Validates the key list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto keys = DataModel::nativeKeyList(
      params, QStringLiteral("keys"), QStringLiteral("temperature,humidity,pressure"));
    if (keys.isEmpty()) {
      error = trNative("At least one key is required.");
      return nullptr;
    }

    const QChar pair_sep =
      DataModel::nativeParamChar(params, QStringLiteral("pairSeparator"), QLatin1Char(','));
    const QChar kv_sep =
      DataModel::nativeParamChar(params, QStringLiteral("kvSeparator"), QLatin1Char('='));
    if (pair_sep == kv_sep) {
      error = trNative("The pair separator must differ from the key-value separator.");
      return nullptr;
    }

    const bool numeric = DataModel::nativeParamBool(params, QStringLiteral("numericOnly"), true);
    return std::make_unique<KeyValueParser>(pair_sep, kv_sep, keys, numeric);
  }
};

//--------------------------------------------------------------------------------------------------
// INI / config format
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching INI line extractor (key=value per line, ; and # comments skipped).
 */
class IniConfigParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the ordered key list.
   */
  explicit IniConfigParser(const QStringList& keys)
    : NativeLatchParser(static_cast<int>(keys.size()))
    , m_keys(keys)
    , m_keyIndex(buildKeyIndex(keys))
  {
    Q_ASSERT(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value line in the frame.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_keys.isEmpty());

    const auto lines = QStringView(frame).split(QLatin1Char('\n'));
    for (const auto raw : lines) {
      const QStringView line = raw.trimmed();
      if (line.isEmpty() || line.startsWith(QLatin1Char(';')) || line.startsWith(QLatin1Char('#')))
        continue;

      const qsizetype split = line.indexOf(QLatin1Char('='));
      if (split < 0)
        continue;

      const QStringView key   = line.left(split).trimmed();
      const QStringView value = line.mid(split + 1).trimmed();
      const auto it           = m_keyIndex.constFind(key.toString());
      if (it != m_keyIndex.constEnd())
        storeAt(it.value(), value.toString());
    }

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QStringList m_keys;
  QHash<QString, int> m_keyIndex;
};

/**
 * @brief Descriptor for the INI/config template.
 */
class IniConfigTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("ini_config"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("INI/config format"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Reads key=value lines (comments with ; or # are skipped) into a fixed "
                    "channel order with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto keys = DataModel::nativeParam(
      "keys",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Keys (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated key names. The position of each key sets its channel "
                        "index."),
      QStringLiteral("temperature,humidity,pressure,battery,signal"));

    return {keys};
  }

  /**
   * @brief Validates the key list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto keys =
      DataModel::nativeKeyList(params,
                               QStringLiteral("keys"),
                               QStringLiteral("temperature,humidity,pressure,battery,signal"));
    if (keys.isEmpty()) {
      error = trNative("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<IniConfigParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// AT command responses
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching AT response extractor: +CMD: p1,p2 routed via a command-to-index table.
 */
class AtCommandsParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the command-to-indices routing table.
   */
  AtCommandsParser(const QHash<QString, QList<int>>& commands, int count)
    : NativeLatchParser(count), m_commands(commands)
  {
    Q_ASSERT(!m_commands.isEmpty());
  }

  /**
   * @brief Routes the response parameters into their mapped channel indices.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_commands.isEmpty());

    const QString trimmed = frame.trimmed();
    const qsizetype colon = trimmed.indexOf(QLatin1Char(':'));
    if (colon < 0)
      return latchedFrame();

    QString command = trimmed.left(colon).trimmed();
    command.remove(QLatin1Char('+'));

    const auto it = m_commands.constFind(command);
    if (it == m_commands.constEnd())
      return latchedFrame();

    const auto values = trimmed.mid(colon + 1).split(QLatin1Char(','));
    const auto& idx   = it.value();
    for (qsizetype i = 0; i < values.size() && i < idx.size(); ++i)
      storeAt(idx.at(i), values.at(i).trimmed());

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QHash<QString, QList<int>> m_commands;
};

/**
 * @brief Descriptor for the AT command responses template.
 */
class AtCommandsTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("at_commands"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("AT command responses"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Maps modem responses like +CSQ: 25,99 into channels using a command "
                    "routing table with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto commands = DataModel::nativeParam(
      "commands",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Command routing table"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Semicolon-separated entries of NAME:index list, e.g. "
                        "CSQ:0,1;CREG:2,3;CGATT:4."),
      QStringLiteral("CSQ:0,1;CREG:2,3;CGATT:4;COPS:5,6,7"));

    return {commands};
  }

  /**
   * @brief Parses the routing table and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString table = DataModel::nativeParamString(
      params, QStringLiteral("commands"), QStringLiteral("CSQ:0,1;CREG:2,3;CGATT:4;COPS:5,6,7"));

    QHash<QString, QList<int>> commands;
    int max_index = -1;

    const auto entries = table.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const auto& entry : entries) {
      const qsizetype colon = entry.indexOf(QLatin1Char(':'));
      const QString name    = (colon < 0) ? QString() : entry.left(colon).trimmed();
      if (name.isEmpty())
        continue;

      QList<int> indices;
      const auto parts = entry.mid(colon + 1).split(QLatin1Char(','), Qt::SkipEmptyParts);
      for (const auto& part : parts) {
        bool ok         = false;
        const int index = part.trimmed().toInt(&ok);
        if (!ok || index < 0)
          continue;

        indices.append(index);
        max_index = qMax(max_index, index);
      }

      if (!indices.isEmpty())
        commands.insert(name, indices);
    }

    if (commands.isEmpty() || max_index < 0) {
      error = trNative("The command routing table must contain at least one NAME:index entry.");
      return nullptr;
    }

    return std::make_unique<AtCommandsParser>(commands, max_index + 1);
  }
};

//--------------------------------------------------------------------------------------------------
// NMEA 0183
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching NMEA 0183 sentence extractor (GGA, RMC, GLL, VTG) with optional checksum.
 */
class Nmea0183Parser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the talker prefix and checksum-validation flag.
   */
  Nmea0183Parser(const QString& talker, bool validateChecksum)
    : NativeLatchParser(kNmeaChannels), m_talker(talker), m_validateChecksum(validateChecksum)
  {
    Q_ASSERT(!m_talker.isEmpty());
  }

  /**
   * @brief Validates and routes a sentence into the fixed channel layout.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    const QString sentence = frame.trimmed();
    if (!sentence.startsWith(QLatin1Char('$')))
      return latchedFrame();

    const qsizetype star = sentence.indexOf(QLatin1Char('*'));
    if (m_validateChecksum && star >= 0 && !checksumOk(sentence, star))
      return latchedFrame();

    const QString data = (star >= 0) ? sentence.left(star) : sentence;
    const auto fields  = data.split(QLatin1Char(','));
    const QString id   = fields.constFirst().mid(1);
    if (!id.startsWith(m_talker))
      return latchedFrame();

    routeSentence(id.mid(m_talker.length()), fields);
    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  /**
   * @brief XOR-validates the checksum between $ and *.
   */
  [[nodiscard]] static bool checksumOk(const QString& sentence, qsizetype star)
  {
    Q_ASSERT(star > 0);
    Q_ASSERT(sentence.startsWith(QLatin1Char('$')));

    bool ok            = false;
    const int expected = QStringView(sentence).mid(star + 1).toInt(&ok, 16);
    if (!ok)
      return false;

    int computed = 0;
    for (qsizetype i = 1; i < star; ++i)
      computed ^= sentence.at(i).toLatin1();

    return computed == expected;
  }

  /**
   * @brief Converts DDMM.MMMM + hemisphere to signed decimal degrees.
   */
  [[nodiscard]] static double parseCoordinate(const QString& text,
                                              const QString& hemisphere,
                                              int degreeDigits)
  {
    Q_ASSERT(degreeDigits == 2 || degreeDigits == 3);

    if (text.length() < degreeDigits + 2)
      return 0.0;

    static constexpr double kInvMinutesPerDegree = 1.0 / 60.0;

    const double degrees = SerialStudio::toDouble(QStringView(text).left(degreeDigits), nullptr);
    const double minutes = SerialStudio::toDouble(QStringView(text).mid(degreeDigits), nullptr);
    const double decimal = degrees + (minutes * kInvMinutesPerDegree);

    const bool negative = (hemisphere == QStringLiteral("S") || hemisphere == QStringLiteral("W"));
    return negative ? -decimal : decimal;
  }

  /**
   * @brief Routes a sentence formatter (GGA/RMC/GLL/VTG) into the channel layout.
   */
  void routeSentence(const QString& formatter, const QStringList& fields)
  {
    const auto field = [&fields](qsizetype i) {
      return (i < fields.size()) ? fields.at(i) : QString();
    };
    const auto numeric = [&field](qsizetype i) {
      return QString::number(SerialStudio::toDouble(QStringView(field(i)), nullptr));
    };

    if (formatter == QStringLiteral("GGA")) {
      storeAt(0, QString::number(parseCoordinate(field(2), field(3), 2)));
      storeAt(1, QString::number(parseCoordinate(field(4), field(5), 3)));
      storeAt(2, numeric(9));
      storeAt(3, numeric(6));
      storeAt(4, numeric(7));
      storeAt(5, numeric(8));
      return;
    }

    if (formatter == QStringLiteral("RMC")) {
      storeAt(0, QString::number(parseCoordinate(field(3), field(4), 2)));
      storeAt(1, QString::number(parseCoordinate(field(5), field(6), 3)));
      storeAt(6, numeric(7));
      storeAt(7, numeric(8));
      return;
    }

    if (formatter == QStringLiteral("GLL")) {
      storeAt(0, QString::number(parseCoordinate(field(1), field(2), 2)));
      storeAt(1, QString::number(parseCoordinate(field(3), field(4), 3)));
      return;
    }

    if (formatter == QStringLiteral("VTG")) {
      storeAt(6, numeric(5));
      storeAt(7, numeric(7));
    }
  }

private:
  static constexpr int kNmeaChannels = 16;

  QString m_talker;
  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the NMEA 0183 template.
 */
class Nmea0183Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("nmea_0183"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("NMEA 0183 sentences"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Decodes GGA, RMC, GLL and VTG sentences into latitude, longitude, "
                    "altitude, speed and fix-quality channels.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto talker =
      DataModel::nativeParam("talker",
                             NativeParamType::String,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Talker prefix"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Two-letter talker id, e.g. GP for GPS or GN for "
                                               "multi-constellation receivers."),
                             QStringLiteral("GP"));

    auto checksum =
      DataModel::nativeParam("validateChecksum",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Validate checksum"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Rejects sentences whose *hh checksum does not "
                                               "match."),
                             true);

    return {talker, checksum};
  }

  /**
   * @brief Validates the talker prefix and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString talker =
      DataModel::nativeParamString(params, QStringLiteral("talker"), QStringLiteral("GP"))
        .trimmed();
    if (talker.isEmpty()) {
      error = trNative("The talker prefix must not be empty.");
      return nullptr;
    }

    const bool checksum =
      DataModel::nativeParamBool(params, QStringLiteral("validateChecksum"), true);
    return std::make_unique<Nmea0183Parser>(talker, checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// URL-encoded data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching key=value extractor for percent-encoded form data (a&b pairs).
 */
class UrlEncodedParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the ordered key list.
   */
  explicit UrlEncodedParser(const QStringList& keys)
    : NativeLatchParser(static_cast<int>(keys.size()))
    , m_keys(keys)
    , m_keyIndex(buildKeyIndex(keys))
  {
    Q_ASSERT(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value pair.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_keys.isEmpty());

    const auto pairs = QStringView(frame).trimmed().split(QLatin1Char('&'), Qt::SkipEmptyParts);
    for (const auto pair : pairs) {
      const qsizetype split = pair.indexOf(QLatin1Char('='));
      if (split < 0)
        continue;

      const auto it = m_keyIndex.constFind(percentDecode(pair.left(split)));
      if (it != m_keyIndex.constEnd())
        storeAt(it.value(), percentDecode(pair.mid(split + 1)));
    }

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QStringList m_keys;
  QHash<QString, int> m_keyIndex;
};

/**
 * @brief Descriptor for the URL-encoded data template.
 */
class UrlEncodedTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("url_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("URL-encoded data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Decodes key=value&key=value form data (percent-encoding handled) into a "
                    "fixed channel order with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto keys = DataModel::nativeParam(
      "keys",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Keys (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated parameter names. The position of each key sets its "
                        "channel index."),
      QStringLiteral("temperature,humidity,pressure,voltage,current,power"));

    return {keys};
  }

  /**
   * @brief Validates the key list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto keys = DataModel::nativeKeyList(
      params,
      QStringLiteral("keys"),
      QStringLiteral("temperature,humidity,pressure,voltage,current,power"));
    if (keys.isEmpty()) {
      error = trNative("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<UrlEncodedParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// JSON data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching extractor for top-level JSON object fields.
 */
class JsonDataParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the ordered field list.
   */
  explicit JsonDataParser(const QStringList& fields)
    : NativeLatchParser(static_cast<int>(fields.size())), m_fields(fields)
  {
    Q_ASSERT(!m_fields.isEmpty());
  }

  /**
   * @brief Updates latched values from every mapped field present in the JSON object.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseJsonBytes(frame.toUtf8());
  }

  /**
   * @brief Parses the JSON bytes directly, skipping the QString round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    return parseJsonBytes(frame);
  }

  /**
   * @brief Treats binary frames as UTF-8 JSON bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseJsonBytes(frame);
  }

private:
  /**
   * @brief Shared byte-level core for the text/UTF-8/binary entry points.
   */
  [[nodiscard]] QList<QStringList> parseJsonBytes(const QByteArray& bytes)
  {
    Q_ASSERT(!m_fields.isEmpty());

    const auto doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject())
      return latchedFrame();

    const auto obj = doc.object();
    for (int i = 0; i < m_fields.size(); ++i)
      storeJsonScalar(i, obj.value(m_fields.at(i)));

    return latchedFrame();
  }

  /**
   * @brief Stores a scalar JSON value at the channel index; non-scalars keep previous values.
   */
  void storeJsonScalar(int index, const QJsonValue& value)
  {
    if (value.isDouble())
      return storeAt(index, QString::number(SerialStudio::toDouble(value)));

    if (value.isString())
      return storeAt(index, value.toString());

    if (value.isBool())
      storeAt(index, value.toBool() ? QStringLiteral("true") : QStringLiteral("false"));
  }

private:
  QStringList m_fields;
};

/**
 * @brief Descriptor for the JSON data template.
 */
class JsonDataTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("json_data"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("JSON data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Extracts top-level fields of a JSON object into a fixed channel order "
                    "with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto fields = DataModel::nativeParam(
      "fields",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Fields (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated field names. The position of each field sets its "
                        "channel index."),
      QStringLiteral("time,speed,angle,distance"));

    return {fields};
  }

  /**
   * @brief Validates the field list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto fields = DataModel::nativeKeyList(
      params, QStringLiteral("fields"), QStringLiteral("time,speed,angle,distance"));
    if (fields.isEmpty()) {
      error = trNative("At least one field is required.");
      return nullptr;
    }

    return std::make_unique<JsonDataParser>(fields);
  }
};

//--------------------------------------------------------------------------------------------------
// XML data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching extractor for simple <tag>value</tag> XML elements.
 */
class XmlDataParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the ordered tag list.
   */
  explicit XmlDataParser(const QStringList& tags)
    : NativeLatchParser(static_cast<int>(tags.size())), m_tags(tags)
  {
    Q_ASSERT(!m_tags.isEmpty());

    m_openTags.reserve(tags.size());
    m_closeTags.reserve(tags.size());
    for (const auto& tag : tags) {
      m_openTags.append(QLatin1Char('<') + tag + QLatin1Char('>'));
      m_closeTags.append(QStringLiteral("</") + tag + QLatin1Char('>'));
    }
  }

  /**
   * @brief Updates latched values from the first occurrence of every mapped tag.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_tags.isEmpty());

    for (int i = 0; i < m_tags.size(); ++i) {
      const qsizetype start = frame.indexOf(m_openTags.at(i));
      if (start < 0)
        continue;

      const qsizetype end = frame.indexOf(m_closeTags.at(i), start);
      if (end < 0)
        continue;

      const qsizetype from = start + m_openTags.at(i).length();
      storeAt(i, QStringView(frame).mid(from, end - from).trimmed().toString());
    }

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  QStringList m_tags;
  QStringList m_openTags;
  QStringList m_closeTags;
};

/**
 * @brief Descriptor for the XML data template.
 */
class XmlDataTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("xml_data"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("XML data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Extracts the text content of simple XML elements into a fixed channel "
                    "order with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto tags = DataModel::nativeParam(
      "tags",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Tags (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated tag names. The position of each tag sets its channel "
                        "index."),
      QStringLiteral("temperature,humidity,pressure,voltage,current,altitude,speed"));

    return {tags};
  }

  /**
   * @brief Validates the tag list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto tags = DataModel::nativeKeyList(
      params,
      QStringLiteral("tags"),
      QStringLiteral("temperature,humidity,pressure,voltage,current,altitude,speed"));
    if (tags.isEmpty()) {
      error = trNative("At least one tag is required.");
      return nullptr;
    }

    return std::make_unique<XmlDataParser>(tags);
  }
};

//--------------------------------------------------------------------------------------------------
// YAML data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching extractor for flat YAML key: value lines.
 */
class YamlDataParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the ordered key list.
   */
  explicit YamlDataParser(const QStringList& keys)
    : NativeLatchParser(static_cast<int>(keys.size()))
    , m_keys(keys)
    , m_keyIndex(buildKeyIndex(keys))
  {
    Q_ASSERT(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key: value line.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!m_keys.isEmpty());

    const auto lines = QStringView(frame).split(QLatin1Char('\n'));
    for (const auto raw : lines) {
      const QStringView line = stripComment(raw).trimmed();
      if (line.isEmpty() || line == QLatin1String("---") || line == QLatin1String("..."))
        continue;

      const qsizetype colon = line.indexOf(QLatin1Char(':'));
      if (colon < 0)
        continue;

      const QString key = stripQuotes(line.left(colon).trimmed()).toString();
      const auto it     = m_keyIndex.constFind(key);
      if (it == m_keyIndex.constEnd())
        continue;

      const QString value = yamlValue(line.mid(colon + 1).trimmed());
      if (!value.isNull())
        storeAt(it.value(), value);
    }

    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  /**
   * @brief Removes an inline # comment from the line.
   */
  [[nodiscard]] static QStringView stripComment(QStringView line)
  {
    const qsizetype pos = line.indexOf(QLatin1Char('#'));
    return (pos < 0) ? line : line.left(pos);
  }

  /**
   * @brief Removes matching single or double quotes around the text.
   */
  [[nodiscard]] static QStringView stripQuotes(QStringView text)
  {
    if (text.length() < 2)
      return text;

    const QChar first = text.front();
    const QChar last  = text.back();
    const bool quoted =
      (first == last) && (first == QLatin1Char('"') || first == QLatin1Char('\''));
    return quoted ? text.mid(1, text.length() - 2) : text;
  }

  /**
   * @brief Normalizes a YAML scalar; null markers return a null QString (keep previous).
   */
  [[nodiscard]] static QString yamlValue(QStringView text)
  {
    if (text.isEmpty() || text == QLatin1String("null") || text == QLatin1String("~"))
      return QString();

    if (text == QLatin1String("true") || text == QLatin1String("yes")
        || text == QLatin1String("on"))
      return QStringLiteral("true");

    if (text == QLatin1String("false") || text == QLatin1String("no")
        || text == QLatin1String("off"))
      return QStringLiteral("false");

    return stripQuotes(text).toString();
  }

private:
  QStringList m_keys;
  QHash<QString, int> m_keyIndex;
};

/**
 * @brief Descriptor for the YAML data template.
 */
class YamlDataTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("yaml_data"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNative("YAML data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNative("Reads flat YAML key: value lines (comments and quoting handled) into a "
                    "fixed channel order with latched values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto keys = DataModel::nativeParam(
      "keys",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Keys (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated key names. The position of each key sets its channel "
                        "index."),
      QStringLiteral("temperature,humidity,pressure,voltage,current,altitude,speed"));

    return {keys};
  }

  /**
   * @brief Validates the key list and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto keys = DataModel::nativeKeyList(
      params,
      QStringLiteral("keys"),
      QStringLiteral("temperature,humidity,pressure,voltage,current,altitude,speed"));
    if (keys.isEmpty()) {
      error = trNative("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<YamlDataParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the text-oriented native templates in display order.
 */
QList<const DataModel::INativeTemplate*> DataModel::textNativeTemplates()
{
  static const DelimitedTextTemplate s_delimited;
  static const FixedWidthTemplate s_fixedWidth;
  static const KeyValueTemplate s_keyValue;
  static const IniConfigTemplate s_iniConfig;
  static const AtCommandsTemplate s_atCommands;
  static const Nmea0183Template s_nmea0183;
  static const UrlEncodedTemplate s_urlEncoded;
  static const JsonDataTemplate s_jsonData;
  static const XmlDataTemplate s_xmlData;
  static const YamlDataTemplate s_yamlData;

  return {&s_delimited,
          &s_fixedWidth,
          &s_keyValue,
          &s_iniConfig,
          &s_atCommands,
          &s_nmea0183,
          &s_urlEncoded,
          &s_jsonData,
          &s_xmlData,
          &s_yamlData};
}
