/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QSettings>
#  include <QString>

#  include "Licensing/SimpleCrypt.h"

namespace MQTT {

/**
 * @brief Username / password pair returned by the vault.
 */
struct Credentials {
  QString username;
  QString password;
};

/**
 * @brief Per-machine encrypted storage for broker/server credentials keyed by host:port. The
 *        scope names the settings group ("mqtt" by default) so another driver can share the
 *        vault without its entries colliding with the MQTT ones.
 */
class CredentialVault {
public:
  explicit CredentialVault(const QString& scope = QStringLiteral("mqtt"));

  [[nodiscard]] Credentials credentials(const QString& host, quint16 port) const;
  [[nodiscard]] bool hasCredentials(const QString& host, quint16 port) const;
  [[nodiscard]] QString keyPassphrase(const QString& host, quint16 port) const;

  void setCredentials(const QString& host,
                      quint16 port,
                      const QString& username,
                      const QString& password);
  void setKeyPassphrase(const QString& host, quint16 port, const QString& passphrase);
  void clear(const QString& host, quint16 port);

private:
  [[nodiscard]] static QString settingsKey(const QString& host, quint16 port);

private:
  QString m_group;
  mutable QSettings m_settings;
  mutable Licensing::SimpleCrypt m_simpleCrypt;
};

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
