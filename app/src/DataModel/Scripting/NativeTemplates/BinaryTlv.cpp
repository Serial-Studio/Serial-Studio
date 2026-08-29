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

#include "DataModel/Scripting/NativeTemplates/BinaryTlv.h"

#include <QHash>

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

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
    SS_ASSERT_LOG(!m_tagMap.isEmpty());
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
    SS_ASSERT(!m_tagMap.isEmpty(), return latchedFrame());

    qsizetype i          = 0;
    const qsizetype size = qMin<qsizetype>(frame.size(), kMaxBytesPerFrame);
    for (qsizetype entry = 0; entry < (size / 2) + 1 && i + 1 < size; ++entry) {
      const int tag    = u8At(frame, i++);
      const int length = u8At(frame, i++);
      if (i + length > size)
        break;

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
  [[nodiscard]] QString name() const override
  {
    return trNativeTemplate("Binary TLV (Tag-Length-Value)");
  }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Walks tag/length/value entries (1-byte tag and length, big-endian "
                            "value) and routes tags to channels. Use with the Binary decoder.");
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
      error = trNativeTemplate("The tag routing table must contain at least one tag:index entry.");
      return nullptr;
    }

    return std::make_unique<BinaryTlvParser>(tag_map, max_index + 1);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide binary TLV template descriptor.
 */
const DataModel::INativeTemplate& DataModel::binaryTlvTemplate()
{
  static const BinaryTlvTemplate s_binaryTlv;
  return s_binaryTlv;
}
