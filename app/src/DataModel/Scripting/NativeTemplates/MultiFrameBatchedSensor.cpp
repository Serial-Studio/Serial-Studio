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

#include "DataModel/Scripting/NativeTemplates/MultiFrameBatchedSensor.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Batched sensor data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Expands one JSON packet with scalar metadata + a sample vector into N frames.
 */
class BatchedSensorParser final : public INativeParser {
public:
  /**
   * @brief Stores the scalar field list and the batched vector field name.
   */
  BatchedSensorParser(const QStringList& scalarFields, const QString& vectorField)
    : m_scalarFields(scalarFields), m_vectorField(vectorField)
  {
    SS_ASSERT_LOG(!m_vectorField.isEmpty());
  }

  /**
   * @brief Emits one frame per vector element, repeating the scalar metadata.
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
    SS_ASSERT(!m_vectorField.isEmpty(), return {});

    const auto doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject())
      return {};

    const auto obj = doc.object();
    QStringList scalars;
    scalars.reserve(m_scalarFields.size());
    for (const auto& field : m_scalarFields)
      scalars.append(jsonScalar(obj.value(field)));

    const auto samples = obj.value(m_vectorField).toArray();
    if (samples.isEmpty()) {
      QList<QStringList> out;
      out.append(scalars);
      return out;
    }

    QList<QStringList> out;
    const qsizetype count = qMin<qsizetype>(samples.size(), kMaxFramesPerPacket);
    out.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
      QStringList row = scalars;
      row.append(jsonScalar(samples.at(i)));
      out.append(std::move(row));
    }

    return out;
  }

private:
  QStringList m_scalarFields;
  QString m_vectorField;
};

/**
 * @brief Descriptor for the batched sensor data template.
 */
class BatchedSensorTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("batched_sensor_data"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override
  {
    return trNativeTemplate("Batched sensor data (multi-frame)");
  }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Expands a JSON packet holding scalar metadata plus a sample array "
                            "into one frame per sample, repeating the metadata.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto scalars = DataModel::nativeParam(
      "scalarFields",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Scalar fields"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated JSON fields repeated in every generated frame."),
      QStringLiteral("device_id,battery,temperature"));

    auto vector = DataModel::nativeParam(
      "vectorField",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Sample array field"),
      QT_TRANSLATE_NOOP("NativeTemplates", "JSON field holding the batched sample array."),
      QStringLiteral("samples"));

    return {scalars, vector};
  }

  /**
   * @brief Validates the vector field and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto scalars = DataModel::nativeKeyList(
      params, QStringLiteral("scalarFields"), QStringLiteral("device_id,battery,temperature"));

    const QString vector =
      DataModel::nativeParamString(params, QStringLiteral("vectorField"), QStringLiteral("samples"))
        .trimmed();
    if (vector.isEmpty()) {
      error = trNativeTemplate("The sample array field must not be empty.");
      return nullptr;
    }

    return std::make_unique<BatchedSensorParser>(scalars, vector);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide batched sensor data template descriptor.
 */
const DataModel::INativeTemplate& DataModel::batchedSensorTemplate()
{
  static const BatchedSensorTemplate s_batched;
  return s_batched;
}
