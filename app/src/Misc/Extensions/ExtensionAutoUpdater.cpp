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

#include "Misc/Extensions/ExtensionAutoUpdater.h"

//--------------------------------------------------------------------------------------------------
// Update policy
//--------------------------------------------------------------------------------------------------

static constexpr int kUpdatePolicyAsk    = 0;
static constexpr int kUpdatePolicyAlways = 1;
static constexpr int kUpdatePolicyNever  = 2;

static const QString kUpdatePolicyKey = QStringLiteral("ExtensionAutoUpdate");

/**
 * @brief Restores the persisted update policy; a store that carries none defaults to asking.
 */
Misc::ExtensionAutoUpdater::ExtensionAutoUpdater(QSettings& settings)
  : m_policy(kUpdatePolicyAsk), m_settings(settings)
{
  m_policy = m_settings.value(kUpdatePolicyKey, kUpdatePolicyAsk).toInt();
}

//--------------------------------------------------------------------------------------------------
// Policy queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether installed extensions are checked for updates at all.
 */
bool Misc::ExtensionAutoUpdater::checkEnabled() const noexcept
{
  return m_policy != kUpdatePolicyNever;
}

/**
 * @brief Returns whether available updates install without asking the user.
 */
bool Misc::ExtensionAutoUpdater::automaticUpdates() const noexcept
{
  return m_policy == kUpdatePolicyAlways;
}

/**
 * @brief Returns whether any queued update is still to be installed.
 */
bool Misc::ExtensionAutoUpdater::hasPending() const noexcept
{
  return !m_queue.isEmpty();
}

/**
 * @brief Returns whether the user refused this extension's update earlier in the session.
 */
bool Misc::ExtensionAutoUpdater::declined(const QString& id) const
{
  return m_declined.contains(id);
}

//--------------------------------------------------------------------------------------------------
// Policy changes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables update checks, returning false when the policy already said so. A
 *        change clears the refusals, so a re-enabled check offers the same updates again.
 */
bool Misc::ExtensionAutoUpdater::setCheckEnabled(const bool enabled)
{
  if (enabled == checkEnabled())
    return false;

  storePolicy(enabled ? kUpdatePolicyAsk : kUpdatePolicyNever);
  m_declined.clear();
  return true;
}

/**
 * @brief Enables or disables silent installation, returning false when the policy already said so
 *        or when checks are off entirely, which leaves nothing to install silently.
 */
bool Misc::ExtensionAutoUpdater::setAutomaticUpdates(const bool enabled)
{
  if (enabled == automaticUpdates())
    return false;

  if (!enabled && !checkEnabled())
    return false;

  storePolicy(enabled ? kUpdatePolicyAlways : kUpdatePolicyAsk);
  m_declined.clear();
  return true;
}

/**
 * @brief Remembers the user's "always update" answer for later runs.
 */
void Misc::ExtensionAutoUpdater::rememberAlways()
{
  storePolicy(kUpdatePolicyAlways);
}

//--------------------------------------------------------------------------------------------------
// Queue
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes the next queued extension ID, or an empty string when the queue is exhausted.
 */
QString Misc::ExtensionAutoUpdater::takeNext()
{
  if (m_queue.isEmpty())
    return QString();

  return m_queue.takeFirst();
}

/**
 * @brief Queues the extensions the user consented to update.
 */
void Misc::ExtensionAutoUpdater::enqueue(const QStringList& ids)
{
  m_queue = ids;
}

/**
 * @brief Records the extensions the user refused, so the next check does not ask again for them.
 */
void Misc::ExtensionAutoUpdater::decline(const QStringList& ids)
{
  for (const auto& id : ids)
    m_declined.insert(id);
}

/**
 * @brief Applies and persists a new policy value.
 */
void Misc::ExtensionAutoUpdater::storePolicy(const int policy)
{
  m_policy = policy;
  m_settings.setValue(kUpdatePolicyKey, m_policy);
}
