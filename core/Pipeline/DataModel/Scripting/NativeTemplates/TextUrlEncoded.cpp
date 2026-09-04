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

#include "DataModel/Scripting/NativeTemplates/TextUrlEncoded.h"

#include <QHash>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// URL-encoded data
//--------------------------------------------------------------------------------------------------

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
    SS_ASSERT_LOG(!m_keys.isEmpty());
  }

  /**
   * @brief Updates latched values from every recognized key=value pair.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!m_keys.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("URL-encoded data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes key=value&key=value form data (percent-encoding handled) "
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
      error = trNativeTemplate("At least one key is required.");
      return nullptr;
    }

    return std::make_unique<UrlEncodedParser>(keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide URL-encoded data template descriptor.
 */
const DataModel::INativeTemplate& DataModel::urlEncodedTemplate()
{
  static const UrlEncodedTemplate s_urlEncoded;
  return s_urlEncoded;
}
