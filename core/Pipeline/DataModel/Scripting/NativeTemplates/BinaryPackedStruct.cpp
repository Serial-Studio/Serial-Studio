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

#include "DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iterator>

#include "Core/DataModel/ValueDecoder.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using DataModel::PackedLayout;
using DataModel::PackedType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Field types
//--------------------------------------------------------------------------------------------------

/**
 * @brief One field type's wire name and width, listed in PackedType order.
 */
struct FieldSpec {
  const char* name;
  PackedType type;
  int width;
};

static constexpr FieldSpec kFieldSpecs[] = {
  { "i8",  PackedType::I8, 1},
  { "u8",  PackedType::U8, 1},
  {"i16", PackedType::I16, 2},
  {"u16", PackedType::U16, 2},
  {"i32", PackedType::I32, 4},
  {"u32", PackedType::U32, 4},
  {"i64", PackedType::I64, 8},
  {"u64", PackedType::U64, 8},
  {"f32", PackedType::F32, 4},
  {"f64", PackedType::F64, 8},
};

/**
 * @brief The byte width of @p type.
 */
[[nodiscard]] static int widthOf(const PackedType type) noexcept
{
  const auto index = static_cast<size_t>(type);
  SS_ASSERT(index < std::size(kFieldSpecs), return 1);
  return kFieldSpecs[index].width;
}

/**
 * @brief The value a single-precision field reads as: the shortest decimal that names the float,
 *        so 1.2f shows as 1.2 and the saved project's text parse yields the same double.
 */
[[nodiscard]] static double floatAsDecimal(const float value) noexcept
{
  if (!std::isfinite(value))
    return static_cast<double>(value);

  char buf[32];
#ifdef SS_APPLE_NO_FLOAT_TO_CHARS
  for (int precision = 1; precision <= 9; ++precision) {
    const int len =
      snprintf_l(buf, sizeof(buf), nullptr, "%.*g", precision, static_cast<double>(value));
    if (len > 0 && len < static_cast<int>(sizeof(buf)) && strtof_l(buf, nullptr, nullptr) == value)
      return SerialStudio::toDouble(QByteArrayView(buf, len));
  }

  return static_cast<double>(value);
#else
  const auto res = std::to_chars(buf, buf + sizeof(buf), value);
  SS_ASSERT(res.ec == std::errc(), return static_cast<double>(value));
  return SerialStudio::toDouble(QByteArrayView(buf, static_cast<qsizetype>(res.ptr - buf)));
#endif
}

/**
 * @brief Reads one field of @p type at @p at; the caller checked the frame length first.
 */
[[nodiscard]] static double readField(const QByteArray& frame,
                                      const qsizetype at,
                                      const PackedType type,
                                      const bool be) noexcept
{
  switch (type) {
    case PackedType::I8:
      return static_cast<qint8>(u8At(frame, at));
    case PackedType::U8:
      return u8At(frame, at);
    case PackedType::I16:
      return be ? i16Be(frame, at) : i16Le(frame, at);
    case PackedType::U16:
      return be ? u16Be(frame, at) : u16Le(frame, at);
    case PackedType::I32:
      return be ? i32Be(frame, at) : i32Le(frame, at);
    case PackedType::U32:
      return be ? u32Be(frame, at) : u32Le(frame, at);
    case PackedType::I64:
      return static_cast<double>(static_cast<qint64>(be ? u64Be(frame, at) : u64Le(frame, at)));
    case PackedType::U64:
      return static_cast<double>(be ? u64Be(frame, at) : u64Le(frame, at));
    case PackedType::F32:
      return floatAsDecimal(be ? f32Be(frame, at) : f32Le(frame, at));
    case PackedType::F64:
      return be ? f64Be(frame, at) : f64Le(frame, at);
  }

  return 0.0;
}

//--------------------------------------------------------------------------------------------------
// Layout and decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses the "fields" list (i8 to u64, f32, f64, comma separated) and the byte order into
 *        @p layout; fails with @p error on an unknown type, too many fields or another byte order.
 */
bool DataModel::parsePackedLayout(const QString& fields,
                                  const QString& endianness,
                                  PackedLayout& layout,
                                  QString& error)
{
  layout           = PackedLayout();
  const auto names = fields.split(QLatin1Char(','), Qt::SkipEmptyParts);
  if (names.isEmpty() || names.size() > kMaxPackedFields) {
    error = trNativeTemplate("A packed struct needs 1 to %1 fields.").arg(kMaxPackedFields);
    return false;
  }

  if (endianness != QLatin1String("little") && endianness != QLatin1String("big")) {
    error = trNativeTemplate("Endianness must be little or big.");
    return false;
  }

  for (const auto& name : names) {
    const auto key = name.trimmed().toLower();
    const auto* it =
      std::find_if(std::begin(kFieldSpecs), std::end(kFieldSpecs), [&key](const FieldSpec& s) {
        return key == QLatin1String(s.name);
      });
    if (it == std::end(kFieldSpecs)) {
      error = trNativeTemplate("Unknown field type \"%1\": use i8, u8, i16, u16, i32, u32, i64, "
                               "u64, f32 or f64.")
                .arg(key.left(16));
      return false;
    }

    layout.types[static_cast<size_t>(layout.count++)]  = it->type;
    layout.bytes                                      += it->width;
  }

  layout.bigEndian = endianness == QLatin1String("big");
  SS_ASSERT(layout.count == names.size(), return false);
  return true;
}

/**
 * @brief Decodes one packed frame into @p out, one double per field, and returns the field count;
 *        -1 when the frame length is not the layout's (a torn or foreign frame) or @p out is too
 *        small. Allocation-free: the template and the Quick Plot lane share it per frame.
 */
qsizetype DataModel::decodePackedStruct(const QByteArray& frame,
                                        const PackedLayout& layout,
                                        double* out,
                                        const qsizetype capacity) noexcept
{
  SS_ASSERT(out != nullptr, return -1);
  SS_ASSERT(layout.count >= 0 && layout.count <= kMaxPackedFields, return -1);
  if (layout.count == 0 || frame.size() != layout.bytes || capacity < layout.count)
    return -1;

  qsizetype offset = 0;
  for (int i = 0; i < layout.count; ++i) {
    const auto type  = layout.types[static_cast<size_t>(i)];
    out[i]           = readField(frame, offset, type, layout.bigEndian);
    offset          += widthOf(type);
  }

  return layout.count;
}

//--------------------------------------------------------------------------------------------------
// Packed struct parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief One channel per packed field (requires the Binary decoder); a wrong-length frame is
 *        dropped rather than read with shifted fields.
 */
class PackedStructParser final : public INativeParser {
public:
  /**
   * @brief Stores the parsed layout.
   */
  explicit PackedStructParser(const PackedLayout& layout) : m_layout(layout)
  {
    SS_ASSERT(m_layout.count > 0, m_layout.count = 0);
    SS_ASSERT(m_layout.count <= DataModel::kMaxPackedFields, m_layout.count = 0);
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
    std::array<double, DataModel::kMaxPackedFields> values;
    const auto count = DataModel::decodePackedStruct(frame, m_layout, values.data(), values.size());
    if (count <= 0)
      return {};

    QStringList row;
    row.reserve(count);
    for (qsizetype i = 0; i < count; ++i) {
      QString text;
      DataModel::render_number_in_place(text, values[static_cast<size_t>(i)]);
      row.append(std::move(text));
    }

    return singleFrame(std::move(row));
  }

private:
  PackedLayout m_layout;
};

/**
 * @brief Descriptor for the packed struct template.
 */
class PackedStructTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("packed_struct"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Packed struct"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a packed C struct of integer and floating-point fields into "
                            "one channel per field. A frame of another length is dropped. Use "
                            "with the Binary (Direct) decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto fields = DataModel::nativeParam(
      "fields",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Fields"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Field types in frame order, comma separated: i8, u8, i16, u16, i32, u32, "
                        "i64, u64, f32 or f64."),
      QStringLiteral("f32,f32,f32"));

    auto endian = DataModel::nativeParam(
      "endianness",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Endianness"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Byte order of every multi-byte field."),
      QStringLiteral("little"));
    endian.optionValues = {QStringLiteral("little"), QStringLiteral("big")};
    endian.optionLabels = {trNativeTemplate("Little endian"), trNativeTemplate("Big endian")};

    return {fields, endian};
  }

  /**
   * @brief Validates the layout and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto fields =
      DataModel::nativeParamString(params, QStringLiteral("fields"), QStringLiteral("f32,f32,f32"));
    const auto endian =
      DataModel::nativeParamString(params, QStringLiteral("endianness"), QStringLiteral("little"));

    PackedLayout layout;
    if (!DataModel::parsePackedLayout(fields, endian, layout, error))
      return nullptr;

    return std::make_unique<PackedStructParser>(layout);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide packed struct template descriptor.
 */
const DataModel::INativeTemplate& DataModel::packedStructTemplate()
{
  static const PackedStructTemplate s_packedStruct;
  return s_packedStruct;
}
