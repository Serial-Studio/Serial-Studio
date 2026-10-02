/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QVariantList>

#include "Core/Bus/Subscription.h"

namespace Console {

class Handler;

/**
 * @brief The console-tier TX command library (spec 0091): persisted send history and
 *        app-global pins. Nothing here enters the project file; project Actions stay the
 *        only durable, project-serialized preset tier.
 */
class SendLibrary : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QStringList history
             READ history
             NOTIFY historyChanged)
  Q_PROPERTY(QVariantList pins
             READ pins
             NOTIFY pinsChanged)
  Q_PROPERTY(bool cyclicArmed
             READ cyclicArmed
             NOTIFY cyclicArmedChanged)
  Q_PROPERTY(int cyclicIntervalMs
             READ cyclicIntervalMs
             WRITE setCyclicIntervalMs
             NOTIFY cyclicIntervalChanged)
  Q_PROPERTY(bool canPromote
             READ canPromote
             NOTIFY canPromoteChanged)
  // clang-format on

signals:
  void pinsChanged();
  void historyChanged();
  void canPromoteChanged();
  void cyclicArmedChanged();
  void cyclicIntervalChanged();

public:
  explicit SendLibrary(Handler& handler);
  SendLibrary(SendLibrary&&)                 = delete;
  SendLibrary(const SendLibrary&)            = delete;
  SendLibrary& operator=(SendLibrary&&)      = delete;
  SendLibrary& operator=(const SendLibrary&) = delete;

  [[nodiscard]] const QStringList& history() const noexcept;
  [[nodiscard]] const QVariantList& pins() const noexcept;
  [[nodiscard]] bool cyclicArmed() const noexcept;
  [[nodiscard]] int cyclicIntervalMs() const noexcept;
  [[nodiscard]] bool canPromote() const;

  [[nodiscard]] Q_INVOKABLE QString recallPin(int index);

  void persistHistory();

public slots:
  void addPin(const QString& title, const QString& payload);
  void removePin(int index);
  void renamePin(int index, const QString& title);
  void movePin(int from, int to);
  void armCyclic(const QString& payload);
  void disarmCyclic();
  void setCyclicIntervalMs(int intervalMs);
  void promotePin(int index);
  void promoteCurrent(const QString& payload);

private slots:
  void onCyclicTick();
  void onConnectionChanged();

private:
  void loadPins();
  void savePins();

private:
  Handler& m_handler;
  QSettings m_settings;
  QVariantList m_pins;
  QTimer m_cyclicTimer;
  QString m_cyclicPayload;
  int m_cyclicIntervalMs;
  Core::Bus::Subscription m_opModeWatch;
};

}  // namespace Console
