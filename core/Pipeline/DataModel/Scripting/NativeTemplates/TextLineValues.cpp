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

#include "DataModel/Scripting/NativeTemplates/TextLineValues.h"

#include <array>
#include <memory>
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <utility>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/LineFormat.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::LineField;
using DataModel::LineFields;
using DataModel::LineFormat;
using DataModel::LineFormatKind;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

namespace LineValuesDetail {

struct SeparatorOption {
  const char* name;
  char separator;
};

/**
 * @brief The separator option names the template exposes and the tokenizer bytes they select.
 */
inline constexpr std::array<SeparatorOption, 4> kSeparatorOptions{
  {{"comma", ','}, {"semicolon", ';'}, {"tab", '\t'}, {"space", ' '}}
};

}  // namespace LineValuesDetail

//--------------------------------------------------------------------------------------------------
// Field rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief The span a dataset converts: the number without its unit for a numeric field, the whole
 *        value text otherwise.
 */
[[nodiscard]] static QByteArrayView numberSpan(const LineField& field) noexcept
{
  if (!field.numeric)
    return field.value;

  const qsizetype numberLength = field.value.size() - field.unit.size();
  SS_ASSERT(numberLength > 0, return field.value);
  return DataModel::trimLineAscii(field.value.first(numberLength));
}

/**
 * @brief The cell text of one field, with a decimal comma rewritten to the point the dataset
 *        conversion understands.
 */
[[nodiscard]] static QString cellText(const LineField& field)
{
  SS_ASSERT(!field.numeric || !field.value.isEmpty(), return QString());
  SS_ASSERT(field.unit.size() <= field.value.size(), return QString());
  QString text = QString::fromUtf8(numberSpan(field));
  if (field.numeric)
    text.replace(QLatin1Char(','), QLatin1Char('.'));

  return text;
}

/**
 * @brief Maps the separator option name to the tokenizer's separator byte.
 */
[[nodiscard]] static bool separatorFromName(const QString& name, char& separator) noexcept
{
  SS_ASSERT(!name.isEmpty(), return false);
  SS_ASSERT(LineValuesDetail::kSeparatorOptions.size() == 4, return false);
  for (const auto& option : LineValuesDetail::kSeparatorOptions) {
    if (name != QLatin1String(option.name))
      continue;

    separator = option.separator;
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Parsers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Positional parser: one cell per field in line order. The span path serves the Native
 *        fast lane whenever no cell needs rewriting; a decimal comma forces the text path.
 */
class LineValuesPositionalParser final : public INativeParser {
public:
  /**
   * @brief Stores the fixed format every line is split under.
   */
  explicit LineValuesPositionalParser(const LineFormat& format) : m_format(format)
  {
    SS_ASSERT_LOG(m_format.kind == LineFormatKind::Delimited);
  }

  /**
   * @brief Transcodes once and reuses the UTF-8 path.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseUtf8(frame.toUtf8());
  }

  /**
   * @brief Treats binary frames as UTF-8 text.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseUtf8(frame);
  }

  /**
   * @brief Splits the line and renders one normalized cell per field.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    const qsizetype count = DataModel::tokenizeLine(frame, m_format, m_fields);
    SS_ASSERT(DataModel::kMaxLineFields >= count, return {});
    if (count <= 0)
      return {};

    QStringList row;
    row.reserve(static_cast<int>(count));
    for (qsizetype i = 0; i < count; ++i)
      row.append(cellText(m_fields.items[static_cast<size_t>(i)]));

    return singleFrame(std::move(row));
  }

  /**
   * @brief Allocation-free cell views for the span lane; -1 when a decimal comma needs rewriting
   *        or the caller's span table is too small.
   */
  [[nodiscard]] qsizetype parseSpans(QByteArrayView frame,
                                     QByteArrayView* out,
                                     qsizetype maxSpans) noexcept override
  {
    SS_ASSERT(out != nullptr, return -1);
    SS_ASSERT(maxSpans > 0, return -1);
    if (m_format.decimalComma)
      return -1;

    const qsizetype count = DataModel::tokenizeLine(frame, m_format, m_fields);
    if (count <= 0)
      return 0;

    if (count > maxSpans)
      return -1;

    for (qsizetype i = 0; i < count; ++i)
      out[i] = numberSpan(m_fields.items[static_cast<size_t>(i)]);

    return count;
  }

private:
  LineFormat m_format;
  LineFields m_fields;
};

/**
 * @brief Keyed parser: named fields and JSON leaves latch into the configured field order, so a
 *        line that omits a name repeats the previous value like every key-addressed template. An
 *        unnamed field of a mixed line lands at its own position.
 */
class LineValuesKeyedParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the format and the field order.
   */
  LineValuesKeyedParser(const LineFormat& format, const QStringList& fields)
    : NativeLatchParser(static_cast<int>(fields.size()))
    , m_format(format)
    , m_index(buildIndex(fields))
  {
    SS_ASSERT_LOG(!fields.isEmpty());
  }

  /**
   * @brief Transcodes once and reuses the UTF-8 path.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseUtf8(frame.toUtf8());
  }

  /**
   * @brief Treats binary frames as UTF-8 text.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseUtf8(frame);
  }

  /**
   * @brief Latches every recognized field of the line and returns the full latched row.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    const qsizetype count = DataModel::tokenizeLine(frame, m_format, m_fields);
    SS_ASSERT(DataModel::kMaxLineFields >= count, return latchedFrame());
    for (qsizetype i = 0; i < count; ++i) {
      const auto& field = m_fields.items[static_cast<size_t>(i)];
      const int slot    = slotFor(field, i);
      if (slot >= 0)
        storeAt(slot, cellText(field));
    }

    return latchedFrame();
  }

private:
  /**
   * @brief Field order lookup keyed by the UTF-8 name or JSON path.
   */
  [[nodiscard]] static QHash<QByteArray, int> buildIndex(const QStringList& fields)
  {
    QHash<QByteArray, int> index;
    for (int i = 0; i < fields.size(); ++i)
      index.insert(fields.at(i).toUtf8(), i);

    return index;
  }

  /**
   * @brief Latch slot of a field: its position when unnamed, its name's configured order
   *        otherwise, -1 when unknown.
   */
  [[nodiscard]] int slotFor(const LineField& field, const qsizetype position) const
  {
    SS_ASSERT(position >= 0, return -1);
    if (field.name.isEmpty())
      return position < latchCount() ? static_cast<int>(position) : -1;

    const auto it = m_index.constFind(QByteArray(field.name.data(), field.name.size()));
    return it != m_index.constEnd() ? it.value() : -1;
  }

private:
  LineFormat m_format;
  QHash<QByteArray, int> m_index;
  LineFields m_fields;
};

//--------------------------------------------------------------------------------------------------
// Descriptor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the line-values template.
 */
class LineValuesTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("line_values"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Line values"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("One line per sample in the shapes Quick Plot detects: values split on "
                            "a separator or a one-line JSON object, optional name:value pairs, "
                            "units after numbers and a decimal comma. A line wrapped in /* */ is "
                            "unwrapped. Missing named fields keep their previous values.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto format = DataModel::nativeParam(
      "format",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Format"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Delimited values on one line, or a one-line JSON object whose numeric "
                        "leaves are the fields."),
      QStringLiteral("delimited"));
    format.optionValues = {QStringLiteral("delimited"), QStringLiteral("json")};
    format.optionLabels = {trNativeTemplate("Delimited"), trNativeTemplate("JSON object")};

    auto separator = DataModel::nativeParam(
      "separator",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Separator"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "What separates the values of a delimited line; space means any run of "
                        "spaces."),
      QStringLiteral("comma"));
    separator.optionValues = {QStringLiteral("comma"),
                              QStringLiteral("semicolon"),
                              QStringLiteral("tab"),
                              QStringLiteral("space")};
    separator.optionLabels = {trNativeTemplate("Comma"),
                              trNativeTemplate("Semicolon"),
                              trNativeTemplate("Tab"),
                              trNativeTemplate("Space")};

    auto decimalComma = DataModel::nativeParam(
      "decimalComma",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Decimal comma"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Reads 23,5 as 23.5 (semicolon or space separated lines)."),
      false);

    auto fields = DataModel::nativeParam(
      "fields",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Fields (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated field names or JSON paths such as temp, hum or imu.ax, "
                        "acc[0]. Leave empty for positional values."),
      QString());

    return {format, separator, decimalComma, fields};
  }

  /**
   * @brief Validates the options and builds the positional or keyed parser.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    LineFormat format;
    const QString formatName =
      DataModel::nativeParamString(params, QStringLiteral("format"), QStringLiteral("delimited"));
    if (formatName == QLatin1String("json"))
      format.kind = LineFormatKind::Json;
    else if (formatName != QLatin1String("delimited")) {
      error = trNativeTemplate("The format must be delimited or json.");
      return nullptr;
    }

    const QString separatorName =
      DataModel::nativeParamString(params, QStringLiteral("separator"), QStringLiteral("comma"));
    if (!separatorFromName(separatorName, format.separator)) {
      error = trNativeTemplate("The separator must be comma, semicolon, tab or space.");
      return nullptr;
    }

    format.decimalComma = DataModel::nativeParamBool(params, QStringLiteral("decimalComma"), false);
    const auto fields   = DataModel::nativeKeyList(params, QStringLiteral("fields"), QString());
    if (format.kind == LineFormatKind::Json && fields.isEmpty()) {
      error = trNativeTemplate("A JSON format needs at least one field path.");
      return nullptr;
    }

    if (fields.isEmpty())
      return std::make_unique<LineValuesPositionalParser>(format);

    return std::make_unique<LineValuesKeyedParser>(format, fields);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide line-values template descriptor.
 */
const DataModel::INativeTemplate& DataModel::lineValuesTemplate()
{
  static const LineValuesTemplate s_lineValues;
  return s_lineValues;
}
