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

#include "DataModel/Scripting/NativeTemplates/BinaryMessagePack.h"

#include <QHash>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!mapMode || !m_keys.isEmpty());

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
    SS_ASSERT(!frame.isEmpty(), return latchedFrame());

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
    SS_ASSERT(length >= 0, length = 0);
    SS_ASSERT(length <= 31, length = 31);

    const qsizetype take  = qMax<qsizetype>(0, qMin<qsizetype>(length, data.size() - offset));
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
  [[nodiscard]] QString name() const override { return trNativeTemplate("MessagePack data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a MessagePack array (one channel per element) or map (keys "
                            "routed to channels). Use with the Binary decoder.");
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
    mode.optionLabels = {trNativeTemplate("Array"), trNativeTemplate("Map")};

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
      error = trNativeTemplate("Map mode requires at least one key.");
      return nullptr;
    }

    return std::make_unique<MessagePackParser>(map_mode, keys);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide MessagePack template descriptor.
 */
const DataModel::INativeTemplate& DataModel::messagePackTemplate()
{
  static const MessagePackTemplate s_messagePack;
  return s_messagePack;
}
