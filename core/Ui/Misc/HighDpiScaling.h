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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QVariantList>

namespace Misc {
/**
 * @brief Manages the high-DPI scaling policy selection (rounding policy, disable, or a
 *        user-defined percentage), applied via Qt/env before QApplication exists.
 */
class HighDpiScaling : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int currentMode
             READ currentMode
             WRITE setCurrentMode
             NOTIFY currentModeChanged)
  Q_PROPERTY(int customPercent
             READ customPercent
             WRITE setCustomPercent
             NOTIFY customPercentChanged)
  Q_PROPERTY(bool customSelected
             READ customSelected
             NOTIFY currentModeChanged)
  Q_PROPERTY(QVariantList availableModes
             READ availableModes
             CONSTANT)
  Q_PROPERTY(bool configurable
             READ configurable
             CONSTANT)
  Q_PROPERTY(int minimumPercent
             READ minimumPercent
             CONSTANT)
  Q_PROPERTY(int maximumPercent
             READ maximumPercent
             CONSTANT)
  // clang-format on

signals:
  void currentModeChanged();
  void customPercentChanged();

public:
  /**
   * @brief Persisted scaling strategy; mapped to a Qt rounding policy or an env override.
   */
  enum Mode {
    SystemDefault = 0,
    Fractional    = 1,
    RoundNearest  = 2,
    RoundUp       = 3,
    RoundDown     = 4,
    Disabled      = 5,
    Custom        = 6
  };
  Q_ENUM(Mode)

private:
  explicit HighDpiScaling();
  HighDpiScaling(HighDpiScaling&&)                 = delete;
  HighDpiScaling(const HighDpiScaling&)            = delete;
  HighDpiScaling& operator=(HighDpiScaling&&)      = delete;
  HighDpiScaling& operator=(const HighDpiScaling&) = delete;

public:
  [[nodiscard]] static HighDpiScaling& instance();

  [[nodiscard]] int currentMode() const noexcept;
  [[nodiscard]] int customPercent() const noexcept;
  [[nodiscard]] bool customSelected() const noexcept;
  [[nodiscard]] bool configurable() const noexcept;
  [[nodiscard]] int minimumPercent() const noexcept;
  [[nodiscard]] int maximumPercent() const noexcept;
  [[nodiscard]] const QVariantList& availableModes() const noexcept;

  static void applyConfiguredPolicy();

public slots:
  void setCurrentMode(int mode);
  void setCustomPercent(int percent);
  void promptRestartAndQuit();

private:
  static int readPersistedMode();
  static int readPersistedPercent();
  static bool isModeAvailable(int mode) noexcept;
  static int clampPercent(int percent) noexcept;
  static const char* modeKey() noexcept;
  static const char* percentKey() noexcept;
  void rebuildAvailableModes();

private:
  static constexpr int kMinPercent     = 50;
  static constexpr int kMaxPercent     = 400;
  static constexpr int kDefaultPercent = 100;

private:
  int m_currentMode;
  int m_customPercent;
  QSettings m_settings;
  QVariantList m_availableModes;
};
}  // namespace Misc
