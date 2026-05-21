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

#include "Misc/CrashTracker.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
// Singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide CrashTracker instance.
 */
Misc::CrashTracker& Misc::CrashTracker::instance()
{
  static CrashTracker singleton;
  return singleton;
}

/**
 * @brief Defers any QSettings work until markStartup() is invoked from main().
 */
Misc::CrashTracker::CrashTracker() : m_previousRunCrashed(false), m_consecutiveCrashes(0) {}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the previous launch did not call markCleanExit().
 */
bool Misc::CrashTracker::previousRunCrashed() const noexcept
{
  return m_previousRunCrashed;
}

/**
 * @brief Returns the number of consecutive abnormal terminations.
 */
int Misc::CrashTracker::consecutiveCrashes() const noexcept
{
  return m_consecutiveCrashes;
}

/**
 * @brief Returns the last checkpoint string recorded before the previous crash.
 */
const QString& Misc::CrashTracker::lastCheckpoint() const noexcept
{
  return m_lastCheckpoint;
}

/**
 * @brief Returns the ISO-8601 timestamp recorded when the previous crash was detected.
 */
const QString& Misc::CrashTracker::lastCrashAt() const noexcept
{
  return m_lastCrashAt;
}

/**
 * @brief Returns true once consecutive crashes reach the recovery prompt threshold.
 */
bool Misc::CrashTracker::recoveryRecommended() const noexcept
{
  return m_consecutiveCrashes >= kRecoveryThreshold;
}

//--------------------------------------------------------------------------------------------------
// Lifecycle hooks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the prior-run flag, updates the streak counter, and arms the new run.
 */
void Misc::CrashTracker::markStartup()
{
  const bool wasRunning = m_settings.value(kKeyRunning, false).toBool();
  if (wasRunning) {
    m_previousRunCrashed = true;
    m_consecutiveCrashes = m_settings.value(kKeyConsecutive, 0).toInt() + 1;
    m_lastCheckpoint     = m_settings.value(kKeyCheckpoint, QString()).toString();
    m_lastCrashAt        = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    m_settings.setValue(kKeyConsecutive, m_consecutiveCrashes);
    m_settings.setValue(kKeyLastCrash, m_lastCrashAt);
  } else {
    m_previousRunCrashed = false;
    m_consecutiveCrashes = 0;
    m_settings.setValue(kKeyConsecutive, 0);
  }

  m_settings.setValue(kKeyRunning, true);
  m_settings.setValue(kKeyCheckpoint, QStringLiteral("startup"));
  m_settings.sync();
}

/**
 * @brief Clears the running flag and resets the consecutive-crash counter.
 */
void Misc::CrashTracker::markCleanExit()
{
  m_settings.setValue(kKeyRunning, false);
  m_settings.setValue(kKeyConsecutive, 0);
  m_settings.remove(kKeyCheckpoint);
  m_settings.sync();
}

//--------------------------------------------------------------------------------------------------
// Checkpoint API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the current boot/runtime stage so a later crash can be attributed.
 */
void Misc::CrashTracker::setCheckpoint(const QString& name)
{
  m_settings.setValue(kKeyCheckpoint, name);
  m_settings.sync();
}

//--------------------------------------------------------------------------------------------------
// User-facing recovery
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dismisses the recovery prompt without taking action and clears the streak.
 */
void Misc::CrashTracker::acknowledge()
{
  m_settings.setValue(kKeyConsecutive, 0);
  m_settings.sync();
  m_consecutiveCrashes = 0;
}

/**
 * @brief Applies one of the recovery actions; the user is expected to restart afterwards.
 */
void Misc::CrashTracker::applyRecovery(int action)
{
  switch (action) {
    case ResetGraphicsBackend:
      m_settings.setValue(QStringLiteral("App/GraphicsBackend"), 0);
      m_settings.remove(QStringLiteral("App/GraphicsBackendPending"));
      break;
    case SkipLastProject:
      m_settings.remove(QStringLiteral("project_file_path"));
      break;
    case ResetAllPreferences: {
      QVariantMap preserved;
      const QStringList groups{QStringLiteral("licensing"), QStringLiteral("trial")};
      for (const QString& group : groups) {
        m_settings.beginGroup(group);
        const QStringList keys = m_settings.allKeys();
        for (const QString& key : keys)
          preserved.insert(group + QLatin1Char('/') + key, m_settings.value(key));

        m_settings.endGroup();
      }

      m_settings.clear();
      for (auto it = preserved.constBegin(); it != preserved.constEnd(); ++it)
        m_settings.setValue(it.key(), it.value());

      break;
    }
    default:
      break;
  }

  acknowledge();
}
