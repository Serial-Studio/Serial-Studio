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
 *
 * All RPC handlers execute on the gRPC thread pool. Commands that interact
 * with Qt singletons are marshaled to the main thread via
 * QMetaObject::invokeMethod with BlockingQueuedConnection.
 */
class SerialStudioServiceImpl final : public serialstudio::SerialStudioAPI::Service {
public:
  explicit SerialStudioServiceImpl(API::GRPC::GRPCServer* server) : m_server(server) {}

  /**
   * @brief Executes a single API command.
   */
  grpc::Status ExecuteCommand(grpc::ServerContext* /*context*/,
                              const serialstudio::CommandRequest* request,
                              serialstudio::CommandResponse* response) override
  {
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
  grpc::Status ExecuteBatch(grpc::ServerContext* /*context*/,
                            const serialstudio::BatchRequest* request,
                            serialstudio::BatchResponse* response) override
  {
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
   *
   * Registers a stream context that the main thread populates with frames.
   * The gRPC thread writes frames to the client as they arrive.
   */
  grpc::Status StreamFrames(grpc::ServerContext* context,
                            const serialstudio::StreamRequest* /*request*/,
                            grpc::ServerWriter<serialstudio::FrameBatch>* writer) override
  {
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
  grpc::Status WriteRawData(grpc::ServerContext* /*context*/,
                            const serialstudio::RawDataRequest* request,
                            serialstudio::CommandResponse* response) override
  {
    const auto& bytes = request->data();
    QByteArray data(bytes.data(), static_cast<int>(bytes.size()));

    bool written = false;
    QMetaObject::invokeMethod(
      QCoreApplication::instance(),
      [&]() { written = IO::ConnectionManager::instance().writeData(data) == data.size(); },
      Qt::BlockingQueuedConnection);

    response->set_id(request->id());
    response->set_success(written);

    if (!written) {
      auto* err = response->mutable_error();
      err->set_code("WRITE_FAILED");
      err->set_message("Failed to write data to device");
    }

    return grpc::Status::OK;
  }

  /**
   * @brief Lists all available API commands.
   */
  grpc::Status ListCommands(grpc::ServerContext* /*context*/,
                            const google::protobuf::Empty* /*request*/,
                            serialstudio::CommandList* response) override
  {
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
  API::GRPC::GRPCServer* m_server;
};

//--------------------------------------------------------------------------------------------------
// Constructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the gRPC server singleton.
 */
API::GRPC::GRPCServer::GRPCServer() : m_enabled(false)
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
 * @brief Returns true — gRPC support was compiled in.
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
 * @brief Enqueues a parsed frame for background writing to gRPC clients.
 *
 * Called from the main thread hotpath. Lock-free enqueue ensures zero
 * blocking on the UI/data thread.
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
void API::GRPC::GRPCServer::hotpathTxData(const IO::ByteArrayPtr& data)
{
  if (!m_enabled || !data || data->isEmpty())
    return;

  m_rawQueue.try_enqueue(data);
}

/**
 * @brief Exports a typed .proto file to the given path.
 *
 * Generates a .proto file with per-command typed messages using the
 * ProtoGenerator utility.
 */
void API::GRPC::GRPCServer::exportProto(const QString& filePath)
{
  (void)ProtoGenerator::exportToFile(filePath);
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
 * @brief Background thread that drains the frame and raw data queues
 *        and writes to all active gRPC stream clients.
 *
 * Serialization (Frame → JSON → protobuf Struct) and the blocking
 * gRPC Write() calls happen here, keeping the main thread free.
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

      if (batch.frames_size() > 0) {
        std::lock_guard<std::mutex> lock(m_frameStreamsMutex);
        for (auto& ctx : m_frameStreams) {
          if (ctx->cancelled.load() || ctx->context->IsCancelled())
            continue;

          if (!ctx->writer->Write(batch))
            ctx->cancelled.store(true);
        }
      }
    }

    // Drain all queued raw data into a single RawBatch
    {
      serialstudio::RawBatch batch;
      IO::ByteArrayPtr data;
      while (m_rawQueue.try_dequeue(data)) {
        did_work = true;
        auto* rd = batch.add_packets();
        rd->set_data(data->constData(), data->size());
        rd->set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count());
      }

      if (batch.packets_size() > 0) {
        std::lock_guard<std::mutex> lock(m_rawStreamsMutex);
        for (auto& ctx : m_rawStreams) {
          if (ctx->cancelled.load() || ctx->context->IsCancelled())
            continue;

          if (!ctx->writer->Write(batch))
            ctx->cancelled.store(true);
        }
      }
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
