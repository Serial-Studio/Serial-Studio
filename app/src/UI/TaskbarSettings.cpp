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

#include "UI/TaskbarSettings.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kDefaultAutohideDelayMs = 1500;
static constexpr int kMinAutohideDelayMs     = 200;
static constexpr int kMaxAutohideDelayMs     = 10000;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads persisted preferences and migrates the legacy show-taskbar key.
 */
UI::TaskbarSettings::TaskbarSettings()
  : m_showTaskbarButtons(false)
  , m_searchEnabled(true)
  , m_autohide(false)
  , m_taskbarHidden(false)
  , m_settingsPersistent(true)
  , m_autohideDelayMs(kDefaultAutohideDelayMs)
{
  // Pinned buttons -- sanitize against the canonical available list
  if (m_settings.contains(QStringLiteral("Taskbar/PinnedButtons")))
    m_pinnedButtons =
      sanitizePinned(m_settings.value(QStringLiteral("Taskbar/PinnedButtons")).toStringList());
  else
    m_pinnedButtons = defaultPinnedButtons();

  // Always-show toggle -- migrate legacy Dashboard/ShowTaskbarButtons on first run
  if (m_settings.contains(QStringLiteral("Taskbar/ShowTaskbarButtons")))
    m_showTaskbarButtons =
      m_settings.value(QStringLiteral("Taskbar/ShowTaskbarButtons"), false).toBool();
  else if (m_settings.contains(QStringLiteral("Dashboard/ShowTaskbarButtons"))) {
    m_showTaskbarButtons =
      m_settings.value(QStringLiteral("Dashboard/ShowTaskbarButtons"), false).toBool();
    m_settings.setValue(QStringLiteral("Taskbar/ShowTaskbarButtons"), m_showTaskbarButtons);
    m_settings.remove(QStringLiteral("Dashboard/ShowTaskbarButtons"));
  }

  m_searchEnabled   = m_settings.value(QStringLiteral("Taskbar/SearchEnabled"), true).toBool();
  m_autohide        = m_settings.value(QStringLiteral("Taskbar/Autohide"), false).toBool();
  m_autohideDelayMs = qBound(
    kMinAutohideDelayMs,
    m_settings.value(QStringLiteral("Taskbar/AutohideDelayMs"), kDefaultAutohideDelayMs).toInt(),
    kMaxAutohideDelayMs);
}

/**
 * @brief Returns the singleton instance.
 */
UI::TaskbarSettings& UI::TaskbarSettings::instance()
{
  static TaskbarSettings instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the ordered list of currently-enabled pin IDs.
 */
QStringList UI::TaskbarSettings::pinnedButtons() const
{
  return m_pinnedButtons;
}

/**
 * @brief Returns the canonical list of all known pin IDs (constant).
 */
QStringList UI::TaskbarSettings::availableButtons() const
{
  return canonicalAvailableButtons();
}

/**
 * @brief Returns whether dashboard widget buttons are pinned regardless of state.
 */
bool UI::TaskbarSettings::showTaskbarButtons() const noexcept
{
  return m_showTaskbarButtons;
}

/**
 * @brief Returns whether the taskbar search field is shown.
 */
bool UI::TaskbarSettings::searchEnabled() const noexcept
{
  return m_searchEnabled;
}

/**
 * @brief Returns whether the taskbar should auto-hide when idle.
 */
bool UI::TaskbarSettings::autohide() const noexcept
{
  return m_autohide;
}

/**
 * @brief Returns the inactivity delay (ms) before the taskbar auto-hides.
 */
int UI::TaskbarSettings::autohideDelayMs() const noexcept
{
  return m_autohideDelayMs;
}

/**
 * @brief Returns true when the taskbar is fully hidden (operator-mode override).
 */
bool UI::TaskbarSettings::taskbarHidden() const noexcept
{
  return m_taskbarHidden;
}

/**
 * @brief Switches between persistent (writes to QSettings) and ephemeral mode.
 */
void UI::TaskbarSettings::setSettingsPersistent(bool persistent)
{
  m_settingsPersistent = persistent;
}

/**
 * @brief Returns true when @a id appears in the current pinned-button list.
 */
bool UI::TaskbarSettings::isButtonPinned(const QString& id) const
{
  return m_pinnedButtons.contains(id);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces the pinned-button list with @a ids, sanitized + persisted.
 */
void UI::TaskbarSettings::setPinnedButtons(const QStringList& ids)
{
  const auto sanitized = sanitizePinned(ids);
  if (m_pinnedButtons == sanitized)
    return;

  m_pinnedButtons = sanitized;
  if (m_settingsPersistent)
    m_settings.setValue(QStringLiteral("Taskbar/PinnedButtons"), m_pinnedButtons);

  Q_EMIT pinnedButtonsChanged();
}

/**
 * @brief Persists the always-show-taskbar-buttons toggle.
 */
void UI::TaskbarSettings::setShowTaskbarButtons(bool enabled)
{
  if (m_showTaskbarButtons == enabled)
    return;

  m_showTaskbarButtons = enabled;
  if (m_settingsPersistent)
    m_settings.setValue(QStringLiteral("Taskbar/ShowTaskbarButtons"), m_showTaskbarButtons);

  Q_EMIT showTaskbarButtonsChanged();
}

/**
 * @brief Persists the search-field-enabled toggle.
 */
void UI::TaskbarSettings::setSearchEnabled(bool enabled)
{
  if (m_searchEnabled == enabled)
    return;

  m_searchEnabled = enabled;
  if (m_settingsPersistent)
    m_settings.setValue(QStringLiteral("Taskbar/SearchEnabled"), m_searchEnabled);

  Q_EMIT searchEnabledChanged();
}

/**
 * @brief Persists the autohide toggle.
 */
void UI::TaskbarSettings::setAutohide(bool enabled)
{
  if (m_autohide == enabled)
    return;

  m_autohide = enabled;
  if (m_settingsPersistent)
    m_settings.setValue(QStringLiteral("Taskbar/Autohide"), m_autohide);

  Q_EMIT autohideChanged();
}

/**
 * @brief Persists the autohide inactivity delay, clamped to [min, max].
 */
void UI::TaskbarSettings::setAutohideDelayMs(int delay)
{
  const int clamped = qBound(kMinAutohideDelayMs, delay, kMaxAutohideDelayMs);
  if (m_autohideDelayMs == clamped)
    return;

  m_autohideDelayMs = clamped;
  if (m_settingsPersistent)
    m_settings.setValue(QStringLiteral("Taskbar/AutohideDelayMs"), m_autohideDelayMs);

  Q_EMIT autohideDelayMsChanged();
}

/**
 * @brief Hides the taskbar entirely. Not persisted -- operator-mode override only.
 */
void UI::TaskbarSettings::setTaskbarHidden(bool hidden)
{
  if (m_taskbarHidden == hidden)
    return;

  m_taskbarHidden = hidden;
  Q_EMIT taskbarHiddenChanged();
}

//--------------------------------------------------------------------------------------------------
// Public mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves a pinned button to a new index in the ordered list.
 */
void UI::TaskbarSettings::movePinnedButton(const QString& id, int targetIndex)
{
  const int from = m_pinnedButtons.indexOf(id);
  if (from < 0)
    return;

  const int to = qBound(0, targetIndex, m_pinnedButtons.size() - 1);
  if (from == to)
    return;

  QStringList next = m_pinnedButtons;
  next.move(from, to);
  setPinnedButtons(next);
}

/**
 * @brief Adds @a id to or removes it from the pinned list, preserving order.
 *
 * Pinning re-inserts the entry at its canonical position when previously
 * unknown so a freshly-enabled button doesn't always land at the end.
 */
void UI::TaskbarSettings::setButtonPinned(const QString& id, bool pinned)
{
  const auto canonical = canonicalAvailableButtons();
  if (!canonical.contains(id))
    return;

  QStringList next   = m_pinnedButtons;
  const bool already = next.contains(id);
  if (pinned && !already) {
    // Insert at the canonical position relative to other already-pinned items
    const int desiredIdx = canonical.indexOf(id);
    int insertAt         = next.size();
    for (int i = 0; i < next.size(); ++i) {
      if (canonical.indexOf(next.at(i)) > desiredIdx) {
        insertAt = i;
        break;
      }
    }
    next.insert(insertAt, id);
  }

  else if (!pinned && already)
    next.removeAll(id);

  else
    return;

  setPinnedButtons(next);
}

/**
 * @brief Restores all taskbar preferences to first-install defaults.
 */
void UI::TaskbarSettings::resetToDefaults()
{
  setShowTaskbarButtons(false);
  setSearchEnabled(true);
  setAutohide(false);
  setAutohideDelayMs(kDefaultAutohideDelayMs);
  setPinnedButtons(defaultPinnedButtons());
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Default-pinned button IDs on first install (file_transmission excluded).
 */
QStringList UI::TaskbarSettings::defaultPinnedButtons()
{
  return {QStringLiteral("settings"),
          QStringLiteral("console"),
          QStringLiteral("notifications"),
          QStringLiteral("pause")};
}

/**
 * @brief Canonical ordered list of every pin ID the registry exposes.
 */
QStringList UI::TaskbarSettings::canonicalAvailableButtons()
{
  return {QStringLiteral("settings"),
          QStringLiteral("console"),
          QStringLiteral("notifications"),
          QStringLiteral("pause"),
          QStringLiteral("file_transmission")};
}

/**
 * @brief Filters @a ids against the canonical list and drops duplicates.
 */
QStringList UI::TaskbarSettings::sanitizePinned(const QStringList& ids) const
{
  const auto canonical = canonicalAvailableButtons();
  QStringList out;
  out.reserve(ids.size());
  for (const auto& id : ids)
    if (canonical.contains(id) && !out.contains(id))
      out.append(id);

  return out;
}
