/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QByteArray>
#  include <QSslCertificate>
#  include <QSslConfiguration>
#  include <QSslKey>
#  include <QString>

namespace MQTT {

/**
 * @brief Parsed client identity (X.509 certificate + private key) for mutual TLS.
 */
struct TlsIdentity {
  QSslCertificate certificate;
  QSslKey privateKey;

  [[nodiscard]] bool isValid() const { return !certificate.isNull() && !privateKey.isNull(); }
};

/**
 * @brief Typed outcome of a TlsIdentity load attempt.
 */
enum class TlsIdentityError {
  NoError,
  MissingFile,
  Unreadable,
  NotPem,
  FileTooLarge,
  CertificateRequired,
  PassphraseRequired,
  PassphraseWrong,
};

/**
 * @brief Result of loadTlsIdentity(): the error, plus the path the error refers to.
 */
struct TlsIdentityResult {
  TlsIdentityError error = TlsIdentityError::NoError;
  QString path;

  [[nodiscard]] bool ok() const { return error == TlsIdentityError::NoError; }
};

[[nodiscard]] TlsIdentityResult loadTlsIdentity(const QString& certificatePath,
                                                const QString& privateKeyPath,
                                                const QString& passphrase,
                                                TlsIdentity& identity);
[[nodiscard]] QString tlsIdentityErrorString(const TlsIdentityResult& result);
void applyTlsIdentity(QSslConfiguration& configuration,
                      const TlsIdentity& identity,
                      const QByteArray& alpnProtocol);

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
