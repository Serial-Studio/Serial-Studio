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

#include "API/LaunchConsent.h"

#include <QCryptographicHash>
#include <QDir>

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts unbound (no project yet). The headless override grants it: an operator who set it
 *        opted into the project launching programs.
 */
API::LaunchConsent::LaunchConsent(QSettings& settings, QObject* parent)
  : QObject(parent)
  , m_tainted(false)
  , m_gate(settings,
           QString(),
           tr("Allow this project to launch programs?"),
           tr("This project wants to run programs with your user account's privileges, from its "
              "scripts or from a Process I/O source. Only allow this for projects you trust."),
           this)
  , m_autoConsent(qEnvironmentVariableIntValue("SERIAL_STUDIO_API_AUTO_CONSENT") != 0)
{
  connect(&m_gate, &ConsentGate::granted, this, &LaunchConsent::granted);

  if (m_autoConsent)
    m_gate.grant();
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether the open project may launch programs right now.
 */
bool API::LaunchConsent::isGranted() const noexcept
{
  return m_gate.isGranted();
}

/**
 * @brief Answers a launch request without blocking; @p detail names what wants to run.
 */
API::ConsentVerdict API::LaunchConsent::authorize(const QString& detail)
{
  return m_gate.authorize(detail);
}

/**
 * @brief The settings key a project file's launch consent persists under; empty (session-only)
 *        for a project that has no file.
 */
QString API::LaunchConsent::keyFor(const QString& projectPath)
{
  if (projectPath.isEmpty())
    return QString();

  const auto digest =
    QCryptographicHash::hash(QDir::cleanPath(projectPath).toUtf8(), QCryptographicHash::Sha1);
  return QStringLiteral("API/ScriptExecConsent/") + QString::fromLatin1(digest.toHex());
}

//--------------------------------------------------------------------------------------------------
// Rebinding and R13
//--------------------------------------------------------------------------------------------------

/**
 * @brief A remote client changed what the project runs: clear the open project's consent, once
 *        this session, and remember that the session saw a remote change.
 */
void API::LaunchConsent::noteRemoteChange()
{
  if (m_autoConsent)
    return;

  m_tainted = true;
  clearOnce(keyFor(m_projectPath));
}

/**
 * @brief Points the consent at @p projectPath and restores that file's standing answer. After a
 *        remote change this session, that answer is cleared once first, so a remote edit saved
 *        over an already-trusted file cannot inherit its grant. The operator's command-line
 *        project starts granted in memory otherwise.
 */
void API::LaunchConsent::bindProject(const QString& projectPath)
{
  m_projectPath = projectPath;
  if (m_autoConsent)
    return;

  const auto key = keyFor(projectPath);
  m_gate.rebind(key);
  if (m_tainted) {
    clearOnce(key);
    return;
  }

  if (isOperatorPath(projectPath))
    m_gate.grant();
}

/**
 * @brief Names the project a headless operator passed on the command line; binding it grants the
 *        launch consent for this run, in memory only.
 */
void API::LaunchConsent::setOperatorConsentedPath(const QString& projectPath)
{
  m_operatorPath = projectPath;
  if (!m_autoConsent && !m_tainted && isOperatorPath(m_projectPath))
    m_gate.grant();
}

/**
 * @brief Revokes @p key's answer unless this session already did; a key cleared once and granted
 *        again stays granted for the rest of the session.
 */
void API::LaunchConsent::clearOnce(const QString& key)
{
  if (m_cleared.contains(key))
    return;

  m_cleared.insert(key);
  m_gate.revoke();
}

/**
 * @brief Whether @p projectPath is the operator's command-line project.
 */
bool API::LaunchConsent::isOperatorPath(const QString& projectPath) const
{
  if (projectPath.isEmpty() || m_operatorPath.isEmpty())
    return false;

  return QDir::cleanPath(projectPath) == QDir::cleanPath(m_operatorPath);
}
