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

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "SerialStudio.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;

static constexpr int kMaxFramesPerPacket = 10000;

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
[[nodiscard]] static QString trMulti(const char* text)
{
  return QCoreApplication::translate("NativeTemplates", text);
}

/**
 * @brief Converts a scalar JSON value to its channel string ("0" when absent or non-scalar).
 */
[[nodiscard]] static QString jsonScalar(const QJsonValue& value)
{
  if (value.isDouble())
    return QString::number(SerialStudio::toDouble(value));

  if (value.isString())
    return value.toString();

  if (value.isBool())
    return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");

  return QStringLiteral("0");
}

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
    Q_ASSERT(!m_vectorField.isEmpty());
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
    Q_ASSERT(!m_vectorField.isEmpty());

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
    return trMulti("Batched sensor data (multi-frame)");
  }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trMulti("Expands a JSON packet holding scalar metadata plus a sample array into one "
                   "frame per sample, repeating the metadata.");
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
      error = trMulti("The sample array field must not be empty.");
      return nullptr;
    }

    return std::make_unique<BatchedSensorParser>(scalars, vector);
  }
};

//--------------------------------------------------------------------------------------------------
// Time-series 2D arrays
//--------------------------------------------------------------------------------------------------

/**
 * @brief Expands a JSON packet holding an array of records into one frame per record.
 */
class TimeSeries2dParser final : public INativeParser {
public:
  /**
   * @brief Stores the records field name and the per-record field order.
   */
  TimeSeries2dParser(const QString& recordsField, const QStringList& fields)
    : m_recordsField(recordsField), m_fields(fields)
  {
    Q_ASSERT(!m_recordsField.isEmpty());
    Q_ASSERT(!m_fields.isEmpty());
  }

  /**
   * @brief Emits one frame per record, ordering values by the configured field list.
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
      return {};

    const auto records = doc.object().value(m_recordsField).toArray();

    QList<QStringList> out;
    const qsizetype count = qMin<qsizetype>(records.size(), kMaxFramesPerPacket);
    out.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
      const auto record = records.at(i).toObject();

      QStringList row;
      row.reserve(m_fields.size());
      for (const auto& field : m_fields)
        row.append(jsonScalar(record.value(field)));

      out.append(std::move(row));
    }

    return out;
  }

private:
  QString m_recordsField;
  QStringList m_fields;
};

/**
 * @brief Descriptor for the time-series 2D template.
 */
class TimeSeries2dTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("time_series_2d"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override
  {
    return trMulti("Time-series 2D arrays (multi-frame)");
  }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trMulti("Expands a JSON packet holding an array of records into one frame per "
                   "record, ordering values by the field list.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto records = DataModel::nativeParam(
      "recordsField",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Records array field"),
      QT_TRANSLATE_NOOP("NativeTemplates", "JSON field holding the array of record objects."),
      QStringLiteral("records"));

    auto fields = DataModel::nativeParam(
      "fields",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Record fields (in channel order)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated record fields. The position of each field sets its "
                        "channel index."),
      QStringLiteral("timestamp,temp,humidity"));

    return {records, fields};
  }

  /**
   * @brief Validates the configuration and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString records = DataModel::nativeParamString(
                              params, QStringLiteral("recordsField"), QStringLiteral("records"))
                              .trimmed();
    if (records.isEmpty()) {
      error = trMulti("The records array field must not be empty.");
      return nullptr;
    }

    const auto fields = DataModel::nativeKeyList(
      params, QStringLiteral("fields"), QStringLiteral("timestamp,temp,humidity"));
    if (fields.isEmpty()) {
      error = trMulti("At least one record field is required.");
      return nullptr;
    }

    return std::make_unique<TimeSeries2dParser>(records, fields);
  }
};

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the multi-frame native templates in display order.
 */
QList<const DataModel::INativeTemplate*> DataModel::multiFrameNativeTemplates()
{
  static const BatchedSensorTemplate s_batched;
  static const TimeSeries2dTemplate s_timeSeries;

  return {&s_batched, &s_timeSeries};
}
