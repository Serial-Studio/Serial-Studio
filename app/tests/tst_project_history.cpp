/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QByteArray>
#include <QJsonObject>
#include <QTest>

#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectPersistence.h"
#include "DataModel/ProjectModel.h"

// ProjectHistory.cpp also carries ProjectUndoScope/ProjectUndoFrame, whose bodies reach the
// project model; the two symbols that leaves undefined are supplied here instead of linking
// ProjectModel.cpp, which would drag the whole application (session_context_stub.cpp precedent).
// No test constructs a ProjectModel, so neither stub is ever executed.

QJsonObject DataModel::ProjectPersistence::serializeToJson() const
{
  return QJsonObject();
}

void DataModel::ProjectModel::projectHistoryChanged() {}

using DataModel::ProjectHistory;

/**
 * @brief Runs one mutating scope to completion: stage the pre-state, then commit it the way
 *        ProjectModel::setModified(true) does.
 */
static void recordStep(ProjectHistory& history,
                       const QString& label,
                       const QByteArray& preState,
                       const QString& key = QString())
{
  if (history.enterScope(label, key) == ProjectHistory::ScopeAction::Capture)
    history.stageCapture(preState);

  history.commitPending();
  (void)history.leave();
}

/**
 * @brief Walks the whole undo stack, returning how many steps it held.
 */
static int drainUndoStack(ProjectHistory& history)
{
  int steps = 0;
  while (history.canUndo()) {
    history.confirmUndo(QByteArray());
    ++steps;
  }

  return steps;
}

/**
 * @brief Bounded whole-document undo history (spec 0031): the two-phase capture contract, the
 *        keystroke coalesce window, the step/byte bounds and the save-point bookkeeping.
 */
class TstProjectHistory : public QObject {
  Q_OBJECT

private slots:
  void stagedCaptureWithoutCommitRecordsNothing();
  void committedCaptureRecordsOneStep();
  void nestedScopesRecordOneStep();
  void hintKeyOverridesSlotKey();
  void sameKeyWithinWindowCoalesces();
  void sameKeyAfterWindowStartsANewStep();
  void differentKeysNeverCoalesce();
  void stepCountIsBounded();
  void byteBudgetIsBounded();
  void redoTailTruncatesOnANewStep();
  void savePointSurvivesUndoRedo();
  void savePointBecomesUnreachableWhenTruncated();
  void markSavedBreaksTheCoalesceChain();
  void disabledHistoryRecordsNothing();
  void applyingSuppressesCapture();
};

//--------------------------------------------------------------------------------------------------
// Two-phase capture
//--------------------------------------------------------------------------------------------------

/**
 * @brief A slot that opens a scope and guard-returns without mutating records nothing: the
 *        staged snapshot is discarded by leave(), never by commitPending().
 */
void TstProjectHistory::stagedCaptureWithoutCommitRecordsNothing()
{
  ProjectHistory history;
  history.setEnabled(true);

  QCOMPARE(history.enterScope(QStringLiteral("Edit"), QString()),
           ProjectHistory::ScopeAction::Capture);
  history.stageCapture(QByteArray("{\"a\":1}"));
  QVERIFY(!history.leave());
  QVERIFY(!history.canUndo());
}

/**
 * @brief The first setModified(true) turns the staged snapshot into a step, labelled by the slot.
 */
void TstProjectHistory::committedCaptureRecordsOneStep()
{
  ProjectHistory history;
  history.setEnabled(true);
  recordStep(history, QStringLiteral("Edit Dataset"), QByteArray("{\"a\":1}"));

  QVERIFY(history.canUndo());
  QVERIFY(!history.canRedo());
  QCOMPARE(history.undoText(), QStringLiteral("Edit Dataset"));
  QCOMPARE(history.peekUndoState(), QByteArray("{\"a\":1}"));
}

/**
 * @brief A composite operation (cascade delete, project.batch) nests scopes; only the outermost
 *        captures, so the whole composite undoes as one step.
 */
void TstProjectHistory::nestedScopesRecordOneStep()
{
  ProjectHistory history;
  history.setEnabled(true);

  history.pushFrame(QStringLiteral("Delete Selection"));
  QCOMPARE(history.enterScope(QStringLiteral("Delete Group"), QString()),
           ProjectHistory::ScopeAction::Capture);
  history.stageCapture(QByteArray("{\"a\":1}"));
  history.commitPending();
  QCOMPARE(history.enterScope(QStringLiteral("Delete Dataset"), QString()),
           ProjectHistory::ScopeAction::None);
  history.commitPending();
  QVERIFY(!history.leave());
  QVERIFY(!history.leave());
  QVERIFY(history.leave());

  QCOMPARE(drainUndoStack(history), 1);
}

/**
 * @brief The editor hint wins over the slot's own coalesce key, and is consumed by the next
 *        scope whether or not that scope captures.
 */
void TstProjectHistory::hintKeyOverridesSlotKey()
{
  ProjectHistory history;
  history.setEnabled(true);

  history.setNextHint(QStringLiteral("Edit Device"), QStringLiteral("hint"));
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"a\":1}"), QStringLiteral("slot"));
  QCOMPARE(history.undoText(), QStringLiteral("Edit Device"));

  QCOMPARE(history.enterScope(QStringLiteral("Edit"), QStringLiteral("hint")),
           ProjectHistory::ScopeAction::Coalesce);
  history.commitPending();
  (void)history.leave();
  QCOMPARE(drainUndoStack(history), 1);
}

//--------------------------------------------------------------------------------------------------
// Keystroke coalescing
//--------------------------------------------------------------------------------------------------

/**
 * @brief A keystroke burst on the same field extends the previous step and skips serialization
 *        entirely -- the pre-state stays the one captured before the first keystroke.
 */
void TstProjectHistory::sameKeyWithinWindowCoalesces()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"), QStringLiteral("title:1"));
  for (int i = 0; i < 5; ++i)
    recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":9}"), QStringLiteral("title:1"));

  QCOMPARE(history.peekUndoState(), QByteArray("{\"n\":0}"));
  QCOMPARE(drainUndoStack(history), 1);
}

/**
 * @brief Past the 1 s window the same key starts a fresh step, so a pause between edits is an
 *        undo boundary.
 */
void TstProjectHistory::sameKeyAfterWindowStartsANewStep()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"), QStringLiteral("title:1"));
  QTest::qSleep(1100);
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":1}"), QStringLiteral("title:1"));

  QCOMPARE(drainUndoStack(history), 2);
}

/**
 * @brief Two different fields never merge, however fast the user moves between them.
 */
void TstProjectHistory::differentKeysNeverCoalesce()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"), QStringLiteral("title:1"));
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":1}"), QStringLiteral("title:2"));
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":2}"), QString());

  QCOMPARE(drainUndoStack(history), 3);
}

//--------------------------------------------------------------------------------------------------
// Bounds
//--------------------------------------------------------------------------------------------------

/**
 * @brief The oldest steps are dropped past 100; the cap is what keeps a long editing session
 *        from growing without bound.
 */
void TstProjectHistory::stepCountIsBounded()
{
  ProjectHistory history;
  history.setEnabled(true);

  for (int i = 0; i < 130; ++i)
    recordStep(history, QStringLiteral("Edit %1").arg(i), QByteArray("{\"n\":1}"));

  QCOMPARE(drainUndoStack(history), 100);
}

/**
 * @brief Large documents hit the 64 MiB budget long before the step cap, and the eviction is
 *        from the front so the newest edits stay undoable.
 */
void TstProjectHistory::byteBudgetIsBounded()
{
  ProjectHistory history;
  history.setEnabled(true);

  const QByteArray payload(4 * 1024 * 1024, 'x');
  for (int i = 0; i < 24; ++i)
    recordStep(history, QStringLiteral("Edit %1").arg(i), payload);

  const int steps = drainUndoStack(history);
  QVERIFY(steps > 0);
  QVERIFY(steps <= 17);
}

//--------------------------------------------------------------------------------------------------
// Redo tail and save point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recording after an undo discards the redo tail: the branch the user walked away from
 *        must not survive as a replayable step.
 */
void TstProjectHistory::redoTailTruncatesOnANewStep()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("A"), QByteArray("{\"n\":0}"));
  recordStep(history, QStringLiteral("B"), QByteArray("{\"n\":1}"));
  history.confirmUndo(QByteArray("{\"n\":2}"));
  QVERIFY(history.canRedo());

  recordStep(history, QStringLiteral("C"), QByteArray("{\"n\":1}"));
  QVERIFY(!history.canRedo());
  QCOMPARE(history.undoText(), QStringLiteral("C"));
}

/**
 * @brief The modified flag after undo/redo is position-versus-save-position, so walking away
 *        from the save point and back clears it again.
 */
void TstProjectHistory::savePointSurvivesUndoRedo()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("A"), QByteArray("{\"n\":0}"));
  history.markSaved();
  QVERIFY(history.isAtSavePoint());

  history.confirmUndo(QByteArray("{\"n\":1}"));
  QVERIFY(!history.isAtSavePoint());

  history.confirmRedo();
  QVERIFY(history.isAtSavePoint());
}

/**
 * @brief A save point inside a truncated redo tail can never be reached again, so the project
 *        stays modified until the next real save.
 */
void TstProjectHistory::savePointBecomesUnreachableWhenTruncated()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("A"), QByteArray("{\"n\":0}"));
  recordStep(history, QStringLiteral("B"), QByteArray("{\"n\":1}"));
  history.markSaved();

  history.confirmUndo(QByteArray("{\"n\":2}"));
  recordStep(history, QStringLiteral("C"), QByteArray("{\"n\":1}"));

  QVERIFY(!history.isAtSavePoint());
  history.confirmUndo(QByteArray("{\"n\":3}"));
  QVERIFY(!history.isAtSavePoint());
}

/**
 * @brief Saving breaks the top step's coalesce chain, so the first keystroke after a save opens
 *        a step instead of extending the saved one.
 */
void TstProjectHistory::markSavedBreaksTheCoalesceChain()
{
  ProjectHistory history;
  history.setEnabled(true);

  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"), QStringLiteral("title:1"));
  history.markSaved();
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":1}"), QStringLiteral("title:1"));

  QCOMPARE(drainUndoStack(history), 2);
}

//--------------------------------------------------------------------------------------------------
// Suppression
//--------------------------------------------------------------------------------------------------

/**
 * @brief History is disabled for the whole ProjectModel ctor closure; nothing recorded there may
 *        become an undoable step.
 */
void TstProjectHistory::disabledHistoryRecordsNothing()
{
  ProjectHistory history;
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"));

  QVERIFY(!history.canUndo());
  QVERIFY(!history.canRedo());
}

/**
 * @brief Re-entrant mutations while a snapshot is being applied record nothing, which is what
 *        keeps undo/redo from pushing steps for its own restore pass.
 */
void TstProjectHistory::applyingSuppressesCapture()
{
  ProjectHistory history;
  history.setEnabled(true);
  history.setApplying(true);

  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"));
  QVERIFY(!history.canUndo());

  history.setApplying(false);
  recordStep(history, QStringLiteral("Edit"), QByteArray("{\"n\":0}"));
  QVERIFY(history.canUndo());
}

QTEST_APPLESS_MAIN(TstProjectHistory)

#include "tst_project_history.moc"
