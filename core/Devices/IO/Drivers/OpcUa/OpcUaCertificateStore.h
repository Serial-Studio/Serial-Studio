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

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

class OpcUaSession;

/**
 * @brief The certificate face of the OPC UA driver (spec 0067): the installation's own client
 *        identity, the accepted-server trust store, and the certificate a refused dial was refused
 *        over. The mbedTLS-backed storage itself lives in the OpcUaSecurity namespace, so the
 *        driver keeps only the pending-trust decision.
 */
class OpcUaCertificateStore : public QObject {
  Q_OBJECT

signals:
  void certificateChanged();
  void serverCertificateUntrusted(const QVariantMap& certificate, const QString& reason);

public:
  explicit OpcUaCertificateStore(QObject* parent = nullptr);

  OpcUaCertificateStore(OpcUaCertificateStore&&)                 = delete;
  OpcUaCertificateStore(const OpcUaCertificateStore&)            = delete;
  OpcUaCertificateStore& operator=(OpcUaCertificateStore&&)      = delete;
  OpcUaCertificateStore& operator=(const OpcUaCertificateStore&) = delete;

  [[nodiscard]] bool ensureClientIdentity();
  [[nodiscard]] bool regenerateCertificate();
  [[nodiscard]] bool exportCertificate(const QString& path) const;
  [[nodiscard]] bool revokeServerCertificate(const QString& fingerprint);
  [[nodiscard]] bool trustCertificate(const OpcUaTypes::CertInfo& pending,
                                      const QString& fingerprint);
  [[nodiscard]] QString captureTrustFailure(const OpcUaSession* session);

  [[nodiscard]] const OpcUaTypes::CertInfo& pendingTrust() const noexcept;
  [[nodiscard]] QVariantMap clientCertificate() const;
  [[nodiscard]] QJsonObject certificateJson() const;
  [[nodiscard]] QVariantList trustedCertificates() const;
  [[nodiscard]] QJsonArray trustedJson() const;

  [[nodiscard]] static QString localPath(const QString& value);
  [[nodiscard]] static QVariantMap certificateMap(const OpcUaTypes::CertInfo& info);
  [[nodiscard]] static QJsonObject certificateObject(const OpcUaTypes::CertInfo& info);
  [[nodiscard]] static QString describeTrustFailure(OpcUaTypes::TrustFailure failure);

private:
  OpcUaTypes::CertInfo m_pendingTrust;
};

}  // namespace Drivers
}  // namespace IO
