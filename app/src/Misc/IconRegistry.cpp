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

#include "Misc/IconRegistry.h"

#include <QDirIterator>
#include <QtAlgorithms>
#include <QtLogging>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the icon catalog from the :/icons resource tree; entries that
 *        do not match <category>/<tier>/<name>.svg (e.g. the exempt buttons
 *        set and compat aliases) are ignored.
 */
Misc::IconRegistry::IconRegistry()
{
  QDirIterator it(QStringLiteral(":/icons"), QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const auto path  = it.next();
    const auto parts = path.mid(8).split(QChar('/'), Qt::SkipEmptyParts);
    if (parts.size() != 3 || !parts[2].endsWith(QStringLiteral(".svg")))
      continue;

    bool numeric  = false;
    const int px  = parts[1].toInt(&numeric);
    const auto id = parts[2].chopped(4);
    if (!numeric || px <= 0 || id.isEmpty())
      continue;

    auto& tiers = m_catalog[parts[0]][id];
    tiers.append(px);
    std::sort(tiers.begin(), tiers.end());
  }

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
// Icon resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the qrc URL (QML Image consumable) of @p category / @p name
 *        at the artwork tier nearest at-or-above @p px.
 */
QString Misc::IconRegistry::icon(const QString& category, const QString& name, int px) const
{
  return QStringLiteral("qrc:") + resolve(category, name, px);
}

/**
 * @brief Resolves a combined "category/name" id as produced by the taskbar
 *        model's iconId role; malformed ids warn and serve the placeholder.
 */
QString Misc::IconRegistry::iconById(const QString& id, int px) const
{
  const auto slash = id.indexOf(QChar('/'));
  if (slash <= 0 || slash == id.length() - 1)
    return QStringLiteral("qrc:") + missingIcon(QStringLiteral("(malformed)"), id);

  return icon(id.left(slash), id.mid(slash + 1), px);
}

/**
 * @brief Returns the resource path (":/...", QPixmap/QIcon consumable) of
 *        @p category / @p name at the artwork tier nearest at-or-above @p px.
 */
QString Misc::IconRegistry::iconPath(const QString& category, const QString& name, int px) const
{
  return QStringLiteral(":") + resolve(category, name, px);
}

/**
 * @brief Scheme-less tier resolution: nearest tier at-or-above @p px, largest
 *        tier when none reaches @p px; unknown keys warn once and resolve to
 *        the missing-icon placeholder (spec 0028 R2/R4).
 */
QString Misc::IconRegistry::resolve(const QString& category, const QString& name, int px) const
{
  SS_ASSERT(!category.isEmpty(), return missingIcon(category, name));
  SS_ASSERT(!name.isEmpty(), return missingIcon(category, name));

  const auto cat = m_catalog.constFind(category);
  if (cat == m_catalog.constEnd())
    return missingIcon(category, name);

  const auto entry = cat->constFind(name);
  if (entry == cat->constEnd())
    return missingIcon(category, name);

  const auto url = QStringLiteral("/icons/%1/%2/%3.svg");
  for (const int tier : *entry)
    if (tier >= px)
      return url.arg(category, QString::number(tier), name);

  return url.arg(category, QString::number(entry->constLast()), name);
}

/**
 * @brief Warns once per unknown (category, name) pair and returns the
 *        placeholder resource path (spec 0028 R4: loud, never blank).
 */
QString Misc::IconRegistry::missingIcon(const QString& category, const QString& name) const
{
  const auto key = QStringLiteral("%1/%2").arg(category, name);
  if (!m_warned.contains(key)) {
    m_warned.insert(key);
    qWarning() << "IconRegistry: unknown icon" << key << "- serving placeholder";
  }

  return QStringLiteral("/icons/system/16/missing.svg");
}
