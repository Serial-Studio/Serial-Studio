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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>

namespace UI {

/**
 * @brief Persistent user preferences for the dashboard taskbar.
 *
 * Owns the canonical list of pinned-button IDs, their display order, plus
 * autohide / search-field / always-show-buttons toggles. Lives independently
 * of any Taskbar QQuickItem instance so the Settings dialog can edit prefs
 * even when no dashboard is loaded, and so all live taskbars stay in sync
 * via the singleton's notify signals.
 */
class TaskbarSettings : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QStringList pinnedButtons
             READ pinnedButtons
             WRITE setPinnedButtons
             NOTIFY pinnedButtonsChanged)
  Q_PROPERTY(QStringList availableButtons
             READ availableButtons
             CONSTANT)
  Q_PROPERTY(bool showTaskbarButtons
             READ showTaskbarButtons
             WRITE setShowTaskbarButtons
             NOTIFY showTaskbarButtonsChanged)
  Q_PROPERTY(bool searchEnabled
             READ searchEnabled
             WRITE setSearchEnabled
             NOTIFY searchEnabledChanged)
  Q_PROPERTY(bool autohide
             READ autohide
             WRITE setAutohide
             NOTIFY autohideChanged)
  Q_PROPERTY(int autohideDelayMs
             READ autohideDelayMs
             WRITE setAutohideDelayMs
             NOTIFY autohideDelayMsChanged)
  Q_PROPERTY(bool taskbarHidden
             READ taskbarHidden
             WRITE setTaskbarHidden
             NOTIFY taskbarHiddenChanged)
  // clang-format on

signals:
  void pinnedButtonsChanged();
  void showTaskbarButtonsChanged();
  void searchEnabledChanged();
  void autohideChanged();
  void autohideDelayMsChanged();
  void taskbarHiddenChanged();

private:
  explicit TaskbarSettings();
  TaskbarSettings(TaskbarSettings&&)                 = delete;
  TaskbarSettings(const TaskbarSettings&)            = delete;
  TaskbarSettings& operator=(TaskbarSettings&&)      = delete;
  TaskbarSettings& operator=(const TaskbarSettings&) = delete;

public:
  [[nodiscard]] static TaskbarSettings& instance();

  [[nodiscard]] QStringList pinnedButtons() const;
  [[nodiscard]] QStringList availableButtons() const;
  [[nodiscard]] bool showTaskbarButtons() const noexcept;
  [[nodiscard]] bool searchEnabled() const noexcept;
  [[nodiscard]] bool autohide() const noexcept;
  [[nodiscard]] int autohideDelayMs() const noexcept;
  [[nodiscard]] bool taskbarHidden() const noexcept;

  void setSettingsPersistent(bool persistent);

  Q_INVOKABLE [[nodiscard]] bool isButtonPinned(const QString& id) const;

public slots:
  void movePinnedButton(const QString& id, int targetIndex);
  void setButtonPinned(const QString& id, bool pinned);
  void resetToDefaults();
  void setPinnedButtons(const QStringList& ids);
  void setShowTaskbarButtons(bool enabled);
  void setSearchEnabled(bool enabled);
  void setAutohide(bool enabled);
  void setAutohideDelayMs(int delay);
  void setTaskbarHidden(bool hidden);

private:
  [[nodiscard]] static QStringList defaultPinnedButtons();
  [[nodiscard]] static QStringList canonicalAvailableButtons();
  [[nodiscard]] QStringList sanitizePinned(const QStringList& ids) const;

  QSettings m_settings;
  QStringList m_pinnedButtons;
  bool m_showTaskbarButtons;
  bool m_searchEnabled;
  bool m_autohide;
  bool m_taskbarHidden;
  bool m_settingsPersistent;
  int m_autohideDelayMs;
};

}  // namespace UI
