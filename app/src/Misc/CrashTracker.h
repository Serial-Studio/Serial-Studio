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

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

namespace Misc {
/**
 * @brief Detects abnormal terminations via a QSettings "running" flag and counts streaks.
 */
class CrashTracker : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool previousRunCrashed
             READ previousRunCrashed
             CONSTANT)
  Q_PROPERTY(int consecutiveCrashes
             READ consecutiveCrashes
             CONSTANT)
  Q_PROPERTY(QString lastCheckpoint
             READ lastCheckpoint
             CONSTANT)
  Q_PROPERTY(QString lastCrashAt
             READ lastCrashAt
             CONSTANT)
  Q_PROPERTY(bool recoveryRecommended
             READ recoveryRecommended
             CONSTANT)
  // clang-format on

  /**
   * @brief Recovery action keys understood by applyRecovery().
   */

public:
  enum RecoveryAction {
    ResetGraphicsBackend = 1,
    SkipLastProject      = 2,
    ResetAllPreferences  = 3
  };
  Q_ENUM(RecoveryAction)

private:
  explicit CrashTracker();
  CrashTracker(CrashTracker&&)                 = delete;
  CrashTracker(const CrashTracker&)            = delete;
  CrashTracker& operator=(CrashTracker&&)      = delete;
  CrashTracker& operator=(const CrashTracker&) = delete;

public:
  [[nodiscard]] static CrashTracker& instance();

  [[nodiscard]] bool previousRunCrashed() const noexcept;
  [[nodiscard]] int consecutiveCrashes() const noexcept;
  [[nodiscard]] const QString& lastCheckpoint() const noexcept;
  [[nodiscard]] const QString& lastCrashAt() const noexcept;
  [[nodiscard]] bool recoveryRecommended() const noexcept;

public slots:
  void markStartup();
  void markCleanExit();
  void setCheckpoint(const QString& name);
  void applyRecovery(int action);
  void acknowledge();

private:
  static constexpr int kRecoveryThreshold      = 3;
  static constexpr const char* kKeyRunning     = "Crash/Running";
  static constexpr const char* kKeyCheckpoint  = "Crash/Checkpoint";
  static constexpr const char* kKeyConsecutive = "Crash/ConsecutiveCount";
  static constexpr const char* kKeyLastCrash   = "Crash/LastCrashTime";

  bool m_previousRunCrashed;
  int m_consecutiveCrashes;
  QString m_lastCheckpoint;
  QString m_lastCrashAt;
  QSettings m_settings;
};
}  // namespace Misc
