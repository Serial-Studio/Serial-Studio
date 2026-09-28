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

#include "API/Server/ConsentGate.h"

#include <QDebug>
#include <QGuiApplication>

#include "Core/Prompt/UserPrompt.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the gate to its settings key and restores a persisted "yes".
 */
API::ConsentGate::ConsentGate(QSettings& settings,
                              const QString& settingsKey,
                              const QString& title,
                              const QString& question,
                              QObject* parent)
  : QObject(parent)
  , m_settings(settings)
  , m_title(title)
  , m_question(question)
  , m_promptPosted(false)
  , m_state(State::Unset)
{
  rebind(settingsKey);
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether the user already said yes.
 */
bool API::ConsentGate::isGranted() const noexcept
{
  return m_state == State::Granted;
}

/**
 * @brief The settings key the decision persists under; empty for a session-only gate.
 */
const QString& API::ConsentGate::settingsKey() const noexcept
{
  return m_key;
}

/**
 * @brief Grants in memory only, for the headless override: a persisted grant would outlive the
 *        environment that asked for it.
 */
void API::ConsentGate::grant()
{
  if (m_state == State::Granted)
    return;

  m_state = State::Granted;
  Q_EMIT granted();
}

/**
 * @brief Points the gate at another settings key and forgets the current decision, restoring
 *        whatever that key persisted. An empty key keeps the next decision session-only.
 */
void API::ConsentGate::rebind(const QString& settingsKey)
{
  m_key   = settingsKey;
  m_state = State::Unset;
  m_detail.clear();

  if (!m_key.isEmpty() && m_settings.value(m_key, false).toBool())
    m_state = State::Granted;
}

//--------------------------------------------------------------------------------------------------
// Consent
//--------------------------------------------------------------------------------------------------

/**
 * @brief Answers without blocking: an unanswered consent posts the prompt (once) and refuses with
 *        ConsentRequired. @p detail is shown under the question the first time it is asked, so the
 *        user sees what the requester wants before deciding. Headless runs cannot prompt, so an
 *        offscreen platform denies and names the override.
 */
API::ConsentVerdict API::ConsentGate::authorize(const QString& detail)
{
  if (m_state == State::Granted)
    return ConsentVerdict::Allowed;

  if (m_state == State::Denied)
    return ConsentVerdict::Denied;

  if (QGuiApplication::platformName() == QLatin1String("offscreen")) {
    m_state = State::Denied;
    qWarning().noquote() << QStringLiteral("[API] %1 denied: no GUI to prompt for consent. Set "
                                           "SERIAL_STUDIO_API_AUTO_CONSENT=1 to grant it in "
                                           "headless mode.")
                              .arg(m_title);
    return ConsentVerdict::Denied;
  }

  if (!m_promptPosted) {
    m_promptPosted = true;
    m_detail       = detail;
    QMetaObject::invokeMethod(this, &ConsentGate::showPrompt, Qt::QueuedConnection);
  }

  return ConsentVerdict::ConsentRequired;
}

/**
 * @brief Asks the user from the event loop rather than from the requesting path, and records the
 *        answer for every later request. A second prompt is refused: the first one already decided.
 */
void API::ConsentGate::showPrompt()
{
  m_promptPosted = false;
  if (m_state != State::Unset)
    return;

  QString informative = m_question;
  if (!m_detail.isEmpty())
    informative += QStringLiteral("\n\n") + m_detail;

  const auto answer = Core::Prompt::showMessageBox(m_title,
                                                   informative,
                                                   Core::Prompt::Question,
                                                   tr("Serial Studio"),
                                                   Core::Prompt::Yes | Core::Prompt::No,
                                                   Core::Prompt::No);

  if (answer != Core::Prompt::Yes) {
    m_state = State::Denied;
    return;
  }

  m_state = State::Granted;
  if (!m_key.isEmpty())
    m_settings.setValue(m_key, true);

  Q_EMIT granted();
}
