/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QSslConfiguration>
#include <QTemporaryDir>
#include <QTest>

#include "MQTT/TlsIdentity.h"

// Throwaway self-signed RSA-2048 identity generated for this suite only (CN=serial-studio-test,
// valid to 2036). The encrypted variant is the same key under AES-128 with passphrase "test1234".
// None of this material protects anything.

static const char kTestKeyPem[] =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCvxrTpP7a2GlkX\n"
  "usIUe2dkoNkWoSOlBAWeAPgODAsU7mZYU3Y79nzBoFRqN0axO5f8N7Jkq/EB2ddI\n"
  "VlHV9YyINdLXTjd9fd9tU9ibC9RkQ5qcpxi4BT3mevRAgdqF6jXcxHnrca9kXcOP\n"
  "EsrEtk7WCLgXUcSCwv2vhR+8U8Zlp3j+eRzPQIOaUYMf350tRqw5myoUB74uYeYQ\n"
  "IHRBx/GPRxTRlZuS7H8JIzSxRvNU+GIwp5vaqnsXdKYl6MBvNrkFjZnrUNh8ZlaT\n"
  "fYGwZ4Hky3DlSgFUN1jDkiXKn3sGMpeZ+fEJtkm2ayi83Gshd3AJyMU0eQkJLQWQ\n"
  "qHGrOAP9AgMBAAECggEADFs8lnT7JBwADVH8mLIAF6vS9utG9S+uMWhCB8LC+Kq1\n"
  "mRns4tQ1+uqJZJoUJxtYWMuVh8wqegXVQ2mGATNsuJqIAsilz/WMQkQ1otiXCqFR\n"
  "+hco/q6npk8YYnx+XxGSzb0mEO4B9V0+BQ2+IUvriNI+Fq2R0dthN/h0valp9fVN\n"
  "BIZK5fWB2jrrQolc51sT8RyYKmB4N6NyuID+K7L8/bphpqjlsf9I/oJuo5iIyy83\n"
  "enPloeXFiigCzxDMPbX1bsrQFxnh/5/REl7NVhSSZWg4nWevlwAAxA8WaW793tcE\n"
  "IMNnm6o8xrJjvnIX7V6l0peaTox62eua6X+MBnVsxQKBgQDhLY7PN2axf216p7Ao\n"
  "s8ETJhv9RDBos5XrkRPDKeWNzMKzEoXlAUxTfSvK8RB0ZuRGSqTMqB46R24xhrBX\n"
  "NfE1KvlCVLXM0Ih4nO+L2YauxdCFI5wKwovXXG0QViBbdsIilVF+R/EWon55V2Jm\n"
  "yoOdGi1TLNK3Zc5NQDtrOlOWbwKBgQDH1hDX8UrM1zjbi6etx9FR8Gdgb5AQLkx8\n"
  "hvTCnTze+BBHrs4fpy/eUbU5S1g0JtAAd2kax+QjCEjcXMiJUEPzgMS/gSh1qosW\n"
  "k3wHPRJ5xPtdPxQ1Ng/DMQe3y2/984XaOBWBwZy4xvRF/mhhnKhodIBVg+kk/sDc\n"
  "mBUkx9SiUwKBgCXEbl9y+1Zp+QLJfVJWU9vSWH9yIGxvMoYAH0BsloPiBAXk5XAz\n"
  "d4nJRL8VD512ZHMlsd5hlDBSKrlDze/SrFIukv/9FUD0+O7B46uhIGXP59lsz48+\n"
  "chX8+o3y5BpzILyMLs7vYhBZ0LypW2fCL7D54wBAVFdOS/vI+i5R6hMhAoGAVvRR\n"
  "zfUDrFB2QCnro5WQAoyOHwtMb44F4CXFOS28P/VG+DobHPDdxmpdZfnf/olo9tr+\n"
  "1BVossm3EpQY+o7/kmRUuFPWLCUycndof6nWcUACdJevDlxgzXl3N0WBjMo7UzIW\n"
  "QajirhB8WDyUZO0K5rDS6uMEPwHPH+3ZPXDqmFsCgYBySreA+XRSD/EKg9dP6Vpz\n"
  "5l2+nT0C9srOQWtlIcGugDXgHL8joSv+GRlsUImzlEkbN2eQshKad5Aq7oIydTmP\n"
  "jGIOhXsVKADZ/LcXPeQS/8HI65s8zZdEpSmtChORDuC3ukrbA1otcgApqSJ1EBA2\n"
  "1rntR3zqrqipFZgFE3zsSA==\n"
  "-----END PRIVATE KEY-----\n";

static const char kTestEncKeyPem[] =
  "-----BEGIN RSA PRIVATE KEY-----\n"
  "Proc-Type: 4,ENCRYPTED\n"
  "DEK-Info: AES-128-CBC,D50CCF9A0D40D37610B586097E4CCC2B\n"
  "\n"
  "NKxOCjXJQBok8kwg7Zubjk4H+FpyDtdMHz5YhklNccwZlayNTE4ohtl7nBBe0xdz\n"
  "mR8iQO9icAON3p3cjDhyU9ft8pCVjSagKe+eVXlQS2LGJhU/oG2YDVj475krd3hT\n"
  "xLFtpL+42lqT6wLItHrCLybcJrFLRBHAXDmi/txtFqWDP2NKv+8Xd2bvN2TEWl1c\n"
  "OZgGdv5rOppFNF1q565lesU/iCRR1GdaCEtJwnZsRcduK165bgvrq3W/yX1R9S00\n"
  "JmOHrSedC1sgvOUFX86aelJRrL9g9VNfmwg/ED4PbMUHFbuw9nHy1M+GzBm/ZfUu\n"
  "w0dhHr944klr5tCG+pic0Jj8F5jYMU2mHh0EJ9IlkojIrgfIqoy7Kh3Jl/bxYEv3\n"
  "Nd6r6PyrbVRS/79LxRDd2tzLbnmcSMJGxMjvmUm8/fK35etmZDLNQXGIPSRpqiY3\n"
  "4qvRGv6+0t4OI/qE0xvm7/zpFXoxJmqM8t1/wRmIUL50M31+sM8IfjWw9UIcCZf7\n"
  "ipCOJ3ggIj9kPWaKd39HBJ3dl/Fdb/UQwWIfnoD0ntWtBQ0iLhQ5PfnYfc8jNEvz\n"
  "Ht2oWj4sDqgFJiI7BqejN/KZlmIx9DdTD42hBEcSGrHGmhWewG8BMgn3/ZrDeXF2\n"
  "r6kixuuaP4dVmHIvdPCtkRl9rRocqVnh/BnsnIO2OYMtgYJjLxl+3mDtu3/pMbJY\n"
  "hP49uVLOmZwKFiJ9kSHPnwIwsf4Sryftipq4jz2ZfU2SeXZXFbfMN9tgTfP+MGVR\n"
  "G0CYovwYTBeKucUQms8t1+ob3MsWHg4hipqY+NjphhZ2Fe7aeSEdeu8LshNYyzor\n"
  "jTCOlNNf0SKRa+Jeu2IGrJ03qdSumgJFL6X1K+8XPBQe3Qyfst/hENv3j9s802ZX\n"
  "RjihCCTRoiq3UPksbOpdzEcrS2leL7lDk9b2ocBhJhGVGMBJn6+8XrFMojmB31Fi\n"
  "/lru7EwLsXHl3U+E5R7USqwM9vC14fEkrQLDCoOQNBpvGZXgshbnhA96nEhSIN0z\n"
  "ZXpif1H5+Ntv2zddD9aZor59H8AGhmRyXKOZsk6ex8amfJgnNlNk7jhsnjsa/aJe\n"
  "BKw8s79y/Y2RurnIZ5FXrBoAbsAFvkJT8fpHwukCWRes6gVRZqtGVKRVDKUgiLLi\n"
  "fjPWrXKSDdrg5vFeCzzwTYOfx/HNELGT0ohpRD81xdMSG58ia/fuyD2K046RD4qV\n"
  "CRtCSrYb/HxGIDBCQfBJ5GF7FlnjXCfwlcWepYcJpQyzevnRjw/KHCPP7O62UaPv\n"
  "WFTWJQTyVUcsOgjEdrSXBgZQkjcKZIzqzjWprtdeaV446iieuG60hoXbC8B8kca2\n"
  "0PfRGE6wjvJf3pJgGbVLJNLAvkn3r9oTCy9+roHTvOKomjFoanHOOwA2alV7cC8k\n"
  "aJrTKe8YJh3Dp3Vv1gTkeTqxiT7kFebt1jXdzE1lh0HzQIhmavmB9C3CgCbT0vsV\n"
  "ECcN2YPjy/jekTPnlQnUCC2+EqFFq/ycrcAkzVURBHMp6oKCjhIIrigKABEJ4r2/\n"
  "D3IZf3izYBMbZruIgmeD3AF2PZrJTcuRdqib9qkx5BTYXgSjbxxfoSWY2kgaNBmW\n"
  "-----END RSA PRIVATE KEY-----\n";

static const char kTestCertPem[] =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDGzCCAgOgAwIBAgIUMpgxQTYpqW1//6r8hZF8U8oO/rMwDQYJKoZIhvcNAQEL\n"
  "BQAwHTEbMBkGA1UEAwwSc2VyaWFsLXN0dWRpby10ZXN0MB4XDTI2MDcyODAyNTg1\n"
  "NFoXDTM2MDcyNTAyNTg1NFowHTEbMBkGA1UEAwwSc2VyaWFsLXN0dWRpby10ZXN0\n"
  "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAr8a06T+2thpZF7rCFHtn\n"
  "ZKDZFqEjpQQFngD4DgwLFO5mWFN2O/Z8waBUajdGsTuX/DeyZKvxAdnXSFZR1fWM\n"
  "iDXS1043fX3fbVPYmwvUZEOanKcYuAU95nr0QIHaheo13MR563GvZF3DjxLKxLZO\n"
  "1gi4F1HEgsL9r4UfvFPGZad4/nkcz0CDmlGDH9+dLUasOZsqFAe+LmHmECB0Qcfx\n"
  "j0cU0ZWbkux/CSM0sUbzVPhiMKeb2qp7F3SmJejAbza5BY2Z61DYfGZWk32BsGeB\n"
  "5Mtw5UoBVDdYw5Ilyp97BjKXmfnxCbZJtmsovNxrIXdwCcjFNHkJCS0FkKhxqzgD\n"
  "/QIDAQABo1MwUTAdBgNVHQ4EFgQUhjLF6E8JUIl564pvDZ5TQlpwKF0wHwYDVR0j\n"
  "BBgwFoAUhjLF6E8JUIl564pvDZ5TQlpwKF0wDwYDVR0TAQH/BAUwAwEB/zANBgkq\n"
  "hkiG9w0BAQsFAAOCAQEArjldPwgG0T7dBftwNe3x6IsWc0utRHSe0ihXQPMDoGUe\n"
  "M7rXCu3vNXqmS3K/D7IEVfO+9eveY7xL2g282I4hMi9Kw4ndHOAN4JWnm5JJrqiP\n"
  "qncsoLRHTGOSH5Bw02JvtLUwvTYkhU6M8oL41+c9Zcm8Jdwv4vnWb8uVMR6iBf7P\n"
  "EZKfuRHgARDYBAVsEblD0HwkcUf/Imt9bm5dvUtWsleWg6geTYYCQycFo0Ktc/h2\n"
  "fm73rEDfadeROgedvHSGEmQTU9631xw3Pjfg3b2mUcV9GremAasHW0YA0Rs07xhr\n"
  "1U1EyzPjoxeOU8EzSZ6hIIVjBVOlNyeu13mgLe7e8g==\n"
  "-----END CERTIFICATE-----\n";

/**
 * @brief Load/validate/apply contract of MQTT::TlsIdentity (spec 0041).
 */
class TstTlsIdentity : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void emptyPathsYieldNoIdentity();
  void validPairLoads();
  void combinedPemLoads();
  void missingCertFile();
  void missingKeyFile();
  void keyWithoutCertificate();
  void nonPemCert();
  void nonPemKey();
  void encryptedKeyWithoutPassphrase();
  void encryptedKeyWrongPassphrase();
  void encryptedKeyRightPassphrase();
  void applyWritesConfiguration();
  void applyClearsWithInvalidIdentity();
  void errorStringsNeverEchoPassphrase();

private:
  [[nodiscard]] QString writeFile(const QString& name, const QByteArray& contents);

private:
  QTemporaryDir m_dir;
  QString m_certPath;
  QString m_keyPath;
  QString m_encKeyPath;
  QString m_combinedPath;
  QString m_garbagePath;
};

/**
 * @brief Materializes the PEM fixtures into a temporary directory.
 */
void TstTlsIdentity::initTestCase()
{
  QVERIFY(m_dir.isValid());

  m_certPath   = writeFile(QStringLiteral("cert.pem"), kTestCertPem);
  m_keyPath    = writeFile(QStringLiteral("key.pem"), kTestKeyPem);
  m_encKeyPath = writeFile(QStringLiteral("key_enc.pem"), kTestEncKeyPem);
  m_combinedPath =
    writeFile(QStringLiteral("combined.pem"), QByteArray(kTestCertPem) + QByteArray(kTestKeyPem));
  m_garbagePath = writeFile(QStringLiteral("garbage.pem"), "this is not pem data\n");
}

/**
 * @brief Writes a fixture file and returns its absolute path.
 */
QString TstTlsIdentity::writeFile(const QString& name, const QByteArray& contents)
{
  const auto path = m_dir.filePath(name);
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    return QString();

  file.write(contents);
  return path;
}

/**
 * @brief Unset paths are "feature off": success with an invalid (empty) identity.
 */
void TstTlsIdentity::emptyPathsYieldNoIdentity()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(QString(), QString(), QString(), identity);
  QVERIFY(result.ok());
  QVERIFY(!identity.isValid());
}

/**
 * @brief A valid unencrypted pair loads into a valid identity.
 */
void TstTlsIdentity::validPairLoads()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(m_certPath, m_keyPath, QString(), identity);
  QVERIFY(result.ok());
  QVERIFY(identity.isValid());
  QCOMPARE(identity.privateKey.algorithm(), QSsl::Rsa);
}

/**
 * @brief An empty key path falls back to the certificate file (combined PEM bundle).
 */
void TstTlsIdentity::combinedPemLoads()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(m_combinedPath, QString(), QString(), identity);
  QVERIFY(result.ok());
  QVERIFY(identity.isValid());
}

/**
 * @brief A nonexistent certificate path reports MissingFile against that path.
 */
void TstTlsIdentity::missingCertFile()
{
  MQTT::TlsIdentity identity;
  const auto missing = m_dir.filePath(QStringLiteral("nope.pem"));
  const auto result  = MQTT::loadTlsIdentity(missing, m_keyPath, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::MissingFile);
  QCOMPARE(result.path, missing);
  QVERIFY(!identity.isValid());
}

/**
 * @brief A nonexistent key path reports MissingFile against the key path.
 */
void TstTlsIdentity::missingKeyFile()
{
  MQTT::TlsIdentity identity;
  const auto missing = m_dir.filePath(QStringLiteral("nokey.pem"));
  const auto result  = MQTT::loadTlsIdentity(m_certPath, missing, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::MissingFile);
  QCOMPARE(result.path, missing);
}

/**
 * @brief A key path with no certificate path reports CertificateRequired, and the message
 *        names the certificate instead of formatting an empty file path.
 */
void TstTlsIdentity::keyWithoutCertificate()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(QString(), m_keyPath, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::CertificateRequired);
  QVERIFY(!identity.isValid());
  QVERIFY(!MQTT::tlsIdentityErrorString(result).contains(QStringLiteral("\"\"")));
}

/**
 * @brief Garbage bytes in the certificate slot report NotPem.
 */
void TstTlsIdentity::nonPemCert()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(m_garbagePath, m_keyPath, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::NotPem);
  QCOMPARE(result.path, m_garbagePath);
}

/**
 * @brief Garbage bytes in the key slot report NotPem against the key path.
 */
void TstTlsIdentity::nonPemKey()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(m_certPath, m_garbagePath, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::NotPem);
  QCOMPARE(result.path, m_garbagePath);
}

/**
 * @brief An encrypted key with no passphrase reports PassphraseRequired, not a parse error.
 */
void TstTlsIdentity::encryptedKeyWithoutPassphrase()
{
  MQTT::TlsIdentity identity;
  const auto result = MQTT::loadTlsIdentity(m_certPath, m_encKeyPath, QString(), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::PassphraseRequired);
  QCOMPARE(result.path, m_encKeyPath);
}

/**
 * @brief An encrypted key with the wrong passphrase reports PassphraseWrong.
 */
void TstTlsIdentity::encryptedKeyWrongPassphrase()
{
  MQTT::TlsIdentity identity;
  const auto result =
    MQTT::loadTlsIdentity(m_certPath, m_encKeyPath, QStringLiteral("wrong"), identity);
  QCOMPARE(result.error, MQTT::TlsIdentityError::PassphraseWrong);
}

/**
 * @brief The right passphrase unlocks the encrypted key.
 */
void TstTlsIdentity::encryptedKeyRightPassphrase()
{
  MQTT::TlsIdentity identity;
  const auto result =
    MQTT::loadTlsIdentity(m_certPath, m_encKeyPath, QStringLiteral("test1234"), identity);
  QVERIFY(result.ok());
  QVERIFY(identity.isValid());
}

/**
 * @brief applyTlsIdentity writes the certificate, key and ALPN protocol into the configuration.
 */
void TstTlsIdentity::applyWritesConfiguration()
{
  MQTT::TlsIdentity identity;
  QVERIFY(MQTT::loadTlsIdentity(m_certPath, m_keyPath, QString(), identity).ok());

  QSslConfiguration config;
  MQTT::applyTlsIdentity(config, identity, QByteArrayLiteral("x-amzn-mqtt-ca"));
  QCOMPARE(config.localCertificate(), identity.certificate);
  QCOMPARE(config.privateKey(), identity.privateKey);
  QCOMPARE(config.allowedNextProtocols(), QList<QByteArray>{QByteArrayLiteral("x-amzn-mqtt-ca")});
}

/**
 * @brief An invalid identity clears any previously configured certificate, key and ALPN.
 */
void TstTlsIdentity::applyClearsWithInvalidIdentity()
{
  MQTT::TlsIdentity identity;
  QVERIFY(MQTT::loadTlsIdentity(m_certPath, m_keyPath, QString(), identity).ok());

  QSslConfiguration config;
  MQTT::applyTlsIdentity(config, identity, QByteArrayLiteral("x-amzn-mqtt-ca"));
  MQTT::applyTlsIdentity(config, MQTT::TlsIdentity(), QByteArray());
  QVERIFY(config.localCertificate().isNull());
  QVERIFY(config.privateKey().isNull());
  QVERIFY(config.allowedNextProtocols().isEmpty());
}

/**
 * @brief No error message contains the passphrase the caller supplied.
 */
void TstTlsIdentity::errorStringsNeverEchoPassphrase()
{
  const auto secret = QStringLiteral("s3cr3t-passphrase");

  MQTT::TlsIdentity identity;
  const auto wrong = MQTT::loadTlsIdentity(m_certPath, m_encKeyPath, secret, identity);
  QCOMPARE(wrong.error, MQTT::TlsIdentityError::PassphraseWrong);
  QVERIFY(!MQTT::tlsIdentityErrorString(wrong).contains(secret));

  const auto required = MQTT::loadTlsIdentity(m_certPath, m_encKeyPath, QString(), identity);
  QVERIFY(!MQTT::tlsIdentityErrorString(required).contains(secret));
}

QTEST_GUILESS_MAIN(TstTlsIdentity)
#include "tst_tls_identity.moc"
