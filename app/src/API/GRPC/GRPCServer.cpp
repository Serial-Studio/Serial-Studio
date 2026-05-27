/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#ifdef ENABLE_GRPC

// Suppress GCC false positive from deeply-inlined protobuf InternalSwap
#  if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstringop-overflow"
#  endif

#  include "API/GRPC/GRPCServer.h"

#  include <grpcpp/grpcpp.h>
#  include <QCoreApplication>
#  include <QFile>
#  include <QFileDialog>
#  include <QJsonDocument>
#  include <QMetaObject>
#  include <QTimer>

#  include "API/CommandHandler.h"
#  include "API/CommandRegistry.h"
#  include "API/GRPC/ConversionUtils.h"
#  include "API/GRPC/ProtoGenerator.h"
#  include "API/Server.h"
#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Service implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Implements the SerialStudioAPI gRPC service.
 */
class SerialStudioServiceImpl final : public serialstudio::SerialStudioAPI::Service {
public:
  /**
   * @brief Constructs the service implementation bound to the given GRPCServer.
   */
  explicit SerialStudioServiceImpl(API::GRPC::GRPCServer* server) : m_server(server) {}

  /**
   * @brief Executes a single API command.
   */
  grpc::Status ExecuteCommand(grpc::ServerContext* context,
                              const serialstudio::CommandRequest* request,
                              serialstudio::CommandResponse* response) override
  {
    if (!authorize(context))
      return unauthenticated();

    const auto params  = API::GRPC::ConversionUtils::toQJsonObject(request->params());
    const auto id      = QString::fromStdString(request->id());
    const auto command = QString::fromStdString(request->command());

    // Marshal command execution to the main thread
    API::CommandResponse result;
    QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [&]() { result = API::CommandRegistry::instance().execute(command, id, params); },
      Qt::BlockingQueuedConnection);

    response->set_id(request->id());
    response->set_success(result.success);

    if (result.success) {
      *response->mutable_result() = API::GRPC::ConversionUtils::toProtoValue(result.result);
    } else {
      auto* err = response->mutable_error();
      err->set_code(result.errorCode.toStdString());
      err->set_message(result.errorMessage.toStdString());
    }

    return grpc::Status::OK;
  }

  /**
   * @brief Executes multiple commands in a batch.
   */
  grpc::Status ExecuteBatch(grpc::ServerContext* context,
                            const serialstudio::BatchRequest* request,
                            serialstudio::BatchResponse* response) override
  {
    if (!authorize(context))
      return unauthenticated();

    response->set_id(request->id());
    bool all_success = true;

    for (const auto& cmd : request->commands()) {
      serialstudio::CommandResponse cmd_response;
      ExecuteCommand(nullptr, &cmd, &cmd_response);

      if (!cmd_response.success())
        all_success = false;

      *response->add_results() = std::move(cmd_response);
    }

    response->set_success(all_success);
    return grpc::Status::OK;
  }

  /**
   * @brief Streams parsed frames to the client.
   */
  grpc::Status StreamFrames(grpc::ServerContext* context,
                            const serialstudio::StreamRequest* /*request*/,
                            grpc::ServerWriter<serialstudio::FrameBatch>* writer) override
  {
    if (!authorize(context))
      return unauthenticated();

    auto ctx     = std::make_shared<API::GRPC::FrameStreamContext>();
    ctx->writer  = writer;
    ctx->context = context;

    // Register this stream
    {
      std::lock_guard<std::mutex> lock(m_server->m_frameStreamsMutex);
      m_server->m_frameStreams.push_back(ctx);
    }

    m_server->m_clientCount.fetch_add(1, std::memory_order_relaxed);
    QMetaObject::invokeMethod(
      m_server, [this]() { Q_EMIT m_server->clientCountChanged(); }, Qt::QueuedConnection);

    // Block until cancelled
    while (!context->IsCancelled() && !ctx->cancelled.load())
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Unregister this stream
    {
      std::lock_guard<std::mutex> lock(m_server->m_frameStreamsMutex);
      auto& streams = m_server->m_frameStreams;
      streams.erase(std::remove(streams.begin(), streams.end(), ctx), streams.end());
    }

    m_server->m_clientCount.fetch_sub(1, std::memory_order_relaxed);
    QMetaObject::invokeMethod(
      m_server, [this]() { Q_EMIT m_server->clientCountChanged(); }, Qt::QueuedConnection);

    return grpc::Status::OK;
  }

  /**
   * @brief Streams raw device data to the client.
   */
  grpc::Status StreamRawData(grpc::ServerContext* context,
                             const serialstudio::StreamRequest* /*request*/,
                             grpc::ServerWriter<serialstudio::RawBatch>* writer) override
  {
    if (!authorize(context))
      return unauthenticated();

    auto ctx     = std::make_shared<API::GRPC::RawStreamContext>();
    ctx->writer  = writer;
    ctx->context = context;

    {
      std::lock_guard<std::mutex> lock(m_server->m_rawStreamsMutex);
      m_server->m_rawStreams.push_back(ctx);
    }

    m_server->m_clientCount.fetch_add(1, std::memory_order_relaxed);
    QMetaObject::invokeMethod(
      m_server, [this]() { Q_EMIT m_server->clientCountChanged(); }, Qt::QueuedConnection);

    while (!context->IsCancelled() && !ctx->cancelled.load())
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
      std::lock_guard<std::mutex> lock(m_server->m_rawStreamsMutex);
      auto& streams = m_server->m_rawStreams;
      streams.erase(std::remove(streams.begin(), streams.end(), ctx), streams.end());
    }

    m_server->m_clientCount.fetch_sub(1, std::memory_order_relaxed);
    QMetaObject::invokeMethod(
      m_server, [this]() { Q_EMIT m_server->clientCountChanged(); }, Qt::QueuedConnection);

    return grpc::Status::OK;
  }

  /**
   * @brief Sends raw data to the connected device.
   */
  grpc::Status WriteRawData(grpc::ServerContext* context,
                            const serialstudio::RawDataRequest* request,
                            serialstudio::CommandResponse* response) override
  {
    if (!authorize(context))
      return unauthenticated();

    const auto& bytes = request->data();
    QByteArray data(bytes.data(), static_cast<int>(bytes.size()));

    bool denied  = false;
    bool written = false;
    QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [&]() {
        if (!API::Server::instance().authorizeDeviceWrite()) {
          denied = true;
          return;
        }

        written = IO::ConnectionManager::instance().writeData(data) == data.size();
      },
      Qt::BlockingQueuedConnection);

    response->set_id(request->id());
    response->set_success(written);

    if (!written) {
      auto* err = response->mutable_error();
      err->set_code(denied ? "WRITE_DENIED" : "WRITE_FAILED");
      err->set_message(denied ? "Device write denied by user" : "Failed to write data to device");
    }

    return grpc::Status::OK;
  }

  /**
   * @brief Lists all available API commands.
   */
  grpc::Status ListCommands(grpc::ServerContext* context,
                            const google::protobuf::Empty* /*request*/,
                            serialstudio::CommandList* response) override
  {
    if (!authorize(context))
      return unauthenticated();

    const auto& commands = API::CommandRegistry::instance().commands();
    for (auto it = commands.begin(); it != commands.end(); ++it) {
      auto* info = response->add_commands();
      info->set_name(it->name.toStdString());
      info->set_description(it->description.toStdString());

      if (!it->inputSchema.isEmpty())
        *info->mutable_input_schema() = API::GRPC::ConversionUtils::toProtoStruct(it->inputSchema);
    }

    return grpc::Status::OK;
  }

private:
  /**
   * @brief Authorizes a peer: loopback is auto-trusted, external peers must present the token.
   */
  static bool authorize(grpc::ServerContext* context)
  {
    // Internal call (e.g. ExecuteBatch -> ExecuteCommand); already authorized by the outer RPC.
    if (context == nullptr)
      return true;

    // Loopback peers are auto-trusted, mirroring the TCP server's local-client policy.
    const std::string peer = context->peer();
    if (peer.find("127.0.0.1") != std::string::npos || peer.find("[::1]") != std::string::npos
        || peer.rfind("unix:", 0) == 0)
      return true;

    // External peers must present the API token via request metadata.
    const auto& md = context->client_metadata();
    const auto it  = md.find("x-serial-studio-token");
    if (it == md.end())
      return false;

    const QByteArray token(it->second.data(), static_cast<int>(it->second.size()));
    return API::Server::instance().verifyToken(token);
  }

  /**
   * @brief Builds the standard UNAUTHENTICATED status returned to rejected peers.
   */
  static grpc::Status unauthenticated()
  {
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Authentication required");
  }

  API::GRPC::GRPCServer* m_server;
};

//--------------------------------------------------------------------------------------------------
// Constructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the gRPC server singleton.
 */
API::GRPC::GRPCServer::GRPCServer()
  : m_enabled(false), m_clientCount(0), m_writerRunning(false), m_frameQueue(4096), m_rawQueue(4096)
{
  auto& server = API::Server::instance();

  // Mirror the API server enabled state
  connect(&server, &API::Server::enabledChanged, this, [this]() {
    setEnabled(API::Server::instance().enabled());
  });

  // Restart gRPC when external connections setting changes
  connect(&server,
          &API::Server::externalConnectionsChanged,
          this,
          &GRPCServer::onExternalConnectionsChanged);

  // Defer initial sync until the event loop is running
  QTimer::singleShot(0, this, [this]() { setEnabled(API::Server::instance().enabled()); });
}

/**
 * @brief Destroys the gRPC server, stopping it if running.
 */
API::GRPC::GRPCServer::~GRPCServer()
{
  stopServer();
}

/**
 * @brief Returns the singleton instance.
 */
API::GRPC::GRPCServer& API::GRPC::GRPCServer::instance()
{
  static GRPCServer singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the gRPC server is currently enabled.
 */
bool API::GRPC::GRPCServer::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns true -- gRPC support was compiled in.
 */
bool API::GRPC::GRPCServer::grpcAvailable() const noexcept
{
  return true;
}

/**
 * @brief Returns the number of active streaming clients.
 */
int API::GRPC::GRPCServer::clientCount() const noexcept
{
  return m_clientCount.load(std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables the gRPC server.
 */
void API::GRPC::GRPCServer::setEnabled(const bool enabled)
{
  if (m_enabled == enabled)
    return;

  if (enabled)
    startServer();
  else
    stopServer();

  m_enabled = enabled;
  Q_EMIT enabledChanged();
}

/**
 * @brief Enqueues a parsed frame for background writing to gRPC clients (lock-free, hotpath-safe).
 */
void API::GRPC::GRPCServer::hotpathTxFrame(const DataModel::TimestampedFramePtr& frame)
{
  if (!m_enabled)
    return;

  m_frameQueue.try_enqueue(frame);
}

/**
 * @brief Enqueues raw device data for background writing to gRPC clients.
 */
void API::GRPC::GRPCServer::hotpathTxData(const QByteArray& data)
{
  if (!m_enabled || data.isEmpty())
    return;

  m_rawQueue.try_enqueue(data);
}

/**
 * @brief Prompts the user for a destination and writes a typed .proto file.
 */
void API::GRPC::GRPCServer::exportProto()
{
  const auto path = QFileDialog::getSaveFileName(nullptr,
                                                 tr("Export Protobuf File"),
                                                 QStringLiteral("serial_studio.proto"),
                                                 tr("Protocol Buffers (*.proto)"));
  if (path.isEmpty())
    return;

  (void)ProtoGenerator::exportToFile(path);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restarts the gRPC server when external connection settings change.
 */
void API::GRPC::GRPCServer::onExternalConnectionsChanged()
{
  if (!m_enabled)
    return;

  stopServer();
  startServer();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the gRPC server on a background thread.
 */
void API::GRPC::GRPCServer::startServer()
{
  // Determine bind address
  const bool external = API::Server::instance().externalConnections();
  const auto address  = external ? QStringLiteral("0.0.0.0:%1").arg(API_GRPC_PORT)
                                 : QStringLiteral("127.0.0.1:%1").arg(API_GRPC_PORT);

  // Ensure command handlers are initialized
  (void)API::CommandHandler::instance();

  // Build and start the server
  m_service = std::make_unique<SerialStudioServiceImpl>(this);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address.toStdString(), grpc::InsecureServerCredentials());
  builder.RegisterService(m_service.get());

  m_grpcServer = builder.BuildAndStart();
  if (!m_grpcServer) {
    m_service.reset();
    Misc::Utilities::showMessageBox(tr("Unable to start gRPC server"),
                                    tr("Failed to bind to %1").arg(address),
                                    QMessageBox::Warning);
    return;
  }

  // Run Wait() on a dedicated thread so the main thread is not blocked
  m_serverThread = std::thread([this]() { m_grpcServer->Wait(); });

  // Start the background writer thread for frame/raw data serialization
  m_writerRunning.store(true);
  m_writerThread = std::thread([this]() { writerLoop(); });
}

/**
 * @brief Stops the gRPC server and cancels all active streams.
 */
void API::GRPC::GRPCServer::stopServer()
{
  // Cancel all active streams
  {
    std::lock_guard<std::mutex> lock(m_frameStreamsMutex);
    for (auto& ctx : m_frameStreams)
      ctx->cancelled.store(true);
  }

  {
    std::lock_guard<std::mutex> lock(m_rawStreamsMutex);
    for (auto& ctx : m_rawStreams)
      ctx->cancelled.store(true);
  }

  // Stop the writer thread
  m_writerRunning.store(false);
  if (m_writerThread.joinable())
    m_writerThread.join();

  // Shut down the gRPC server
  if (m_grpcServer)
    m_grpcServer->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(3));

  // Wait for the server thread to finish
  if (m_serverThread.joinable())
    m_serverThread.join();

  m_grpcServer.reset();
  m_service.reset();

  m_clientCount.store(0, std::memory_order_relaxed);
  Q_EMIT clientCountChanged();
}

/**
 * @brief Writes a RawBatch to every active raw stream, marking unwritable streams cancelled.
 */
void API::GRPC::GRPCServer::broadcastRawBatch(const serialstudio::RawBatch& batch)
{
  std::lock_guard<std::mutex> lock(m_rawStreamsMutex);
  for (auto& ctx : m_rawStreams) {
    if (ctx->cancelled.load() || ctx->context->IsCancelled())
      continue;

    if (!ctx->writer->Write(batch))
      ctx->cancelled.store(true);
  }
}

/**
 * @brief Writes a FrameBatch to every active stream, marking unwritable streams cancelled.
 */
void API::GRPC::GRPCServer::broadcastFrameBatch(const serialstudio::FrameBatch& batch)
{
  std::lock_guard<std::mutex> lock(m_frameStreamsMutex);
  for (auto& ctx : m_frameStreams) {
    if (ctx->cancelled.load() || ctx->context->IsCancelled())
      continue;

    if (!ctx->writer->Write(batch))
      ctx->cancelled.store(true);
  }
}

/**
 * @brief Background thread that drains the frame and raw data queues and writes to all active gRPC
 * stream clients.
 */
void API::GRPC::GRPCServer::writerLoop()
{
  while (m_writerRunning.load()) {
    bool did_work = false;

    // Drain all queued frames into a single FrameBatch to amortize HTTP/2 overhead
    {
      serialstudio::FrameBatch batch;
      DataModel::TimestampedFramePtr frame;
      while (m_frameQueue.try_dequeue(frame)) {
        did_work             = true;
        auto* fd             = batch.add_frames();
        *fd->mutable_frame() = ConversionUtils::frameToProtoStruct(frame->data);
        fd->set_timestamp_ms(
          std::chrono::duration_cast<std::chrono::milliseconds>(frame->timestamp.time_since_epoch())
            .count());
      }

      if (batch.frames_size() > 0)
        broadcastFrameBatch(batch);
    }

    // Drain all queued raw data into a single RawBatch
    {
      serialstudio::RawBatch batch;
      QByteArray data;
      while (m_rawQueue.try_dequeue(data)) {
        did_work = true;
        auto* rd = batch.add_packets();
        rd->set_data(data.constData(), data.size());
        rd->set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count());
      }

      if (batch.packets_size() > 0)
        broadcastRawBatch(batch);
    }

    // Sleep briefly if no work to avoid busy-spinning
    if (!did_work)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

#  if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic pop
#  endif

#endif  // ENABLE_GRPC
