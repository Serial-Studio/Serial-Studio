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

#ifdef BUILD_COMMERCIAL

#  include "MQTT/TlsIdentity.h"

#  include <QCoreApplication>
#  include <QFile>
#  include <QFileInfo>

//--------------------------------------------------------------------------------------------------
// Constants: size cap for user-picked PEM files (real certificate/key PEMs are KB-scale)
//--------------------------------------------------------------------------------------------------

static constexpr qint64 kMaxPemFileBytes = 1024 * 1024;

/**
 * @brief Returns true when the PEM payload declares an encrypted private key.
 */
static bool isEncryptedPem(const QByteArray& pem)
{
  return pem.contains("ENCRYPTED PRIVATE KEY") || pem.contains("Proc-Type: 4,ENCRYPTED");
}

/**
 * @brief Reads a file fully; distinguishes a missing path from an unreadable file. The size
 *        cap keeps an accidental pick of a huge binary from being read whole on the GUI
 *        thread: real certificate/key PEMs are KB-scale.
 */
static MQTT::TlsIdentityError readPemFile(const QString& path, QByteArray& data)
{
  const QFileInfo info(path);
  if (!info.exists())
    return MQTT::TlsIdentityError::MissingFile;

  if (info.size() > kMaxPemFileBytes)
    return MQTT::TlsIdentityError::FileTooLarge;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return MQTT::TlsIdentityError::Unreadable;

  data = file.readAll();
  if (data.isEmpty())
    return MQTT::TlsIdentityError::NotPem;

  return MQTT::TlsIdentityError::NoError;
}

/**
 * @brief Parses a PEM private key, trying each supported public-key algorithm in turn.
 */
static QSslKey parsePrivateKey(const QByteArray& pem, const QByteArray& passphrase)
{
  constexpr QSsl::KeyAlgorithm kAlgorithms[] = {QSsl::Rsa, QSsl::Ec, QSsl::Dsa};
  for (const auto algorithm : kAlgorithms) {
    QSslKey key(pem, algorithm, QSsl::Pem, QSsl::PrivateKey, passphrase);
    if (!key.isNull())
      return key;
  }

  return QSslKey();
}

/**
 * @brief Loads a client certificate + private key pair from PEM files. An empty key path
 *        falls back to the certificate file (combined-PEM bundles). Empty cert path with an
 *        empty key path yields a valid no-identity result so callers can treat "unset" as off.
 */
MQTT::TlsIdentityResult MQTT::loadTlsIdentity(const QString& certificatePath,
                                              const QString& privateKeyPath,
                                              const QString& passphrase,
                                              TlsIdentity& identity)
{
  identity = TlsIdentity();

  if (certificatePath.isEmpty() && privateKeyPath.isEmpty())
    return TlsIdentityResult();

  if (certificatePath.isEmpty())
    return TlsIdentityResult{TlsIdentityError::CertificateRequired, privateKeyPath};

  QByteArray certPem;
  const auto certRead = readPemFile(certificatePath, certPem);
  if (certRead != TlsIdentityError::NoError)
    return TlsIdentityResult{certRead, certificatePath};

  const auto certs = QSslCertificate::fromData(certPem, QSsl::Pem);
  if (certs.isEmpty() || certs.first().isNull())
    return TlsIdentityResult{TlsIdentityError::NotPem, certificatePath};

  const QString keySource = privateKeyPath.isEmpty() ? certificatePath : privateKeyPath;
  QByteArray keyPem       = certPem;
  if (!privateKeyPath.isEmpty()) {
    const auto keyRead = readPemFile(privateKeyPath, keyPem);
    if (keyRead != TlsIdentityError::NoError)
      return TlsIdentityResult{keyRead, privateKeyPath};
  }

  const bool encrypted = isEncryptedPem(keyPem);
  if (encrypted && passphrase.isEmpty())
    return TlsIdentityResult{TlsIdentityError::PassphraseRequired, keySource};

  const auto key = parsePrivateKey(keyPem, passphrase.toUtf8());
  if (key.isNull()) {
    if (encrypted)
      return TlsIdentityResult{TlsIdentityError::PassphraseWrong, keySource};

    return TlsIdentityResult{TlsIdentityError::NotPem, keySource};
  }

  identity.certificate = certs.first();
  identity.privateKey  = key;
  return TlsIdentityResult();
}

/**
 * @brief Returns a localized, user-readable message for a load failure. Never echoes the
 *        passphrase; names the offending file instead.
 */
QString MQTT::tlsIdentityErrorString(const TlsIdentityResult& result)
{
  const auto file = QFileInfo(result.path).fileName();
  switch (result.error) {
    case TlsIdentityError::NoError:
      return QString();
    case TlsIdentityError::MissingFile:
      return QCoreApplication::translate("TlsIdentity", "The file \"%1\" does not exist.")
        .arg(result.path);
    case TlsIdentityError::Unreadable:
      return QCoreApplication::translate("TlsIdentity", "The file \"%1\" cannot be read.")
        .arg(result.path);
    case TlsIdentityError::NotPem:
      return QCoreApplication::translate("TlsIdentity",
                                         "\"%1\" is not a valid PEM certificate or key file.")
        .arg(file);
    case TlsIdentityError::FileTooLarge:
      return QCoreApplication::translate("TlsIdentity",
                                         "\"%1\" is too large to be a PEM certificate or key "
                                         "file.")
        .arg(file);
    case TlsIdentityError::CertificateRequired:
      return QCoreApplication::translate("TlsIdentity",
                                         "A private key is set, but no client certificate is "
                                         "selected. Select the certificate that matches it.");
    case TlsIdentityError::PassphraseRequired:
      return QCoreApplication::translate("TlsIdentity",
                                         "The private key \"%1\" is encrypted. Enter its "
                                         "passphrase and try again.")
        .arg(file);
    case TlsIdentityError::PassphraseWrong:
      return QCoreApplication::translate("TlsIdentity",
                                         "The passphrase does not unlock the private key \"%1\".")
        .arg(file);
  }

  return QString();
}

/**
 * @brief Writes the identity (and optional ALPN protocol) into a QSslConfiguration. Always-set
 *        semantics: an invalid identity writes null certificate/key and an empty ALPN clears the
 *        protocol list, so un-setting a previously configured identity really removes it.
 */
void MQTT::applyTlsIdentity(QSslConfiguration& configuration,
                            const TlsIdentity& identity,
                            const QByteArray& alpnProtocol)
{
  configuration.setLocalCertificate(identity.certificate);
  configuration.setPrivateKey(identity.privateKey);

  if (alpnProtocol.isEmpty())
    configuration.setAllowedNextProtocols({});
  else
    configuration.setAllowedNextProtocols({alpnProtocol});
}

#endif  // BUILD_COMMERCIAL
