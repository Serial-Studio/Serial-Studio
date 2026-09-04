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

#include <array>
#include <QDateTime>
#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QString>

#include "Core/Async/TaskTree.h"
#include "Misc/Diagnostics/DiagnosticsShared.h"
#include "Misc/ProblemCenter.h"

namespace Misc {

/**
 * @brief Session-scoped runner for the connection self-checks: instant checks answer inside the
 *        call, probing checks run on the async task tree under explicit timeouts. Results cache
 *        per bus and the problem-center checkers registered here only read that cache, since a
 *        checker must return its findings synchronously. The constructor is inert.
 */
class ConnectionDiagnostics : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool running
             READ  running
             NOTIFY runningChanged)
  Q_PROPERTY(bool hasFailure
             READ  hasFailure
             NOTIFY resultsChanged)
  Q_PROPERTY(QString lastRunTime
             READ  lastRunTime
             NOTIFY resultsChanged)
  Q_PROPERTY(QString failureTitle
             READ  failureTitle
             NOTIFY resultsChanged)
  Q_PROPERTY(QString failureRemedy
             READ  failureRemedy
             NOTIFY resultsChanged)
  // clang-format on

public:
  using Bus         = Diagnostics::Bus;
  using BusMask     = Diagnostics::BusMask;
  using Result      = Diagnostics::Result;
  using ResultCache = std::array<QList<Diagnostics::Result>, Diagnostics::kBusCount>;

signals:
  void runFinished();
  void runningChanged();
  void resultsChanged();

private:
  explicit ConnectionDiagnostics();
  ConnectionDiagnostics(ConnectionDiagnostics&&)                 = delete;
  ConnectionDiagnostics(const ConnectionDiagnostics&)            = delete;
  ConnectionDiagnostics& operator=(ConnectionDiagnostics&&)      = delete;
  ConnectionDiagnostics& operator=(const ConnectionDiagnostics&) = delete;

public:
  [[nodiscard]] static ConnectionDiagnostics& instance();

  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] bool hasFailure() const noexcept;

  [[nodiscard]] QString lastRunTime() const;
  [[nodiscard]] QString failureTitle() const;
  [[nodiscard]] QString failureRemedy() const;

  [[nodiscard]] QList<Result> results(BusMask scope) const;

  [[nodiscard]] static BusMask supportedBuses() noexcept;
  [[nodiscard]] static BusMask probingBuses(BusMask scope);
  [[nodiscard]] static int estimatedMsec(BusMask scope);

  void run(BusMask scope);
  void runInstant(BusMask scope, bool preserveStanding);
  void onOpenFailed(Bus bus, const QString& reason);
  void onOpenSucceeded(Bus bus);

public slots:
  void cancel();
  void runAll();
  void setupExternalConnections();

private:
  void publish();
  void clearScope(BusMask scope);
  void store(const Result& result);
  void restoreStanding(BusMask scope, const ResultCache& standing);
  void appendCached(Bus bus, QList<ProblemCenter::Finding>& out) const;
  void addReachabilityProbe(Async::SequentialGroup* group, Bus bus);
  void onRunFinished(Async::Outcome outcome, const Async::StepError& error);

  [[nodiscard]] bool allowAutoProbe(Bus bus);
  [[nodiscard]] const Result* topResult() const noexcept;
  [[nodiscard]] Async::Task* buildProbeTree(BusMask scope);

private:
  bool m_running;
  BusMask m_activeScope;
  QDateTime m_lastRun;
  Async::TaskRunner m_runner;
  Misc::ProblemCenter* m_problems;
  ResultCache m_results;
  std::array<QElapsedTimer, Diagnostics::kBusCount> m_autoRunClocks;
};

}  // namespace Misc
