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

#include "DataModel/Scripting/NativeTemplates/TextYamlData.h"

#include <QHash>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key: value line.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_keys.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("YAML data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Reads flat YAML key: value lines (comments and quoting handled) "
                            "into a fixed channel order with latched values.");
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
      error = trNativeTemplate("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<YamlDataParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide YAML data template descriptor.
 */
const DataModel::INativeTemplate& DataModel::yamlDataTemplate()
{
  static const YamlDataTemplate s_yamlData;
  return s_yamlData;
}
