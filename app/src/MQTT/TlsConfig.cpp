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

#  include "MQTT/TlsConfig.h"

#  include <QDir>
#  include <QFile>
#  include <QFileInfo>

#  include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the safe defaults a publisher starts from: TLS off, but configured so that the
 *        moment it is switched on the handshake verifies the peer against the system trust store.
 *        The ALPN protocol defaults to the AWS IoT name, which is the port-443 case users hit.
 */
MQTT::TlsConfig::TlsConfig()
  : m_enabled(false)
  , m_alpnEnabled(false)
  , m_peerVerifyDepth(10)
  , m_protocol(QSsl::SecureProtocols)
  , m_peerVerifyMode(QSslSocket::AutoVerifyPeer)
  , m_alpnProtocol(QStringLiteral("x-amzn-mqtt-ca"))
{}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the broker connection is opened over TLS.
 */
bool MQTT::TlsConfig::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns whether ALPN is announced during the TLS handshake (MQTT over port 443).
 */
bool MQTT::TlsConfig::alpnEnabled() const noexcept
{
  return m_alpnEnabled;
}

/**
 * @brief Returns the peer-verification chain depth.
 */
int MQTT::TlsConfig::peerVerifyDepth() const noexcept
{
  return m_peerVerifyDepth;
}

/**
 * @brief Returns the negotiated TLS/DTLS protocol selection.
 */
QSsl::SslProtocol MQTT::TlsConfig::protocol() const noexcept
{
  return m_protocol;
}

/**
 * @brief Returns the peer-verification mode applied to the broker's certificate.
 */
QSslSocket::PeerVerifyMode MQTT::TlsConfig::peerVerifyMode() const noexcept
{
  return m_peerVerifyMode;
}

/**
 * @brief Returns the ALPN protocol name announced when ALPN is enabled.
 */
QString MQTT::TlsConfig::alpnProtocol() const
{
  return m_alpnProtocol;
}

/**
 * @brief Returns the client certificate PEM path used for mutual TLS (empty = off).
 */
QString MQTT::TlsConfig::certificatePath() const
{
  return m_certificatePath;
}

/**
 * @brief Returns the private key PEM path (empty = look in the certificate file).
 */
QString MQTT::TlsConfig::privateKeyPath() const
{
  return m_privateKeyPath;
}

/**
 * @brief Returns the bytes the handshake announces, or an empty array when ALPN is off. Empty is
 *        load-bearing: applyTlsIdentity() clears the protocol list on it, so disabling ALPN really
 *        removes the announcement instead of leaving the previous one configured.
 */
QByteArray MQTT::TlsConfig::alpnPayload() const
{
  if (!m_alpnEnabled)
    return QByteArray();

  return m_alpnProtocol.toUtf8();
}

/**
 * @brief Returns the parsed mutual-TLS identity; null halves mean "no client identity".
 */
const MQTT::TlsIdentity& MQTT::TlsConfig::identity() const noexcept
{
  return m_identity;
}

/**
 * @brief Returns the extra CA certificates the user trusted on top of the system store.
 */
const QList<QSslCertificate>& MQTT::TlsConfig::caCertificates() const noexcept
{
  return m_caCertificates;
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables TLS for the broker connection.
 */
void MQTT::TlsConfig::setEnabled(const bool enabled)
{
  m_enabled = enabled;
}

/**
 * @brief Enables or disables the ALPN announcement.
 */
void MQTT::TlsConfig::setAlpnEnabled(const bool enabled)
{
  m_alpnEnabled = enabled;
}

/**
 * @brief Sets the peer-verification chain depth.
 */
void MQTT::TlsConfig::setPeerVerifyDepth(const int depth)
{
  m_peerVerifyDepth = depth;
}

/**
 * @brief Sets the TLS/DTLS protocol selection.
 */
void MQTT::TlsConfig::setProtocol(const QSsl::SslProtocol protocol)
{
  m_protocol = protocol;
}

/**
 * @brief Sets the peer-verification mode.
 */
void MQTT::TlsConfig::setPeerVerifyMode(const QSslSocket::PeerVerifyMode mode)
{
  m_peerVerifyMode = mode;
}

/**
 * @brief Sets the ALPN protocol name (AWS IoT uses "x-amzn-mqtt-ca" on port 443).
 */
void MQTT::TlsConfig::setAlpnProtocol(const QString& protocol)
{
  m_alpnProtocol = protocol;
}

/**
 * @brief Sets the client certificate PEM path; the caller re-parses the identity once its path
 *        state is final, because parsing here would run against a half-restored pair.
 */
void MQTT::TlsConfig::setCertificatePath(const QString& path)
{
  m_certificatePath = path;
}

/**
 * @brief Sets the private key PEM path; see setCertificatePath() for why parsing is the caller's.
 */
void MQTT::TlsConfig::setPrivateKeyPath(const QString& path)
{
  m_privateKeyPath = path;
}

/**
 * @brief Adds every certificate found in a directory of PEM/DER files to the trusted set. Both
 *        encodings are attempted on each file because the extension does not tell them apart, and
 *        a certificate already trusted is not added twice.
 */
void MQTT::TlsConfig::addCaCertificatesFromDirectory(const QString& path)
{
  SS_ASSERT(!path.isEmpty(), return);

  QDir dir(path);
  if (!dir.exists())
    return;

  const auto entries = dir.entryInfoList({"*.pem", "*.crt", "*.cer"}, QDir::Files | QDir::Readable);
  for (const auto& info : entries) {
    QFile file(info.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
      continue;

    const auto data = file.readAll();
    const auto pem  = QSslCertificate::fromData(data, QSsl::Pem);
    const auto der  = QSslCertificate::fromData(data, QSsl::Der);
    for (const auto& cert : pem)
      if (!cert.isNull() && !m_caCertificates.contains(cert))
        m_caCertificates.append(cert);

    for (const auto& cert : der)
      if (!cert.isNull() && !m_caCertificates.contains(cert))
        m_caCertificates.append(cert);
  }
}

/**
 * @brief Re-parses the certificate + key pair from the configured paths under @p passphrase. A
 *        failed parse leaves the identity cleared by loadTlsIdentity(), so a stale pair is never
 *        sent to a broker after the user pointed the publisher at a different one.
 */
MQTT::TlsIdentityResult MQTT::TlsConfig::reloadIdentity(const QString& passphrase)
{
  return loadTlsIdentity(m_certificatePath, m_privateKeyPath, passphrase, m_identity);
}

#endif  // BUILD_COMMERCIAL
