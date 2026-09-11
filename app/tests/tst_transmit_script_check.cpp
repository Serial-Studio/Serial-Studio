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

#include <QJSEngine>
#include <QString>
#include <QTest>

#include "DataModel/Scripting/TransmitScriptCheck.h"

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

static const QString kValidScript =
  QStringLiteral("function transmit(value) { return [0x01, value & 0xFF]; }");

static const QString kSyntaxError =
  QStringLiteral("function transmit(value) {\n  return [0x01,\n}");

static const QString kWrongEntryPoint = QStringLiteral("function send(value) { return [value]; }");

static const QString kTopLevelLoop =
  QStringLiteral("var i = 0; while (true) { i = (i + 1) % 1000003; }");

/**
 * @brief The suite validates the verdict machine, not the host surface, so it prepares nothing.
 *        Production callers pass prepareTransmitScriptEngine; keeping that out of this link set is
 *        why the environment lives in its own translation unit.
 */
static bool noPreparation(QJSEngine&)
{
  return true;
}

/**
 * @brief The one verdict the transmit editor and the assistant's dry run both read. A script that
 *        compiles but never defines the entry point is a distinct outcome from one that does not
 *        compile, because the two need different corrections and the runtime's bare
 *        `return transmit` cannot tell them apart.
 */
class TransmitScriptCheckTests : public QObject {
  Q_OBJECT

private slots:
  void validScriptIsAccepted();
  void syntaxErrorReportsMessageAndLine();
  void wrongEntryPointIsNotACompileError();
  void emptyScriptIsEmpty();
  void hostFailureIsNotACodeVerdict();
  void runawayTopLevelTimesOut();
};

//--------------------------------------------------------------------------------------------------
// Tests
//--------------------------------------------------------------------------------------------------

/**
 * @brief A script defining a callable transmit(value) is the only accepted shape.
 */
void TransmitScriptCheckTests::validScriptIsAccepted()
{
  const auto verdict = DataModel::checkTransmitScript(kValidScript, noPreparation);
  QCOMPARE(verdict.status, DataModel::TransmitScriptStatus::Ok);
  QVERIFY(verdict.ok());
  QVERIFY(verdict.message.isEmpty());
}

/**
 * @brief A compile failure carries the engine's own text and the line it happened on, which is
 *        what the editor's status strip and the assistant's compileError field both surface.
 */
void TransmitScriptCheckTests::syntaxErrorReportsMessageAndLine()
{
  const auto verdict = DataModel::checkTransmitScript(kSyntaxError, noPreparation);
  QCOMPARE(verdict.status, DataModel::TransmitScriptStatus::CompileError);
  QVERIFY(!verdict.ok());
  QVERIFY(!verdict.message.isEmpty());
  QVERIFY(verdict.line > 0);
}

/**
 * @brief Code that compiles but names its function something other than transmit is reported as a
 *        missing entry point, not as a compile error: the user has a working script and the wrong
 *        name, and telling them it does not compile sends them looking for a typo that is not
 * there.
 */
void TransmitScriptCheckTests::wrongEntryPointIsNotACompileError()
{
  const auto verdict = DataModel::checkTransmitScript(kWrongEntryPoint, noPreparation);
  QCOMPARE(verdict.status, DataModel::TransmitScriptStatus::NoEntryPoint);
  QVERIFY(!verdict.ok());
}

/**
 * @brief An empty script is its own outcome, distinct from a script that compiles without an
 *        entry point: deleting a transmit function is a deliberate edit and has to be storable,
 *        where NoEntryPoint is a mistake the editor refuses to write back.
 */
void TransmitScriptCheckTests::emptyScriptIsEmpty()
{
  const auto blank = DataModel::checkTransmitScript(QString(), noPreparation);
  QCOMPARE(blank.status, DataModel::TransmitScriptStatus::Empty);
  QVERIFY(!blank.ok());
  QVERIFY(blank.persistable());

  const auto spaces = DataModel::checkTransmitScript(QStringLiteral("   \n\t "), noPreparation);
  QCOMPARE(spaces.status, DataModel::TransmitScriptStatus::Empty);
  QVERIFY(spaces.persistable());
}

/**
 * @brief A host surface that could not be installed is reported as such rather than as a verdict
 *        about the user's code. On the judging surface the effectful bridges are installed first
 *        and masked second, so an engine whose masking failed still holds the live ones and must
 *        not compile anything.
 */
void TransmitScriptCheckTests::hostFailureIsNotACodeVerdict()
{
  const auto refuse = [](QJSEngine&) {
    return false;
  };
  const auto verdict = DataModel::checkTransmitScript(kValidScript, refuse);
  QCOMPARE(verdict.status, DataModel::TransmitScriptStatus::HostUnavailable);
  QVERIFY(!verdict.ok());
  QVERIFY(!verdict.persistable());
}

/**
 * @brief A script that loops at the top level is cut off by the session budget and reported as a
 *        timeout, which the assistant's dry run must keep answering with ScriptTimeout rather than
 *        folding into a compile error.
 */
void TransmitScriptCheckTests::runawayTopLevelTimesOut()
{
  const auto verdict = DataModel::checkTransmitScript(kTopLevelLoop, noPreparation);
  QCOMPARE(verdict.status, DataModel::TransmitScriptStatus::Timeout);
  QVERIFY(!verdict.ok());
}

QTEST_GUILESS_MAIN(TransmitScriptCheckTests)

#include "tst_transmit_script_check.moc"
