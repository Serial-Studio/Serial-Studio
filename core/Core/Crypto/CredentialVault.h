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

#  include "Core/Crypto/SimpleCrypt.h"

namespace MQTT {

/**
 * @brief Username / password pair returned by the vault.
 */
struct Credentials {
  QString username;
  QString password;
};

/**
 * @brief Per-machine OBFUSCATED storage for broker/server credentials keyed by host:port. The
 *        cipher is SimpleCrypt under a machine-derived key, which keeps a credential out of the
 *        settings file as plain text and is not encryption: no user-facing string may call it
 *        that. The scope names the settings group ("mqtt") so another driver can share the vault.
 */
class CredentialVault {
public:
  /**
   * @brief While alive in a sealed session (spec 0096), every vault write lands in an in-memory
   *        overlay and marks its slot package-owned; package-owned slots never reach the settings
   *        file, and reads prefer them. GUI thread; one is open only while a package's included
   *        credentials are applied.
   */
  class TransientScope {
  public:
    TransientScope();
    ~TransientScope();

    TransientScope(TransientScope&&)                 = delete;
    TransientScope(const TransientScope&)            = delete;
    TransientScope& operator=(TransientScope&&)      = delete;
    TransientScope& operator=(const TransientScope&) = delete;
  };

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
  [[nodiscard]] QString readField(const QString& key, const QString& field) const;
  [[nodiscard]] bool writeField(const QString& key, const QString& field, const QString& value);

private:
  QString m_group;
  mutable QSettings m_settings;
  mutable Licensing::SimpleCrypt m_simpleCrypt;
};

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
