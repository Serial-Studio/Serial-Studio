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

#include <deque>
#include <QByteArray>
#include <QElapsedTimer>
#include <QString>

namespace DataModel {

class ProjectModel;

/**
 * @brief Bounded undo/redo history of whole-document snapshots; plain value type (no QObject,
 *        no singleton access) so ProjectModel can own it inside its protected ctor closure.
 *        Depth-counted scopes make composite operations atomic; hinted keystroke bursts
 *        coalesce without capturing.
 */
class ProjectHistory {
public:
  enum class ScopeAction {
    None = 0,
    Coalesce,
    Capture,
  };

  ProjectHistory();

  void setEnabled(const bool enabled);
  void setApplying(const bool applying);
  [[nodiscard]] bool applying() const noexcept;

  [[nodiscard]] bool canUndo() const noexcept;
  [[nodiscard]] bool canRedo() const noexcept;
  [[nodiscard]] QString undoText() const;
  [[nodiscard]] QString redoText() const;

  void setNextHint(const QString& label, const QString& coalesceKey);

  void pushFrame(const QString& label);
  [[nodiscard]] ScopeAction enterScope(const QString& slotLabel, const QString& slotKey);
  void stageCapture(QByteArray preState);
  void commitPending();
  [[nodiscard]] bool leave();

  [[nodiscard]] QByteArray peekUndoState() const;
  [[nodiscard]] QByteArray peekRedoState() const;
  void confirmUndo(const QByteArray& currentState);
  void confirmRedo();

  void markSaved();
  [[nodiscard]] bool isAtSavePoint() const noexcept;

  void clear();

private:
  struct Step {
    QString label;
    QString coalesceKey;
    QByteArray preState;
    QByteArray postState;
    qint64 timestampMs;
  };

  void dropOverflow();
  void dropFront();

private:
  bool m_enabled;
  bool m_applying;
  int m_depth;
  bool m_stackCaptured;
  bool m_pendingStaged;
  bool m_pendingCoalesce;
  QString m_rootLabel;
  QString m_hintLabel;
  QString m_hintKey;
  QString m_pendingLabel;
  QString m_pendingKey;
  QByteArray m_pendingState;
  std::deque<Step> m_steps;
  size_t m_position;
  qint64 m_savePosition;
  qint64 m_totalBytes;
  QElapsedTimer m_clock;
};

/**
 * @brief RAII label frame: names the step captured by any nested mutating scope; never
 *        captures on its own, so wrapping a read-only operation costs nothing.
 */
class ProjectUndoFrame {
public:
  ProjectUndoFrame(ProjectModel& model, const QString& label);
  ~ProjectUndoFrame();

  ProjectUndoFrame(ProjectUndoFrame&&)                 = delete;
  ProjectUndoFrame(const ProjectUndoFrame&)            = delete;
  ProjectUndoFrame& operator=(ProjectUndoFrame&&)      = delete;
  ProjectUndoFrame& operator=(const ProjectUndoFrame&) = delete;

private:
  ProjectModel& m_model;
};

/**
 * @brief RAII mutating scope opened at the top of every document-mutating ProjectModel slot;
 *        only the outermost scope serializes the pre-state, nested scopes and coalesced
 *        keystroke bursts skip capture.
 */
class ProjectUndoScope {
public:
  ProjectUndoScope(ProjectModel& model, const QString& label, const QString& coalesceKey = {});
  ~ProjectUndoScope();

  ProjectUndoScope(ProjectUndoScope&&)                 = delete;
  ProjectUndoScope(const ProjectUndoScope&)            = delete;
  ProjectUndoScope& operator=(ProjectUndoScope&&)      = delete;
  ProjectUndoScope& operator=(const ProjectUndoScope&) = delete;

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
