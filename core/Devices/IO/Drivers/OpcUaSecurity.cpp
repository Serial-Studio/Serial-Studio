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

#include "IO/Drivers/OpcUaSecurity.h"

#include <open62541.h>

#include <mbedtls/x509.h>
#include <mbedtls/x509_crt.h>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtEndian>
#include <QTimeZone>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr int kCertificateDays           = 3650;
static constexpr int kCertificateBits           = 2048;
static constexpr const char* kCertFile          = "client_cert.der";
static constexpr const char* kKeyFile           = "client_key.der";
static constexpr const char* kTrustDir          = "trusted";
static constexpr int kMaxSubjectAltNames        = 128;
static constexpr qsizetype kMaxCertificateBytes = 64 * 1024;

// Per-installation acknowledgement that a password may travel over an unencrypted channel
static constexpr const char* kPlaintextPasswordKey = "OpcUaDriver/allowPlaintextPassword";

//--------------------------------------------------------------------------------------------------
// Storage layout
//--------------------------------------------------------------------------------------------------

/**
 * @brief Where the installation's OPC UA identity and trust store live. Per-installation and
 *        outside any project: a private key that travelled with a project file would be shared
 *        the moment the project is, and a trust decision stored there would have to be re-made
 *        on every machine that opens it.
 */
QString IO::Drivers::OpcUaSecurity::storageDirectory()
{
  static const QString k_path =
    QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/OpcUa");
  return k_path;
}

/**
 * @brief Creates the identity and trust directories, false when they cannot exist. Split from
 *        storageDirectory() so that reading a path is not a filesystem write.
 */
static bool ensureStorage()
{
  const auto root = IO::Drivers::OpcUaSecurity::storageDirectory();
  SS_ASSERT(!root.isEmpty(), return false);

  return QDir().mkpath(root + QLatin1Char('/') + QString::fromLatin1(kTrustDir));
}

/**
 * @brief The application URI the client presents. The OPC UA specification requires it to match
 *        the URI inside the client certificate, and a server that finds them different refuses
 *        the channel, so both come from this one function.
 */
QString IO::Drivers::OpcUaSecurity::applicationUri()
{
  return QStringLiteral("urn:%1:SerialStudio:OpcUaClient").arg(QSysInfo::machineHostName());
}

/**
 * @brief Reads a whole file, or an empty array when it does not exist.
 */
static QByteArray readFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return {};

  return file.readAll();
}

/**
 * @brief Writes a file, narrowing it to owner-only BEFORE the payload is written: a key that is
 *        world-readable for the duration of the write is world-readable. A refused narrowing
 *        fails the write rather than leaving key bytes at the umask default.
 */
static bool writeFile(const QString& path, const QByteArray& data, bool ownerOnly)
{
  SS_ASSERT(!path.isEmpty(), return false);
  SS_ASSERT(!data.isEmpty(), return false);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    return false;

  if (ownerOnly && !file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
    file.close();
    (void)QFile::remove(path);
    return false;
  }

  const bool ok = file.write(data) == data.size();
  file.close();
  if (!ok)
    (void)QFile::remove(path);

  return ok;
}

//--------------------------------------------------------------------------------------------------
// Client identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates the self-signed client certificate and key, overwriting whatever is there.
 *        Both halves land through temporaries: a certificate written beside the PREVIOUS key
 *        would satisfy ensureClientIdentity() forever while failing every handshake.
 */
bool IO::Drivers::OpcUaSecurity::regenerateClientIdentity()
{
  const auto uri  = applicationUri();
  const auto host = QSysInfo::machineHostName();

  const auto subjectUtf8 = QStringLiteral("CN=Serial Studio@%1").arg(host).toUtf8();
  const auto orgUtf8     = QByteArrayLiteral("O=Serial Studio");
  const auto dnsUtf8     = QStringLiteral("DNS:%1").arg(host).toUtf8();
  const auto uriUtf8     = QStringLiteral("URI:%1").arg(uri).toUtf8();

  UA_String subject[2] = {UA_STRING(const_cast<char*>(subjectUtf8.constData())),
                          UA_STRING(const_cast<char*>(orgUtf8.constData()))};
  UA_String altName[2] = {UA_STRING(const_cast<char*>(dnsUtf8.constData())),
                          UA_STRING(const_cast<char*>(uriUtf8.constData()))};

  UA_KeyValueMap params       = UA_KEYVALUEMAP_NULL;
  UA_UInt16 expiresIn         = kCertificateDays;
  UA_UInt16 keyBits           = kCertificateBits;
  UA_QualifiedName expiresKey = UA_QUALIFIEDNAME(0, const_cast<char*>("expires-in-days"));
  UA_QualifiedName bitsKey    = UA_QUALIFIEDNAME(0, const_cast<char*>("key-size-bits"));

  UA_Variant expiresValue;
  UA_Variant bitsValue;
  UA_Variant_setScalar(&expiresValue, &expiresIn, &UA_TYPES[UA_TYPES_UINT16]);
  UA_Variant_setScalar(&bitsValue, &keyBits, &UA_TYPES[UA_TYPES_UINT16]);
  (void)UA_KeyValueMap_set(&params, expiresKey, &expiresValue);
  (void)UA_KeyValueMap_set(&params, bitsKey, &bitsValue);

  UA_ByteString certificate = UA_BYTESTRING_NULL;
  UA_ByteString privateKey  = UA_BYTESTRING_NULL;
  const auto status         = UA_CreateCertificate(UA_Log_Stdout,
                                           subject,
                                           2,
                                           altName,
                                           2,
                                           UA_CERTIFICATEFORMAT_DER,
                                           &params,
                                           &privateKey,
                                           &certificate);
  UA_KeyValueMap_clear(&params);
  if (status != UA_STATUSCODE_GOOD)
    return false;

  const QByteArray certDer(reinterpret_cast<const char*>(certificate.data),
                           static_cast<qsizetype>(certificate.length));
  const QByteArray keyDer(reinterpret_cast<const char*>(privateKey.data),
                          static_cast<qsizetype>(privateKey.length));
  UA_ByteString_clear(&certificate);
  UA_ByteString_clear(&privateKey);

  if (!ensureStorage())
    return false;

  const auto dir      = storageDirectory();
  const auto certPath = dir + QLatin1Char('/') + QString::fromLatin1(kCertFile);
  const auto keyPath  = dir + QLatin1Char('/') + QString::fromLatin1(kKeyFile);
  const auto certTmp  = certPath + QStringLiteral(".new");
  const auto keyTmp   = keyPath + QStringLiteral(".new");

  if (!writeFile(certTmp, certDer, false) || !writeFile(keyTmp, keyDer, true)) {
    (void)QFile::remove(certTmp);
    (void)QFile::remove(keyTmp);
    return false;
  }

  (void)QFile::remove(certPath);
  (void)QFile::remove(keyPath);
  return QFile::rename(certTmp, certPath) && QFile::rename(keyTmp, keyPath);
}

/**
 * @brief Generates the client certificate on first secure use and reuses it forever after, so a
 *        server operator trusts this installation once rather than on every launch. Hands back
 *        what it read, so the caller does not read both files a second time.
 */
bool IO::Drivers::OpcUaSecurity::ensureClientIdentity(QByteArray& certificate, QByteArray& key)
{
  certificate = clientCertificate();
  key         = clientPrivateKey();
  if (!certificate.isEmpty() && !key.isEmpty())
    return true;

  if (!regenerateClientIdentity())
    return false;

  certificate = clientCertificate();
  key         = clientPrivateKey();
  return !certificate.isEmpty() && !key.isEmpty();
}

/**
 * @brief The client certificate in DER form, or empty when none has been generated.
 */
QByteArray IO::Drivers::OpcUaSecurity::clientCertificate()
{
  return readFile(storageDirectory() + QLatin1Char('/') + QString::fromLatin1(kCertFile));
}

/**
 * @brief The client private key in DER form, or empty when none has been generated.
 */
QByteArray IO::Drivers::OpcUaSecurity::clientPrivateKey()
{
  return readFile(storageDirectory() + QLatin1Char('/') + QString::fromLatin1(kKeyFile));
}

/**
 * @brief Writes the client certificate where the user asked, so it can be handed to the server's
 *        own trust store. The PRIVATE key is never exported by this path.
 */
bool IO::Drivers::OpcUaSecurity::exportClientCertificate(const QString& path)
{
  const auto certificate = clientCertificate();
  if (certificate.isEmpty() || path.isEmpty())
    return false;

  return writeFile(path, certificate, false);
}

//--------------------------------------------------------------------------------------------------
// Trust store
//--------------------------------------------------------------------------------------------------

/**
 * @brief SHA-256 of the DER encoding, uppercase hex. This is the string the trust store is keyed
 *        on and the one the prompt shows, so the two can never disagree.
 */
static QString fingerprintOf(const QByteArray& certificate)
{
  const auto digest = QCryptographicHash::hash(certificate, QCryptographicHash::Sha256);
  return QString::fromLatin1(digest.toHex()).toUpper();
}

/**
 * @brief The trust-store path for a fingerprint, or empty when the fingerprint is not one this
 *        layer could have produced. A fingerprint arrives from the API and becomes a FILENAME, so
 *        it is matched against the exact shape of a SHA-256 hex digest first; "../../x" would
 *        otherwise resolve straight out of the trust directory and delete an unrelated file.
 */
static QString trustedPathFor(const QString& fingerprint)
{
  static const QRegularExpression digest(QStringLiteral("\\A[0-9A-F]{64}\\z"));
  const auto upper = fingerprint.toUpper();
  if (!digest.match(upper).hasMatch())
    return {};

  return IO::Drivers::OpcUaSecurity::storageDirectory() + QLatin1Char('/')
       + QString::fromLatin1(kTrustDir) + QLatin1Char('/') + upper + QStringLiteral(".der");
}

/**
 * @brief Every server certificate the user has accepted, in DER form.
 */
QList<QByteArray> IO::Drivers::OpcUaSecurity::trustedCertificates()
{
  QList<QByteArray> out;
  QDir dir(storageDirectory() + QLatin1Char('/') + QString::fromLatin1(kTrustDir));
  const auto entries = dir.entryList({QStringLiteral("*.der")}, QDir::Files, QDir::Name);
  for (const auto& entry : entries) {
    const auto data = readFile(dir.filePath(entry));
    if (!data.isEmpty())
      out.append(data);
  }

  return out;
}

/**
 * @brief Trust check for an already-computed fingerprint, so inspect() does not hash twice.
 */
static bool isTrustedFingerprint(const QString& fingerprint, const QByteArray& certificate)
{
  SS_ASSERT(!certificate.isEmpty(), return false);

  const auto path = trustedPathFor(fingerprint);
  if (path.isEmpty())
    return false;

  return readFile(path) == certificate;
}

/**
 * @brief True when this exact certificate has been accepted before. The stored bytes are read
 *        back and compared: treating the filename as proof would let anything able to create a
 *        file in the trust directory pre-authorize a server the user never saw.
 */
bool IO::Drivers::OpcUaSecurity::isTrusted(const QByteArray& certificate)
{
  if (certificate.isEmpty())
    return false;

  return isTrustedFingerprint(fingerprintOf(certificate), certificate);
}

/**
 * @brief Records the user's decision to accept a server certificate. Keyed on the fingerprint, so
 *        a server that re-keys is a NEW decision rather than a silently inherited one.
 */
bool IO::Drivers::OpcUaSecurity::trustCertificate(const QByteArray& certificate)
{
  if (certificate.isEmpty())
    return false;

  const auto path = trustedPathFor(fingerprintOf(certificate));
  if (path.isEmpty() || !ensureStorage())
    return false;

  return writeFile(path, certificate, false);
}

/**
 * @brief Withdraws a previously accepted certificate.
 */
bool IO::Drivers::OpcUaSecurity::revokeTrust(const QString& fingerprint)
{
  const auto path = trustedPathFor(fingerprint);
  if (path.isEmpty())
    return false;

  return QFile::remove(path);
}

/**
 * @brief Whether the user has accepted that a password may cross an unencrypted channel. Off until
 *        granted: open62541 refuses it by default, and overriding that for everyone shipped every
 *        None-policy login's password in the clear without ever asking. Per-INSTALLATION like the
 *        trust store, never a project key, so opening someone else's project cannot grant it.
 */
bool IO::Drivers::OpcUaSecurity::plaintextPasswordAllowed()
{
  QSettings settings;
  return settings.value(QString::fromLatin1(kPlaintextPasswordKey), false).toBool();
}

/**
 * @brief Records the user's answer to the plaintext-password prompt.
 */
void IO::Drivers::OpcUaSecurity::setPlaintextPasswordAllowed(const bool allowed)
{
  QSettings settings;
  settings.setValue(QString::fromLatin1(kPlaintextPasswordKey), allowed);
}

/**
 * @brief Reads a DER or PEM certificate from disk; PEM is converted so the rest of the layer
 *        only ever handles DER.
 */
QByteArray IO::Drivers::OpcUaSecurity::readCertificateFile(const QString& path)
{
  const auto data = readFile(path);
  if (data.isEmpty() || !data.startsWith("-----BEGIN"))
    return data;

  mbedtls_x509_crt chain;
  mbedtls_x509_crt_init(&chain);

  auto terminated = data;
  terminated.append('\0');

  QByteArray out;
  const auto* bytes = reinterpret_cast<const unsigned char*>(terminated.constData());
  if (mbedtls_x509_crt_parse(&chain, bytes, static_cast<size_t>(terminated.size())) == 0)
    out =
      QByteArray(reinterpret_cast<const char*>(chain.raw.p), static_cast<qsizetype>(chain.raw.len));

  mbedtls_x509_crt_free(&chain);
  return out;
}

//--------------------------------------------------------------------------------------------------
// Inspection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts an X.509 timestamp (always UTC) into a QDateTime.
 */
static QDateTime toDateTime(const mbedtls_x509_time& time)
{
  const QDate date(time.year, time.mon, time.day);
  const QTime clock(time.hour, time.min, time.sec);
  if (!date.isValid() || !clock.isValid())
    return {};

  return QDateTime(date, clock, QTimeZone::UTC);
}

/**
 * @brief Renders an iPAddress SAN, which is raw network-order bytes: 4 for IPv4, 16 for IPv6.
 */
static QString addressFromSan(const mbedtls_x509_buf& raw)
{
  if (raw.len == 4)
    return QHostAddress(qFromBigEndian<quint32>(raw.p)).toString();

  if (raw.len == 16) {
    Q_IPV6ADDR bytes;
    memcpy(bytes.c, raw.p, sizeof(bytes.c));
    return QHostAddress(bytes).toString();
  }

  return {};
}

/**
 * @brief Renders a distinguished name.
 */
static QString distinguishedName(const mbedtls_x509_name& name)
{
  char buffer[512] = {0};
  const int length = mbedtls_x509_dn_gets(buffer, sizeof(buffer), &name);
  if (length <= 0)
    return {};

  SS_ASSERT(static_cast<size_t>(length) <= sizeof(buffer), return {});
  return QString::fromUtf8(buffer, length);
}

/**
 * @brief Converts one SubjectAltName entry into a candidate name or the application URI.
 */
static void collectAltName(const mbedtls_x509_buf& buf, QStringList& names, QString& uri)
{
  mbedtls_x509_subject_alternative_name san;
  memset(&san, 0, sizeof(san));
  if (mbedtls_x509_parse_subject_alt_name(&buf, &san) != 0)
    return;

  const auto& raw = san.san.unstructured_name;
  const auto text =
    QString::fromUtf8(reinterpret_cast<const char*>(raw.p), static_cast<qsizetype>(raw.len));

  if (san.type == MBEDTLS_X509_SAN_DNS_NAME)
    names.append(text);
  else if (san.type == MBEDTLS_X509_SAN_UNIFORM_RESOURCE_IDENTIFIER)
    uri = text;
  else if (san.type == MBEDTLS_X509_SAN_IP_ADDRESS)
    names.append(addressFromSan(raw));

  mbedtls_x509_free_subject_alt_name(&san);
}

/**
 * @brief Collects the DNS names, IP addresses and URIs in the SubjectAltName extension. An
 *        iPAddress entry is how a certificate for an IP-addressed PLC names its host, and
 *        dropping it made every such certificate look like a hostname mismatch. The walk is
 *        capped: the list is built from bytes the server supplied.
 */
static QStringList subjectAltNames(const mbedtls_x509_crt& certificate, QString& uri)
{
  QStringList names;
  int visited = 0;
  for (const auto* entry = &certificate.subject_alt_names; entry && visited < kMaxSubjectAltNames;
       entry             = entry->next, ++visited) {
    if (entry->buf.p != nullptr)
      collectAltName(entry->buf, names, uri);
  }

  names.removeAll(QString());
  return names;
}

/**
 * @brief True when any candidate parses as an address equal to @p host.
 */
static bool matchesAddress(const QStringList& candidates, const QHostAddress& host)
{
  for (const auto& candidate : candidates) {
    QHostAddress other;
    if (other.setAddress(candidate) && other.isEqual(host))
      return true;
  }

  return false;
}

/**
 * @brief True when the certificate names @p host. A wildcard leftmost label matches one level,
 *        as in TLS, but NEVER against an IP literal: RFC 6125 forbids it, and "*.168.1.10" would
 *        otherwise match 192.168.1.10. An IP host is compared by address value only.
 */
static bool matchesHost(const QStringList& names, const QString& subject, const QString& host)
{
  if (host.isEmpty())
    return true;

  auto candidates       = names;
  const auto commonName = subject.section(QStringLiteral("CN="), 1).section(QLatin1Char(','), 0, 0);
  if (!commonName.isEmpty())
    candidates.append(commonName.trimmed());

  QHostAddress literal;
  if (literal.setAddress(host))
    return matchesAddress(candidates, literal);

  for (const auto& candidate : candidates) {
    if (candidate.compare(host, Qt::CaseInsensitive) == 0)
      return true;

    if (!candidate.startsWith(QStringLiteral("*.")) || candidate.size() <= 2)
      continue;

    if (host.section(QLatin1Char('.'), 1).compare(candidate.mid(2), Qt::CaseInsensitive) == 0)
      return true;
  }

  return false;
}

/**
 * @brief Everything a trust prompt needs about a server certificate, plus the four failure
 *        classifications kept apart: untrusted, expired, not yet valid and hostname mismatch are
 *        different problems with different fixes, and collapsing them into one message is what
 *        leaves an operator guessing.
 */
IO::Drivers::OpcUaTypes::CertInfo IO::Drivers::OpcUaSecurity::inspect(const QByteArray& certificate,
                                                                      const QString& host)
{
  OpcUaTypes::CertInfo info;
  info.certificate = certificate;
  if (certificate.isEmpty())
    return info;

  SS_ASSERT(certificate.size() <= kMaxCertificateBytes, return info);
  info.fingerprint = fingerprintOf(certificate);
  info.trusted     = isTrustedFingerprint(info.fingerprint, certificate);

  mbedtls_x509_crt parsed;
  mbedtls_x509_crt_init(&parsed);

  const auto* bytes = reinterpret_cast<const unsigned char*>(certificate.constData());
  SS_ASSERT(bytes != nullptr, return info);
  if (mbedtls_x509_crt_parse_der(&parsed, bytes, static_cast<size_t>(certificate.size())) != 0) {
    mbedtls_x509_crt_free(&parsed);
    return info;
  }

  info.valid     = true;
  info.subject   = distinguishedName(parsed.subject);
  info.issuer    = distinguishedName(parsed.issuer);
  info.notBefore = toDateTime(parsed.valid_from);
  info.notAfter  = toDateTime(parsed.valid_to);

  const auto names     = subjectAltNames(parsed, info.applicationUri);
  info.hostnameMatches = matchesHost(names, info.subject, host);
  mbedtls_x509_crt_free(&parsed);

  const auto now   = QDateTime::currentDateTimeUtc();
  info.expired     = info.notAfter.isValid() && info.notAfter < now;
  info.notYetValid = info.notBefore.isValid() && info.notBefore > now;
  return info;
}
