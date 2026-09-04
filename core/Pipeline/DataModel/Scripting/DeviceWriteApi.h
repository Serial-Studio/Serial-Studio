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
 * @brief QObject bridge that backs the JS deviceWrite() global. One per QJSEngine.
 */
class DeviceWriteBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit DeviceWriteBridge(QObject* parent = nullptr);

  int defaultSourceId = 0;

  Q_INVOKABLE [[nodiscard]] QVariantMap write(const QJSValue& data, const QJSValue& sourceIdVal);
};

/**
 * @brief QObject bridge that backs the JS actionFire() global. One per QJSEngine.
 */
class ActionFireBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit ActionFireBridge(QObject* parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QVariantMap fire(const QJSValue& actionIdVal);
};

/**
 * @brief Installs the deviceWrite(data, sourceId?) global into Lua or JS scripting engines.
 */
class DeviceWriteApi {
public:
  static void installLua(lua_State* L, int defaultSourceId);
  static void installJS(QJSEngine* js, int defaultSourceId);

  static void setLuaDefaultSourceId(lua_State* L, int defaultSourceId);
  static void setJSDefaultSourceId(QJSEngine* js, int defaultSourceId);
};

/**
 * @brief Installs the actionFire(actionId) global into Lua or JS scripting engines.
 */
class ActionFireApi {
public:
  static void installLua(lua_State* L);
  static void installJS(QJSEngine* js);
};

}  // namespace DataModel
