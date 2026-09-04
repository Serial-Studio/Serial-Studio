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

#include <QJSValue>
#include <QObject>
#include <QVariantMap>

class QJSEngine;
struct lua_State;

namespace DataModel {

/**
 * @brief QObject bridge that backs the JS apiCall() global. One per QJSEngine.
 */
class ScriptApiCallBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit ScriptApiCallBridge(int sourceId, QObject* parent = nullptr);

  void bindSourceId(const int* sourceId);

  Q_INVOKABLE [[nodiscard]] QVariantMap call(const QJSValue& methodVal, const QJSValue& paramsVal);
  Q_INVOKABLE [[nodiscard]] QVariantList listCommands();

private:
  int m_ownSourceId;
  const int* m_sourceId;
};

/**
 * @brief Installs the apiCall(method, params?) global into Lua and JS engines.
 */
class ScriptApiCall {
public:
  static void installLua(lua_State* L, int sourceId);
  static void installJS(QJSEngine* js, int sourceId);
  static void bindSourceIdJS(QJSEngine* js, const int* sourceId);
  static void installHelperBridgesJS(QJSEngine* js, int sourceId);
  static void installAll(QJSEngine* js, int sourceId);
  static void installAll(lua_State* L, int sourceId);
};

}  // namespace DataModel
