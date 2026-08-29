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

#include "DataModel/Scripting/NativeTemplates/TextJsonData.h"

#include <QJsonDocument>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SerialStudio.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!m_fields.isEmpty());
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
    SS_ASSERT(!m_fields.isEmpty(), return latchedFrame());

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("JSON data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Extracts top-level fields of a JSON object into a fixed channel "
                            "order with latched values.");
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
      error = trNativeTemplate("At least one field is required.");
      return nullptr;
    }

    return std::make_unique<JsonDataParser>(fields);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide JSON data template descriptor.
 */
const DataModel::INativeTemplate& DataModel::jsonDataTemplate()
{
  static const JsonDataTemplate s_jsonData;
  return s_jsonData;
}
