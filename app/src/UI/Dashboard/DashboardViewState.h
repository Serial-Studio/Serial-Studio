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
#include <QString>
#include <QVariant>

class QSettings;

namespace UI {

/**
 * @brief Session view state (spec 0062) and layout preferences of the dashboard. View state is
 *        session state, never project state: it never marks the project modified and is dropped
 *        whenever the widget identity space changes. Every mutator answers whether it changed
 *        anything instead of emitting; the notify signals belong to the Dashboard facade.
 */
class DashboardViewState {
public:
  explicit DashboardViewState(QSettings& settings);
  DashboardViewState(DashboardViewState&&)                 = delete;
  DashboardViewState(const DashboardViewState&)            = delete;
  DashboardViewState& operator=(DashboardViewState&&)      = delete;
  DashboardViewState& operator=(const DashboardViewState&) = delete;

  [[nodiscard]] bool autoHideToolbar() const noexcept;
  [[nodiscard]] bool showActionPanel() const noexcept;
  [[nodiscard]] bool showAlignmentGuides() const noexcept;
  [[nodiscard]] int layoutMargin() const noexcept;
  [[nodiscard]] int layoutSpacing() const noexcept;

  [[nodiscard]] QString viewStateJson() const;
  [[nodiscard]] QJsonObject globalViewState() const;
  [[nodiscard]] QJsonObject widgetViewState(const QString& widgetId) const;

  [[nodiscard]] bool clearViewState();
  [[nodiscard]] bool setViewStateJson(const QString& json);
  [[nodiscard]] bool saveGlobalViewState(const QString& key, const QVariant& value);
  [[nodiscard]] bool saveWidgetViewState(const QString& widgetId,
                                         const QString& key,
                                         const QVariant& value);

  [[nodiscard]] bool setLayoutMargin(const int margin);
  [[nodiscard]] bool setLayoutSpacing(const int spacing);
  [[nodiscard]] bool setAutoHideToolbar(const bool enabled);
  [[nodiscard]] bool setShowActionPanel(const bool enabled);
  [[nodiscard]] bool setShowAlignmentGuides(const bool enabled);

  void restoreViewPreferences();
  void restoreLayoutPreferences();
  void setSettingsPersistent(const bool persistent);

private:
  QSettings& m_settings;

  bool m_persistSettings;
  bool m_showActionPanel;
  bool m_autoHideToolbar;
  bool m_showAlignmentGuides;

  int m_layoutMargin;
  int m_layoutSpacing;

  QJsonObject m_widgetViewState;
  QJsonObject m_globalViewState;
};

}  // namespace UI
