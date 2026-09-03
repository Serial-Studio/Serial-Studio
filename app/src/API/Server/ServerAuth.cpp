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

#include <QDebug>
#include <QGuiApplication>

#include "API/Server/AuthPrimitives.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores the persisted credential and consent decision. The environment override exists
 *        for headless runs (CI included), which cannot answer the consent prompt at all.
 */
API::ServerAuth::ServerAuth(QSettings& settings)
  : m_settings(settings)
  , m_consentPromptPosted(false)
  , m_deviceWriteConsent(DeviceWriteConsent::Unset)
{
  m_authToken = m_settings.value("API/AuthToken").toString();

  if (m_settings.value("API/DeviceWriteConsent", false).toBool())
    m_deviceWriteConsent = DeviceWriteConsent::Granted;

  if (qEnvironmentVariableIntValue("SERIAL_STUDIO_API_AUTO_CONSENT") != 0)
    m_deviceWriteConsent = DeviceWriteConsent::Granted;
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
// Device-write consent
//--------------------------------------------------------------------------------------------------

/**
 * @brief Answers whether an API device write may proceed, never blocking: an unanswered consent
 *        posts the prompt and refuses with ConsentRequired, because the modal used to run inside
 *        the receive loop whose connection state it could outlive (spec 0075 I1). Headless runs
 *        cannot prompt, so consent is pre-granted through SERIAL_STUDIO_API_AUTO_CONSENT.
 */
API::DeviceWriteVerdict API::ServerAuth::authorizeDeviceWrite()
{
  if (m_deviceWriteConsent == DeviceWriteConsent::Granted)
    return DeviceWriteVerdict::Allowed;

  if (m_deviceWriteConsent == DeviceWriteConsent::Denied)
    return DeviceWriteVerdict::Denied;

  if (qApp->platformName() == QLatin1String("offscreen")) {
    m_deviceWriteConsent = DeviceWriteConsent::Denied;
    qWarning() << "[API] Device write denied: no GUI to prompt for consent. Set "
                  "SERIAL_STUDIO_API_AUTO_CONSENT=1 to allow API device writes in headless mode.";
    return DeviceWriteVerdict::Denied;
  }

  if (!m_consentPromptPosted) {
    m_consentPromptPosted = true;
    QMetaObject::invokeMethod(this, "showDeviceWriteConsentPrompt", Qt::QueuedConnection);
  }

  return DeviceWriteVerdict::ConsentRequired;
}

/**
 * @brief Asks the user, from the event loop rather than from the receive path, and records the
 *        answer for every later write. A second prompt is refused: the first one already decided.
 */
void API::ServerAuth::showDeviceWriteConsentPrompt()
{
  if (m_deviceWriteConsent != DeviceWriteConsent::Unset) {
    m_consentPromptPosted = false;
    return;
  }

  const auto answer = Misc::Utilities::showMessageBox(
    tr("Allow API device control?"),
    tr("A program using Serial Studio's local API is requesting to send data to the connected "
       "device. Allow API clients to write to the device?"),
    QMessageBox::Question,
    tr("Serial Studio"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer == QMessageBox::Yes) {
    m_deviceWriteConsent = DeviceWriteConsent::Granted;
    m_settings.setValue("API/DeviceWriteConsent", true);
    m_consentPromptPosted = false;
    return;
  }

  m_deviceWriteConsent  = DeviceWriteConsent::Denied;
  m_consentPromptPosted = false;
}

/**
 * @brief Gates remote-origin device-write commands behind the consent prompt; commands that
 *        never touch the hardware always pass. Keeps the command path consistent with the
 *        raw byte paths, which run the same gate: an unanswered consent refuses this command
 *        and the client retries once the posted prompt is answered.
 */
bool API::ServerAuth::authorizeRemoteCommand(const QString& command)
{
  if (API::Auth::commandIsControlScriptOnly(command))
    return false;

  if (!API::Auth::commandWritesToDevice(command))
    return true;

  return authorizeDeviceWrite() == DeviceWriteVerdict::Allowed;
}
