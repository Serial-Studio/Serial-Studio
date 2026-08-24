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

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

#include "IO/Drivers/OpcUaTypes.h"

struct UA_Client;

namespace IO {
namespace Drivers {

/**
 * @brief The OPC UA client the driver talks to (spec 0067): discovery, dial, subscription,
 *        batched reads and browse. Owns the open62541 client and is the only object that sees a
 *        `UA_` type. Affine to its creating thread: a QTimer drives UA_Client_run_iterate(), which
 *        dispatches the stack's callbacks inline, so every signal fires on that one thread.
 */
class OpcUaSession : public QObject {
  Q_OBJECT

signals:
  void connected();
  void disconnected();
  void connectFailed(const QString& reason);
  void endpointsReady(const QList<OpcUaTypes::Endpoint>& endpoints, OpcUaTypes::StatusCode status);
  void subscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus);
  void valueChanged(const OpcUaTypes::MonitoredValue& value);
  void subscriptionLost(const QString& reason);
  void readFinished(const QList<OpcUaTypes::ReadRow>& rows, OpcUaTypes::StatusCode status);
  void browseFinished(const QString& nodeId,
                      const QList<OpcUaTypes::ReferenceRow>& children,
                      OpcUaTypes::StatusCode status);

public:
  /**
   * @brief How the session presents itself to the server.
   */
  struct Identity {
    QString username;
    QString password;
    int mode;

    Identity() : mode(0) {}
  };

  explicit OpcUaSession(QObject* parent = nullptr);
  ~OpcUaSession();

  OpcUaSession(OpcUaSession&&)                 = delete;
  OpcUaSession(const OpcUaSession&)            = delete;
  OpcUaSession& operator=(OpcUaSession&&)      = delete;
  OpcUaSession& operator=(const OpcUaSession&) = delete;

  [[nodiscard]] bool isOpen() const noexcept;
  [[nodiscard]] bool isConnecting() const noexcept;
  [[nodiscard]] int readLimit() const noexcept;

  [[nodiscard]] bool discoverEndpoints(const QString& url);
  [[nodiscard]] bool connectToEndpoint(const QString& url, const Identity& identity);
  [[nodiscard]] bool subscribe(const QStringList& nodeIds, int publishingIntervalMs);
  [[nodiscard]] bool readValues(const QStringList& nodeIds);
  [[nodiscard]] bool readAttributes(const QStringList& nodeIds,
                                    const QList<OpcUaTypes::NodeAttribute>& attributes);
  [[nodiscard]] bool browse(const QString& nodeId);

  void close();

  void handleStateChanged(int channelState, int sessionState, OpcUaTypes::StatusCode status);
  void handleEndpoints(const QList<OpcUaTypes::Endpoint>& endpoints, OpcUaTypes::StatusCode status);
  void handleSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus);
  void handleValue(const OpcUaTypes::MonitoredValue& value);
  void handleRead(const QList<OpcUaTypes::ReadRow>& rows, OpcUaTypes::StatusCode status);
  void handleBrowse(quint32 requestId,
                    const QList<OpcUaTypes::ReferenceRow>& children,
                    OpcUaTypes::StatusCode status);

private slots:
  void pump();

private:
  void teardown();
  void startPump();
  [[nodiscard]] bool ensureClient();
  void requestEndpoints();
  void failDial(const QString& reason);
  void applyUsernameIdentity(const Identity& identity);

  /**
   * @brief What the session is currently trying to do, so a channel that opens knows whether to
   *        ask for endpoints or to hand the dial verdict to the driver.
   */
  enum class Intent : quint8 {
    Idle,
    Discovering,
    Connecting,
  };

  bool m_connecting;
  bool m_open;
  bool m_readInFlight;
  Intent m_intent;
  int m_readLimit;
  quint32 m_subscriptionId;
  QString m_endpointUrl;
  QString m_lastReason;
  QTimer* m_pump;
  UA_Client* m_client;
  QList<QString> m_monitoredNodes;
  QHash<quint32, QString> m_browseRequests;
};

}  // namespace Drivers
}  // namespace IO
