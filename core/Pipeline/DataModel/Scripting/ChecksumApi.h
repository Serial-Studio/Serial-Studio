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
#include <QStringList>

class QJSEngine;
struct lua_State;

namespace DataModel {

/**
 * @brief QObject bridge behind the JS checksum globals (the SDK prelude wraps it as
 *        checksum / checksumHex / checksumAlgorithms). One per QJSEngine.
 */
class ChecksumBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit ChecksumBridge(QObject* parent = nullptr);

  Q_INVOKABLE [[nodiscard]] QJSValue checksum(const QJSValue& algorithm, const QJSValue& data);
  Q_INVOKABLE [[nodiscard]] QJSValue checksumHex(const QJSValue& algorithm, const QJSValue& data);
  Q_INVOKABLE [[nodiscard]] QStringList checksumAlgorithms() const;

private:
  [[nodiscard]] QJSValue compute(const QJSValue& algorithm, const QJSValue& data, bool hex);
};

/**
 * @brief Installs the checksum helpers every script engine shares: checksum(algorithm, data)
 *        returns the digest bytes in wire order as a byte string, checksumHex(algorithm, data) the
 *        same as lowercase hex, checksumAlgorithms() the names Core::Checksum knows. Lua gets
 *        native globals; JS gets the __ss_ck bridge the SDK prelude wraps.
 */
class ChecksumApi {
public:
  static void installLua(lua_State* L);
  static void installJS(QJSEngine* js);
};

}  // namespace DataModel
