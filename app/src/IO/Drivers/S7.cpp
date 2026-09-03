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

#include "IO/Drivers/S7.h"

#include <chrono>
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSet>
#include <QStringList>
#include <QTcpSocket>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

static constexpr int kS7MinIntervalMs     = 50;
static constexpr int kS7MaxIntervalMs     = 60000;
static constexpr int kS7DefaultIntervalMs = 200;
static constexpr int kS7MaxRack           = IO::Drivers::S7Comm::kMaxRack;
static constexpr int kS7MaxSlot           = IO::Drivers::S7Comm::kMaxSlot;
static constexpr int kS7DefaultSlot       = 1;
static constexpr int kS7DialDeadlineMs    = 5000;
static constexpr int kS7ReplyDeadlineMs   = 3000;
static constexpr int kS7MaxTpdusPerReply  = 16;
static constexpr int kS7MaxReadRounds     = 64;
static constexpr int kS7MaxReceiveBytes   = 65536;
static constexpr int kS7FaultIndexShift   = 8;
static constexpr int kS7JoinTimeoutMs     = 5000;

//--------------------------------------------------------------------------------------------------
// Poll worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the worker with no socket: the socket is created on this object's thread inside
 *        connectToPlc(), never in the constructor, which still runs on the GUI thread.
 */
IO::Drivers::S7PollWorker::S7PollWorker()
  : m_rack(0), m_slot(kS7DefaultSlot), m_socket(nullptr), m_lastFault(0), m_itemErrors(0)
{}

/**
 * @brief Releases the socket. The driver joins the thread before destroying the worker, so this
 *        always runs with no poll in flight.
 */
IO::Drivers::S7PollWorker::~S7PollWorker()
{
  shutdown();
}

/**
 * @brief Stores the endpoint and the flattened read list. Called before the thread starts, so the
 *        worker's state is complete by the time its event loop delivers anything.
 */
void IO::Drivers::S7PollWorker::configure(
  const QString& host, int rack, int slot, int interval, QVector<S7ReadItem> items)
{
  SS_ASSERT(!items.isEmpty(), return);
  SS_ASSERT_LOG(interval >= kS7MinIntervalMs);

  m_host  = host;
  m_rack  = rack;
  m_slot  = slot;
  m_items = std::move(items);

  m_reads.clear();
  m_reads.reserve(m_items.size());

  QVector<OpcUaWire::Type> types;
  types.reserve(m_items.size());
  for (const auto& item : m_items) {
    m_reads.append(S7Comm::itemForAddress(item.address));
    types.append(item.wireType);
  }

  configureChannels(interval, std::move(types));
}

/**
 * @brief Opens the S7comm session. This blocks for as long as the controller takes to answer,
 *        which is exactly why it runs on the worker thread; the return value is the verdict
 *        beginDial() reports.
 */
bool IO::Drivers::S7PollWorker::connectToPlc()
{
  SS_ASSERT(!sessionOpen(), return true);
  SS_ASSERT(!m_reads.isEmpty(), return false);

  clearAbort();
  noteDialError(QString());
  if (!dial()) {
    shutdown();
    return false;
  }

  startPolling();
  return true;
}

/**
 * @brief Walks the three steps a controller needs before it answers a read: the TCP connection,
 *        the ISO connect request naming the rack and slot, and the S7comm setup that fixes the
 *        message budget. Each failure names the step, because they fail for different reasons: a
 *        refused ISO connection is a wrong slot, a refused setup is usually a locked-down CPU.
 */
bool IO::Drivers::S7PollWorker::dial()
{
  SS_ASSERT(m_socket == nullptr, return false);
  SS_ASSERT(!m_host.isEmpty(), return false);

  m_rx.clear();
  m_codec.reset();
  m_transport.reset();

  m_socket = new QTcpSocket(this);
  m_socket->connectToHost(m_host, S7Comm::kIsoTsapPort);
  if (!m_socket->waitForConnected(kS7DialDeadlineMs)) {
    noteDialError(m_socket->errorString());
    return false;
  }

  const auto request = m_transport.buildConnectRequest(m_rack, m_slot);
  if (request.isEmpty() || m_socket->write(request) != request.size()) {
    noteDialError(tr("The ISO connection request could not be sent"));
    return false;
  }

  QByteArray tpdu;
  if (!readTpdu(tpdu, kS7DialDeadlineMs) || !m_transport.parseConnectConfirm(tpdu)) {
    noteDialError(tr("The controller refused the ISO connection: check the rack and slot numbers"));
    return false;
  }

  return negotiate();
}

/**
 * @brief Runs the setup-communication exchange and plans the poll's chunks against the length the
 *        controller answered with. The plan is built HERE and not at configure time: the budget
 *        only exists once the controller has named it.
 */
bool IO::Drivers::S7PollWorker::negotiate()
{
  SS_ASSERT(m_socket != nullptr, return false);
  SS_ASSERT(!m_reads.isEmpty(), return false);

  const auto request = m_codec.buildSetupRequest(m_codec.nextReference());
  if (request.isEmpty() || !exchange(request, m_response)) {
    noteDialError(tr("The controller did not answer the S7comm setup request"));
    return false;
  }

  if (m_codec.parseSetupResponse(m_response) != S7Comm::PduResult::Ok) {
    const auto reason = m_codec.lastError();
    noteDialError(reason.isEmpty() ? tr("The controller refused the S7comm session") : reason);
    return false;
  }

  m_chunks = m_codec.planChunks(m_reads);
  SS_ASSERT_LOG(!m_chunks.isEmpty());
  return !m_chunks.isEmpty();
}

/**
 * @brief Sends one protocol data unit inside a data TPDU. The write is flushed synchronously: the
 *        answer is read back on this same call stack, so a queued write would deadlock the wait.
 */
bool IO::Drivers::S7PollWorker::sendPdu(const QByteArray& pdu)
{
  SS_ASSERT(!pdu.isEmpty(), return false);
  SS_ASSERT(m_socket != nullptr, return false);

  const auto packet = m_transport.wrapData(pdu);
  if (packet.isEmpty() || m_socket->write(packet) != packet.size())
    return false;

  return m_socket->waitForBytesWritten(kS7ReplyDeadlineMs);
}

/**
 * @brief Blocks until one complete TPKT has arrived and hands back the TPDU inside it. Each round
 *        drains whatever the socket already holds BEFORE it waits: the worker's event loop may
 *        have buffered the answer between ticks, and waiting on data that already arrived would
 *        hang the poll until the deadline. A malformed packet is terminal and cannot be resynced.
 */
bool IO::Drivers::S7PollWorker::readTpdu(QByteArray& tpdu, int timeoutMs)
{
  SS_ASSERT(m_socket != nullptr, return false);
  SS_ASSERT_LOG(timeoutMs > 0);

  QElapsedTimer clock;
  clock.start();
  for (int round = 0; round < kS7MaxReadRounds; ++round) {
    if (aborted())
      return false;

    m_rx.append(m_socket->readAll());

    S7Comm::Tpkt frame;
    const auto found = m_transport.extractTpkt(m_rx, frame);
    if (found == S7Comm::TpktResult::Malformed || m_rx.size() > kS7MaxReceiveBytes)
      return false;

    if (found == S7Comm::TpktResult::Ok) {
      tpdu = m_rx.mid(frame.tpduOffset, frame.tpduSize);
      m_rx.remove(0, frame.totalBytes);
      return true;
    }

    const auto left = timeoutMs - clock.elapsed();
    if (left <= 0 || !m_socket->waitForReadyRead(static_cast<int>(left)))
      return false;
  }

  return false;
}

/**
 * @brief Writes one request and reassembles the answer across however many data TPDUs carry it.
 *        The loop is bounded: a controller that never sets the end-of-transmission flag would
 *        otherwise hold the poll thread for as long as it kept sending.
 */
bool IO::Drivers::S7PollWorker::exchange(const QByteArray& request, QByteArray& response)
{
  SS_ASSERT(!request.isEmpty(), return false);
  SS_ASSERT(m_socket != nullptr, return false);

  response.clear();
  if (!sendPdu(request))
    return false;

  for (int part = 0; part < kS7MaxTpdusPerReply; ++part) {
    if (aborted())
      return false;

    QByteArray tpdu;
    if (!readTpdu(tpdu, kS7ReplyDeadlineMs))
      return false;

    bool complete = false;
    if (m_transport.acceptData(tpdu, response, complete) != S7Comm::TpktResult::Ok)
      return false;

    if (complete)
      return !response.isEmpty();
  }

  return false;
}

/**
 * @brief Closes the socket and drops the poll plan on the thread that owns them. Idempotent,
 *        because both the driver's teardown and the destructor reach it through shutdown().
 */
void IO::Drivers::S7PollWorker::releaseResources()
{
  if (m_socket) {
    m_socket->abort();
    m_socket->close();
    delete m_socket;
    m_socket = nullptr;
  }

  m_rx.clear();
  m_chunks.clear();
  SS_ASSERT_LOG(m_socket == nullptr);
  SS_ASSERT_LOG(m_chunks.isEmpty());
}

/**
 * @brief Polls every configured variable once, latches what changed and publishes one delta frame.
 *        A failed EXCHANGE ends the session, because a poll loop running against a controller
 *        that stopped answering looks healthy while publishing nothing. A refused ITEM does not:
 *        a bad address is a configuration error the other variables should survive.
 */
void IO::Drivers::S7PollWorker::pollTick()
{
  const auto now       = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 stampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

  SS_ASSERT_LOG(!m_chunks.isEmpty());
  for (const auto& chunk : m_chunks) {
    if (aborted())
      return;

    if (pollChunk(chunk))
      continue;

    if (aborted())
      return;

    reportFailure(tr("The controller closed the S7comm session"));
    return;
  }

  publishDirtySlots(stampNs);
}

/**
 * @brief Reads one chunk of the variable list in a single exchange. A job the controller refuses
 *        outright is counted against every item it carried and keeps the link, which is what a
 *        CPU with PUT/GET disabled answers on every tick.
 */
bool IO::Drivers::S7PollWorker::pollChunk(const S7Comm::Chunk& chunk)
{
  SS_ASSERT(chunk.count > 0, return false);
  SS_ASSERT(chunk.first + chunk.count <= m_reads.size(), return false);

  const auto request = m_codec.buildReadRequest(m_codec.nextReference(), m_reads, chunk);
  if (request.isEmpty() || !exchange(request, m_response))
    return false;

  m_results.clear();
  const auto parsed = m_codec.parseReadResponse(m_response, chunk.count, m_results);
  if (parsed == S7Comm::PduResult::Refused) {
    countReadsFailed(chunk.count);
    m_itemErrors.fetch_add(chunk.count, std::memory_order_relaxed);
    return true;
  }

  if (parsed != S7Comm::PduResult::Ok)
    return false;

  for (int i = 0; i < m_results.size(); ++i)
    applyResult(chunk.first + i, m_results.at(i), m_response);

  return true;
}

/**
 * @brief Decodes one answered item and latches it when its value moved. A refused item records only
 *        an index and a code, so the pane assembles the string instead of the poll thread. A
 *        SUCCESS item whose payload does not decode (a controller-declared length of zero) counts
 *        as an item error too. Both keep the previous value; neither asserts on wire input.
 */
void IO::Drivers::S7PollWorker::applyResult(int index,
                                            const S7Comm::ReadResult& result,
                                            QByteArrayView pdu)
{
  SS_ASSERT(index >= 0 && index < m_items.size(), return);
  SS_ASSERT(result.offset + result.size <= pdu.size(), return);

  if (result.returnCode != S7Comm::kReturnSuccess) {
    const auto fault = (static_cast<quint64>(index) + 1) << kS7FaultIndexShift;
    countReadsFailed(1);
    m_itemErrors.fetch_add(1, std::memory_order_relaxed);
    m_lastFault.store(fault | result.returnCode, std::memory_order_relaxed);
    return;
  }

  const auto& address = m_items.at(index).address;
  const auto value    = S7Comm::decodeValue(address, pdu.sliced(result.offset, result.size));
  if (!value.isValid()) {
    countReadsFailed(1);
    m_itemErrors.fetch_add(1, std::memory_order_relaxed);
    return;
  }

  countReadsOk(1);
  (void)latchChannel(index, value);
}

/**
 * @brief The last refused variable packed as ((index + 1) << 8 | return code), zero when the
 *        session has refused nothing.
 */
quint64 IO::Drivers::S7PollWorker::lastFault() const noexcept
{
  return m_lastFault.load(std::memory_order_relaxed);
}

/**
 * @brief Items the controller answered with a non-success return code since the session opened.
 */
quint64 IO::Drivers::S7PollWorker::itemErrors() const noexcept
{
  return m_itemErrors.load(std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Driver construction and teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the driver, restores persisted settings and wires the configuration signals.
 */
IO::Drivers::S7::S7()
  : m_appState(AppState::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_open(false)
  , m_connecting(false)
  , m_persistent(true)
  , m_rack(0)
  , m_slot(kS7DefaultSlot)
  , m_pollInterval(kS7DefaultIntervalMs)
  , m_lastStampNs(0)
  , m_linkDrops(0)
  , m_host(QStringLiteral("192.168.0.1"))
  , m_thread(std::make_unique<QThread>())
  , m_worker(nullptr)
{
  loadSettings();

  static constexpr void (S7::* kConfigSignals[])() = {&S7::hostChanged,
                                                      &S7::rackChanged,
                                                      &S7::slotChanged,
                                                      &S7::pollIntervalChanged,
                                                      &S7::variablesChanged};
  for (const auto signal : kConfigSignals)
    connect(this, signal, this, &IO::Drivers::S7::configurationChanged);

  Q_EMIT configurationChanged();
}

/**
 * @brief Joins the poll thread and destroys the worker.
 */
IO::Drivers::S7::~S7()
{
  doClose();
}

/**
 * @brief Restores the endpoint and the variable list.
 */
void IO::Drivers::S7::loadSettings()
{
  m_host         = m_settings.value("S7Driver/host", m_host).toString();
  m_rack         = qBound(0, m_settings.value("S7Driver/rack", 0).toInt(), kS7MaxRack);
  m_slot         = qBound(0, m_settings.value("S7Driver/slot", kS7DefaultSlot).toInt(), kS7MaxSlot);
  m_pollInterval = qBound(kS7MinIntervalMs,
                          m_settings.value("S7Driver/pollInterval", kS7DefaultIntervalMs).toInt(),
                          kS7MaxIntervalMs);

  const auto doc =
    QJsonDocument::fromJson(m_settings.value("S7Driver/variables", QByteArray("[]")).toByteArray());
  if (doc.isArray())
    setVariables(doc.array());
}

/**
 * @brief Persists the variable list as compact JSON; only the UI-config instance writes settings.
 */
void IO::Drivers::S7::saveVariables()
{
  if (!m_persistent)
    return;

  m_settings.setValue("S7Driver/variables",
                      QJsonDocument(variablesJson()).toJson(QJsonDocument::Compact));
}

/**
 * @brief Re-resolves the flattened read list from the variable list; an address that no longer
 *        parses leaves the list SHORTER than the variable list, which is what configurationOk()
 *        reads to refuse the connection instead of reading the wrong memory.
 */
void IO::Drivers::S7::refreshItems()
{
  m_items.clear();
  m_items.reserve(m_variables.size());

  for (const auto& variable : m_variables) {
    QString error;
    const auto parsed = S7Address::parse(variable.address, error);
    if (!S7Address::isValid(parsed))
      continue;

    S7ReadItem item;
    item.address  = parsed;
    item.wireType = wireTypeFor(parsed.type);
    m_items.append(item);
  }

  SS_ASSERT_LOG(m_items.size() <= m_variables.size());
  SS_ASSERT_LOG(m_items.size() <= OpcUaWire::kMaxTags);
}

/**
 * @brief Only the UI-config instance writes settings; the per-source live instance is fed by the
 *        manager and must never echo its state back into QSettings.
 */
void IO::Drivers::S7::setPersistent(const bool persistent) noexcept
{
  if (m_persistent == persistent)
    return;

  m_persistent = persistent;
}

/**
 * @brief Points this UI-config instance at the per-source instance that owns the live session, so
 *        the pane and the API read the counters of the link that is actually running.
 */
void IO::Drivers::S7::setSessionPeer(S7* peer)
{
  SS_ASSERT(peer != this, return);
  if (m_sessionPeer == peer)
    return;

  m_sessionPeer = peer;
  if (peer)
    connect(peer, &QObject::destroyed, this, &IO::Drivers::S7::statusChanged, Qt::UniqueConnection);

  Q_EMIT statusChanged();
}

/**
 * @brief The live session whose state answers a status query, or nullptr when this instance is
 *        itself the session; only the persistent instance delegates, so the hop is one deep.
 */
const IO::Drivers::S7* IO::Drivers::S7::sessionPeer() const
{
  return m_persistent ? m_sessionPeer.data() : nullptr;
}

/**
 * @brief Closes the session; a user's disconnect is final.
 */
void IO::Drivers::S7::close()
{
  doClose();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Non-virtual teardown shared by close() and the destructor. The worker is asked to release
 *        its client on its OWN thread first, then the event loop is QUIT before wait(): a thread
 *        whose started() signal drives the work never returns from wait() otherwise (HID
 *        cleanupDevice() is the reference).
 */
void IO::Drivers::S7::doClose()
{
  m_open       = false;
  m_connecting = false;
  if (!m_worker) {
    SS_ASSERT_LOG(!m_thread->isRunning());
    return;
  }

  for (const auto& link : std::as_const(m_workerLinks))
    disconnect(link);

  m_workerLinks.clear();
  if (m_thread->isRunning()) {
    m_worker->requestAbort();
    QMetaObject::invokeMethod(
      m_worker, [this] { m_worker->shutdown(); }, Qt::BlockingQueuedConnection);
    m_thread->quit();
    if (!m_thread->wait(kS7JoinTimeoutMs)) {
      qWarning() << "[S7] poll thread did not stop within" << kS7JoinTimeoutMs
                 << "ms -- abandoning it";
      (void)m_thread.release();
      m_thread      = std::make_unique<QThread>();
      m_worker      = nullptr;
      m_lastStampNs = 0;
      return;
    }
  }

  delete m_worker;
  m_worker      = nullptr;
  m_lastStampNs = 0;
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while the S7comm session is established.
 */
bool IO::Drivers::S7::isOpen() const noexcept
{
  return m_open;
}

/**
 * @brief Returns true while the worker's dial is in flight; the connect button reads this rather
 *        than looking like a dead click for the length of the controller's timeout.
 */
bool IO::Drivers::S7::isConnecting() const noexcept
{
  return m_connecting;
}

/**
 * @brief Values arrive through the poll worker, so readable means open.
 */
bool IO::Drivers::S7::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Writing controller memory is out of scope (spec 0073 non-goal).
 */
bool IO::Drivers::S7::isWritable() const noexcept
{
  return false;
}

/**
 * @brief A host, a rack/slot pair in range, and a variable list whose every address parses.
 */
bool IO::Drivers::S7::configurationOk() const noexcept
{
  if (m_host.trimmed().isEmpty() || m_variables.isEmpty())
    return false;

  if (m_variables.size() > OpcUaWire::kMaxTags)
    return false;

  return m_items.size() == m_variables.size();
}

/**
 * @brief The driver never writes; a caller that tries gets a hard failure rather than a silent 0.
 */
qint64 IO::Drivers::S7::write(const QByteArray& data)
{
  Q_UNUSED(data)
  return -1;
}

/**
 * @brief Opens the session. The worker thread is started, the blocking dial runs ON that thread,
 *        and its result is returned straight to the connect fan-out: one attempt, one verdict, no
 *        openFinished latch and no retry stack (spec 0050).
 */
bool IO::Drivers::S7::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  doClose();
  if (!configurationOk()) {
    m_lastError = tr("The connection is not configured: check the host and the variable list");
    Q_EMIT statusChanged();
    return false;
  }

  m_lastError.clear();
  m_worker = new S7PollWorker();
  m_worker->configure(m_host.trimmed(), m_rack, m_slot, m_pollInterval, m_items);
  m_worker->moveToThread(m_thread.get());
  m_workerLinks = {
    connect(m_worker, &S7PollWorker::frameReady, this, &IO::Drivers::S7::onFrameReady),
    connect(m_worker, &S7PollWorker::linkLost, this, &IO::Drivers::S7::onLinkLost),
    connect(m_worker, &S7PollWorker::dialFinished, this, &IO::Drivers::S7::onDialFinished),
  };

  m_thread->start();

  m_connecting = true;
  QMetaObject::invokeMethod(m_worker, &S7PollWorker::beginDial, Qt::QueuedConnection);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  return true;
}

/**
 * @brief Settles the worker's dial verdict exactly once. A verdict landing after the user closed
 *        the session is dropped: doClose() clears the dialing flag, so a late report can neither
 *        reopen the driver nor report an attempt nobody is waiting for.
 */
void IO::Drivers::S7::onDialFinished(bool ok, const QString& reason)
{
  if (!m_connecting)
    return;

  m_connecting = false;
  m_open       = ok;

  if (!ok) {
    m_lastError = reason;
    logDriverError(
      tr("S7 Connection Failed"),
      tr("\"%1\" (rack %2, slot %3): %4")
        .arg(m_host)
        .arg(m_rack)
        .arg(m_slot)
        .arg(m_lastError.isEmpty() ? tr("the controller did not answer") : m_lastError));
    doClose();
  }

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  reportOpenFinished(ok, reason);
}

//--------------------------------------------------------------------------------------------------
// Worker signal handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a delta frame the worker built. The stamp travelled WITH the payload, so it is
 *        the poll's own time; it is only clamped forward here so a rounded steady clock can never
 *        hand the pipeline two frames with the same timestamp.
 */
void IO::Drivers::S7::onFrameReady(const QByteArray& frame, qint64 stampNs)
{
  SS_ASSERT(!frame.isEmpty(), return);
  SS_ASSERT_LOG(stampNs > 0);

  const qint64 stamp = qMax(stampNs, m_lastStampNs + 1);
  m_lastStampNs      = stamp;
  publishReceivedData(frame, CapturedData::SteadyTimePoint(std::chrono::nanoseconds(stamp)));
}

/**
 * @brief The session dropped: report once, then let the manager close the device on the next
 *        event-loop turn so nothing tears the driver down from inside its own handler. This is
 *        never sessionClosed() -- a drop is a link event and the session outlives it.
 */
void IO::Drivers::S7::onLinkLost(const QString& reason)
{
  m_lastError = reason;
  ++m_linkDrops;
  if (!m_open) {
    Q_EMIT statusChanged();
    return;
  }

  m_open = false;
  logDriverError(tr("S7 Connection Lost"), reason);

  static auto& manager = ConnectionManager::instance();
  QMetaObject::invokeMethod(this, [this] { manager.disconnectDevice(this); }, Qt::QueuedConnection);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the controller's host name or IP address.
 */
QString IO::Drivers::S7::host() const
{
  return m_host;
}

/**
 * @brief Returns the configured rack number.
 */
int IO::Drivers::S7::rack() const
{
  return m_rack;
}

/**
 * @brief Returns the configured slot number.
 */
int IO::Drivers::S7::slot() const
{
  return m_slot;
}

/**
 * @brief Returns the poll interval in milliseconds.
 */
int IO::Drivers::S7::pollInterval() const
{
  return m_pollInterval;
}

/**
 * @brief Returns the number of configured variables.
 */
int IO::Drivers::S7::variableCount() const
{
  return static_cast<int>(m_variables.size());
}

/**
 * @brief Returns the configured variables in wire order.
 */
const QVector<IO::Drivers::S7Variable>& IO::Drivers::S7::variables() const noexcept
{
  return m_variables;
}

/**
 * @brief One-line session status for the pane, read from the live session when this instance is
 *        the UI-config one.
 */
QString IO::Drivers::S7::statusText() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusText();

  if (!m_open)
    return m_lastError.isEmpty() ? tr("Not connected") : m_lastError;

  const double hz = 1000.0 / qMax(1, m_pollInterval);
  return tr("Polling %1 variables at %2 Hz").arg(m_variables.size()).arg(hz, 0, 'f', 1);
}

/**
 * @brief Human-readable row for the pane's variable list.
 */
QString IO::Drivers::S7::variableInfo(const int index) const
{
  if (index < 0 || index >= m_variables.size())
    return {};

  const auto& variable = m_variables.at(index);

  QString error;
  const auto parsed = S7Address::parse(variable.address, error);
  if (!S7Address::isValid(parsed))
    return QStringLiteral("%1 (%2) %3").arg(variable.name, tr("invalid"), variable.address);

  return QStringLiteral("%1 (%2) %3")
    .arg(variable.name, S7Address::codeFromType(parsed.type), S7Address::normalize(parsed));
}

/**
 * @brief Returns an empty string when @p address parses, otherwise the reason it does not; the
 *        variable dialog shows this while the user types.
 */
QString IO::Drivers::S7::validateAddress(const QString& address) const
{
  QString error;
  const auto parsed = S7Address::parse(address, error);
  if (S7Address::isValid(parsed))
    return {};

  SS_ASSERT_LOG(!error.isEmpty());
  return error.isEmpty() ? tr("The address could not be parsed.") : error;
}

/**
 * @brief The variable list as JSON (conn-settings and API shape).
 */
QJsonArray IO::Drivers::S7::variablesJson() const
{
  QJsonArray array;
  for (const auto& variable : m_variables)
    array.append(QJsonObject{
      {   QStringLiteral("name"),    variable.name},
      {QStringLiteral("address"), variable.address},
    });

  return array;
}

/**
 * @brief The `s7` native template schema: one {index, name} entry per wire index.
 */
QJsonArray IO::Drivers::S7::wireSchema() const
{
  QJsonArray schema;
  for (int i = 0; i < m_variables.size() && i < OpcUaWire::kMaxTags; ++i)
    schema.append(QJsonObject{
      {QStringLiteral("index"),                      i},
      { QStringLiteral("name"), m_variables.at(i).name},
    });

  return schema;
}

/**
 * @brief Pulled diagnostics snapshot (spec 0033: counters, never pushed), read from the live
 *        session when this instance is the UI-config one.
 */
QJsonObject IO::Drivers::S7::statusJson() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusJson();

  const quint64 reads  = m_worker ? m_worker->readsOk() : 0;
  const quint64 failed = m_worker ? m_worker->readsFailed() : 0;
  const quint64 frames = m_worker ? m_worker->framesPublished() : 0;
  const quint64 items  = m_worker ? m_worker->itemErrors() : 0;

  return QJsonObject{
    {      QStringLiteral("connected"),                           m_open},
    {           QStringLiteral("host"),                           m_host},
    {           QStringLiteral("rack"),                           m_rack},
    {           QStringLiteral("slot"),                           m_slot},
    {   QStringLiteral("pollInterval"),                   m_pollInterval},
    {  QStringLiteral("variableCount"),                  variableCount()},
    {        QStringLiteral("readsOk"),       static_cast<qint64>(reads)},
    {    QStringLiteral("readsFailed"),      static_cast<qint64>(failed)},
    {     QStringLiteral("itemErrors"),       static_cast<qint64>(items)},
    {  QStringLiteral("lastItemError"),                  itemFaultText()},
    {QStringLiteral("framesPublished"),      static_cast<qint64>(frames)},
    {      QStringLiteral("linkDrops"), static_cast<qint64>(m_linkDrops)},
    {      QStringLiteral("lastError"),                      m_lastError},
    {     QStringLiteral("statusText"),                     statusText()},
  };
}

/**
 * @brief Names the variable the controller last refused and why. The poll thread records only an
 *        index and a return code, so the string is assembled HERE against the variable list the
 *        GUI thread owns rather than shared across the two.
 */
QString IO::Drivers::S7::itemFaultText() const
{
  const quint64 fault = m_worker ? m_worker->lastFault() : 0;
  if (fault == 0)
    return {};

  const auto index = static_cast<int>(fault >> kS7FaultIndexShift) - 1;
  const auto code  = static_cast<std::uint8_t>(fault & 0xFF);
  if (index < 0 || index >= m_variables.size())
    return S7Comm::returnCodeText(code);

  return QStringLiteral("%1: %2").arg(m_variables.at(index).name, S7Comm::returnCodeText(code));
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the controller's host name or IP address.
 */
void IO::Drivers::S7::setHost(const QString& host)
{
  const auto trimmed = host.trimmed();
  if (m_host == trimmed)
    return;

  m_host = trimmed;
  if (m_persistent)
    m_settings.setValue("S7Driver/host", m_host);

  Q_EMIT hostChanged();
}

/**
 * @brief Sets the rack number (0-7).
 */
void IO::Drivers::S7::setRack(const int rack)
{
  const int clamped = qBound(0, rack, kS7MaxRack);
  if (m_rack == clamped)
    return;

  m_rack = clamped;
  if (m_persistent)
    m_settings.setValue("S7Driver/rack", m_rack);

  Q_EMIT rackChanged();
}

/**
 * @brief Sets the slot number (0-31); S7-300/400 CPUs sit at slot 2, S7-1200/1500 at slot 1.
 */
void IO::Drivers::S7::setSlot(const int slot)
{
  const int clamped = qBound(0, slot, kS7MaxSlot);
  if (m_slot == clamped)
    return;

  m_slot = clamped;
  if (m_persistent)
    m_settings.setValue("S7Driver/slot", m_slot);

  Q_EMIT slotChanged();
}

/**
 * @brief Sets the poll interval in milliseconds; a live session keeps the interval it opened with.
 */
void IO::Drivers::S7::setPollInterval(const int interval)
{
  const int clamped = qBound(kS7MinIntervalMs, interval, kS7MaxIntervalMs);
  if (m_pollInterval == clamped)
    return;

  m_pollInterval = clamped;
  if (m_persistent)
    m_settings.setValue("S7Driver/pollInterval", m_pollInterval);

  Q_EMIT pollIntervalChanged();
  Q_EMIT statusChanged();
}

/**
 * @brief True while a live session exists for this configuration, so the wire layout is immutable.
 *        The per-source instance owns a worker; the UI-config instance owns none and must consult
 *        its live peer, or the pane would let the user edit a list the running session already
 *        sized. Silent: the UI-to-live property echo hits this on every configuration change.
 */
bool IO::Drivers::S7::variablesFrozen() const noexcept
{
  return m_worker != nullptr || sessionPeer() != nullptr;
}

/**
 * @brief Peer-aware frozen flag the variables dialog binds its editing controls to, so the dialog
 *        visibly locks while a session is live rather than silently dropping edits.
 */
bool IO::Drivers::S7::variablesLocked() const noexcept
{
  return variablesFrozen();
}

/**
 * @brief Replaces the variable list from its JSON shape, dropping unnamed and duplicate entries.
 */
void IO::Drivers::S7::setVariables(const QJsonArray& variables)
{
  QVector<S7Variable> list;
  QSet<QString> seen;
  for (const auto& item : variables) {
    const auto obj = item.toObject();

    S7Variable variable;
    variable.address = obj.value(QStringLiteral("address")).toString().trimmed();
    variable.name    = obj.value(QStringLiteral("name")).toString(variable.address).trimmed();
    if (variable.address.isEmpty() || seen.contains(variable.name))
      continue;

    if (list.size() >= OpcUaWire::kMaxTags)
      break;

    seen.insert(variable.name);
    list.append(variable);
  }

  if (list == m_variables || variablesFrozen())
    return;

  m_variables = std::move(list);
  refreshItems();
  saveVariables();
  Q_EMIT variablesChanged();
}

/**
 * @brief Appends a variable unless its name is taken or the list is full.
 */
void IO::Drivers::S7::addVariable(const QString& name, const QString& address)
{
  const auto trimmedAddress = address.trimmed();
  const auto trimmedName    = name.trimmed().isEmpty() ? trimmedAddress : name.trimmed();
  if (trimmedAddress.isEmpty() || variablesFrozen())
    return;

  if (m_variables.size() >= OpcUaWire::kMaxTags)
    return;

  for (const auto& existing : m_variables)
    if (existing.name == trimmedName)
      return;

  QString error;
  if (!S7Address::isValid(S7Address::parse(trimmedAddress, error))) {
    logDriverError(tr("S7 Address"), error);
    return;
  }

  m_variables.append(S7Variable{trimmedName, trimmedAddress});
  refreshItems();
  saveVariables();
  Q_EMIT variablesChanged();
}

/**
 * @brief Removes the variable at the given position.
 */
void IO::Drivers::S7::removeVariable(const int index)
{
  if (index < 0 || index >= m_variables.size() || variablesFrozen())
    return;

  m_variables.removeAt(index);
  refreshItems();
  saveVariables();
  Q_EMIT variablesChanged();
}

/**
 * @brief Drops every configured variable.
 */
void IO::Drivers::S7::clearVariables()
{
  if (m_variables.isEmpty() || variablesFrozen())
    return;

  m_variables.clear();
  refreshItems();
  saveVariables();
  Q_EMIT variablesChanged();
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat editable property list; the variable list rides along so projects capture it.
 */
QList<IO::DriverProperty> IO::Drivers::S7::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty host;
  host.key   = QStringLiteral("host");
  host.label = tr("Host");
  host.type  = IO::DriverProperty::Text;
  host.value = m_host;
  props.append(host);

  IO::DriverProperty rack;
  rack.key   = QStringLiteral("rack");
  rack.label = tr("Rack");
  rack.type  = IO::DriverProperty::IntField;
  rack.value = m_rack;
  rack.min   = 0;
  rack.max   = kS7MaxRack;
  props.append(rack);

  IO::DriverProperty slot;
  slot.key   = QStringLiteral("slot");
  slot.label = tr("Slot");
  slot.type  = IO::DriverProperty::IntField;
  slot.value = m_slot;
  slot.min   = 0;
  slot.max   = kS7MaxSlot;
  props.append(slot);

  IO::DriverProperty interval;
  interval.key   = QStringLiteral("pollInterval");
  interval.label = tr("Poll Interval (ms)");
  interval.type  = IO::DriverProperty::IntField;
  interval.value = m_pollInterval;
  interval.min   = kS7MinIntervalMs;
  interval.max   = kS7MaxIntervalMs;
  props.append(interval);

  IO::DriverProperty variables;
  variables.key   = QStringLiteral("variables");
  variables.type  = IO::DriverProperty::Text;
  variables.value = QVariant::fromValue(variablesJson());
  props.append(variables);

  return props;
}

/**
 * @brief Applies a single configuration change by key; the twin of driverProperties(), and the
 *        half a project round-trip dies without.
 */
void IO::Drivers::S7::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("host")) {
    setHost(value.toString());
    return;
  }

  if (key == QLatin1String("rack")) {
    setRack(value.toInt());
    return;
  }

  if (key == QLatin1String("slot")) {
    setSlot(value.toInt());
    return;
  }

  if (key == QLatin1String("pollInterval")) {
    setPollInterval(value.toInt());
    return;
  }

  if (key != QLatin1String("variables"))
    return;

  QJsonArray array;
  if (value.canConvert<QJsonArray>())
    array = value.toJsonArray();
  else if (value.typeId() == QMetaType::QVariantList) {
    const auto list = value.toList();
    for (const auto& item : list)
      array.append(QJsonValue::fromVariant(item));
  }

  setVariables(array);
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The wire type a parsed S7 type encodes as; anything unmapped travels as text.
 */
IO::Drivers::OpcUaWire::Type IO::Drivers::S7::wireTypeFor(S7Address::Type type) noexcept
{
  switch (type) {
    case S7Address::Type::Bool:
      return OpcUaWire::Type::Bool;
    case S7Address::Type::Byte:
      return OpcUaWire::Type::U8;
    case S7Address::Type::Word:
      return OpcUaWire::Type::U16;
    case S7Address::Type::DWord:
      return OpcUaWire::Type::U32;
    case S7Address::Type::Int:
      return OpcUaWire::Type::I16;
    case S7Address::Type::DInt:
      return OpcUaWire::Type::I32;
    case S7Address::Type::Real:
      return OpcUaWire::Type::F32;
    case S7Address::Type::Str:
    case S7Address::Type::Invalid:
      break;
  }

  return OpcUaWire::Type::Str;
}

/**
 * @brief One dataset per variable: LED for bits, plot for numerics, plain text for strings.
 */
DataModel::Dataset IO::Drivers::S7::datasetFor(const S7Variable& variable,
                                               S7Address::Type type,
                                               int index)
{
  SS_ASSERT_LOG(index >= 1);
  SS_ASSERT_LOG(!variable.name.isEmpty());

  DataModel::Dataset dataset;
  dataset.index = index;
  dataset.log   = true;
  dataset.title = variable.name;

  if (type == S7Address::Type::Bool) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (type != S7Address::Type::Str && type != S7Address::Type::Invalid)
    dataset.plt = true;

  return dataset;
}

/**
 * @brief One group per memory area, one dataset per wire index, the s7 native template.
 */
QJsonObject IO::Drivers::S7::buildProject() const
{
  QJsonObject project;
  project[Keys::Title]   = tr("Siemens S7 Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("Siemens S7");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::S7);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("s7");
  source[Keys::FrameParserParams]     = QJsonObject{
        {QStringLiteral("schema"), wireSchema()}
  };

  QJsonObject conn;
  for (const auto& prop : driverProperties())
    if (prop.type != IO::DriverProperty::Password)
      conn.insert(prop.key, QJsonValue::fromVariant(prop.value));

  source[Keys::SourceConn] = conn;
  project[Keys::Sources]   = QJsonArray{source};

  QStringList order;
  QHash<QString, DataModel::Group> groups;
  for (int i = 0; i < m_variables.size(); ++i) {
    QString error;
    const auto parsed = S7Address::parse(m_variables.at(i).address, error);
    const QString key =
      parsed.area == S7Address::Area::DataBlk ? tr("DB%1").arg(parsed.dbNumber) : tr("Memory");
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = key;
      groups.insert(key, group);
      order.append(key);
    }

    groups[key].datasets.push_back(datasetFor(m_variables.at(i), parsed.type, i + 1));
  }

  QJsonArray groupArray;
  for (const auto& key : order)
    groupArray.append(DataModel::serialize(groups.value(key)));

  project[Keys::Groups] = groupArray;
  return project;
}

/**
 * @brief Builds the project and loads it into the editor (no save dialog); the API path.
 */
DataModel::ProjectModel* IO::Drivers::S7::loadGeneratedProject()
{
  if (m_variables.isEmpty())
    return nullptr;

  m_appState.setOperationMode(SerialStudio::ProjectFile);
  if (!m_projectModel.loadFromJsonDocument(QJsonDocument(buildProject()), QString())) {
    logDriverError(tr("Failed to load generated project"),
                   tr("The generated project JSON could not be loaded."));
    return nullptr;
  }

  m_projectModel.setModified(true);
  return &m_projectModel;
}

/**
 * @brief Generates a project from the variable list and opens it in the editor.
 */
void IO::Drivers::S7::generateProject()
{
  if (m_variables.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No variables configured"),
                                    tr("Add at least one variable before generating a project."),
                                    QMessageBox::Warning,
                                    tr("S7 Project Generator"));
    return;
  }

  auto* pm = loadGeneratedProject();
  if (!pm)
    return;

  const int datasets = wireSchema().size();
  QObject::connect(
    pm,
    &DataModel::ProjectModel::saveDialogCompleted,
    this,
    [datasets](bool accepted) {
      if (!accepted)
        return;

      Misc::Utilities::showMessageBox(
        tr("Successfully generated project with %1 datasets.").arg(datasets),
        tr("The project editor is now open for customization."),
        QMessageBox::Information,
        tr("S7 Project Generator"));
    },
    Qt::SingleShotConnection);

  (void)pm->saveJsonFile(true);
}
