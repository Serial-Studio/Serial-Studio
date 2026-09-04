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

#include <QCoreApplication>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/OpcUaWire.h"
#endif

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;

//--------------------------------------------------------------------------------------------------
// Driver-generated delta frames (specs 0066, 0073)
//--------------------------------------------------------------------------------------------------

#ifdef BUILD_COMMERCIAL

static constexpr int kMaxWireFrameBytes = 65536;

/**
 * @brief Returns the translated UI string for the shared native-template context.
 */
[[nodiscard]] static QString trWire(const char* text)
{
  return QCoreApplication::translate("NativeTemplates", text);
}

/**
 * @brief Base for the driver-generated delta-frame templates: walks the OpcUaWire layout once,
 *        allocation-free, and hands every decoded entry to the routing callable the subclass
 *        supplies. The walk, the entry cap and the latch live here; how an entry maps to a
 *        channel is the subclass' schema interpretation.
 */
class WireLatchParser : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Sizes the latch row to the channel count the schema declares.
   */
  explicit WireLatchParser(int count) : NativeLatchParser(count)
  {
    SS_ASSERT_LOG(count > 0);
    SS_ASSERT_LOG(count <= IO::Drivers::OpcUaWire::kMaxTags);
  }

  /**
   * @brief Treats text frames as raw bytes and reuses the binary path.
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

protected:
  /**
   * @brief Walks [version][index u16][type u8][payload]* and routes every decoded entry through
   *        the caller's callable; a version mismatch or a truncated entry ends the walk, leaving
   *        the latch holding whatever earlier frames stored.
   */
  template<typename Route>
  void walkEntries(const QByteArray& frame, Route&& route)
  {
    using namespace IO::Drivers::OpcUaWire;
    SS_ASSERT_LOG(latchCount() > 0);

    const QByteArrayView view(frame.constData(), qMin<qsizetype>(frame.size(), kMaxWireFrameBytes));
    if (!checkHeader(view))
      return;

    qsizetype pos = kHeaderBytes;
    Entry entry;
    for (int n = 0; n < kMaxTags && pos < view.size(); ++n) {
      if (!readEntry(view, pos, entry))
        break;

      SS_ASSERT_LOG(entry.index >= 0);
      route(entry);
    }
  }
};

/**
 * @brief Latching decoder for schemas that pin an expected wire type per index (OPC UA): an entry
 *        whose type disagrees with the schema is skipped without touching the latch, because a
 *        mistyped channel is a configuration error, not a value.
 */
class TypedWireParser final : public WireLatchParser {
public:
  /**
   * @brief Stores the per-index expected wire type (Invalid = not in schema).
   */
  explicit TypedWireParser(const QList<IO::Drivers::OpcUaWire::Type>& schema)
    : WireLatchParser(static_cast<int>(schema.size())), m_schema(schema)
  {
    SS_ASSERT_LOG(!m_schema.isEmpty());
    SS_ASSERT_LOG(m_schema.size() <= IO::Drivers::OpcUaWire::kMaxTags);
  }

  /**
   * @brief Latches every entry whose index and wire type both match the schema; the version
   *        check, the entry cap and the truncation stop belong to the shared walk.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    SS_ASSERT(!m_schema.isEmpty(), return latchedFrame());
    SS_ASSERT_LOG(m_schema.size() == latchCount());

    walkEntries(frame, [this](const IO::Drivers::OpcUaWire::Entry& entry) {
      if (entry.index < 0 || entry.index >= m_schema.size())
        return;

      if (m_schema.at(entry.index) == entry.type)
        storeAt(entry.index, entry.text);
    });

    return latchedFrame();
  }

private:
  QList<IO::Drivers::OpcUaWire::Type> m_schema;
};

/**
 * @brief Latching decoder for schemas that name channels but do not pin their types (Sparkplug,
 *        S7comm, EtherNet/IP): the driver normalized the type upstream and the wire carries the
 *        authoritative type byte per entry, so only the index is checked.
 */
class NamedWireParser final : public WireLatchParser {
public:
  /**
   * @brief Sizes the latch row to the channel count the schema declares.
   */
  explicit NamedWireParser(int count) : WireLatchParser(count)
  {
    SS_ASSERT_LOG(count > 0);
    SS_ASSERT_LOG(count <= IO::Drivers::OpcUaWire::kMaxTags);
  }

  /**
   * @brief Latches every entry whose index falls within the declared channel count; the version
   *        check, the entry cap and the truncation stop belong to the shared walk.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    SS_ASSERT(latchCount() > 0, return latchedFrame());
    SS_ASSERT_LOG(latchCount() <= IO::Drivers::OpcUaWire::kMaxTags);

    walkEntries(frame, [this](const IO::Drivers::OpcUaWire::Entry& entry) {
      if (entry.index >= 0 && entry.index < latchCount())
        storeAt(entry.index, entry.text);
    });

    return latchedFrame();
  }
};

/**
 * @brief Validates a machine-managed {index, name} schema and returns its channel count, or -1
 *        with @p error set. Shared by every named-schema template so one malformed schema is
 *        refused with the same words wherever it was generated.
 */
[[nodiscard]] static int namedSchemaCount(const QJsonObject& params,
                                          const char* emptyMessage,
                                          QString& error)
{
  using namespace IO::Drivers::OpcUaWire;

  const auto items = DataModel::nativeParamArray(params, QStringLiteral("schema"));
  if (items.isEmpty() || items.size() > kMaxTags) {
    error = trWire(emptyMessage).arg(kMaxTags);
    return -1;
  }

  QList<bool> seen(items.size(), false);
  for (const auto& item : items) {
    const auto obj  = item.toObject();
    const int index = obj.value(QStringLiteral("index")).toInt(-1);
    if (index < 0 || index >= seen.size() || seen[index]) {
      error = trWire("The channel schema has an invalid, duplicate or missing index.");
      return -1;
    }

    seen[index] = true;
  }

  SS_ASSERT_LOG(!items.isEmpty());
  return static_cast<int>(items.size());
}

/**
 * @brief Builds the {index, name} schema parameter every named-schema template declares.
 */
[[nodiscard]] static NativeParamSpec namedSchemaParam(const char* label, const char* description)
{
  return DataModel::nativeParam("schema", NativeParamType::Json, label, description, QJsonArray());
}

//--------------------------------------------------------------------------------------------------
// OPC UA delta frames (spec 0066)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the OPC UA delta-frame template; the schema is machine-managed by the
 *        driver's project generator.
 */
class OpcUaTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("opcua"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trWire("OPC UA tag frames"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trWire("Decodes the OPC UA driver's tag update frames into one latched channel "
                  "per subscribed tag. Generated with the OPC UA project; use with the "
                  "Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto schema = DataModel::nativeParam(
      "schema",
      NativeParamType::Json,
      QT_TRANSLATE_NOOP("NativeTemplates", "Tag schema"),
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "Ordered tag list written by the OPC UA project generator: one "
                        "{\"i\": index, \"t\": type} entry per channel."),
      QJsonArray());
    return {schema};
  }

  /**
   * @brief Validates the schema and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    using namespace IO::Drivers::OpcUaWire;

    const auto items = DataModel::nativeParamArray(params, QStringLiteral("schema"));
    if (items.isEmpty() || items.size() > kMaxTags) {
      error = trWire("The tag schema must list between 1 and %1 tags.").arg(kMaxTags);
      return nullptr;
    }

    QList<Type> schema(items.size(), Type::Invalid);
    for (const auto& item : items) {
      const auto obj  = item.toObject();
      const int index = obj.value(QStringLiteral("i")).toInt(-1);
      const auto type = typeFromCode(obj.value(QStringLiteral("t")).toString());
      if (index < 0 || index >= schema.size() || type == Type::Invalid
          || schema[index] != Type::Invalid) {
        error = trWire("The tag schema has an invalid, duplicate or missing index.");
        return nullptr;
      }

      schema[index] = type;
    }

    return std::make_unique<TypedWireParser>(schema);
  }
};

//--------------------------------------------------------------------------------------------------
// Sparkplug B delta frames (spec 0073)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the Sparkplug B delta-frame template; the schema is machine-managed by
 *        the MQTT driver's project generator.
 */
class SparkplugTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("sparkplug"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trWire("Sparkplug"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trWire("Decodes the delta frames the MQTT driver emits in Sparkplug mode; the "
                  "schema is machine-managed.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    return {namedSchemaParam(QT_TRANSLATE_NOOP("NativeTemplates", "Metric schema"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Ordered metric list written by the MQTT project "
                                               "generator: one {\"index\": index, \"name\": name} "
                                               "entry per channel."))};
  }

  /**
   * @brief Validates the schema and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int count = namedSchemaCount(
      params,
      QT_TRANSLATE_NOOP("NativeTemplates", "The metric schema must list between 1 and %1 metrics."),
      error);
    if (count < 0)
      return nullptr;

    return std::make_unique<NamedWireParser>(count);
  }
};

//--------------------------------------------------------------------------------------------------
// Siemens S7comm delta frames (spec 0073)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the S7comm delta-frame template; the schema is machine-managed by the S7
 *        driver's project generator.
 */
class S7Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("s7"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trWire("Siemens S7 variables"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trWire("Decodes the S7comm driver's variable update frames into one latched channel "
                  "per polled variable. Generated with the S7 project; use with the Binary "
                  "decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    return {namedSchemaParam(QT_TRANSLATE_NOOP("NativeTemplates", "Variable schema"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Ordered variable list written by the S7 project "
                                               "generator: one {\"index\": index, \"name\": name} "
                                               "entry per channel."))};
  }

  /**
   * @brief Validates the schema and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int count = namedSchemaCount(
      params,
      QT_TRANSLATE_NOOP("NativeTemplates",
                        "The variable schema must list between 1 and %1 variables."),
      error);
    if (count < 0)
      return nullptr;

    return std::make_unique<NamedWireParser>(count);
  }
};

//--------------------------------------------------------------------------------------------------
// EtherNet/IP delta frames (spec 0073)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the EtherNet/IP delta-frame template; the schema is machine-managed by
 *        the EtherNet/IP driver's project generator.
 */
class EthernetIpTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("ethernetip"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trWire("EtherNet/IP tags"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trWire("Decodes the EtherNet/IP driver's tag update frames into one latched channel "
                  "per polled tag. Generated with the EtherNet/IP project; use with the Binary "
                  "decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    return {namedSchemaParam(QT_TRANSLATE_NOOP("NativeTemplates", "Tag schema"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Ordered tag list written by the EtherNet/IP "
                                               "project generator: one {\"index\": index, "
                                               "\"name\": name} entry per channel."))};
  }

  /**
   * @brief Validates the schema and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int count = namedSchemaCount(
      params,
      QT_TRANSLATE_NOOP("NativeTemplates", "The tag schema must list between 1 and %1 tags."),
      error);
    if (count < 0)
      return nullptr;

    return std::make_unique<NamedWireParser>(count);
  }
};

//--------------------------------------------------------------------------------------------------
// IEC 60870-5-104 delta frames (spec 0073)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Descriptor for the IEC 60870-5-104 delta-frame template; the schema is machine-managed by
 *        the driver's project generator, whose channel order is the order the station's own
 *        interrogation reply discovered the information objects in.
 */
class Iec104Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("iec104"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trWire("IEC 60870-5-104 points"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trWire("Decodes the IEC 60870-5-104 driver's point update frames into one latched "
                  "channel per information object. Generated with the IEC 60870-5-104 project; "
                  "use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    return {namedSchemaParam(QT_TRANSLATE_NOOP("NativeTemplates", "Point schema"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Ordered point list written by the IEC 60870-5-104 "
                                               "project generator: one {\"index\": index, "
                                               "\"name\": name} entry per channel."))};
  }

  /**
   * @brief Validates the schema and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int count = namedSchemaCount(
      params,
      QT_TRANSLATE_NOOP("NativeTemplates", "The point schema must list between 1 and %1 points."),
      error);
    if (count < 0)
      return nullptr;

    return std::make_unique<NamedWireParser>(count);
  }
};

#endif

//--------------------------------------------------------------------------------------------------
// Family registry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the driver-generated wire-latch templates in display order. Every driver that
 *        discovers its own channel list publishes the same OpcUaWire delta layout, so their
 *        decoders share one base and one translation unit. Empty in a GPL build.
 */
QList<const DataModel::INativeTemplate*> DataModel::wireLatchNativeTemplates()
{
#ifndef BUILD_COMMERCIAL
  return {};
#else
  static const OpcUaTemplate s_opcUa;
  static const SparkplugTemplate s_sparkplug;
  static const S7Template s_s7;
  static const EthernetIpTemplate s_ethernetIp;
  static const Iec104Template s_iec104;

  return {&s_opcUa, &s_sparkplug, &s_s7, &s_ethernetIp, &s_iec104};
#endif
}
