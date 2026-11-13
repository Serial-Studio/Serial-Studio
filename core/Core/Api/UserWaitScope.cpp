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

#include "Core/Api/UserWaitScope.h"

#include <QCoreApplication>
#include <QThread>
#include <utility>

#include "Core/SSAssert.h"

static thread_local bool s_userWaitAllowed        = false;
static thread_local const void* s_userWaitGranter = nullptr;

/**
 * @brief Closes the open API message box; GUI thread only, so it needs no lock. Never destroyed:
 *        ControlScript's function-static destructor asks waitingBoxOpen() during static teardown.
 */
static std::function<void()>& openBoxCloser()
{
  static auto* closer = new std::function<void()>();
  return *closer;
}

/**
 * @brief The marshaller whose script waits on the open API box, or null when none does (a box an
 *        API client posted); GUI thread only.
 */
static const void*& openBoxWaiter()
{
  static const void* waiter = nullptr;
  return waiter;
}

/**
 * @brief Whether the caller runs on the GUI thread, or before any application exists.
 */
static bool onGuiThread()
{
  return qApp == nullptr || QThread::currentThread() == qApp->thread();
}

/**
 * @brief Sets this thread's permission to @p allowed, granted by @p waiter (the marshaller whose
 *        script would wait), remembering the state it replaces.
 */
API::UserWaitScope::UserWaitScope(bool allowed, const void* waiter) noexcept
  : m_previous(s_userWaitAllowed), m_previousWaiter(s_userWaitGranter)
{
  s_userWaitAllowed = allowed;
  s_userWaitGranter = allowed ? waiter : nullptr;
}

/**
 * @brief Restores the permission that was in force when the scope opened.
 */
API::UserWaitScope::~UserWaitScope() noexcept
{
  s_userWaitAllowed = m_previous;
  s_userWaitGranter = m_previousWaiter;
}

/**
 * @brief Whether the command now dispatching on this thread may block on the user.
 */
bool API::UserWaitScope::allowed() noexcept
{
  return s_userWaitAllowed;
}

/**
 * @brief The marshaller that granted this thread's wait, or null when no wait is granted.
 */
const void* API::UserWaitScope::waiter() noexcept
{
  return s_userWaitGranter;
}

/**
 * @brief Whether an API or script caller has a message box open or queued.
 */
bool API::UserWaitScope::boxOpen() noexcept
{
  return static_cast<bool>(openBoxCloser());
}

/**
 * @brief Whether @p waiter's script waits on the open API box. GUI-thread code running while it is
 *        open runs inside its modal loop, which is what that script's teardown has to know.
 */
bool API::UserWaitScope::waitingBoxOpen(const void* waiter) noexcept
{
  return waiter != nullptr && boxOpen() && openBoxWaiter() == waiter;
}

/**
 * @brief Records how to close the box an API caller is about to show, and which marshaller's
 *        script waits on it (null for none); one at a time (R1).
 */
void API::UserWaitScope::registerCloser(std::function<void()> closer, const void* waiter)
{
  SS_ASSERT(onGuiThread(), return);
  SS_ASSERT_LOG(!openBoxCloser());
  openBoxCloser() = std::move(closer);
  openBoxWaiter() = waiter;
}

/**
 * @brief Forgets the closer once its box has returned from its modal loop.
 */
void API::UserWaitScope::clearCloser() noexcept
{
  SS_ASSERT_LOG(onGuiThread());
  openBoxCloser() = nullptr;
  openBoxWaiter() = nullptr;
}

/**
 * @brief Closes the open API box, if any: a stopping script or an automation Escape. The box's
 *        modal loop returns on its next pass, so the closer only asks; it never waits.
 */
void API::UserWaitScope::cancelOpenBox()
{
  SS_ASSERT(onGuiThread(), return);

  const auto closer = openBoxCloser();
  if (closer)
    closer();
}

/**
 * @brief Closes the open API box only when @p waiter's script waits on it: a stopping script
 *        releases its own wait and leaves another script's box, or one an API client posted.
 */
void API::UserWaitScope::cancelWaitingBox(const void* waiter)
{
  SS_ASSERT(onGuiThread(), return);

  if (waitingBoxOpen(waiter))
    cancelOpenBox();
}
