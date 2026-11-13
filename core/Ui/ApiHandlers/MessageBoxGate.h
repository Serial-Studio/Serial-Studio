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

#include <QtGlobal>

namespace API::MessageBoxGate {

/**
 * @brief Why ui.window.showMessageBox must refuse a request, or None when it may proceed.
 */
enum class Refusal : quint8 {
  None,
  WaitNotAllowed,
  HeadlessWait,
  Busy,
};

/**
 * @brief The refusal for one request (spec 0120): a wait outside a granted script dispatch, a
 *        wait nobody can answer, and any request while another API box is open or queued (R1).
 */
[[nodiscard]] constexpr Refusal refusal(bool wait,
                                        bool waitGranted,
                                        bool headless,
                                        bool boxOpen) noexcept
{
  if (wait && !waitGranted)
    return Refusal::WaitNotAllowed;

  if (wait && headless)
    return Refusal::HeadlessWait;

  if (boxOpen)
    return Refusal::Busy;

  return Refusal::None;
}

/**
 * @brief Whether a box nobody waits on goes to the log instead of the screen (R4).
 */
[[nodiscard]] constexpr bool logInsteadOfShow(bool headless) noexcept
{
  return headless;
}

}  // namespace API::MessageBoxGate
