/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <atomic>
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

#include "IO/Drivers/OpcUaWire.h"

class QTimer;

namespace IO {
namespace Drivers {

/**
 * @brief The protocol-independent half of a polled-PLC worker (spec 0075, E8): abort latch, poll
 *        timer, change-latch table, OpcUaWire delta encoder and the three pulled counters. S7 and
 *        EtherNet/IP derive from it and supply only the wire work. Everything runs on the worker's
 *        OWN thread, except requestAbort() and the counter reads, which the GUI thread reaches.
 */
class PolledPlcWorkerBase : public QObject {
  Q_OBJECT

signals:
  void linkLost(const QString& reason);
  void dialFinished(bool ok, const QString& reason);
  void frameReady(const QByteArray& frame, qint64 stampNs);

public:
  explicit PolledPlcWorkerBase();
  ~PolledPlcWorkerBase() override;

  PolledPlcWorkerBase(PolledPlcWorkerBase&&)                 = delete;
  PolledPlcWorkerBase(const PolledPlcWorkerBase&)            = delete;
  PolledPlcWorkerBase& operator=(PolledPlcWorkerBase&&)      = delete;
  PolledPlcWorkerBase& operator=(const PolledPlcWorkerBase&) = delete;

  void requestAbort() noexcept;

  [[nodiscard]] quint64 readsOk() const noexcept;
  [[nodiscard]] quint64 readsFailed() const noexcept;
  [[nodiscard]] quint64 framesPublished() const noexcept;

public slots:
  void beginDial();
  void shutdown();

protected:
  [[nodiscard]] virtual bool connectToPlc() = 0;
  virtual void pollTick()                   = 0;
  virtual void releaseResources()           = 0;

  void stopPolling();
  void startPolling();
  void clearAbort() noexcept;
  void reportFailure(const QString& reason);
  void publishDirtySlots(qint64 stampNs);
  void noteDialError(const QString& reason);
  void countReadsOk(quint64 count) noexcept;
  void countReadsFailed(quint64 count) noexcept;
  void configureChannels(int interval, QVector<OpcUaWire::Type> types);

  [[nodiscard]] bool aborted() const noexcept;
  [[nodiscard]] bool sessionOpen() const noexcept;
  [[nodiscard]] int channelCount() const noexcept;
  [[nodiscard]] int pollIntervalMs() const noexcept;
  [[nodiscard]] bool latchChannel(int index, const QVariant& value);

private slots:
  void onPollTimer();

private:
  bool m_open;
  bool m_reported;
  int m_interval;
  int m_frameSlot;
  QTimer* m_timer;
  QString m_dialError;
  QByteArray m_frames[2];
  QList<bool> m_dirty;
  QList<QVariant> m_values;
  QVector<OpcUaWire::Type> m_types;
  std::atomic<bool> m_abort;
  // code-verify off
  // Counters are pulled from the GUI thread while the poll thread increments them, so they are
  // atomic rather than the plain quint64 of spec 0033. They are DELIBERATELY packed: one poll
  // thread writes all three at poll rate and one reader samples them at 1 Hz, so there is no
  // cross-core write contention to pad against and three cache lines of padding would cost more
  // than the sharing.
  std::atomic<quint64> m_readsOk;
  std::atomic<quint64> m_readsFailed;
  std::atomic<quint64> m_framesPublished;
  // code-verify on
};

}  // namespace Drivers
}  // namespace IO
