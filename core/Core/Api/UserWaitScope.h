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

namespace API {

/**
 * @brief Whether this thread's command dispatch may wait on the user. Only the control-script
 *        marshaller grants it: a parser or transform on the data path would stall acquisition. A
 *        handler running a modal loop reopens it as false so nested dispatches never inherit it.
 */
class UserWaitScope {
public:
  explicit UserWaitScope(bool allowed) noexcept;
  ~UserWaitScope() noexcept;

  UserWaitScope(UserWaitScope&&)                 = delete;
  UserWaitScope(const UserWaitScope&)            = delete;
  UserWaitScope& operator=(UserWaitScope&&)      = delete;
  UserWaitScope& operator=(const UserWaitScope&) = delete;

public:
  [[nodiscard]] static bool allowed() noexcept;

private:
  bool m_previous;
};

}  // namespace API
