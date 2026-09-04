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

#pragma once

#include <cstdint>
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>

namespace IO {
namespace Drivers {

/**
 * @brief Transport-neutral OPC UA vocabulary (spec 0067). The driver and the tag browser speak
 *        these types; only OpcUaSession and OpcUaMarshal ever see an open62541 type. Qt Core
 *        only on purpose, so the ctest tier can link it without the protocol stack.
 */
namespace OpcUaTypes {

/**
 * @brief A raw OPC UA status code. The numeric value is the wire value, so it survives the trip
 *        from the stack unmodified and the severity bits below still mean what the spec says.
 */
using StatusCode = std::uint32_t;

// The IANA-assigned OPC UA port, used wherever a URL omits one
inline constexpr int kDefaultPort = 4840;

inline constexpr StatusCode kStatusGood          = 0x00000000u;
inline constexpr StatusCode kStatusBadInternal   = 0x80020000u;
inline constexpr StatusCode kStatusBadTimeout    = 0x800A0000u;
inline constexpr StatusCode kStatusSeverityMask  = 0xC0000000u;
inline constexpr StatusCode kStatusUncertainBase = 0x40000000u;
inline constexpr StatusCode kStatusBadBase       = 0x80000000u;

/**
 * @brief Quality is decided by the two severity bits, never by "!= Good": an Uncertain value is
 *        real data a PLC flagged, and dropping it silently flat-lines the dashboard.
 */
[[nodiscard]] inline constexpr StatusCode severityOf(StatusCode code) noexcept
{
  return code & kStatusSeverityMask;
}

[[nodiscard]] inline constexpr bool isGood(StatusCode code) noexcept
{
  return severityOf(code) == kStatusGood;
}

[[nodiscard]] inline constexpr bool isUncertain(StatusCode code) noexcept
{
  return severityOf(code) == kStatusUncertainBase;
}

[[nodiscard]] inline constexpr bool isBad(StatusCode code) noexcept
{
  return severityOf(code) >= kStatusBadBase;
}

/**
 * @brief Node attributes the driver and the tag browser read. Values match the OPC UA attribute
 *        ids so the marshal layer is a cast rather than a table.
 */
enum class NodeAttribute : std::uint32_t {
  NodeId          = 1,
  NodeClass       = 2,
  BrowseName      = 3,
  DisplayName     = 4,
  Description     = 5,
  Value           = 13,
  DataType        = 14,
  ValueRank       = 15,
  ArrayDimensions = 16,
  AccessLevel     = 17,
  UserAccessLevel = 18,
};

/**
 * @brief Node classes, as a bit mask so a browse can ask for several at once.
 */
enum class NodeClass : std::uint32_t {
  Unspecified   = 0,
  Object        = 1,
  Variable      = 2,
  Method        = 4,
  ObjectType    = 8,
  VariableType  = 16,
  ReferenceType = 32,
  DataType      = 64,
  View          = 128,
};

[[nodiscard]] inline constexpr NodeClass operator|(NodeClass a, NodeClass b) noexcept
{
  return static_cast<NodeClass>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

[[nodiscard]] inline constexpr bool testFlag(NodeClass mask, NodeClass flag) noexcept
{
  return (static_cast<std::uint32_t>(mask) & static_cast<std::uint32_t>(flag)) != 0;
}

/**
 * @brief The access-level bit that says a variable can be read at all.
 */
inline constexpr std::uint32_t kAccessLevelCurrentRead = 0x01u;

/**
 * @brief The two reference sets the picker walks: the hierarchy for the tree itself, and
 *        HasProperty for the EngineeringUnits/EURange lookup of a ticked variable.
 */
enum class ReferenceKind : std::uint8_t {
  Hierarchical,
  HasProperty,
};

/**
 * @brief One browse, plus the token the caller gets back with the reply. A node can be browsed
 *        for three different reasons at once (level expansion, the has-children probe, a units
 *        lookup), so the node id alone cannot route the answer.
 */
struct BrowseQuery {
  std::uint32_t token;
  ReferenceKind kind;
  NodeClass nodeClassMask;

  BrowseQuery()
    : token(0)
    , kind(ReferenceKind::Hierarchical)
    , nodeClassMask(NodeClass::Object | NodeClass::Variable)
  {}
};

/**
 * @brief Well-known namespace-0 nodes the session and the picker read directly.
 */
inline constexpr const char* kNodeNamespaceArray  = "ns=0;i=2255";
inline constexpr const char* kNodeMaxNodesPerRead = "ns=0;i=11705";

/**
 * @brief Channel security mode. None is the only one spec 0066 could open; the rest arrive with
 *        the secure-channel stage of spec 0067.
 */
enum class SecurityMode : std::uint8_t {
  Invalid        = 0,
  None           = 1,
  Sign           = 2,
  SignAndEncrypt = 3,
};

/**
 * @brief User identity token types an endpoint can advertise.
 */
enum class UserTokenType : std::uint8_t {
  Anonymous   = 0,
  Username    = 1,
  Certificate = 2,
  IssuedToken = 3,
};

/**
 * @brief One endpoint a server advertises. `securityLevel` is the server's own ranking, used to
 *        pick a default; the policy stays a URI string because the set is open-ended.
 */
struct Endpoint {
  QString endpointUrl;
  QString securityPolicyUri;
  QString applicationUri;
  QByteArray serverCertificate;
  QList<UserTokenType> userTokenTypes;
  SecurityMode securityMode;
  std::uint8_t securityLevel;

  Endpoint() : securityMode(SecurityMode::Invalid), securityLevel(0) {}
};

/**
 * @brief One row from a browse: the child node and enough of its identity to build a tree row.
 */
struct ReferenceRow {
  QString nodeId;
  QString namespaceUri;
  QString browseName;
  QString displayName;
  NodeClass nodeClass;

  ReferenceRow() : nodeClass(NodeClass::Unspecified) {}
};

/**
 * @brief One attribute value from a read. `sourceTimestamp` is the PLC's own clock and is the
 *        stamp the recording keeps: never substitute the local clock for it here.
 */
struct ReadRow {
  QString nodeId;
  QVariant value;
  QDateTime sourceTimestamp;
  QDateTime serverTimestamp;
  NodeAttribute attribute;
  StatusCode status;

  ReadRow() : attribute(NodeAttribute::Value), status(kStatusGood) {}
};

/**
 * @brief One monitored-item notification, carrying the tag index the subscription was created
 *        with so the driver does not have to map node ids on the value path.
 */
struct MonitoredValue {
  QVariant value;
  QDateTime sourceTimestamp;
  QDateTime serverTimestamp;
  int tag;
  StatusCode status;

  MonitoredValue() : tag(-1), status(kStatusGood) {}
};

/**
 * @brief What a certificate is and why it was not accepted, for the trust prompt. Populated by
 *        the security layer in the secure-channel stage.
 */
struct CertInfo {
  QByteArray certificate;
  QString subject;
  QString issuer;
  QString fingerprint;
  QString applicationUri;
  QDateTime notBefore;
  QDateTime notAfter;
  bool valid;
  bool trusted;
  bool hostnameMatches;
  bool expired;
  bool notYetValid;

  CertInfo()
    : valid(false), trusted(false), hostnameMatches(false), expired(false), notYetValid(false)
  {}
};

/**
 * @brief Why a secure channel could not be opened, kept distinct so the user is told what to fix
 *        rather than a single "certificate error" that could mean any of four different things.
 */
enum class TrustFailure : std::uint8_t {
  None,
  Untrusted,
  Expired,
  NotYetValid,
  HostnameMismatch,
  Unreadable,
};

}  // namespace OpcUaTypes
}  // namespace Drivers
}  // namespace IO
