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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJSValue>
#include <QObject>
#include <QVariantMap>

class QJSEngine;
struct lua_State;

namespace DataModel {

/**
 * @brief QObject bridge that backs the JS dashboard globals. One per QJSEngine.
 */
class DashboardBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit DashboardBridge(QObject* parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QVariantMap clearPlots();
  Q_INVOKABLE [[nodiscard]] QVariantMap setPlotPoints(const QJSValue& pointsVal);
  Q_INVOKABLE [[nodiscard]] QVariantMap setTerminalVisible(const QJSValue& visibleVal);
  Q_INVOKABLE [[nodiscard]] QVariantMap setNotificationLogVisible(const QJSValue& visibleVal);
  Q_INVOKABLE [[nodiscard]] QVariantMap setClockVisible(const QJSValue& visibleVal);
  Q_INVOKABLE [[nodiscard]] QVariantMap setStopwatchVisible(const QJSValue& visibleVal);
  Q_INVOKABLE [[nodiscard]] QVariantMap setActiveWorkspace(const QJSValue& targetVal);
};

/**
 * @brief Installs the dashboard.* globals (clearPlots, setPlotPoints, UI toggles,
 *        setActiveWorkspace) into Lua or JS scripting engines.
 */
class DashboardApi {
public:
  static void installLua(lua_State* L);
  static void installJS(QJSEngine* js);
};

}  // namespace DataModel
