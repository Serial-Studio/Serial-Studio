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

#include <QObject>
#include <QString>
#include <QThread>
#include <QVariantMap>

struct lua_State;

namespace DataModel {

class MacroWorker;
class FrameBuilder;
class ControlApiMarshaller;

/**
 * @brief QML-instantiable facade behind the API Terminal's Script tab: owns the JS macro
 *        worker thread, the GUI-synchronous Lua path, verify, and macro load/save (spec 0046).
 */
class MacroRunner : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(bool canStop
             READ canStop
             NOTIFY busyChanged)
  // clang-format on

signals:
  void busyChanged();
  void macroSaved(const QString& fileName);
  void logMessage(const QString& message);
  void finished(const QString& result);
  void scriptError(const QString& message);
  void macroLoaded(const QString& text, int language, const QString& fileName);

public:
  explicit MacroRunner(QObject* parent = nullptr);
  ~MacroRunner() override;

  MacroRunner(MacroRunner&&)                 = delete;
  MacroRunner(const MacroRunner&)            = delete;
  MacroRunner& operator=(MacroRunner&&)      = delete;
  MacroRunner& operator=(const MacroRunner&) = delete;

  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] bool canStop() const noexcept;

  void postLog(const QString& message);

  Q_INVOKABLE [[nodiscard]] QVariantMap verify(const QString& source, int language) const;

public slots:
  void stop();
  void loadMacro();
  void runJs(const QString& source);
  void runLua(const QString& source);
  void saveMacro(const QString& source, int language);

private slots:
  void onWorkerFinished(const QString& result);
  void onWorkerError(const QString& message);

private:
  [[nodiscard]] QString macrosDirectory() const;
  [[nodiscard]] QVariantMap verifyJs(const QString& source) const;
  [[nodiscard]] QVariantMap verifyLua(const QString& source) const;
  void closeMacroLuaState(lua_State* state);

private:
  bool m_busy;
  bool m_jsRunning;
  QThread* m_thread;
  MacroWorker* m_worker;
  ControlApiMarshaller* m_marshaller;
  FrameBuilder& m_frameBuilder;
};

}  // namespace DataModel
