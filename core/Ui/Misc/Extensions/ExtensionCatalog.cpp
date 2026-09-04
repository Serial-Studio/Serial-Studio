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

#include "Misc/Extensions/ExtensionCatalog.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QMap>
#include <QRegularExpression>
#include <QSysInfo>
#include <QVersionNumber>

//--------------------------------------------------------------------------------------------------
// Catalog vocabulary
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the extension types the filter combo offers, "All" first.
 */
QStringList Misc::ExtensionCatalog::extensionTypes()
{
  return {
    QStringLiteral("All"),
    QStringLiteral("theme"),
    QStringLiteral("frame-parser"),
    QStringLiteral("project-template"),
    QStringLiteral("plugin"),
    QStringLiteral("widget"),
  };
}

/**
 * @brief Returns the grid's sort rank for @p type; an unknown type sorts last so a catalog that
 *        introduces a new type still renders in a stable order.
 */
int Misc::ExtensionCatalog::typeSortRank(const QString& type)
{
  static const QMap<QString, int> order = {
    {          QStringLiteral("plugin"), 0},
    {           QStringLiteral("theme"), 1},
    {    QStringLiteral("frame-parser"), 2},
    {QStringLiteral("project-template"), 3},
  };

  return order.value(type, 99);
}

/**
 * @brief Returns whether the given repository URL points at a local directory.
 */
bool Misc::ExtensionCatalog::isLocalRepo(const QString& url)
{
  return url.startsWith('/') || url.startsWith("file://");
}

/**
 * @brief Whether a repository may be added: a local folder the user picked, or https. Plain http
 *        is refused because the catalog decides what code lands in the extensions directory, and
 *        a cleartext fetch lets anyone on the path choose it (K5).
 */
bool Misc::ExtensionCatalog::isTrustedRepoUrl(const QString& url)
{
  const auto trimmed = url.trimmed();
  if (trimmed.isEmpty())
    return false;

  if (isLocalRepo(trimmed))
    return true;

  return QUrl(trimmed, QUrl::StrictMode)
           .scheme()
           .compare(QStringLiteral("https"), Qt::CaseInsensitive)
      == 0;
}

//--------------------------------------------------------------------------------------------------
// Catalog v2 file list
//--------------------------------------------------------------------------------------------------

/**
 * @brief Merges a platform override's digest table over the entry-wide one, so an override may
 *        publish digests for the files only it names without restating the rest.
 */
QVariantMap Misc::ExtensionCatalog::mergeDigests(const QVariantMap& base,
                                                 const QVariantMap& overrides)
{
  auto merged = base;
  for (auto it = overrides.cbegin(); it != overrides.cend(); ++it)
    merged.insert(it.key(), it.value());

  return merged;
}

/**
 * @brief Reads one file list element: the inline shape carries its own digest and size, the
 *        sidecar shape is a bare path whose digest comes from @p digests, and any other type
 *        fails. The sidecar shape leaves `files` a plain string list, which is what releases up
 *        to 4.1.0 read, so publishing digests does not break the installer those versions ship.
 */
static bool readCatalogFile(const QVariant& value,
                            const QVariantMap& digests,
                            Misc::ExtensionCatalog::CatalogFile& file)
{
  if (value.typeId() == QMetaType::QVariantMap) {
    const auto map = value.toMap();
    file.path      = map.value(QStringLiteral("path")).toString();
    file.sha256    = map.value(QStringLiteral("sha256")).toString().toLower();
    file.size      = map.value(QStringLiteral("size")).toLongLong();
    return true;
  }

  if (value.typeId() != QMetaType::QString)
    return false;

  file.path       = value.toString();
  const auto side = digests.value(file.path).toMap();
  file.sha256     = side.value(QStringLiteral("sha256")).toString().toLower();
  file.size       = side.value(QStringLiteral("size")).toLongLong();
  return true;
}

/**
 * @brief Reports @p reason to the optional out-parameter and yields the empty list that refuses
 *        the catalog entry, so a refusal stays one statement deep inside the parse loop.
 */
static QList<Misc::ExtensionCatalog::CatalogFile> refuseEntry(QString* rejectReason,
                                                              const QString& reason)
{
  if (rejectReason)
    *rejectReason = reason;

  return {};
}

/**
 * @brief Parses a catalog v2 file list in either published shape, inline or sidecar, so one
 *        repository can serve every Serial Studio version. A file resolving to no lowercase hex
 *        SHA-256 refuses the whole entry with a reason instead of installing unchecked (K3, K5);
 *        the entry's own info.json is exempt, and an entry of nothing else is refused as well.
 */
QList<Misc::ExtensionCatalog::CatalogFile> Misc::ExtensionCatalog::parseFileList(
  const QVariantList& files, const QVariantMap& digests, QString* rejectReason)
{
  static const QRegularExpression kDigest(QStringLiteral("^[0-9a-f]{64}$"));

  QList<CatalogFile> parsed;
  int verifiable = 0;
  for (const auto& value : files) {
    CatalogFile file;
    if (!readCatalogFile(value, digests, file))
      return refuseEntry(
        rejectReason, QStringLiteral("catalog entry: a file is neither a path nor a file object"));

    if (file.path.isEmpty())
      return refuseEntry(rejectReason, QStringLiteral("catalog entry: a file has no path"));

    if (file.sha256.isEmpty() && file.path == QLatin1String(kMetadataFile)) {
      file.selfMetadata = true;
      parsed.append(file);
      continue;
    }

    if (!kDigest.match(file.sha256).hasMatch())
      return refuseEntry(
        rejectReason,
        file.sha256.isEmpty()
          ? QStringLiteral("catalog v1 entry: file '%1' carries no sha256 digest").arg(file.path)
          : QStringLiteral("catalog entry: file '%1' has an invalid sha256 digest").arg(file.path));

    ++verifiable;
    parsed.append(file);
  }

  if (verifiable == 0)
    return refuseEntry(rejectReason,
                       QStringLiteral("catalog entry: no file carries a sha256 digest"));

  return parsed;
}

/**
 * @brief Whether a file list is installable at all: non-empty and parseable as catalog v2.
 */
bool Misc::ExtensionCatalog::hasVerifiableFiles(const QVariantList& files,
                                                const QVariantMap& digests)
{
  return !files.isEmpty() && !parseFileList(files, digests, nullptr).isEmpty();
}

/**
 * @brief Whether downloaded bytes are the file the catalog described. The size is checked first
 *        so an oversized body is rejected without hashing it.
 */
bool Misc::ExtensionCatalog::digestMatches(const QByteArray& payload, const CatalogFile& file)
{
  if (file.size > 0 && payload.size() != file.size)
    return false;

  const auto actual = QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex();
  return QString::fromLatin1(actual) == file.sha256;
}

/**
 * @brief Compares two version strings numerically: > 0 when remote is newer. String inequality
 *        used to offer "1.10.0 -> 1.9.0" as an update, which is a silent downgrade (K12).
 */
int Misc::ExtensionCatalog::compareVersions(const QString& remote, const QString& local)
{
  const auto lhs = QVersionNumber::fromString(remote.trimmed());
  const auto rhs = QVersionNumber::fromString(local.trimmed());
  if (lhs.isNull() || rhs.isNull())
    return remote == local ? 0 : 1;

  return QVersionNumber::compare(lhs, rhs);
}

//--------------------------------------------------------------------------------------------------
// Platform resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the platform key for the current OS and CPU architecture.
 */
QString Misc::ExtensionCatalog::currentPlatformKey()
{
#if defined(Q_OS_MACOS)
  const auto os = QStringLiteral("darwin");
#elif defined(Q_OS_WIN)
  const auto os = QStringLiteral("windows");
#else
  const auto os = QStringLiteral("linux");
#endif

  const auto arch = QSysInfo::currentCpuArchitecture();
  auto normalized = arch;
  if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64"))
    normalized = QStringLiteral("arm64");
  else if (arch == QStringLiteral("x86_64") || arch == QStringLiteral("amd64"))
    normalized = QStringLiteral("x86_64");

  return os + "/" + normalized;
}

/**
 * @brief Reports whether @p entry declares support for @p platformKey; an entry that declares no
 *        platforms at all is available everywhere.
 */
bool Misc::ExtensionCatalog::platformSupported(const QJsonObject& entry, const QString& platformKey)
{
  const auto platforms = entry.value("platforms").toObject();
  if (platforms.isEmpty())
    return true;

  const auto os = platformKey.left(platformKey.indexOf('/'));
  return platforms.contains(platformKey) || platforms.contains(os + "/*")
      || platforms.contains("*");
}

/**
 * @brief Picks the best matching platform override map for the given platform key.
 */
QVariantMap Misc::ExtensionCatalog::selectPlatformOverride(const QVariantMap& platforms,
                                                           const QString& platformKey)
{
  if (platforms.isEmpty())
    return {};

  if (platforms.contains(platformKey))
    return platforms.value(platformKey).toMap();

  const auto os       = platformKey.left(platformKey.indexOf('/'));
  const auto wildcard = os + QStringLiteral("/*");
  if (platforms.contains(wildcard))
    return platforms.value(wildcard).toMap();

  if (platforms.contains(QStringLiteral("*")))
    return platforms.value(QStringLiteral("*")).toMap();

  return {};
}

/**
 * @brief Folds the platform-specific overrides of @p meta into a flat metadata object.
 */
QJsonObject Misc::ExtensionCatalog::resolvePlatform(const QJsonObject& meta,
                                                    const QString& platformKey)
{
  auto result = meta;

  const auto platforms = meta.value("platforms").toObject();
  if (platforms.isEmpty())
    return result;

  const auto os = platformKey.left(platformKey.indexOf('/'));

  QJsonObject override;
  if (platforms.contains(platformKey))
    override = platforms.value(platformKey).toObject();
  else if (platforms.contains(os + "/*"))
    override = platforms.value(os + "/*").toObject();
  else if (platforms.contains("*"))
    override = platforms.value("*").toObject();

  if (override.isEmpty())
    return result;

  for (auto it = override.begin(); it != override.end(); ++it)
    result.insert(it.key(), it.value());

  return result;
}

//--------------------------------------------------------------------------------------------------
// Path safety
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports whether an identifier may be used as a single install-path component: anything
 *        carrying a separator or a parent reference is refused before it reaches the filesystem.
 */
bool Misc::ExtensionCatalog::isSafePathComponent(const QString& component)
{
  return !component.contains("..") && !component.contains('/') && !component.contains('\\');
}

/**
 * @brief Validates that a resolved file path stays within the expected base directory.
 */
bool Misc::ExtensionCatalog::isPathSafe(const QString& filePath, const QString& baseDir)
{
  const auto canonical = QFileInfo(filePath).absoluteFilePath();
  const auto base      = QFileInfo(baseDir).absoluteFilePath();
  return canonical.startsWith(base + "/") || canonical == base;
}

/**
 * @brief Resolves a relative file path against a repository base URL.
 */
QUrl Misc::ExtensionCatalog::resolveFileUrl(const QString& repoBaseUrl, const QString& relativePath)
{
  return QUrl(repoBaseUrl + relativePath);
}

//--------------------------------------------------------------------------------------------------
// Filtering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports whether a title/description pair matches @p search; an empty search matches
 *        everything, which is what makes an unfiltered catalog list every entry.
 */
bool Misc::ExtensionCatalog::matchesSearch(const QString& title,
                                           const QString& description,
                                           const QString& search)
{
  if (search.isEmpty())
    return true;

  return title.contains(search, Qt::CaseInsensitive)
      || description.contains(search, Qt::CaseInsensitive);
}

/**
 * @brief Returns true when @p entry passes the type, category and search filters.
 */
bool Misc::ExtensionCatalog::entryMatchesFilters(const QJsonObject& entry,
                                                 const CatalogFilter& filter)
{
  if (!filter.type.isEmpty() && filter.type != QStringLiteral("All"))
    if (entry.value("type").toString() != filter.type)
      return false;

  if (!filter.category.isEmpty() && filter.category != QStringLiteral("All")) {
    const auto category = entry.value("category").toString();
    if (!category.contains(filter.category, Qt::CaseInsensitive))
      return false;
  }

  if (filter.search.isEmpty())
    return true;

  const auto title  = entry.value("title").toString();
  const auto desc   = entry.value("description").toString();
  const auto author = entry.value("author").toString();

  return title.contains(filter.search, Qt::CaseInsensitive)
      || desc.contains(filter.search, Qt::CaseInsensitive)
      || author.contains(filter.search, Qt::CaseInsensitive);
}

/**
 * @brief Builds the QML-friendly variant map for a catalog entry, carrying the install state the
 *        caller resolved and the availability of the entry on @p platformKey.
 */
QVariantMap Misc::ExtensionCatalog::buildEntryMap(const QJsonObject& entry,
                                                  const EntryState& state,
                                                  const QString& platformKey)
{
  auto map = entry.toVariantMap();
  map.insert("installed", state.installed);
  map.insert("updateAvailable", state.updateAvailable);
  map.insert("installedVersion", state.installedVersion);
  map.insert("pluginRunning", state.pluginRunning);
  map.insert("platformAvailable", platformSupported(entry, platformKey));
  return map;
}
