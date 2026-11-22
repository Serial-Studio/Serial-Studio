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

#include "DataModel/Scripting/NativeTemplates/BinaryJustFloat.h"

#include <array>

#include "Core/DataModel/ValueDecoder.h"
#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Frame shape
//--------------------------------------------------------------------------------------------------

static constexpr qsizetype kFloatBytes                          = 4;
static constexpr char kMarker[DataModel::kJustFloatMarkerBytes] = {'\x00', '\x00', '\x80', '\x7F'};

/**
 * @brief Whether @p frame still carries the end marker as its last four bytes.
 */
bool DataModel::endsWithJustFloatMarker(const QByteArrayView frame) noexcept
{
  SS_ASSERT(frame.size() >= 0, return false);
  if (frame.size() < kJustFloatMarkerBytes)
    return false;

  return frame.last(kJustFloatMarkerBytes) == QByteArrayView(kMarker, kJustFloatMarkerBytes);
}

/**
 * @brief Channels @p frame carries, or -1 when it is empty, holds a partial float or exceeds the
 *        channel cap. With @p stripMarker a trailing end marker is not counted as a value.
 */
qsizetype DataModel::justFloatChannelCount(const QByteArrayView frame,
                                           const bool stripMarker) noexcept
{
  SS_ASSERT(frame.size() >= 0, return -1);
  qsizetype bytes = frame.size();
  if (stripMarker && endsWithJustFloatMarker(frame))
    bytes -= kJustFloatMarkerBytes;

  if (bytes <= 0 || (bytes & (kFloatBytes - 1)) != 0)
    return -1;

  const qsizetype count = bytes >> 2;
  return count <= kMaxPackedFields ? count : -1;
}

//--------------------------------------------------------------------------------------------------
// JustFloat parser
//--------------------------------------------------------------------------------------------------

/**
 * @brief One channel per little-endian float (requires the Binary decoder). The channel count
 *        follows the frame, so the layout is rebuilt only when a frame's length changes.
 */
class JustFloatParser final : public INativeParser {
public:
  /**
   * @brief Stores whether a trailing end marker is removed before decoding.
   */
  explicit JustFloatParser(const bool stripMarker) : m_stripMarker(stripMarker), m_layout() {}

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
   * @brief Emits one row with every float rendered as the shortest round-trip decimal; a frame
   *        holding a partial float yields no row. The payload is a view over the frame's own
   *        bytes, so the marker is skipped without copying.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    const auto count = DataModel::justFloatChannelCount(frame, m_stripMarker);
    if (count <= 0)
      return {};

    if (m_layout.count != count)
      resize(static_cast<int>(count));

    const auto payload = QByteArray::fromRawData(frame.constData(), count * kFloatBytes);
    std::array<double, DataModel::kMaxPackedFields> values;
    const auto decoded =
      DataModel::decodePackedStruct(payload, m_layout, values.data(), values.size());
    SS_ASSERT(decoded == count, return {});

    QStringList row;
    row.reserve(decoded);
    for (qsizetype i = 0; i < decoded; ++i) {
      QString text;
      DataModel::render_number_in_place(text, values[static_cast<size_t>(i)]);
      row.append(std::move(text));
    }

    return singleFrame(std::move(row));
  }

private:
  /**
   * @brief Rebuilds the layout as @p count little-endian floats.
   */
  void resize(const int count)
  {
    SS_ASSERT(count > 0, return);
    SS_ASSERT(count <= DataModel::kMaxPackedFields, return);

    m_layout.count     = count;
    m_layout.bytes     = count * static_cast<int>(kFloatBytes);
    m_layout.bigEndian = false;
    for (int i = 0; i < count; ++i)
      m_layout.types[static_cast<size_t>(i)] = DataModel::PackedType::F32;
  }

private:
  bool m_stripMarker;
  DataModel::PackedLayout m_layout;
};

/**
 * @brief Descriptor for the JustFloat template.
 */
class JustFloatTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("justfloat"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("VOFA+ JustFloat"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a frame of little-endian 32-bit floats into one channel per "
                            "float. Set the frame end sequence to 00 00 80 7F (hexadecimal) and "
                            "use the Binary (Direct) decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto strip = DataModel::nativeParam(
      "strip_marker",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Remove end marker"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Drops a trailing 00 00 80 7F before decoding. Turn off only when the "
                        "last value of a frame can be positive infinity."),
      true);

    return {strip};
  }

  /**
   * @brief Builds a configured parser instance; the template has no invalid configuration.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool strip = DataModel::nativeParamBool(params, QStringLiteral("strip_marker"), true);
    return std::make_unique<JustFloatParser>(strip);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide JustFloat template descriptor.
 */
const DataModel::INativeTemplate& DataModel::justFloatTemplate()
{
  static const JustFloatTemplate s_justFloat;
  return s_justFloat;
}
