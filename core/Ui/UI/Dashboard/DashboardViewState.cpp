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

#include "UI/Dashboard/DashboardViewState.h"

#include <QJsonDocument>
#include <QJSValue>
#include <QSettings>

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the view state with the same defaults the dashboard shows before any settings
 *        are restored; @p settings is the facade's store, so both write the same keys.
 */
UI::DashboardViewState::DashboardViewState(QSettings& settings)
  : m_settings(settings)
  , m_persistSettings(true)
  , m_showActionPanel(true)
  , m_autoHideToolbar(false)
  , m_showAlignmentGuides(false)
  , m_layoutMargin(0)
  , m_layoutSpacing(-1)
{}

//--------------------------------------------------------------------------------------------------
// Layout & panel preferences
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the toolbar should automatically hide when the dashboard is visible.
 */
bool UI::DashboardViewState::autoHideToolbar() const noexcept
{
  return m_autoHideToolbar;
}

/**
 * @brief Returns true if a rectangle with a list of actions should be displayed alongside the
 * dashboard.
 */
bool UI::DashboardViewState::showActionPanel() const noexcept
{
  return m_showActionPanel;
}

/**
 * @brief Returns true if smart alignment guides are shown during manual-mode gestures.
 */
bool UI::DashboardViewState::showAlignmentGuides() const noexcept
{
  return m_showAlignmentGuides;
}

/**
 * @brief Returns the margin (px) reserved between the widget canvas and the pane edges,
 *        shared by the auto and manual layout modes.
 */
int UI::DashboardViewState::layoutMargin() const noexcept
{
  return m_layoutMargin;
}

/**
 * @brief Returns the spacing (px) between adjacent windows in both layout modes: the auto
 *        tiler inserts it, manual gestures snap and weld to it (-1 = shared border).
 */
int UI::DashboardViewState::layoutSpacing() const noexcept
{
  return m_layoutSpacing;
}

/**
 * @brief Enables or disables auto-hiding the toolbar when the dashboard is shown; answers
 *        whether the flag moved. Persisted unconditionally: the toolbar policy is a global
 *        preference, not part of the state an ephemeral session must leave untouched.
 */
bool UI::DashboardViewState::setAutoHideToolbar(const bool enabled)
{
  if (m_autoHideToolbar == enabled)
    return false;

  m_autoHideToolbar = enabled;
  m_settings.setValue("Dashboard/AutoHideToolbar", m_autoHideToolbar);
  return true;
}

/**
 * @brief Enables/disables the action panel; answers whether the flag moved.
 */
bool UI::DashboardViewState::setShowActionPanel(const bool enabled)
{
  if (m_showActionPanel == enabled)
    return false;

  m_showActionPanel = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ShowActionPanel", m_showActionPanel);

  return true;
}

/**
 * @brief Shows or hides the smart alignment guides drawn during manual-mode gestures; answers
 *        whether the flag moved.
 */
bool UI::DashboardViewState::setShowAlignmentGuides(const bool enabled)
{
  if (m_showAlignmentGuides == enabled)
    return false;

  m_showAlignmentGuides = enabled;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/ShowAlignmentGuides", m_showAlignmentGuides);

  return true;
}

/**
 * @brief Sets the canvas edge margin (px) shared by both layout modes; clamped to >= 0 and
 *        persisted. Answers whether the value moved.
 */
bool UI::DashboardViewState::setLayoutMargin(const int margin)
{
  const int clamped = qMax(0, margin);
  if (m_layoutMargin == clamped)
    return false;

  m_layoutMargin = clamped;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/LayoutMargin", m_layoutMargin);

  return true;
}

/**
 * @brief Sets the inter-window spacing (px) shared by both layout modes; clamped to >= -1
 *        (the default, which overlaps two borders into one shared line) and persisted.
 *        Answers whether the value moved.
 */
bool UI::DashboardViewState::setLayoutSpacing(const int spacing)
{
  const int clamped = qMax(-1, spacing);
  if (m_layoutSpacing == clamped)
    return false;

  m_layoutSpacing = clamped;
  if (m_persistSettings)
    m_settings.setValue("Dashboard/LayoutSpacing", m_layoutSpacing);

  return true;
}

//--------------------------------------------------------------------------------------------------
// Persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores the persisted panel/toolbar flags from QSettings.
 */
void UI::DashboardViewState::restoreViewPreferences()
{
  m_autoHideToolbar     = m_settings.value("Dashboard/AutoHideToolbar", false).toBool();
  m_showActionPanel     = m_settings.value("Dashboard/ShowActionPanel", true).toBool();
  m_showAlignmentGuides = m_settings.value("Dashboard/ShowAlignmentGuides", false).toBool();
}

/**
 * @brief Restores the persisted canvas margin and window spacing, honoring the legacy
 *        auto-layout-only keys they replaced.
 */
void UI::DashboardViewState::restoreLayoutPreferences()
{
  m_layoutMargin = qMax(
    0,
    m_settings.value("Dashboard/LayoutMargin", m_settings.value("Dashboard/AutoLayoutMargin", 0))
      .toInt());
  m_layoutSpacing = qMax(
    -1,
    m_settings.value("Dashboard/LayoutSpacing", m_settings.value("Dashboard/AutoLayoutSpacing", -1))
      .toInt());
}

/**
 * @brief Toggles whether preference changes are written to QSettings.
 */
void UI::DashboardViewState::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

//--------------------------------------------------------------------------------------------------
// Session view state (spec 0062)
//--------------------------------------------------------------------------------------------------

/**
 * @brief View state one widget pushed (cursors, zoom/pan, paused, ...); empty when none.
 *        View state is session state, never project state: it never marks the project modified.
 */
QJsonObject UI::DashboardViewState::widgetViewState(const QString& widgetId) const
{
  return m_widgetViewState.value(widgetId).toObject();
}

/**
 * @brief Global view state (active workspace, plot time range, theme id).
 */
QJsonObject UI::DashboardViewState::globalViewState() const
{
  return m_globalViewState;
}

/**
 * @brief The whole view state as one compact JSON document (what a recording bundles).
 */
QString UI::DashboardViewState::viewStateJson() const
{
  QJsonObject root;
  root.insert(QStringLiteral("version"), 1);
  root.insert(QStringLiteral("global"), m_globalViewState);
  root.insert(QStringLiteral("widgets"), m_widgetViewState);
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

/**
 * @brief Records one per-widget view value; answers true only on a real change so the
 *        recording bundle's debounce sees edits, not repaints.
 */
bool UI::DashboardViewState::saveWidgetViewState(const QString& widgetId,
                                                 const QString& key,
                                                 const QVariant& value)
{
  if (widgetId.isEmpty() || key.isEmpty())
    return false;

  auto normalized = value;
  if (normalized.userType() == qMetaTypeId<QJSValue>())
    normalized = normalized.value<QJSValue>().toVariant();

  auto obj            = m_widgetViewState.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(normalized);
  if (obj.value(key) == newValue)
    return false;

  obj.insert(key, newValue);
  m_widgetViewState.insert(widgetId, obj);
  return true;
}

/**
 * @brief Records one global view value (see saveWidgetViewState).
 */
bool UI::DashboardViewState::saveGlobalViewState(const QString& key, const QVariant& value)
{
  if (key.isEmpty())
    return false;

  auto normalized = value;
  if (normalized.userType() == qMetaTypeId<QJSValue>())
    normalized = normalized.value<QJSValue>().toVariant();

  const auto newValue = QJsonValue::fromVariant(normalized);
  if (m_globalViewState.value(key) == newValue)
    return false;

  m_globalViewState.insert(key, newValue);
  return true;
}

/**
 * @brief Replaces the view state from a bundled document (session playback); widgets created
 *        afterwards read it in their Component.onCompleted. Malformed input clears it.
 */
bool UI::DashboardViewState::setViewStateJson(const QString& json)
{
  const auto doc = QJsonDocument::fromJson(json.toUtf8());
  m_widgetViewState =
    doc.isObject() ? doc.object().value(QStringLiteral("widgets")).toObject() : QJsonObject();
  m_globalViewState =
    doc.isObject() ? doc.object().value(QStringLiteral("global")).toObject() : QJsonObject();
  return true;
}

/**
 * @brief Drops every recorded view value; answers false when there was nothing to drop.
 */
bool UI::DashboardViewState::clearViewState()
{
  if (m_widgetViewState.isEmpty() && m_globalViewState.isEmpty())
    return false;

  m_widgetViewState = QJsonObject();
  m_globalViewState = QJsonObject();
  return true;
}
