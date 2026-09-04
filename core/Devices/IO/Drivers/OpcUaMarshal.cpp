/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/OpcUaMarshal.h"

#include <QTimeZone>
#include <QVariantList>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Strings and node identifiers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies an open62541 string, which is a length plus a byte pointer and is NOT
 *        null-terminated; reading it as a C string walks off the end.
 */
QString IO::Drivers::OpcUaMarshal::toQString(const UA_String& string)
{
  if (!string.data || string.length == 0)
    return {};

  return QString::fromUtf8(reinterpret_cast<const char*>(string.data),
                           static_cast<qsizetype>(string.length));
}

/**
 * @brief Renders a node id in the textual form the project files and the API speak
 *        ("ns=2;s=Plant.Line1.Level"), using the stack's own printer so numeric, string, GUID and
 *        opaque identifiers all round-trip.
 */
QString IO::Drivers::OpcUaMarshal::nodeIdToString(const UA_NodeId& id)
{
  UA_String text = UA_STRING_NULL;
  if (UA_NodeId_print(&id, &text) != UA_STATUSCODE_GOOD)
    return {};

  const auto out = toQString(text);
  UA_String_clear(&text);
  return out;
}

/**
 * @brief Parses the textual form back into a node id. The caller owns @p out and must clear it.
 */
bool IO::Drivers::OpcUaMarshal::nodeIdFromString(const QString& text, UA_NodeId& out)
{
  UA_NodeId_init(&out);
  const auto utf8 = text.toUtf8();

  UA_String view;
  view.length = static_cast<size_t>(utf8.size());
  view.data   = reinterpret_cast<UA_Byte*>(const_cast<char*>(utf8.constData()));
  return UA_NodeId_parse(&out, view) == UA_STATUSCODE_GOOD;
}

//--------------------------------------------------------------------------------------------------
// Timestamps
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts an OPC UA timestamp (100 ns ticks since 1601-01-01 UTC) to a QDateTime. A zero
 *        tick count is "unset", not the year 1601: it must come back INVALID so the caller can
 *        see the absence instead of recording a plausible-looking 400-year-old stamp.
 */
QDateTime IO::Drivers::OpcUaMarshal::toDateTime(UA_DateTime timestamp)
{
  if (timestamp == 0)
    return {};

  const qint64 msecs = (timestamp - UA_DATETIME_UNIX_EPOCH) / UA_DATETIME_MSEC;
  return QDateTime::fromMSecsSinceEpoch(msecs, QTimeZone::UTC);
}

/**
 * @brief The server's own clock for a value, or an invalid QDateTime when the server did not send
 *        one. Source owns time (spec 0066 R10): losing this silently demotes recordings to the
 *        local monotonic fallback, which is why it is read explicitly and pinned by a unit test.
 */
QDateTime IO::Drivers::OpcUaMarshal::sourceTimeOf(const UA_DataValue& value)
{
  if (!value.hasSourceTimestamp)
    return {};

  return toDateTime(value.sourceTimestamp);
}

//--------------------------------------------------------------------------------------------------
// Values
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts one builtin scalar at @p data, dispatched on the type's typeKind. The builtin
 *        kind values coincide with the UA_TYPES indices, but typeKind is the field open62541
 *        actually guarantees: UA_DataType carries no type index.
 */
static QVariant scalarAt(const void* data, quint32 kind)
{
  SS_ASSERT(data != nullptr, return {});

  switch (kind) {
    case UA_DATATYPEKIND_BOOLEAN:
      return QVariant(*static_cast<const UA_Boolean*>(data));
    case UA_DATATYPEKIND_SBYTE:
      return QVariant(static_cast<int>(*static_cast<const UA_SByte*>(data)));
    case UA_DATATYPEKIND_BYTE:
      return QVariant(static_cast<uint>(*static_cast<const UA_Byte*>(data)));
    case UA_DATATYPEKIND_INT16:
      return QVariant(static_cast<int>(*static_cast<const UA_Int16*>(data)));
    case UA_DATATYPEKIND_UINT16:
      return QVariant(static_cast<uint>(*static_cast<const UA_UInt16*>(data)));
    case UA_DATATYPEKIND_INT32:
      return QVariant(static_cast<int>(*static_cast<const UA_Int32*>(data)));
    case UA_DATATYPEKIND_UINT32:
      return QVariant(static_cast<uint>(*static_cast<const UA_UInt32*>(data)));
    case UA_DATATYPEKIND_INT64:
      return QVariant(static_cast<qint64>(*static_cast<const UA_Int64*>(data)));
    case UA_DATATYPEKIND_UINT64:
      return QVariant(static_cast<quint64>(*static_cast<const UA_UInt64*>(data)));
    case UA_DATATYPEKIND_FLOAT:
      return QVariant(static_cast<float>(*static_cast<const UA_Float*>(data)));
    case UA_DATATYPEKIND_DOUBLE:
      return QVariant(static_cast<double>(*static_cast<const UA_Double*>(data)));
    case UA_DATATYPEKIND_STATUSCODE:
      return QVariant(static_cast<uint>(*static_cast<const UA_StatusCode*>(data)));
    default:
      break;
  }

  return {};
}

/**
 * @brief Renders the OPC UA wrappers the dashboard has no vocabulary for: LocalizedText and
 *        EUInformation become display text, Range becomes {low, high} because both bounds become
 *        a generated dataset's plot range. Both of the latter are plain STRUCTUREs, so only the
 *        type pointer separates them.
 */
static QVariant wrapperAt(const void* data, const UA_DataType* type)
{
  SS_ASSERT(data != nullptr, return {});
  SS_ASSERT(type != nullptr, return {});
  using namespace IO::Drivers::OpcUaMarshal;

  switch (type->typeKind) {
    case UA_DATATYPEKIND_STRING:
    case UA_DATATYPEKIND_BYTESTRING:
    case UA_DATATYPEKIND_XMLELEMENT:
      return toQString(*static_cast<const UA_String*>(data));
    case UA_DATATYPEKIND_DATETIME:
      return toDateTime(*static_cast<const UA_DateTime*>(data));
    case UA_DATATYPEKIND_LOCALIZEDTEXT:
      return toQString(static_cast<const UA_LocalizedText*>(data)->text);
    case UA_DATATYPEKIND_QUALIFIEDNAME:
      return toQString(static_cast<const UA_QualifiedName*>(data)->name);
    case UA_DATATYPEKIND_NODEID:
      return nodeIdToString(*static_cast<const UA_NodeId*>(data));
    default:
      break;
  }

  if (type == &UA_TYPES[UA_TYPES_EUINFORMATION])
    return toQString(static_cast<const UA_EUInformation*>(data)->displayName.text);

  if (type == &UA_TYPES[UA_TYPES_RANGE]) {
    const auto* range = static_cast<const UA_Range*>(data);
    return QVariantList{range->low, range->high};
  }

  return {};
}

/**
 * @brief Converts one element of @p variant at @p index.
 */
static QVariant elementOf(const UA_Variant& variant, size_t index)
{
  SS_ASSERT(variant.type != nullptr, return {});

  const auto* bytes = static_cast<const quint8*>(variant.data);
  SS_ASSERT(bytes != nullptr, return {});

  const void* element = bytes + (index * variant.type->memSize);

  const auto scalar = scalarAt(element, variant.type->typeKind);
  if (scalar.isValid())
    return scalar;

  return wrapperAt(element, variant.type);
}

/**
 * @brief Converts a value to a QVariant. An array becomes a QVariantList, which is the shape the
 *        driver's slot cache branches on to fan elements out across wire indices; collapsing one
 *        to a scalar would silently latch only its first element.
 */
QVariant IO::Drivers::OpcUaMarshal::toVariant(const UA_Variant& variant)
{
  if (!variant.type || !variant.data)
    return {};

  if (UA_Variant_isScalar(&variant))
    return elementOf(variant, 0);

  QVariantList list;
  list.reserve(static_cast<qsizetype>(variant.arrayLength));
  for (size_t i = 0; i < variant.arrayLength; ++i)
    list.append(elementOf(variant, i));

  return list;
}

//--------------------------------------------------------------------------------------------------
// Status and enumerations
//--------------------------------------------------------------------------------------------------

/**
 * @brief The stack's symbolic name for a status code, for logs and the diagnostics snapshot.
 */
QString IO::Drivers::OpcUaMarshal::statusText(OpcUaTypes::StatusCode status)
{
  return QString::fromLatin1(UA_StatusCode_name(static_cast<UA_StatusCode>(status)));
}

/**
 * @brief Maps an OPC UA node-class mask onto the driver's enumeration.
 */
IO::Drivers::OpcUaTypes::NodeClass IO::Drivers::OpcUaMarshal::toNodeClass(quint32 mask) noexcept
{
  return static_cast<OpcUaTypes::NodeClass>(mask);
}

/**
 * @brief Maps a message-security-mode value onto the driver's enumeration; anything unexpected is
 *        Invalid rather than silently treated as None.
 */
IO::Drivers::OpcUaTypes::SecurityMode IO::Drivers::OpcUaMarshal::toSecurityMode(int mode) noexcept
{
  if (mode < static_cast<int>(OpcUaTypes::SecurityMode::None)
      || mode > static_cast<int>(OpcUaTypes::SecurityMode::SignAndEncrypt))
    return OpcUaTypes::SecurityMode::Invalid;

  return static_cast<OpcUaTypes::SecurityMode>(mode);
}

/**
 * @brief Maps a user-token-type value onto the driver's enumeration; an unknown token is reported
 *        as Anonymous so an endpoint listing never claims credentials it cannot present.
 */
IO::Drivers::OpcUaTypes::UserTokenType IO::Drivers::OpcUaMarshal::toUserTokenType(int type) noexcept
{
  if (type < static_cast<int>(OpcUaTypes::UserTokenType::Anonymous)
      || type > static_cast<int>(OpcUaTypes::UserTokenType::IssuedToken))
    return OpcUaTypes::UserTokenType::Anonymous;

  return static_cast<OpcUaTypes::UserTokenType>(type);
}
