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

#include <bit>
#include <QCoreApplication>
#include <QHash>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;

static constexpr int kMaxBytesPerFrame = 65536;

//--------------------------------------------------------------------------------------------------
// Shared binary helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
[[nodiscard]] static QString trBinary(const char* text)
{
  return QCoreApplication::translate("NativeTemplates", text);
}

/**
 * @brief Returns the unsigned byte at index i.
 */
[[nodiscard]] static quint8 u8At(const QByteArray& data, qsizetype i)
{
  Q_ASSERT(i >= 0);
  Q_ASSERT(i < data.size());

  return static_cast<quint8>(data.at(i));
}

/**
 * @brief Reads a 16-bit unsigned little-endian value at the offset.
 */
[[nodiscard]] static quint16 u16Le(const QByteArray& data, qsizetype i)
{
  return static_cast<quint16>(u8At(data, i) | (u8At(data, i + 1) << 8));
}

/**
 * @brief Reads a 16-bit signed little-endian value at the offset.
 */
[[nodiscard]] static qint16 i16Le(const QByteArray& data, qsizetype i)
{
  return static_cast<qint16>(u16Le(data, i));
}

/**
 * @brief Reads a 32-bit unsigned little-endian value at the offset.
 */
[[nodiscard]] static quint32 u32Le(const QByteArray& data, qsizetype i)
{
  return static_cast<quint32>(u8At(data, i)) | (static_cast<quint32>(u8At(data, i + 1)) << 8)
       | (static_cast<quint32>(u8At(data, i + 2)) << 16)
       | (static_cast<quint32>(u8At(data, i + 3)) << 24);
}

/**
 * @brief Reads a 32-bit signed little-endian value at the offset.
 */
[[nodiscard]] static qint32 i32Le(const QByteArray& data, qsizetype i)
{
  return static_cast<qint32>(u32Le(data, i));
}

/**
 * @brief Reads a 16-bit unsigned big-endian value at the offset.
 */
[[nodiscard]] static quint16 u16Be(const QByteArray& data, qsizetype i)
{
  return static_cast<quint16>((u8At(data, i) << 8) | u8At(data, i + 1));
}

/**
 * @brief Reads a 16-bit signed big-endian value at the offset.
 */
[[nodiscard]] static qint16 i16Be(const QByteArray& data, qsizetype i)
{
  return static_cast<qint16>(u16Be(data, i));
}

/**
 * @brief Reads a 32-bit unsigned big-endian value at the offset.
 */
[[nodiscard]] static quint32 u32Be(const QByteArray& data, qsizetype i)
{
  return (static_cast<quint32>(u8At(data, i)) << 24)
       | (static_cast<quint32>(u8At(data, i + 1)) << 16)
       | (static_cast<quint32>(u8At(data, i + 2)) << 8) | static_cast<quint32>(u8At(data, i + 3));
}

/**
 * @brief Reads a 32-bit signed big-endian value at the offset.
 */
[[nodiscard]] static qint32 i32Be(const QByteArray& data, qsizetype i)
{
  return static_cast<qint32>(u32Be(data, i));
}

/**
 * @brief Reads a 32-bit IEEE-754 float (little-endian) at the offset.
 */
[[nodiscard]] static float f32Le(const QByteArray& data, qsizetype i)
{
  return std::bit_cast<float>(u32Le(data, i));
}

/**
 * @brief Reads a 32-bit IEEE-754 float (big-endian) at the offset.
 */
[[nodiscard]] static float f32Be(const QByteArray& data, qsizetype i)
{
  return std::bit_cast<float>(u32Be(data, i));
}

/**
 * @brief Converts every byte into one decimal channel value.
 */
[[nodiscard]] static QList<QStringList> byteRowFrame(const QByteArray& bytes)
{
  Q_ASSERT(bytes.size() <= kMaxBytesPerFrame);

  QStringList row;
  row.reserve(bytes.size());
  const qsizetype count = qMin<qsizetype>(bytes.size(), kMaxBytesPerFrame);
  for (qsizetype i = 0; i < count; ++i)
    row.append(QString::number(u8At(bytes, i)));

  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

/**
 * @brief Combines one byte group at the offset into a decimal value string.
 */
[[nodiscard]] static QString byteGroupValue(
  const QByteArray& bytes, qsizetype offset, int bytesPerValue, bool bigEndian, bool signedValues)
{
  Q_ASSERT(bytesPerValue >= 1);
  Q_ASSERT(bytesPerValue <= 8);
  Q_ASSERT(offset + bytesPerValue <= bytes.size());

  quint64 value = 0;
  for (int b = 0; b < bytesPerValue; ++b) {
    const qsizetype idx = bigEndian ? (offset + b) : (offset + bytesPerValue - 1 - b);
    value               = (value << 8) | u8At(bytes, idx);
  }

  if (!signedValues)
    return QString::number(value);

  const int bits         = bytesPerValue * 8;
  const quint64 sign_bit = quint64(1) << (bits - 1);
  if (bits < 64 && (value & sign_bit))
    return QString::number(static_cast<qint64>(value) - (qint64(1) << bits));

  return QString::number(static_cast<qint64>(value));
}

/**
 * @brief Groups frame bytes into one channel per group; trailing partial groups are dropped.
 */
[[nodiscard]] static QList<QStringList> groupedByteFrame(const QByteArray& bytes,
                                                         int bytesPerValue,
                                                         bool bigEndian,
                                                         bool signedValues)
{
  Q_ASSERT(bytesPerValue >= 1);

  QStringList row;
  row.reserve(bytes.size() / bytesPerValue + 1);

  const qsizetype count = qMin<qsizetype>(bytes.size(), kMaxBytesPerFrame);
  for (qsizetype i = 0; i + bytesPerValue <= count; i += bytesPerValue)
    row.append(byteGroupValue(bytes, i, bytesPerValue, bigEndian, signedValues));

  QList<QStringList> out;
  out.append(std::move(row));
  return out;
}

//--------------------------------------------------------------------------------------------------
// Raw bytes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Groups raw frame bytes into numeric channels (requires the Binary decoder).
 */
class RawBytesParser final : public INativeParser {
public:
  /**
   * @brief Stores the grouping, endianness and sign configuration.
   */
  RawBytesParser(int bytesPerValue, bool bigEndian, bool signedValues)
    : m_bytesPerValue(bytesPerValue), m_bigEndian(bigEndian), m_signedValues(signedValues)
  {
    Q_ASSERT(m_bytesPerValue >= 1);
    Q_ASSERT(m_bytesPerValue <= 8);
  }

  /**
   * @brief Groups the UTF-8 bytes of a text frame.
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
   * @brief Emits one channel per byte group.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!frame.isEmpty());

    return groupedByteFrame(frame, m_bytesPerValue, m_bigEndian, m_signedValues);
  }

private:
  int m_bytesPerValue;
  bool m_bigEndian;
  bool m_signedValues;
};

/**
 * @brief Descriptor for the raw bytes template.
 */
class RawBytesTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("raw_bytes"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("Raw bytes"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Groups raw frame bytes into numeric channels, with configurable grouping, "
                    "endianness and sign. Use with the Binary (Direct) decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto group = DataModel::nativeParam(
      "bytesPerValue",
      NativeParamType::Int,
      QT_TRANSLATE_NOOP("NativeTemplates", "Bytes per value"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Number of bytes combined into each channel value."),
      1);
    group.minValue = 1;
    group.maxValue = 8;

    auto endian = DataModel::nativeParam(
      "endianness",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Endianness"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Byte order used when combining multi-byte values."),
      QStringLiteral("big"));
    endian.optionValues = {QStringLiteral("big"), QStringLiteral("little")};
    endian.optionLabels = {trBinary("Big endian"), trBinary("Little endian")};

    auto sign = DataModel::nativeParam(
      "signedValues",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Signed values"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Interprets each value as two's-complement signed."),
      false);

    return {group, endian, sign};
  }

  /**
   * @brief Validates the grouping and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int group = DataModel::nativeParamInt(params, QStringLiteral("bytesPerValue"), 1);
    if (group < 1 || group > 8) {
      error = trBinary("Bytes per value must be between 1 and 8.");
      return nullptr;
    }

    const QString endian =
      DataModel::nativeParamString(params, QStringLiteral("endianness"), QStringLiteral("big"));
    const bool sign = DataModel::nativeParamBool(params, QStringLiteral("signedValues"), false);
    return std::make_unique<RawBytesParser>(group, endian != QStringLiteral("little"), sign);
  }
};

//--------------------------------------------------------------------------------------------------
// Hexadecimal bytes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a hex string into grouped numeric channels (requires the Hexadecimal decoder).
 */
class HexBytesParser final : public INativeParser {
public:
  /**
   * @brief Stores the grouping, endianness and sign configuration.
   */
  HexBytesParser(int bytesPerValue, bool bigEndian, bool signedValues)
    : m_bytesPerValue(bytesPerValue), m_bigEndian(bigEndian), m_signedValues(signedValues)
  {
    Q_ASSERT(m_bytesPerValue >= 1);
    Q_ASSERT(m_bytesPerValue <= 8);
  }

  /**
   * @brief Parses the hex string into byte groups and emits one channel per group.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(m_bytesPerValue >= 1);

    QString hex = frame;
    hex.remove(QLatin1Char(' '));
    hex.remove(QLatin1Char('\t'));

    QByteArray bytes = QByteArray::fromHex(hex.toLatin1());
    return parseBinary(bytes);
  }

  /**
   * @brief Byte-level twin of parseText: strips whitespace and hex-decodes without QString.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    QByteArray hex;
    hex.reserve(frame.size());
    for (const char c : frame)
      if (c != ' ' && c != '\t')
        hex.append(c);

    return parseBinary(QByteArray::fromHex(hex));
  }

  /**
   * @brief Emits grouped values from raw bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return groupedByteFrame(frame, m_bytesPerValue, m_bigEndian, m_signedValues);
  }

private:
  int m_bytesPerValue;
  bool m_bigEndian;
  bool m_signedValues;
};

/**
 * @brief Descriptor for the hexadecimal bytes template.
 */
class HexBytesTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("hexadecimal_bytes"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("Hexadecimal bytes"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes a hex string into numeric channels, with configurable grouping, "
                    "endianness and sign. Use with the Hexadecimal decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto group = DataModel::nativeParam(
      "bytesPerValue",
      NativeParamType::Int,
      QT_TRANSLATE_NOOP("NativeTemplates", "Bytes per value"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Number of bytes combined into each channel value."),
      1);
    group.minValue = 1;
    group.maxValue = 8;

    auto endian = DataModel::nativeParam(
      "endianness",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Endianness"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Byte order used when combining multi-byte values."),
      QStringLiteral("big"));
    endian.optionValues = {QStringLiteral("big"), QStringLiteral("little")};
    endian.optionLabels = {trBinary("Big endian"), trBinary("Little endian")};

    auto sign = DataModel::nativeParam(
      "signedValues",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Signed values"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Interprets each value as two's-complement signed."),
      false);

    return {group, endian, sign};
  }

  /**
   * @brief Validates the grouping and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int group = DataModel::nativeParamInt(params, QStringLiteral("bytesPerValue"), 1);
    if (group < 1 || group > 8) {
      error = trBinary("Bytes per value must be between 1 and 8.");
      return nullptr;
    }

    const QString endian =
      DataModel::nativeParamString(params, QStringLiteral("endianness"), QStringLiteral("big"));
    const bool sign = DataModel::nativeParamBool(params, QStringLiteral("signedValues"), false);
    return std::make_unique<HexBytesParser>(group, endian != QStringLiteral("little"), sign);
  }
};

//--------------------------------------------------------------------------------------------------
// Base64-encoded data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a Base64 string into one decimal channel per byte.
 */
class Base64Parser final : public INativeParser {
public:
  /**
   * @brief Decodes the Base64 payload and emits one channel per byte.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    Q_ASSERT(!frame.isEmpty());

    const auto decoded = QByteArray::fromBase64(frame.trimmed().toLatin1());
    if (decoded.isEmpty())
      return {};

    return byteRowFrame(decoded);
  }

  /**
   * @brief Treats binary frames as the Base64 text itself.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromLatin1(frame));
  }

  /**
   * @brief UTF-8 text frames are the Base64 bytes themselves (ASCII); skips the round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    const auto decoded = QByteArray::fromBase64(frame.trimmed());
    if (decoded.isEmpty())
      return {};

    return byteRowFrame(decoded);
  }
};

/**
 * @brief Descriptor for the Base64 template.
 */
class Base64Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("base64_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("Base64-encoded data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes a Base64 payload into one decimal channel per byte. Use with the "
                    "Base64 decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<Base64Parser>();
  }
};

//--------------------------------------------------------------------------------------------------
// Binary TLV
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching TLV extractor: 1-byte tag, 1-byte length, big-endian value.
 */
class BinaryTlvParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the tag-to-channel routing table.
   */
  BinaryTlvParser(const QHash<int, int>& tagMap, int count)
    : NativeLatchParser(count), m_tagMap(tagMap)
  {
    Q_ASSERT(!m_tagMap.isEmpty());
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the binary path.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseBinary(frame.toUtf8());
  }

  /**
   * @brief Walks the TLV entries and updates the latched channels.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!m_tagMap.isEmpty());

    // Each entry consumes at least two bytes, so size/2 + 1 bounds the walk
    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    for (qsizetype entry = 0; entry < (size / 2) + 1 && i + 1 < size; ++entry) {
      const int tag    = u8At(frame, i++);
      const int length = u8At(frame, i++);
      if (i + length > size)
        break;

      // Big-endian accumulation, capped at 8 bytes to stay within 64 bits
      quint64 value       = 0;
      const int use_bytes = qMin(length, 8);
      for (int b = 0; b < use_bytes; ++b)
        value = (value << 8) | u8At(frame, i + b);

      i += length;

      const auto it = m_tagMap.constFind(tag);
      if (it != m_tagMap.constEnd())
        storeAt(it.value(), QString::number(value));
    }

    return latchedFrame();
  }

private:
  QHash<int, int> m_tagMap;
};

/**
 * @brief Descriptor for the binary TLV template.
 */
class BinaryTlvTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("binary_tlv"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("Binary TLV (Tag-Length-Value)"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Walks tag/length/value entries (1-byte tag and length, big-endian value) "
                    "and routes tags to channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto tags = DataModel::nativeParam(
      "tagMap",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Tag routing table"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be "
                        "decimal or 0x-prefixed hex."),
      QStringLiteral("1:0,2:1,3:2,4:3,5:4"));

    return {tags};
  }

  /**
   * @brief Parses the routing table and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const auto entries = DataModel::nativeKeyList(
      params, QStringLiteral("tagMap"), QStringLiteral("1:0,2:1,3:2,4:3,5:4"));

    QHash<int, int> tag_map;
    int max_index = -1;
    for (const auto& entry : entries) {
      const qsizetype colon = entry.indexOf(QLatin1Char(':'));
      if (colon < 0)
        continue;

      bool tag_ok     = false;
      bool index_ok   = false;
      const int tag   = entry.left(colon).trimmed().toInt(&tag_ok, 0);
      const int index = entry.mid(colon + 1).trimmed().toInt(&index_ok);
      if (!tag_ok || !index_ok || tag < 0 || index < 0)
        continue;

      tag_map.insert(tag, index);
      max_index = qMax(max_index, index);
    }

    if (tag_map.isEmpty() || max_index < 0) {
      error = trBinary("The tag routing table must contain at least one tag:index entry.");
      return nullptr;
    }

    return std::make_unique<BinaryTlvParser>(tag_map, max_index + 1);
  }
};

//--------------------------------------------------------------------------------------------------
// COBS-encoded frames
//--------------------------------------------------------------------------------------------------

/**
 * @brief COBS decoder emitting one decimal channel per decoded byte.
 */
class CobsParser final : public INativeParser {
public:
  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes the COBS framing and emits the recovered bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!frame.isEmpty());

    QByteArray decoded;
    decoded.reserve(frame.size());

    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    while (i < size) {
      const int code = u8At(frame, i++);
      if (code == 0)
        break;

      for (int j = 1; j < code && i < size; ++j)
        decoded.append(static_cast<char>(u8At(frame, i++)));

      if (code < 0xFF && i < size)
        decoded.append('\0');
    }

    return byteRowFrame(decoded);
  }
};

/**
 * @brief Descriptor for the COBS template.
 */
class CobsTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("cobs_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("COBS-encoded frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Removes Consistent Overhead Byte Stuffing and emits the decoded bytes as "
                    "channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<CobsParser>();
  }
};

//--------------------------------------------------------------------------------------------------
// SLIP-encoded frames
//--------------------------------------------------------------------------------------------------

/**
 * @brief SLIP (RFC 1055) decoder emitting one decimal channel per decoded byte.
 */
class SlipParser final : public INativeParser {
public:
  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Resolves SLIP escape sequences and emits the recovered bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!frame.isEmpty());

    QByteArray decoded;
    decoded.reserve(frame.size());

    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    while (i < size) {
      const int byte = u8At(frame, i++);
      if (byte == kEnd)
        continue;

      if (byte != kEsc) {
        decoded.append(static_cast<char>(byte));
        continue;
      }

      if (i >= size) {
        decoded.append(static_cast<char>(kEsc));
        continue;
      }

      const int next = u8At(frame, i++);
      if (next == kEscEnd)
        decoded.append(static_cast<char>(kEnd));
      else if (next == kEscEsc)
        decoded.append(static_cast<char>(kEsc));
      else {
        decoded.append(static_cast<char>(kEsc));
        decoded.append(static_cast<char>(next));
      }
    }

    return byteRowFrame(decoded);
  }

private:
  static constexpr int kEnd    = 0xC0;
  static constexpr int kEsc    = 0xDB;
  static constexpr int kEscEnd = 0xDC;
  static constexpr int kEscEsc = 0xDD;
};

/**
 * @brief Descriptor for the SLIP template.
 */
class SlipTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("slip_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("SLIP-encoded frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Resolves SLIP escape sequences (RFC 1055) and emits the decoded bytes as "
                    "channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<SlipParser>();
  }
};

//--------------------------------------------------------------------------------------------------
// UBX protocol (u-blox)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching UBX decoder for NAV-PVT, NAV-SAT, NAV-SOL and NAV-POSLLH messages.
 */
class UbxParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the checksum-validation flag.
   */
  explicit UbxParser(bool validateChecksum)
    : NativeLatchParser(kUbxChannels), m_validateChecksum(validateChecksum)
  {
    Q_ASSERT(latchCount() == kUbxChannels);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one sync-stripped UBX message: [class][id][len lo][len hi][payload][ckA][ckB].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6)
      return latchedFrame();

    const int msg_class = u8At(frame, 0);
    const int msg_id    = u8At(frame, 1);
    const int length    = u16Le(frame, 2);
    if (frame.size() < 4 + length + 2)
      return latchedFrame();

    if (m_validateChecksum && !checksumOk(frame, length))
      return latchedFrame();

    const QByteArray payload = frame.mid(4, length);
    routeMessage(msg_class, msg_id, payload);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the Fletcher checksum over class, id, length and payload.
   */
  [[nodiscard]] static bool checksumOk(const QByteArray& frame, int length)
  {
    Q_ASSERT(frame.size() >= 4 + length + 2);

    quint8 ck_a = 0;
    quint8 ck_b = 0;
    for (qsizetype i = 0; i < 4 + length; ++i) {
      ck_a = static_cast<quint8>(ck_a + u8At(frame, i));
      ck_b = static_cast<quint8>(ck_b + ck_a);
    }

    return ck_a == u8At(frame, 4 + length) && ck_b == u8At(frame, 4 + length + 1);
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int msgClass, int msgId, const QByteArray& payload)
  {
    if (msgClass != 0x01)
      return;

    if (msgId == 0x07 && payload.size() >= 68)
      return routeNavPvt(payload);

    if (msgId == 0x35 && payload.size() >= 6)
      return routeNavSat(payload);

    if (msgId == 0x06 && payload.size() >= 48)
      return routeNavSol(payload);

    if (msgId == 0x02 && payload.size() >= 20)
      routeNavPosLlh(payload);
  }

  /**
   * @brief NAV-PVT: position, velocity and time into channels 0-10.
   */
  void routeNavPvt(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 68);

    storeAt(0, QString::number(i32Le(p, 28) * 1e-7));
    storeAt(1, QString::number(i32Le(p, 24) * 1e-7));
    storeAt(2, QString::number(i32Le(p, 36) * 0.001));
    storeAt(3, QString::number(i32Le(p, 32) * 0.001));
    storeAt(4, QString::number(i32Le(p, 60) * 0.001));
    storeAt(5, QString::number(i32Le(p, 64) * 1e-5));
    storeAt(6, QString::number(i32Le(p, 48) * 0.001));
    storeAt(7, QString::number(i32Le(p, 52) * 0.001));
    storeAt(8, QString::number(i32Le(p, 56) * 0.001));
    storeAt(9, QString::number(u8At(p, 23)));
    storeAt(10, QString::number(u32Le(p, 0)));
  }

  /**
   * @brief NAV-SAT summary into channels 11-13.
   */
  void routeNavSat(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 6);

    storeAt(11, QString::number(u32Le(p, 0)));
    storeAt(12, QString::number(u8At(p, 4)));
    storeAt(13, QString::number(u8At(p, 5)));
  }

  /**
   * @brief NAV-SOL ECEF solution into channels 14-18.
   */
  void routeNavSol(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 48);

    storeAt(14, QString::number(i32Le(p, 12) * 0.01));
    storeAt(15, QString::number(i32Le(p, 16) * 0.01));
    storeAt(16, QString::number(i32Le(p, 20) * 0.01));
    storeAt(17, QString::number(u32Le(p, 24) * 0.01));
    storeAt(18, QString::number(u8At(p, 47)));
  }

  /**
   * @brief NAV-POSLLH geodetic position into channels 0-3.
   */
  void routeNavPosLlh(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 20);

    storeAt(0, QString::number(i32Le(p, 8) * 1e-7));
    storeAt(1, QString::number(i32Le(p, 4) * 1e-7));
    storeAt(2, QString::number(i32Le(p, 16) * 0.001));
    storeAt(3, QString::number(i32Le(p, 12) * 0.001));
  }

private:
  static constexpr int kUbxChannels = 20;

  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the UBX template.
 */
class UbxTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("ubx_ublox"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("UBX protocol (u-blox)"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes NAV-PVT, NAV-SAT, NAV-SOL and NAV-POSLLH messages into navigation "
                    "channels. Use with the Binary decoder and 0xB5 0x62 start delimiter.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto checksum =
      DataModel::nativeParam("validateChecksum",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Validate checksum"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Rejects messages with an invalid Fletcher "
                                               "checksum."),
                             true);

    return {checksum};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool checksum =
      DataModel::nativeParamBool(params, QStringLiteral("validateChecksum"), true);
    return std::make_unique<UbxParser>(checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// SiRF binary protocol
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching SiRF decoder for messages 41 (geodetic), 2, 4 and 7.
 */
class SirfParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the checksum-validation flag.
   */
  explicit SirfParser(bool validateChecksum)
    : NativeLatchParser(kSirfChannels), m_validateChecksum(validateChecksum)
  {
    Q_ASSERT(latchCount() == kSirfChannels);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one delimiter-stripped SiRF message: [len hi][len lo][payload][chk hi][chk lo].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 5)
      return latchedFrame();

    const int length = ((u8At(frame, 0) & 0x7F) << 8) | u8At(frame, 1);
    if (frame.size() < 2 + length + 2)
      return latchedFrame();

    if (m_validateChecksum && !checksumOk(frame, length))
      return latchedFrame();

    // Payload starts at the message id; handlers receive the bytes after it
    const int message_id  = u8At(frame, 2);
    const QByteArray data = frame.mid(3, length - 1);
    routeMessage(message_id, data);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the 15-bit additive checksum over the payload.
   */
  [[nodiscard]] static bool checksumOk(const QByteArray& frame, int length)
  {
    Q_ASSERT(frame.size() >= 2 + length + 2);

    quint32 sum = 0;
    for (qsizetype i = 2; i < 2 + length; ++i)
      sum += u8At(frame, i);

    const int expected = ((u8At(frame, 2 + length) & 0x7F) << 8) | u8At(frame, 2 + length + 1);
    return static_cast<int>(sum & 0x7FFF) == expected;
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int messageId, const QByteArray& data)
  {
    if (messageId == 41 && data.size() >= 90)
      return routeGeodetic(data);

    if (messageId == 2 && data.size() >= 29)
      return routeMeasured(data);

    if (messageId == 4 && data.size() >= 8)
      return routeTracker(data);

    if (messageId == 7 && data.size() >= 8)
      routeClock(data);
  }

  /**
   * @brief Geodetic navigation data (id 41) into channels 0-8.
   */
  void routeGeodetic(const QByteArray& d)
  {
    Q_ASSERT(d.size() >= 90);

    storeAt(0, QString::number(i32Be(d, 23) * 1e-7));
    storeAt(1, QString::number(i32Be(d, 27) * 1e-7));
    storeAt(2, QString::number(i32Be(d, 35) * 0.01));
    storeAt(3, QString::number(i32Be(d, 31) * 0.01));
    storeAt(4, QString::number(i16Be(d, 40) * 0.01));
    storeAt(5, QString::number(u16Be(d, 42) * 0.01));
    storeAt(6, QString::number(u8At(d, 88)));
    storeAt(7, QString::number(u8At(d, 89) * 0.2));
    storeAt(8, QString::number(u16Be(d, 0)));
  }

  /**
   * @brief Measurement data (id 2) into channels 10-13.
   */
  void routeMeasured(const QByteArray& d)
  {
    Q_ASSERT(d.size() >= 29);

    storeAt(10, QString::number(i32Be(d, 1)));
    storeAt(11, QString::number(i32Be(d, 5)));
    storeAt(12, QString::number(i32Be(d, 9)));
    storeAt(13, QString::number(u8At(d, 28)));
  }

  /**
   * @brief Tracker data (id 4) into channels 14-16.
   */
  void routeTracker(const QByteArray& d)
  {
    Q_ASSERT(d.size() >= 8);

    storeAt(14, QString::number(u16Be(d, 1)));
    storeAt(15, QString::number(u32Be(d, 3) * 0.01));
    storeAt(16, QString::number(u8At(d, 7)));
  }

  /**
   * @brief Clock status (id 7) into channels 17-19.
   */
  void routeClock(const QByteArray& d)
  {
    Q_ASSERT(d.size() >= 8);

    storeAt(17, QString::number(u16Be(d, 1)));
    storeAt(18, QString::number(u32Be(d, 3)));
    storeAt(19, QString::number(u8At(d, 7)));
  }

private:
  static constexpr int kSirfChannels = 20;

  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the SiRF template.
 */
class SirfTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("sirf_binary"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("SiRF binary protocol"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes SiRF navigation messages (geodetic data, measurements, clock) "
                    "into channels. Use with the Binary decoder and 0xA0 0xA2 framing.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto checksum =
      DataModel::nativeParam("validateChecksum",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Validate checksum"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Rejects messages with an invalid additive "
                                               "checksum."),
                             true);

    return {checksum};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool checksum =
      DataModel::nativeParamBool(params, QStringLiteral("validateChecksum"), true);
    return std::make_unique<SirfParser>(checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// MAVLink messages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching MAVLink decoder for ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages.
 */
class MavlinkParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the expected start marker for the configured protocol version.
   */
  explicit MavlinkParser(int version)
    : NativeLatchParser(kMavlinkChannels), m_marker(version == 2 ? 0xFD : 0xFE)
  {
    Q_ASSERT(version == 1 || version == 2);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one MAVLink frame and updates the latched channels.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 8 || u8At(frame, 0) != m_marker)
      return latchedFrame();

    const int payload_length = u8At(frame, 1);
    const int message_id     = u8At(frame, 5);
    const QByteArray payload = frame.mid(6, payload_length);

    if (message_id == 30 && payload.size() >= 16)
      routeAttitude(payload);
    else if (message_id == 74 && payload.size() >= 12)
      routeVfrHud(payload);
    else if (message_id == 33 && payload.size() >= 16)
      routeGlobalPosition(payload);

    return latchedFrame();
  }

private:
  /**
   * @brief ATTITUDE (id 30): roll, pitch, yaw into channels 0-2.
   */
  void routeAttitude(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 16);

    storeAt(0, QString::number(f32Le(p, 4)));
    storeAt(1, QString::number(f32Le(p, 8)));
    storeAt(2, QString::number(f32Le(p, 12)));
  }

  /**
   * @brief VFR_HUD (id 74): airspeed, groundspeed, heading, throttle into channels 3-6.
   */
  void routeVfrHud(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 12);

    storeAt(3, QString::number(f32Le(p, 0)));
    storeAt(4, QString::number(f32Le(p, 4)));
    storeAt(5, QString::number(i16Le(p, 8)));
    storeAt(6, QString::number(u16Le(p, 10)));
  }

  /**
   * @brief GLOBAL_POSITION_INT (id 33): latitude, longitude, altitude into channels 7-9.
   */
  void routeGlobalPosition(const QByteArray& p)
  {
    Q_ASSERT(p.size() >= 16);

    storeAt(7, QString::number(i32Le(p, 4) * 1e-7));
    storeAt(8, QString::number(i32Le(p, 8) * 1e-7));
    storeAt(9, QString::number(i32Le(p, 12) * 0.001));
  }

private:
  static constexpr int kMavlinkChannels = 16;

  int m_marker;
};

/**
 * @brief Descriptor for the MAVLink template.
 */
class MavlinkTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("mavlink"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("MAVLink messages"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages into channels. "
                    "Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto version =
      DataModel::nativeParam("version",
                             NativeParamType::Enum,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Protocol version"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Selects the expected start marker (0xFE for v1, "
                                               "0xFD for v2)."),
                             QStringLiteral("v1"));
    version.optionValues = {QStringLiteral("v1"), QStringLiteral("v2")};
    version.optionLabels = {trBinary("MAVLink v1"), trBinary("MAVLink v2")};

    return {version};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const QString version =
      DataModel::nativeParamString(params, QStringLiteral("version"), QStringLiteral("v1"));
    return std::make_unique<MavlinkParser>(version == QStringLiteral("v2") ? 2 : 1);
  }
};

//--------------------------------------------------------------------------------------------------
// NMEA 2000 messages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching NMEA 2000 decoder routing common PGNs into channels.
 */
class Nmea2000Parser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Builds the parser with the fixed channel layout.
   */
  Nmea2000Parser() : NativeLatchParser(kN2kChannels) { Q_ASSERT(latchCount() == kN2kChannels); }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one preprocessed CAN frame: [id u32 le][len][data...].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6)
      return latchedFrame();

    const quint32 can_id = u32Le(frame, 0);
    const int length     = qMin<int>(u8At(frame, 4), 8);
    if (frame.size() < 5 + length)
      return latchedFrame();

    const QByteArray data = frame.mid(5, length);
    routePgn(extractPgn(can_id), data);
    return latchedFrame();
  }

private:
  /**
   * @brief Extracts the PGN from a 29-bit CAN identifier.
   */
  [[nodiscard]] static quint32 extractPgn(quint32 canId)
  {
    const quint32 pf = (canId >> 16) & 0xFF;
    const quint32 ps = (canId >> 8) & 0xFF;
    const quint32 dp = (canId >> 24) & 0x01;

    if (pf < 240)
      return (dp << 16) | (pf << 8);

    return (dp << 16) | (pf << 8) | ps;
  }

  /**
   * @brief Routes a PGN payload into the channel layout.
   */
  void routePgn(quint32 pgn, const QByteArray& d)
  {
    Q_ASSERT(d.size() <= 8);

    if (pgn == 127257 && d.size() >= 7)
      return routeAttitude(d);

    if (pgn == 129025 && d.size() >= 8)
      return routePosition(d);

    if (pgn == 129026 && d.size() >= 6)
      return routeCogSog(d);

    if (pgn == 128259 && d.size() >= 5)
      return routeSpeed(d);

    if (pgn == 128267 && d.size() >= 7)
      return routeDepth(d);

    if (pgn == 130310 && d.size() >= 5)
      return routeWaterEnv(d);

    if (pgn == 130311 && d.size() >= 7)
      routeAtmosphere(d);
  }

  /**
   * @brief Attitude (127257): yaw, pitch, roll in degrees into channels 0-2.
   */
  void routeAttitude(const QByteArray& d)
  {
    static constexpr double kRadToDeg = 180.0 / 3.14159265358979323846;

    storeAt(0, QString::number(i16Le(d, 1) * 0.0001 * kRadToDeg));
    storeAt(1, QString::number(i16Le(d, 3) * 0.0001 * kRadToDeg));
    storeAt(2, QString::number(i16Le(d, 5) * 0.0001 * kRadToDeg));
  }

  /**
   * @brief Position rapid update (129025) into channels 3-4.
   */
  void routePosition(const QByteArray& d)
  {
    storeAt(3, QString::number(i32Le(d, 0) * 1e-7));
    storeAt(4, QString::number(i32Le(d, 4) * 1e-7));
  }

  /**
   * @brief COG and SOG rapid update (129026) into channels 5-7.
   */
  void routeCogSog(const QByteArray& d)
  {
    static constexpr double kRadToDeg   = 180.0 / 3.14159265358979323846;
    static constexpr double kMpsToKnots = 1.94384;

    storeAt(5, QString::number(u16Le(d, 2) * 0.0001 * kRadToDeg));
    storeAt(6, QString::number(u16Le(d, 4) * 0.01 * kMpsToKnots));
    storeAt(7, QString::number(u8At(d, 1) & 0x03));
  }

  /**
   * @brief Speed water referenced (128259) into channels 8-9.
   */
  void routeSpeed(const QByteArray& d)
  {
    static constexpr double kMpsToKnots = 1.94384;

    storeAt(8, QString::number(u16Le(d, 1) * 0.01 * kMpsToKnots));
    storeAt(9, QString::number(u16Le(d, 3) * 0.01 * kMpsToKnots));
  }

  /**
   * @brief Water depth (128267) into channels 10-11.
   */
  void routeDepth(const QByteArray& d)
  {
    storeAt(10, QString::number(u32Le(d, 1) * 0.01));
    storeAt(11, QString::number(i16Le(d, 5) * 0.001));
  }

  /**
   * @brief Environmental water parameters (130310) into channels 12-13.
   */
  void routeWaterEnv(const QByteArray& d)
  {
    storeAt(12, QString::number(u16Le(d, 1) * 0.01 - 273.15));
    storeAt(13, QString::number(u16Le(d, 3) * 0.01 - 273.15));
  }

  /**
   * @brief Environmental atmospheric parameters (130311) into channels 14-16.
   */
  void routeAtmosphere(const QByteArray& d)
  {
    storeAt(14, QString::number(u16Le(d, 1)));
    storeAt(15, QString::number(u16Le(d, 3) * 0.01 - 273.15));
    storeAt(16, QString::number(i16Le(d, 5) * 0.004));
  }

private:
  static constexpr int kN2kChannels = 20;
};

/**
 * @brief Descriptor for the NMEA 2000 template.
 */
class Nmea2000Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("nmea_2000"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("NMEA 2000 messages"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes common marine PGNs (attitude, position, speed, depth, "
                    "environment) into channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<Nmea2000Parser>();
  }
};

//--------------------------------------------------------------------------------------------------
// RTCM corrections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching RTCM3 decoder for reference-station (1005) and MSM7 (1077, 1087) messages.
 */
class RtcmParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the CRC-validation flag.
   */
  explicit RtcmParser(bool validateCrc)
    : NativeLatchParser(kRtcmChannels), m_validateCrc(validateCrc)
  {
    Q_ASSERT(latchCount() == kRtcmChannels);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one RTCM3 frame: 0xD3, 10-bit length, payload, CRC-24Q.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6 || u8At(frame, 0) != 0xD3)
      return latchedFrame();

    const int length = ((u8At(frame, 1) & 0x03) << 8) | u8At(frame, 2);
    if (frame.size() < 3 + length + 3)
      return latchedFrame();

    if (m_validateCrc && !crc24Ok(frame, length))
      return latchedFrame();

    const QByteArray payload = frame.mid(3, length);
    const int message_type   = static_cast<int>(readBits(payload, 0, 12));
    routeMessage(message_type, payload);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the CRC-24Q over header + payload against the trailing 3 bytes.
   */
  [[nodiscard]] static bool crc24Ok(const QByteArray& frame, int length)
  {
    Q_ASSERT(frame.size() >= 3 + length + 3);

    quint32 crc = 0;
    for (qsizetype i = 0; i < 3 + length; ++i) {
      crc ^= static_cast<quint32>(u8At(frame, i)) << 16;
      for (int bit = 0; bit < 8; ++bit) {
        crc <<= 1;
        if (crc & 0x1000000)
          crc ^= 0x1864CFB;
      }
    }

    crc                    &= 0xFFFFFF;
    const quint32 expected  = (static_cast<quint32>(u8At(frame, 3 + length)) << 16)
                           | (static_cast<quint32>(u8At(frame, 3 + length + 1)) << 8)
                           | static_cast<quint32>(u8At(frame, 3 + length + 2));
    return crc == expected;
  }

  /**
   * @brief Reads big-endian bits from the payload with optional sign extension.
   */
  [[nodiscard]] static qint64 readBits(const QByteArray& data,
                                       int bitPos,
                                       int numBits,
                                       bool isSigned = false)
  {
    Q_ASSERT(numBits >= 1);
    Q_ASSERT(numBits <= 62);

    quint64 value = 0;
    for (int i = 0; i < numBits; ++i) {
      const int byte_index = (bitPos + i) / 8;
      const int bit_index  = 7 - ((bitPos + i) % 8);
      const int bit = (byte_index < data.size()) ? ((u8At(data, byte_index) >> bit_index) & 1) : 0;
      value         = (value << 1) | static_cast<quint64>(bit);
    }

    if (isSigned && (value & (quint64(1) << (numBits - 1))))
      return static_cast<qint64>(value) - (qint64(1) << numBits);

    return static_cast<qint64>(value);
  }

  /**
   * @brief Counts the satellites flagged in a 64-bit mask.
   */
  [[nodiscard]] static int satelliteCount(qint64 mask)
  {
    return std::popcount(static_cast<quint64>(mask));
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int messageType, const QByteArray& payload)
  {
    if (messageType == 1005)
      routeReferenceStation(payload);
    else if (messageType == 1077)
      routeMsm7(payload, 30, 4);
    else if (messageType == 1087)
      routeMsm7(payload, 27, 8);
  }

  /**
   * @brief Reference station ARP (1005): station id + ECEF into channels 0-3.
   */
  void routeReferenceStation(const QByteArray& p)
  {
    int bits = 12;
    storeAt(0, QString::number(readBits(p, bits, 12)));
    bits += 12 + 6;
    storeAt(1, QString::number(readBits(p, bits, 38, true) * 0.0001));
    bits += 38;
    storeAt(2, QString::number(readBits(p, bits, 38, true) * 0.0001));
    bits += 38;
    storeAt(3, QString::number(readBits(p, bits, 38, true) * 0.0001));
  }

  /**
   * @brief MSM7 header: station id, epoch, sync flag and satellite count.
   */
  void routeMsm7(const QByteArray& p, int epochBits, int channelBase)
  {
    int bits = 12;
    storeAt(channelBase, QString::number(readBits(p, bits, 12)));
    bits += 12;
    storeAt(channelBase + 1, QString::number(readBits(p, bits, epochBits)));
    bits += epochBits;
    storeAt(channelBase + 2, QString::number(readBits(p, bits, 1)));
    bits += 1;
    storeAt(channelBase + 3, QString::number(satelliteCount(readBits(p, bits, 62))));
  }

private:
  static constexpr int kRtcmChannels = 16;

  bool m_validateCrc;
};

/**
 * @brief Descriptor for the RTCM corrections template.
 */
class RtcmTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("rtcm_corrections"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("RTCM corrections"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes RTCM3 reference-station and MSM7 headers (station id, epoch, "
                    "satellite count) into channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto crc = DataModel::nativeParam(
      "validateCrc",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Validate CRC"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Rejects frames with an invalid CRC-24Q checksum."),
      true);

    return {crc};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool crc = DataModel::nativeParamBool(params, QStringLiteral("validateCrc"), true);
    return std::make_unique<RtcmParser>(crc);
  }
};

//--------------------------------------------------------------------------------------------------
// Modbus frames
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching Modbus ADU decoder (coils, registers and single-write echoes).
 */
class ModbusParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the channel count, register offset and sign configuration.
   */
  ModbusParser(int numItems, int registerOffset, bool signedRegisters)
    : NativeLatchParser(numItems)
    , m_registerOffset(registerOffset)
    , m_signedRegisters(signedRegisters)
  {
    Q_ASSERT(numItems >= 1);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes one Modbus ADU (no CRC): [address][function][data...].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 3)
      return latchedFrame();

    const int function = u8At(frame, 1);
    if (function >= 0x80)
      return latchedFrame();

    if (function == 0x01 || function == 0x02)
      routeBits(frame);
    else if (function == 0x03 || function == 0x04)
      routeRegisters(frame);
    else if ((function == 0x05 || function == 0x06) && frame.size() >= 6)
      routeSingleWrite(frame, function);

    return latchedFrame();
  }

private:
  /**
   * @brief Read coils / discrete inputs: one bit per channel.
   */
  void routeBits(const QByteArray& frame)
  {
    Q_ASSERT(frame.size() >= 3);

    const int byte_count = u8At(frame, 2);
    int channel          = 0;
    for (int byte = 0; byte < byte_count && 3 + byte < frame.size(); ++byte) {
      const int data = u8At(frame, 3 + byte);
      for (int bit = 0; bit < 8 && channel < latchCount(); ++bit)
        storeAt(channel++, QString::number((data >> bit) & 0x01));
    }
  }

  /**
   * @brief Read holding / input registers: one 16-bit big-endian value per channel.
   */
  void routeRegisters(const QByteArray& frame)
  {
    Q_ASSERT(frame.size() >= 3);

    const int register_count = u8At(frame, 2) / 2;
    for (int i = 0; i < register_count && i < latchCount(); ++i) {
      const qsizetype offset = 3 + (i * 2);
      if (offset + 1 >= frame.size())
        break;

      storeAt(i, registerValue(u16Be(frame, offset)));
    }
  }

  /**
   * @brief Write single coil/register echo: routes the value to the addressed channel.
   */
  void routeSingleWrite(const QByteArray& frame, int function)
  {
    Q_ASSERT(frame.size() >= 6);

    const int address = u16Be(frame, 2);
    const int value   = u16Be(frame, 4);
    const int channel = address - m_registerOffset;

    if (function == 0x05)
      storeAt(channel, QString::number(value == 0xFF00 ? 1 : 0));
    else
      storeAt(channel, registerValue(static_cast<quint16>(value)));
  }

  /**
   * @brief Converts one raw register honoring the signed flag.
   */
  [[nodiscard]] QString registerValue(quint16 raw) const
  {
    if (m_signedRegisters)
      return QString::number(static_cast<qint16>(raw));

    return QString::number(raw);
  }

private:
  int m_registerOffset;
  bool m_signedRegisters;
};

/**
 * @brief Descriptor for the Modbus template.
 */
class ModbusTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("modbus"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("Modbus frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes Modbus responses (coils, registers and single-write echoes) into "
                    "channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto items = DataModel::nativeParam(
      "numItems",
      NativeParamType::Int,
      QT_TRANSLATE_NOOP("NativeTemplates", "Channel count"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Number of output channels (registers or coils)."),
      9);
    items.minValue = 1;
    items.maxValue = 256;

    auto offset =
      DataModel::nativeParam("registerOffset",
                             NativeParamType::Int,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Register offset"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Address offset subtracted from single-write echoes "
                                               "(40001 for legacy Modicon maps)."),
                             0);
    offset.minValue = 0;
    offset.maxValue = 65535;

    auto sign =
      DataModel::nativeParam("signedRegisters",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Signed registers"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Interprets 16-bit registers as two's-complement "
                                               "signed values."),
                             false);

    return {items, offset, sign};
  }

  /**
   * @brief Validates the channel count and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int items = DataModel::nativeParamInt(params, QStringLiteral("numItems"), 9);
    if (items < 1 || items > 256) {
      error = trBinary("The channel count must be between 1 and 256.");
      return nullptr;
    }

    const int offset = DataModel::nativeParamInt(params, QStringLiteral("registerOffset"), 0);
    const bool sign  = DataModel::nativeParamBool(params, QStringLiteral("signedRegisters"), false);
    return std::make_unique<ModbusParser>(items, offset, sign);
  }
};

//--------------------------------------------------------------------------------------------------
// MessagePack data
//--------------------------------------------------------------------------------------------------

/**
 * @brief MessagePack decoder supporting the common scalar, array and map encodings.
 */
class MessagePackParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the parse mode and (for map mode) the ordered key list.
   */
  MessagePackParser(bool mapMode, const QStringList& keys)
    : NativeLatchParser(mapMode ? static_cast<int>(keys.size()) : 1)
    , m_mapMode(mapMode)
    , m_keys(keys)
  {
    Q_ASSERT(!mapMode || !m_keys.isEmpty());

    m_keyIndex.reserve(keys.size());
    for (qsizetype i = 0; i < keys.size(); ++i)
      m_keyIndex.insert(keys.at(i), static_cast<int>(i));
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
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
   * @brief Decodes the top-level MessagePack value into channels.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!frame.isEmpty());

    qsizetype offset = 0;
    if (m_mapMode) {
      decodeTopLevelMap(frame, offset);
      return latchedFrame();
    }

    QStringList row;
    decodeTopLevelArray(frame, offset, row);

    QList<QStringList> out;
    out.append(std::move(row));
    return out;
  }

private:
  /**
   * @brief Decodes a scalar value at the offset; containers are skipped (ok = false).
   */
  [[nodiscard]] static QString decodeScalar(const QByteArray& data, qsizetype& offset, bool& ok)
  {
    ok = false;
    if (offset >= data.size())
      return QString();

    const int byte = u8At(data, offset++);
    ok             = true;

    if (byte <= 0x7F)
      return QString::number(byte);

    if (byte >= 0xE0)
      return QString::number(byte - 256);

    if (byte >= 0xA0 && byte <= 0xBF)
      return decodeFixStr(data, offset, byte & 0x1F);

    return decodeTyped(data, offset, byte, ok);
  }

  /**
   * @brief Decodes a fixstr payload of the given length.
   */
  [[nodiscard]] static QString decodeFixStr(const QByteArray& data, qsizetype& offset, int length)
  {
    Q_ASSERT(length >= 0);
    Q_ASSERT(length <= 31);

    const qsizetype take  = qMin<qsizetype>(length, data.size() - offset);
    const QString text    = QString::fromUtf8(data.constData() + offset, take);
    offset               += take;
    return text;
  }

  /**
   * @brief Decodes the explicit-marker scalar encodings.
   */
  [[nodiscard]] static QString decodeTyped(const QByteArray& data,
                                           qsizetype& offset,
                                           int marker,
                                           bool& ok)
  {
    const auto need = [&data, &offset](qsizetype bytes) {
      return offset + bytes <= data.size();
    };

    switch (marker) {
      case 0xC0:
        return QStringLiteral("0");
      case 0xC2:
        return QStringLiteral("false");
      case 0xC3:
        return QStringLiteral("true");
      case 0xCC:
        if (need(1))
          return QString::number(u8At(data, offset++));

        break;
      case 0xCD:
        if (need(2)) {
          const auto value  = u16Be(data, offset);
          offset           += 2;
          return QString::number(value);
        }
        break;
      case 0xCE:
        if (need(4)) {
          const auto value  = u32Be(data, offset);
          offset           += 4;
          return QString::number(value);
        }
        break;
      case 0xD0:
        if (need(1))
          return QString::number(static_cast<qint8>(u8At(data, offset++)));

        break;
      case 0xD1:
        if (need(2)) {
          const auto value  = i16Be(data, offset);
          offset           += 2;
          return QString::number(value);
        }
        break;
      case 0xD2:
        if (need(4)) {
          const auto value  = i32Be(data, offset);
          offset           += 4;
          return QString::number(value);
        }
        break;
      case 0xCA:
        if (need(4)) {
          const auto value  = f32Be(data, offset);
          offset           += 4;
          return QString::number(value);
        }
        break;
      default:
        break;
    }

    ok = false;
    return QString();
  }

  /**
   * @brief Decodes a top-level array (fixarray or array16) of scalars into the row.
   */
  static void decodeTopLevelArray(const QByteArray& data, qsizetype& offset, QStringList& row)
  {
    if (data.isEmpty())
      return;

    const int marker = u8At(data, 0);
    qsizetype count  = 0;
    offset           = 1;

    if (marker >= 0x90 && marker <= 0x9F)
      count = marker & 0x0F;
    else if (marker == 0xDC && data.size() >= 3) {
      count  = u16Be(data, 1);
      offset = 3;
    } else {
      // Single scalar payload: emit it as one channel
      offset = 0;
      bool ok;
      const QString value = decodeScalar(data, offset, ok);
      if (ok)
        row.append(value);

      return;
    }

    row.reserve(count);
    for (qsizetype i = 0; i < count && offset < data.size(); ++i) {
      bool ok;
      const QString value = decodeScalar(data, offset, ok);
      if (!ok)
        break;

      row.append(value);
    }
  }

  /**
   * @brief Decodes a top-level map (fixmap or map16) and routes string keys to channels.
   */
  void decodeTopLevelMap(const QByteArray& data, qsizetype& offset)
  {
    if (data.isEmpty())
      return;

    const int marker = u8At(data, 0);
    qsizetype count  = 0;
    offset           = 1;

    if (marker >= 0x80 && marker <= 0x8F)
      count = marker & 0x0F;
    else if (marker == 0xDE && data.size() >= 3) {
      count  = u16Be(data, 1);
      offset = 3;
    } else
      return;

    for (qsizetype i = 0; i < count && offset < data.size(); ++i) {
      bool key_ok;
      bool value_ok;
      const QString key   = decodeScalar(data, offset, key_ok);
      const QString value = decodeScalar(data, offset, value_ok);
      if (!key_ok || !value_ok)
        break;

      storeAt(m_keyIndex.value(key, -1), value);
    }
  }

private:
  bool m_mapMode;
  QStringList m_keys;
  QHash<QString, int> m_keyIndex;
};

/**
 * @brief Descriptor for the MessagePack template.
 */
class MessagePackTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("messagepack"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trBinary("MessagePack data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trBinary("Decodes a MessagePack array (one channel per element) or map (keys routed "
                    "to channels). Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto mode =
      DataModel::nativeParam("mode",
                             NativeParamType::Enum,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Payload layout"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Array emits every element in order; map routes "
                                               "keys through the key list."),
                             QStringLiteral("array"));
    mode.optionValues = {QStringLiteral("array"), QStringLiteral("map")};
    mode.optionLabels = {trBinary("Array"), trBinary("Map")};

    auto keys = DataModel::nativeParam(
      "keys",
      NativeParamType::String,
      QT_TRANSLATE_NOOP("NativeTemplates", "Keys (map mode)"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Comma-separated map keys in channel order. Only used for the map "
                        "layout."),
      QStringLiteral("temperature,humidity,pressure,voltage"));

    return {mode, keys};
  }

  /**
   * @brief Validates the configuration and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString mode =
      DataModel::nativeParamString(params, QStringLiteral("mode"), QStringLiteral("array"));
    const bool map_mode = (mode == QStringLiteral("map"));

    const auto keys = DataModel::nativeKeyList(
      params, QStringLiteral("keys"), QStringLiteral("temperature,humidity,pressure,voltage"));
    if (map_mode && keys.isEmpty()) {
      error = trBinary("Map mode requires at least one key.");
      return nullptr;
    }

    return std::make_unique<MessagePackParser>(map_mode, keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the binary-protocol native templates in display order.
 */
QList<const DataModel::INativeTemplate*> DataModel::binaryNativeTemplates()
{
  static const RawBytesTemplate s_rawBytes;
  static const HexBytesTemplate s_hexBytes;
  static const Base64Template s_base64;
  static const BinaryTlvTemplate s_binaryTlv;
  static const CobsTemplate s_cobs;
  static const SlipTemplate s_slip;
  static const UbxTemplate s_ubx;
  static const SirfTemplate s_sirf;
  static const MavlinkTemplate s_mavlink;
  static const Nmea2000Template s_nmea2000;
  static const RtcmTemplate s_rtcm;
  static const ModbusTemplate s_modbus;
  static const MessagePackTemplate s_messagePack;

  return {&s_rawBytes,
          &s_hexBytes,
          &s_base64,
          &s_binaryTlv,
          &s_cobs,
          &s_slip,
          &s_ubx,
          &s_sirf,
          &s_mavlink,
          &s_nmea2000,
          &s_rtcm,
          &s_modbus,
          &s_messagePack};
}
