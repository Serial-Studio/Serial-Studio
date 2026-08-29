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

#include "DataModel/Scripting/NativeTemplates/MultiFrameTimeSeries2d.h"

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
    SS_ASSERT_LOG(!m_recordsField.isEmpty());
    SS_ASSERT_LOG(!m_fields.isEmpty());
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
    SS_ASSERT(!m_fields.isEmpty(), return {});

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
    return trNativeTemplate("Time-series 2D arrays (multi-frame)");
  }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Expands a JSON packet holding an array of records into one frame "
                            "per record, ordering values by the field list.");
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
      error = trNativeTemplate("The records array field must not be empty.");
      return nullptr;
    }

    const auto fields = DataModel::nativeKeyList(
      params, QStringLiteral("fields"), QStringLiteral("timestamp,temp,humidity"));
    if (fields.isEmpty()) {
      error = trNativeTemplate("At least one record field is required.");
      return nullptr;
    }

    return std::make_unique<TimeSeries2dParser>(records, fields);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide time-series 2D template descriptor.
 */
const DataModel::INativeTemplate& DataModel::timeSeries2dTemplate()
{
  static const TimeSeries2dTemplate s_timeSeries;
  return s_timeSeries;
}
