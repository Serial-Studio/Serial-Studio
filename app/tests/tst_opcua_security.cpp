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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QTest>

#include "IO/Drivers/OpcUaSecurity.h"

using IO::Drivers::OpcUaSecurity::inspect;

namespace Security = IO::Drivers::OpcUaSecurity;

/**
 * @brief Wraps DER certificate bytes in the PEM armor a server operator hands out, so the
 *        PEM-to-DER path is driven with a real file rather than a fixture blob.
 */
[[nodiscard]] static QByteArray toPem(const QByteArray& der)
{
  QByteArray out("-----BEGIN CERTIFICATE-----\n");
  const auto base64 = der.toBase64();
  for (qsizetype i = 0; i < base64.size(); i += 64) {
    out.append(base64.mid(i, 64));
    out.append('\n');
  }

  out.append("-----END CERTIFICATE-----\n");
  return out;
}

/**
 * @brief The installation's own certificate and trust store: what a certificate names, whether the
 *        host it was dialed by is one of those names, and whether the user has accepted it. The
 *        trust answer is INDEPENDENT of the name and the clock, because trust pins the exact bytes
 *        by SHA-256, and the session reads it before either (spec 0075 E11).
 */
class TstOpcUaSecurity : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void identityIsGeneratedOnceAndReused();
  void hostnameMatchesTheSubjectAltNames();
  void anIpHostNeverMatchesAWildcard();
  void trustIsIndependentOfTheHostname();
  void trustSurvivesRevokeAndReturns();
  void pemCertificatesAreReadAsDer();
  void plaintextPasswordIsOffUntilGranted();

private:
  QByteArray m_certificate;
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects the config location into the test sandbox and generates one client certificate
 *        the whole suite inspects. The certificate is a real self-signed DER carrying this host's
 *        name in its SubjectAltName, which is exactly the shape a small PLC presents.
 */
void TstOpcUaSecurity::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_opcua_security"));
  QVERIFY(Security::regenerateClientIdentity());

  m_certificate = Security::clientCertificate();
  QVERIFY(!m_certificate.isEmpty());
}

/**
 * @brief Drops the generated identity and every trust decision the suite recorded.
 */
void TstOpcUaSecurity::cleanupTestCase()
{
  const auto root = Security::storageDirectory();
  if (!root.isEmpty())
    QDir(root).removeRecursively();
}

//--------------------------------------------------------------------------------------------------
// Client identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief The identity is generated once and REUSED: a server operator trusts this installation
 *        once, so a certificate regenerated on every launch would have to be re-accepted forever.
 */
void TstOpcUaSecurity::identityIsGeneratedOnceAndReused()
{
  QByteArray certificate;
  QByteArray key;
  QVERIFY(Security::ensureClientIdentity(certificate, key));
  QCOMPARE(certificate, m_certificate);
  QVERIFY(!key.isEmpty());

  QByteArray again;
  QByteArray againKey;
  QVERIFY(Security::ensureClientIdentity(again, againKey));
  QCOMPARE(again, m_certificate);

  const auto info = inspect(m_certificate, QString());
  QVERIFY(info.valid);
  QVERIFY(!info.fingerprint.isEmpty());
  QVERIFY(!info.expired);
  QVERIFY(!info.notYetValid);
  QVERIFY(!info.applicationUri.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Hostname matching
//--------------------------------------------------------------------------------------------------

/**
 * @brief A certificate names its hosts in the SubjectAltName extension; the host the link was
 *        dialed by has to be one of them, and an unrelated host is a mismatch rather than a
 *        generic failure, because the two have different fixes.
 */
void TstOpcUaSecurity::hostnameMatchesTheSubjectAltNames()
{
  const auto host = QSysInfo::machineHostName();
  QVERIFY(!host.isEmpty());

  QVERIFY(inspect(m_certificate, host).hostnameMatches);
  QVERIFY(!inspect(m_certificate, QStringLiteral("some-other-plc.local")).hostnameMatches);
}

/**
 * @brief A wildcard leftmost label matches one DNS level but NEVER an IP literal: RFC 6125 forbids
 *        it, and "*.168.1.10" would otherwise match 192.168.1.10.
 */
void TstOpcUaSecurity::anIpHostNeverMatchesAWildcard()
{
  QVERIFY(!inspect(m_certificate, QStringLiteral("192.168.1.10")).hostnameMatches);
  QVERIFY(!inspect(m_certificate, QStringLiteral("::1")).hostnameMatches);
  QVERIFY(!inspect(QByteArray(), QStringLiteral("192.168.1.10")).valid);
}

//--------------------------------------------------------------------------------------------------
// Trust store
//--------------------------------------------------------------------------------------------------

/**
 * @brief The regression E11 pins: trust is recorded against the certificate's SHA-256 fingerprint,
 *        so it is true for a certificate whose name does not match the host it was dialed by. That
 *        is the whole point of the Trust action on a self-signed controller reached by IP.
 */
void TstOpcUaSecurity::trustIsIndependentOfTheHostname()
{
  const QString wrongHost = QStringLiteral("192.168.1.10");
  (void)Security::revokeTrust(inspect(m_certificate, QString()).fingerprint);

  auto before = inspect(m_certificate, wrongHost);
  QVERIFY(before.valid);
  QVERIFY(!before.hostnameMatches);
  QVERIFY(!before.trusted);

  QVERIFY(Security::trustCertificate(m_certificate));

  auto after = inspect(m_certificate, wrongHost);
  QVERIFY(after.trusted);
  QVERIFY(!after.hostnameMatches);
  QCOMPARE(after.fingerprint, before.fingerprint);
  QVERIFY(Security::isTrusted(m_certificate));
}

/**
 * @brief Revoking removes exactly the fingerprint it names and nothing else, and a re-accepted
 *        certificate is trusted again: the store is keyed by bytes, so a server that re-keys is a
 *        NEW decision rather than an inherited one.
 */
void TstOpcUaSecurity::trustSurvivesRevokeAndReturns()
{
  QVERIFY(Security::trustCertificate(m_certificate));

  const auto fingerprint = inspect(m_certificate, QString()).fingerprint;
  QVERIFY(!fingerprint.isEmpty());
  QVERIFY(Security::trustedCertificates().contains(m_certificate));

  QVERIFY(Security::revokeTrust(fingerprint));
  QVERIFY(!Security::isTrusted(m_certificate));
  QVERIFY(!Security::trustedCertificates().contains(m_certificate));

  QVERIFY(Security::trustCertificate(m_certificate));
  QVERIFY(Security::isTrusted(m_certificate));
}

//--------------------------------------------------------------------------------------------------
// File formats
//--------------------------------------------------------------------------------------------------

/**
 * @brief A user hands over whichever of the two encodings their tooling produced, so the reader
 *        converts PEM and leaves DER alone; the rest of the layer only ever handles DER.
 */
void TstOpcUaSecurity::pemCertificatesAreReadAsDer()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());

  const QByteArray armored = toPem(m_certificate);
  const QString pemPath    = dir.filePath(QStringLiteral("server.pem"));
  QFile pem(pemPath);
  QVERIFY(pem.open(QIODevice::WriteOnly));
  QCOMPARE(pem.write(armored), qint64(armored.size()));
  pem.close();

  QCOMPARE(Security::readCertificateFile(pemPath), m_certificate);

  const QString derPath = dir.filePath(QStringLiteral("server.der"));
  QFile der(derPath);
  QVERIFY(der.open(QIODevice::WriteOnly));
  QCOMPARE(der.write(m_certificate), qint64(m_certificate.size()));
  der.close();

  QCOMPARE(Security::readCertificateFile(derPath), m_certificate);
  QVERIFY(Security::readCertificateFile(dir.filePath(QStringLiteral("absent"))).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Plaintext-password acknowledgement
//--------------------------------------------------------------------------------------------------

/**
 * @brief open62541 refuses a password on an unencrypted channel by default; the override is the
 *        user's per-installation answer and starts OFF, so a None-policy login sends nothing in
 *        the clear until someone accepts that it will (spec 0075 E11).
 */
void TstOpcUaSecurity::plaintextPasswordIsOffUntilGranted()
{
  Security::setPlaintextPasswordAllowed(false);
  QVERIFY(!Security::plaintextPasswordAllowed());

  Security::setPlaintextPasswordAllowed(true);
  QVERIFY(Security::plaintextPasswordAllowed());

  Security::setPlaintextPasswordAllowed(false);
  QVERIFY(!Security::plaintextPasswordAllowed());
}

QTEST_GUILESS_MAIN(TstOpcUaSecurity)

#include "tst_opcua_security.moc"
