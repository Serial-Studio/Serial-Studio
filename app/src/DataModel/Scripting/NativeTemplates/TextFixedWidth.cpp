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

#include "DataModel/Scripting/NativeTemplates/TextFixedWidth.h"

#include <QRegularExpression>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
  FixedWidthParser(const QList<int>& widths, bool trim)
    : m_widths(widths.mid(0, kMaxFields)), m_trim(trim)
  {
    SS_ASSERT_LOG(widths.size() <= kMaxFields);
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
      SS_ASSERT_LOG(width > 0);
      if (width <= 0)
        continue;

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("Fixed-width fields"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Splits each frame on whitespace runs, or slices fixed column widths "
                            "when a width list is configured.");
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
        error = trNativeTemplate("Column widths must be positive integers.");
        return nullptr;
      }

      widths.append(width);
    }

    const bool trim = DataModel::nativeParamBool(params, QStringLiteral("trimFields"), true);
    return std::make_unique<FixedWidthParser>(widths, trim);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide fixed-width-fields template descriptor.
 */
const DataModel::INativeTemplate& DataModel::fixedWidthTemplate()
{
  static const FixedWidthTemplate s_fixedWidth;
  return s_fixedWidth;
}
