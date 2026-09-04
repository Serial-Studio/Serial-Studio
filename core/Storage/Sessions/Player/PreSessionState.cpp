/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player/PreSessionState.h"

#  include "Core/SSAssert.h"

/**
 * @brief Constructs an empty snapshot: nothing captured, QuickPlot as the neutral mode.
 */
Sessions::PreSessionState::PreSessionState()
  : m_captured(false), m_operationMode(SerialStudio::QuickPlot)
{}

/**
 * @brief Returns whether a snapshot is currently held.
 */
bool Sessions::PreSessionState::captured() const noexcept
{
  return m_captured;
}

/**
 * @brief Returns the dashboard view state captured before playback.
 */
const QString& Sessions::PreSessionState::viewState() const noexcept
{
  return m_viewState;
}

/**
 * @brief Returns the project file path captured before playback.
 */
const QString& Sessions::PreSessionState::projectPath() const noexcept
{
  return m_projectPath;
}

/**
 * @brief Returns the operation mode captured before playback.
 */
SerialStudio::OperationMode Sessions::PreSessionState::operationMode() const noexcept
{
  return m_operationMode;
}

/**
 * @brief Drops the snapshot and returns every field to its neutral value.
 */
void Sessions::PreSessionState::clear()
{
  m_captured = false;
  m_viewState.clear();
  m_projectPath.clear();
  m_operationMode = SerialStudio::QuickPlot;
}

/**
 * @brief Stores the pre-playback state. Ignores repeat calls so the first snapshot wins: a
 *        second capture during playback would record the recording's own state as the one to
 *        restore, stranding the user in the session's project after closing it.
 */
void Sessions::PreSessionState::capture(const SerialStudio::OperationMode mode,
                                        const QString& projectPath,
                                        const QString& viewState)
{
  SS_ASSERT_LOG(!m_captured);
  if (m_captured)
    return;

  m_captured      = true;
  m_viewState     = viewState;
  m_projectPath   = projectPath;
  m_operationMode = mode;
}

#endif
