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

#include <functional>
#include <QByteArray>
#include <QObject>
#include <QString>

#include "Core/Async/AsyncClock.h"
#include "Core/Async/TaskTree.h"
#include "Core/IO/ConnectionSequence.h"

namespace IO {
namespace Drivers {

class ISequencePort;

/**
 * @brief Runs one Connection Sequence over an open port (spec 0103) as an Async task tree: step
 *        tasks in a SequentialGroup under a TimeoutTask budget, timers from the injected clock.
 *        Fed bytes fill a bounded receive buffer the expect steps consume; bytes fed during a
 *        BREAK are dropped. Never blocks; finished() fires once per run, never for a cancel.
 */
class SequenceRunner : public QObject {
  Q_OBJECT

signals:
  void bytesSent(const QByteArray& data);
  void stepNote(int index, const QString& text);
  void stepStarted(int index, const QString& text);
  void finished(bool ok, const QString& reason, int stepIndex);

public:
  SequenceRunner(const IO::ConnectionSequence& sequence,
                 ISequencePort& port,
                 Async::AsyncClock& clock,
                 QObject* parent = nullptr);
  ~SequenceRunner() override;

  SequenceRunner(SequenceRunner&&)                 = delete;
  SequenceRunner(const SequenceRunner&)            = delete;
  SequenceRunner& operator=(SequenceRunner&&)      = delete;
  SequenceRunner& operator=(const SequenceRunner&) = delete;

  [[nodiscard]] bool breakHeld() const noexcept;
  [[nodiscard]] bool isRunning() const noexcept;
  [[nodiscard]] int currentStep() const noexcept;

public slots:
  void start();
  void cancel();
  void feed(const QByteArray& bytes);

private:
  [[nodiscard]] Async::Task* buildTree();
  [[nodiscard]] Async::Task* makeStep(int index);
  [[nodiscard]] bool holdBreak(bool enabled);
  [[nodiscard]] bool runInstant(int index, QString& reason);
  [[nodiscard]] bool runSend(const IO::SequenceStep& step, QString& reason);
  [[nodiscard]] QString failureText(int index, const QString& reason) const;

  void beginStep(int index);
  void finishWith(bool ok, const QString& reason, int index);
  void onTreeFinished(Async::Outcome outcome, const Async::StepError& error);

private:
  bool m_running;
  bool m_cancelled;
  bool m_breakHeld;
  int m_current;
  QByteArray m_rx;
  ISequencePort& m_port;
  Async::AsyncClock& m_clock;
  std::function<void()> m_poll;
  IO::ConnectionSequence m_sequence;
  Async::TaskRunner m_runner;
};

}  // namespace Drivers
}  // namespace IO
