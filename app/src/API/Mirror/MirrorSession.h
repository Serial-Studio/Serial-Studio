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

#include <chrono>
#include <QJsonObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <vector>

#include "DataModel/Frame.h"

class SessionContext;

namespace API {

class MirrorClient;

/**
 * @brief Viewing-side session half of the remote dashboard mirror (spec 0040): snapshots the local
 *        session before attach, loads the mirrored structure through the ordinary project-load
 *        path, publishes each arriving value snapshot straight into UI::Dashboard::hotpathRxFrame,
 *        and restores the local session on every exit path including an abnormal disconnect.
 */
class MirrorSession : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool attached
             READ attached
             NOTIFY attachedChanged)
  Q_PROPERTY(bool live
             READ live
             NOTIFY statusChanged)
  Q_PROPERTY(bool stale
             READ stale
             NOTIFY statusChanged)
  Q_PROPERTY(int hz
             READ hz
             NOTIFY statusChanged)
  Q_PROPERTY(int datasetCount
             READ datasetCount
             NOTIFY attachedChanged)
  Q_PROPERTY(bool canAttach
             READ canAttach
             NOTIFY canAttachChanged)
  Q_PROPERTY(QString endpoint
             READ endpoint
             NOTIFY attachedChanged)
  Q_PROPERTY(QString remoteVersion
             READ remoteVersion
             NOTIFY attachedChanged)
  Q_PROPERTY(QString lastError
             READ lastError
             NOTIFY lastErrorChanged)
  Q_PROPERTY(QString lastErrorCode
             READ lastErrorCode
             NOTIFY lastErrorChanged)
  Q_PROPERTY(QStringList recentEndpoints
             READ recentEndpoints
             NOTIFY recentEndpointsChanged)
  // clang-format on

signals:
  void statusChanged();
  void attachedChanged();
  void lastErrorChanged();
  void canAttachChanged();
  void recentEndpointsChanged();

public:
  explicit MirrorSession(SessionContext& ctx);
  MirrorSession(MirrorSession&&)                 = delete;
  MirrorSession(const MirrorSession&)            = delete;
  MirrorSession& operator=(MirrorSession&&)      = delete;
  MirrorSession& operator=(const MirrorSession&) = delete;

public:
  [[nodiscard]] static MirrorSession& instance();

  [[nodiscard]] static bool mirroring() noexcept;

  [[nodiscard]] int hz() const;
  [[nodiscard]] bool live() const;
  [[nodiscard]] bool stale() const;
  [[nodiscard]] bool attached() const noexcept;
  [[nodiscard]] bool canAttach() const;
  [[nodiscard]] int datasetCount() const noexcept;
  [[nodiscard]] QString endpoint() const;
  [[nodiscard]] QString remoteVersion() const;
  [[nodiscard]] const QString& lastError() const noexcept;
  [[nodiscard]] const QString& lastErrorCode() const noexcept;
  [[nodiscard]] QStringList recentEndpoints() const;

public slots:
  void detach();
  void refreshCanAttach();
  void forgetEndpoint(const QString& endpoint);
  void attach(const QString& host, const int port, const QString& token, const int hz);

private slots:
  void onLinkStatusChanged();
  void onStructure(const QJsonObject& structure);
  void onSnapshot(const QJsonObject& snapshot);
  void onFailed(const QString& code, const QString& message, const bool fatal);

private:
  /**
   * @brief Positional address of one wire slot inside the per-source template frames.
   */
  struct Slot {
    int frame   = 0;
    int group   = 0;
    int dataset = 0;
  };

  /**
   * @brief Everything attach replaces and detach must put back. Captured before the first write,
   *        so a failed attach restores the same document the user had open.
   */
  struct LocalState {
    bool valid        = false;
    bool frozen       = false;
    bool modified     = false;
    double plotRange  = 10.0;
    int operationMode = 0;
    QString path;
    QJsonObject project;
  };

  void captureLocalState();
  void restoreLocalState();
  void setAttached(const bool value);
  void rememberEndpoint(const QString& address);
  void publishFrames(const QJsonObject& snapshot);
  void assignValues(const QJsonObject& snapshot);
  void setError(const QString& code, const QString& message);
  [[nodiscard]] bool buildTemplates(const QJsonObject& structure);
  [[nodiscard]] bool applyRemoteState(const QJsonObject& structure);
  [[nodiscard]] DataModel::Dataset* datasetAt(const std::size_t index);
  [[nodiscard]] DataModel::Frame buildSourceFrame(const int sourceId) const;

  SessionContext& m_ctx;
  MirrorClient* m_client;

  QSettings m_settings;
  bool m_attached;
  bool m_canAttach;
  bool m_anchored;
  quint64 m_epoch;
  QString m_lastError;
  QString m_lastErrorCode;
  LocalState m_local;

  std::vector<Slot> m_slots;
  std::vector<int> m_tNsIndex;
  std::vector<qint64> m_tNsAnchor;
  std::vector<DataModel::TimestampedFramePtr> m_frames;
  std::chrono::steady_clock::time_point m_localAnchor;
};

}  // namespace API
