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

#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

/**
 * @brief Pure decisions about extension catalog entries: which entries a filter keeps, how an
 *        entry is lowered into the map QML renders, which platform override applies, and whether
 *        a resolved path stays inside the directory it belongs to. Nothing here holds state,
 *        reads a singleton or touches the network, which is what makes the whole surface
 *        checkable from a unit test.
 */
namespace Misc::ExtensionCatalog {

/**
 * @brief The three user-facing catalog filters. An empty field, and the literal "All" for type
 *        and category, mean "do not filter on this".
 */
struct CatalogFilter {
  QString type;
  QString category;
  QString search;
};

/**
 * @brief One installable file of a catalog v2 entry: where it goes, the digest an install
 *        verifies the downloaded bytes against, and the size checked before hashing them.
 */
struct CatalogFile {
  QString path;
  QString sha256;
  qint64 size = 0;
};

/**
 * @brief The install-state facts the catalog cannot derive from a manifest entry on its own; the
 *        facade supplies them per entry.
 */
struct EntryState {
  bool installed       = false;
  bool pluginRunning   = false;
  bool updateAvailable = false;
  QString installedVersion;
};

[[nodiscard]] QStringList extensionTypes();
[[nodiscard]] QString currentPlatformKey();
[[nodiscard]] int typeSortRank(const QString& type);
[[nodiscard]] bool isLocalRepo(const QString& url);
[[nodiscard]] bool isTrustedRepoUrl(const QString& url);
[[nodiscard]] bool hasVerifiableFiles(const QVariantList& files);
[[nodiscard]] bool digestMatches(const QByteArray& payload, const CatalogFile& file);
[[nodiscard]] int compareVersions(const QString& remote, const QString& local);

[[nodiscard]] QList<CatalogFile> parseFileList(const QVariantList& files, QString* rejectReason);
[[nodiscard]] bool isSafePathComponent(const QString& component);
[[nodiscard]] bool isPathSafe(const QString& filePath, const QString& baseDir);
[[nodiscard]] QUrl resolveFileUrl(const QString& repoBaseUrl, const QString& relativePath);
[[nodiscard]] bool platformSupported(const QJsonObject& entry, const QString& platformKey);
[[nodiscard]] bool entryMatchesFilters(const QJsonObject& entry, const CatalogFilter& filter);
[[nodiscard]] QJsonObject resolvePlatform(const QJsonObject& meta, const QString& platformKey);

[[nodiscard]] bool matchesSearch(const QString& title,
                                 const QString& description,
                                 const QString& search);

[[nodiscard]] QVariantMap selectPlatformOverride(const QVariantMap& platforms,
                                                 const QString& platformKey);

[[nodiscard]] QVariantMap buildEntryMap(const QJsonObject& entry,
                                        const EntryState& state,
                                        const QString& platformKey);

}  // namespace Misc::ExtensionCatalog
