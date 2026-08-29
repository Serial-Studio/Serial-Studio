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

#include "DataModel/Scripting/NativeTemplates/BinaryModbus.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(numItems >= 1);
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
    SS_ASSERT(frame.size() >= 3, return);

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
    SS_ASSERT(frame.size() >= 3, return);

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
    SS_ASSERT(frame.size() >= 6, return);

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
  [[nodiscard]] QString name() const override { return trNativeTemplate("Modbus frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes Modbus responses (coils, registers and single-write echoes) "
                            "into channels. Use with the Binary decoder.");
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
      error = trNativeTemplate("The channel count must be between 1 and 256.");
      return nullptr;
    }

    const int offset = DataModel::nativeParamInt(params, QStringLiteral("registerOffset"), 0);
    const bool sign  = DataModel::nativeParamBool(params, QStringLiteral("signedRegisters"), false);
    return std::make_unique<ModbusParser>(items, offset, sign);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide Modbus template descriptor.
 */
const DataModel::INativeTemplate& DataModel::modbusTemplate()
{
  static const ModbusTemplate s_modbus;
  return s_modbus;
}
