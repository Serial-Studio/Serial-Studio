/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>
#include <QString>
#include <QSettings>
#include <QDateTime>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include "Licensing/SimpleCrypt.h"

namespace Licensing
{
/**
 * @class Licensing::LemonSqueezy
 * @brief Handles software activation, validation, and deactivation using the
 *        Lemon Squeezy API.
 *
 * This class manages all license-related operations for Serial Studio,
 * including:
 * - Activating new licenses on a per-device basis
 * - Validating license keys and assigned instances
 * - Deactivating licenses to free up seats
 * - Securely storing encrypted license data locally
 *
 * It communicates directly with the Lemon Squeezy licensing endpoints and
 * ensures that:
 * - Licenses match the expected product and store ID
 * - Activations are tied to a unique machine ID
 * - Only valid and active keys are accepted
 *
 * This class is implemented as a singleton and is fully integrated with Qt's
 * signal/slot system for reactive UI updates. All sensitive data is encrypted
 * using a machine-specific key.
 *
 * This implementation is designed to function correctly even in open-source
 * environments, where security and local enforcement are handled client-side
 * without exposing API keys or backend logic.
 */
class LemonSqueezy : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(int seatLimit
             READ seatLimit
             NOTIFY licenseDataChanged)
  Q_PROPERTY(int seatUsage
             READ seatUsage
             NOTIFY licenseDataChanged)
  Q_PROPERTY(bool isActivated
             READ isActivated
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString appName
             READ appName
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString license
             READ license
             WRITE setLicense
             NOTIFY licenseChanged)
  Q_PROPERTY(bool canActivate
             READ canActivate
             NOTIFY licenseChanged)
  Q_PROPERTY(QString instanceId
             READ instanceId
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString customerName
             READ customerName
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString customerEmail
             READ customerEmail
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString variantName
             READ variantName
             NOTIFY licenseDataChanged)
  Q_PROPERTY(QString instanceName
             READ instanceName
             NOTIFY licenseDataChanged)
  // clang-format on

signals:
  void busyChanged();
  void licenseChanged();
  void activatedChanged();
  void licenseDataChanged();

private:
  explicit LemonSqueezy();
  LemonSqueezy(LemonSqueezy &&) = delete;
  LemonSqueezy(const LemonSqueezy &) = delete;
  LemonSqueezy &operator=(LemonSqueezy &&) = delete;
  LemonSqueezy &operator=(const LemonSqueezy &) = delete;

public:
  [[nodiscard]] static LemonSqueezy &instance();

  [[nodiscard]] bool busy() const;
  [[nodiscard]] int seatLimit() const;
  [[nodiscard]] int seatUsage() const;
  [[nodiscard]] bool isActivated() const;
  [[nodiscard]] bool canActivate() const;
  [[nodiscard]] const QString &appName() const;
  [[nodiscard]] const QString &license() const;
  [[nodiscard]] const QString &instanceId() const;
  [[nodiscard]] const QString &variantName() const;
  [[nodiscard]] const QString &instanceName() const;
  [[nodiscard]] const QString &customerName() const;
  [[nodiscard]] const QString &customerEmail() const;
  [[nodiscard]] const QJsonObject &licensingData() const;

public slots:
  void activate();
  void validate();
  void deactivate();
  void openCustomerPortal();
  void setLicense(const QString &license);

private slots:
  void readSettings();
  void writeSettings();
  void clearLicenseCache(const bool clearLicense = false);

private:
  void readValidationResponse(const QByteArray &data);
  void readActivationResponse(const QByteArray &data);
  void readDeactivationResponse(const QByteArray &data);

private:
  bool m_busy;
  int m_seatLimit;
  int m_seatUsage;
  bool m_activated;
  QString m_appName;
  QString m_license;
  QString m_instanceId;
  QString m_variantName;
  QString m_instanceName;
  QString m_customerName;
  QString m_customerEmail;
  bool m_silentValidation;
  QDateTime m_activationDate;

  QSettings m_settings;
  SimpleCrypt m_simpleCrypt;
  QJsonObject m_licensingData;
  QNetworkAccessManager m_manager;
};
} // namespace Licensing
