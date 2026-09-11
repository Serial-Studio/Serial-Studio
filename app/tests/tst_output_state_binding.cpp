/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <chrono>
#include <QString>
#include <QTest>

#include "UI/Widgets/Output/StateBinding.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

using Binding = Widgets::Output::StateBinding;

static constexpr qint64 kNow    = 1'000'000;
static constexpr int kConfirmMs = 3000;

/**
 * @brief A widget bound to a dataset, with the given on-value rule.
 */
static DataModel::OutputWidget boundWidget(const QString& onValue = QString())
{
  DataModel::OutputWidget w;
  w.title          = QStringLiteral("Fan");
  w.stateSource    = DataModel::OutputStateSource::Dataset;
  w.stateDatasetId = 7;
  w.stateOnValue   = onValue;
  w.stateConfirmMs = kConfirmMs;
  return w;
}

/**
 * @brief The rules a bound output control displays by (spec 0080). The load-bank case is the
 *        reason: a control that latches on its own click keeps claiming a fan is running after an
 *        interlock stopped it, so what the control shows must come from what the equipment says.
 */
class OutputStateBindingTests : public QObject {
  Q_OBJECT

private slots:
  void unboundKnowsNothing();
  void numericSourceDrivesTheDisplay();
  void aTextSourceNeedsAnExplicitOnValue();
  void onValueMatchesAsTextAndAsNumber();
  void silenceBecomesUnknownRatherThanOff();
  void anOutstandingRequestClearsOnConfirmationAndOnExpiry();
  void interactionFreezesTheDisplay();
  void theIngestStampAndTheVerdictShareOneClock();
  void aTableVariableGoesStaleWhenItsWriterStops();
  void theConfirmWindowIsCapped();
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief An unbound control reports nothing, which is what keeps every existing project unchanged.
 */
void OutputStateBindingTests::unboundKnowsNothing()
{
  Binding binding;
  binding.configure(DataModel::OutputWidget());

  QVERIFY(!binding.bound());
  QVERIFY(!binding.verdict(kNow).known);
}

/**
 * @brief With no on-value, any non-zero number is on and zero is off.
 */
void OutputStateBindingTests::numericSourceDrivesTheDisplay()
{
  Binding binding;
  binding.configure(boundWidget());
  QVERIFY(binding.bound());

  binding.observe(1.0, QString(), kNow);
  auto verdict = binding.verdict(kNow);
  QVERIFY(verdict.known);
  QVERIFY(verdict.on);

  binding.observe(0.0, QString(), kNow);
  verdict = binding.verdict(kNow);
  QVERIFY(verdict.known);
  QVERIFY(!verdict.on);
}

/**
 * @brief The trap the on-value field exists to remove: a device reporting RUN/STOP carries no
 *        number, so under the bare non-zero rule it reads as off forever -- including while it is
 *        running. Nothing here is truthy; the user must say which text means on.
 */
void OutputStateBindingTests::aTextSourceNeedsAnExplicitOnValue()
{
  Binding binding;
  binding.configure(boundWidget());

  binding.observe(0.0, QStringLiteral("RUN"), kNow);
  QVERIFY(!binding.verdict(kNow).on);

  binding.configure(boundWidget(QStringLiteral("RUN")));
  binding.observe(0.0, QStringLiteral("RUN"), kNow);
  QVERIFY(binding.verdict(kNow).on);
}

/**
 * @brief A filled on-value matches numerically when it parses as a number, and case-insensitively
 *        as text otherwise, so both `1` and `RUN` work without the user choosing a mode.
 */
void OutputStateBindingTests::onValueMatchesAsTextAndAsNumber()
{
  QVERIFY(Binding::truth(QStringLiteral("1"), 1.0, QString()));
  QVERIFY(!Binding::truth(QStringLiteral("1"), 0.0, QString()));
  QVERIFY(Binding::truth(QStringLiteral("3"), 3.0, QString()));
  QVERIFY(!Binding::truth(QStringLiteral("3"), 1.0, QString()));

  QVERIFY(Binding::truth(QStringLiteral("RUN"), 0.0, QStringLiteral("run")));
  QVERIFY(Binding::truth(QStringLiteral("RUN"), 0.0, QStringLiteral("  RUN ")));
  QVERIFY(!Binding::truth(QStringLiteral("RUN"), 0.0, QStringLiteral("STOP")));
}

/**
 * @brief A source that stops arriving reports unknown, never off. Showing "off" for "we have not
 *        heard" is the same false statement about the plant that this feature exists to remove.
 */
void OutputStateBindingTests::silenceBecomesUnknownRatherThanOff()
{
  Binding binding;
  binding.configure(boundWidget());
  QVERIFY(!binding.verdict(kNow).known);

  binding.observe(1.0, QString(), kNow);
  QVERIFY(binding.verdict(kNow).known);

  const qint64 wellPast = kNow + (kConfirmMs * 2) + 1;
  QVERIFY(!binding.verdict(wellPast).known);
}

/**
 * @brief The outstanding window closes on its own, so a command the equipment silently refused
 *        does not read as pending forever.
 */
void OutputStateBindingTests::anOutstandingRequestClearsOnConfirmationAndOnExpiry()
{
  Binding binding;
  binding.configure(boundWidget());
  QVERIFY(!binding.pending(kNow));

  binding.noteRequested(kNow);
  QVERIFY(binding.pending(kNow + 1));
  QVERIFY(binding.holdsDisplay(kNow + 1));

  QVERIFY(!binding.pending(kNow + kConfirmMs));
  QVERIFY(!binding.holdsDisplay(kNow + kConfirmMs));
}

/**
 * @brief Feedback never moves a control the operator is holding (R14).
 */
void OutputStateBindingTests::interactionFreezesTheDisplay()
{
  Binding binding;
  binding.configure(boundWidget());

  binding.beginInteraction();
  QVERIFY(binding.holdsDisplay(kNow));

  binding.endInteraction(kNow);
  QVERIFY(binding.holdsDisplay(kNow + 1));
  QVERIFY(!binding.holdsDisplay(kNow + kConfirmMs));
}

/**
 * @brief The stamp the dashboard puts on a sample and the "now" the verdict measures against must
 *        be the same clock. They were not: ingest stamped steady_clock while the verdict asked
 *        QDateTime, and the ~1.7e12 ms gap between the epochs exceeded every stale window, so
 *        every dataset-bound control reported unknown forever. The suite could not see it because
 *        each test fed one synthetic constant to both sides, so this one reproduces the producer's
 *        own expression rather than reusing nowMs().
 */
void OutputStateBindingTests::theIngestStampAndTheVerdictShareOneClock()
{
  const auto ingestStamp =
    static_cast<qint64>(std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count());

  Binding binding;
  binding.configure(boundWidget());
  binding.observe(1.0, QString(), ingestStamp);

  const auto verdict = binding.verdict(Binding::nowMs());
  QVERIFY(verdict.known);
  QVERIFY(verdict.on);
}

/**
 * @brief A table variable carries no receipt time, so freshness comes from the store's write
 *        clock. Re-reading a variable nobody is writing must not keep it alive: stamping "now" on
 *        every read made a table-bound control permanently live no matter how long ago its writer
 *        died, which is the opposite of what R7 promises.
 */
void OutputStateBindingTests::aTableVariableGoesStaleWhenItsWriterStops()
{
  auto widget          = boundWidget();
  widget.stateSource   = DataModel::OutputStateSource::Table;
  widget.stateTable    = QStringLiteral("Config");
  widget.stateVariable = QStringLiteral("fan");

  Binding binding;
  binding.configure(widget);

  binding.observeTable(1.0, QString(), 100);
  const auto first = Binding::nowMs();
  QVERIFY(binding.verdict(first).known);

  binding.observeTable(1.0, QString(), 100);
  QVERIFY(!binding.verdict(first + (kConfirmMs * 2) + 1).known);

  binding.observeTable(1.0, QString(), 101);
  QVERIFY(binding.verdict(Binding::nowMs()).known);
}

/**
 * @brief The confirm-within window has a ceiling as well as a floor. Without one a hand-edited
 *        project could hold a control in "request outstanding" for weeks, and a control that never
 *        adopts what the equipment reports is the exact failure this feature exists to remove.
 */
void OutputStateBindingTests::theConfirmWindowIsCapped()
{
  auto widget           = boundWidget();
  widget.stateConfirmMs = 7'000'000;

  Binding binding;
  binding.configure(widget);
  binding.noteRequested(kNow);

  QVERIFY(!binding.pending(kNow + DataModel::kMaxOutputStateConfirmMs));
}

QTEST_APPLESS_MAIN(OutputStateBindingTests)

#include "tst_output_state_binding.moc"
