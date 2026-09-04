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

#include "DataModel/Project/ProjectHistory.h"

#include <QJsonDocument>

#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"

static constexpr int kMaxUndoSteps        = 100;
static constexpr qint64 kMaxUndoBytes     = 64 * 1024 * 1024;
static constexpr qint64 kCoalesceWindowMs = 1000;
static constexpr qint64 kSaveUnreachable  = -1;

//--------------------------------------------------------------------------------------------------
// Construction & status
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty, disabled history; ProjectModel enables it after its ctor closure.
 */
DataModel::ProjectHistory::ProjectHistory()
  : m_enabled(false)
  , m_applying(false)
  , m_depth(0)
  , m_stackCaptured(false)
  , m_pendingStaged(false)
  , m_pendingCoalesce(false)
  , m_position(0)
  , m_savePosition(0)
  , m_totalBytes(0)
{
  m_clock.start();
}

/**
 * @brief Enables or disables step recording (disabled during the ProjectModel ctor closure).
 */
void DataModel::ProjectHistory::setEnabled(const bool enabled)
{
  m_enabled = enabled;
}

/**
 * @brief Marks an undo/redo apply in progress so re-entrant mutations record nothing.
 */
void DataModel::ProjectHistory::setApplying(const bool applying)
{
  SS_ASSERT_LOG(m_applying != applying);
  m_applying = applying;
}

/**
 * @brief Returns true while an undo/redo snapshot is being applied to the document.
 */
bool DataModel::ProjectHistory::applying() const noexcept
{
  return m_applying;
}

/**
 * @brief Returns true when at least one step can be undone.
 */
bool DataModel::ProjectHistory::canUndo() const noexcept
{
  return m_enabled && m_position > 0;
}

/**
 * @brief Returns true when at least one undone step can be replayed.
 */
bool DataModel::ProjectHistory::canRedo() const noexcept
{
  return m_enabled && m_position < m_steps.size();
}

/**
 * @brief Returns the label of the step undo would revert, or an empty string.
 */
QString DataModel::ProjectHistory::undoText() const
{
  if (!canUndo())
    return QString();

  return m_steps[m_position - 1].label;
}

/**
 * @brief Returns the label of the step redo would replay, or an empty string.
 */
QString DataModel::ProjectHistory::redoText() const
{
  if (!canRedo())
    return QString();

  return m_steps[m_position].label;
}

//--------------------------------------------------------------------------------------------------
// Scope & frame stack
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the label/coalesce hint consumed by the next mutating scope.
 */
void DataModel::ProjectHistory::setNextHint(const QString& label, const QString& coalesceKey)
{
  m_hintLabel = label;
  m_hintKey   = coalesceKey;
}

/**
 * @brief Opens a label frame; the outermost frame's label names any step captured beneath it.
 */
void DataModel::ProjectHistory::pushFrame(const QString& label)
{
  if (m_depth == 0)
    m_rootLabel = label;

  ++m_depth;
}

/**
 * @brief Opens a mutating scope and decides whether it must capture, coalesce, or do nothing;
 *        the editor hint key wins over the slot's own key when both exist.
 */
DataModel::ProjectHistory::ScopeAction DataModel::ProjectHistory::enterScope(
  const QString& slotLabel, const QString& slotKey)
{
  ++m_depth;

  const QString hintLabel = m_hintLabel;
  const QString hintKey   = m_hintKey.isEmpty() ? slotKey : m_hintKey;
  m_hintLabel.clear();
  m_hintKey.clear();

  if (!m_enabled || m_applying || m_stackCaptured || m_pendingStaged)
    return ScopeAction::None;

  const bool noRedoTail = m_position == m_steps.size();
  if (!hintKey.isEmpty() && noRedoTail && !m_steps.empty()) {
    const auto& top = m_steps.back();
    if (top.coalesceKey == hintKey && m_clock.elapsed() - top.timestampMs <= kCoalesceWindowMs) {
      m_pendingStaged   = true;
      m_pendingCoalesce = true;
      return ScopeAction::Coalesce;
    }
  }

  if (!m_rootLabel.isEmpty())
    m_pendingLabel = m_rootLabel;
  else if (!hintLabel.isEmpty())
    m_pendingLabel = hintLabel;
  else
    m_pendingLabel = slotLabel;

  m_pendingKey = hintKey;
  return ScopeAction::Capture;
}

/**
 * @brief Stages the pre-mutation snapshot; it becomes a step only when the slot actually
 *        mutates (signalled by the setModified(true) hook calling commitPending()).
 */
void DataModel::ProjectHistory::stageCapture(QByteArray preState)
{
  SS_ASSERT(m_depth > 0, return);
  SS_ASSERT(!m_stackCaptured, return);
  SS_ASSERT(!m_pendingStaged, return);

  m_pendingStaged   = true;
  m_pendingCoalesce = false;
  m_pendingState    = std::move(preState);
}

/**
 * @brief Turns the staged snapshot into a real step (or extends the coalesced burst); no-op
 *        when nothing is staged. A coalesced step drops any stale redo state, and a save
 *        point discarded with the redo tail becomes unreachable.
 */
void DataModel::ProjectHistory::commitPending()
{
  if (!m_pendingStaged)
    return;

  m_pendingStaged = false;
  m_stackCaptured = true;

  if (m_pendingCoalesce) {
    m_pendingCoalesce = false;
    SS_ASSERT(!m_steps.empty(), return);
    auto& top = m_steps.back();
    if (!top.postState.isEmpty()) {
      m_totalBytes  -= top.postState.size();
      top.postState  = QByteArray();
    }

    top.timestampMs = m_clock.elapsed();
    return;
  }

  if (m_savePosition > static_cast<qint64>(m_position))
    m_savePosition = kSaveUnreachable;

  while (m_steps.size() > m_position) {
    m_totalBytes -= m_steps.back().preState.size() + m_steps.back().postState.size();
    m_steps.pop_back();
  }

  Step step;
  step.label        = m_pendingLabel;
  step.coalesceKey  = m_pendingKey;
  step.preState     = std::move(m_pendingState);
  step.timestampMs  = m_clock.elapsed();
  m_totalBytes     += step.preState.size();
  m_steps.push_back(std::move(step));
  m_position = m_steps.size();

  m_pendingState = QByteArray();
  dropOverflow();
}

/**
 * @brief Closes a scope or frame; returns true when the emptied stack recorded history changes.
 */
bool DataModel::ProjectHistory::leave()
{
  SS_ASSERT(m_depth > 0, return false);
  --m_depth;
  if (m_depth > 0)
    return false;

  const bool changed = m_stackCaptured;
  m_stackCaptured    = false;
  m_pendingStaged    = false;
  m_pendingCoalesce  = false;
  m_pendingState     = QByteArray();
  m_rootLabel.clear();
  return changed;
}

//--------------------------------------------------------------------------------------------------
// Undo/redo state exchange
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the pre-state undo would apply, without moving the position.
 */
QByteArray DataModel::ProjectHistory::peekUndoState() const
{
  SS_ASSERT_LOG(canUndo());
  if (m_position == 0)
    return QByteArray();

  return m_steps[m_position - 1].preState;
}

/**
 * @brief Returns the post-state redo would apply, without moving the position.
 */
QByteArray DataModel::ProjectHistory::peekRedoState() const
{
  SS_ASSERT(canRedo(), return QByteArray());
  return m_steps[m_position].postState;
}

/**
 * @brief Commits a successful undo: stores @p currentState as the step's redo state and steps
 *        the position back. Called only after the snapshot applied cleanly.
 */
void DataModel::ProjectHistory::confirmUndo(const QByteArray& currentState)
{
  SS_ASSERT_LOG(canUndo());
  if (m_position == 0)
    return;

  auto& step = m_steps[m_position - 1];
  if (step.postState.isEmpty()) {
    step.postState  = currentState;
    m_totalBytes   += step.postState.size();
  }

  --m_position;
  dropOverflow();
}

/**
 * @brief Commits a successful redo by stepping the position forward.
 */
void DataModel::ProjectHistory::confirmRedo()
{
  SS_ASSERT(canRedo(), return);
  ++m_position;
}

//--------------------------------------------------------------------------------------------------
// Save point & lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the current position as the on-disk state; undoing past it re-dirties. Also
 *        breaks the top step's coalesce chain so a post-save keystroke starts a fresh step
 *        (keeps the save position byte-accurate).
 */
void DataModel::ProjectHistory::markSaved()
{
  m_savePosition = static_cast<qint64>(m_position);
  if (!m_steps.empty() && m_position == m_steps.size())
    m_steps.back().coalesceKey.clear();
}

/**
 * @brief Returns true when the current position matches the last saved position.
 */
bool DataModel::ProjectHistory::isAtSavePoint() const noexcept
{
  return m_savePosition != kSaveUnreachable && static_cast<size_t>(m_savePosition) == m_position;
}

/**
 * @brief Drops every step (document load, new project, lock transition); open scopes survive.
 */
void DataModel::ProjectHistory::clear()
{
  m_steps.clear();
  m_position      = 0;
  m_savePosition  = 0;
  m_totalBytes    = 0;
  m_stackCaptured = false;
}

//--------------------------------------------------------------------------------------------------
// Bounds
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enforces the step-count and byte bounds by discarding the oldest undoable steps. The
 *        byte cap is deliberately soft: redo-tail states are never evicted (that would break
 *        the redo chain), so materialized redo states can transiently hold up to ~2x the cap.
 */
void DataModel::ProjectHistory::dropOverflow()
{
  while ((m_steps.size() > static_cast<size_t>(kMaxUndoSteps) || m_totalBytes > kMaxUndoBytes)
         && m_position > 0 && m_steps.size() > 1)
    dropFront();
}

/**
 * @brief Removes the oldest step and shifts the position and save-point bookkeeping.
 */
void DataModel::ProjectHistory::dropFront()
{
  SS_ASSERT(!m_steps.empty(), return);
  SS_ASSERT(m_position > 0, return);

  m_totalBytes -= m_steps.front().preState.size() + m_steps.front().postState.size();
  m_steps.pop_front();
  --m_position;

  if (m_savePosition == 0)
    m_savePosition = kSaveUnreachable;
  else if (m_savePosition > 0)
    --m_savePosition;
}

//--------------------------------------------------------------------------------------------------
// RAII frame & scope
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a label frame on the model's history.
 */
DataModel::ProjectUndoFrame::ProjectUndoFrame(ProjectModel& model, const QString& label)
  : m_model(model)
{
  m_model.history().pushFrame(label);
}

/**
 * @brief Closes the frame and notifies QML/API bindings when history state changed.
 */
DataModel::ProjectUndoFrame::~ProjectUndoFrame()
{
  if (m_model.history().leave())
    Q_EMIT m_model.projectHistoryChanged();
}

/**
 * @brief Opens a mutating scope; the outermost scope captures the pre-mutation snapshot. The
 *        optional @p coalesceKey lets slots fed by per-keystroke editors (parser/control
 *        script code) self-coalesce without an editor-side hint.
 */
DataModel::ProjectUndoScope::ProjectUndoScope(ProjectModel& model,
                                              const QString& label,
                                              const QString& coalesceKey)
  : m_model(model)
{
  auto& history     = m_model.history();
  const auto action = history.enterScope(label, coalesceKey);
  if (action == ProjectHistory::ScopeAction::Capture) {
    const auto json = QJsonDocument(m_model.serializeToJson()).toJson(QJsonDocument::Compact);
    history.stageCapture(json);
  }
}

/**
 * @brief Closes the scope and notifies QML/API bindings when history state changed.
 */
DataModel::ProjectUndoScope::~ProjectUndoScope()
{
  if (m_model.history().leave())
    Q_EMIT m_model.projectHistoryChanged();
}
