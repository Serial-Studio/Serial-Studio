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

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariantMap>

#include "Misc/Extensions/ExtensionCatalog.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

using Misc::ExtensionCatalog::CatalogFilter;
using Misc::ExtensionCatalog::EntryState;

/**
 * @brief Builds a catalog entry with the four fields the filters read.
 */
[[nodiscard]] static QJsonObject makeEntry(const QString& type,
                                           const QString& category,
                                           const QString& title,
                                           const QString& description,
                                           const QString& author)
{
  QJsonObject entry;
  entry.insert("id", QStringLiteral("com.example.entry"));
  entry.insert("type", type);
  entry.insert("category", category);
  entry.insert("title", title);
  entry.insert("description", description);
  entry.insert("author", author);
  return entry;
}

/**
 * @brief Builds a catalog entry that declares the given platform keys.
 */
[[nodiscard]] static QJsonObject makePlatformEntry(const QStringList& keys)
{
  QJsonObject platforms;
  for (const auto& key : keys)
    platforms.insert(key, QJsonObject());

  QJsonObject entry;
  entry.insert("id", QStringLiteral("com.example.entry"));
  if (!keys.isEmpty())
    entry.insert("platforms", platforms);

  return entry;
}

/**
 * @brief Coverage of the pure catalog helpers: filtering, platform resolution, install-path
 *        safety and the entry map QML binds to.
 */
class TstExtensionCatalog : public QObject {
  Q_OBJECT

private slots:
  void entryMatchesFilters_data();
  void entryMatchesFilters();

  void matchesSearch_data();
  void matchesSearch();

  void typeSortRank_data();
  void typeSortRank();

  void platformSupported_data();
  void platformSupported();

  void selectPlatformOverride();
  void resolvePlatform();

  void isPathSafe_data();
  void isPathSafe();

  void isSafePathComponent_data();
  void isSafePathComponent();

  void isLocalRepo_data();
  void isLocalRepo();

  void currentPlatformKey();
  void resolveFileUrl();
  void buildEntryMap();
};

//--------------------------------------------------------------------------------------------------
// Filtering
//--------------------------------------------------------------------------------------------------

void TstExtensionCatalog::entryMatchesFilters_data()
{
  QTest::addColumn<QString>("type");
  QTest::addColumn<QString>("category");
  QTest::addColumn<QString>("search");
  QTest::addColumn<bool>("expected");

  QTest::newRow("no filters at all") << QString() << QString() << QString() << true;
  QTest::newRow("All is not a type") << QStringLiteral("All") << QString() << QString() << true;
  QTest::newRow("All is not a category") << QString() << QStringLiteral("All") << QString() << true;
  QTest::newRow("matching type") << QStringLiteral("plugin") << QString() << QString() << true;
  QTest::newRow("other type") << QStringLiteral("theme") << QString() << QString() << false;
  QTest::newRow("matching category") << QString() << QStringLiteral("Tools") << QString() << true;
  QTest::newRow("category is a substring")
    << QString() << QStringLiteral("ool") << QString() << true;
  QTest::newRow("category is case insensitive")
    << QString() << QStringLiteral("tools") << QString() << true;
  QTest::newRow("other category") << QString() << QStringLiteral("Themes") << QString() << false;
  QTest::newRow("search hits the title")
    << QString() << QString() << QStringLiteral("Logger") << true;
  QTest::newRow("search hits the description")
    << QString() << QString() << QStringLiteral("records") << true;
  QTest::newRow("search hits the author")
    << QString() << QString() << QStringLiteral("Spataru") << true;
  QTest::newRow("search is case insensitive")
    << QString() << QString() << QStringLiteral("LOGGER") << true;
  QTest::newRow("search hits nothing")
    << QString() << QString() << QStringLiteral("oscilloscope") << false;
  QTest::newRow("type matches but search does not")
    << QStringLiteral("plugin") << QString() << QStringLiteral("oscilloscope") << false;
  QTest::newRow("search matches but type does not")
    << QStringLiteral("theme") << QString() << QStringLiteral("Logger") << false;
}

/**
 * @brief The three filters are conjunctive, "All" and the empty string both disable one, and only
 *        the category and search comparisons are case insensitive.
 */
void TstExtensionCatalog::entryMatchesFilters()
{
  QFETCH(QString, type);
  QFETCH(QString, category);
  QFETCH(QString, search);
  QFETCH(bool, expected);

  const auto entry = makeEntry(QStringLiteral("plugin"),
                               QStringLiteral("Tools"),
                               QStringLiteral("Data Logger"),
                               QStringLiteral("Silently records every frame"),
                               QStringLiteral("Alex Spataru"));

  const CatalogFilter filter{type, category, search};
  QCOMPARE(Misc::ExtensionCatalog::entryMatchesFilters(entry, filter), expected);
}

void TstExtensionCatalog::matchesSearch_data()
{
  QTest::addColumn<QString>("search");
  QTest::addColumn<bool>("expected");

  QTest::newRow("empty search matches") << QString() << true;
  QTest::newRow("title hit") << QStringLiteral("Logger") << true;
  QTest::newRow("description hit") << QStringLiteral("frame") << true;
  QTest::newRow("case insensitive") << QStringLiteral("logger") << true;
  QTest::newRow("no hit") << QStringLiteral("waterfall") << false;
}

/**
 * @brief The orphan-entry search looks at the title and description only; an empty search keeps
 *        every entry, which is what lists an unfiltered catalog in full.
 */
void TstExtensionCatalog::matchesSearch()
{
  QFETCH(QString, search);
  QFETCH(bool, expected);

  const auto matched = Misc::ExtensionCatalog::matchesSearch(
    QStringLiteral("Data Logger"), QStringLiteral("Records every frame"), search);

  QCOMPARE(matched, expected);
}

void TstExtensionCatalog::typeSortRank_data()
{
  QTest::addColumn<QString>("type");
  QTest::addColumn<int>("expected");

  QTest::newRow("plugin first") << QStringLiteral("plugin") << 0;
  QTest::newRow("theme second") << QStringLiteral("theme") << 1;
  QTest::newRow("frame parser third") << QStringLiteral("frame-parser") << 2;
  QTest::newRow("project template fourth") << QStringLiteral("project-template") << 3;
  QTest::newRow("unknown type sorts last") << QStringLiteral("widget") << 99;
  QTest::newRow("empty type sorts last") << QString() << 99;
}

/**
 * @brief The grid's type order, including the "everything unknown goes last" fallback that keeps a
 *        catalog with a new type stable.
 */
void TstExtensionCatalog::typeSortRank()
{
  QFETCH(QString, type);
  QFETCH(int, expected);

  QCOMPARE(Misc::ExtensionCatalog::typeSortRank(type), expected);
}

//--------------------------------------------------------------------------------------------------
// Platform resolution
//--------------------------------------------------------------------------------------------------

void TstExtensionCatalog::platformSupported_data()
{
  QTest::addColumn<QStringList>("keys");
  QTest::addColumn<bool>("expected");

  QTest::newRow("no platforms declared") << QStringList() << true;
  QTest::newRow("exact key") << QStringList{QStringLiteral("darwin/arm64")} << true;
  QTest::newRow("os wildcard") << QStringList{QStringLiteral("darwin/*")} << true;
  QTest::newRow("global wildcard") << QStringList{QStringLiteral("*")} << true;
  QTest::newRow("other architecture") << QStringList{QStringLiteral("darwin/x86_64")} << false;
  QTest::newRow("other os") << QStringList{QStringLiteral("linux/arm64")} << false;
  QTest::newRow("other os wildcard") << QStringList{QStringLiteral("windows/*")} << false;
  QTest::newRow("one of several") << QStringList{QStringLiteral("linux/x86_64"),
                                                 QStringLiteral("darwin/arm64")}
                                  << true;
}

/**
 * @brief Availability is decided by exact key, then the OS wildcard, then the global wildcard; an
 *        entry that declares no platforms at all is available everywhere.
 */
void TstExtensionCatalog::platformSupported()
{
  QFETCH(QStringList, keys);
  QFETCH(bool, expected);

  const auto entry = makePlatformEntry(keys);
  const auto key   = QStringLiteral("darwin/arm64");
  QCOMPARE(Misc::ExtensionCatalog::platformSupported(entry, key), expected);
}

/**
 * @brief The override picker prefers the exact key, falls back to the OS wildcard and then to the
 *        global one, and reports nothing when no bucket applies.
 */
void TstExtensionCatalog::selectPlatformOverride()
{
  const auto key = QStringLiteral("linux/x86_64");

  QVariantMap exact;
  exact.insert(QStringLiteral("runtime"), QStringLiteral("exact"));

  QVariantMap wildcard;
  wildcard.insert(QStringLiteral("runtime"), QStringLiteral("os"));

  QVariantMap global;
  global.insert(QStringLiteral("runtime"), QStringLiteral("any"));

  QVariantMap platforms;
  QCOMPARE(Misc::ExtensionCatalog::selectPlatformOverride(platforms, key).isEmpty(), true);

  platforms.insert(QStringLiteral("windows/x86_64"), exact);
  QCOMPARE(Misc::ExtensionCatalog::selectPlatformOverride(platforms, key).isEmpty(), true);

  platforms.insert(QStringLiteral("*"), global);
  QCOMPARE(Misc::ExtensionCatalog::selectPlatformOverride(platforms, key).value("runtime"),
           QVariant(QStringLiteral("any")));

  platforms.insert(QStringLiteral("linux/*"), wildcard);
  QCOMPARE(Misc::ExtensionCatalog::selectPlatformOverride(platforms, key).value("runtime"),
           QVariant(QStringLiteral("os")));

  platforms.insert(key, exact);
  QCOMPARE(Misc::ExtensionCatalog::selectPlatformOverride(platforms, key).value("runtime"),
           QVariant(QStringLiteral("exact")));
}

/**
 * @brief Resolving a plugin's metadata folds the matching platform bucket over the base fields and
 *        leaves everything the bucket does not name untouched.
 */
void TstExtensionCatalog::resolvePlatform()
{
  QJsonObject platformEntry;
  platformEntry.insert(QStringLiteral("entry"), QStringLiteral("main-linux.py"));

  QJsonObject platforms;
  platforms.insert(QStringLiteral("linux/*"), platformEntry);

  QJsonObject meta;
  meta.insert(QStringLiteral("entry"), QStringLiteral("main.py"));
  meta.insert(QStringLiteral("runtime"), QStringLiteral("python3"));
  meta.insert(QStringLiteral("platforms"), platforms);

  const auto resolved =
    Misc::ExtensionCatalog::resolvePlatform(meta, QStringLiteral("linux/x86_64"));
  QCOMPARE(resolved.value("entry").toString(), QStringLiteral("main-linux.py"));
  QCOMPARE(resolved.value("runtime").toString(), QStringLiteral("python3"));

  const auto other = Misc::ExtensionCatalog::resolvePlatform(meta, QStringLiteral("darwin/arm64"));
  QCOMPARE(other.value("entry").toString(), QStringLiteral("main.py"));

  QJsonObject plain;
  plain.insert(QStringLiteral("entry"), QStringLiteral("main.py"));
  const auto untouched =
    Misc::ExtensionCatalog::resolvePlatform(plain, QStringLiteral("linux/x86_64"));
  QCOMPARE(untouched.value("entry").toString(), QStringLiteral("main.py"));
}

/**
 * @brief The key is "<os>/<arch>" with a normalized architecture, which is what the manifest's
 *        platform buckets are written against.
 */
void TstExtensionCatalog::currentPlatformKey()
{
  const auto key = Misc::ExtensionCatalog::currentPlatformKey();

  QVERIFY(!key.isEmpty());
  QVERIFY(key.count('/') == 1);
  QVERIFY(!key.startsWith('/'));
  QVERIFY(!key.endsWith('/'));

  const auto os = key.left(key.indexOf('/'));
  QVERIFY(os == QStringLiteral("darwin") || os == QStringLiteral("windows")
          || os == QStringLiteral("linux"));
}

//--------------------------------------------------------------------------------------------------
// Path safety
//--------------------------------------------------------------------------------------------------

void TstExtensionCatalog::isPathSafe_data()
{
  QTest::addColumn<QString>("path");
  QTest::addColumn<bool>("expected");

  QTest::newRow("file in the base") << QStringLiteral("/opt/ext/plugin/main.py") << true;
  QTest::newRow("file in a subdirectory") << QStringLiteral("/opt/ext/plugin/lib/util.py") << true;
  QTest::newRow("the base itself") << QStringLiteral("/opt/ext/plugin") << true;
  QTest::newRow("parent traversal") << QStringLiteral("/opt/ext/plugin/../evil.py") << false;
  QTest::newRow("deep traversal") << QStringLiteral("/opt/ext/plugin/lib/../../evil.py") << false;
  QTest::newRow("traversal to the root")
    << QStringLiteral("/opt/ext/plugin/../../../etc/passwd") << false;
  QTest::newRow("sibling with the base as prefix")
    << QStringLiteral("/opt/ext/plugin-evil/x.py") << false;
  QTest::newRow("unrelated absolute path") << QStringLiteral("/etc/passwd") << false;
  QTest::newRow("parent of the base") << QStringLiteral("/opt/ext") << false;
  QTest::newRow("dot segment inside the base")
    << QStringLiteral("/opt/ext/plugin/./main.py") << true;
}

/**
 * @brief The install-path guard: a resolved path is accepted only when it is the install directory
 *        or sits under it. Traversal and the "same prefix, different directory" trap are the two
 *        cases that decide whether a manifest can write outside its own folder.
 */
void TstExtensionCatalog::isPathSafe()
{
  QFETCH(QString, path);
  QFETCH(bool, expected);

  const auto base = QStringLiteral("/opt/ext/plugin");
  QCOMPARE(Misc::ExtensionCatalog::isPathSafe(path, base), expected);
}

void TstExtensionCatalog::isSafePathComponent_data()
{
  QTest::addColumn<QString>("component");
  QTest::addColumn<bool>("expected");

  QTest::newRow("plain identifier") << QStringLiteral("com.example.plugin") << true;
  QTest::newRow("single dots are fine") << QStringLiteral("v1.2.3") << true;
  QTest::newRow("parent reference") << QStringLiteral("..") << false;
  QTest::newRow("embedded parent reference") << QStringLiteral("a..b") << false;
  QTest::newRow("traversal component") << QStringLiteral("../../etc") << false;
  QTest::newRow("forward slash") << QStringLiteral("a/b") << false;
  QTest::newRow("backslash") << QStringLiteral("a\\b") << false;
  QTest::newRow("leading slash") << QStringLiteral("/etc") << false;
}

/**
 * @brief An extension's id and type become one path component each; anything carrying a separator
 *        or a parent reference is refused before any directory is created.
 */
void TstExtensionCatalog::isSafePathComponent()
{
  QFETCH(QString, component);
  QFETCH(bool, expected);

  QCOMPARE(Misc::ExtensionCatalog::isSafePathComponent(component), expected);
}

//--------------------------------------------------------------------------------------------------
// Repositories & entry maps
//--------------------------------------------------------------------------------------------------

void TstExtensionCatalog::isLocalRepo_data()
{
  QTest::addColumn<QString>("url");
  QTest::addColumn<bool>("expected");

  QTest::newRow("absolute unix path") << QStringLiteral("/home/user/repo") << true;
  QTest::newRow("file url") << QStringLiteral("file:///home/user/repo") << true;
  QTest::newRow("https url") << QStringLiteral("https://example.com/manifest.json") << false;
  QTest::newRow("empty") << QString() << false;
}

/**
 * @brief A repository is local when it is an absolute path or a file:// URL; everything else is
 *        fetched over the network.
 */
void TstExtensionCatalog::isLocalRepo()
{
  QFETCH(QString, url);
  QFETCH(bool, expected);

  QCOMPARE(Misc::ExtensionCatalog::isLocalRepo(url), expected);
}

/**
 * @brief File URLs are resolved by concatenation against the repository base, so a base without a
 *        trailing separator is the caller's responsibility.
 */
void TstExtensionCatalog::resolveFileUrl()
{
  const auto base = QStringLiteral("https://example.com/ext/");
  const auto url  = Misc::ExtensionCatalog::resolveFileUrl(base, QStringLiteral("lib/main.py"));

  QCOMPARE(url.toString(), QStringLiteral("https://example.com/ext/lib/main.py"));
}

/**
 * @brief The map QML renders carries the entry's own fields plus the five install-state keys the
 *        delegate binds to.
 */
void TstExtensionCatalog::buildEntryMap()
{
  QJsonObject entry = makeEntry(QStringLiteral("plugin"),
                                QStringLiteral("Tools"),
                                QStringLiteral("Data Logger"),
                                QStringLiteral("Records every frame"),
                                QStringLiteral("Alex Spataru"));
  entry.insert(QStringLiteral("version"), QStringLiteral("2.0"));

  EntryState state;
  state.installed        = true;
  state.pluginRunning    = true;
  state.updateAvailable  = true;
  state.installedVersion = QStringLiteral("1.0");

  const auto map =
    Misc::ExtensionCatalog::buildEntryMap(entry, state, QStringLiteral("darwin/arm64"));

  QCOMPARE(map.value("id").toString(), QStringLiteral("com.example.entry"));
  QCOMPARE(map.value("version").toString(), QStringLiteral("2.0"));
  QCOMPARE(map.value("installed").toBool(), true);
  QCOMPARE(map.value("pluginRunning").toBool(), true);
  QCOMPARE(map.value("updateAvailable").toBool(), true);
  QCOMPARE(map.value("installedVersion").toString(), QStringLiteral("1.0"));
  QCOMPARE(map.value("platformAvailable").toBool(), true);

  const auto restricted = Misc::ExtensionCatalog::buildEntryMap(
    makePlatformEntry({QStringLiteral("windows/x86_64")}), state, QStringLiteral("darwin/arm64"));
  QCOMPARE(restricted.value("platformAvailable").toBool(), false);
}

QTEST_APPLESS_MAIN(TstExtensionCatalog)

#include "tst_extension_catalog.moc"
