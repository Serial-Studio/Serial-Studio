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

#include <QJSEngine>
#include <QJSValue>
#include <QTimer>

#include "DataModel/IScriptEngine.h"

namespace DataModel {

/**
 * @brief JavaScript script engine implementation backed by QJSEngine.
 */
class JsScriptEngine final : public IScriptEngine {
public:
  JsScriptEngine();
  ~JsScriptEngine() override = default;

  [[nodiscard]] bool loadScript(const QString& script,
                                int sourceId,
                                bool showMessageBoxes) override;

  [[nodiscard]] QList<QStringList> parseString(const QString& frame) override;
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override;

  [[nodiscard]] bool isLoaded() const noexcept override;

  void collectGarbage() override;
  void reset() override;

private:
  static constexpr int kRuntimeWatchdogMs = 1000;

  [[nodiscard]] QJSValue guardedCall(QJSValueList& args);

  [[nodiscard]] bool validateScriptSyntax(const QString& script,
                                          int sourceId,
                                          bool showMessageBoxes);
  [[nodiscard]] QJSValue validateParseFunction(int sourceId, bool showMessageBoxes);
  [[nodiscard]] bool probeParseFunction(const QJSValue& parseFunction,
                                        int sourceId,
                                        bool showMessageBoxes);
  [[nodiscard]] bool validateParseSignature(const QString& script, bool showMessageBoxes);

private:
  QJSEngine m_engine;
  QJSValue m_parseFunction;
  QJSValue m_hexToArray;
  QTimer m_watchdog;
};

}  // namespace DataModel
