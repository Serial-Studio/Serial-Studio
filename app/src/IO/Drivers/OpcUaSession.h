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

#include "IO/AsyncTcpDial.h"
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
  void readFinished(quint32 token,
                    const QList<OpcUaTypes::ReadRow>& rows,
                    OpcUaTypes::StatusCode status);
  void browseFinished(quint32 token,
                      const QString& nodeId,
                      const QList<OpcUaTypes::ReferenceRow>& children,
                      OpcUaTypes::StatusCode status);

public:
  /**
   * @brief How the session presents itself to the server.
   */
  struct Identity {
    QString username;
    QString password;
    QString certificatePath;
    QString privateKeyPath;
    int mode;
    bool allowPlaintextPassword;

    Identity() : mode(0), allowPlaintextPassword(false) {}
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
  [[nodiscard]] int revisedInterval() const noexcept;
  [[nodiscard]] QStringList namespaceArray() const;
  [[nodiscard]] QString securityPolicyUri() const;
  [[nodiscard]] OpcUaTypes::SecurityMode securityMode() const noexcept;
  [[nodiscard]] OpcUaTypes::CertInfo serverCertificate() const;
  [[nodiscard]] OpcUaTypes::TrustFailure trustFailure() const noexcept;

  [[nodiscard]] bool discoverEndpoints(const QString& url);
  [[nodiscard]] bool connectToEndpoint(const OpcUaTypes::Endpoint& endpoint,
                                       const Identity& identity);
  [[nodiscard]] bool subscribe(const QStringList& nodeIds, int publishingIntervalMs);
  [[nodiscard]] bool readValues(const QStringList& nodeIds);
  [[nodiscard]] bool readAttributes(const QStringList& nodeIds,
                                    const QList<OpcUaTypes::NodeAttribute>& attributes,
                                    quint32 token = 0);
  [[nodiscard]] bool browse(const QString& nodeId, const OpcUaTypes::BrowseQuery& query);
  [[nodiscard]] bool modifyPublishingInterval(int intervalMs);

  [[nodiscard]] static QString describeStatus(OpcUaTypes::StatusCode status);

  void close();

  void handleStateChanged(int channelState, int sessionState, OpcUaTypes::StatusCode status);
  void handleEndpoints(const QList<OpcUaTypes::Endpoint>& endpoints, OpcUaTypes::StatusCode status);
  void handleSubscriptionCreated(quint32 subscriptionId,
                                 int revisedIntervalMs,
                                 OpcUaTypes::StatusCode status);
  void handleSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus,
                        OpcUaTypes::StatusCode serviceStatus);
  void handleSubscriptionLost(const QString& reason);
  [[nodiscard]] OpcUaTypes::StatusCode verifyServerCertificate(const QByteArray& certificate);
  void handleValue(const OpcUaTypes::MonitoredValue& value);
  void handleRead(quint32 requestId,
                  const QList<OpcUaTypes::ReadRow>& rows,
                  OpcUaTypes::StatusCode status);
  void handleBrowse(quint32 requestId,
                    const QList<OpcUaTypes::ReferenceRow>& children,
                    OpcUaTypes::StatusCode status);

private slots:
  void pump();
  void onResolveFinished(bool ok, const QString& reason);

private:
  /**
   * @brief What the session is currently trying to do, so a channel that opens knows whether to
   *        ask for endpoints or to hand the dial verdict to the driver.
   */
  enum class Intent : quint8 {
    Idle,
    Discovering,
    Connecting,
  };

  /**
   * @brief Which of the session's own bookkeeping reads a reply belongs to, if any. These are
   *        consumed here rather than published, so the driver never sees a phantom tag.
   */
  enum class InternalRead : quint8 {
    No,
    ReadLimit,
    NamespaceArray,
  };

  /**
   * @brief A read still on the wire. The Read service answers positionally, with no node id on
   *        the reply, so the rows are staged here in request order and filled in on arrival, and
   *        `token` is what routes the reply back to the caller that asked for it.
   */
  struct PendingRead {
    QList<OpcUaTypes::ReadRow> rows;
    InternalRead internalRead;
    bool valueRead;
    quint32 token;

    PendingRead() : internalRead(InternalRead::No), valueRead(false), token(0) {}
  };

  /**
   * @brief A browse still on the wire, with the caller's routing token.
   */
  struct PendingBrowse {
    QString nodeId;
    quint32 token;

    PendingBrowse() : token(0) {}
  };

  void teardown();
  void startPump();
  void applyPumpCadence();
  void startResolution();
  [[nodiscard]] QString dialUrl() const;
  [[nodiscard]] bool ensureClient(const OpcUaTypes::Endpoint& endpoint, const Identity& identity);
  void requestEndpoints();
  void failDial(const QString& reason);
  void applyUsernameIdentity(const Identity& identity);
  [[nodiscard]] bool applyIdentity(const Identity& identity);
  [[nodiscard]] bool applySecurity(const OpcUaTypes::Endpoint& endpoint);
  void createMonitoredItems();
  void requestServerLimits();
  void requestNamespaceArray();
  [[nodiscard]] bool sendRead(const QList<OpcUaTypes::ReadRow>& rows,
                              InternalRead internalRead,
                              bool valueRead,
                              quint32 token);

  bool m_connecting;
  bool m_open;
  bool m_readInFlight;
  int m_stackDepth;
  Intent m_intent;
  int m_readLimit;
  int m_revisedInterval;
  quint32 m_subscriptionId;
  QString m_endpointUrl;
  QString m_lastReason;
  QTimer* m_pump;
  AsyncTcpDial m_resolver;
  UA_Client* m_client;
  OpcUaTypes::SecurityMode m_securityMode;
  OpcUaTypes::TrustFailure m_trustFailure;
  QString m_securityPolicyUri;
  OpcUaTypes::CertInfo m_serverCertificate;
  QList<int> m_monitorTags;
  QList<QString> m_monitoredNodes;
  QStringList m_namespaceArray;
  qsizetype m_pollCursor;
  QHash<quint32, PendingRead> m_readRequests;
  QHash<quint32, PendingBrowse> m_browseRequests;
};

}  // namespace Drivers
}  // namespace IO
