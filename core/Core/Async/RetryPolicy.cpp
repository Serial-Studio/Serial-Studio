/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "Core/Async/RetryPolicy.h"

#include <QtGlobal>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants: the whole retry schedule of the application lives here and nowhere else
//--------------------------------------------------------------------------------------------------

static constexpr int kBackoffStepCap = 24;

static constexpr int kConnectAttempts    = 5;
static constexpr int kConnectInitialMsec = 300;
static constexpr int kConnectCeilingMsec = 300;
static constexpr double kConnectGrowth   = 1.0;

static constexpr int kReconnectAttempts    = 60;
static constexpr int kReconnectInitialMsec = 500;
static constexpr int kReconnectCeilingMsec = 5000;
static constexpr double kReconnectGrowth   = 2.0;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a policy that never retries.
 */
Async::RetryPolicy::RetryPolicy()
  : m_multiplier(1.0), m_maxAttempts(1), m_maxDelayMsec(0), m_initialDelayMsec(0)
{}

/**
 * @brief Constructs a policy from an attempt cap and a geometric backoff schedule.
 */
Async::RetryPolicy::RetryPolicy(int max_attempts,
                                int initial_delay_msec,
                                int max_delay_msec,
                                double multiplier)
  : m_multiplier(multiplier)
  , m_maxAttempts(max_attempts)
  , m_maxDelayMsec(max_delay_msec)
  , m_initialDelayMsec(initial_delay_msec)
{
  SS_ASSERT(max_attempts >= 1, m_maxAttempts = 1);
  SS_ASSERT(initial_delay_msec >= 0, m_initialDelayMsec = 0);
  SS_ASSERT(max_delay_msec >= initial_delay_msec, m_maxDelayMsec = m_initialDelayMsec);
  SS_ASSERT(multiplier >= 1.0, m_multiplier = 1.0);
}

//--------------------------------------------------------------------------------------------------
// Named policies
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the policy applied when a link that was open drops on its own.
 */
Async::RetryPolicy Async::RetryPolicy::autoReconnect()
{
  return RetryPolicy(
    kReconnectAttempts, kReconnectInitialMsec, kReconnectCeilingMsec, kReconnectGrowth);
}

/**
 * @brief Returns the policy applied to a connect the user explicitly asked for. The schedule is
 *        flat rather than geometric: the user waits behind a wait cursor, so the retries must fit
 *        the blocking budget the pre-async connect had instead of growing past it.
 */
Async::RetryPolicy Async::RetryPolicy::initialConnect()
{
  return RetryPolicy(kConnectAttempts, kConnectInitialMsec, kConnectCeilingMsec, kConnectGrowth);
}

//--------------------------------------------------------------------------------------------------
// Schedule
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the geometric growth factor between consecutive attempts.
 */
double Async::RetryPolicy::multiplier() const noexcept
{
  return m_multiplier;
}

/**
 * @brief Returns the total number of attempts allowed, the first one included.
 */
int Async::RetryPolicy::maxAttempts() const noexcept
{
  return m_maxAttempts;
}

/**
 * @brief Returns the ceiling the backoff delay never grows past.
 */
int Async::RetryPolicy::maxDelayMsec() const noexcept
{
  return m_maxDelayMsec;
}

/**
 * @brief Returns the delay applied before the second attempt.
 */
int Async::RetryPolicy::initialDelayMsec() const noexcept
{
  return m_initialDelayMsec;
}

/**
 * @brief Returns whether another attempt is allowed after the given number of failures.
 */
bool Async::RetryPolicy::shouldRetry(int completed_attempts) const noexcept
{
  SS_ASSERT(completed_attempts >= 0, return false);
  SS_ASSERT(m_maxAttempts >= 1, return false);

  return completed_attempts >= 1 && completed_attempts < m_maxAttempts;
}

/**
 * @brief Returns the wait applied after the given attempt failed, clamped to the ceiling. The
 *        growth loop is bounded by kBackoffStepCap so the schedule cannot run away on a policy
 *        with a large attempt cap.
 */
int Async::RetryPolicy::delayForAttempt(int completed_attempts) const noexcept
{
  SS_ASSERT(completed_attempts >= 1, return m_initialDelayMsec);
  SS_ASSERT(m_maxDelayMsec >= m_initialDelayMsec, return m_maxDelayMsec);

  const int steps  = qBound(0, completed_attempts - 1, kBackoffStepCap);
  const double top = static_cast<double>(m_maxDelayMsec);
  double delay     = static_cast<double>(m_initialDelayMsec);

  for (int i = 0; i < steps && delay < top; ++i)
    delay *= m_multiplier;

  return qBound(0, static_cast<int>(delay), m_maxDelayMsec);
}
