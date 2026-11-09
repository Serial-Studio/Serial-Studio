/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "Core/IconRegistry.h"

#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QtAlgorithms>
#include <QtLogging>

#include "Core/SSAssert.h"

static const QString kHouseRoot = QStringLiteral(":/icons");
static const QString kPacksRoot = QStringLiteral(":/iconpacks");
static const QString kMissing   = QStringLiteral("/icons/system/16/missing.svg");

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the catalog from the bundled resource trees.
 */
Misc::IconRegistry::IconRegistry() : IconRegistry(kHouseRoot, kPacksRoot) {}

/**
 * @brief Builds the catalog from @p houseRoot (<category>/<tier>/<name>.svg, entries that do
 *        not match are ignored) and every pack folder under @p packsRoot; the test tier points
 *        both at a temporary directory.
 */
Misc::IconRegistry::IconRegistry(const QString& houseRoot, const QString& packsRoot)
  : m_mono(false), m_textTint(QStringLiteral("000000")), m_stripTint(QStringLiteral("ffffff"))
{
  scanTree(houseRoot, QString());
  scanPacks(packsRoot);

  SS_ASSERT_LOG(!m_catalog.isEmpty());
  SS_ASSERT_LOG(m_catalog.contains(QStringLiteral("system")));
}

/**
 * @brief Returns the global IconRegistry instance.
 */
Misc::IconRegistry& Misc::IconRegistry::instance()
{
  static IconRegistry s;
  return s;
}

//--------------------------------------------------------------------------------------------------
// Catalog construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers every <category>/<tier>/<name>.svg or .png under @p root, into the house
 *        catalog when @p pack is empty and into that pack's catalog otherwise. An @2x file is not
 *        an icon of its own: Qt finds it beside the @1x file. A pack file in a fixed category is
 *        skipped with one warning: the chrome glyphs never follow a pack.
 */
void Misc::IconRegistry::scanTree(const QString& root, const QString& pack)
{
  const int prefix = root.length() + 1;
  QDirIterator it(root, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const auto path  = it.next();
    const auto parts = path.mid(prefix).split(QChar('/'), Qt::SkipEmptyParts);
    if (parts.size() != 3)
      continue;

    const bool raster = parts[2].endsWith(QStringLiteral(".png"));
    if (!raster && !parts[2].endsWith(QStringLiteral(".svg")))
      continue;

    bool numeric  = false;
    const int px  = parts[1].toInt(&numeric);
    const auto id = parts[2].chopped(4);
    if (!numeric || px <= 0 || id.isEmpty() || id.endsWith(QStringLiteral("@2x")))
      continue;

    if (!pack.isEmpty() && fixedCategories().contains(parts[0])) {
      qWarning() << "IconRegistry: pack" << pack << "ships a fixed-category icon" << path
                 << "- ignored";
      continue;
    }

    auto& tiers = pack.isEmpty() ? m_catalog[parts[0]][id] : m_packCatalog[pack][parts[0]][id];
    tiers.append({px, raster});
    std::sort(
      tiers.begin(), tiers.end(), [](const IconTier& a, const IconTier& b) { return a.px < b.px; });
  }
}

/**
 * @brief Reads every <pack>/pack.json under @p packsRoot and scans the pack's tree; a folder
 *        without a readable pack.json is not a pack.
 */
void Misc::IconRegistry::scanPacks(const QString& packsRoot)
{
  QDirIterator dirs(packsRoot, QDir::Dirs | QDir::NoDotAndDotDot);
  while (dirs.hasNext()) {
    const auto folder = dirs.next();
    QFile file(folder + QStringLiteral("/pack.json"));
    if (!file.open(QFile::ReadOnly))
      continue;

    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto obj = doc.object();
    const auto id  = obj.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || id != dirs.fileName()) {
      qWarning() << "IconRegistry: pack.json in" << folder << "names id" << id << "- skipped";
      continue;
    }

    IconPackInfo info;
    info.id     = id;
    info.title  = obj.value(QStringLiteral("title")).toString(id);
    info.mono   = obj.value(QStringLiteral("mono")).toBool(false);
    info.hidden = obj.value(QStringLiteral("hidden")).toBool(false);

    const auto translations = obj.value(QStringLiteral("translations")).toObject();
    for (auto it = translations.constBegin(); it != translations.constEnd(); ++it)
      info.translations.insert(it.key(), it.value().toString());

    m_packs.append(info);
    scanTree(folder, id);
  }

  std::sort(m_packs.begin(), m_packs.end(), [](const IconPackInfo& a, const IconPackInfo& b) {
    return a.id < b.id;
  });
}

//--------------------------------------------------------------------------------------------------
// Pack selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the active pack id; empty means the house set.
 */
const QString& Misc::IconRegistry::pack() const noexcept
{
  return m_pack;
}

/**
 * @brief Returns the bundled packs, sorted by id.
 */
const QList<Misc::IconPackInfo>& Misc::IconRegistry::packs() const noexcept
{
  return m_packs;
}

/**
 * @brief Returns the packs a user may choose, sorted by id; a hidden pack exists only to serve
 *        monoIconById() (spec 0116 R9).
 */
QList<Misc::IconPackInfo> Misc::IconRegistry::visiblePacks() const
{
  QList<IconPackInfo> visible;
  for (const auto& info : m_packs)
    if (!info.hidden)
      visible.append(info);

  return visible;
}

/**
 * @brief Returns @p id when it names a pack a user may choose and an empty string otherwise, with
 *        no warning: a theme or saved setting may name a pack this build lacks (spec 0116 R11).
 */
QString Misc::IconRegistry::selectablePack(const QString& id) const
{
  for (const auto& info : m_packs)
    if (info.id == id && !info.hidden)
      return id;

  return {};
}

/**
 * @brief Returns whether the active pack is monochrome and therefore tinted.
 */
bool Misc::IconRegistry::isMono() const noexcept
{
  return m_mono;
}

/**
 * @brief Returns whether a pack with @p id is bundled (declared by a pack.json, artwork or not).
 */
bool Misc::IconRegistry::hasPack(const QString& id) const
{
  for (const auto& info : m_packs)
    if (info.id == id)
      return true;

  return false;
}

/**
 * @brief The categories no pack may restyle: the chrome glyphs and the placeholder (the exempt
 *        buttons folder never reaches the registry at all).
 */
const QStringList& Misc::IconRegistry::fixedCategories()
{
  static const QStringList kFixed = {QStringLiteral("window"), QStringLiteral("system")};
  return kFixed;
}

/**
 * @brief The bundled mono pack the monochrome-by-design surfaces draw from (the Fine Line set).
 */
const QString& Misc::IconRegistry::monoPackId()
{
  static const QString kMono = QStringLiteral("fine-line");
  return kMono;
}

/**
 * @brief Selects the pack with @p id (empty for the house set); an unknown id falls back to the
 *        house set with a warning so a theme naming a pack this build lacks stays usable.
 */
void Misc::IconRegistry::setPack(const QString& id)
{
  QString next = id;
  if (!next.isEmpty() && !hasPack(next)) {
    qWarning() << "IconRegistry: no bundled icon pack" << next << "- using the house set";
    next.clear();
  }

  if (next == m_pack)
    return;

  m_pack = next;
  m_mono = false;
  for (const auto& info : m_packs)
    if (info.id == m_pack)
      m_mono = info.mono;

  Q_EMIT packChanged();
  Q_EMIT revisionChanged();
}

/**
 * @brief Sets the two tints a mono pack is drawn in, as RRGGBB without the hash: @p text for
 *        pane and dialog surfaces, @p strip for the dark toolbar and taskbar gradients. A change
 *        announces a revision, since every mono URL carries the tint.
 */
void Misc::IconRegistry::setTints(const QString& text, const QString& strip)
{
  static const QRegularExpression kHex(QStringLiteral("^[0-9a-f]{6}$"));
  const auto clean = [](QString hex) {
    hex.remove(QChar('#'));
    hex = hex.left(6).toLower();
    return kHex.match(hex).hasMatch() ? hex : QStringLiteral("000000");
  };

  const auto nextText  = clean(text);
  const auto nextStrip = clean(strip);
  if (nextText == m_textTint && nextStrip == m_stripTint)
    return;

  m_textTint  = nextText;
  m_stripTint = nextStrip;
  Q_EMIT revisionChanged();
}

//--------------------------------------------------------------------------------------------------
// Icon resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the URL (QML Image consumable) of @p category / @p name at the artwork tier
 *        nearest at-or-above @p px, in the text tone.
 */
QString Misc::IconRegistry::icon(const QString& category, const QString& name, int px) const
{
  return iconOn(Tone::Text, category, name, px);
}

/**
 * @brief Resolves a combined "category/name" id as produced by the taskbar model's iconId role;
 *        malformed ids warn and serve the placeholder.
 */
QString Misc::IconRegistry::iconById(const QString& id, int px) const
{
  return iconByIdOn(Tone::Text, id, px);
}

/**
 * @brief Returns the URL of @p category / @p name at the tier nearest at-or-above @p px, tinted
 *        for @p tone when the active pack is monochrome.
 */
QString Misc::IconRegistry::iconOn(const Tone tone,
                                   const QString& category,
                                   const QString& name,
                                   const int px) const
{
  return url(locate(category, name, px), tone);
}

/**
 * @brief Tone-aware form of iconById.
 */
QString Misc::IconRegistry::iconByIdOn(const Tone tone, const QString& id, const int px) const
{
  const auto slash = id.indexOf(QChar('/'));
  if (slash <= 0 || slash == id.length() - 1)
    return QStringLiteral("qrc:") + missingIcon(QStringLiteral("(malformed)"), id);

  return iconOn(tone, id.left(slash), id.mid(slash + 1), px);
}

/**
 * @brief Resolves a combined id against the fixed mono pack in the text tint whatever pack is
 *        active, for the surfaces that are monochrome by design (the dashboard widget toolbars,
 *        spec 0115 R23); a glyph that pack lacks, or a build without it, serves the house file.
 */
QString Misc::IconRegistry::monoIconById(const QString& id, const int px) const
{
  const auto slash = id.indexOf(QChar('/'));
  if (slash <= 0 || slash == id.length() - 1)
    return QStringLiteral("qrc:") + missingIcon(QStringLiteral("(malformed)"), id);

  const auto category = id.left(slash);
  const auto name     = id.mid(slash + 1);
  const auto path     = packPathIn(monoPackId(), category, name, px);
  if (path.isEmpty())
    return QStringLiteral("qrc:") + locateHouse(category, name, px).path;

  return QStringLiteral("image://iconpack/%1/%2%3?tint=%4")
    .arg(monoPackId(), category, path, m_textTint);
}

/**
 * @brief Returns the resource path (":/...", QPixmap/QIcon consumable) of @p category / @p name
 *        at the tier nearest at-or-above @p px; a mono pack's file is served untinted here.
 */
QString Misc::IconRegistry::iconPath(const QString& category, const QString& name, int px) const
{
  return QStringLiteral(":") + locate(category, name, px).path;
}

/**
 * @brief Returns the active pack's "/<tier>/<name>.svg" for @p category / @p name, or an empty
 *        string when no pack is active, the category is fixed, or the pack lacks the icon.
 */
QString Misc::IconRegistry::packPath(const QString& category, const QString& name, int px) const
{
  if (m_pack.isEmpty())
    return {};

  return packPathIn(m_pack, category, name, px);
}

/**
 * @brief Returns @p pack's "/<tier>/<name>.svg" for @p category / @p name, or an empty string
 *        when the category is fixed or the pack lacks the icon.
 */
QString Misc::IconRegistry::packPathIn(const QString& pack,
                                       const QString& category,
                                       const QString& name,
                                       int px) const
{
  if (fixedCategories().contains(category))
    return {};

  const auto entry = m_packCatalog.constFind(pack);
  if (entry == m_packCatalog.constEnd())
    return {};

  const auto cat = entry->constFind(category);
  if (cat == entry->constEnd())
    return {};

  return tierPath(*cat, name, px);
}

/**
 * @brief Finds the artwork for @p category / @p name: the active pack first, unless the
 *        category is fixed, then the house tree; unknown keys warn once and resolve to the
 *        missing-icon placeholder (spec 0028 R2/R4, spec 0115 R4).
 */
Misc::IconRegistry::Located Misc::IconRegistry::locate(const QString& category,
                                                       const QString& name,
                                                       const int px) const
{
  SS_ASSERT(!category.isEmpty(), return notFound(category, name));
  SS_ASSERT(!name.isEmpty(), return notFound(category, name));

  const auto fromPack = packPath(category, name, px);
  if (!fromPack.isEmpty())
    return {QStringLiteral("/iconpacks/%1/%2%3").arg(m_pack, category, fromPack), true};

  return locateHouse(category, name, px);
}

/**
 * @brief The house tree alone: the artwork for @p category / @p name at the tier nearest
 *        at-or-above @p px, else the placeholder.
 */
Misc::IconRegistry::Located Misc::IconRegistry::locateHouse(const QString& category,
                                                            const QString& name,
                                                            const int px) const
{
  const auto cat = m_catalog.constFind(category);
  if (cat == m_catalog.constEnd())
    return notFound(category, name);

  const auto path = tierPath(*cat, name, px);
  if (path.isEmpty())
    return notFound(category, name);

  return {QStringLiteral("/icons/%1%2").arg(category, path), false};
}

/**
 * @brief The placeholder as a located house file.
 */
Misc::IconRegistry::Located Misc::IconRegistry::notFound(const QString& category,
                                                         const QString& name) const
{
  return {missingIcon(category, name), false};
}

/**
 * @brief Formats a located file as a URL: a qrc path for house and colour-pack artwork, the
 *        iconpack image provider with the tone's tint for a mono pack.
 */
QString Misc::IconRegistry::url(const Located& hit, const Tone tone) const
{
  if (!hit.fromPack || !isMono())
    return QStringLiteral("qrc:") + hit.path;

  const auto& tint = tone == Tone::Strip ? m_stripTint : m_textTint;
  return QStringLiteral("image://iconpack") + hit.path.mid(QStringLiteral("/iconpacks").length())
       + QStringLiteral("?tint=") + tint;
}

/**
 * @brief Returns "/<tier>/<name>.<svg or png>" for the tier of @p name nearest at-or-above @p px,
 *        the largest tier when none reaches it, or an empty string when @p name is unknown.
 */
QString Misc::IconRegistry::tierPath(const QHash<QString, QList<IconTier>>& names,
                                     const QString& name,
                                     const int px)
{
  const auto entry = names.constFind(name);
  if (entry == names.constEnd() || entry->isEmpty())
    return {};

  const auto form = [&name](const IconTier& tier) {
    const auto suffix = tier.raster ? QStringLiteral("png") : QStringLiteral("svg");
    return QStringLiteral("/%1/%2.%3").arg(QString::number(tier.px), name, suffix);
  };

  for (const auto& tier : *entry)
    if (tier.px >= px)
      return form(tier);

  return form(entry->constLast());
}

/**
 * @brief Warns once per unknown (category, name) pair and returns the placeholder resource path
 *        (spec 0028 R4: loud, never blank).
 */
QString Misc::IconRegistry::missingIcon(const QString& category, const QString& name) const
{
  const auto key = QStringLiteral("%1/%2").arg(category, name);
  if (!m_warned.contains(key)) {
    m_warned.insert(key);
    qWarning() << "IconRegistry: unknown icon" << key << "- serving placeholder";
  }

  return kMissing;
}
