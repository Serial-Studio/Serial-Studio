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

#include "API/Server/ServerAuth.h"

#include "API/Server/AuthPrimitives.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores the persisted credential and consent decisions. The environment override exists
 *        for headless runs (CI included), which cannot answer a consent prompt at all.
 */
API::ServerAuth::ServerAuth(QSettings& settings)
  : m_settings(settings)
  , m_deviceWrite(settings,
                  QStringLiteral("API/DeviceWriteConsent"),
                  tr("Allow API device control?"),
                  tr("A program using Serial Studio's local API is requesting to send data to the "
                     "connected device. Allow API clients to write to the device?"),
                  this)
  , m_scriptInstall(settings,
                    QStringLiteral("API/ScriptInstallConsent"),
                    tr("Allow API clients to modify project scripts?"),
                    tr("A program using Serial Studio's local API wants to change this project's "
                       "scripts. Scripts run with the application's full privileges, including "
                       "launching programs. Allow API clients to modify project scripts?"),
                    this)
{
  m_authToken = m_settings.value("API/AuthToken").toString();

  if (qEnvironmentVariableIntValue("SERIAL_STUDIO_API_AUTO_CONSENT") != 0) {
    m_deviceWrite.grant();
    m_scriptInstall.grant();
  }
}

//--------------------------------------------------------------------------------------------------
// Token management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the token external (non-loopback) clients must present to authenticate.
 */
QString API::ServerAuth::authToken() const
{
  return m_authToken;
}

/**
 * @brief Pins a caller-supplied auth token, for provisioning a headless machine from the command
 *        line. Refuses anything that is not at least 32 hex characters rather than quietly
 *        weakening the credential that guards every non-loopback connection.
 */
bool API::ServerAuth::setAuthToken(const QString& token)
{
  const auto normalized = API::Auth::normalizeToken(token);
  if (normalized.isEmpty())
    return false;

  if (m_authToken == normalized)
    return true;

  m_authToken = normalized;
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
  return true;
}

/**
 * @brief Generates and persists the auth token once; a no-op when one already exists.
 */
void API::ServerAuth::ensureAuthToken()
{
  if (!m_authToken.isEmpty())
    return;

  m_authToken = API::Auth::generateToken();
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
}

/**
 * @brief Issues a fresh auth token; already-authenticated sessions stay connected.
 */
void API::ServerAuth::regenerateAuthToken()
{
  m_authToken = API::Auth::generateToken();
  m_settings.setValue("API/AuthToken", m_authToken);
  Q_EMIT authTokenChanged();
}

/**
 * @brief Constant-time check of a client-provided token against the configured one.
 */
bool API::ServerAuth::verifyToken(const QByteArray& provided) const
{
  return !m_authToken.isEmpty() && API::Auth::constantTimeEquals(provided, m_authToken.toUtf8());
}

//--------------------------------------------------------------------------------------------------
// Consent gates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Answers whether an API device write may proceed, never blocking: an unanswered consent
 *        posts the prompt and refuses with ConsentRequired (spec 0075 I1).
 */
API::DeviceWriteVerdict API::ServerAuth::authorizeDeviceWrite()
{
  return m_deviceWrite.authorize();
}

/**
 * @brief Turns a gate's verdict into the refusal a remote client gets, or nothing when allowed.
 *        CONSENT_REQUIRED is retryable: the prompt was posted, the client re-sends once answered.
 */
static std::optional<API::CommandResponse> refusalFor(const API::ConsentVerdict verdict,
                                                      const QString& id,
                                                      const QString& subject)
{
  if (verdict == API::ConsentVerdict::Allowed)
    return std::nullopt;

  if (verdict == API::ConsentVerdict::ConsentRequired)
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::ConsentRequired,
      QStringLiteral("%1 need the user's consent; a prompt was shown, retry after it is answered")
        .arg(subject));

  return API::CommandResponse::makeError(
    id, API::ErrorCode::ExecutionError, QStringLiteral("%1 denied by the user").arg(subject));
}

/**
 * @brief The ONE policy for a remote-origin command, outer or nested: control-script-only commands
 *        are refused outright, script installs and device writes each clear their consent gate,
 *        everything else passes. Returns the refusal to send, or nothing when the command may run.
 */
std::optional<API::CommandResponse> API::ServerAuth::authorizeRemoteCommand(
  const QString& id, const QString& command, const QJsonObject& params)
{
  if (API::Auth::commandIsControlScriptOnly(command))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("%1 is control-script only and not available to API clients").arg(command));

  if (API::Auth::commandInstallsScript(command, params))
    return refusalFor(m_scriptInstall.authorize(), id, QStringLiteral("Script changes"));

  if (API::Auth::commandWritesToDevice(command))
    return refusalFor(m_deviceWrite.authorize(), id, QStringLiteral("Device writes"));

  return std::nullopt;
}
