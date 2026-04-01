/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef ENABLE_GRPC

#  include <atomic>
#  include <grpcpp/grpcpp.h>
#  include <memory>
#  include <mutex>
#  include <QObject>
#  include <thread>
#  include <vector>

#  include "DataModel/Frame.h"
#  include "IO/HAL_Driver.h"
#  include "serialstudio.grpc.pb.h"
#  include "ThirdParty/readerwriterqueue.h"

/**
 * Default gRPC port for the Serial Studio API.
 */
#  define API_GRPC_PORT 8888

class SerialStudioServiceImpl;

namespace API {
namespace GRPC {

/**
 * @brief Context for an active StreamFrames call.
 *
 * Each streaming client gets one of these. The gRPC thread writes frames
 * to the writer, and the main thread enqueues frames via the atomic queue.
 */
struct FrameStreamContext {
  grpc::ServerWriter<serialstudio::FrameBatch>* writer = nullptr;
  grpc::ServerContext* context                         = nullptr;
  std::atomic<bool> cancelled{false};
};

/**
 * @brief Context for an active StreamRawData call.
 */
struct RawStreamContext {
  grpc::ServerWriter<serialstudio::RawBatch>* writer = nullptr;
  grpc::ServerContext* context                       = nullptr;
  std::atomic<bool> cancelled{false};
};

/**
 * @class GRPCServer
 * @brief gRPC server that mirrors the TCP/JSON API on port 8888.
 *
 * Implements the SerialStudioAPI service defined in serialstudio.proto.
 * Uses google.protobuf.Struct for dynamic command parameters, reusing
 * the existing CommandRegistry for all command execution.
 *
 * The server respects the same externalConnections setting as the TCP
 * API server, binding to localhost or all interfaces accordingly.
 *
 * Frame streaming is implemented via server-streaming RPCs. The main
 * thread pushes frames to active stream contexts, which the gRPC
 * threads then write to their respective clients.
 */
class GRPCServer : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool enabled
             READ enabled
             WRITE setEnabled
             NOTIFY enabledChanged)
  Q_PROPERTY(bool grpcAvailable
             READ grpcAvailable
             CONSTANT)
  Q_PROPERTY(int clientCount
             READ clientCount
             NOTIFY clientCountChanged)
  // clang-format on

  friend class ::SerialStudioServiceImpl;

signals:
  void enabledChanged();
  void clientCountChanged();

private:
  explicit GRPCServer();
  GRPCServer(GRPCServer&&)                 = delete;
  GRPCServer(const GRPCServer&)            = delete;
  GRPCServer& operator=(GRPCServer&&)      = delete;
  GRPCServer& operator=(const GRPCServer&) = delete;

public:
  ~GRPCServer();

  [[nodiscard]] static GRPCServer& instance();

  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool grpcAvailable() const noexcept;
  [[nodiscard]] int clientCount() const noexcept;

public slots:
  void setEnabled(const bool enabled);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);
  void hotpathTxData(const IO::ByteArrayPtr& data);

  void exportProto(const QString& filePath);

private slots:
  void onExternalConnectionsChanged();

private:
  void startServer();
  void stopServer();
  void writerLoop();

private:
  bool m_enabled;
  std::atomic<int> m_clientCount{0};
  std::atomic<bool> m_writerRunning{false};

  std::unique_ptr<grpc::Server> m_grpcServer;
  std::unique_ptr<SerialStudioServiceImpl> m_service;
  std::thread m_serverThread;
  std::thread m_writerThread;

  moodycamel::ReaderWriterQueue<DataModel::TimestampedFramePtr> m_frameQueue{4096};
  moodycamel::ReaderWriterQueue<IO::ByteArrayPtr> m_rawQueue{4096};

  std::mutex m_frameStreamsMutex;
  std::vector<std::shared_ptr<FrameStreamContext>> m_frameStreams;

  std::mutex m_rawStreamsMutex;
  std::vector<std::shared_ptr<RawStreamContext>> m_rawStreams;
};

}  // namespace GRPC
}  // namespace API

#endif  // ENABLE_GRPC
