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

#include <atomic>
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTcpSocket>

#include "DataModel/DataBlock.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

namespace API {

/**
 * @brief Worker that handles JSON serialization and socket I/O on a background thread.
 */
class ServerWorker : public DataModel::FrameConsumerWorker<DataModel::DataBlockPtr> {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void clientCountChanged(int count);
  void socketRemoved(QTcpSocket* socket, const QString& sessionId);
  void streamWriteDone(QTcpSocket* socket, const QString& sessionId);
  void dataReceived(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);

public:
  ServerWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
               std::atomic<bool>* enabled,
               std::atomic<size_t>* queueSize);
  ~ServerWorker() override;

  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void closeResources() override;
  void removeSocket(QTcpSocket* socket);
  void writeRawData(const QByteArray& data);
  void broadcastEvent(const QJsonObject& event);
  void addSocket(QTcpSocket* socket, const QString& sessionId);
  void disconnectSocket(QTcpSocket* socket, const QString& sessionId);
  void writeToSocket(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void writeMirrorPayload(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void writeStreamBlock(QTcpSocket* socket, const QString& sessionId, const QByteArray& data);
  void setTemplateFrame(int sourceId, const DataModel::Frame& frame);
  void setSocketStreamFrames(QTcpSocket* socket, const QString& sessionId, const bool enabled);

protected:
  void processItems(const std::vector<DataModel::DataBlockPtr>& items) override;

private:
  std::map<int, DataModel::FrameTemplate> m_templates;

private slots:
  void onSocketReadyRead();
  void onSocketDisconnected();

private:
  [[nodiscard]] bool underWriteCap(QTcpSocket* socket);

private:
  QHash<QTcpSocket*, QString> m_sockets;

  // Sockets whose client opted out of the per-frame broadcast; empty for every ordinary client
  QSet<QTcpSocket*> m_mutedSockets;

  // Sockets already reported as over-cap; per socket so a later client's stall is not swallowed
  QSet<QTcpSocket*> m_warnedSockets;

  // One broadcast document carries at most this many samples; a dense block can exceed it
  static constexpr int kMaxBroadcastSamples = 4096;

  quint64 m_droppedBroadcasts;
};

}  // namespace API
