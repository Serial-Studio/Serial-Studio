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

#  include <QByteArray>
#  include <QList>
#  include <QSsl>
#  include <QSslCertificate>
#  include <QSslSocket>
#  include <QString>

#  include "MQTT/TlsIdentity.h"

namespace MQTT {

/**
 * @brief The publisher's transport-security state: the handshake parameters, the mutual-TLS
 *        identity paths and their parsed pair, and the extra CA certificates the user trusted. The
 *        private-key passphrase deliberately does NOT live here: it is vault-backed material owned
 *        by the facade, so no copy is retained beside the paths written into project files.
 */
class TlsConfig {
public:
  TlsConfig();

  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool alpnEnabled() const noexcept;
  [[nodiscard]] int peerVerifyDepth() const noexcept;
  [[nodiscard]] QSsl::SslProtocol protocol() const noexcept;
  [[nodiscard]] QSslSocket::PeerVerifyMode peerVerifyMode() const noexcept;

  [[nodiscard]] QString alpnProtocol() const;
  [[nodiscard]] QString certificatePath() const;
  [[nodiscard]] QString privateKeyPath() const;
  [[nodiscard]] QByteArray alpnPayload() const;

  [[nodiscard]] const TlsIdentity& identity() const noexcept;
  [[nodiscard]] const QList<QSslCertificate>& caCertificates() const noexcept;

  void setEnabled(const bool enabled);
  void setAlpnEnabled(const bool enabled);
  void setPeerVerifyDepth(const int depth);
  void setProtocol(const QSsl::SslProtocol protocol);
  void setPeerVerifyMode(const QSslSocket::PeerVerifyMode mode);
  void setAlpnProtocol(const QString& protocol);
  void setCertificatePath(const QString& path);
  void setPrivateKeyPath(const QString& path);

  void addCaCertificatesFromDirectory(const QString& path);
  [[nodiscard]] TlsIdentityResult reloadIdentity(const QString& passphrase);

private:
  bool m_enabled;
  bool m_alpnEnabled;
  int m_peerVerifyDepth;
  QSsl::SslProtocol m_protocol;
  QSslSocket::PeerVerifyMode m_peerVerifyMode;

  QString m_alpnProtocol;
  QString m_certificatePath;
  QString m_privateKeyPath;

  TlsIdentity m_identity;
  QList<QSslCertificate> m_caCertificates;
};

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
