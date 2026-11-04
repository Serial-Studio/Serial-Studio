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
using Outcome = Binding::Outcome;

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
 * @brief A control with a reply binding and no state source, with the given type.
 */
static DataModel::OutputWidget replyWidget(const int confirmMs = kConfirmMs)
{
  DataModel::OutputWidget w;
  w.title                 = QStringLiteral("Blink");
  w.replyTokenDatasetId   = 11;
  w.replyCodeDatasetId    = 12;
  w.replyMessageDatasetId = 13;
  w.stateConfirmMs        = confirmMs;
  return w;
}

/**
 * @brief A toggle with both a state source and a reply binding.
 */
static DataModel::OutputWidget replyAndStateWidget()
{
  auto w                  = boundWidget();
  w.type                  = DataModel::OutputWidgetType::Toggle;
  w.replyTokenDatasetId   = 11;
  w.replyCodeDatasetId    = 12;
  w.replyMessageDatasetId = 13;
  return w;
}

/**
 * @brief Sends a command the way Base does: records the request, then the token that went out.
 */
static int sendCommand(Binding& binding, const qint64 now, const QVariant& request = QVariant())
{
  binding.noteRequested(now, request);
  const int token = binding.issueToken();
  binding.noteSent(now, token);
  return token;
}

/**
 * @brief The rules a bound output control displays by (spec 0080). The load-bank case is the
 *        reason: a control that latches on its own click keeps claiming a fan is running after an
 *        interlock stopped it, so what the control shows must come from what the equipment says.
 */
class OutputStateBindingTests : public QObject {
  Q_OBJECT

private slots:
  void init();
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
  void theRequestedStateEndsTheWaitEarly();
  void aReplyWithThePreviousTokenLeavesTheControlWaiting();
  void controlsOnOneTokenDatasetNeverDrawTheSameToken();
  void aTokenNeverRepeatsTheOneTheLineShowed();
  void theCounterWrapsToOneAndNeverIssuesZero();
  void codeZeroAcknowledgesAndEndsPending();
  void aRefusalEndsTheHoldOnTheDisplay();
  void anAcknowledgementHoldsUntilTheStateConfirms();
  void anAcknowledgementThatNeverConfirmsEndsWithTheWindow();
  void silenceBecomesUnanswered();
  void theReplyWindowHasAFloor();
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief Token counters are process-wide; each case starts from a clean map.
 */
void OutputStateBindingTests::init()
{
  Binding::resetTokens();
}

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

  auto widget = boundWidget();
  widget.type = DataModel::OutputWidgetType::Toggle;
  binding.configure(widget);
  binding.observe(0.0, QString(), kNow);
  binding.noteRequested(kNow, 1);
  QVERIFY(binding.pending(kNow + 1));

  binding.observe(1.0, QString(), kNow + 2);
  QVERIFY(!binding.pending(kNow + 3));
  QVERIFY(!binding.holdsDisplay(kNow + 3));
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

/**
 * @brief The state the operator asked for ends the wait for a toggle, a value and a text control
 *        (R11); any other state leaves it open, and once confirmed the window stays closed.
 */
void OutputStateBindingTests::theRequestedStateEndsTheWaitEarly()
{
  Binding toggle;
  auto widget = boundWidget();
  widget.type = DataModel::OutputWidgetType::Toggle;
  toggle.configure(widget);
  toggle.noteRequested(kNow, 1);
  toggle.observe(0.0, QString(), kNow + 1);
  QVERIFY(toggle.pending(kNow + 2));
  toggle.observe(1.0, QString(), kNow + 3);
  QVERIFY(!toggle.pending(kNow + 4));
  toggle.settle(kNow + 4);
  toggle.observe(0.0, QString(), kNow + 5);
  QVERIFY(!toggle.pending(kNow + 6));
  QVERIFY(!toggle.holdsDisplay(kNow + 6));

  Binding value;
  widget      = boundWidget();
  widget.type = DataModel::OutputWidgetType::Slider;
  value.configure(widget);
  value.noteRequested(kNow, 40.0);
  value.observe(39.0, QString(), kNow + 1);
  QVERIFY(value.pending(kNow + 2));
  value.observe(40.0, QString(), kNow + 3);
  QVERIFY(!value.pending(kNow + 4));

  Binding text;
  widget      = boundWidget();
  widget.type = DataModel::OutputWidgetType::TextField;
  text.configure(widget);
  text.noteRequested(kNow, QStringLiteral("go"));
  text.observe(0.0, QStringLiteral("stop"), kNow + 1);
  QVERIFY(text.pending(kNow + 2));
  text.observe(0.0, QStringLiteral("go"), kNow + 3);
  QVERIFY(!text.pending(kNow + 4));
}

/**
 * @brief A reply carrying the previous token, seen after a second send, settles nothing (AC2).
 */
void OutputStateBindingTests::aReplyWithThePreviousTokenLeavesTheControlWaiting()
{
  Binding binding;
  binding.configure(replyWidget());

  const int first = sendCommand(binding, kNow);
  binding.observeReply(kNow + 10, first, 0, QStringLiteral("OK"));
  QVERIFY(binding.outcome(kNow + 11) == Outcome::Acknowledged);

  const int second = sendCommand(binding, kNow + 100);
  QVERIFY(second != first);
  binding.observeReply(kNow + 110, first, 0, QStringLiteral("OK"));
  QVERIFY(binding.outcome(kNow + 111) == Outcome::Waiting);
  QVERIFY(binding.pending(kNow + 111));

  binding.observeReply(kNow + 120, second, 0, QStringLiteral("OK"));
  QVERIFY(binding.outcome(kNow + 121) == Outcome::Acknowledged);
}

/**
 * @brief Two controls answering on one token dataset share one counter, so they never wait on the
 *        same number.
 */
void OutputStateBindingTests::controlsOnOneTokenDatasetNeverDrawTheSameToken()
{
  Binding a;
  Binding b;
  a.configure(replyWidget());
  b.configure(replyWidget());

  QSet<int> seen;
  for (int i = 0; i < 20; ++i) {
    const int fromA = a.issueToken();
    QVERIFY(!seen.contains(fromA));
    seen.insert(fromA);
    const int fromB = b.issueToken();
    QVERIFY(!seen.contains(fromB));
    seen.insert(fromB);
  }
}

/**
 * @brief The skip rule: the token the line showed last tick is never issued next, so a device
 *        that kept running across a reconnect cannot acknowledge a fresh command with a stale
 *        reply.
 */
void OutputStateBindingTests::aTokenNeverRepeatsTheOneTheLineShowed()
{
  Binding binding;
  binding.configure(replyWidget());

  binding.observeReply(kNow, 1, 0, QString());
  QCOMPARE(binding.issueToken(), 2);

  Binding::resetTokens();
  binding.observeReply(kNow, 2, 0, QString());
  QCOMPARE(binding.issueToken(), 1);
}

/**
 * @brief Tokens run 1..32767, wrap to 1, and zero (a device's power-on value) is never issued; a
 *        control with no reply binding draws from the shared counter without error.
 */
void OutputStateBindingTests::theCounterWrapsToOneAndNeverIssuesZero()
{
  Binding binding;
  binding.configure(replyWidget());

  int last = 0;
  for (int i = 0; i < 32767; ++i) {
    last = binding.issueToken();
    QVERIFY(last > 0);
  }

  QCOMPARE(last, 32767);
  QCOMPARE(binding.issueToken(), 1);

  Binding plain;
  plain.configure(DataModel::OutputWidget());
  QCOMPARE(plain.issueToken(), 1);
}

/**
 * @brief Code 0 acknowledges and ends the pending wait; the message is kept.
 */
void OutputStateBindingTests::codeZeroAcknowledgesAndEndsPending()
{
  Binding binding;
  binding.configure(replyWidget());

  const int token = sendCommand(binding, kNow);
  QVERIFY(binding.pending(kNow + 1));
  QVERIFY(binding.outcome(kNow + 1) == Outcome::Waiting);

  binding.observeReply(kNow + 2, token, 0, QStringLiteral("OK"));
  QVERIFY(binding.outcome(kNow + 3) == Outcome::Acknowledged);
  QVERIFY(!binding.pending(kNow + 3));
  QCOMPARE(binding.replyCode(), 0);
  QCOMPARE(binding.replyMessage(), QStringLiteral("OK"));
}

/**
 * @brief A refusal ends the hold on the display: the reported state wins at once.
 */
void OutputStateBindingTests::aRefusalEndsTheHoldOnTheDisplay()
{
  Binding binding;
  binding.configure(replyAndStateWidget());
  binding.observe(0.0, QString(), kNow);

  const int token = sendCommand(binding, kNow, 1);
  QVERIFY(binding.holdsDisplay(kNow + 1));

  binding.observeReply(kNow + 2, token, 3, QStringLiteral("too hot"));
  QVERIFY(binding.outcome(kNow + 3) == Outcome::Refused);
  QVERIFY(!binding.holdsDisplay(kNow + 3));
  QVERIFY(!binding.pending(kNow + 3));
  QCOMPARE(binding.replyCode(), 3);
  QCOMPARE(binding.replyMessage(), QStringLiteral("too hot"));
}

/**
 * @brief With a state source, an acknowledgement still holds the display until the state
 *        confirms the request (AC3).
 */
void OutputStateBindingTests::anAcknowledgementHoldsUntilTheStateConfirms()
{
  Binding binding;
  binding.configure(replyAndStateWidget());
  binding.observe(0.0, QString(), kNow);

  const int token = sendCommand(binding, kNow, 1);
  binding.observeReply(kNow + 2, token, 0, QStringLiteral("OK"));
  QVERIFY(!binding.pending(kNow + 3));
  QVERIFY(binding.holdsDisplay(kNow + 3));

  binding.observe(1.0, QString(), kNow + 4);
  QVERIFY(!binding.holdsDisplay(kNow + 5));
}

/**
 * @brief An acknowledged request whose state never arrives stops holding when the window ends.
 */
void OutputStateBindingTests::anAcknowledgementThatNeverConfirmsEndsWithTheWindow()
{
  Binding binding;
  binding.configure(replyAndStateWidget());
  binding.observe(0.0, QString(), kNow);

  const int token = sendCommand(binding, kNow, 1);
  binding.observeReply(kNow + 2, token, 0, QStringLiteral("OK"));
  QVERIFY(binding.holdsDisplay(kNow + kConfirmMs - 1));
  QVERIFY(!binding.holdsDisplay(kNow + kConfirmMs));
}

/**
 * @brief No echo inside the window reads unanswered, even with no frame to prompt a refresh, and
 *        a late echo does not undo it.
 */
void OutputStateBindingTests::silenceBecomesUnanswered()
{
  Binding binding;
  binding.configure(replyWidget(1000));

  const int token = sendCommand(binding, kNow);
  QVERIFY(binding.outcome(kNow + 999) == Outcome::Waiting);
  QVERIFY(binding.outcome(kNow + 1000) == Outcome::Unanswered);
  QVERIFY(!binding.pending(kNow + 1000));

  binding.settle(kNow + 1000);
  binding.observeReply(kNow + 1001, token, 0, QStringLiteral("OK"));
  QVERIFY(binding.outcome(kNow + 1002) == Outcome::Unanswered);
}

/**
 * @brief A zero Confirm Within never declares a reply unanswered at once: the reply window is
 *        floored at 100 ms, while a state-only control keeps its zero.
 */
void OutputStateBindingTests::theReplyWindowHasAFloor()
{
  Binding binding;
  binding.configure(replyWidget(0));
  QCOMPARE(binding.windowMs(), qint64(100));

  const int token = sendCommand(binding, kNow);
  QVERIFY(binding.outcome(kNow + 50) == Outcome::Waiting);
  binding.observeReply(kNow + 60, token, 0, QString());
  QVERIFY(binding.outcome(kNow + 61) == Outcome::Acknowledged);

  auto widget           = boundWidget();
  widget.stateConfirmMs = 0;
  binding.configure(widget);
  QCOMPARE(binding.windowMs(), qint64(0));
}

QTEST_APPLESS_MAIN(OutputStateBindingTests)

#include "tst_output_state_binding.moc"
