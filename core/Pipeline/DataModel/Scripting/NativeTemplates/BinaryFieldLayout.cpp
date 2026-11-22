/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "DataModel/Scripting/NativeTemplates/BinaryFieldLayout.h"

#include <array>
#include <bit>
#include <cmath>
#include <limits>
#include <QJsonObject>
#include <utility>

#include "Core/DataModel/ValueDecoder.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::FieldLayout;
using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::LayoutField;
using DataModel::LayoutFieldType;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Bit extraction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads @p field's bits most significant first, counting the start bit from the top of
 *        its byte; the caller checked that the field ends inside @p frame.
 */
[[nodiscard]] static quint64 extractBigEndian(const QByteArrayView frame,
                                              const LayoutField& field) noexcept
{
  SS_ASSERT(field.bits >= 1 && field.bits <= 64, return 0);
  SS_ASSERT(field.startBit >= 0, return 0);

  quint64 value = 0;
  for (qint32 i = 0; i < field.bits; ++i) {
    const qint32 position = field.startBit + i;
    const auto byte       = static_cast<quint8>(frame.at(position >> 3));
    const quint64 bit     = (byte >> (7 - (position & 7))) & 0x01U;
    value                 = (value << 1) | bit;
  }

  return value;
}

/**
 * @brief Reads @p field's bits least significant first, counting the start bit from the bottom
 *        of its byte; the caller checked that the field ends inside @p frame.
 */
[[nodiscard]] static quint64 extractLittleEndian(const QByteArrayView frame,
                                                 const LayoutField& field) noexcept
{
  SS_ASSERT(field.bits >= 1 && field.bits <= 64, return 0);
  SS_ASSERT(field.startBit >= 0, return 0);

  quint64 value   = 0;
  qint32 read     = 0;
  qint32 shift    = field.startBit & 7;
  qsizetype index = field.startBit >> 3;
  while (read < field.bits) {
    const qint32 take   = qMin(8 - shift, field.bits - read);
    const quint64 mask  = (quint64(1) << take) - 1;
    const auto byte     = static_cast<quint8>(frame.at(index));
    value              |= ((static_cast<quint64>(byte) >> shift) & mask) << read;
    read               += take;
    shift               = 0;
    ++index;
  }

  return value;
}

/**
 * @brief Turns the raw bits of @p field into its number before scaling: sign extension for a
 *        signed field narrower than 64 bits, the IEEE image for a floating-point one.
 */
[[nodiscard]] static double interpretBits(const quint64 raw, const LayoutField& field) noexcept
{
  SS_ASSERT(field.bits >= 1 && field.bits <= 64, return 0.0);

  switch (field.type) {
    case LayoutFieldType::Float32:
      return DataModel::packedFloatAsDecimal(std::bit_cast<float>(static_cast<quint32>(raw)));
    case LayoutFieldType::Float64:
      return std::bit_cast<double>(raw);
    case LayoutFieldType::Unsigned:
      return static_cast<double>(raw);
    case LayoutFieldType::Signed:
      break;
  }

  if (field.bits < 64 && (raw & (quint64(1) << (field.bits - 1))) != 0)
    return static_cast<double>(static_cast<qint64>(raw | (~quint64(0) << field.bits)));

  return static_cast<double>(static_cast<qint64>(raw));
}

/**
 * @brief Decodes every field of @p layout into @p out and returns the field count, or -1 when
 *        @p out is too small. A field that reaches past the end of @p frame decodes as NaN and
 *        the others are unaffected, so one short frame never shifts or drops a channel.
 */
qsizetype DataModel::decodeFieldLayout(const QByteArrayView frame,
                                       const FieldLayout& layout,
                                       double* out,
                                       const qsizetype capacity) noexcept
{
  SS_ASSERT(out != nullptr, return -1);
  const auto count = static_cast<qsizetype>(layout.fields.size());
  SS_ASSERT(count <= kMaxLayoutFields, return -1);
  if (capacity < count)
    return -1;

  const qint64 frame_bits = static_cast<qint64>(frame.size()) * 8;
  for (qsizetype i = 0; i < count; ++i) {
    const auto& field = layout.fields[static_cast<size_t>(i)];
    if (static_cast<qint64>(field.startBit) + field.bits > frame_bits) {
      out[i] = std::numeric_limits<double>::quiet_NaN();
      continue;
    }

    const quint64 raw =
      field.bigEndian ? extractBigEndian(frame, field) : extractLittleEndian(frame, field);
    out[i] = interpretBits(raw, field) * field.scale + field.add;
  }

  return count;
}

//--------------------------------------------------------------------------------------------------
// Layout parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a field's "type" text to its enum; false for an unknown name.
 */
[[nodiscard]] static bool parseFieldType(const QString& name, LayoutFieldType& type)
{
  static const std::array<std::pair<QLatin1StringView, LayoutFieldType>, 4> kTypes = {
    {
     {QLatin1StringView("uint"), LayoutFieldType::Unsigned},
     {QLatin1StringView("int"), LayoutFieldType::Signed},
     {QLatin1StringView("f32"), LayoutFieldType::Float32},
     {QLatin1StringView("f64"), LayoutFieldType::Float64},
     }
  };

  SS_ASSERT(!kTypes.empty(), return false);
  for (const auto& entry : kTypes) {
    if (name == entry.first) {
      type = entry.second;
      return true;
    }
  }

  return false;
}

/**
 * @brief Parses one field object; a floating-point field takes its width from its type, so a
 *        stale "length" left behind by a type change cannot produce a 12-bit float.
 */
[[nodiscard]] static bool parseField(const QJsonObject& obj, LayoutField& field, QString& error)
{
  const auto type_name = obj.value(QStringLiteral("type")).toString(QStringLiteral("uint"));
  if (!parseFieldType(type_name.trimmed().toLower(), field.type)) {
    error = trNativeTemplate("Unknown field type \"%1\": use uint, int, f32 or f64.")
              .arg(type_name.left(16));
    return false;
  }

  const int offset = obj.value(QStringLiteral("offset")).toInt(0);
  const int bit    = obj.value(QStringLiteral("bit")).toInt(0);
  int length       = obj.value(QStringLiteral("length")).toInt(8);
  if (field.type == LayoutFieldType::Float32)
    length = 32;

  if (field.type == LayoutFieldType::Float64)
    length = 64;

  if (offset < 0 || offset > DataModel::kMaxLayoutOffset || bit < 0 || bit > 7) {
    error = trNativeTemplate("A field starts at byte 0 to %1 and bit 0 to 7.")
              .arg(DataModel::kMaxLayoutOffset);
    return false;
  }

  if (length < 1 || length > 64) {
    error = trNativeTemplate("A field is 1 to 64 bits long.");
    return false;
  }

  const auto endian = obj.value(QStringLiteral("endian")).toString(QStringLiteral("little"));
  field.startBit    = offset * 8 + bit;
  field.bits        = length;
  field.bigEndian   = endian == QLatin1String("big");
  field.scale       = SerialStudio::toDouble(obj.value(QStringLiteral("scale")), 1.0);
  field.add         = SerialStudio::toDouble(obj.value(QStringLiteral("add")), 0.0);
  if (!std::isfinite(field.scale) || !std::isfinite(field.add)) {
    error = trNativeTemplate("Scale and offset must be finite numbers.");
    return false;
  }

  return true;
}

/**
 * @brief Parses the "fields" table into @p layout; fails with @p error on an empty table, too
 *        many fields or the first field that is not valid.
 */
bool DataModel::parseFieldLayout(const QJsonArray& json, FieldLayout& layout, QString& error)
{
  layout.fields.clear();
  if (json.isEmpty() || json.size() > kMaxLayoutFields) {
    error = trNativeTemplate("A field layout needs 1 to %1 fields.").arg(kMaxLayoutFields);
    return false;
  }

  layout.fields.reserve(static_cast<size_t>(json.size()));
  for (const auto& entry : json) {
    LayoutField field;
    if (!entry.isObject() || !parseField(entry.toObject(), field, error)) {
      if (error.isEmpty())
        error = trNativeTemplate("Every field must be an object.");

      layout.fields.clear();
      return false;
    }

    layout.fields.push_back(field);
  }

  SS_ASSERT(static_cast<qsizetype>(layout.fields.size()) == json.size(), return false);
  return true;
}

/**
 * @brief The table a new Field Layout source starts with: one unsigned 16-bit value followed by
 *        one float.
 */
QJsonArray DataModel::defaultFieldLayout()
{
  QJsonObject word;
  word.insert(QStringLiteral("offset"), 0);
  word.insert(QStringLiteral("length"), 16);
  word.insert(QStringLiteral("type"), QStringLiteral("uint"));
  word.insert(QStringLiteral("endian"), QStringLiteral("little"));

  QJsonObject real;
  real.insert(QStringLiteral("offset"), 2);
  real.insert(QStringLiteral("length"), 32);
  real.insert(QStringLiteral("type"), QStringLiteral("f32"));
  real.insert(QStringLiteral("endian"), QStringLiteral("little"));

  return QJsonArray{word, real};
}

//--------------------------------------------------------------------------------------------------
// Field layout parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief One channel per declared field (requires the Binary decoder); a field the frame does
 *        not reach is an empty value.
 */
class FieldLayoutParser final : public INativeParser {
public:
  /**
   * @brief Takes ownership of the parsed layout.
   */
  explicit FieldLayoutParser(FieldLayout&& layout) : m_layout(std::move(layout))
  {
    SS_ASSERT(!m_layout.fields.empty(), m_layout.fields.clear());
    SS_ASSERT(m_layout.fields.size() <= static_cast<size_t>(DataModel::kMaxLayoutFields),
              m_layout.fields.clear());
  }

  /**
   * @brief Decodes the UTF-8 bytes of a text frame.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseBinary(frame.toUtf8());
  }

  /**
   * @brief UTF-8 text frames already carry the raw bytes; skips the QString round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    return parseBinary(frame);
  }

  /**
   * @brief Emits one row with every field rendered as the shortest round-trip decimal.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    std::array<double, DataModel::kMaxLayoutFields> values;
    const auto count = DataModel::decodeFieldLayout(frame, m_layout, values.data(), values.size());
    if (count <= 0 || frame.isEmpty())
      return {};

    QStringList row;
    row.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
      QString text;
      const double value = values[static_cast<size_t>(i)];
      if (!std::isnan(value))
        DataModel::render_number_in_place(text, value);

      row.append(std::move(text));
    }

    return singleFrame(std::move(row));
  }

private:
  FieldLayout m_layout;
};

/**
 * @brief Descriptor for the field layout template.
 */
class FieldLayoutTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("field_layout"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Field layout"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a binary frame field by field: each field has its own "
                            "position, width, type, byte order and scaling. Use with the Binary "
                            "(Direct) decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto fields = DataModel::nativeParam(
      "fields",
      NativeParamType::Json,
      QT_TRANSLATE_NOOP("NativeTemplates", "Fields"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "One entry per channel: byte offset, bit offset, bit length, type, byte "
                        "order, scale and offset."),
      DataModel::defaultFieldLayout());

    return {fields};
  }

  /**
   * @brief Validates the field table and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    auto json = DataModel::nativeParamArray(params, QStringLiteral("fields"));
    if (!params.contains(QStringLiteral("fields")))
      json = DataModel::defaultFieldLayout();

    FieldLayout layout;
    if (!DataModel::parseFieldLayout(json, layout, error))
      return nullptr;

    return std::make_unique<FieldLayoutParser>(std::move(layout));
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide field layout template descriptor.
 */
const DataModel::INativeTemplate& DataModel::fieldLayoutTemplate()
{
  static const FieldLayoutTemplate s_fieldLayout;
  return s_fieldLayout;
}
