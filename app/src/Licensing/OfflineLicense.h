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
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QSettings>
#include <QString>

#include "Licensing/SimpleCrypt.h"

namespace Licensing {
/**
 * @brief Activates Serial Studio Pro from a signed, machine-bound offline
 *        certificate, with no network access.
 */
class OfflineLicense : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString machineId
             READ machineId
             CONSTANT)
  Q_PROPERTY(QString activationUrl
             READ activationUrl
             CONSTANT)
  Q_PROPERTY(bool activated
             READ isActivated
             NOTIFY activatedChanged)
  Q_PROPERTY(int daysRemaining
             READ daysRemaining
             NOTIFY activatedChanged)
  Q_PROPERTY(QDateTime expiresAt
             READ expiresAt
             NOTIFY activatedChanged)
  Q_PROPERTY(QString variantName
             READ variantName
             NOTIFY activatedChanged)
  // clang-format on

signals:
  void activatedChanged();

private:
  explicit OfflineLicense();
  OfflineLicense(OfflineLicense&&)                 = delete;
  OfflineLicense(const OfflineLicense&)            = delete;
  OfflineLicense& operator=(OfflineLicense&&)      = delete;
  OfflineLicense& operator=(const OfflineLicense&) = delete;

public:
  [[nodiscard]] static OfflineLicense& instance();

  [[nodiscard]] const QString& machineId() const;
  [[nodiscard]] QString activationUrl() const;
  [[nodiscard]] bool isActivated() const noexcept;
  [[nodiscard]] int daysRemaining() const;
  [[nodiscard]] const QDateTime& expiresAt() const noexcept;
  [[nodiscard]] const QString& variantName() const noexcept;
  [[nodiscard]] const QString& lastError() const noexcept;

public slots:
  void deactivate();
  bool activateFromFile(const QString& path);
  bool exportMachineInfo(const QString& path);
  void openActivationPortal();

private slots:
  void readSettings();
  void writeSettings(const QByteArray& framedCert);
  void clearOfflineLicense();
  void resetActivationState();
  void onOnlineActivationChanged();

private:
  [[nodiscard]] bool applyCertificate(const QByteArray& framedCert, bool persist);

private:
  bool m_activated;
  QString m_variantName;
  QString m_lastError;
  QDateTime m_expiresAt;

  QSettings m_settings;
  SimpleCrypt m_simpleCrypt;
};
}  // namespace Licensing
