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

#include "UI/Widgets/Terminal/TerminalCatchUp.h"

#include "Core/SSAssert.h"

/**
 * @brief Builds a tracker with nothing skipped.
 */
Widgets::TerminalCatchUp::TerminalCatchUp() : m_mark(0), m_pending(false) {}

/**
 * @brief True when output was skipped since the last take() or reset().
 */
bool Widgets::TerminalCatchUp::pending() const noexcept
{
  return m_pending;
}

/**
 * @brief Returns the mark of the first skipped chunk and clears the pending state.
 */
qint64 Widgets::TerminalCatchUp::take() noexcept
{
  SS_ASSERT_LOG(m_pending);
  m_pending = false;
  return m_mark;
}

/**
 * @brief Forgets any skipped output: the view was cleared, so there is nothing to catch up on.
 */
void Widgets::TerminalCatchUp::reset() noexcept
{
  m_pending = false;
}

/**
 * @brief Records a skipped chunk of @p size characters that the console's display mark
 *        @p markAfter already counts; only the first skip of a hidden stretch sets the mark.
 */
void Widgets::TerminalCatchUp::skip(const qint64 markAfter, const qsizetype size) noexcept
{
  SS_ASSERT(size >= 0 && markAfter >= size, return);

  if (!m_pending)
    m_mark = markAfter - size;

  m_pending = true;
}
