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

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantMap>

#include "Misc/Extensions/ExtensionCatalog.h"
#include "Misc/Extensions/ExtensionInstaller.h"
#include "Misc/WorkspaceManager.h"
#include "support/FakeTransport.h"

// The installer writes into the workspace, so the fixture points WorkspaceManager at a temporary
// tree for the whole suite. Local (file-based) installs are driven directly; the download lane is
// driven through FakeTransport.

/**
 * @brief Pins the extension install trust boundary (spec 0075, K3/K5/K12): a catalog without
 *        digests is refused, a file whose bytes do not match its digest aborts the install, and
 *        every failure leaves the previously installed version exactly as it was.
 */
class TstExtensionInstaller : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();

  void catalogV1EntryIsRefused();
  void entryWithBadDigestIsRefused();
  void sidecarDigestsInstallEveryFile();
  void sidecarMismatchLeavesPreviousVersionIntact();
  void metadataFileNeedsNoDigest();
  void metadataOnlyEntryIsRefused();
  void platformDigestsOverrideEntryWideOnes();
  void goodDigestInstallsEveryFile();
  void corruptFileLeavesPreviousVersionIntact();
  void installedRecordCarriesDigests();
  void digestMatchRejectsWrongSizeAndBytes();
  void versionCompareRefusesDowngrade();
  void repoSchemePolicy();

private:
  [[nodiscard]] QString repoDir() const;
  [[nodiscard]] QString installDir() const;
  void writeRepoFile(const QString& name, const QByteArray& payload);
  [[nodiscard]] QVariantMap entryFor(const QVariantList& files,
                                     const QString& version,
                                     const QVariantMap& digests = {}) const;

  QTemporaryDir m_root;
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the lowercase hex SHA-256 a catalog entry would publish for these bytes.
 */
static QString digestOf(const QByteArray& payload)
{
  return QString::fromLatin1(QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex());
}

/**
 * @brief Builds one sidecar digest row: what `"sha256": {"<path>": ...}` maps a path to.
 */
static QVariantMap digestRow(const QByteArray& payload)
{
  QVariantMap row;
  row.insert(QStringLiteral("sha256"), digestOf(payload));
  row.insert(QStringLiteral("size"), static_cast<qlonglong>(payload.size()));
  return row;
}

/**
 * @brief Builds one catalog v2 file row.
 */
static QVariantMap fileRow(const QString& path, const QString& sha256, qint64 size)
{
  QVariantMap row;
  row.insert(QStringLiteral("path"), path);
  row.insert(QStringLiteral("sha256"), sha256);
  row.insert(QStringLiteral("size"), static_cast<qlonglong>(size));
  return row;
}

/**
 * @brief Returns the repository directory the fixture copies packages from.
 */
QString TstExtensionInstaller::repoDir() const
{
  return m_root.path() + QStringLiteral("/repo/");
}

/**
 * @brief Returns the directory a "theme" package with this suite's id installs into.
 */
QString TstExtensionInstaller::installDir() const
{
  return m_root.path() + QStringLiteral("/workspace/Extensions/theme/demo");
}

/**
 * @brief Writes one file into the repository directory.
 */
void TstExtensionInstaller::writeRepoFile(const QString& name, const QByteArray& payload)
{
  QDir().mkpath(repoDir());
  QFile file(repoDir() + name);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write(payload);
  file.close();
}

/**
 * @brief Builds the local catalog entry the installer consumes.
 */
QVariantMap TstExtensionInstaller::entryFor(const QVariantList& files,
                                            const QString& version,
                                            const QVariantMap& digests) const
{
  QVariantMap entry;
  entry.insert(QStringLiteral("id"), QStringLiteral("demo"));
  entry.insert(QStringLiteral("type"), QStringLiteral("theme"));
  entry.insert(QStringLiteral("version"), version);
  entry.insert(QStringLiteral("files"), files);
  if (!digests.isEmpty())
    entry.insert(QStringLiteral("sha256"), digests);

  entry.insert(QStringLiteral("_isLocal"), true);
  entry.insert(QStringLiteral("_repoBase"), repoDir());
  return entry;
}

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects the workspace at a temporary tree for the whole suite.
 */
void TstExtensionInstaller::initTestCase()
{
  QVERIFY(m_root.isValid());
  QVERIFY(QDir().mkpath(m_root.path() + QStringLiteral("/workspace")));
  Misc::WorkspaceManager::instance().setTemporaryPath(m_root.path() + QStringLiteral("/workspace"));
}

/**
 * @brief Restores the real workspace.
 */
void TstExtensionInstaller::cleanupTestCase()
{
  Misc::WorkspaceManager::instance().clearTemporaryPath();
}

/**
 * @brief Starts every case from an empty extensions tree and a fresh repository.
 */
void TstExtensionInstaller::init()
{
  QDir(m_root.path() + QStringLiteral("/workspace/Extensions")).removeRecursively();
  QDir(repoDir()).removeRecursively();
}

//--------------------------------------------------------------------------------------------------
// Catalog gate
//--------------------------------------------------------------------------------------------------

/**
 * @brief A v1 entry (files as bare strings) is refused: there is nothing to verify against.
 */
void TstExtensionInstaller::catalogV1EntryIsRefused()
{
  writeRepoFile(QStringLiteral("theme.json"), QByteArrayLiteral("{}"));

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());
  QSignalSpy failed(&installer, &Misc::ExtensionInstaller::installFailed);

  const QVariantList files{QStringLiteral("theme.json")};
  QVERIFY(!installer.install(entryFor(files, QStringLiteral("1.0.0"))));
  QCOMPARE(failed.count(), 1);
  QVERIFY(!installer.isInstalled(QStringLiteral("demo")));
  QVERIFY(!QDir(installDir()).exists());
}

/**
 * @brief An entry whose digest is not 64 lowercase hex characters is refused before any I/O.
 */
void TstExtensionInstaller::entryWithBadDigestIsRefused()
{
  writeRepoFile(QStringLiteral("theme.json"), QByteArrayLiteral("{}"));

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList files{fileRow(QStringLiteral("theme.json"), QStringLiteral("nope"), 2)};
  QVERIFY(!installer.install(entryFor(files, QStringLiteral("1.0.0"))));
  QVERIFY(!installer.lastError().isEmpty());
  QVERIFY(!QDir(installDir()).exists());
}

//--------------------------------------------------------------------------------------------------
// Install & rollback
//--------------------------------------------------------------------------------------------------

/**
 * @brief A verified package lands whole in the install directory.
 */
void TstExtensionInstaller::goodDigestInstallsEveryFile()
{
  const auto meta = QByteArrayLiteral("{\"id\":\"demo\"}");
  const auto body = QByteArrayLiteral("body-v1");
  writeRepoFile(QStringLiteral("info.json"), meta);
  writeRepoFile(QStringLiteral("theme.qml"), body);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());
  QSignalSpy installed(&installer, &Misc::ExtensionInstaller::installed);

  const QVariantList files{
    fileRow(QStringLiteral("info.json"), digestOf(meta), meta.size()),
    fileRow(QStringLiteral("theme.qml"), digestOf(body), body.size()),
  };

  QVERIFY(installer.install(entryFor(files, QStringLiteral("1.0.0"))));
  QCOMPARE(installed.count(), 1);
  QVERIFY(installer.isInstalled(QStringLiteral("demo")));
  QVERIFY(QFile::exists(installDir() + QStringLiteral("/info.json")));
  QVERIFY(QFile::exists(installDir() + QStringLiteral("/theme.qml")));
  QVERIFY(!QDir(installDir() + QStringLiteral(".staging")).exists());
  QVERIFY(!QDir(installDir() + QStringLiteral(".previous")).exists());
}

/**
 * @brief A corrupted second file aborts the update, and the version already installed keeps its
 *        files and its recorded version. This is the whole point of staging.
 */
void TstExtensionInstaller::corruptFileLeavesPreviousVersionIntact()
{
  const auto meta = QByteArrayLiteral("{\"id\":\"demo\"}");
  const auto body = QByteArrayLiteral("body-v1");
  writeRepoFile(QStringLiteral("info.json"), meta);
  writeRepoFile(QStringLiteral("theme.qml"), body);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList good{
    fileRow(QStringLiteral("info.json"), digestOf(meta), meta.size()),
    fileRow(QStringLiteral("theme.qml"), digestOf(body), body.size()),
  };
  QVERIFY(installer.install(entryFor(good, QStringLiteral("1.0.0"))));

  const auto tampered = QByteArrayLiteral("body-v2-tampered");
  writeRepoFile(QStringLiteral("theme.qml"), tampered);

  QSignalSpy failed(&installer, &Misc::ExtensionInstaller::installFailed);
  const QVariantList mismatched{
    fileRow(QStringLiteral("info.json"), digestOf(meta), meta.size()),
    fileRow(QStringLiteral("theme.qml"), digestOf(body), body.size()),
  };
  QVERIFY(!installer.install(entryFor(mismatched, QStringLiteral("2.0.0"))));
  QCOMPARE(failed.count(), 1);

  QFile kept(installDir() + QStringLiteral("/theme.qml"));
  QVERIFY(kept.open(QIODevice::ReadOnly));
  QCOMPARE(kept.readAll(), body);
  QCOMPARE(installer.installedVersion(QStringLiteral("demo")), QStringLiteral("1.0.0"));
  QVERIFY(!QDir(installDir() + QStringLiteral(".staging")).exists());
}

/**
 * @brief installed.json records the digest of every file, which is what a repair check reads.
 */
void TstExtensionInstaller::installedRecordCarriesDigests()
{
  const auto meta = QByteArrayLiteral("{\"id\":\"demo\"}");
  writeRepoFile(QStringLiteral("info.json"), meta);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList files{fileRow(QStringLiteral("info.json"), digestOf(meta), meta.size())};
  QVERIFY(installer.install(entryFor(files, QStringLiteral("1.0.0"))));

  const auto record = installer.installedInfo(QStringLiteral("demo"));
  const auto rows   = record.value(QStringLiteral("files")).toArray();
  QCOMPARE(rows.size(), 1);
  QCOMPARE(rows.at(0).toObject().value(QStringLiteral("sha256")).toString(), digestOf(meta));
  QCOMPARE(record.value(QStringLiteral("type")).toString(), QStringLiteral("theme"));
}

//--------------------------------------------------------------------------------------------------
// Sidecar digest form
//--------------------------------------------------------------------------------------------------

/**
 * @brief The sidecar form installs exactly like the inline one. It exists so `files` can stay a
 *        plain string list -- what releases up to 4.1.0 read -- while still carrying digests.
 */
void TstExtensionInstaller::sidecarDigestsInstallEveryFile()
{
  const auto meta = QByteArrayLiteral("{\"id\":\"demo\"}");
  const auto body = QByteArrayLiteral("body-v1");
  writeRepoFile(QStringLiteral("info.json"), meta);
  writeRepoFile(QStringLiteral("theme.qml"), body);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList files{QStringLiteral("info.json"), QStringLiteral("theme.qml")};
  QVariantMap digests;
  digests.insert(QStringLiteral("info.json"), digestRow(meta));
  digests.insert(QStringLiteral("theme.qml"), digestRow(body));

  QVERIFY(installer.install(entryFor(files, QStringLiteral("1.0.0"), digests)));
  QVERIFY(installer.isInstalled(QStringLiteral("demo")));

  QFile written(installDir() + QStringLiteral("/theme.qml"));
  QVERIFY(written.open(QIODevice::ReadOnly));
  QCOMPARE(written.readAll(), body);
}

/**
 * @brief A sidecar digest that does not match the bytes aborts the install, and the version
 *        already installed keeps its files. The staging guarantee does not depend on the shape
 *        the digest was published in.
 */
void TstExtensionInstaller::sidecarMismatchLeavesPreviousVersionIntact()
{
  const auto body = QByteArrayLiteral("body-v1");
  writeRepoFile(QStringLiteral("theme.qml"), body);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList files{QStringLiteral("theme.qml")};
  QVariantMap digests;
  digests.insert(QStringLiteral("theme.qml"), digestRow(body));
  QVERIFY(installer.install(entryFor(files, QStringLiteral("1.0.0"), digests)));

  writeRepoFile(QStringLiteral("theme.qml"), QByteArrayLiteral("body-v2-tampered"));

  QSignalSpy failed(&installer, &Misc::ExtensionInstaller::installFailed);
  QVERIFY(!installer.install(entryFor(files, QStringLiteral("2.0.0"), digests)));
  QCOMPARE(failed.count(), 1);

  QFile kept(installDir() + QStringLiteral("/theme.qml"));
  QVERIFY(kept.open(QIODevice::ReadOnly));
  QCOMPARE(kept.readAll(), body);
  QCOMPARE(installer.installedVersion(QStringLiteral("demo")), QStringLiteral("1.0.0"));
}

/**
 * @brief info.json needs no digest: the digest would have to live inside the bytes it covers.
 *        Every other file still does, and the metadata still lands in the install directory.
 */
void TstExtensionInstaller::metadataFileNeedsNoDigest()
{
  const auto meta = QByteArrayLiteral("{\"id\":\"demo\"}");
  const auto body = QByteArrayLiteral("body-v1");
  writeRepoFile(QStringLiteral("info.json"), meta);
  writeRepoFile(QStringLiteral("theme.qml"), body);

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());

  const QVariantList files{QStringLiteral("info.json"), QStringLiteral("theme.qml")};
  QVariantMap digests;
  digests.insert(QStringLiteral("theme.qml"), digestRow(body));

  QVERIFY(installer.install(entryFor(files, QStringLiteral("1.0.0"), digests)));

  QFile installedMeta(installDir() + QStringLiteral("/info.json"));
  QVERIFY(installedMeta.open(QIODevice::ReadOnly));
  QCOMPARE(installedMeta.readAll(), meta);

  const auto rows =
    installer.installedInfo(QStringLiteral("demo")).value(QStringLiteral("files")).toArray();
  QCOMPARE(rows.size(), 2);
  QVERIFY(!rows.at(0).toObject().contains(QStringLiteral("sha256")));
  QCOMPARE(rows.at(1).toObject().value(QStringLiteral("sha256")).toString(), digestOf(body));
}

/**
 * @brief An entry whose only file is the exempt info.json is refused: it would install nothing
 *        that was verified against anything.
 */
void TstExtensionInstaller::metadataOnlyEntryIsRefused()
{
  writeRepoFile(QStringLiteral("info.json"), QByteArrayLiteral("{\"id\":\"demo\"}"));

  Test::FakeTransport transport;
  Misc::ExtensionInstaller installer(transport, Misc::WorkspaceManager::instance());
  QSignalSpy failed(&installer, &Misc::ExtensionInstaller::installFailed);

  const QVariantList files{QStringLiteral("info.json")};
  QVERIFY(!installer.install(entryFor(files, QStringLiteral("1.0.0"))));
  QCOMPARE(failed.count(), 1);
  QVERIFY(!QDir(installDir()).exists());
}

//--------------------------------------------------------------------------------------------------
// Pure catalog decisions
//--------------------------------------------------------------------------------------------------

/**
 * @brief A platform override's digests are merged over the entry-wide table, so an override
 *        publishes digests only for the files it adds.
 */
void TstExtensionInstaller::platformDigestsOverrideEntryWideOnes()
{
  QVariantMap base;
  base.insert(QStringLiteral("a"), QStringLiteral("entry"));
  base.insert(QStringLiteral("b"), QStringLiteral("entry"));

  QVariantMap plat;
  plat.insert(QStringLiteral("b"), QStringLiteral("platform"));
  plat.insert(QStringLiteral("c"), QStringLiteral("platform"));

  const auto merged = Misc::ExtensionCatalog::mergeDigests(base, plat);
  QCOMPARE(merged.value(QStringLiteral("a")).toString(), QStringLiteral("entry"));
  QCOMPARE(merged.value(QStringLiteral("b")).toString(), QStringLiteral("platform"));
  QCOMPARE(merged.value(QStringLiteral("c")).toString(), QStringLiteral("platform"));
}

/**
 * @brief The digest check rejects both a wrong size and wrong bytes of the right size.
 */
void TstExtensionInstaller::digestMatchRejectsWrongSizeAndBytes()
{
  const auto payload = QByteArrayLiteral("abcd");

  Misc::ExtensionCatalog::CatalogFile file;
  file.path   = QStringLiteral("x.bin");
  file.sha256 = digestOf(payload);
  file.size   = payload.size();

  QVERIFY(Misc::ExtensionCatalog::digestMatches(payload, file));
  QVERIFY(!Misc::ExtensionCatalog::digestMatches(QByteArrayLiteral("abce"), file));
  QVERIFY(!Misc::ExtensionCatalog::digestMatches(QByteArrayLiteral("abcde"), file));
}

/**
 * @brief Version comparison is numeric, so a lower remote version is never an update.
 */
void TstExtensionInstaller::versionCompareRefusesDowngrade()
{
  QVERIFY(Misc::ExtensionCatalog::compareVersions(QStringLiteral("1.10.0"), QStringLiteral("1.9.0"))
          > 0);
  QVERIFY(Misc::ExtensionCatalog::compareVersions(QStringLiteral("1.9.0"), QStringLiteral("1.10.0"))
          < 0);
  QCOMPARE(
    Misc::ExtensionCatalog::compareVersions(QStringLiteral("2.0.0"), QStringLiteral("2.0.0")), 0);
}

/**
 * @brief A repository is a local folder or https; plain http is refused.
 */
void TstExtensionInstaller::repoSchemePolicy()
{
  QVERIFY(
    Misc::ExtensionCatalog::isTrustedRepoUrl(QStringLiteral("https://example.com/manifest.json")));
  QVERIFY(Misc::ExtensionCatalog::isTrustedRepoUrl(QStringLiteral("/home/user/repo")));
  QVERIFY(Misc::ExtensionCatalog::isTrustedRepoUrl(QStringLiteral("file:///home/user/repo")));
  QVERIFY(
    !Misc::ExtensionCatalog::isTrustedRepoUrl(QStringLiteral("http://example.com/manifest.json")));
  QVERIFY(!Misc::ExtensionCatalog::isTrustedRepoUrl(QString()));
}

QTEST_GUILESS_MAIN(TstExtensionInstaller)

#include "tst_extension_installer.moc"
