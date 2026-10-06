/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "Core/License.h"

#include <atomic>
#include <QCoreApplication>
#include <QThread>

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

static std::atomic<bool> s_activated{false};
static std::atomic<quint8> s_tier{0};
static std::atomic<bool> s_trialExpired{false};
static std::atomic<int> s_trialDaysRemaining{-1};
static Core::License::ProFeatureHandler s_proFeatureHandler;
static std::function<bool()> s_remoteDispatchProbe;

//--------------------------------------------------------------------------------------------------
// Readers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a valid commercial entitlement (paid or trial) is installed.
 */
bool Core::License::activated() noexcept
{
  return s_activated.load(std::memory_order_relaxed);
}

/**
 * @brief The feature tier the root derived from the installed token (0 when none).
 */
quint8 Core::License::tier() noexcept
{
  return s_tier.load(std::memory_order_relaxed);
}

/**
 * @brief Whether the trial period ended on this machine.
 */
bool Core::License::trialExpired() noexcept
{
  return s_trialExpired.load(std::memory_order_relaxed);
}

/**
 * @brief Days left in the trial on this machine (-1 when no trial is registered).
 */
int Core::License::trialDaysRemaining() noexcept
{
  return s_trialDaysRemaining.load(std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Writer (composition root only)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes the licensing facts; called by the root after the licensing block constructs
 *        and on every activatedChanged transition, never from a library.
 */
void Core::License::set(const bool activated,
                        const quint8 tier,
                        const bool trialExpired,
                        const int trialDaysRemaining) noexcept
{
  s_tier.store(tier, std::memory_order_relaxed);
  s_trialExpired.store(trialExpired, std::memory_order_relaxed);
  s_trialDaysRemaining.store(trialDaysRemaining, std::memory_order_relaxed);
  s_activated.store(activated, std::memory_order_release);
}

/**
 * @brief Installs (or clears) the Pro-intent handler; composition root only, before the QML
 *        load, so requestProFeature() needs no synchronization: both run on the GUI thread.
 */
void Core::License::setProFeatureHandler(ProFeatureHandler handler)
{
  s_proFeatureHandler = std::move(handler);
}

/**
 * @brief Installs the root-bound probe for an active remote dispatch scope; a gate site that
 *        QUEUES its intent consults it at queue time, because the scope unwinds before the
 *        queued lambda runs and the handler's own check can no longer see it.
 */
void Core::License::setRemoteDispatchProbe(std::function<bool()> probe)
{
  s_remoteDispatchProbe = std::move(probe);
}

/**
 * @brief Whether the current call stack runs under a remote dispatch scope (false when no
 *        probe is bound, as in headless and GPL roots).
 */
bool Core::License::remoteDispatchActive()
{
  return s_remoteDispatchProbe && s_remoteDispatchProbe();
}

/**
 * @brief Raises a command-rate Pro-feature intent from a gate site's refusal branch (explicit
 *        user gestures only, never from a per-frame or per-message path); a build or mode
 *        without an installed handler keeps the silent refusal, as does a call from any thread
 *        but the GUI thread, because the handler runs blocking UI.
 */
void Core::License::requestProFeature(const QString& featureId, ProFeatureRetry retry)
{
  if (!s_proFeatureHandler)
    return;

  if (qApp == nullptr || QThread::currentThread() != qApp->thread())
    return;

  s_proFeatureHandler(featureId, std::move(retry));
}
