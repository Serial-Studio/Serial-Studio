/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QElapsedTimer>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <vector>

#include "API/Mirror/MirrorProtocol.h"

class QTcpSocket;
class SessionContext;

namespace API {

/**
 * @brief Capture-side half of the remote dashboard mirror (spec 0040): wakes on
 *        UI::Dashboard::updated(), never on a per-frame path, and is wired to the dashboard
 *        only while a viewer is subscribed, so an unwatched instance runs no mirror code at
 *        all. One snapshot per display tick is fanned out to every due subscriber.
 */
class MirrorPublisher : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void viewersChanged();
  void payloadReady(QTcpSocket* socket, const QString& sessionId, const QByteArray& payload);

public:
  explicit MirrorPublisher(SessionContext& ctx);
  MirrorPublisher(MirrorPublisher&&)                 = delete;
  MirrorPublisher(const MirrorPublisher&)            = delete;
  MirrorPublisher& operator=(MirrorPublisher&&)      = delete;
  MirrorPublisher& operator=(const MirrorPublisher&) = delete;

public:
  [[nodiscard]] static MirrorPublisher& instance();

  [[nodiscard]] int viewers() const noexcept;
  [[nodiscard]] int maxViewers() const;
  [[nodiscard]] bool viewersAllowed() const;
  [[nodiscard]] quint64 epoch() const noexcept;
  [[nodiscard]] bool subscribed(const QString& sessionId) const;
  [[nodiscard]] int effectiveHz(const int hz) const;

  [[nodiscard]] int structureParts();
  [[nodiscard]] QJsonObject info();
  [[nodiscard]] QJsonObject structure();
  [[nodiscard]] QJsonObject structureChunk(const int part);

  [[nodiscard]] bool subscribe(QTcpSocket* socket,
                               const QString& sessionId,
                               const int hz,
                               const int precision);
  [[nodiscard]] bool setRate(const QString& sessionId, const int hz);

public slots:
  void unsubscribe(const QString& sessionId);
  void clearSubscribers();

private slots:
  void onDashboardUpdated();
  void onHeartbeatTimeout();
  void onStructureChanged();

private:
  /**
   * @brief One attached viewer. The socket pointer is only ever used as the delivery target of
   *        a session-id-tagged write, which is what makes a reused pointer harmless.
   */
  struct Subscriber {
    QTcpSocket* socket = nullptr;
    QString sessionId;
    int hz              = Mirror::kHzDefault;
    int precision       = 0;
    quint64 lastEpoch   = 0;
    qint64 lastSnapshot = 0;
  };

  void activate();
  void deactivate();
  void ensureStructure();
  void rebuildStructure();
  void invalidateStructure();
  void publishStructure(Subscriber& subscriber);
  [[nodiscard]] bool collectValues();
  [[nodiscard]] bool due(const Subscriber& subscriber, const qint64 now) const;
  [[nodiscard]] QByteArray snapshotLine(const int precision);

  SessionContext& m_ctx;

  QSettings m_settings;
  quint64 m_epoch;
  quint64 m_seq;
  bool m_structureValid;
  qint64 m_lastSnapshot;
  QString m_layoutHash;
  QElapsedTimer m_clock;
  QTimer m_heartbeat;
  QJsonObject m_structure;

  QMetaObject::Connection m_updatedLink;
  QMetaObject::Connection m_widgetCountLink;
  QMetaObject::Connection m_dataResetLink;

  std::vector<qint64> m_tNs;
  std::vector<int> m_sourceIds;
  std::vector<qint64> m_sourceOrigins;
  std::vector<Mirror::DatasetId> m_datasets;
  std::vector<Mirror::SnapshotValue> m_values;
  std::vector<QByteArray> m_structureLines;

  QHash<int, QByteArray> m_tickLines;
  QHash<QString, Subscriber> m_subscribers;
};

}  // namespace API
