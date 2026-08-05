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

#include "Misc/ConnectionDiagnostics.h"

#include <algorithm>
#include <functional>
#include <QLocale>
#include <utility>

#include "Misc/Diagnostics/BluetoothChecks.h"
#include "Misc/Diagnostics/NetworkChecks.h"
#include "Misc/Diagnostics/SerialChecks.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "Misc/Diagnostics/AudioChecks.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants & local aliases
//--------------------------------------------------------------------------------------------------

using Misc::Diagnostics::busBit;
using Misc::Diagnostics::kBusCount;
using Misc::Diagnostics::kBusNone;
using Misc::Diagnostics::scopeCovers;

static constexpr int kRunTimeoutMs    = 15000;
static constexpr int kAutoRunWindowMs = 30000;

//--------------------------------------------------------------------------------------------------
// Probe wrapper
//--------------------------------------------------------------------------------------------------

namespace Misc::detail {
/**
 * @brief Wraps one probe so a failing probe cannot abort the rest of the run: the child's outcome
 *        goes to a reporter and the wrapper finishes Success regardless, except on Cancelled,
 *        which propagates so cancel() genuinely stops the run. It carries no new signal, so it
 *        needs no meta-object of its own.
 */
class ProbeTask final : public Async::Task {
public:
  using Reporter = std::function<void(Async::Outcome, const Async::StepError&)>;

  ProbeTask(Async::Task* child, Reporter reporter);

protected:
  void doStart() override;
  void doCancel() override;

private:
  void onChildFinished(Async::Outcome outcome, const Async::StepError& error);

private:
  Async::Task* m_child;
  Reporter m_reporter;
};
}  // namespace Misc::detail

using Misc::detail::ProbeTask;

/**
 * @brief Adopts @p child and the reporter its outcome is handed to.
 */
Misc::detail::ProbeTask::ProbeTask(Async::Task* child, Reporter reporter)
  : Async::Task(child != nullptr ? child->name() : QStringLiteral("probe"))
  , m_child(child)
  , m_reporter(std::move(reporter))
{
  SS_ASSERT_LOG(m_child != nullptr);
  SS_ASSERT_LOG(static_cast<bool>(m_reporter));

  m_child->setParent(this);
  connect(m_child, &Async::Task::finished, this, &ProbeTask::onChildFinished);
}

/**
 * @brief Starts the wrapped probe.
 */
void Misc::detail::ProbeTask::doStart()
{
  SS_ASSERT(m_child != nullptr, {
    reportFinished(Async::Outcome::Success, Async::StepError());
    return;
  });
  SS_ASSERT_LOG(!m_child->isRunning());

  m_child->start();
}

/**
 * @brief Cancels the wrapped probe, letting the child's Cancelled outcome finish this wrapper.
 */
void Misc::detail::ProbeTask::doCancel()
{
  SS_ASSERT(m_child != nullptr, return);

  if (m_child->isRunning())
    m_child->cancel();
}

/**
 * @brief Records the child's outcome and swallows its failure, so the next probe in the run still
 *        gets started.
 */
void Misc::detail::ProbeTask::onChildFinished(Async::Outcome outcome, const Async::StepError& error)
{
  SS_ASSERT_LOG(m_child != nullptr);

  if (!isRunning())
    return;

  if (outcome == Async::Outcome::Cancelled) {
    reportFinished(outcome, error);
    return;
  }

  m_reporter(outcome, error);
  reportFinished(Async::Outcome::Success, Async::StepError());
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the runner with no dependencies at all: the spec-0001 ctor-edge proof holds
 *        only while this constructor stays a leaf (see the class documentation).
 */
Misc::ConnectionDiagnostics::ConnectionDiagnostics()
  : m_running(false)
  , m_activeScope(kBusNone)
  , m_lastRun()
  , m_runner()
  , m_problems(nullptr)
  , m_results()
  , m_autoRunClocks()
{}

/**
 * @brief Returns the singleton ConnectionDiagnostics instance.
 */
Misc::ConnectionDiagnostics& Misc::ConnectionDiagnostics::instance()
{
  static ConnectionDiagnostics singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Public accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a probing run is currently in flight.
 */
bool Misc::ConnectionDiagnostics::running() const noexcept
{
  return m_running;
}

/**
 * @brief Returns whether the last run left anything worth acting on.
 */
bool Misc::ConnectionDiagnostics::hasFailure() const noexcept
{
  const Result* top = topResult();
  return top != nullptr && top->verdict >= Diagnostics::Verdict::Warning;
}

/**
 * @brief Returns the localized time of the last run, or an empty string before the first one.
 */
QString Misc::ConnectionDiagnostics::lastRunTime() const
{
  if (!m_lastRun.isValid())
    return QString();

  return QLocale::system().toString(m_lastRun.time(), QLocale::ShortFormat);
}

/**
 * @brief Returns the title of the most severe standing result, for the setup-pane banner.
 */
QString Misc::ConnectionDiagnostics::failureTitle() const
{
  const Result* top = topResult();
  return top != nullptr ? top->title : QString();
}

/**
 * @brief Returns the remedy of the most severe standing result, verbatim so an embedded command
 *        can be selected and copied without editing.
 */
QString Misc::ConnectionDiagnostics::failureRemedy() const
{
  const Result* top = topResult();
  return top != nullptr ? top->remedy : QString();
}

/**
 * @brief Returns the cached results for every bus in @p scope, in bus order.
 */
QList<Misc::ConnectionDiagnostics::Result> Misc::ConnectionDiagnostics::results(BusMask scope) const
{
  QList<Result> list;
  for (int i = 0; i < kBusCount; ++i)
    if (scopeCovers(scope, static_cast<Bus>(i)))
      list.append(m_results[static_cast<size_t>(i)]);

  return list;
}

/**
 * @brief Returns the buses this build can check; a bus that ships only with Pro modules is absent
 *        from the mask rather than present with a stubbed checker.
 */
Misc::ConnectionDiagnostics::BusMask Misc::ConnectionDiagnostics::supportedBuses() noexcept
{
  BusMask mask = busBit(Bus::Serial) | busBit(Bus::Bluetooth) | busBit(Bus::Network);

#ifdef BUILD_COMMERCIAL
  mask |= busBit(Bus::Broker) | busBit(Bus::Audio);
#endif

  return mask;
}

/**
 * @brief Returns the subset of @p scope that has probing work to do, which is every bus with a
 *        fully configured endpoint to reach.
 */
Misc::ConnectionDiagnostics::BusMask Misc::ConnectionDiagnostics::probingBuses(BusMask scope)
{
  BusMask mask                 = kBusNone;
  const Bus candidates[]       = {Bus::Network, Bus::Broker};
  constexpr int kCandidateSize = 2;

  for (int i = 0; i < kCandidateSize; ++i) {
    QString host;
    quint16 port = 0;
    if (!scopeCovers(scope, candidates[i]))
      continue;

    if (!Diagnostics::NetworkChecks::endpoint(candidates[i], host, port))
      continue;

    if (!host.isEmpty() && port != 0)
      mask |= busBit(candidates[i]);
  }

  return mask;
}

/**
 * @brief Returns the declared worst-case duration of a run over @p scope, in milliseconds.
 */
int Misc::ConnectionDiagnostics::estimatedMsec(BusMask scope)
{
  const auto probing = probingBuses(scope);

  int count = 0;
  for (int i = 0; i < kBusCount; ++i)
    if (scopeCovers(probing, static_cast<Bus>(i)))
      ++count;

  return qMin(count * Diagnostics::NetworkChecks::probeBudgetMsec(), kRunTimeoutMs);
}

//--------------------------------------------------------------------------------------------------
// Running
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the instant checks for @p scope and publishes them, then starts the probing checks.
 *        A request already covered by a run in flight is refused rather than interleaved.
 */
void Misc::ConnectionDiagnostics::run(BusMask scope)
{
  scope &= supportedBuses();
  if (scope == kBusNone)
    return;

  if (m_running && (m_activeScope & scope) == scope)
    return;

  runInstant(scope, false);

  Async::Task* root = buildProbeTree(scope);
  if (root == nullptr)
    return;

  m_running     = true;
  m_activeScope = scope;
  Q_EMIT runningChanged();

  m_runner.run(root);
}

/**
 * @brief Replaces the cached results of every bus in @p scope with what the instant checks report
 *        right now, and publishes them through the problem center. With @p preserveStanding the
 *        results the instant checks did not re-produce survive (deduplicated by code): the
 *        rate-limited failure path must not erase a standing probe finding it cannot re-run.
 */
void Misc::ConnectionDiagnostics::runInstant(BusMask scope, bool preserveStanding)
{
  scope &= supportedBuses();
  if (scope == kBusNone)
    return;

  ResultCache standing;
  if (preserveStanding)
    for (int i = 0; i < kBusCount; ++i)
      if (scopeCovers(scope, static_cast<Bus>(i)))
        standing[static_cast<size_t>(i)] = m_results[static_cast<size_t>(i)];

  clearScope(scope);

  QList<Result> scratch;
  if (scopeCovers(scope, Bus::Serial))
    Diagnostics::SerialChecks::collect(scratch);

  if (scopeCovers(scope, Bus::Bluetooth))
    Diagnostics::BluetoothChecks::collect(scratch);

  if (scopeCovers(scope, Bus::Network))
    Diagnostics::NetworkChecks::collectInstant(Bus::Network, scratch);

#ifdef BUILD_COMMERCIAL
  if (scopeCovers(scope, Bus::Broker))
    Diagnostics::NetworkChecks::collectInstant(Bus::Broker, scratch);

  if (scopeCovers(scope, Bus::Audio))
    Diagnostics::AudioChecks::collect(scratch);
#endif

  for (const auto& result : std::as_const(scratch))
    store(result);

  if (preserveStanding)
    restoreStanding(scope, standing);

  m_lastRun = QDateTime::currentDateTime();
  publish();
}

/**
 * @brief Re-appends every standing result of @p scope whose code the fresh instant results do not
 *        already carry, so a preserved finding is never duplicated.
 */
void Misc::ConnectionDiagnostics::restoreStanding(BusMask scope, const ResultCache& standing)
{
  for (int i = 0; i < kBusCount; ++i) {
    if (!scopeCovers(scope, static_cast<Bus>(i)))
      continue;

    auto& fresh = m_results[static_cast<size_t>(i)];
    for (const auto& result : standing[static_cast<size_t>(i)]) {
      const bool duplicate = std::any_of(
        fresh.cbegin(), fresh.cend(), [&result](const Result& r) { return r.code == result.code; });
      if (!duplicate)
        fresh.append(result);
    }
  }
}

/**
 * @brief Runs every check this build supports.
 */
void Misc::ConnectionDiagnostics::runAll()
{
  run(supportedBuses());
}

/**
 * @brief Stops a run in flight; the cached results of the buses it already answered stand.
 */
void Misc::ConnectionDiagnostics::cancel()
{
  m_runner.cancel();
}

//--------------------------------------------------------------------------------------------------
// Connection-outcome hooks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Diagnoses a failed open of @p bus. The instant checks always run, so the remedy is
 *        available immediately; the probing checks are rate-limited per bus so a retry loop
 *        cannot spam a broker. The driver's own reason is already surfaced by the connection
 *        error path and is not duplicated as a finding.
 */
void Misc::ConnectionDiagnostics::onOpenFailed(Bus bus, const QString& reason)
{
  Q_UNUSED(reason)

  const BusMask scope = busBit(bus) & supportedBuses();
  if (scope == kBusNone)
    return;

  if (allowAutoProbe(bus)) {
    run(scope);
    return;
  }

  runInstant(scope, true);
}

/**
 * @brief Drops the cached results of a bus that just opened successfully, so a fixed condition
 *        does not stand in the panel until the next manual run.
 */
void Misc::ConnectionDiagnostics::onOpenSucceeded(Bus bus)
{
  const auto index = static_cast<size_t>(bus);
  m_autoRunClocks[index].invalidate();

  if (m_results[index].isEmpty())
    return;

  m_results[index].clear();
  publish();
}

//--------------------------------------------------------------------------------------------------
// Registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers one problem-center checker per bus and wires the runner's completion. Each
 *        checker is a pure reader of the result cache: the collector's contract is synchronous,
 *        so an asynchronous probe can never live inside one.
 */
void Misc::ConnectionDiagnostics::setupExternalConnections()
{
  connect(&m_runner, &Async::TaskRunner::finished, this, &ConnectionDiagnostics::onRunFinished);

  auto& center = Misc::ProblemCenter::instance();
  m_problems   = &center;
  for (int i = 0; i < kBusCount; ++i) {
    const auto bus = static_cast<Bus>(i);
    center.registerChecker(
      Diagnostics::checkerId(bus),
      Misc::ProblemCenter::OnDemand,
      [this, bus](QList<ProblemCenter::Finding>& out) { appendCached(bus, out); });
  }
}

//--------------------------------------------------------------------------------------------------
// Internals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-reads every checker so the cache change reaches the panel, the badge and the API.
 */
void Misc::ConnectionDiagnostics::publish()
{
  SS_ASSERT(m_problems != nullptr, return);

  m_problems->runNow();
  Q_EMIT resultsChanged();
}

/**
 * @brief Drops the cached results of every bus in @p scope, so a fixed condition disappears
 *        instead of accumulating across runs.
 */
void Misc::ConnectionDiagnostics::clearScope(BusMask scope)
{
  for (int i = 0; i < kBusCount; ++i)
    if (scopeCovers(scope, static_cast<Bus>(i)))
      m_results[static_cast<size_t>(i)].clear();
}

/**
 * @brief Caches one result under the bus it names; a Pass carries no finding and is dropped.
 */
void Misc::ConnectionDiagnostics::store(const Result& result)
{
  if (result.verdict == Diagnostics::Verdict::Pass)
    return;

  const auto index = static_cast<size_t>(result.bus);
  SS_ASSERT(index < m_results.size(), return);

  m_results[index].append(result);
}

/**
 * @brief Converts the cached results of @p bus into problem-center findings.
 */
void Misc::ConnectionDiagnostics::appendCached(Bus bus, QList<ProblemCenter::Finding>& out) const
{
  const auto index = static_cast<size_t>(bus);
  SS_ASSERT(index < m_results.size(), return);

  for (const auto& result : m_results[index])
    out.append(Diagnostics::toFinding(result));
}

/**
 * @brief Returns the most severe standing result across every bus, or nullptr when none stands.
 */
const Misc::ConnectionDiagnostics::Result* Misc::ConnectionDiagnostics::topResult() const noexcept
{
  const Result* best = nullptr;
  for (const auto& slice : m_results) {
    for (const auto& result : slice)
      if (best == nullptr || result.verdict > best->verdict)
        best = &result;
  }

  return best;
}

/**
 * @brief Reports whether a failure-triggered probing run may start for @p bus, arming the window
 *        when it may.
 */
bool Misc::ConnectionDiagnostics::allowAutoProbe(Bus bus)
{
  auto& clock = m_autoRunClocks[static_cast<size_t>(bus)];
  if (clock.isValid() && clock.elapsed() < kAutoRunWindowMs)
    return false;

  clock.start();
  return true;
}

/**
 * @brief Appends one bus's reachability probe to @p group, capturing the endpoint by value so the
 *        probe never holds a pointer into the driver whose configuration it read.
 */
void Misc::ConnectionDiagnostics::addReachabilityProbe(Async::SequentialGroup* group, Bus bus)
{
  SS_ASSERT(group != nullptr, return);

  QString host;
  quint16 port = 0;
  if (!Diagnostics::NetworkChecks::endpoint(bus, host, port))
    return;

  if (host.isEmpty() || port == 0)
    return;

  auto* flow = Diagnostics::NetworkChecks::makeReachabilityFlow(host, port, m_runner.clock());
  ProbeTask::Reporter reporter = [this, bus, host, port](Async::Outcome outcome,
                                                         const Async::StepError& error) {
    store(Diagnostics::NetworkChecks::reachabilityResult(bus, host, port, outcome, error));
  };

  group->addChild(new ProbeTask(flow, std::move(reporter)));
}

/**
 * @brief Builds the run's probe tree under one overall deadline, or nullptr when @p scope has no
 *        probing work at all.
 */
Async::Task* Misc::ConnectionDiagnostics::buildProbeTree(BusMask scope)
{
  auto* group = Async::sequential(QStringLiteral("connection-diagnostics"));

  if (scopeCovers(scope, Bus::Network))
    addReachabilityProbe(group, Bus::Network);

  if (scopeCovers(scope, Bus::Broker))
    addReachabilityProbe(group, Bus::Broker);

  if (group->childCount() > 0)
    return Async::timeout(group, kRunTimeoutMs, m_runner.clock());

  delete group;
  return nullptr;
}

/**
 * @brief Settles a finished run: the probes have already cached their results, so this publishes
 *        them and releases the scope.
 */
void Misc::ConnectionDiagnostics::onRunFinished(Async::Outcome outcome,
                                                const Async::StepError& error)
{
  Q_UNUSED(outcome)
  Q_UNUSED(error)

  m_running     = false;
  m_activeScope = kBusNone;
  m_lastRun     = QDateTime::currentDateTime();

  publish();
  Q_EMIT runningChanged();
  Q_EMIT runFinished();
}
