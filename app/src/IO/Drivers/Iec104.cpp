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

#include "IO/Drivers/Iec104.h"

#include <chrono>
#include <QHash>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStringList>
#include <QTcpSocket>
#include <QTimer>
#include <utility>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

using namespace IO::Drivers::Iec104Proto;

static constexpr int kIec104DialDeadlineMs         = 5000;
static constexpr int kIec104ProtocolTickMs         = 100;
static constexpr int kIec104MaxCommonAddress       = 65535;
static constexpr int kIec104MaxRxBytes             = 262144;
static constexpr int kIec104ReassemblyReserveBytes = 8192;
static constexpr int kIec104MaxFramesPerRead       = 512;
static constexpr int kIec104MaxSkewMs              = 86400000;
static constexpr qint64 kIec104NsPerMs             = 1000000;

/**
 * @brief Coerces whatever a project file, the API or a control script stored under a JSON-valued
 *        driver property back into a JSON array; anything else reads as an empty list.
 */
[[nodiscard]] static QJsonArray asJsonArray(const QVariant& value)
{
  if (value.canConvert<QJsonArray>())
    return value.toJsonArray();

  if (value.typeId() != QMetaType::QVariantList)
    return {};

  QJsonArray array;
  const auto list = value.toList();
  for (const auto& item : list)
    array.append(QJsonValue::fromVariant(item));

  return array;
}

//--------------------------------------------------------------------------------------------------
// Construction and teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the driver, restores persisted settings and wires the configuration signals.
 */
IO::Drivers::Iec104::Iec104()
  : m_appState(AppState::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_open(false)
  , m_loading(true)
  , m_started(false)
  , m_persistent(true)
  , m_clockValid(false)
  , m_port(Iec104Proto::kDefaultPort)
  , m_commonAddress(1)
  , m_windowK(kDefaultK)
  , m_windowW(kDefaultW)
  , m_timeoutT1(kDefaultT1Ms)
  , m_timeoutT2(kDefaultT2Ms)
  , m_timeoutT3(kDefaultT3Ms)
  , m_lastStampNs(0)
  , m_clockOffsetNs(0)
  , m_stationBaseMs(0)
  , m_linkDrops(0)
  , m_badQualityPoints(0)
  , m_skippedAsdus(0)
  , m_testTimeouts(0)
  , m_unstampedPoints(0)
  , m_framesPublished(0)
  , m_dirtyCount(0)
  , m_frameReserveBytes(OpcUaWire::kHeaderBytes)
  , m_host(QStringLiteral("127.0.0.1"))
  , m_timer(new QTimer(this))
  , m_socket(new QTcpSocket(this))
{
  loadSettings();
  applyProtocolParameters();
  m_loading = false;

  m_timer->setInterval(kIec104ProtocolTickMs);
  connect(m_timer, &QTimer::timeout, this, &IO::Drivers::Iec104::onProtocolTick);
  connect(&m_dial, &IO::AsyncTcpDial::finished, this, &IO::Drivers::Iec104::onDialFinished);

  static constexpr void (Iec104::* kConfigSignals[])() = {&Iec104::hostChanged,
                                                          &Iec104::portChanged,
                                                          &Iec104::pointsChanged,
                                                          &Iec104::commonAddressChanged,
                                                          &Iec104::protocolParametersChanged};
  for (const auto signal : kConfigSignals)
    connect(this, signal, this, &IO::Drivers::Iec104::configurationChanged);

  Q_EMIT configurationChanged();
}

/**
 * @brief Closes the link and releases the socket.
 */
IO::Drivers::Iec104::~Iec104()
{
  doClose();
}

/**
 * @brief Restores the endpoint, the protocol parameters and the point table discovered by an
 *        earlier session; the table is what keeps a generated project's slot indices stable.
 */
void IO::Drivers::Iec104::loadSettings()
{
  m_host = m_settings.value("Iec104Driver/host", m_host).toString();
  m_port =
    qBound(1, m_settings.value("Iec104Driver/port", Iec104Proto::kDefaultPort).toInt(), 65535);
  m_commonAddress = qBound(0,
                           m_settings.value("Iec104Driver/commonAddress", m_commonAddress).toInt(),
                           kIec104MaxCommonAddress);

  m_windowK   = m_settings.value("Iec104Driver/windowK", kDefaultK).toInt();
  m_windowW   = m_settings.value("Iec104Driver/windowW", kDefaultW).toInt();
  m_timeoutT1 = m_settings.value("Iec104Driver/timeoutT1", kDefaultT1Ms).toInt();
  m_timeoutT2 = m_settings.value("Iec104Driver/timeoutT2", kDefaultT2Ms).toInt();
  m_timeoutT3 = m_settings.value("Iec104Driver/timeoutT3", kDefaultT3Ms).toInt();

  const auto doc = QJsonDocument::fromJson(
    m_settings.value("Iec104Driver/points", QByteArray("[]")).toByteArray());
  if (doc.isArray())
    setPoints(doc.array());
}

/**
 * @brief Persists the discovered point table; only the UI-config instance writes settings, and
 *        never while the constructor is still restoring them back out of QSettings.
 */
void IO::Drivers::Iec104::savePoints()
{
  if (!m_persistent || m_loading)
    return;

  m_settings.setValue("Iec104Driver/points",
                      QJsonDocument(pointsJson()).toJson(QJsonDocument::Compact));
}

/**
 * @brief Sizes the delta-frame buffer from the discovered point table and reserves its worst case
 *        once, so the steady-state publish appends in place instead of regrowing every tick.
 */
void IO::Drivers::Iec104::reserveFrame()
{
  qsizetype bytes = OpcUaWire::kHeaderBytes;
  for (const auto& point : m_points)
    bytes += OpcUaWire::maxEntryBytes(wireTypeFor(point.kind));

  m_frameReserveBytes = qMin<qsizetype>(bytes, OpcUaWire::kMaxFrameBytes);
  m_frame             = QByteArray();
  m_frame.reserve(m_frameReserveBytes);
}

/**
 * @brief Hands the windows and deadlines to the APCI state machine, which clamps them.
 */
void IO::Drivers::Iec104::applyProtocolParameters()
{
  m_link.configure(m_windowK, m_windowW, m_timeoutT1, m_timeoutT2, m_timeoutT3);

  m_windowK   = m_link.windowK();
  m_windowW   = m_link.windowW();
  m_timeoutT1 = m_link.t1Ms();
  m_timeoutT2 = m_link.t2Ms();
  m_timeoutT3 = m_link.t3Ms();
}

/**
 * @brief Only the UI-config instance writes settings; the per-source live instance is fed by the
 *        manager and must never echo its state back into QSettings.
 */
void IO::Drivers::Iec104::setPersistent(const bool persistent) noexcept
{
  if (m_persistent == persistent)
    return;

  m_persistent = persistent;
}

/**
 * @brief Points this UI-config instance at the per-source instance that owns the live session, so
 *        the pane and the API read the counters and the discovered points of the running link.
 */
void IO::Drivers::Iec104::setSessionPeer(Iec104* peer)
{
  SS_ASSERT(peer != this, return);

  if (m_sessionPeer == peer)
    return;

  m_sessionPeer = peer;
  if (peer) {
    connect(
      peer, &QObject::destroyed, this, &IO::Drivers::Iec104::statusChanged, Qt::UniqueConnection);
    connect(peer,
            &IO::Drivers::Iec104::pointsChanged,
            this,
            &IO::Drivers::Iec104::pointsChanged,
            Qt::UniqueConnection);
  }

  Q_EMIT statusChanged();
  Q_EMIT pointsChanged();
}

/**
 * @brief The live session whose state answers a status query, or nullptr when this instance is
 *        itself the session; only the persistent instance delegates, so the hop is one deep.
 */
const IO::Drivers::Iec104* IO::Drivers::Iec104::sessionPeer() const
{
  return m_persistent ? m_sessionPeer.data() : nullptr;
}

/**
 * @brief The point table a query should read: the live session's when there is one, so the pane
 *        shows the points the station is actually reporting rather than a stale restored list.
 */
const QVector<IO::Drivers::Iec104Point>& IO::Drivers::Iec104::pointTable() const noexcept
{
  if (const auto* peer = sessionPeer())
    return peer->m_points;

  return m_points;
}

/**
 * @brief Copies the live session's discovered points into this instance so they persist and so a
 *        generated project keeps its slot indices across restarts. The three parallel value arrays
 *        are resized alongside the point table so every mutator keeps them in lockstep. A no-op on
 *        the live instance.
 */
void IO::Drivers::Iec104::adoptDiscoveredPoints()
{
  const auto* peer = sessionPeer();
  if (!peer || peer->m_points == m_points)
    return;

  m_points = peer->m_points;
  m_slotForKey.clear();
  for (int i = 0; i < m_points.size(); ++i)
    m_slotForKey.insert(slotKey(m_points.at(i).ioa, m_points.at(i).typeId), i);

  m_values     = QList<QVariant>(m_points.size());
  m_stamps     = QList<qint64>(m_points.size(), -1);
  m_dirty      = QList<bool>(m_points.size(), false);
  m_dirtyCount = 0;

  savePoints();
  Q_EMIT pointsChanged();
}

/**
 * @brief Closes the session; a user's disconnect is final.
 */
void IO::Drivers::Iec104::close()
{
  doClose();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Non-virtual teardown shared by close() and the destructor. The handlers are dropped
 *        BEFORE the socket is torn down so a queued error cannot re-enter the failure funnel and
 *        report a drop the user already asked for.
 */
void IO::Drivers::Iec104::doClose()
{
  m_open    = false;
  m_started = false;

  m_dial.cancel();
  m_timer->stop();
  disconnect(m_socket, &QTcpSocket::readyRead, this, &IO::Drivers::Iec104::onReadyRead);
  disconnect(m_socket, &QTcpSocket::errorOccurred, this, &IO::Drivers::Iec104::onSocketError);

  m_socket->abort();
  m_socket->close();

  m_rx.clear();
  m_clockValid    = false;
  m_lastStampNs   = 0;
  m_clockOffsetNs = 0;
  m_stationBaseMs = 0;
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while the APCI link is established.
 */
bool IO::Drivers::Iec104::isOpen() const noexcept
{
  return m_open;
}

/**
 * @brief Returns true while the dial started by open() has neither connected nor failed; the
 *        connect button reads "Connecting" from this instead of looking like a dead click.
 */
bool IO::Drivers::Iec104::isConnecting() const noexcept
{
  return m_dial.active();
}

/**
 * @brief Values arrive through the socket, so readable means open.
 */
bool IO::Drivers::Iec104::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief The control direction is out of scope (spec 0073 non-goal): this client only monitors.
 */
bool IO::Drivers::Iec104::isWritable() const noexcept
{
  return false;
}

/**
 * @brief A host and a port are all the station needs; the point list is discovered, not declared.
 */
bool IO::Drivers::Iec104::configurationOk() const noexcept
{
  return !m_host.trimmed().isEmpty() && m_port > 0;
}

/**
 * @brief The driver never writes user data; a caller that tries gets a hard failure, not a
 *        silent 0 that would look like a successful send of nothing.
 */
qint64 IO::Drivers::Iec104::write(const QByteArray& data)
{
  Q_UNUSED(data)
  return -1;
}

/**
 * @brief Starts the dial. The attempt runs asynchronously and settles through openFinished()
 *        exactly once (spec 0050): a blocking waitForConnected() here froze the window on every
 *        unreachable station, resolver included.
 */
bool IO::Drivers::Iec104::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  doClose();
  if (!configurationOk()) {
    m_lastError = tr("The connection is not configured: check the host and the port");
    Q_EMIT statusChanged();
    return false;
  }

  dialStation();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  return m_dial.active() || isOpen();
}

/**
 * @brief Dials the station with the driver's own socket, exactly once and with no refusal probe:
 *        strict 104 stations permit a single client and would count a probe socket as a second
 *        one. The handlers are wired only once the dial reports success.
 */
void IO::Drivers::Iec104::dialStation()
{
  SS_ASSERT(m_port > 0, return);
  SS_ASSERT_LOG(m_socket->state() == QAbstractSocket::UnconnectedState);

  m_dial.setProbeEnabled(false);
  m_dial.setDeadline(kIec104DialDeadlineMs);
  m_dial.start(m_host.trimmed(), static_cast<quint16>(m_port), m_socket, QIODevice::ReadWrite);
}

/**
 * @brief Settles the dial verdict exactly once: a failure reports through openFinished() and
 *        leaves the link down, a success wires the handlers and starts the session.
 */
void IO::Drivers::Iec104::onDialFinished(bool ok, const QString& reason)
{
  if (!ok) {
    m_lastError = reason;
    m_socket->abort();
    logDriverError(
      tr("IEC 104 Connection Failed"),
      tr("Cannot connect to %1:%2 (%3)").arg(m_host.trimmed(), QString::number(m_port), reason));

    Q_EMIT statusChanged();
    reportOpenFinished(false, reason);
    return;
  }

  beginSession();
  reportOpenFinished(true);
}

/**
 * @brief Wires the socket handlers and sends STARTDT once the link is up. Split out of open() so
 *        the sequence runs at the same point whether the dial settled early or late.
 */
void IO::Drivers::Iec104::beginSession()
{
  connect(m_socket,
          &QTcpSocket::readyRead,
          this,
          &IO::Drivers::Iec104::onReadyRead,
          Qt::UniqueConnection);
  connect(m_socket,
          &QTcpSocket::errorOccurred,
          this,
          &IO::Drivers::Iec104::onSocketError,
          Qt::UniqueConnection);

  m_lastError.clear();
  m_open = true;
  m_rx.reserve(kIec104ReassemblyReserveBytes);
  reserveFrame();
  applyProtocolParameters();
  m_link.reset(monotonicMs());
  sendApdu(m_link.encodeUnnumbered(UFunction::StartDtAct, monotonicMs()));
  m_timer->start();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Socket and protocol handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes one encoded APDU. An empty array means the encoder refused the frame (a closed
 *        send window), which is a queue-or-drop decision the caller already made.
 */
void IO::Drivers::Iec104::sendApdu(const QByteArray& apdu)
{
  if (apdu.isEmpty() || !m_open)
    return;

  SS_ASSERT_LOG(apdu.size() >= kApciBytes);
  (void)m_socket->write(apdu);
}

/**
 * @brief Buffers whatever arrived and decodes every complete APDU in it. Bytes are read STRAIGHT
 *        into the reassembly buffer open() reserved once, because readAll() would build a second
 *        array per wake-up only to copy it away. A peer that never completes a frame would
 *        otherwise grow the buffer without limit, so an overrun drops the link instead.
 */
void IO::Drivers::Iec104::onReadyRead()
{
  if (!m_open)
    return;

  const qint64 available = m_socket->bytesAvailable();
  if (available <= 0)
    return;

  const qsizetype used = m_rx.size();
  if (used + available > kIec104MaxRxBytes) {
    reportLinkLost(tr("The station sent more than the receive buffer holds"));
    return;
  }

  m_rx.resize(used + static_cast<qsizetype>(available));
  const qint64 read = m_socket->read(m_rx.data() + used, available);
  m_rx.resize(used + static_cast<qsizetype>(qMax<qint64>(0, read)));
  drainReceiveBuffer(monotonicMs());
}

/**
 * @brief An established link that errors reports once and stays down; post-drop recovery is not
 *        this driver's business (spec 0050).
 */
void IO::Drivers::Iec104::onSocketError()
{
  reportLinkLost(m_socket->errorString());
}

/**
 * @brief Decodes every complete APDU sitting in the receive buffer. A malformed or out-of-order
 *        frame ends the session: the specification has no resynchronization rule, and guessing
 *        where the next frame starts is how a decoder publishes noise as telemetry.
 */
void IO::Drivers::Iec104::drainReceiveBuffer(qint64 nowMs)
{
  const qsizetype discovered = m_points.size();
  qsizetype offset           = 0;
  for (int guard = 0; guard < kIec104MaxFramesPerRead && offset < m_rx.size(); ++guard) {
    const auto view = QByteArrayView(m_rx).sliced(offset);

    Apdu apdu;
    const auto result = m_link.consume(view, apdu, nowMs);
    if (result == ParseResult::NeedMore)
      break;

    if (result != ParseResult::Ok) {
      m_rx.clear();
      reportLinkLost(tr("The station sent a frame this link could not decode"));
      return;
    }

    offset += apdu.apduSize;
    if (apdu.type == FrameType::Unnumbered)
      handleUnnumbered(apdu.function, nowMs);
    else if (apdu.type == FrameType::Information)
      handleInformation(view.sliced(apdu.asduOffset, apdu.asduSize));
  }

  if (offset > 0)
    m_rx.remove(0, offset);

  if (m_points.size() != discovered)
    Q_EMIT pointsChanged();
}

/**
 * @brief Answers the U-format functions a monitoring client owes an answer to, and starts the
 *        station interrogation once data transfer is confirmed.
 */
void IO::Drivers::Iec104::handleUnnumbered(UFunction function, qint64 nowMs)
{
  if (function == UFunction::TestFrAct) {
    sendApdu(m_link.encodeUnnumbered(UFunction::TestFrCon, nowMs));
    return;
  }

  if (function == UFunction::StopDtAct) {
    sendApdu(m_link.encodeUnnumbered(UFunction::StopDtCon, nowMs));
    return;
  }

  if (function != UFunction::StartDtCon || m_started)
    return;

  m_started          = true;
  const auto request = encodeInterrogation(static_cast<quint16>(m_commonAddress), kQoiStation);
  sendApdu(m_link.encodeInformation(request, nowMs));
}

/**
 * @brief Decodes one ASDU into points. The six-octet header is read first so a frame from another
 *        station is dropped on the common-address mismatch before the object-list walk runs; only a
 *        matching frame is fully decoded. A type this build does not know is COUNTED and skipped:
 *        its object list has an unknown stride, so walking it would publish the wrong octets.
 */
void IO::Drivers::Iec104::handleInformation(QByteArrayView asdu)
{
  Header header;
  if (decodeHeader(asdu, header) != DecodeResult::Ok) {
    ++m_skippedAsdus;
    return;
  }

  if (header.commonAddress != static_cast<quint16>(m_commonAddress))
    return;

  QList<Iec104Proto::Point> points;
  if (decode(asdu, header, points) != DecodeResult::Ok) {
    ++m_skippedAsdus;
    return;
  }

  for (const auto& point : points)
    ingestPoint(point);
}

/**
 * @brief Latches one decoded point into its slot. An invalid point is counted and NOT latched:
 *        the station is saying its own reading is untrustworthy, and overwriting the last good
 *        value with it would present the failure as data.
 */
void IO::Drivers::Iec104::ingestPoint(const Iec104Proto::Point& point)
{
  const int slot = slotForPoint(point);
  if (slot < 0)
    return;

  if (point.quality & QualityInvalid) {
    ++m_badQualityPoints;
    return;
  }

  if (point.quality != QualityGood)
    ++m_badQualityPoints;

  if (m_values.at(slot) == point.value)
    return;

  m_values[slot] = point.value;
  m_stamps[slot] = point.timeValid ? point.timeMsecs : -1;
  if (!m_dirty.at(slot)) {
    m_dirty[slot] = true;
    ++m_dirtyCount;
  }
}

/**
 * @brief Resolves the wire slot of a point, appending it the first time the station reports it.
 *        Slots are only ever appended: renumbering repoints every dataset of a generated project.
 *        The identity is (address, type id), and the LIVE kind wins over the restored one, because
 *        the station is the authority on what it is sending.
 */
int IO::Drivers::Iec104::slotForPoint(const Iec104Proto::Point& point)
{
  const auto known = m_slotForKey.constFind(slotKey(point.ioa, point.typeId));
  if (known != m_slotForKey.constEnd()) {
    const int slot = known.value();
    if (slot >= 0 && slot < m_points.size())
      m_points[slot].kind = point.kind;

    return slot;
  }

  if (m_points.size() >= OpcUaWire::kMaxTags)
    return -1;

  Iec104Point discovered;
  discovered.ioa    = point.ioa;
  discovered.typeId = point.typeId;
  discovered.kind   = point.kind;

  const int slot = static_cast<int>(m_points.size());
  m_points.append(discovered);
  m_slotForKey.insert(slotKey(point.ioa, point.typeId), slot);
  m_values.append(QVariant());
  m_stamps.append(-1);
  m_dirty.append(false);

  const int entryBytes = OpcUaWire::maxEntryBytes(wireTypeFor(point.kind));
  m_frameReserveBytes =
    qMin<qsizetype>(OpcUaWire::kMaxFrameBytes, m_frameReserveBytes + entryBytes);

  SS_ASSERT_LOG(m_values.size() == m_points.size());
  return slot;
}

/**
 * @brief Protocol tick: enforces t1, honours the t2/w acknowledgement obligation, keeps an idle
 *        link alive with TESTFR at t3, and publishes whatever changed since the previous tick.
 */
void IO::Drivers::Iec104::onProtocolTick()
{
  if (!m_open)
    return;

  const qint64 now = monotonicMs();
  if (m_link.confirmOverdue(now)) {
    ++m_testTimeouts;
    reportLinkLost(tr("The station did not answer within t1"));
    return;
  }

  if (m_link.ackDue(now))
    sendApdu(m_link.encodeSupervisory(now));

  if (m_link.testDue(now))
    sendApdu(m_link.encodeUnnumbered(UFunction::TestFrAct, now));

  publishDeltaFrame();
}

/**
 * @brief Encodes every dirty slot into one OpcUaWire delta frame, stamped with the EARLIEST station
 *        time it carries so the source owns time. A tick with nothing dirty returns before touching
 *        the buffer; the buffer is HANDED off and re-reserved rather than reused, so the next
 *        beginFrame cannot detach a copy the pipeline still holds. Overflow slots stay dirty.
 */
void IO::Drivers::Iec104::publishDeltaFrame()
{
  SS_ASSERT_LOG(m_dirty.size() == m_points.size());
  if (m_dirtyCount <= 0)
    return;

  OpcUaWire::beginFrame(m_frame);

  qint64 earliest = -1;
  for (int i = 0; i < m_points.size(); ++i) {
    if (!m_dirty.at(i))
      continue;

    const auto type = wireTypeFor(m_points.at(i).kind);
    if (m_frame.size() + OpcUaWire::maxEntryBytes(type) > OpcUaWire::kMaxFrameBytes)
      break;

    OpcUaWire::appendEntry(m_frame, i, type, m_values.at(i));
    if (m_stamps.at(i) >= 0 && (earliest < 0 || m_stamps.at(i) < earliest))
      earliest = m_stamps.at(i);

    m_dirty[i] = false;
    --m_dirtyCount;
  }

  if (m_frame.size() <= OpcUaWire::kHeaderBytes)
    return;

  ++m_framesPublished;
  publishReceivedData(std::move(m_frame), toSteady(earliest));
  m_frame = QByteArray();
  m_frame.reserve(m_frameReserveBytes);
}

/**
 * @brief Maps a station timestamp onto the steady clock through the offset sampled when the first
 *        stamped point arrived, so an un-NTP'd station is FOLLOWED rather than rejected. A missing
 *        or wildly skewed reading falls back to now and counts. The result never goes backwards.
 */
IO::CapturedData::SteadyTimePoint IO::Drivers::Iec104::toSteady(qint64 stationMs)
{
  const auto now = CapturedData::SteadyClock::now();
  const qint64 nowNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

  if (stationMs >= 0 && !m_clockValid) {
    m_clockValid    = true;
    m_stationBaseMs = stationMs;
    m_clockOffsetNs = nowNs;
  }

  qint64 stamp = nowNs;
  if (stationMs < 0 || !m_clockValid)
    ++m_unstampedPoints;
  else {
    const qint64 elapsedMs = (nowNs - m_clockOffsetNs) / kIec104NsPerMs;
    const qint64 skewMs    = stationMs - m_stationBaseMs - elapsedMs;
    if (skewMs > kIec104MaxSkewMs || skewMs < -kIec104MaxSkewMs)
      ++m_unstampedPoints;
    else
      stamp = m_clockOffsetNs + (stationMs - m_stationBaseMs) * kIec104NsPerMs;
  }

  stamp         = qMax(stamp, m_lastStampNs + 1);
  m_lastStampNs = stamp;
  return CapturedData::SteadyTimePoint(std::chrono::nanoseconds(stamp));
}

/**
 * @brief The link dropped: report once, then let the manager close the device on the next
 *        event-loop turn so nothing tears the driver down from inside its own handler. This is
 *        never sessionClosed() -- a drop is a link event and the session outlives it.
 */
void IO::Drivers::Iec104::reportLinkLost(const QString& reason)
{
  m_lastError = reason;
  ++m_linkDrops;
  if (!m_open) {
    Q_EMIT statusChanged();
    return;
  }

  m_open    = false;
  m_started = false;
  m_timer->stop();
  logDriverError(tr("IEC 104 Connection Lost"), reason);

  static auto& manager = ConnectionManager::instance();
  QMetaObject::invokeMethod(this, [this] { manager.disconnectDevice(this); }, Qt::QueuedConnection);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief The driver's own monotonic millisecond clock; the APCI layer owns no timer and is driven
 *        entirely by the value this returns.
 */
qint64 IO::Drivers::Iec104::monotonicMs()
{
  const auto now = CapturedData::SteadyClock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the station's host name or IP address.
 */
QString IO::Drivers::Iec104::host() const
{
  return m_host;
}

/**
 * @brief Returns the TCP port; 2404 is the port the specification assigns.
 */
int IO::Drivers::Iec104::port() const
{
  return m_port;
}

/**
 * @brief Returns the common address of ASDU this client accepts.
 */
int IO::Drivers::Iec104::commonAddress() const
{
  return m_commonAddress;
}

/**
 * @brief Returns the maximum number of unacknowledged I-frames.
 */
int IO::Drivers::Iec104::windowK() const
{
  return m_windowK;
}

/**
 * @brief Returns the number of received I-frames that forces an acknowledgement.
 */
int IO::Drivers::Iec104::windowW() const
{
  return m_windowW;
}

/**
 * @brief Returns the send/confirm timeout in milliseconds.
 */
int IO::Drivers::Iec104::timeoutT1() const
{
  return m_timeoutT1;
}

/**
 * @brief Returns the acknowledgement timeout in milliseconds.
 */
int IO::Drivers::Iec104::timeoutT2() const
{
  return m_timeoutT2;
}

/**
 * @brief Returns the idle-test timeout in milliseconds.
 */
int IO::Drivers::Iec104::timeoutT3() const
{
  return m_timeoutT3;
}

/**
 * @brief Returns the number of discovered information objects.
 */
int IO::Drivers::Iec104::pointCount() const
{
  return static_cast<int>(pointTable().size());
}

/**
 * @brief Returns the discovered points in wire order.
 */
const QVector<IO::Drivers::Iec104Point>& IO::Drivers::Iec104::points() const noexcept
{
  return pointTable();
}

/**
 * @brief The channel name a point takes on the dashboard. The information object address is the
 *        only identity a station publishes, so it is the name.
 */
QString IO::Drivers::Iec104::pointName(const Iec104Point& point)
{
  return QStringLiteral("IOA %1").arg(point.ioa);
}

/**
 * @brief One-line session status for the pane, read from the live session when this instance is
 *        the UI-config one.
 */
QString IO::Drivers::Iec104::statusText() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusText();

  if (!m_open)
    return m_lastError.isEmpty() ? tr("Not connected") : m_lastError;

  if (!m_started)
    return tr("Starting data transfer…");

  return tr("Monitoring %1 point(s) from station %2").arg(m_points.size()).arg(m_commonAddress);
}

/**
 * @brief Human-readable row for the pane's point list.
 */
QString IO::Drivers::Iec104::pointInfo(const int index) const
{
  const auto& table = pointTable();
  if (index < 0 || index >= table.size())
    return {};

  const auto& point = table.at(index);
  return QStringLiteral("%1 (TI %2) %3")
    .arg(pointName(point))
    .arg(point.typeId)
    .arg(OpcUaWire::codeFromType(wireTypeFor(point.kind)));
}

/**
 * @brief The point table as JSON (conn-settings and API shape).
 */
QJsonArray IO::Drivers::Iec104::pointsJson() const
{
  QJsonArray array;
  for (const auto& point : pointTable())
    array.append(QJsonObject{
      {   QStringLiteral("ioa"), static_cast<qint64>(point.ioa)},
      {QStringLiteral("typeId"), static_cast<int>(point.typeId)},
    });

  return array;
}

/**
 * @brief The `iec104` native template schema: one {index, name} entry per wire index.
 */
QJsonArray IO::Drivers::Iec104::wireSchema() const
{
  const auto& table = pointTable();

  QJsonArray schema;
  for (int i = 0; i < table.size() && i < OpcUaWire::kMaxTags; ++i)
    schema.append(QJsonObject{
      {QStringLiteral("index"),                      i},
      { QStringLiteral("name"), pointName(table.at(i))},
    });

  return schema;
}

/**
 * @brief Pulled diagnostics snapshot (spec 0033: counters, never pushed), read from the live
 *        session when this instance is the UI-config one.
 */
QJsonObject IO::Drivers::Iec104::statusJson() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusJson();

  return QJsonObject{
    {       QStringLiteral("connected"),                                        m_open},
    {         QStringLiteral("started"),                                     m_started},
    {            QStringLiteral("host"),                                        m_host},
    {            QStringLiteral("port"),                                        m_port},
    {   QStringLiteral("commonAddress"),                               m_commonAddress},
    {      QStringLiteral("pointCount"),                                  pointCount()},
    {QStringLiteral("badQualityPoints"),       static_cast<qint64>(m_badQualityPoints)},
    {    QStringLiteral("skippedAsdus"),           static_cast<qint64>(m_skippedAsdus)},
    {    QStringLiteral("testTimeouts"),           static_cast<qint64>(m_testTimeouts)},
    {  QStringLiteral("sequenceErrors"),  static_cast<qint64>(m_link.sequenceErrors())},
    { QStringLiteral("malformedFrames"), static_cast<qint64>(m_link.malformedFrames())},
    { QStringLiteral("framesPublished"),        static_cast<qint64>(m_framesPublished)},
    { QStringLiteral("unstampedPoints"),        static_cast<qint64>(m_unstampedPoints)},
    {       QStringLiteral("linkDrops"),              static_cast<qint64>(m_linkDrops)},
    {       QStringLiteral("lastError"),                                   m_lastError},
    {      QStringLiteral("statusText"),                                  statusText()},
  };
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the station's host name or IP address.
 */
void IO::Drivers::Iec104::setHost(const QString& host)
{
  const auto trimmed = host.trimmed();
  if (m_host == trimmed)
    return;

  m_host = trimmed;
  if (m_persistent)
    m_settings.setValue("Iec104Driver/host", m_host);

  Q_EMIT hostChanged();
}

/**
 * @brief Sets the TCP port.
 */
void IO::Drivers::Iec104::setPort(const int port)
{
  const int clamped = qBound(1, port, 65535);
  if (m_port == clamped)
    return;

  m_port = clamped;
  if (m_persistent)
    m_settings.setValue("Iec104Driver/port", m_port);

  Q_EMIT portChanged();
}

/**
 * @brief Sets the common address of ASDU; frames from any other station are ignored.
 */
void IO::Drivers::Iec104::setCommonAddress(const int address)
{
  const int clamped = qBound(0, address, kIec104MaxCommonAddress);
  if (m_commonAddress == clamped)
    return;

  m_commonAddress = clamped;
  if (m_persistent)
    m_settings.setValue("Iec104Driver/commonAddress", m_commonAddress);

  Q_EMIT commonAddressChanged();
}

/**
 * @brief Sets the k window; a live session keeps the window it opened with.
 */
void IO::Drivers::Iec104::setWindowK(const int k)
{
  if (m_windowK == k)
    return;

  m_windowK = k;
  applyProtocolParameters();
  if (m_persistent)
    m_settings.setValue("Iec104Driver/windowK", m_windowK);

  Q_EMIT protocolParametersChanged();
}

/**
 * @brief Sets the w window; a live session keeps the window it opened with.
 */
void IO::Drivers::Iec104::setWindowW(const int w)
{
  if (m_windowW == w)
    return;

  m_windowW = w;
  applyProtocolParameters();
  if (m_persistent)
    m_settings.setValue("Iec104Driver/windowW", m_windowW);

  Q_EMIT protocolParametersChanged();
}

/**
 * @brief Sets the t1 send/confirm timeout in milliseconds.
 */
void IO::Drivers::Iec104::setTimeoutT1(const int ms)
{
  if (m_timeoutT1 == ms)
    return;

  m_timeoutT1 = ms;
  applyProtocolParameters();
  if (m_persistent)
    m_settings.setValue("Iec104Driver/timeoutT1", m_timeoutT1);

  Q_EMIT protocolParametersChanged();
}

/**
 * @brief Sets the t2 acknowledgement timeout in milliseconds.
 */
void IO::Drivers::Iec104::setTimeoutT2(const int ms)
{
  if (m_timeoutT2 == ms)
    return;

  m_timeoutT2 = ms;
  applyProtocolParameters();
  if (m_persistent)
    m_settings.setValue("Iec104Driver/timeoutT2", m_timeoutT2);

  Q_EMIT protocolParametersChanged();
}

/**
 * @brief Sets the t3 idle-test timeout in milliseconds.
 */
void IO::Drivers::Iec104::setTimeoutT3(const int ms)
{
  if (m_timeoutT3 == ms)
    return;

  m_timeoutT3 = ms;
  applyProtocolParameters();
  if (m_persistent)
    m_settings.setValue("Iec104Driver/timeoutT3", m_timeoutT3);

  Q_EMIT protocolParametersChanged();
}

/**
 * @brief Restores a point table from its JSON shape, dropping malformed and duplicate entries.
 *        The ORDER is the wire layout of an already-generated project, so it is preserved exactly
 *        as stored rather than sorted into address order.
 */
void IO::Drivers::Iec104::setPoints(const QJsonArray& points)
{
  QVector<Iec104Point> table;
  QHash<quint64, int> index;
  for (const auto& item : points) {
    const auto obj    = item.toObject();
    const auto ioa    = static_cast<quint32>(obj.value(QStringLiteral("ioa")).toInteger(-1));
    const auto typeId = static_cast<std::uint8_t>(obj.value(QStringLiteral("typeId")).toInt(0));
    const auto key    = slotKey(ioa, typeId);
    if (ioa > kMaxIoa || index.contains(key) || table.size() >= OpcUaWire::kMaxTags)
      continue;

    Iec104Point point;
    point.ioa    = ioa;
    point.typeId = typeId;
    point.kind   = kindForType(point.typeId);
    index.insert(key, static_cast<int>(table.size()));
    table.append(point);
  }

  if (table == m_points || m_open)
    return;

  m_points     = table;
  m_slotForKey = index;
  m_values     = QList<QVariant>(m_points.size());
  m_stamps     = QList<qint64>(m_points.size(), -1);
  m_dirty      = QList<bool>(m_points.size(), false);
  m_dirtyCount = 0;
  savePoints();
  Q_EMIT pointsChanged();
}

/**
 * @brief Forgets every discovered point; the next session rediscovers them from scratch.
 */
void IO::Drivers::Iec104::clearPoints()
{
  if (m_points.isEmpty() || m_open)
    return;

  m_points.clear();
  m_slotForKey.clear();
  m_values.clear();
  m_stamps.clear();
  m_dirty.clear();
  m_dirtyCount = 0;
  savePoints();
  Q_EMIT pointsChanged();
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat editable property list; the discovered point table rides along so a project
 *        captures the wire layout its datasets were generated against.
 */
QList<IO::DriverProperty> IO::Drivers::Iec104::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty host;
  host.key   = QStringLiteral("host");
  host.label = tr("Host");
  host.type  = IO::DriverProperty::Text;
  host.value = m_host;
  props.append(host);

  IO::DriverProperty port;
  port.key   = QStringLiteral("port");
  port.label = tr("Port");
  port.type  = IO::DriverProperty::IntField;
  port.value = m_port;
  port.min   = 1;
  port.max   = 65535;
  props.append(port);

  IO::DriverProperty address;
  address.key   = QStringLiteral("commonAddress");
  address.label = tr("Common Address");
  address.type  = IO::DriverProperty::IntField;
  address.value = m_commonAddress;
  address.min   = 0;
  address.max   = kIec104MaxCommonAddress;
  props.append(address);

  static constexpr const char* kWindowKeys[] = {"windowK", "windowW"};
  const int windows[]                        = {m_windowK, m_windowW};
  for (int i = 0; i < 2; ++i) {
    IO::DriverProperty window;
    window.key   = QString::fromLatin1(kWindowKeys[i]);
    window.label = i == 0 ? tr("Send Window (k)") : tr("Ack Window (w)");
    window.type  = IO::DriverProperty::IntField;
    window.value = windows[i];
    window.min   = kMinWindow;
    window.max   = kMaxWindow;
    props.append(window);
  }

  static constexpr const char* kTimeoutKeys[] = {"timeoutT1", "timeoutT2", "timeoutT3"};
  const int timeouts[]                        = {m_timeoutT1, m_timeoutT2, m_timeoutT3};
  for (int i = 0; i < 3; ++i) {
    IO::DriverProperty timeout;
    timeout.key   = QString::fromLatin1(kTimeoutKeys[i]);
    timeout.label = tr("Timeout t%1 (ms)").arg(i + 1);
    timeout.type  = IO::DriverProperty::IntField;
    timeout.value = timeouts[i];
    timeout.min   = kMinTimeMs;
    timeout.max   = kMaxTimeMs;
    props.append(timeout);
  }

  IO::DriverProperty points;
  points.key   = QStringLiteral("points");
  points.type  = IO::DriverProperty::Text;
  points.value = QVariant::fromValue(pointsJson());
  props.append(points);

  return props;
}

/**
 * @brief Applies a single configuration change by key; the twin of driverProperties(), and the
 *        half a project round-trip dies without.
 */
void IO::Drivers::Iec104::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("host")) {
    setHost(value.toString());
    return;
  }

  if (key == QLatin1String("port")) {
    setPort(value.toInt());
    return;
  }

  if (key == QLatin1String("commonAddress")) {
    setCommonAddress(value.toInt());
    return;
  }

  if (key == QLatin1String("windowK")) {
    setWindowK(value.toInt());
    return;
  }

  if (key == QLatin1String("windowW")) {
    setWindowW(value.toInt());
    return;
  }

  if (key == QLatin1String("timeoutT1")) {
    setTimeoutT1(value.toInt());
    return;
  }

  if (key == QLatin1String("timeoutT2")) {
    setTimeoutT2(value.toInt());
    return;
  }

  if (key == QLatin1String("timeoutT3")) {
    setTimeoutT3(value.toInt());
    return;
  }

  if (key == QLatin1String("points"))
    setPoints(asJsonArray(value));
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The wire type a point's value class encodes as; a normalized measurand keeps full double
 *        precision because it is a fraction of full scale, not an engineering integer.
 */
IO::Drivers::OpcUaWire::Type IO::Drivers::Iec104::wireTypeFor(PointKind kind) noexcept
{
  switch (kind) {
    case PointKind::Single:
      return OpcUaWire::Type::Bool;
    case PointKind::Double:
      return OpcUaWire::Type::U8;
    case PointKind::Normalized:
      return OpcUaWire::Type::F64;
    case PointKind::Scaled:
    case PointKind::Counter:
      return OpcUaWire::Type::I32;
    case PointKind::Float:
      return OpcUaWire::Type::F32;
    case PointKind::Invalid:
      break;
  }

  return OpcUaWire::Type::Str;
}

/**
 * @brief One dataset per point: an LED for the status points, a plot for everything measured.
 */
DataModel::Dataset IO::Drivers::Iec104::datasetFor(const Iec104Point& point, int index)
{
  SS_ASSERT_LOG(index >= 1);

  DataModel::Dataset dataset;
  dataset.index = index;
  dataset.log   = true;
  dataset.title = pointName(point);

  if (point.kind == PointKind::Single) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (point.kind != PointKind::Invalid && point.kind != PointKind::Double)
    dataset.plt = true;

  return dataset;
}

/**
 * @brief One group per ASDU type class, one dataset per wire index, the iec104 native template.
 */
QJsonObject IO::Drivers::Iec104::buildProject() const
{
  QJsonObject project;
  project[Keys::Title]   = tr("IEC 60870-5-104 Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("IEC 60870-5-104");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::Iec104);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("iec104");
  source[Keys::FrameParserParams]     = QJsonObject{
        {QStringLiteral("schema"), wireSchema()}
  };

  QJsonObject conn;
  for (const auto& prop : driverProperties())
    if (prop.type != IO::DriverProperty::Password)
      conn.insert(prop.key, QJsonValue::fromVariant(prop.value));

  source[Keys::SourceConn] = conn;
  project[Keys::Sources]   = QJsonArray{source};

  const auto& table = pointTable();

  QStringList order;
  QHash<QString, DataModel::Group> groups;
  for (int i = 0; i < table.size(); ++i) {
    const auto& point = table.at(i);
    const QString key = point.kind == PointKind::Single || point.kind == PointKind::Double
                        ? tr("Status Points")
                        : (point.kind == PointKind::Counter ? tr("Counters") : tr("Measurements"));
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = key;
      groups.insert(key, group);
      order.append(key);
    }

    groups[key].datasets.push_back(datasetFor(point, i + 1));
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
DataModel::ProjectModel* IO::Drivers::Iec104::loadGeneratedProject()
{
  adoptDiscoveredPoints();
  if (pointTable().isEmpty())
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
 * @brief Generates a project from the discovered point table and opens it in the editor.
 */
void IO::Drivers::Iec104::generateProject()
{
  adoptDiscoveredPoints();
  if (pointTable().isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("No points discovered"),
      tr("Connect to the station and let the interrogation finish before generating a project."),
      QMessageBox::Warning,
      tr("IEC 60870-5-104 Project Generator"));
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
        tr("IEC 60870-5-104 Project Generator"));
    },
    Qt::SingleShotConnection);

  (void)pm->saveJsonFile(true);
}
