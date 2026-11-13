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

#include <functional>

namespace API {

/**
 * @brief Whether this thread's dispatch may wait on the user, and which marshaller granted it
 *        (only the control-script and macro marshallers do; a modal handler reopens it as false so
 *        nested dispatches never inherit it), plus the closer of the one API message box open
 *        (spec 0120), so a stopping script closes only its own box and automation can close any.
 */
class UserWaitScope {
public:
  explicit UserWaitScope(bool allowed, const void* waiter = nullptr) noexcept;
  ~UserWaitScope() noexcept;

  UserWaitScope(UserWaitScope&&)                 = delete;
  UserWaitScope(const UserWaitScope&)            = delete;
  UserWaitScope& operator=(UserWaitScope&&)      = delete;
  UserWaitScope& operator=(const UserWaitScope&) = delete;

public:
  [[nodiscard]] static bool allowed() noexcept;
  [[nodiscard]] static const void* waiter() noexcept;
  [[nodiscard]] static bool boxOpen() noexcept;
  [[nodiscard]] static bool waitingBoxOpen(const void* waiter) noexcept;

  static void registerCloser(std::function<void()> closer, const void* waiter);
  static void clearCloser() noexcept;
  static void cancelOpenBox();
  static void cancelWaitingBox(const void* waiter);

private:
  bool m_previous;
  const void* m_previousWaiter;
};

}  // namespace API
