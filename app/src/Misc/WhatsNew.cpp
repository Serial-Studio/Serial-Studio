/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/WhatsNew.h"

#include "AppInfo.h"

//--------------------------------------------------------------------------------------------------
// Singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide WhatsNew instance.
 */
Misc::WhatsNew& Misc::WhatsNew::instance()
{
  static WhatsNew singleton;
  return singleton;
}

/**
 * @brief Constructs the tracker; all state is read lazily from QSettings.
 */
Misc::WhatsNew::WhatsNew() {}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the running application version string.
 */
QString Misc::WhatsNew::currentVersion() const
{
  return QStringLiteral(APP_VERSION);
}

/**
 * @brief True when the user has opted to see What's New on startup (the checkbox is the gate).
 */
bool Misc::WhatsNew::shouldShowWhatsNew() const
{
  return showWhatsNewOnStartup();
}

/**
 * @brief Returns whether the What's New dialog should appear automatically on startup.
 */
bool Misc::WhatsNew::showWhatsNewOnStartup() const
{
  return m_settings.value(kKeyStartup, true).toBool();
}

/**
 * @brief Returns whether the Tips dialog should appear automatically on startup.
 */
bool Misc::WhatsNew::showTipsOnStartup() const
{
  return m_settings.value(kKeyTips, true).toBool();
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stamps the current version and re-enables startup auto-show on an upgrade.
 */
void Misc::WhatsNew::syncVersion()
{
  const QString lastSeen = m_settings.value(kKeyLastSeen, QString()).toString();
  const bool upgraded =
    !lastSeen.isEmpty() && compareVersions(QStringLiteral(APP_VERSION), lastSeen) > 0;

  m_settings.setValue(kKeyLastSeen, QStringLiteral(APP_VERSION));
  if (upgraded)
    setShowWhatsNewOnStartup(true);

  m_settings.sync();
}

/**
 * @brief Persists the Tips-on-startup opt-out flag and notifies bound QML.
 */
void Misc::WhatsNew::setShowTipsOnStartup(const bool show)
{
  if (show == showTipsOnStartup())
    return;

  m_settings.setValue(kKeyTips, show);
  m_settings.sync();
  Q_EMIT showTipsOnStartupChanged();
}

/**
 * @brief Persists the What's-New-on-startup opt-out flag and notifies bound QML.
 */
void Misc::WhatsNew::setShowWhatsNewOnStartup(const bool show)
{
  if (show == showWhatsNewOnStartup())
    return;

  m_settings.setValue(kKeyStartup, show);
  m_settings.sync();
  Q_EMIT showWhatsNewOnStartupChanged();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compares two dotted version strings numerically (returns <0, 0, or >0).
 */
int Misc::WhatsNew::compareVersions(const QString& a, const QString& b)
{
  const QStringList partsA = a.split(QLatin1Char('.'), Qt::SkipEmptyParts);
  const QStringList partsB = b.split(QLatin1Char('.'), Qt::SkipEmptyParts);

  const qsizetype count = qMax(partsA.size(), partsB.size());
  Q_ASSERT(count <= 8);
  for (qsizetype i = 0; i < count; ++i) {
    const int va = i < partsA.size() ? partsA.at(i).toInt() : 0;
    const int vb = i < partsB.size() ? partsB.at(i).toInt() : 0;
    if (va != vb)
      return va < vb ? -1 : 1;
  }

  return 0;
}
