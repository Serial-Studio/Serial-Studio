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

#include "DataModel/Scripting/NativeTemplates/TextXmlData.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!m_tags.isEmpty());

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
    SS_ASSERT(!m_tags.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("XML data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Extracts the text content of simple XML elements into a fixed "
                            "channel order with latched values.");
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
      error = trNativeTemplate("At least one tag is required.");
      return nullptr;
    }

    return std::make_unique<XmlDataParser>(tags);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide XML data template descriptor.
 */
const DataModel::INativeTemplate& DataModel::xmlDataTemplate()
{
  static const XmlDataTemplate s_xmlData;
  return s_xmlData;
}
