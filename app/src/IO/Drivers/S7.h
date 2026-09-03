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
#include <memory>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVector>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUaWire.h"
#include "IO/Drivers/PolledPlcWorkerBase.h"
#include "IO/Drivers/S7/IsoTsap.h"
#include "IO/Drivers/S7/S7Pdu.h"
#include "IO/Drivers/S7Address.h"
#include "IO/HAL_Driver.h"

class QTimer;
class AppState;
class QTcpSocket;

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO {
namespace Drivers {

/**
 * @brief One polled S7 variable: the name the dashboard shows and the absolute address it reads.
 */
struct S7Variable {
  QString name;
  QString address;

  [[nodiscard]] bool operator==(const S7Variable& other) const noexcept
  {
    return name == other.name && address == other.address;
  }
};

/**
 * @brief One resolved read the poll worker issues, flattened out of @ref S7Variable so the worker
 *        never touches the driver's variable list while a poll is in flight.
 */
struct S7ReadItem {
  S7Address::Address address;
  OpcUaWire::Type wireType = OpcUaWire::Type::Invalid;
};

/**
 * @brief Poll worker for the in-house S7comm client (spec 0073). Every exchange with the
 *        controller BLOCKS until it answers, so the socket runs here on the driver's own QThread
 *        and never on the GUI thread. It is created, used and destroyed on this thread, which is
 *        what lets the driver tear it down by quitting the event loop.
 */
class S7PollWorker : public PolledPlcWorkerBase {
  Q_OBJECT

public:
  explicit S7PollWorker();
  ~S7PollWorker() override;

  S7PollWorker(S7PollWorker&&)                 = delete;
  S7PollWorker(const S7PollWorker&)            = delete;
  S7PollWorker& operator=(S7PollWorker&&)      = delete;
  S7PollWorker& operator=(const S7PollWorker&) = delete;

  void configure(const QString& host, int rack, int slot, int interval, QVector<S7ReadItem> items);

  [[nodiscard]] quint64 lastFault() const noexcept;
  [[nodiscard]] quint64 itemErrors() const noexcept;

private:
  [[nodiscard]] bool dial();
  [[nodiscard]] bool negotiate();
  [[nodiscard]] bool sendPdu(const QByteArray& pdu);
  [[nodiscard]] bool pollChunk(const S7Comm::Chunk& chunk);
  [[nodiscard]] bool readTpdu(QByteArray& tpdu, int timeoutMs);
  [[nodiscard]] bool exchange(const QByteArray& request, QByteArray& response);
  [[nodiscard]] bool connectToPlc() override;
  void pollTick() override;
  void releaseResources() override;
  void applyResult(int index, const S7Comm::ReadResult& result, QByteArrayView pdu);

  int m_rack;
  int m_slot;
  QString m_host;
  QTcpSocket* m_socket;
  QByteArray m_rx;
  QByteArray m_response;
  S7Comm::PduCodec m_codec;
  S7Comm::Transport m_transport;
  QVector<S7ReadItem> m_items;
  QList<S7Comm::Chunk> m_chunks;
  QList<S7Comm::ReadItem> m_reads;
  QList<S7Comm::ReadResult> m_results;
  // code-verify off
  // Both counters are pulled from the GUI thread while the poll thread increments them, so they
  // are atomic rather than the plain quint64 of spec 0033. m_lastFault packs the offending
  // variable's index and its return code into one word ((index + 1) << 8 | code, zero for none)
  // so the GUI can name the variable without reading a QString the poll thread is writing.
  std::atomic<quint64> m_lastFault;
  std::atomic<quint64> m_itemErrors;
  // code-verify on
};

/**
 * @brief HAL driver for Siemens S7-300/400/1200/1500 controllers over S7comm (spec 0073). The
 *        driver keeps the configuration and the wire layout; a driver-owned worker thread does the
 *        blocking protocol work and hands back one delta frame per poll tick.
 */
class S7 : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString host
             READ host
             WRITE setHost
             NOTIFY hostChanged)
  Q_PROPERTY(int rack
             READ rack
             WRITE setRack
             NOTIFY rackChanged)
  Q_PROPERTY(int slot
             READ slot
             WRITE setSlot
             NOTIFY slotChanged)
  Q_PROPERTY(int pollInterval
             READ pollInterval
             WRITE setPollInterval
             NOTIFY pollIntervalChanged)
  Q_PROPERTY(int variableCount
             READ variableCount
             NOTIFY variablesChanged)
  Q_PROPERTY(QString statusText
             READ statusText
             NOTIFY statusChanged)
  Q_PROPERTY(bool variablesLocked
             READ variablesLocked
             NOTIFY statusChanged)
  // clang-format on

signals:
  void hostChanged();
  void rackChanged();
  void slotChanged();
  void statusChanged();
  void variablesChanged();
  void pollIntervalChanged();

public:
  explicit S7();
  ~S7();

  S7(S7&&)                 = delete;
  S7(const S7&)            = delete;
  S7& operator=(S7&&)      = delete;
  S7& operator=(const S7&) = delete;

  void close() override;
  void setPersistent(const bool persistent) noexcept;
  void setSessionPeer(S7* peer);

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QString host() const;
  [[nodiscard]] int rack() const;
  [[nodiscard]] int slot() const;
  [[nodiscard]] int pollInterval() const;
  [[nodiscard]] int variableCount() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] bool variablesLocked() const noexcept;

  [[nodiscard]] const QVector<S7Variable>& variables() const noexcept;
  [[nodiscard]] QJsonArray variablesJson() const;
  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject statusJson() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] DataModel::ProjectModel* loadGeneratedProject();

  [[nodiscard]] Q_INVOKABLE QString variableInfo(const int index) const;
  [[nodiscard]] Q_INVOKABLE QString validateAddress(const QString& address) const;

public slots:
  void generateProject();
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setHost(const QString& host);
  void setRack(const int rack);
  void setSlot(const int slot);
  void setPollInterval(const int interval);
  void setVariables(const QJsonArray& variables);
  void addVariable(const QString& name, const QString& address);
  void removeVariable(const int index);
  void clearVariables();

private slots:
  void onFrameReady(const QByteArray& frame, qint64 stampNs);
  void onLinkLost(const QString& reason);
  void onDialFinished(bool ok, const QString& reason);

private:
  void doClose();
  void loadSettings();
  void saveVariables();
  void refreshItems();
  [[nodiscard]] QString itemFaultText() const;
  [[nodiscard]] bool variablesFrozen() const noexcept;
  [[nodiscard]] const S7* sessionPeer() const;
  [[nodiscard]] static OpcUaWire::Type wireTypeFor(S7Address::Type type) noexcept;
  [[nodiscard]] static DataModel::Dataset datasetFor(const S7Variable& variable,
                                                     S7Address::Type type,
                                                     int index);

  AppState& m_appState;
  DataModel::ProjectModel& m_projectModel;

  bool m_open;
  bool m_connecting;
  bool m_persistent;
  int m_rack;
  int m_slot;
  int m_pollInterval;
  qint64 m_lastStampNs;
  quint64 m_linkDrops;
  QString m_host;
  QString m_lastError;
  std::unique_ptr<QThread> m_thread;
  S7PollWorker* m_worker;
  QPointer<S7> m_sessionPeer;
  QVector<S7Variable> m_variables;
  QVector<S7ReadItem> m_items;
  QList<QMetaObject::Connection> m_workerLinks;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO
