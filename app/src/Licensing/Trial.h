/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <QObject>
#include <QDateTime>
#include <QSettings>
#include <QNetworkAccessManager>

#include "SimpleCrypt.h"

namespace Licensing
{
/**
 * @class Licensing::Trial
 * @brief Handles activation and validation of a time-limited trial license.
 *
 * This singleton class manages trial license functionality, including:
 * - Secure storage of trial activation state and expiration.
 * - Requesting and validating trial status from the licensing backend.
 * - Ensuring trials are only activated once per machine, based on MachineID.
 *
 * The trial is optional and must be explicitly enabled by the user.
 * If a commercial license is already activated, the trial becomes unavailable.
 *
 * Encrypted local settings are stored with integrity checks using SimpleCrypt.
 *
 * Backend communication is handled via HTTPS and emits `trialEnabledChanged()`
 * upon updates.
 *
 * Example use cases:
 * - Show trial remaining time via `daysRemaining()`
 * - Trigger trial activation via `enableTrial()`
 * - Check trial eligibility via `trialAvailable()`
 */
class Trial : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(bool firstRun READ firstRun NOTIFY enabledChanged)
  Q_PROPERTY(bool trialEnabled READ trialEnabled NOTIFY enabledChanged)
  Q_PROPERTY(bool trialExpired READ trialExpired NOTIFY enabledChanged)
  Q_PROPERTY(int daysRemaining READ daysRemaining NOTIFY enabledChanged)
  Q_PROPERTY(bool trialAvailable READ trialAvailable NOTIFY availableChanged)

private:
  explicit Trial();
  Trial(Trial &&) = delete;
  Trial(const Trial &) = delete;
  Trial &operator=(Trial &&) = delete;
  Trial &operator=(const Trial &) = delete;

signals:
  void busyChanged();
  void enabledChanged();
  void availableChanged();

public:
  [[nodiscard]] static Trial &instance();

  [[nodiscard]] bool busy() const;
  [[nodiscard]] bool firstRun() const;
  [[nodiscard]] bool trialEnabled() const;
  [[nodiscard]] bool trialExpired() const;
  [[nodiscard]] bool trialAvailable() const;

  [[nodiscard]] int daysRemaining() const;

public slots:
  void enableTrial();
  void readSettings();
  void writeSettings();
  void fetchTrialState();

private slots:
  void onServerReply(QNetworkReply *reply);

private:
  bool m_busy;
  bool m_trialEnabled;
  bool m_deviceRegistered;

  SimpleCrypt m_crypt;
  QSettings m_settings;
  QDateTime m_trialExpiry;
  QNetworkAccessManager m_manager;
};
} // namespace Licensing
