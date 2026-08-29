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

#include "DataModel/Scripting/NativeTemplates/TextIniConfig.h"

#include <QHash>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value line in the frame.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_keys.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("INI/config format"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Reads key=value lines (comments with ; or # are skipped) into a "
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
      error = trNativeTemplate("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<IniConfigParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide INI/config template descriptor.
 */
const DataModel::INativeTemplate& DataModel::iniConfigTemplate()
{
  static const IniConfigTemplate s_iniConfig;
  return s_iniConfig;
}
