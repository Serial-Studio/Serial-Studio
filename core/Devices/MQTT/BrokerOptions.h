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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QMap>
#  include <QMqttClient>
#  include <QSsl>
#  include <QSslSocket>
#  include <QString>
#  include <QStringList>

namespace MQTT {

/**
 * @brief The enumerated broker choices the settings UI binds to: the display list a combo box
 *        shows, and the index<->value mapping that list implies. The index a project file stores is
 *        a position in these tables, so the label-keyed QMap's sorted order is persisted state that
 *        every saved project already refers to. Never swap it for an insertion-ordered container.
 */
class BrokerOptions {
public:
  BrokerOptions();

  void setModes(const QStringList& labels);
  void addMqttVersion(const QString& label, const QMqttClient::ProtocolVersion version);
  void addSslProtocol(const QString& label, const QSsl::SslProtocol protocol);
  void addPeerVerifyMode(const QString& label, const QSslSocket::PeerVerifyMode mode);

  [[nodiscard]] const QStringList& modes() const noexcept;
  [[nodiscard]] const QStringList& mqttVersions() const noexcept;
  [[nodiscard]] const QStringList& sslProtocols() const noexcept;
  [[nodiscard]] const QStringList& peerVerifyModes() const noexcept;

  [[nodiscard]] quint8 mqttVersionIndex(const QMqttClient::ProtocolVersion version) const;
  [[nodiscard]] quint8 sslProtocolIndex(const QSsl::SslProtocol protocol) const;
  [[nodiscard]] quint8 peerVerifyModeIndex(const QSslSocket::PeerVerifyMode mode) const;

  [[nodiscard]] bool mqttVersionAt(const quint8 index, QMqttClient::ProtocolVersion& out) const;
  [[nodiscard]] bool sslProtocolAt(const quint8 index, QSsl::SslProtocol& out) const;
  [[nodiscard]] bool peerVerifyModeAt(const quint8 index, QSslSocket::PeerVerifyMode& out) const;

private:
  QStringList m_modeLabels;
  QStringList m_mqttVersionLabels;
  QStringList m_sslProtocolLabels;
  QStringList m_peerVerifyModeLabels;

  QMap<QString, QSsl::SslProtocol> m_sslProtocols;
  QMap<QString, QMqttClient::ProtocolVersion> m_mqttVersions;
  QMap<QString, QSslSocket::PeerVerifyMode> m_peerVerifyModes;
};

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
