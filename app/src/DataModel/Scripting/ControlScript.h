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

#include <QObject>
#include <QString>
#include <QThread>

namespace DataModel {

class ControlApiMarshaller;
class ControlScriptWorker;

/**
 * @brief Drives an Arduino-style setup()/loop() control script over the connection lifecycle.
 */
class ControlScript : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString code
             READ code
             WRITE setCode
             NOTIFY codeChanged)
  Q_PROPERTY(bool running
             READ running
             NOTIFY runningChanged)
  // clang-format on

signals:
  void codeChanged();
  void runningChanged();
  void error(const QString& message);

public:
  ControlScript(ControlScript&&)                 = delete;
  ControlScript(const ControlScript&)            = delete;
  ControlScript& operator=(ControlScript&&)      = delete;
  ControlScript& operator=(const ControlScript&) = delete;

  [[nodiscard]] static ControlScript& instance();

  void setupExternalConnections();

  [[nodiscard]] QString code() const;
  [[nodiscard]] bool running() const;

public slots:
  void shutdown();
  void setCode(const QString& code);
  void runOnConnect();

private slots:
  void onConnectedChanged();
  void onWorkerError(const QString& message);

private:
  explicit ControlScript();
  ~ControlScript() override;

  void startWorker();
  void stopWorker();
  [[nodiscard]] bool shouldRun() const;

private:
  QString m_code;
  bool m_ready;
  bool m_running;
  bool m_shouldRun;
  bool m_shutdown;
  QThread m_thread;
  ControlScriptWorker* m_worker;
  ControlApiMarshaller* m_marshaller;
};

}  // namespace DataModel
