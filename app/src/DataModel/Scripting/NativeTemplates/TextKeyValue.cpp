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

#include "DataModel/Scripting/NativeTemplates/TextKeyValue.h"

#include <QHash>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SerialStudio.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Key-value pairs
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
    SS_ASSERT_LOG(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value pair in the frame.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_keys.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("Key-value pairs"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Extracts key=value pairs into a fixed channel order. Missing keys "
                            "keep their previous values between frames.");
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
      error = trNativeTemplate("At least one key is required.");
      return nullptr;
    }

    const QChar pair_sep =
      DataModel::nativeParamChar(params, QStringLiteral("pairSeparator"), QLatin1Char(','));
    const QChar kv_sep =
      DataModel::nativeParamChar(params, QStringLiteral("kvSeparator"), QLatin1Char('='));
    if (pair_sep == kv_sep) {
      error = trNativeTemplate("The pair separator must differ from the key-value separator.");
      return nullptr;
    }

    const bool numeric = DataModel::nativeParamBool(params, QStringLiteral("numericOnly"), true);
    return std::make_unique<KeyValueParser>(pair_sep, kv_sep, keys, numeric);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide key-value pairs template descriptor.
 */
const DataModel::INativeTemplate& DataModel::keyValueTemplate()
{
  static const KeyValueTemplate s_keyValue;
  return s_keyValue;
}
