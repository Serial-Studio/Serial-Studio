/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "MQTT/BrokerOptions.h"

#  include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Position of @p value in a label-sorted table, or the table size when it holds no such
 *        entry. Out-of-range is the historical answer for an unmapped value: a combo box clamps it
 *        and the configuration is left untouched, which is what a stale index should do.
 */
template<typename T>
[[nodiscard]] static quint8 indexOfValue(const QMap<QString, T>& table, const T value)
{
  SS_ASSERT_LOG(table.size() <= 255);

  quint8 index = 0;
  for (auto i = table.begin(); i != table.end(); ++i) {
    if (i.value() == value)
      break;

    ++index;
  }

  return index;
}

/**
 * @brief Reads the value at @p index of a label-sorted table; false when the table is shorter.
 */
template<typename T>
[[nodiscard]] static bool valueAtIndex(const QMap<QString, T>& table, const quint8 index, T& out)
{
  SS_ASSERT_LOG(table.size() <= 255);

  quint8 current = 0;
  for (auto i = table.begin(); i != table.end(); ++i) {
    if (current == index) {
      out = i.value();
      return true;
    }

    ++current;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Construction and registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty option set; the owner registers the localized entries.
 */
MQTT::BrokerOptions::BrokerOptions() {}

/**
 * @brief Adopts the publisher mode labels, in Mode-enum order.
 */
void MQTT::BrokerOptions::setModes(const QStringList& labels)
{
  SS_ASSERT(!labels.isEmpty(), return);

  m_modeLabels = labels;
}

/**
 * @brief Registers one MQTT protocol version under its display label.
 */
void MQTT::BrokerOptions::addMqttVersion(const QString& label,
                                         const QMqttClient::ProtocolVersion version)
{
  SS_ASSERT(!label.isEmpty(), return);

  m_mqttVersions.insert(label, version);
  m_mqttVersionLabels = m_mqttVersions.keys();
}

/**
 * @brief Registers one TLS/DTLS protocol under its display label.
 */
void MQTT::BrokerOptions::addSslProtocol(const QString& label, const QSsl::SslProtocol protocol)
{
  SS_ASSERT(!label.isEmpty(), return);

  m_sslProtocols.insert(label, protocol);
  m_sslProtocolLabels = m_sslProtocols.keys();
}

/**
 * @brief Registers one peer-verification mode under its display label.
 */
void MQTT::BrokerOptions::addPeerVerifyMode(const QString& label,
                                            const QSslSocket::PeerVerifyMode mode)
{
  SS_ASSERT(!label.isEmpty(), return);

  m_peerVerifyModes.insert(label, mode);
  m_peerVerifyModeLabels = m_peerVerifyModes.keys();
}

//--------------------------------------------------------------------------------------------------
// Display lists
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the publisher mode labels.
 */
const QStringList& MQTT::BrokerOptions::modes() const noexcept
{
  return m_modeLabels;
}

/**
 * @brief Returns the MQTT protocol version labels, in index order.
 */
const QStringList& MQTT::BrokerOptions::mqttVersions() const noexcept
{
  return m_mqttVersionLabels;
}

/**
 * @brief Returns the TLS/DTLS protocol labels, in index order.
 */
const QStringList& MQTT::BrokerOptions::sslProtocols() const noexcept
{
  return m_sslProtocolLabels;
}

/**
 * @brief Returns the peer-verification mode labels, in index order.
 */
const QStringList& MQTT::BrokerOptions::peerVerifyModes() const noexcept
{
  return m_peerVerifyModeLabels;
}

//--------------------------------------------------------------------------------------------------
// Index mapping
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list position of an MQTT protocol version.
 */
quint8 MQTT::BrokerOptions::mqttVersionIndex(const QMqttClient::ProtocolVersion version) const
{
  return indexOfValue(m_mqttVersions, version);
}

/**
 * @brief Returns the list position of a TLS/DTLS protocol.
 */
quint8 MQTT::BrokerOptions::sslProtocolIndex(const QSsl::SslProtocol protocol) const
{
  return indexOfValue(m_sslProtocols, protocol);
}

/**
 * @brief Returns the list position of a peer-verification mode.
 */
quint8 MQTT::BrokerOptions::peerVerifyModeIndex(const QSslSocket::PeerVerifyMode mode) const
{
  return indexOfValue(m_peerVerifyModes, mode);
}

/**
 * @brief Resolves a list position to an MQTT protocol version.
 */
bool MQTT::BrokerOptions::mqttVersionAt(const quint8 index, QMqttClient::ProtocolVersion& out) const
{
  return valueAtIndex(m_mqttVersions, index, out);
}

/**
 * @brief Resolves a list position to a TLS/DTLS protocol.
 */
bool MQTT::BrokerOptions::sslProtocolAt(const quint8 index, QSsl::SslProtocol& out) const
{
  return valueAtIndex(m_sslProtocols, index, out);
}

/**
 * @brief Resolves a list position to a peer-verification mode.
 */
bool MQTT::BrokerOptions::peerVerifyModeAt(const quint8 index,
                                           QSslSocket::PeerVerifyMode& out) const
{
  return valueAtIndex(m_peerVerifyModes, index, out);
}

#endif  // BUILD_COMMERCIAL
