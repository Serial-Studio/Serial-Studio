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
#include <QJsonArray>
#include <QJsonObject>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "SerialStudio.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;

static constexpr int kMaxMapBlocks   = 64;
static constexpr int kMaxMapChannels = 4096;
static constexpr int kMaxCanMessages = 1024;
static constexpr int kMaxCanPayload  = 64;

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
[[nodiscard]] static QString trMap(const char* text)
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
 * @brief Reads an n-byte unsigned big-endian value at the offset (n capped at 8).
 */
[[nodiscard]] static quint64 uBeAt(const QByteArray& data, qsizetype i, int bytes)
{
  Q_ASSERT(bytes >= 1);
  Q_ASSERT(i + bytes <= data.size());

  quint64 value       = 0;
  const int use_bytes = qMin(bytes, 8);
  for (int b = 0; b < use_bytes; ++b)
    value = (value << 8) | u8At(data, i + b);

  return value;
}

/**
 * @brief Formats a channel value keeping enough digits for a lossless double round-trip.
 */
[[nodiscard]] static QString mapValue(double value)
{
  return QString::number(value, 'g', 15);
}

//--------------------------------------------------------------------------------------------------
// Modbus register map
//--------------------------------------------------------------------------------------------------

/**
 * @brief Data layout of a single mapped Modbus register entry.
 */
enum class ModbusEntryType : quint8 {
  U16,
  I16,
  U32,
  I32,
  U64,
  I64,
  F32,
  F64,
  Bit,
  RegBool,
};

/**
 * @brief One mapped register: latch channel, register offset within the block and scaling.
 */
struct ModbusMapEntry {
  int channel          = 0;
  int reg_offset       = 0;
  double scale         = 1.0;
  double offset        = 0.0;
  ModbusEntryType type = ModbusEntryType::U16;
};

/**
 * @brief One polled register block: expected read function code and its mapped entries.
 */
struct ModbusMapBlock {
  quint8 function_code = 0x03;
  QList<ModbusMapEntry> entries;
};

/**
 * @brief Maps a register-type index (0-3) to the read response function code.
 */
[[nodiscard]] static quint8 modbusFunctionForType(int registerType)
{
  switch (registerType) {
    case 1:
      return 0x04;
    case 2:
      return 0x01;
    case 3:
      return 0x02;
    case 0:
    default:
      return 0x03;
  }
}

/**
 * @brief Maps a dataType string to the entry layout (bit blocks decode as packed
 * bits; a register-block bool decodes as a 16-bit truthiness word).
 */
[[nodiscard]] static ModbusEntryType modbusEntryType(const QString& name, bool bitBlock)
{
  if (bitBlock)
    return ModbusEntryType::Bit;

  if (name == QLatin1String("bool"))
    return ModbusEntryType::RegBool;

  if (name == QLatin1String("int16"))
    return ModbusEntryType::I16;

  if (name == QLatin1String("uint32"))
    return ModbusEntryType::U32;

  if (name == QLatin1String("int32"))
    return ModbusEntryType::I32;

  if (name == QLatin1String("uint64"))
    return ModbusEntryType::U64;

  if (name == QLatin1String("int64"))
    return ModbusEntryType::I64;

  if (name == QLatin1String("float32"))
    return ModbusEntryType::F32;

  if (name == QLatin1String("float64"))
    return ModbusEntryType::F64;

  return ModbusEntryType::U16;
}

/**
 * @brief Returns the number of payload bytes consumed by an entry layout.
 */
[[nodiscard]] static int modbusEntryBytes(ModbusEntryType type)
{
  switch (type) {
    case ModbusEntryType::U32:
    case ModbusEntryType::I32:
    case ModbusEntryType::F32:
      return 4;
    case ModbusEntryType::U64:
    case ModbusEntryType::I64:
    case ModbusEntryType::F64:
      return 8;
    default:
      return 2;
  }
}

/**
 * @brief Latching register-map decoder that tracks the driver's round-robin poll order.
 * Responses carry no block identity, so the instance advances a group cursor per frame and
 * resyncs on the response function code when a poll reply was dropped.
 */
class ModbusRegisterMapParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the block list; the group cursor starts at the first block.
   */
  ModbusRegisterMapParser(const QList<ModbusMapBlock>& blocks, int channelCount)
    : NativeLatchParser(channelCount), m_group(0), m_blocks(blocks)
  {
    Q_ASSERT(!m_blocks.isEmpty());
    Q_ASSERT(channelCount >= 1);
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
   * @brief Decodes one Modbus response ADU: [slave address][function][byte count][data...].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!m_blocks.isEmpty());

    if (frame.size() < 3)
      return latchedFrame();

    const int function = u8At(frame, 1);
    if (function >= 0x80)
      return latchedFrame();

    int block_idx = -1;
    for (int probe = 0; probe < m_blocks.size(); ++probe) {
      const int candidate = (m_group + probe) % m_blocks.size();
      if (m_blocks.at(candidate).function_code == function) {
        block_idx = candidate;
        break;
      }
    }

    if (block_idx < 0)
      return latchedFrame();

    decodeBlock(m_blocks.at(block_idx), frame);
    m_group = (block_idx + 1) % m_blocks.size();
    return latchedFrame();
  }

  /**
   * @brief Clears the latch row and rewinds the poll cursor to the first block.
   */
  void reset() override
  {
    NativeLatchParser::reset();
    m_group = 0;
  }

private:
  /**
   * @brief Decodes every entry of the block that fits inside the response payload.
   */
  void decodeBlock(const ModbusMapBlock& block, const QByteArray& frame)
  {
    Q_ASSERT(frame.size() >= 3);

    const int byte_count  = u8At(frame, 2);
    const qsizetype limit = qMin<qsizetype>(3 + byte_count, frame.size());
    Q_ASSERT(limit <= frame.size());

    for (const auto& entry : block.entries)
      if (entry.type == ModbusEntryType::Bit)
        decodeBit(entry, frame, limit);
      else
        decodeRegister(entry, frame, limit);
  }

  /**
   * @brief Decodes a coil / discrete-input entry (LSB-first packing, one bit per channel).
   */
  void decodeBit(const ModbusMapEntry& entry, const QByteArray& frame, qsizetype limit)
  {
    Q_ASSERT(entry.reg_offset >= 0);

    const qsizetype byte_idx = 3 + (entry.reg_offset / 8);
    if (byte_idx >= limit)
      return;

    const int bit = (u8At(frame, byte_idx) >> (entry.reg_offset % 8)) & 0x01;
    storeAt(entry.channel, QString::number(bit));
  }

  /**
   * @brief Decodes a register entry (big-endian words) and applies its scale and offset.
   */
  void decodeRegister(const ModbusMapEntry& entry, const QByteArray& frame, qsizetype limit)
  {
    Q_ASSERT(entry.reg_offset >= 0);

    const int bytes          = modbusEntryBytes(entry.type);
    const qsizetype byte_off = 3 + (qsizetype(entry.reg_offset) * 2);
    if (byte_off + bytes > limit)
      return;

    const quint64 raw = uBeAt(frame, byte_off, bytes);
    const bool scaled = (entry.scale != 1.0 || entry.offset != 0.0);

    if (entry.type == ModbusEntryType::RegBool) {
      storeAt(entry.channel, QString::number(raw != 0 ? 1 : 0));
      return;
    }

    if (!scaled && isIntegerEntry(entry.type)) {
      storeAt(entry.channel, integerValue(entry.type, raw));
      return;
    }

    const double value = numericValue(entry.type, raw) * entry.scale + entry.offset;
    storeAt(entry.channel, mapValue(value));
  }

  /**
   * @brief Returns true for the integer entry layouts.
   */
  [[nodiscard]] static bool isIntegerEntry(ModbusEntryType type)
  {
    return type != ModbusEntryType::F32 && type != ModbusEntryType::F64;
  }

  /**
   * @brief Formats an unscaled integer entry exactly, honoring the signed layouts.
   */
  [[nodiscard]] static QString integerValue(ModbusEntryType type, quint64 raw)
  {
    switch (type) {
      case ModbusEntryType::I16:
        return QString::number(static_cast<qint16>(raw));
      case ModbusEntryType::I32:
        return QString::number(static_cast<qint32>(raw));
      case ModbusEntryType::I64:
        return QString::number(static_cast<qint64>(raw));
      default:
        return QString::number(raw);
    }
  }

  /**
   * @brief Converts the raw big-endian payload of an entry into a double.
   */
  [[nodiscard]] static double numericValue(ModbusEntryType type, quint64 raw)
  {
    switch (type) {
      case ModbusEntryType::I16:
        return static_cast<qint16>(raw);
      case ModbusEntryType::I32:
        return static_cast<qint32>(raw);
      case ModbusEntryType::I64:
        return static_cast<double>(static_cast<qint64>(raw));
      case ModbusEntryType::F32:
        return std::bit_cast<float>(static_cast<quint32>(raw));
      case ModbusEntryType::F64:
        return std::bit_cast<double>(raw);
      case ModbusEntryType::U64:
        return static_cast<double>(raw);
      default:
        return static_cast<double>(static_cast<quint32>(raw));
    }
  }

private:
  int m_group;
  QList<ModbusMapBlock> m_blocks;
};

/**
 * @brief Builds the default example map: one holding-register block with two registers.
 */
[[nodiscard]] static QJsonArray modbusMapDefault()
{
  QJsonObject reg0;
  reg0.insert(QStringLiteral("address"), 0);
  reg0.insert(QStringLiteral("dataType"), QStringLiteral("uint16"));

  QJsonObject reg1 = reg0;
  reg1.insert(QStringLiteral("address"), 1);

  QJsonObject block;
  block.insert(QStringLiteral("type"), 0);
  block.insert(QStringLiteral("start"), 0);
  block.insert(QStringLiteral("entries"), QJsonArray{reg0, reg1});
  return QJsonArray{block};
}

/**
 * @brief Parses the blocks JSON into the runtime block list; returns the channel count.
 */
[[nodiscard]] static int parseModbusBlocks(const QJsonArray& json, QList<ModbusMapBlock>& blocks)
{
  int channel                 = 0;
  const qsizetype block_count = qMin<qsizetype>(json.size(), kMaxMapBlocks);
  for (qsizetype b = 0; b < block_count; ++b) {
    const auto obj     = json.at(b).toObject();
    const int reg_type = obj.value(QStringLiteral("type")).toInt();
    const int start    = obj.value(QStringLiteral("start")).toInt();
    const bool is_bits = (reg_type >= 2);
    const auto entries = obj.value(QStringLiteral("entries")).toArray();

    ModbusMapBlock block;
    block.function_code = modbusFunctionForType(reg_type);

    for (const auto& item : entries) {
      if (channel >= kMaxMapChannels)
        break;

      const auto entry_obj = item.toObject();
      const int address    = entry_obj.value(QStringLiteral("address")).toInt(-1);
      if (address < 0 || address < start || address > 65535)
        continue;

      ModbusMapEntry entry;
      entry.channel    = channel++;
      entry.reg_offset = address - start;
      entry.scale      = SerialStudio::toDouble(entry_obj.value(QStringLiteral("scale")), 1.0);
      entry.offset     = SerialStudio::toDouble(entry_obj.value(QStringLiteral("offset")), 0.0);
      entry.type = modbusEntryType(entry_obj.value(QStringLiteral("dataType")).toString(), is_bits);
      block.entries.append(entry);
    }

    if (!block.entries.isEmpty())
      blocks.append(block);
  }

  return channel;
}

/**
 * @brief Descriptor for the Modbus register map template.
 */
class ModbusRegisterMapTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("modbus_register_map"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trMap("Modbus register map"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trMap("Decodes polled Modbus register blocks through a typed register map, with "
                 "scaling per register. Generated by the Modbus map importer; use with the "
                 "Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto blocks = DataModel::nativeParam(
      "blocks",
      NativeParamType::Json,
      QT_TRANSLATE_NOOP("NativeTemplates", "Register blocks"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Polled register blocks with typed, scaled entries. Written by the "
                        "Modbus register map importer."),
      modbusMapDefault());

    return {blocks};
  }

  /**
   * @brief Validates the register map and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    auto json = DataModel::nativeParamArray(params, QStringLiteral("blocks"));
    if (json.isEmpty())
      json = modbusMapDefault();

    QList<ModbusMapBlock> blocks;
    const int channels = parseModbusBlocks(json, blocks);
    if (blocks.isEmpty() || channels < 1) {
      error = trMap("The register map must contain at least one valid register entry.");
      return nullptr;
    }

    return std::make_unique<ModbusRegisterMapParser>(blocks, channels);
  }
};

//--------------------------------------------------------------------------------------------------
// CAN signal map
//--------------------------------------------------------------------------------------------------

/**
 * @brief Multiplex role of a mapped CAN signal.
 */
enum class CanSignalRole : quint8 {
  Plain,
  Selector,
  Muxed,
};

/**
 * @brief One mapped CAN signal: latch channel, bit layout and physical scaling.
 */
struct CanMapSignal {
  int channel        = 0;
  int start_bit      = 0;
  int bit_length     = 1;
  double factor      = 1.0;
  double offset      = 0.0;
  qint64 mux_value   = 0;
  CanSignalRole role = CanSignalRole::Plain;
  bool big_endian    = true;
  bool is_signed     = false;
};

/**
 * @brief One mapped CAN message: frame id and its signals in channel order.
 */
struct CanMapMessage {
  quint32 id = 0;
  QList<CanMapSignal> signal_list;
};

/**
 * @brief Extracts a raw signal value from the payload, mirroring the DBC bit layouts
 * (Motorola MSB-first sawtooth, Intel LSB-first) with Qt's verbatim DBC start bits.
 */
[[nodiscard]] static qint64 extractCanSignal(const QByteArray& frame,
                                             qsizetype payloadStart,
                                             int payloadLen,
                                             const CanMapSignal& signal)
{
  Q_ASSERT(signal.bit_length >= 1);
  Q_ASSERT(signal.bit_length <= 64);

  quint64 value = 0;
  if (signal.big_endian) {
    int bit_pos = signal.start_bit;
    for (int i = 0; i < signal.bit_length; ++i) {
      const int byte_idx = bit_pos / 8;
      if (byte_idx < payloadLen) {
        const int bit = (u8At(frame, payloadStart + byte_idx) >> (bit_pos % 8)) & 0x01;
        value         = (value << 1) | static_cast<quint64>(bit);
      }

      bit_pos = ((bit_pos % 8) == 0) ? (bit_pos + 15) : (bit_pos - 1);
    }
  } else {
    int bits_read = 0;
    int bit_shift = signal.start_bit % 8;
    for (int byte_idx = signal.start_bit / 8;
         byte_idx < payloadLen && bits_read < signal.bit_length;
         ++byte_idx) {
      const int take      = qMin(8 - bit_shift, signal.bit_length - bits_read);
      const quint64 mask  = (quint64(1) << take) - 1;
      const quint64 bits  = (u8At(frame, payloadStart + byte_idx) >> bit_shift) & mask;
      value              |= bits << bits_read;
      bits_read          += take;
      bit_shift           = 0;
    }
  }

  if (signal.is_signed && signal.bit_length < 64
      && (value & (quint64(1) << (signal.bit_length - 1))))
    return static_cast<qint64>(value) - (qint64(1) << signal.bit_length);

  return static_cast<qint64>(value);
}

/**
 * @brief Latching CAN decoder routing frames through a DBC-style signal map. Muxed signals
 * only update when the message's selector raw value matches their mux value.
 */
class CanSignalMapParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the message list and indexes it by CAN id.
   */
  CanSignalMapParser(const QList<CanMapMessage>& messages, int channelCount)
    : NativeLatchParser(channelCount), m_messages(messages)
  {
    Q_ASSERT(!m_messages.isEmpty());

    m_index.reserve(m_messages.size());
    for (qsizetype i = 0; i < m_messages.size(); ++i)
      m_index.insert(m_messages.at(i).id, static_cast<int>(i));
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
   * @brief Decodes one preprocessed CAN frame: [id hi][id lo][DLC][data...].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    Q_ASSERT(!m_messages.isEmpty());

    if (frame.size() < 3)
      return latchedFrame();

    const quint32 can_id = (static_cast<quint32>(u8At(frame, 0)) << 8) | u8At(frame, 1);
    const auto it        = m_index.constFind(can_id);
    if (it == m_index.constEnd())
      return latchedFrame();

    const int dlc = u8At(frame, 2);
    const int payload_len =
      static_cast<int>(qMin<qsizetype>(qMin<qsizetype>(dlc, kMaxCanPayload), frame.size() - 3));

    decodeMessage(m_messages.at(it.value()), frame, payload_len);
    return latchedFrame();
  }

private:
  /**
   * @brief Decodes the message's signals in channel order, gating muxed signals on the
   * selector's raw value (selectors precede their muxed signals in the list).
   */
  void decodeMessage(const CanMapMessage& message, const QByteArray& frame, int payloadLen)
  {
    Q_ASSERT(payloadLen >= 0);
    Q_ASSERT(payloadLen <= kMaxCanPayload);

    qint64 selector_raw = 0;
    bool has_selector   = false;

    for (const auto& signal : message.signal_list) {
      if (signal.role == CanSignalRole::Muxed
          && (!has_selector || selector_raw != signal.mux_value))
        continue;

      const qint64 raw = extractCanSignal(frame, 3, payloadLen, signal);
      if (signal.role == CanSignalRole::Selector) {
        selector_raw = raw;
        has_selector = true;
      }

      const double value = static_cast<double>(raw) * signal.factor + signal.offset;
      storeAt(signal.channel, mapValue(value));
    }
  }

private:
  QList<CanMapMessage> m_messages;
  QHash<quint32, int> m_index;
};

/**
 * @brief Builds the default example map: one message with a single 16-bit signal.
 */
[[nodiscard]] static QJsonArray canMapDefault()
{
  QJsonObject signal;
  signal.insert(QStringLiteral("startBit"), 0);
  signal.insert(QStringLiteral("length"), 16);
  signal.insert(QStringLiteral("bigEndian"), true);

  QJsonObject message;
  message.insert(QStringLiteral("id"), 256);
  message.insert(QStringLiteral("signals"), QJsonArray{signal});
  return QJsonArray{message};
}

/**
 * @brief Parses one signal JSON object into the runtime layout.
 */
[[nodiscard]] static CanMapSignal parseCanSignal(const QJsonObject& obj, int channel)
{
  Q_ASSERT(channel >= 0);

  CanMapSignal signal;
  signal.channel    = channel;
  signal.start_bit  = qBound(0, obj.value(QStringLiteral("startBit")).toInt(), 511);
  signal.bit_length = qBound(1, obj.value(QStringLiteral("length")).toInt(1), 64);
  signal.factor     = SerialStudio::toDouble(obj.value(QStringLiteral("factor")), 1.0);
  signal.offset     = SerialStudio::toDouble(obj.value(QStringLiteral("offset")), 0.0);
  signal.big_endian = obj.value(QStringLiteral("bigEndian")).toBool(true);
  signal.is_signed  = obj.value(QStringLiteral("signed")).toBool(false);

  if (obj.value(QStringLiteral("selector")).toBool(false))
    signal.role = CanSignalRole::Selector;
  else if (obj.contains(QStringLiteral("mux"))) {
    signal.role = CanSignalRole::Muxed;
    signal.mux_value =
      static_cast<qint64>(SerialStudio::toDouble(obj.value(QStringLiteral("mux")), 0.0));
  }

  return signal;
}

/**
 * @brief Parses the messages JSON into the runtime message list; returns the channel count.
 */
[[nodiscard]] static int parseCanMessages(const QJsonArray& json, QList<CanMapMessage>& messages)
{
  int channel                   = 0;
  const qsizetype message_count = qMin<qsizetype>(json.size(), kMaxCanMessages);
  for (qsizetype m = 0; m < message_count; ++m) {
    const auto obj          = json.at(m).toObject();
    const auto signals_json = obj.value(QStringLiteral("signals")).toArray();

    CanMapMessage message;
    message.id = static_cast<quint32>(obj.value(QStringLiteral("id")).toInteger(0));

    for (const auto& item : signals_json) {
      if (channel >= kMaxMapChannels)
        break;

      message.signal_list.append(parseCanSignal(item.toObject(), channel++));
    }

    if (!message.signal_list.isEmpty())
      messages.append(message);
  }

  return channel;
}

/**
 * @brief Descriptor for the CAN signal map template.
 */
class CanSignalMapTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("can_signal_map"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trMap("CAN signal map (DBC)"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trMap("Decodes CAN frames through a DBC-style signal map (bit layout, scaling and "
                 "simple multiplexing). Generated by the DBC importer; use with the Binary "
                 "decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto messages = DataModel::nativeParam(
      "messages",
      NativeParamType::Json,
      QT_TRANSLATE_NOOP("NativeTemplates", "Signal map"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "CAN messages with their signal bit layouts and scaling. Written by "
                        "the DBC importer."),
      canMapDefault());

    return {messages};
  }

  /**
   * @brief Validates the signal map and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    auto json = DataModel::nativeParamArray(params, QStringLiteral("messages"));
    if (json.isEmpty())
      json = canMapDefault();

    QList<CanMapMessage> messages;
    const int channels = parseCanMessages(json, messages);
    if (messages.isEmpty() || channels < 1) {
      error = trMap("The signal map must contain at least one message with one signal.");
      return nullptr;
    }

    return std::make_unique<CanSignalMapParser>(messages, channels);
  }
};

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the protocol-map native templates in display order.
 */
QList<const DataModel::INativeTemplate*> DataModel::mapNativeTemplates()
{
  static const ModbusRegisterMapTemplate s_modbusMap;
  static const CanSignalMapTemplate s_canMap;

  return {&s_modbusMap, &s_canMap};
}
