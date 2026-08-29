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

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "IO/Drivers/OpcUa/OpcUaFrameAssembler.h"
#include "IO/Drivers/OpcUa/OpcUaTag.h"
#include "IO/Drivers/OpcUaSession.h"
#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

/**
 * @brief The publishing rate the subscription accepts, in milliseconds. The driver clamps the
 *        configured interval to the same range, so a server-revised interval and a user-typed one
 *        can never disagree about what is representable.
 */
inline constexpr int kOpcUaMinIntervalMs = 10;
inline constexpr int kOpcUaMaxIntervalMs = 60000;

/**
 * @brief What the subscription engine needs from the driver that owns it: the live session, the
 *        tag list, the configured interval, and the two driver-side outlets it cannot reach itself
 *        (pipeline publish, console error log). Implemented by the OpcUa facade, which is why the
 *        engine resolves no singleton, owns no session and never touches the dial verdict.
 */
class OpcUaSubscriptionHost {
public:
  OpcUaSubscriptionHost()          = default;
  virtual ~OpcUaSubscriptionHost() = default;

  OpcUaSubscriptionHost(OpcUaSubscriptionHost&&)                 = delete;
  OpcUaSubscriptionHost(const OpcUaSubscriptionHost&)            = delete;
  OpcUaSubscriptionHost& operator=(OpcUaSubscriptionHost&&)      = delete;
  OpcUaSubscriptionHost& operator=(const OpcUaSubscriptionHost&) = delete;

  virtual void publishFrame(QByteArray&& frame, CapturedData::SteadyTimePoint timestamp) = 0;
  virtual void reportDriverError(const QString& title, const QString& detail) const      = 0;

  [[nodiscard]] virtual int publishingInterval() const               = 0;
  [[nodiscard]] virtual OpcUaSession* liveSession() const            = 0;
  [[nodiscard]] virtual const QList<OpcUaTag>& tags() const noexcept = 0;
};

/**
 * @brief The value-acquisition half of the OPC UA driver (spec 0066 R7/R8/R11): one subscription
 *        carrying every tag, the per-item refusal fallback to timed reads, the silence watchdog,
 *        and the publishing tick that hands the delta frame to the pipeline. It owns the value
 *        cache; the dial, session lifetime and single `openFinished` verdict stay with the facade.
 */
class OpcUaSubscriptions : public QObject {
  Q_OBJECT

signals:
  void statusChanged();

public:
  explicit OpcUaSubscriptions(OpcUaSubscriptionHost& host, QObject* parent = nullptr);

  OpcUaSubscriptions(OpcUaSubscriptions&&)                 = delete;
  OpcUaSubscriptions(const OpcUaSubscriptions&)            = delete;
  OpcUaSubscriptions& operator=(OpcUaSubscriptions&&)      = delete;
  OpcUaSubscriptions& operator=(const OpcUaSubscriptions&) = delete;

  void reset();
  void subscribeAll();
  void bindSession(OpcUaSession* session);
  void unbindSession(OpcUaSession* session);
  void applyPublishingInterval(const int interval);

  [[nodiscard]] bool idle() const noexcept { return m_assembler.idle(); }

  [[nodiscard]] bool pollMode() const noexcept { return m_pollMode; }

  [[nodiscard]] bool subscribing() const noexcept { return m_subscribing; }

  [[nodiscard]] int pendingMonitors() const noexcept { return m_pendingMonitors; }

  [[nodiscard]] int revisedInterval() const noexcept { return m_revisedInterval; }

  [[nodiscard]] int refusedTags() const noexcept { return static_cast<int>(m_polledTags.size()); }

  [[nodiscard]] quint64 skippedPolls() const noexcept { return m_skippedPolls; }

  [[nodiscard]] quint64 framesPublished() const noexcept { return m_framesPublished; }

  [[nodiscard]] quint64 valuesReceived() const noexcept { return m_assembler.valuesReceived(); }

  [[nodiscard]] quint64 badStatusCount() const noexcept { return m_assembler.badStatusCount(); }

  [[nodiscard]] quint64 unstampedCount() const noexcept { return m_assembler.unstampedCount(); }

  [[nodiscard]] QStringList badTags() const;

private slots:
  void onPollTick();
  void onFrameTick();
  void onWatchdogTick();
  void onSubscriptionLost(const QString& reason);
  void onValueChanged(const OpcUaTypes::MonitoredValue& value);
  void onSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus);
  void onTypeMismatch(int index, const QString& declared, const QString& actual);
  void onReadFinished(quint32 token,
                      const QList<OpcUaTypes::ReadRow>& rows,
                      OpcUaTypes::StatusCode status);

private:
  void adoptRevisedInterval();
  void issueRead(const QList<int>& tags);
  void enterPollMode(const QString& reason);
  [[nodiscard]] bool sessionOpen() const;

  OpcUaSubscriptionHost& m_host;
  bool m_pollMode;
  bool m_subscribing;
  bool m_readInFlight;
  int m_pendingMonitors;
  int m_failedMonitors;
  int m_revisedInterval;
  quint64 m_skippedPolls;
  quint64 m_framesPublished;
  QTimer* m_watchdog;
  QTimer* m_pollTimer;
  QTimer* m_frameTimer;
  qint64 m_lastNotifyNs;
  QList<int> m_polledTags;
  QHash<QString, int> m_nodeIndex;
  OpcUaFrameAssembler m_assembler;
};

}  // namespace Drivers
}  // namespace IO
