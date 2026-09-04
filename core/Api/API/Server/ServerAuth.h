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

#include <QByteArray>
#include <QObject>
#include <QSettings>
#include <QString>

#include "API/Server/DeviceWriteVerdict.h"

namespace API {

/**
 * @brief Credential and consent half of the API server: the token external (non-loopback) clients
 *        must present, and the one-time user consent that gates API-originated device writes. Takes
 *        the server's QSettings by injection so both halves persist into the same store, and holds
 *        no socket state -- the reception machine asks it questions, it never answers a client.
 */
class ServerAuth : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void authTokenChanged();

public:
  explicit ServerAuth(QSettings& settings);
  ServerAuth(ServerAuth&&)                 = delete;
  ServerAuth(const ServerAuth&)            = delete;
  ServerAuth& operator=(ServerAuth&&)      = delete;
  ServerAuth& operator=(const ServerAuth&) = delete;

public:
  [[nodiscard]] QString authToken() const;
  [[nodiscard]] bool setAuthToken(const QString& token);
  [[nodiscard]] bool verifyToken(const QByteArray& provided) const;
  [[nodiscard]] DeviceWriteVerdict authorizeDeviceWrite();
  [[nodiscard]] bool authorizeRemoteCommand(const QString& command);

public slots:
  void ensureAuthToken();
  void regenerateAuthToken();
  void showDeviceWriteConsentPrompt();

private:
  /**
   * @brief Tri-state user consent for API-originated device writes.
   */
  enum class DeviceWriteConsent {
    Unset,
    Granted,
    Denied
  };

  QSettings& m_settings;
  QString m_authToken;
  bool m_consentPromptPosted;
  DeviceWriteConsent m_deviceWriteConsent;
};

}  // namespace API
