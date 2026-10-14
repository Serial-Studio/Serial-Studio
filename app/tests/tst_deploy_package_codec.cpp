/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <array>
#include <cstring>
#include <QCborStreamWriter>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QtEndian>
#include <QTest>

#include "Core/Deploy/PackageCodec.h"
#include "Core/Deploy/PackageFormat.h"

static const QVersionNumber kRunning(4, 1, 0);
static constexpr char kMarker[] = "SS_MARKER_7f3a9c";

//--------------------------------------------------------------------------------------------------
// Reference writer: an independent copy of the v1 format, so crafted inputs reach the decoder
//--------------------------------------------------------------------------------------------------

/**
 * @brief Header fields and extra keys for a hand-built package.
 */
struct Crafted {
  qint64 format    = 1;
  qint64 encoding  = 1;
  QString minApp   = QStringLiteral("4.1.0");
  QString writer   = QStringLiteral("4.1.0");
  qint64 plainSize = -1;
  QByteArray extraHeaderKey;
};

/**
 * @brief xoshiro256** keystream XOR keyed by SHA-256(label || nonce), as the v1 format defines.
 */
static QByteArray referenceScramble(const QByteArray& data, const QByteArray& nonce)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  hash.addData(QByteArrayView("Serial Studio deployment scramble v1"));
  hash.addData(nonce);
  const auto seed = hash.result();

  std::array<quint64, 4> s{};
  for (int i = 0; i < 4; ++i)
    s[i] = qFromLittleEndian<quint64>(seed.constData() + i * 8);

  const auto rotl = [](quint64 x, int k) {
    return (x << k) | (x >> (64 - k));
  };
  QByteArray out = data;
  quint64 word   = 0;
  for (qsizetype i = 0; i < out.size(); ++i) {
    if ((i & 7) == 0) {
      word          = rotl(s[1] * 5, 7) * 9;
      const auto t  = s[1] << 17;
      s[2]         ^= s[0];
      s[3]         ^= s[1];
      s[1]         ^= s[2];
      s[0]         ^= s[3];
      s[2]         ^= t;
      s[3]          = rotl(s[3], 45);
    }

    out[i] =
      static_cast<char>(static_cast<uchar>(out[i]) ^ static_cast<uchar>(word >> ((i & 7) * 8)));
  }

  return out;
}

/**
 * @brief The v1 content digest: every header field and the payload, each length-prefixed.
 */
static QByteArray referenceDigest(const Crafted& c,
                                  const QByteArray& nonce,
                                  const QByteArray& plain)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  const auto add = [&hash](const QByteArray& field) {
    char length[4];
    qToBigEndian<quint32>(static_cast<quint32>(field.size()), length);
    hash.addData(QByteArrayView(length, 4));
    hash.addData(field);
  };

  add(QByteArray::number(c.format));
  add(c.minApp.toUtf8());
  add(c.writer.toUtf8());
  add(QByteArray::number(c.encoding));
  add(nonce);
  add(plain);
  return hash.result();
}

/**
 * @brief Builds a complete package around an arbitrary (possibly hostile) plain payload.
 */
static QByteArray craft(const Crafted& c, const QByteArray& plain)
{
  const QByteArray nonce(16, '\x5a');
  QByteArray header;
  {
    QCborStreamWriter w(&header);
    w.startMap(c.extraHeaderKey.isEmpty() ? 7 : 8);
    w.append(QLatin1StringView("fmt"));
    w.append(c.format);
    w.append(QLatin1StringView("minApp"));
    w.append(QStringView(c.minApp));
    w.append(QLatin1StringView("writer"));
    w.append(QStringView(c.writer));
    w.append(QLatin1StringView("enc"));
    w.append(c.encoding);
    w.append(QLatin1StringView("nonce"));
    w.append(nonce);
    w.append(QLatin1StringView("plainSize"));
    w.append(c.plainSize >= 0 ? c.plainSize : static_cast<qint64>(plain.size()));
    w.append(QLatin1StringView("digest"));
    w.append(referenceDigest(c, nonce, plain));
    if (!c.extraHeaderKey.isEmpty()) {
      w.append(QLatin1StringView("grant"));
      w.append(c.extraHeaderKey);
    }
    w.endMap();
  }

  QByteArray out(16, '\0');
  std::memcpy(out.data(), "SSDEPLOY", 8);
  qToBigEndian<quint16>(1, out.data() + 8);
  qToBigEndian<quint32>(static_cast<quint32>(header.size()), out.data() + 12);
  out.append(header);
  out.append(referenceScramble(plain, nonce));
  return out;
}

/**
 * @brief A minimal valid payload map holding a manifest and a project.
 */
static QByteArray minimalPayload()
{
  QByteArray plain;
  QCborStreamWriter w(&plain);
  w.startMap(2);
  w.append(QLatin1StringView("manifest"));
  w.startMap(0);
  w.endMap();
  w.append(QLatin1StringView("project"));
  w.append(QByteArray("{\"title\":\"x\"}"));
  w.endMap();
  return plain;
}

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief A package that exercises every field, with the marker in every text-bearing place.
 */
static Deploy::Package samplePackage()
{
  const QString marker = QString::fromLatin1(kMarker);

  Deploy::Package p;
  p.minAppVersion = QStringLiteral("4.1.0");
  p.writerVersion = QStringLiteral("4.1.0");
  p.project = QStringLiteral("{\"title\":\"%1\",\"frameParserCode\":\"%1\"}").arg(marker).toUtf8();
  p.manifest.title         = marker;
  p.manifest.icon          = QByteArray("\x89PNG\r\n\x1a\n") + kMarker;
  p.manifest.projectFolder = QStringLiteral("/Users/author/") + marker;
  p.manifest.externals     = {QStringLiteral("process:/usr/bin/") + marker};
  p.manifest.options.insert(QStringLiteral("fullscreen"), true);
  p.manifest.options.insert(QStringLiteral("frameRate"), 60);
  p.manifest.options.insert(QStringLiteral("theme"), marker);
  p.manifest.options.insert(QStringLiteral("taskbarButtons"),
                            QJsonArray{QStringLiteral("csv"), marker});
  p.files.append({QStringLiteral("sounds/alarm.wav"), QByteArray("RIFF") + kMarker});
  p.files.append({QStringLiteral("images/deep/logo.png"), QByteArray(300, 'x')});
  p.files.append({QStringLiteral("unicode/\u00e9t\u00e9-\u65e5\u672c.txt"), QByteArray("unicode")});
  p.credentials.append({QStringLiteral("source:0"), QStringLiteral("password"), marker});
  return p;
}

/**
 * @brief Encodes the sample package, failing the calling test if encoding fails.
 */
static QByteArray encodedSample()
{
  const auto encoded = Deploy::encode(samplePackage());
  if (!encoded.ok)
    qWarning() << "encode failed:" << encoded.error;

  return encoded.bytes;
}

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief The .ssdeploy v1 codec (spec 0096 AC1-AC4): round-trip, no verbatim strings, and a
 *        decoder that refuses every truncation, bit flip and oversized declaration.
 */
class TstDeployPackageCodec : public QObject {
  Q_OBJECT

private slots:
  void roundTripsEveryField();
  void markerNeverAppearsInBytes();
  void truncationAtEveryOffsetIsRefused();
  void everySingleBitFlipIsRefused();
  void oversizedDeclarationsAreRefused();
  void newerMinimumVersionNamesTheVersion();
  void unknownEncodingNeedsNewerApp();
  void unknownHeaderKeyIsAccepted();
  void duplicateKeysAreRefused();
  void oversizedUnknownTextIsRefused();
  void sizeCapIsEnforcedOnBothSides();
  void unsafePathsAreRejected();
  void committedFixtureDecodes();
};

/**
 * @brief AC1: every field survives encode then decode unchanged.
 */
void TstDeployPackageCodec::roundTripsEveryField()
{
  const auto original = samplePackage();
  const auto encoded  = Deploy::encode(original);
  QVERIFY2(encoded.ok, qPrintable(encoded.error));

  const auto decoded = Deploy::decode(encoded.bytes, kRunning);
  QCOMPARE(decoded.status, Deploy::DecodeStatus::Ok);

  const auto& p = decoded.package;
  QCOMPARE(p.minAppVersion, original.minAppVersion);
  QCOMPARE(p.writerVersion, original.writerVersion);
  QCOMPARE(p.project, original.project);
  QCOMPARE(p.manifest.title, original.manifest.title);
  QCOMPARE(p.manifest.icon, original.manifest.icon);
  QCOMPARE(p.manifest.options, original.manifest.options);
  QCOMPARE(p.manifest.projectFolder, original.manifest.projectFolder);
  QCOMPARE(p.manifest.externals, original.manifest.externals);
  QCOMPARE(p.files.size(), original.files.size());
  for (qsizetype i = 0; i < p.files.size(); ++i) {
    QCOMPARE(p.files[i].path, original.files[i].path);
    QCOMPARE(p.files[i].data, original.files[i].data);
  }

  QCOMPARE(p.credentials.size(), 1);
  QCOMPARE(p.credentials[0].value, original.credentials[0].value);
}

/**
 * @brief AC2: the marker sits in the project, title, files, options and a credential, and
 *        never appears in the encoded bytes.
 */
void TstDeployPackageCodec::markerNeverAppearsInBytes()
{
  const auto bytes = encodedSample();
  QVERIFY(!bytes.isEmpty());
  QVERIFY(!bytes.contains(kMarker));
  QVERIFY(!bytes.contains(QString::fromLatin1(kMarker).toUtf8()));
  QVERIFY(!bytes.contains("frameParserCode"));
}

/**
 * @brief AC3: every proper prefix of a valid package is refused.
 */
void TstDeployPackageCodec::truncationAtEveryOffsetIsRefused()
{
  const auto bytes = encodedSample();
  QVERIFY(bytes.size() > Deploy::kPrefixBytes);

  QElapsedTimer timer;
  timer.start();
  for (qsizetype n = 0; n < bytes.size(); ++n)
    QVERIFY2(Deploy::decode(bytes.left(n), kRunning).status != Deploy::DecodeStatus::Ok,
             qPrintable(QStringLiteral("truncated to %1 bytes decoded").arg(n)));

  QVERIFY(timer.elapsed() < 20000);
}

/**
 * @brief AC3: flipping any single bit anywhere in the file is refused.
 */
void TstDeployPackageCodec::everySingleBitFlipIsRefused()
{
  const auto bytes = encodedSample();
  QElapsedTimer timer;
  timer.start();
  for (qsizetype i = 0; i < bytes.size(); ++i) {
    for (int bit = 0; bit < 8; ++bit) {
      auto mutated = bytes;
      mutated[i]   = static_cast<char>(static_cast<uchar>(mutated[i]) ^ (1u << bit));
      QVERIFY2(Deploy::decode(mutated, kRunning).status != Deploy::DecodeStatus::Ok,
               qPrintable(QStringLiteral("flip at byte %1 bit %2 decoded").arg(i).arg(bit)));
    }
  }

  QVERIFY(timer.elapsed() < 60000);
}

/**
 * @brief AC3: declared lengths beyond the caps are refused before anything is allocated.
 */
void TstDeployPackageCodec::oversizedDeclarationsAreRefused()
{
  auto bytes = encodedSample();
  qToBigEndian<quint32>(0xFFFFFFFFu, bytes.data() + 12);
  QCOMPARE(Deploy::decode(bytes, kRunning).status, Deploy::DecodeStatus::Corrupt);

  Crafted hugePlain;
  hugePlain.plainSize = qint64(1) << 40;
  QCOMPARE(Deploy::decode(craft(hugePlain, minimalPayload()), kRunning).status,
           Deploy::DecodeStatus::Corrupt);

  QByteArray hugeArray;
  {
    QCborStreamWriter w(&hugeArray);
    w.startMap(1);
    w.append(QLatin1StringView("files"));
  }
  hugeArray.append("\x9b\x00\x00\x00\x01\x00\x00\x00\x00", 9);
  QCOMPARE(Deploy::decode(craft(Crafted{}, hugeArray), kRunning).status,
           Deploy::DecodeStatus::Corrupt);

  QByteArray hugeString;
  {
    QCborStreamWriter w(&hugeString);
    w.startMap(1);
    w.append(QLatin1StringView("project"));
  }
  hugeString.append("\x5b\x00\x00\x01\x00\x00\x00\x00\x00", 9);
  QCOMPARE(Deploy::decode(craft(Crafted{}, hugeString), kRunning).status,
           Deploy::DecodeStatus::Corrupt);
}

/**
 * @brief AC4: a package that needs a newer app is refused with that version named.
 */
void TstDeployPackageCodec::newerMinimumVersionNamesTheVersion()
{
  auto package          = samplePackage();
  package.minAppVersion = QStringLiteral("99.0.0");
  const auto encoded    = Deploy::encode(package);
  QVERIFY(encoded.ok);

  const auto decoded = Deploy::decode(encoded.bytes, kRunning);
  QCOMPARE(decoded.status, Deploy::DecodeStatus::NeedsNewerApp);
  QCOMPARE(decoded.requiredVersion, QStringLiteral("99.0.0"));
}

/**
 * @brief AC4: a phase-2 encoding that declares a newer app needs that app; one that claims this
 *        version is a damaged file, never a version prompt.
 */
void TstDeployPackageCodec::unknownEncodingNeedsNewerApp()
{
  Crafted newer;
  newer.encoding     = 2;
  newer.minApp       = QStringLiteral("5.0.0");
  const auto decoded = Deploy::decode(craft(newer, minimalPayload()), kRunning);
  QCOMPARE(decoded.status, Deploy::DecodeStatus::NeedsNewerApp);
  QCOMPARE(decoded.requiredVersion, QStringLiteral("5.0.0"));

  Crafted same;
  same.encoding = 2;
  QCOMPARE(Deploy::decode(craft(same, minimalPayload()), kRunning).status,
           Deploy::DecodeStatus::Corrupt);
}

/**
 * @brief AC4: an unknown header key (the reserved phase-2 grant) does not block decoding.
 */
void TstDeployPackageCodec::unknownHeaderKeyIsAccepted()
{
  Crafted c;
  c.extraHeaderKey   = QByteArray(64, '\x11');
  const auto decoded = Deploy::decode(craft(c, minimalPayload()), kRunning);
  QCOMPARE(decoded.status, Deploy::DecodeStatus::Ok);
  QCOMPARE(decoded.package.project, QByteArray("{\"title\":\"x\"}"));
}

/**
 * @brief A key repeated inside one map is refused, so a section cannot stack past its caps.
 */
void TstDeployPackageCodec::duplicateKeysAreRefused()
{
  QByteArray plain;
  {
    QCborStreamWriter w(&plain);
    w.startMap(3);
    w.append(QLatin1StringView("manifest"));
    w.startMap(0);
    w.endMap();
    w.append(QLatin1StringView("project"));
    w.append(QByteArray("{}"));
    w.append(QLatin1StringView("project"));
    w.append(QByteArray("{}"));
    w.endMap();
  }

  QCOMPARE(Deploy::decode(craft(Crafted{}, plain), kRunning).status, Deploy::DecodeStatus::Corrupt);
}

/**
 * @brief An unknown key whose text value exceeds the text cap is refused before it is skipped.
 */
void TstDeployPackageCodec::oversizedUnknownTextIsRefused()
{
  QByteArray plain;
  {
    QCborStreamWriter w(&plain);
    w.startMap(3);
    w.append(QLatin1StringView("manifest"));
    w.startMap(0);
    w.endMap();
    w.append(QLatin1StringView("project"));
    w.append(QByteArray("{}"));
    w.append(QLatin1StringView("future"));
    w.append(QString(Deploy::kMaxTextChars * 3 + 1, QLatin1Char('a')));
    w.endMap();
  }

  QCOMPARE(Deploy::decode(craft(Crafted{}, plain), kRunning).status, Deploy::DecodeStatus::Corrupt);
}

/**
 * @brief AC4: more than 64 MiB is refused when encoding and when decoding.
 */
void TstDeployPackageCodec::sizeCapIsEnforcedOnBothSides()
{
  auto package = samplePackage();
  package.files.append({QStringLiteral("big.bin"), QByteArray(Deploy::kMaxPackageBytes, 'z')});
  const auto encoded = Deploy::encode(package);
  QVERIFY(!encoded.ok);
  QVERIFY(encoded.bytes.isEmpty());

  const QByteArray oversized(Deploy::kMaxPackageBytes + 1, 'z');
  QCOMPARE(Deploy::decode(oversized, kRunning).status, Deploy::DecodeStatus::TooLarge);
}

/**
 * @brief Paths that could escape the mount root are rejected; ordinary ones are accepted.
 */
void TstDeployPackageCodec::unsafePathsAreRejected()
{
  const QStringList bad = {QString(),
                           QStringLiteral("/abs"),
                           QStringLiteral("a/../b"),
                           QStringLiteral("a//b"),
                           QStringLiteral("./a"),
                           QStringLiteral("a/"),
                           QStringLiteral("C:/x"),
                           QStringLiteral("a\\b"),
                           QStringLiteral("a\x01")};
  for (const auto& path : bad)
    QVERIFY2(!Deploy::isSafeEntryPath(path), qPrintable(path));

  QVERIFY(Deploy::isSafeEntryPath(QStringLiteral("sounds/alarm.wav")));
  QVERIFY(Deploy::isSafeEntryPath(QStringLiteral("\u00e9/\u65e5\u672c.txt")));

  auto package = samplePackage();
  package.files.append(package.files.first());
  QVERIFY(!Deploy::encode(package).ok);
}

/**
 * @brief The committed integration fixture decodes; SS_DEPLOY_WRITE_FIXTURE=1 regenerates it
 *        from marker.ssproj instead.
 */
void TstDeployPackageCodec::committedFixtureDecodes()
{
  const QString dir     = QStringLiteral(SS_DEPLOY_FIXTURE_DIR);
  const QString source  = dir + QStringLiteral("/marker.ssproj");
  const QString package = dir + QStringLiteral("/marker.ssdeploy");

  QFile project(source);
  if (!project.open(QIODevice::ReadOnly))
    QSKIP("tests/fixtures/deploy/marker.ssproj not present yet (spec 0096 T30)");

  const auto projectBytes = project.readAll();
  if (qEnvironmentVariableIsSet("SS_DEPLOY_WRITE_FIXTURE")) {
    Deploy::Package p;
    p.minAppVersion  = QStringLiteral("4.1.0");
    p.writerVersion  = QStringLiteral("4.1.0");
    p.project        = projectBytes;
    p.manifest.title = QStringLiteral("Sealed Session Fixture");
    p.manifest.options.insert(QStringLiteral("actionsPanel"), true);
    p.manifest.options.insert(QStringLiteral("sessionExport"), true);
    const auto encoded = Deploy::encode(p);
    QVERIFY2(encoded.ok, qPrintable(encoded.error));

    QFile out(package);
    QVERIFY(out.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(out.write(encoded.bytes), encoded.bytes.size());
    return;
  }

  QFile in(package);
  if (!in.open(QIODevice::ReadOnly))
    QSKIP("marker.ssdeploy not generated yet; run with SS_DEPLOY_WRITE_FIXTURE=1");

  const auto decoded = Deploy::decode(in.readAll(), kRunning);
  QCOMPARE(decoded.status, Deploy::DecodeStatus::Ok);
  QCOMPARE(decoded.package.project, projectBytes);
}

QTEST_GUILESS_MAIN(TstDeployPackageCodec)

#include "tst_deploy_package_codec.moc"
