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

#include "IO/Drivers/EthernetIp.h"

#include <chrono>
#include <iterator>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSet>
#include <QTimer>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

#ifdef SS_EIP_ACTIVE
#  include <libplctag.h>
#endif

static constexpr int kEipMinIntervalMs                    = 50;
static constexpr int kEipMaxIntervalMs                    = 60000;
static constexpr int kEipDefaultIntervalMs                = 250;
[[maybe_unused]] static constexpr int kEipCreateTimeoutMs = 5000;
[[maybe_unused]] static constexpr int kEipReadTimeoutMs   = 1500;
static constexpr int kEipDeadTickLimit                    = 3;
static constexpr int kEipDialDeadlineMs                   = 5000;
static constexpr int kEipJoinTimeoutMs                    = 5000;
static constexpr int kEipMaxTagNameChars                  = 256;
[[maybe_unused]] static constexpr const char* kEipBackend = "libplctag";

/**
 * @brief The controller families libplctag addresses, in the order the picker shows them; the
 *        string is the value its `plc=` attribute takes, so the ORDER is persisted state.
 */
static constexpr const char* kEipPlcTypes[] = {
  "controllogix",
  "compactlogix",
  "micrologix",
  "micrologix800",
  "plc5",
  "slc500",
  "logixpccc",
  "omron-njnx",
};

/**
 * @brief Rejects a CIP tag name that would break out of libplctag's `&`-delimited attribute string.
 *        The name is interpolated into `...&name=%1`, so a `&`, `=` or whitespace could inject a
 *        second attribute (a rogue `gateway=`); the length cap bounds the scan.
 */
[[nodiscard]] static bool eipTagNameSafe(const QString& tag)
{
  SS_ASSERT_LOG(kEipMaxTagNameChars > 0);
  if (tag.isEmpty() || tag.size() > kEipMaxTagNameChars)
    return false;

  for (const QChar ch : tag)
    if (ch == QLatin1Char('&') || ch == QLatin1Char('=') || ch.isSpace())
      return false;

  return true;
}

/**
 * @brief Rejects a host or CIP path that would break out of the same attribute string: `gateway=`
 *        and `path=` carry user text too, and a `&` there appends an attribute just as
 *        effectively. Whitespace passes here, unlike in a tag name, because a path is commonly
 *        typed as "1, 0".
 */
[[nodiscard]] static bool eipAttributeValueSafe(const QString& value)
{
  SS_ASSERT_LOG(kEipMaxTagNameChars > 0);
  if (value.size() > kEipMaxTagNameChars)
    return false;

  for (const QChar ch : value)
    if (ch == QLatin1Char('&') || ch == QLatin1Char('='))
      return false;

  return true;
}

//--------------------------------------------------------------------------------------------------
// libplctag seam
//--------------------------------------------------------------------------------------------------
//
// Every call into the stack goes through these four helpers, so the driver body below carries no
// preprocessor branch at all and reads the same whether or not the build has libplctag.
//

#ifdef SS_EIP_ACTIVE

/**
 * @brief Opens one tag handle, or returns -1 with @p error set to what the stack reported.
 */
[[nodiscard]] static int createHandle(const QByteArray& attributes, QString& error)
{
  SS_ASSERT(!attributes.isEmpty(), {
    error = QCoreApplication::translate("EthernetIp", "The tag attributes are empty");
    return -1;
  });

  const int handle = plc_tag_create(attributes.constData(), kEipCreateTimeoutMs);
  const int status = handle < 0 ? handle : plc_tag_status(handle);
  if (status == PLCTAG_STATUS_OK)
    return handle;

  SS_ASSERT_LOG(status != PLCTAG_STATUS_OK);
  error = QString::fromLatin1(plc_tag_decode_error(status));
  if (handle >= 0)
    (void)plc_tag_destroy(handle);

  return -1;
}

/**
 * @brief Destroys every live tag handle and marks its slot free.
 */
static void releaseHandles(QList<int>& handles)
{
  SS_ASSERT_LOG(handles.size() >= 0);

  for (int i = 0; i < handles.size(); ++i) {
    if (handles.at(i) < 0)
      continue;

    (void)plc_tag_destroy(handles.at(i));
    handles[i] = -1;
  }
}

/**
 * @brief Issues one blocking read; false means the controller did not answer in time.
 */
[[nodiscard]] static bool readHandle(int handle)
{
  SS_ASSERT(handle >= 0, return false);
  SS_ASSERT_LOG(kEipReadTimeoutMs > 0);

  return plc_tag_read(handle, kEipReadTimeoutMs) == PLCTAG_STATUS_OK;
}

/**
 * @brief Renders the value the stack holds for @p handle as the wire type the tag declares; an
 *        invalid QVariant means the read cannot be represented and the channel keeps its latch.
 */
[[nodiscard]] static QVariant decodeTag(int handle, IO::Drivers::OpcUaWire::Type type)
{
  using IO::Drivers::OpcUaWire::Type;
  SS_ASSERT(handle >= 0, return {});
  SS_ASSERT(type != Type::Invalid, return {});

  switch (type) {
    case Type::Bool:
      return QVariant(plc_tag_get_uint8(handle, 0) != 0);
    case Type::I8:
      return QVariant(static_cast<int>(plc_tag_get_int8(handle, 0)));
    case Type::U8:
      return QVariant(static_cast<uint>(plc_tag_get_uint8(handle, 0)));
    case Type::I16:
      return QVariant(static_cast<int>(plc_tag_get_int16(handle, 0)));
    case Type::U16:
      return QVariant(static_cast<uint>(plc_tag_get_uint16(handle, 0)));
    case Type::I32:
      return QVariant(static_cast<int>(plc_tag_get_int32(handle, 0)));
    case Type::U32:
      return QVariant(static_cast<qulonglong>(plc_tag_get_uint32(handle, 0)));
    case Type::I64:
      return QVariant(static_cast<qlonglong>(plc_tag_get_int64(handle, 0)));
    case Type::U64:
      return QVariant(static_cast<qulonglong>(plc_tag_get_uint64(handle, 0)));
    case Type::F32:
      return QVariant(static_cast<double>(plc_tag_get_float32(handle, 0)));
    case Type::F64:
      return QVariant(plc_tag_get_float64(handle, 0));
    case Type::Str:
      break;
    case Type::Invalid:
      return {};
  }

  constexpr int cap = IO::Drivers::OpcUaWire::kMaxStringBytes;
  QByteArray text(cap, static_cast<char>(0));
  if (plc_tag_get_string(handle, 0, text.data(), cap - 1) != PLCTAG_STATUS_OK)
    return {};

  return QVariant(QString::fromLatin1(text.constData()));
}

#endif

#ifndef SS_EIP_ACTIVE

/**
 * @brief Stack-less build: no handle can be opened, and the reason says so once.
 */
[[nodiscard]] static int createHandle(const QByteArray& attributes, QString& error)
{
  SS_ASSERT_LOG(!attributes.isEmpty());
  Q_UNUSED(attributes);

  error = QCoreApplication::translate("EthernetIp", "The %1 client is not available in this build")
            .arg(QLatin1String(kEipBackend));
  return -1;
}

/**
 * @brief Stack-less build: no handle was ever created, so there is nothing to release.
 */
static void releaseHandles(QList<int>& handles)
{
  SS_ASSERT_LOG(handles.size() >= 0);
  handles.fill(-1);
}

/**
 * @brief Stack-less build: nothing answers a read.
 */
[[nodiscard]] static bool readHandle(int handle)
{
  SS_ASSERT_LOG(handle >= -1);
  Q_UNUSED(handle);
  return false;
}

/**
 * @brief Stack-less build: nothing decodes.
 */
[[nodiscard]] static QVariant decodeTag(int handle, IO::Drivers::OpcUaWire::Type type)
{
  SS_ASSERT_LOG(handle >= -1);
  SS_ASSERT_LOG(type != IO::Drivers::OpcUaWire::Type::Invalid);
  Q_UNUSED(handle);
  Q_UNUSED(type);
  return {};
}

#endif

//--------------------------------------------------------------------------------------------------
// Poll worker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the worker with no tag handles: they are created on this object's thread inside
 *        connectToPlc(), never in the constructor, which still runs on the GUI thread.
 */
IO::Drivers::EipPollWorker::EipPollWorker()
  : m_open(false)
  , m_reported(false)
  , m_interval(kEipDefaultIntervalMs)
  , m_deadTicks(0)
  , m_frameSlot(0)
  , m_timer(nullptr)
  , m_abort(false)
  , m_readsOk(0)
  , m_readsFailed(0)
  , m_framesPublished(0)
{}

/**
 * @brief Releases every tag handle. The driver joins the thread before destroying the worker, so
 *        this always runs with no poll in flight.
 */
IO::Drivers::EipPollWorker::~EipPollWorker()
{
  shutdown();
}

/**
 * @brief Stores the endpoint and the tag list. Called before the thread starts, so the worker's
 *        state is complete by the time its event loop delivers anything.
 */
void IO::Drivers::EipPollWorker::configure(const QString& host,
                                           const QString& path,
                                           const QString& plcType,
                                           int interval,
                                           QVector<EipTag> tags)
{
  SS_ASSERT(!tags.isEmpty(), return);
  SS_ASSERT_LOG(interval >= kEipMinIntervalMs);

  m_host     = host;
  m_path     = path;
  m_plcType  = plcType;
  m_interval = interval;
  m_tags     = std::move(tags);
  m_values   = QList<QVariant>(m_tags.size());
  m_dirty    = QList<bool>(m_tags.size(), false);
  m_handles  = QList<int>(m_tags.size(), -1);

  qsizetype bytes = OpcUaWire::kHeaderBytes;
  for (const auto& tag : m_tags)
    bytes += OpcUaWire::maxEntryBytes(tag.type);

  const auto reserve = qMin<qsizetype>(bytes, OpcUaWire::kMaxFrameBytes);
  m_frames[0].reserve(reserve);
  m_frames[1].reserve(reserve);
}

/**
 * @brief Renders one tag's libplctag attribute string. An element index becomes part of the CIP
 *        symbolic name, which is how the protocol addresses an array member. The host and path
 *        reaching here are already attribute-safe: configurationOk() refuses the dial otherwise.
 */
QByteArray IO::Drivers::EipPollWorker::attributes(const EipTag& tag) const
{
  SS_ASSERT(!tag.tag.isEmpty(), return {});
  SS_ASSERT_LOG(!m_host.isEmpty());

  const QString path = m_path.trimmed();

  QString name = tag.tag;
  if (tag.element >= 0)
    name += QStringLiteral("[%1]").arg(tag.element);

  QString attribs = QStringLiteral("protocol=ab-eip&gateway=%1&plc=%2&elem_count=1&name=%3")
                      .arg(m_host, m_plcType, name);
  if (!path.isEmpty())
    attribs += QStringLiteral("&path=%1").arg(path);

  return attribs.toLatin1();
}

/**
 * @brief Creates every tag handle, which is what actually opens the CIP session. The RETURN VALUE
 *        is the driver's single dial verdict (spec 0050): a controller that refuses the first tag
 *        fails the attempt here rather than leaving a half-open link behind a retry timer.
 */
bool IO::Drivers::EipPollWorker::connectToPlc()
{
  SS_ASSERT(!m_open, return true);

  m_dialError.clear();
  SS_ASSERT(!m_tags.isEmpty(), return false);

  QElapsedTimer budget;
  budget.start();
  for (int i = 0; i < m_tags.size(); ++i) {
    QString reason;
    const int handle = createHandle(attributes(m_tags.at(i)), reason);
    if (handle < 0) {
      m_dialError = tr("\"%1\": %2").arg(m_tags.at(i).tag, reason);
      shutdown();
      return false;
    }

    m_handles[i] = handle;
    if (budget.hasExpired(kEipDialDeadlineMs) && i + 1 < m_tags.size()) {
      m_dialError = tr("The controller did not open every tag within the connection deadline");
      shutdown();
      return false;
    }
  }

  m_open      = true;
  m_reported  = false;
  m_deadTicks = 0;
  m_timer     = new QTimer(this);
  m_timer->setInterval(m_interval);
  connect(m_timer, &QTimer::timeout, this, &IO::Drivers::EipPollWorker::onPollTick);
  m_timer->start();
  return true;
}

/**
 * @brief Stops polling and destroys every tag handle. Idempotent, because both the driver's
 *        teardown and the destructor reach it.
 */
void IO::Drivers::EipPollWorker::shutdown()
{
  m_open = false;
  if (m_timer) {
    m_timer->stop();
    delete m_timer;
    m_timer = nullptr;
  }

  releaseHandles(m_handles);

  SS_ASSERT_LOG(m_timer == nullptr);
  SS_ASSERT_LOG(!m_open);
}

/**
 * @brief Polls every configured tag once, latches what changed and publishes one delta frame. A
 *        tick where NOTHING answered is counted; libplctag reconnects on its own, so only a run of
 *        fully-dead ticks is treated as a lost link.
 */
void IO::Drivers::EipPollWorker::onPollTick()
{
  if (!m_open || m_abort.load(std::memory_order_relaxed))
    return;

  const auto now       = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 stampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

  int answered = 0;
  for (int i = 0; i < m_tags.size(); ++i) {
    if (m_abort.load(std::memory_order_relaxed))
      return;

    QVariant value;
    if (!readTag(i, value))
      continue;

    ++answered;
    if (m_values.at(i) == value)
      continue;

    m_values[i] = value;
    m_dirty[i]  = true;
  }

  m_deadTicks = answered > 0 ? 0 : m_deadTicks + 1;
  if (m_deadTicks >= kEipDeadTickLimit) {
    reportFailure(tr("The controller stopped answering tag reads"));
    return;
  }

  publishDirtySlots(stampNs);
}

/**
 * @brief Reads one tag and renders it as the declared wire type. An undecodable value counts as a
 *        FAILED read: the caller feeds this call's false to the dead-tick watchdog, so counting it
 *        as a success reports a healthy link right up to the drop it is about to declare.
 */
bool IO::Drivers::EipPollWorker::readTag(int index, QVariant& value)
{
  SS_ASSERT(index >= 0 && index < m_tags.size() && index < m_handles.size(), return false);

  const int handle = m_handles.at(index);
  SS_ASSERT_LOG(handle >= -1);
  if (handle < 0 || !readHandle(handle)) {
    m_readsFailed.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  value = decodeTag(handle, m_tags.at(index).type);
  if (!value.isValid()) {
    m_readsFailed.fetch_add(1, std::memory_order_relaxed);
    return false;
  }

  m_readsOk.fetch_add(1, std::memory_order_relaxed);
  return true;
}

/**
 * @brief Encodes every dirty slot into one OpcUaWire delta frame and hands it to the driver with
 *        the poll's own timestamp. The stamp is captured HERE, before the queued hop, because the
 *        source owns time and a receipt-time stamp on the GUI thread would carry the queue's
 *        latency into every recording.
 */
void IO::Drivers::EipPollWorker::publishDirtySlots(qint64 stampNs)
{
  SS_ASSERT_LOG(m_dirty.size() == m_tags.size());
  SS_ASSERT_LOG(stampNs > 0);

  QByteArray& frame = m_frames[m_frameSlot];
  OpcUaWire::beginFrame(frame);
  for (int i = 0; i < m_tags.size(); ++i) {
    if (!m_dirty.at(i))
      continue;

    if (frame.size() + OpcUaWire::maxEntryBytes(m_tags.at(i).type) > OpcUaWire::kMaxFrameBytes)
      break;

    OpcUaWire::appendEntry(frame, i, m_tags.at(i).type, m_values.at(i));
    m_dirty[i] = false;
  }

  if (frame.size() <= OpcUaWire::kHeaderBytes)
    return;

  m_framesPublished.fetch_add(1, std::memory_order_relaxed);
  Q_EMIT frameReady(frame, stampNs);
  m_frameSlot ^= 1;
}

/**
 * @brief Reports a lost link exactly once and stops polling; the driver turns it into a queued
 *        disconnect so nothing tears the device down from inside this handler.
 */
void IO::Drivers::EipPollWorker::reportFailure(const QString& reason)
{
  SS_ASSERT_LOG(!reason.isEmpty());
  if (m_reported)
    return;

  m_reported = true;
  m_open     = false;
  if (m_timer)
    m_timer->stop();

  Q_EMIT linkLost(reason);
}

/**
 * @brief Signals the in-flight poll to stop reading between tags. Set from the GUI thread before
 *        the blocking teardown invoke so a poll mid-way through N serial reads returns at the next
 *        tag boundary instead of after every remaining read timeout.
 */
void IO::Drivers::EipPollWorker::requestAbort() noexcept
{
  m_abort.store(true, std::memory_order_relaxed);
}

/**
 * @brief Why the last dial failed, read by the driver after the blocking call returns.
 */
const QString& IO::Drivers::EipPollWorker::dialError() const noexcept
{
  return m_dialError;
}

/**
 * @brief Successful tag reads since the session opened.
 */
quint64 IO::Drivers::EipPollWorker::readsOk() const noexcept
{
  return m_readsOk.load(std::memory_order_relaxed);
}

/**
 * @brief Refused or timed-out tag reads since the session opened.
 */
quint64 IO::Drivers::EipPollWorker::readsFailed() const noexcept
{
  return m_readsFailed.load(std::memory_order_relaxed);
}

/**
 * @brief Delta frames handed to the driver since the session opened.
 */
quint64 IO::Drivers::EipPollWorker::framesPublished() const noexcept
{
  return m_framesPublished.load(std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Driver construction and teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the driver, restores persisted settings and wires the configuration signals.
 */
IO::Drivers::EthernetIp::EthernetIp()
  : m_appState(AppState::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_open(false)
  , m_persistent(true)
  , m_plcTypeIndex(0)
  , m_pollInterval(kEipDefaultIntervalMs)
  , m_lastStampNs(0)
  , m_linkDrops(0)
  , m_host(QStringLiteral("192.168.0.1"))
  , m_cipPath(QStringLiteral("1,0"))
  , m_thread(std::make_unique<QThread>())
  , m_worker(nullptr)
{
  loadSettings();

  static constexpr void (EthernetIp::* kConfigSignals[])() = {&EthernetIp::hostChanged,
                                                              &EthernetIp::cipPathChanged,
                                                              &EthernetIp::plcTypeChanged,
                                                              &EthernetIp::pollIntervalChanged,
                                                              &EthernetIp::tagsChanged};
  for (const auto signal : kConfigSignals)
    connect(this, signal, this, &IO::Drivers::EthernetIp::configurationChanged);

  Q_EMIT configurationChanged();
}

/**
 * @brief Joins the poll thread and destroys the worker.
 */
IO::Drivers::EthernetIp::~EthernetIp()
{
  doClose();
}

/**
 * @brief Restores the endpoint and the tag list.
 */
void IO::Drivers::EthernetIp::loadSettings()
{
  m_host         = m_settings.value("EipDriver/host", m_host).toString();
  m_cipPath      = m_settings.value("EipDriver/cipPath", m_cipPath).toString();
  m_plcTypeIndex = qBound(0,
                          m_settings.value("EipDriver/plcType", 0).toInt(),
                          static_cast<int>(std::size(kEipPlcTypes)) - 1);
  m_pollInterval = qBound(kEipMinIntervalMs,
                          m_settings.value("EipDriver/pollInterval", kEipDefaultIntervalMs).toInt(),
                          kEipMaxIntervalMs);

  const auto doc =
    QJsonDocument::fromJson(m_settings.value("EipDriver/tags", QByteArray("[]")).toByteArray());
  if (doc.isArray())
    setTags(doc.array());
}

/**
 * @brief Persists the tag list as compact JSON; only the UI-config instance writes settings.
 */
void IO::Drivers::EthernetIp::saveTags()
{
  if (!m_persistent)
    return;

  m_settings.setValue("EipDriver/tags", QJsonDocument(tagsJson()).toJson(QJsonDocument::Compact));
}

/**
 * @brief Only the UI-config instance writes settings; the per-source live instance is fed by the
 *        manager and must never echo its state back into QSettings.
 */
void IO::Drivers::EthernetIp::setPersistent(const bool persistent) noexcept
{
  if (m_persistent == persistent)
    return;

  m_persistent = persistent;
}

/**
 * @brief Points this UI-config instance at the per-source instance that owns the live session, so
 *        the pane and the API read the counters of the link that is actually running.
 */
void IO::Drivers::EthernetIp::setSessionPeer(EthernetIp* peer)
{
  SS_ASSERT(peer != this, return);
  if (m_sessionPeer == peer)
    return;

  m_sessionPeer = peer;
  if (peer)
    connect(peer,
            &QObject::destroyed,
            this,
            &IO::Drivers::EthernetIp::statusChanged,
            Qt::UniqueConnection);

  Q_EMIT statusChanged();
}

/**
 * @brief The live session whose state answers a status query, or nullptr when this instance is
 *        itself the session; only the persistent instance delegates, so the hop is one deep.
 */
const IO::Drivers::EthernetIp* IO::Drivers::EthernetIp::sessionPeer() const
{
  return m_persistent ? m_sessionPeer.data() : nullptr;
}

/**
 * @brief Closes the session; a user's disconnect is final.
 */
void IO::Drivers::EthernetIp::close()
{
  doClose();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Non-virtual teardown shared by close() and the destructor. The worker releases its tag
 *        handles on its OWN thread first, then the event loop is QUIT before wait(): a thread
 *        whose work is driven from its own event loop never returns from wait() otherwise (HID
 *        cleanupDevice() is the reference).
 */
void IO::Drivers::EthernetIp::doClose()
{
  m_open = false;
  if (!m_worker) {
    SS_ASSERT_LOG(!m_thread->isRunning());
    return;
  }

  disconnect(m_worker, nullptr, this, nullptr);
  if (m_thread->isRunning()) {
    m_worker->requestAbort();
    QMetaObject::invokeMethod(
      m_worker, [this] { m_worker->shutdown(); }, Qt::BlockingQueuedConnection);
    m_thread->quit();
    if (!m_thread->wait(kEipJoinTimeoutMs)) {
      qWarning() << "[EtherNet/IP] poll thread did not stop within" << kEipJoinTimeoutMs
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
 * @brief Returns true while the CIP session is established.
 */
bool IO::Drivers::EthernetIp::isOpen() const noexcept
{
  return m_open;
}

/**
 * @brief Values arrive through the poll worker, so readable means open.
 */
bool IO::Drivers::EthernetIp::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Writing controller tags is out of scope (spec 0073 non-goal).
 */
bool IO::Drivers::EthernetIp::isWritable() const noexcept
{
  return false;
}

/**
 * @brief A host and a tag list whose entries all name a tag and a known type. The endpoint is
 *        checked against the attribute grammar here rather than at dial time, so an unusable host
 *        greys the connect button instead of failing inside the worker.
 */
bool IO::Drivers::EthernetIp::configurationOk() const noexcept
{
  if (m_host.trimmed().isEmpty() || m_tags.isEmpty())
    return false;

  if (!eipAttributeValueSafe(m_host.trimmed()) || !eipAttributeValueSafe(m_cipPath.trimmed()))
    return false;

  return m_tags.size() <= OpcUaWire::kMaxTags;
}

/**
 * @brief The driver never writes; a caller that tries gets a hard failure rather than a silent 0.
 */
qint64 IO::Drivers::EthernetIp::write(const QByteArray& data)
{
  Q_UNUSED(data);
  return -1;
}

/**
 * @brief Opens the session. The worker thread is started, the blocking tag creation runs ON that
 *        thread, and its result is returned straight to the connect fan-out: one attempt, one
 *        verdict, no openFinished latch and no retry stack (spec 0050).
 */
bool IO::Drivers::EthernetIp::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);

  doClose();
  if (!configurationOk()) {
    m_lastError = tr("The connection is not configured: check the host and the tag list");
    Q_EMIT statusChanged();
    return false;
  }

  m_lastError.clear();
  m_worker = new EipPollWorker();
  m_worker->configure(m_host.trimmed(), m_cipPath, plcType(), m_pollInterval, m_tags);
  m_worker->moveToThread(m_thread.get());
  connect(m_worker, &EipPollWorker::frameReady, this, &IO::Drivers::EthernetIp::onFrameReady);
  connect(m_worker, &EipPollWorker::linkLost, this, &IO::Drivers::EthernetIp::onLinkLost);

  m_thread->start();

  bool ok = false;
  QMetaObject::invokeMethod(
    m_worker, [this, &ok] { ok = m_worker->connectToPlc(); }, Qt::BlockingQueuedConnection);

  m_open = ok;
  if (!ok) {
    m_lastError = m_worker->dialError();
    logDriverError(
      tr("EtherNet/IP Connection Failed"),
      tr("\"%1\": %2")
        .arg(m_host, m_lastError.isEmpty() ? tr("the controller did not answer") : m_lastError));
    doClose();
  }

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  return ok;
}

//--------------------------------------------------------------------------------------------------
// Worker signal handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes a delta frame the worker built. The stamp travelled WITH the payload, so it is
 *        the poll's own time; it is only clamped forward here so a rounded steady clock can never
 *        hand the pipeline two frames with the same timestamp.
 */
void IO::Drivers::EthernetIp::onFrameReady(const QByteArray& frame, qint64 stampNs)
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
void IO::Drivers::EthernetIp::onLinkLost(const QString& reason)
{
  m_lastError = reason;
  ++m_linkDrops;
  if (!m_open) {
    Q_EMIT statusChanged();
    return;
  }

  m_open = false;
  logDriverError(tr("EtherNet/IP Connection Lost"), reason);

  static auto& manager = ConnectionManager::instance();
  QMetaObject::invokeMethod(this, [this] { manager.disconnectDevice(this); }, Qt::QueuedConnection);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the gateway host name or IP address.
 */
QString IO::Drivers::EthernetIp::host() const
{
  return m_host;
}

/**
 * @brief Returns the CIP routing path from the gateway to the CPU (for example "1,0").
 */
QString IO::Drivers::EthernetIp::cipPath() const
{
  return m_cipPath;
}

/**
 * @brief Returns the controller family as libplctag's `plc=` attribute value.
 */
QString IO::Drivers::EthernetIp::plcType() const
{
  const int index = qBound(0, m_plcTypeIndex, static_cast<int>(std::size(kEipPlcTypes)) - 1);
  return QString::fromLatin1(kEipPlcTypes[index]);
}

/**
 * @brief Returns the selected row of plcTypeList().
 */
int IO::Drivers::EthernetIp::plcTypeIndex() const
{
  return m_plcTypeIndex;
}

/**
 * @brief Returns the poll interval in milliseconds.
 */
int IO::Drivers::EthernetIp::pollInterval() const
{
  return m_pollInterval;
}

/**
 * @brief Returns the number of configured tags.
 */
int IO::Drivers::EthernetIp::tagCount() const
{
  return static_cast<int>(m_tags.size());
}

/**
 * @brief Returns the controller families this build can address, in persisted order.
 */
QStringList IO::Drivers::EthernetIp::plcTypeList()
{
  QStringList list;
  for (const auto* type : kEipPlcTypes)
    list.append(QString::fromLatin1(type));

  return list;
}

/**
 * @brief Returns the controller families as their vendors capitalize them, ROW-ALIGNED with
 *        plcTypeList(): the picker shows these while the slug at the same row is what the
 *        protocol, the CLI and every persisted project keep carrying.
 */
QStringList IO::Drivers::EthernetIp::plcTypeLabels()
{
  const QStringList labels = {tr("ControlLogix"),
                              tr("CompactLogix"),
                              tr("MicroLogix"),
                              tr("Micro800"),
                              tr("PLC-5"),
                              tr("SLC 500"),
                              tr("Logix PCCC"),
                              tr("Omron NJ/NX")};

  SS_ASSERT(labels.size() == static_cast<int>(std::size(kEipPlcTypes)), return plcTypeList());
  return labels;
}

/**
 * @brief Returns the wire type codes a tag can declare, in the OpcUaWire vocabulary.
 */
QStringList IO::Drivers::EthernetIp::tagTypeList()
{
  return {QStringLiteral("bool"),
          QStringLiteral("i8"),
          QStringLiteral("u8"),
          QStringLiteral("i16"),
          QStringLiteral("u16"),
          QStringLiteral("i32"),
          QStringLiteral("u32"),
          QStringLiteral("i64"),
          QStringLiteral("u64"),
          QStringLiteral("f32"),
          QStringLiteral("f64"),
          QStringLiteral("str")};
}

/**
 * @brief Returns the configured tags in wire order.
 */
const QVector<IO::Drivers::EipTag>& IO::Drivers::EthernetIp::tags() const noexcept
{
  return m_tags;
}

/**
 * @brief One-line session status for the pane, read from the live session when this instance is
 *        the UI-config one.
 */
QString IO::Drivers::EthernetIp::statusText() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusText();

  if (!m_open)
    return m_lastError.isEmpty() ? tr("Not connected") : m_lastError;

  const double hz = 1000.0 / qMax(1, m_pollInterval);
  return tr("Polling %1 tags at %2 Hz").arg(m_tags.size()).arg(hz, 0, 'f', 1);
}

/**
 * @brief Human-readable row for the pane's tag list.
 */
QString IO::Drivers::EthernetIp::tagInfo(const int index) const
{
  if (index < 0 || index >= m_tags.size())
    return {};

  const auto& tag  = m_tags.at(index);
  const auto label = tag.element >= 0
                     ? QStringLiteral("%1[%2]").arg(tag.tag, QString::number(tag.element))
                     : tag.tag;
  return QStringLiteral("%1 (%2) %3").arg(tag.name, OpcUaWire::codeFromType(tag.type), label);
}

/**
 * @brief The tag list as JSON (conn-settings and API shape).
 */
QJsonArray IO::Drivers::EthernetIp::tagsJson() const
{
  QJsonArray array;
  for (const auto& tag : m_tags)
    array.append(QJsonObject{
      {   QStringLiteral("name"),                          tag.name},
      {    QStringLiteral("tag"),                           tag.tag},
      {      QStringLiteral("t"), OpcUaWire::codeFromType(tag.type)},
      {QStringLiteral("element"),                       tag.element},
    });

  return array;
}

/**
 * @brief The `ethernetip` native template schema: one {index, name} entry per wire index.
 */
QJsonArray IO::Drivers::EthernetIp::wireSchema() const
{
  QJsonArray schema;
  for (int i = 0; i < m_tags.size() && i < OpcUaWire::kMaxTags; ++i)
    schema.append(QJsonObject{
      {QStringLiteral("index"),                 i},
      { QStringLiteral("name"), m_tags.at(i).name},
    });

  return schema;
}

/**
 * @brief Pulled diagnostics snapshot (spec 0033: counters, never pushed), read from the live
 *        session when this instance is the UI-config one.
 */
QJsonObject IO::Drivers::EthernetIp::statusJson() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusJson();

  const quint64 reads  = m_worker ? m_worker->readsOk() : 0;
  const quint64 failed = m_worker ? m_worker->readsFailed() : 0;
  const quint64 frames = m_worker ? m_worker->framesPublished() : 0;

  return QJsonObject{
    {      QStringLiteral("connected"),                           m_open},
    {           QStringLiteral("host"),                           m_host},
    {        QStringLiteral("cipPath"),                        m_cipPath},
    {        QStringLiteral("plcType"),                        plcType()},
    {   QStringLiteral("pollInterval"),                   m_pollInterval},
    {       QStringLiteral("tagCount"),                       tagCount()},
    {        QStringLiteral("readsOk"),       static_cast<qint64>(reads)},
    {    QStringLiteral("readsFailed"),      static_cast<qint64>(failed)},
    {QStringLiteral("framesPublished"),      static_cast<qint64>(frames)},
    {      QStringLiteral("linkDrops"), static_cast<qint64>(m_linkDrops)},
    {      QStringLiteral("lastError"),                      m_lastError},
    {     QStringLiteral("statusText"),                     statusText()},
  };
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the gateway host name or IP address.
 */
void IO::Drivers::EthernetIp::setHost(const QString& host)
{
  const auto trimmed = host.trimmed();
  if (m_host == trimmed)
    return;

  m_host = trimmed;
  if (m_persistent)
    m_settings.setValue("EipDriver/host", m_host);

  Q_EMIT hostChanged();
}

/**
 * @brief Sets the CIP routing path; empty addresses the gateway itself.
 */
void IO::Drivers::EthernetIp::setCipPath(const QString& path)
{
  const auto trimmed = path.trimmed();
  if (m_cipPath == trimmed)
    return;

  m_cipPath = trimmed;
  if (m_persistent)
    m_settings.setValue("EipDriver/cipPath", m_cipPath);

  Q_EMIT cipPathChanged();
}

/**
 * @brief Selects the controller family by row of plcTypeList().
 */
void IO::Drivers::EthernetIp::setPlcTypeIndex(const int index)
{
  const int clamped = qBound(0, index, static_cast<int>(std::size(kEipPlcTypes)) - 1);
  if (m_plcTypeIndex == clamped)
    return;

  m_plcTypeIndex = clamped;
  if (m_persistent)
    m_settings.setValue("EipDriver/plcType", m_plcTypeIndex);

  Q_EMIT plcTypeChanged();
}

/**
 * @brief Sets the poll interval in milliseconds; a live session keeps the interval it opened with.
 */
void IO::Drivers::EthernetIp::setPollInterval(const int interval)
{
  const int clamped = qBound(kEipMinIntervalMs, interval, kEipMaxIntervalMs);
  if (m_pollInterval == clamped)
    return;

  m_pollInterval = clamped;
  if (m_persistent)
    m_settings.setValue("EipDriver/pollInterval", m_pollInterval);

  Q_EMIT pollIntervalChanged();
  Q_EMIT statusChanged();
}

/**
 * @brief The wire layout is sized when the worker is configured, so the list is immutable while a
 *        session is live. The per-source instance owns the worker; the UI-config instance owns
 *        none and must consult its live peer, or the pane edits a list the running session already
 *        sized. Silent: the UI-to-live property echo hits this on every configuration change.
 */
bool IO::Drivers::EthernetIp::tagsFrozen() const noexcept
{
  return m_worker != nullptr || sessionPeer() != nullptr;
}

/**
 * @brief Peer-aware frozen flag the tags dialog binds its editing controls to, so the dialog
 *        visibly locks while a session is live rather than silently dropping edits.
 */
bool IO::Drivers::EthernetIp::tagsLocked() const noexcept
{
  return tagsFrozen();
}

/**
 * @brief Replaces the tag list from its JSON shape, dropping unnamed, untyped and duplicate
 *        entries.
 */
void IO::Drivers::EthernetIp::setTags(const QJsonArray& tags)
{
  QVector<EipTag> list;
  QSet<QString> seen;
  for (const auto& item : tags) {
    const auto obj = item.toObject();

    EipTag tag;
    tag.tag     = obj.value(QStringLiteral("tag")).toString().trimmed();
    tag.name    = obj.value(QStringLiteral("name")).toString(tag.tag).trimmed();
    tag.type    = OpcUaWire::typeFromCode(obj.value(QStringLiteral("t")).toString());
    tag.element = obj.value(QStringLiteral("element")).toInt(-1);
    if (tag.tag.isEmpty() || tag.type == OpcUaWire::Type::Invalid || seen.contains(tag.name)
        || !eipTagNameSafe(tag.tag))
      continue;

    if (list.size() >= OpcUaWire::kMaxTags)
      break;

    seen.insert(tag.name);
    list.append(tag);
  }

  if (list == m_tags || tagsFrozen())
    return;

  m_tags = std::move(list);
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief Appends a tag unless its name is taken, its type is unknown or the list is full.
 */
void IO::Drivers::EthernetIp::addTag(const QString& name,
                                     const QString& tag,
                                     const QString& type,
                                     const int element)
{
  const auto trimmedTag  = tag.trimmed();
  const auto trimmedName = name.trimmed().isEmpty() ? trimmedTag : name.trimmed();
  if (trimmedTag.isEmpty() || tagsFrozen() || m_tags.size() >= OpcUaWire::kMaxTags)
    return;

  if (!eipTagNameSafe(trimmedTag)) {
    logDriverError(tr("EtherNet/IP Tag"),
                   tr("The tag name \"%1\" is too long or contains characters that are not "
                      "allowed (no '&', '=' or whitespace).")
                     .arg(trimmedTag));
    return;
  }

  const auto wireType = OpcUaWire::typeFromCode(type);
  if (wireType == OpcUaWire::Type::Invalid) {
    logDriverError(tr("EtherNet/IP Tag"), tr("Unknown tag type \"%1\".").arg(type));
    return;
  }

  for (const auto& existing : m_tags)
    if (existing.name == trimmedName)
      return;

  m_tags.append(EipTag{trimmedName, trimmedTag, wireType, qMax(-1, element)});
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief Removes the tag at the given position.
 */
void IO::Drivers::EthernetIp::removeTag(const int index)
{
  if (index < 0 || index >= m_tags.size() || tagsFrozen())
    return;

  m_tags.removeAt(index);
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief Drops every configured tag.
 */
void IO::Drivers::EthernetIp::clearTags()
{
  if (m_tags.isEmpty() || tagsFrozen())
    return;

  m_tags.clear();
  saveTags();
  Q_EMIT tagsChanged();
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat editable property list; the tag list rides along so projects capture it.
 */
QList<IO::DriverProperty> IO::Drivers::EthernetIp::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty host;
  host.key   = QStringLiteral("host");
  host.label = tr("Gateway");
  host.type  = IO::DriverProperty::Text;
  host.value = m_host;
  props.append(host);

  IO::DriverProperty path;
  path.key   = QStringLiteral("cipPath");
  path.label = tr("CIP Path");
  path.type  = IO::DriverProperty::Text;
  path.value = m_cipPath;
  props.append(path);

  IO::DriverProperty family;
  family.key     = QStringLiteral("plcType");
  family.label   = tr("Controller Family");
  family.type    = IO::DriverProperty::ComboBox;
  family.value   = m_plcTypeIndex;
  family.options = plcTypeLabels();
  props.append(family);

  IO::DriverProperty interval;
  interval.key   = QStringLiteral("pollInterval");
  interval.label = tr("Poll Interval (ms)");
  interval.type  = IO::DriverProperty::IntField;
  interval.value = m_pollInterval;
  interval.min   = kEipMinIntervalMs;
  interval.max   = kEipMaxIntervalMs;
  props.append(interval);

  IO::DriverProperty tags;
  tags.key   = QStringLiteral("tags");
  tags.type  = IO::DriverProperty::Text;
  tags.value = QVariant::fromValue(tagsJson());
  props.append(tags);

  return props;
}

/**
 * @brief Applies a single configuration change by key; the twin of driverProperties(), and the
 *        half a project round-trip dies without.
 */
void IO::Drivers::EthernetIp::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("host")) {
    setHost(value.toString());
    return;
  }

  if (key == QLatin1String("cipPath")) {
    setCipPath(value.toString());
    return;
  }

  if (key == QLatin1String("plcType")) {
    if (value.typeId() == QMetaType::QString)
      setPlcTypeIndex(static_cast<int>(plcTypeList().indexOf(value.toString())));
    else
      setPlcTypeIndex(value.toInt());

    return;
  }

  if (key == QLatin1String("pollInterval")) {
    setPollInterval(value.toInt());
    return;
  }

  if (key != QLatin1String("tags"))
    return;

  QJsonArray array;
  if (value.canConvert<QJsonArray>())
    array = value.toJsonArray();
  else if (value.typeId() == QMetaType::QVariantList) {
    const auto list = value.toList();
    for (const auto& item : list)
      array.append(QJsonValue::fromVariant(item));
  }

  setTags(array);
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief One dataset per tag: LED for booleans, plot for numerics, plain text for strings.
 */
DataModel::Dataset IO::Drivers::EthernetIp::datasetFor(const EipTag& tag, int index)
{
  SS_ASSERT_LOG(index >= 1);
  SS_ASSERT_LOG(!tag.name.isEmpty());

  DataModel::Dataset dataset;
  dataset.index = index;
  dataset.log   = true;
  dataset.title = tag.name;

  if (tag.type == OpcUaWire::Type::Bool) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (tag.type != OpcUaWire::Type::Str && tag.type != OpcUaWire::Type::Invalid)
    dataset.plt = true;

  return dataset;
}

/**
 * @brief One group per CIP program scope, one dataset per wire index, the ethernetip template.
 */
QJsonObject IO::Drivers::EthernetIp::buildProject() const
{
  QJsonObject project;
  project[Keys::Title]   = tr("EtherNet/IP Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("EtherNet/IP");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::EthernetIp);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("ethernetip");
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
  for (int i = 0; i < m_tags.size(); ++i) {
    const auto& tag   = m_tags.at(i);
    const QString key = tag.tag.startsWith(QLatin1String("Program:"))
                        ? tag.tag.section(QLatin1Char('.'), 0, 0)
                        : tr("Controller Tags");
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = key;
      groups.insert(key, group);
      order.append(key);
    }

    groups[key].datasets.push_back(datasetFor(tag, i + 1));
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
DataModel::ProjectModel* IO::Drivers::EthernetIp::loadGeneratedProject()
{
  if (m_tags.isEmpty())
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
 * @brief Generates a project from the tag list and opens it in the editor.
 */
void IO::Drivers::EthernetIp::generateProject()
{
  if (m_tags.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No tags configured"),
                                    tr("Add at least one tag before generating a project."),
                                    QMessageBox::Warning,
                                    tr("EtherNet/IP Project Generator"));
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
        tr("EtherNet/IP Project Generator"));
    },
    Qt::SingleShotConnection);

  (void)pm->saveJsonFile(true);
}
