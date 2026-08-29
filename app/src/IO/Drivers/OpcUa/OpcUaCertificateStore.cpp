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

#include "IO/Drivers/OpcUa/OpcUaCertificateStore.h"

#include <QByteArray>
#include <QUrl>

#include "IO/Drivers/OpcUaSecurity.h"
#include "IO/Drivers/OpcUaSession.h"

/**
 * @brief Constructs the store; the client identity itself is materialized on first secure use.
 */
IO::Drivers::OpcUaCertificateStore::OpcUaCertificateStore(QObject* parent) : QObject(parent) {}

/**
 * @brief Materializes the installation's client certificate before a secure dial needs it, and
 *        republishes it. Generating an RSA-2048 identity is a prime search that can run for
 *        seconds, and leaving it to the first handshake put that inside DeviceManager's
 *        synchronous open() call, freezing the window mid-dial.
 */
bool IO::Drivers::OpcUaCertificateStore::ensureClientIdentity()
{
  QByteArray certificate;
  QByteArray key;
  const bool ok = OpcUaSecurity::ensureClientIdentity(certificate, key);

  Q_EMIT certificateChanged();
  return ok;
}

/**
 * @brief Replaces the installation's client certificate and key. Every server that trusted the
 *        old one has to trust the new one, so this is a deliberate action and never automatic.
 */
bool IO::Drivers::OpcUaCertificateStore::regenerateCertificate()
{
  const bool ok = OpcUaSecurity::regenerateClientIdentity();
  if (ok)
    Q_EMIT certificateChanged();

  return ok;
}

/**
 * @brief Writes the client certificate where the user asked, to hand to the server's trust store.
 */
bool IO::Drivers::OpcUaCertificateStore::exportCertificate(const QString& path) const
{
  return OpcUaSecurity::exportClientCertificate(localPath(path));
}

/**
 * @brief Withdraws a previously accepted server certificate.
 */
bool IO::Drivers::OpcUaCertificateStore::revokeServerCertificate(const QString& fingerprint)
{
  const bool ok = OpcUaSecurity::revokeTrust(fingerprint);
  if (ok)
    Q_EMIT certificateChanged();

  return ok;
}

/**
 * @brief Accepts the server certificate the last attempt was refused over, and only when the
 *        caller names it: the fingerprint is the confirmation token for a security decision, so an
 *        empty one is a mismatch rather than a wildcard. This does NOT retry -- a trust decision
 *        followed by a connect is a NEW attempt with its own single verdict.
 */
bool IO::Drivers::OpcUaCertificateStore::trustCertificate(const OpcUaTypes::CertInfo& pending,
                                                          const QString& fingerprint)
{
  if (fingerprint.isEmpty() || pending.fingerprint.compare(fingerprint, Qt::CaseInsensitive) != 0)
    return false;

  if (!OpcUaSecurity::trustCertificate(pending.certificate))
    return false;

  Q_EMIT certificateChanged();
  return true;
}

/**
 * @brief Records a rejected server certificate and publishes it so the pane can offer the trust
 *        prompt, returning the reason for the caller's error state. Emitted QUEUED: a modal opened
 *        synchronously from inside the dial's error path would spin a nested event loop in the
 *        middle of an emission (the macOS file-dialog reentrancy class).
 */
QString IO::Drivers::OpcUaCertificateStore::captureTrustFailure(const OpcUaSession* session)
{
  if (!session || session->trustFailure() == OpcUaTypes::TrustFailure::None)
    return {};

  m_pendingTrust    = session->serverCertificate();
  const auto detail = describeTrustFailure(session->trustFailure());
  const auto map    = certificateMap(m_pendingTrust);

  QMetaObject::invokeMethod(
    this,
    [this, map, detail] { Q_EMIT serverCertificateUntrusted(map, detail); },
    Qt::QueuedConnection);

  return detail;
}

/**
 * @brief The certificate the last refused attempt was refused over.
 */
const IO::Drivers::OpcUaTypes::CertInfo& IO::Drivers::OpcUaCertificateStore::pendingTrust()
  const noexcept
{
  return m_pendingTrust;
}

/**
 * @brief The installation's own client certificate, generated on first secure use.
 */
QVariantMap IO::Drivers::OpcUaCertificateStore::clientCertificate() const
{
  return certificateMap(OpcUaSecurity::inspect(OpcUaSecurity::clientCertificate(), QString()));
}

/**
 * @brief The installation's client certificate, for the API.
 */
QJsonObject IO::Drivers::OpcUaCertificateStore::certificateJson() const
{
  return certificateObject(OpcUaSecurity::inspect(OpcUaSecurity::clientCertificate(), QString()));
}

/**
 * @brief Every server certificate the user has accepted.
 */
QVariantList IO::Drivers::OpcUaCertificateStore::trustedCertificates() const
{
  QVariantList out;
  const auto certificates = OpcUaSecurity::trustedCertificates();
  for (const auto& certificate : certificates)
    out.append(certificateMap(OpcUaSecurity::inspect(certificate, QString())));

  return out;
}

/**
 * @brief Every accepted server certificate, for the API.
 */
QJsonArray IO::Drivers::OpcUaCertificateStore::trustedJson() const
{
  QJsonArray out;
  const auto certificates = OpcUaSecurity::trustedCertificates();
  for (const auto& certificate : certificates)
    out.append(certificateObject(OpcUaSecurity::inspect(certificate, QString())));

  return out;
}

/**
 * @brief Turns whatever a caller hands us into a local path. QML file dialogs deliver a
 *        `file://` URL, the API delivers a plain path, and both reach the same setter.
 */
QString IO::Drivers::OpcUaCertificateStore::localPath(const QString& value)
{
  if (!value.startsWith(QLatin1String("file:")))
    return value;

  return QUrl(value).toLocalFile();
}

/**
 * @brief A certificate rendered for QML.
 */
QVariantMap IO::Drivers::OpcUaCertificateStore::certificateMap(const OpcUaTypes::CertInfo& info)
{
  return QVariantMap{
    {          QStringLiteral("valid"),           info.valid},
    {        QStringLiteral("subject"),         info.subject},
    {         QStringLiteral("issuer"),          info.issuer},
    {    QStringLiteral("fingerprint"),     info.fingerprint},
    { QStringLiteral("applicationUri"),  info.applicationUri},
    {      QStringLiteral("notBefore"),       info.notBefore},
    {       QStringLiteral("notAfter"),        info.notAfter},
    {        QStringLiteral("trusted"),         info.trusted},
    {        QStringLiteral("expired"),         info.expired},
    {    QStringLiteral("notYetValid"),     info.notYetValid},
    {QStringLiteral("hostnameMatches"), info.hostnameMatches},
  };
}

/**
 * @brief A certificate rendered for the API.
 */
QJsonObject IO::Drivers::OpcUaCertificateStore::certificateObject(const OpcUaTypes::CertInfo& info)
{
  return QJsonObject{
    {          QStringLiteral("valid"),                           info.valid},
    {        QStringLiteral("subject"),                         info.subject},
    {         QStringLiteral("issuer"),                          info.issuer},
    {    QStringLiteral("fingerprint"),                     info.fingerprint},
    { QStringLiteral("applicationUri"),                  info.applicationUri},
    {      QStringLiteral("notBefore"), info.notBefore.toString(Qt::ISODate)},
    {       QStringLiteral("notAfter"),  info.notAfter.toString(Qt::ISODate)},
    {        QStringLiteral("trusted"),                         info.trusted},
    {        QStringLiteral("expired"),                         info.expired},
    {    QStringLiteral("notYetValid"),                     info.notYetValid},
    {QStringLiteral("hostnameMatches"),                 info.hostnameMatches},
  };
}

/**
 * @brief Why a certificate was refused, in the user's words. Kept distinct on purpose: trust it,
 *        renew it, wait for it and dial the right name are four different fixes.
 */
QString IO::Drivers::OpcUaCertificateStore::describeTrustFailure(OpcUaTypes::TrustFailure failure)
{
  switch (failure) {
    case OpcUaTypes::TrustFailure::Untrusted:
      return tr("The server certificate is not trusted");
    case OpcUaTypes::TrustFailure::Expired:
      return tr("The server certificate has expired");
    case OpcUaTypes::TrustFailure::NotYetValid:
      return tr("The server certificate is not valid yet");
    case OpcUaTypes::TrustFailure::HostnameMismatch:
      return tr("The server certificate was not issued for this host");
    case OpcUaTypes::TrustFailure::Unreadable:
      return tr("The server certificate could not be parsed");
    case OpcUaTypes::TrustFailure::None:
      break;
  }

  return {};
}
