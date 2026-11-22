/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "Console/BusMonitor.h"

#include <algorithm>
#include <QTime>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kModeKey    = QStringLiteral("ConsoleBusMonitor/Mode");
static const QString kTimeoutKey = QStringLiteral("ConsoleBusMonitor/TimeoutMs");
static const QString kGapKey     = QStringLiteral("ConsoleBusMonitor/GapMs");
static const QString kFormatsKey = QStringLiteral("ConsoleBusMonitor/Formats");
static const QString kIssuesKey  = QStringLiteral("ConsoleBusMonitor/IssuesOnly");

static constexpr int kAllFormats = Console::FormatU16 | Console::FormatS16 | Console::FormatU32
                                 | Console::FormatS32 | Console::FormatF32 | Console::FormatBin;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores the panel's settings; the store starts empty and unwatched.
 */
Console::BusMonitor::BusMonitor(QObject* parent)
  : QAbstractTableModel(parent)
  , m_epoch(std::chrono::steady_clock::now())
  , m_dayOffsetMs(QTime::currentTime().msecsSinceStartOfDay())
  , m_nextSequence(1)
  , m_requests()
  , m_requestCursor(0)
  , m_packetStartMs(0)
  , m_packetLastMs(0)
  , m_mode(Messages)
  , m_gapMs(kBusMonitorGapDefaultMs)
  , m_formats(FormatU16)
  , m_frameTotal(0)
  , m_errorTotal(0)
  , m_warningTotal(0)
  , m_paused(false)
  , m_issuesOnly(false)
  , m_countDirty(false)
{
  m_mode       = m_settings.value(kModeKey, Messages).toInt() == ModbusRtu ? ModbusRtu : Messages;
  m_issuesOnly = m_settings.value(kIssuesKey, false).toBool();
  m_formats    = m_settings.value(kFormatsKey, FormatU16).toInt() & kAllFormats;
  m_gapMs      = qBound(kBusMonitorGapMinMs,
                        m_settings.value(kGapKey, kBusMonitorGapDefaultMs).toInt(),
                        kBusMonitorGapMaxMs);
  m_analyzer.setTimeoutMs(m_settings.value(kTimeoutKey, kModbusTimeoutDefaultMs).toInt());

  m_pending.reserve(64);
  m_frameScratch.reserve(64);
  m_updateScratch.reserve(8);
  m_packet.reserve(kBusMonitorMaxMessageBytes);
}

//--------------------------------------------------------------------------------------------------
// Table model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Messages currently kept.
 */
int Console::BusMonitor::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

/**
 * @brief The table's fixed columns.
 */
int Console::BusMonitor::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : ColumnCount;
}

/**
 * @brief Cell text for the display role, plus the severity and role of the row for its tint.
 */
QVariant Console::BusMonitor::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_rows.size()))
    return {};

  const auto& frame = m_rows[static_cast<size_t>(index.row())];
  if (role == Qt::DisplayRole)
    return cellText(frame, index.column());

  if (role == SeverityRole)
    return static_cast<int>(busSeverity(frame.findings));

  if (role == BusRoleRole)
    return static_cast<int>(frame.role);

  return {};
}

/**
 * @brief Column titles; the second column is the role in Modbus RTU mode and the direction
 *        otherwise.
 */
QVariant Console::BusMonitor::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return {};

  switch (section) {
    case TimeColumn:
      return tr("Time");
    case RoleColumn:
      return m_mode == ModbusRtu ? tr("Role") : tr("Direction");
    case SlaveColumn:
      return tr("Slave");
    case FunctionColumn:
      return tr("Function");
    case LengthColumn:
      return tr("Length");
    case SummaryColumn:
      return tr("Data Summary");
    case CrcColumn:
      return tr("CRC");
    case DiagnosisColumn:
      return tr("Diagnosis");
    case RawColumn:
      return tr("Raw Frame");
    default:
      return {};
  }
}

/**
 * @brief Role names for the QML delegates.
 */
QHash<int, QByteArray> Console::BusMonitor::roleNames() const
{
  static const QHash<int, QByteArray> kNames = {
    {Qt::DisplayRole,  "display"},
    {   SeverityRole, "severity"},
    {    BusRoleRole,  "busRole"},
  };

  return kNames;
}

/**
 * @brief The text of one cell. Slave, function, summary and CRC are Modbus columns: they stay
 *        empty for a plain message, and for bytes that never verified as a frame.
 */
QString Console::BusMonitor::cellText(const BusFrame& frame, int column) const
{
  const bool modbus =
    frame.role == BusRole::Master || frame.role == BusRole::Slave || frame.role == BusRole::Unknown;
  const bool framed = modbus && frame.crcOk && frame.bytes.size() >= 2;
  switch (column) {
    case TimeColumn:
      return QTime::fromMSecsSinceStartOfDay(
               static_cast<int>((m_dayOffsetMs + qMax<qint64>(0, frame.timeMs)) % 86400000))
        .toString(QStringLiteral("HH:mm:ss.zzz"));
    case RoleColumn:
      switch (frame.role) {
        case BusRole::Master:
          return tr("Master");
        case BusRole::Slave:
          return tr("Slave");
        case BusRole::Sent:
          return tr("TX");
        case BusRole::Received:
          return tr("RX");
        case BusRole::Unknown:
          return tr("Unknown");
      }

      return {};
    case SlaveColumn:
      return framed ? QString::number(static_cast<quint8>(frame.bytes.at(0))) : QStringLiteral("-");
    case FunctionColumn:
      return framed ? modbusFunctionName(static_cast<quint8>(frame.bytes.at(1)))
                    : QStringLiteral("-");
    case LengthColumn:
      return QString::number(frame.bytes.size());
    case SummaryColumn:
      return modbus ? modbusDataSummary(frame, static_cast<quint8>(m_formats)) : QString();
    case CrcColumn:
      if (!modbus)
        return QString();

      return frame.crcOk ? tr("OK") : tr("Failed");
    case DiagnosisColumn:
      return busFindingsText(frame.findings, frame);
    case RawColumn:
      return QString::fromLatin1(frame.bytes.toHex(' ')).toUpper();
    default:
      return {};
  }
}

//--------------------------------------------------------------------------------------------------
// Properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief Messages or Modbus RTU.
 */
int Console::BusMonitor::mode() const noexcept
{
  return m_mode;
}

/**
 * @brief Idle time that ends a received message in Messages mode, in milliseconds.
 */
int Console::BusMonitor::gapMs() const noexcept
{
  return m_gapMs;
}

/**
 * @brief Messages currently kept.
 */
int Console::BusMonitor::count() const noexcept
{
  return static_cast<int>(m_rows.size());
}

/**
 * @brief The ticked register formats, as BusValueFormat flags.
 */
int Console::BusMonitor::formats() const noexcept
{
  return m_formats;
}

/**
 * @brief Whether incoming data is being discarded.
 */
bool Console::BusMonitor::paused() const noexcept
{
  return m_paused;
}

/**
 * @brief How long a Modbus request waits for its response.
 */
int Console::BusMonitor::timeoutMs() const noexcept
{
  return m_analyzer.timeoutMs();
}

/**
 * @brief Whether the table keeps only rows with a warning or an error.
 */
bool Console::BusMonitor::issuesOnly() const noexcept
{
  return m_issuesOnly;
}

/**
 * @brief Messages seen since the last clear; trimming old rows does not lower it.
 */
int Console::BusMonitor::frameTotal() const noexcept
{
  return m_frameTotal;
}

/**
 * @brief Messages with an error since the last clear.
 */
int Console::BusMonitor::errorTotal() const noexcept
{
  return m_errorTotal;
}

/**
 * @brief Messages whose worst finding is a warning since the last clear.
 */
int Console::BusMonitor::warningTotal() const noexcept
{
  return m_warningTotal;
}

/**
 * @brief Whether at least one panel is on screen; unwatched, the monitor ingests nothing.
 */
bool Console::BusMonitor::watched() const noexcept
{
  return !m_viewers.isEmpty();
}

/**
 * @brief Time of the newest kept message, which the timeline anchors its right edge on.
 */
qint64 Console::BusMonitor::lastTimeMs() const noexcept
{
  return m_rows.empty() ? 0 : m_rows.back().timeMs;
}

/**
 * @brief Mean gap between the kept messages, 0 with fewer than two.
 */
qreal Console::BusMonitor::averageIntervalMs() const noexcept
{
  if (m_rows.size() < 2)
    return 0.0;

  const auto span = static_cast<qreal>(m_rows.back().timeMs - m_rows.front().timeMs);
  return span / static_cast<qreal>(m_rows.size() - 1);
}

/**
 * @brief The message in row @a row.
 */
const Console::BusFrame& Console::BusMonitor::at(int row) const
{
  static const BusFrame kEmpty;
  SS_ASSERT(row >= 0 && row < static_cast<int>(m_rows.size()), return kEmpty);
  return m_rows[static_cast<size_t>(row)];
}

/**
 * @brief Milliseconds from the monitor's start to @a time. A chunk's driver timestamp goes
 *        through here, so received messages carry the time their source stamped.
 */
qint64 Console::BusMonitor::toMs(TimePoint time) const noexcept
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(time - m_epoch).count();
}

//--------------------------------------------------------------------------------------------------
// Settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Switches between Messages and Modbus RTU. Buffered bytes and request pairing are
 *        dropped; rows already shown stay.
 */
void Console::BusMonitor::setMode(int mode)
{
  const int next = mode == ModbusRtu ? ModbusRtu : Messages;
  if (m_mode == next)
    return;

  m_mode = next;
  m_settings.setValue(kModeKey, m_mode);
  resetIngest();
  Q_EMIT headerDataChanged(Qt::Horizontal, RoleColumn, RoleColumn);
  Q_EMIT settingsChanged();
}

/**
 * @brief Sets the idle gap of Messages mode, clamped.
 */
void Console::BusMonitor::setGapMs(int gapMs)
{
  const int clamped = qBound(kBusMonitorGapMinMs, gapMs, kBusMonitorGapMaxMs);
  if (m_gapMs == clamped)
    return;

  m_gapMs = clamped;
  m_settings.setValue(kGapKey, m_gapMs);
  Q_EMIT settingsChanged();
}

/**
 * @brief Sets the ticked register formats; every summary cell is redrawn in the new formats.
 */
void Console::BusMonitor::setFormats(int formats)
{
  const int masked = formats & kAllFormats;
  if (m_formats == masked)
    return;

  m_formats = masked;
  m_settings.setValue(kFormatsKey, m_formats);
  if (!m_rows.empty())
    Q_EMIT dataChanged(index(0, SummaryColumn),
                       index(static_cast<int>(m_rows.size()) - 1, SummaryColumn),
                       {Qt::DisplayRole});

  Q_EMIT settingsChanged();
}

/**
 * @brief Pauses or resumes ingestion. Resuming starts from a clean buffer: bytes from before the
 *        pause must not be joined to what arrives after it.
 */
void Console::BusMonitor::setPaused(bool paused)
{
  if (m_paused == paused)
    return;

  m_paused = paused;
  if (!m_paused)
    resetIngest();

  Q_EMIT settingsChanged();
}

/**
 * @brief Sets the Modbus response timeout, clamped by the analyzer.
 */
void Console::BusMonitor::setTimeoutMs(int timeoutMs)
{
  const int before = m_analyzer.timeoutMs();
  m_analyzer.setTimeoutMs(timeoutMs);
  if (before == m_analyzer.timeoutMs())
    return;

  m_settings.setValue(kTimeoutKey, m_analyzer.timeoutMs());
  Q_EMIT settingsChanged();
}

/**
 * @brief Turns the table's "issues only" filter on or off.
 */
void Console::BusMonitor::setIssuesOnly(bool issuesOnly)
{
  if (m_issuesOnly == issuesOnly)
    return;

  m_issuesOnly = issuesOnly;
  m_settings.setValue(kIssuesKey, m_issuesOnly);
  Q_EMIT settingsChanged();
}

/**
 * @brief Registers or removes a panel that shows the monitor. Views are tracked by identity with
 *        a destroyed() guard each, so a window torn down with its panel open cannot leave the
 *        monitor ingesting for nobody. The first view starts from a clean buffer.
 */
void Console::BusMonitor::setViewerActive(QObject* viewer, bool active)
{
  if (!viewer)
    return;

  const bool known = m_viewers.contains(viewer);
  if (active && !known) {
    if (m_viewers.isEmpty())
      resetIngest();

    m_viewers.insert(viewer, connect(viewer, &QObject::destroyed, this, [this, viewer] {
                       m_viewers.remove(viewer);
                     }));
  }

  if (!active && known) {
    QObject::disconnect(m_viewers.take(viewer));
    if (m_viewers.isEmpty())
      resetIngest();
  }
}

/**
 * @brief Empties the table, the counters and every buffer.
 */
void Console::BusMonitor::clear()
{
  beginResetModel();
  m_rows.clear();
  m_pending.clear();
  endResetModel();

  m_frameTotal   = 0;
  m_errorTotal   = 0;
  m_warningTotal = 0;
  resetIngest();
  Q_EMIT countChanged();
}

/**
 * @brief Drops buffered bytes and the request pairing without touching the rows shown.
 */
void Console::BusMonitor::resetIngest()
{
  m_packet.clear();
  m_analyzer.reset();
  m_packetStartMs = 0;
  m_packetLastMs  = 0;
  m_requestCursor = 0;
  m_requests.fill(RequestRow());
}

//--------------------------------------------------------------------------------------------------
// Ingestion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes one received chunk stamped @a nowMs. Unwatched or paused, it returns at once. In
 *        Messages mode the chunk extends the current message, or starts a new one when the line
 *        was idle for the gap; in Modbus RTU mode it goes to the analyzer.
 */
void Console::BusMonitor::feedReceived(const QByteArray& bytes, qint64 nowMs)
{
  if (bytes.isEmpty() || m_viewers.isEmpty() || m_paused)
    return;

  if (m_mode == ModbusRtu) {
    m_analyzer.feed(bytes, nowMs, m_frameScratch, m_updateScratch);
    stageAnalyzed(nowMs);
    return;
  }

  if (!m_packet.isEmpty() && nowMs - m_packetLastMs >= m_gapMs)
    flushPacket(nowMs);

  if (m_packet.isEmpty())
    m_packetStartMs = nowMs;

  m_packetLastMs = nowMs;
  for (qsizetype at = 0; at < bytes.size();) {
    const qsizetype room = kBusMonitorMaxMessageBytes - m_packet.size();
    const qsizetype take = qMin(room, bytes.size() - at);
    m_packet.append(bytes.constData() + at, take);
    at += take;
    if (m_packet.size() < kBusMonitorMaxMessageBytes)
      break;

    flushPacket(nowMs);
    m_packetStartMs = nowMs;
  }
}

/**
 * @brief Takes the bytes a write put on the wire. In Messages mode a send is one message, and it
 *        closes the received message before it so the two keep their order. In Modbus RTU mode
 *        the bytes join the same stream as the received ones: the console may be the master.
 */
void Console::BusMonitor::feedSent(const QByteArray& bytes, qint64 nowMs)
{
  if (bytes.isEmpty() || m_viewers.isEmpty() || m_paused)
    return;

  if (m_mode == ModbusRtu) {
    m_analyzer.feed(bytes, nowMs, m_frameScratch, m_updateScratch);
    stageAnalyzed(nowMs);
    return;
  }

  flushPacket(nowMs);

  BusFrame frame;
  frame.timeMs = nowMs;
  frame.role   = BusRole::Sent;
  frame.crcOk  = true;
  frame.bytes  = bytes.first(qMin<qsizetype>(bytes.size(), kBusMonitorMaxMessageBytes));
  stage(frame);
}

/**
 * @brief Closes the received message being assembled in Messages mode, stamped with the time
 *        its first byte arrived.
 */
void Console::BusMonitor::flushPacket(qint64 nowMs)
{
  Q_UNUSED(nowMs)
  if (m_packet.isEmpty())
    return;

  BusFrame frame;
  frame.timeMs = m_packetStartMs;
  frame.role   = BusRole::Received;
  frame.crcOk  = true;
  frame.bytes  = QByteArray(m_packet.constData(), m_packet.size());
  m_packet.resize(0);
  stage(frame);
}

/**
 * @brief Moves what the analyzer just reported into the staging list: frames first, so a finding
 *        on a request reported in the same call finds its row.
 */
void Console::BusMonitor::stageAnalyzed(qint64 nowMs)
{
  Q_UNUSED(nowMs)
  for (const auto& frame : m_frameScratch) {
    const quint64 analyzerSequence = frame.sequence;
    stage(frame);
    if (frame.role != BusRole::Master)
      continue;

    m_requests[static_cast<size_t>(m_requestCursor)] = {analyzerSequence, m_nextSequence - 1};
    m_requestCursor                                  = (m_requestCursor + 1) % kRequestMemory;
  }

  for (const auto& update : m_updateScratch)
    applyUpdate(update);

  m_frameScratch.clear();
  m_updateScratch.clear();
}

/**
 * @brief Stages one message under the store's own sequence number and counts it. Nothing is
 *        published here: commitPending() opens one model transaction per UI tick. A staging list
 *        that fills between two ticks (a stalled GUI thread) loses its oldest quarter, which
 *        stays counted in the totals.
 */
void Console::BusMonitor::stage(const BusFrame& frame)
{
  if (m_pending.size() >= static_cast<size_t>(kBusMonitorMaxMessages))
    m_pending.erase(m_pending.begin(), m_pending.begin() + kBusMonitorMaxMessages / 4);

  m_pending.push_back(frame);
  m_pending.back().sequence = m_nextSequence++;
  ++m_frameTotal;
  tally(BusSeverity::None, busSeverity(frame.findings));
  m_countDirty = true;
}

/**
 * @brief Moves a message between the error and warning counters when its worst finding changes.
 */
void Console::BusMonitor::tally(BusSeverity before, BusSeverity after)
{
  SS_ASSERT(m_errorTotal >= 0 && m_warningTotal >= 0, return);
  if (before == after)
    return;

  if (before == BusSeverity::Error)
    --m_errorTotal;

  if (before == BusSeverity::Warning)
    --m_warningTotal;

  if (after == BusSeverity::Error)
    ++m_errorTotal;

  if (after == BusSeverity::Warning)
    ++m_warningTotal;

  m_countDirty = true;
}

/**
 * @brief Row of the kept message with sequence number @a sequence, or -1. Sequence numbers rise
 *        with the row but may skip (staged messages dropped by stage()), so the row is searched
 *        for, never computed as a distance.
 */
int Console::BusMonitor::rowOfSequence(quint64 sequence) const noexcept
{
  const auto it = std::lower_bound(
    m_rows.begin(), m_rows.end(), sequence, [](const BusFrame& frame, quint64 wanted) {
      return frame.sequence < wanted;
    });
  if (it == m_rows.end() || it->sequence != sequence)
    return -1;

  return static_cast<int>(it - m_rows.begin());
}

/**
 * @brief Adds a finding to a message already reported: the request a slave never answered. The
 *        message may still be staged, or already in the table, or already trimmed away.
 */
void Console::BusMonitor::applyUpdate(const BusUpdate& update)
{
  quint64 target = 0;
  for (const auto& request : m_requests)
    if (request.rowSequence != 0 && request.analyzerSequence == update.sequence)
      target = request.rowSequence;

  if (target == 0)
    return;

  for (auto& staged : m_pending) {
    if (staged.sequence != target)
      continue;

    const auto before  = busSeverity(staged.findings);
    staged.findings   |= update.findings;
    tally(before, busSeverity(staged.findings));
    return;
  }

  const int row = rowOfSequence(target);
  if (row < 0)
    return;

  auto& frame        = m_rows[static_cast<size_t>(row)];
  const auto before  = busSeverity(frame.findings);
  frame.findings    |= update.findings;
  tally(before, busSeverity(frame.findings));
  Q_EMIT dataChanged(index(row, 0), index(row, ColumnCount - 1), {Qt::DisplayRole, SeverityRole});
}

/**
 * @brief Runs once per UI tick at @a nowMs: lets the analyzer time out a silent request, closes
 *        an idle Messages-mode message, then publishes everything staged in one model
 *        transaction, dropping the oldest rows past the cap first.
 */
void Console::BusMonitor::commitPending(qint64 nowMs)
{
  if (!m_viewers.isEmpty() && m_mode == ModbusRtu) {
    m_analyzer.tick(nowMs, m_frameScratch, m_updateScratch);
    stageAnalyzed(nowMs);
  }

  if (m_mode == Messages && !m_packet.isEmpty() && nowMs - m_packetLastMs >= m_gapMs)
    flushPacket(nowMs);

  if (!m_pending.empty()) {
    const int staged   = static_cast<int>(m_pending.size());
    const int overflow = static_cast<int>(m_rows.size()) + staged - kBusMonitorMaxMessages;
    const int drop     = qMin(overflow, static_cast<int>(m_rows.size()));
    if (drop > 0) {
      beginRemoveRows(QModelIndex(), 0, drop - 1);
      m_rows.erase(m_rows.begin(), m_rows.begin() + drop);
      endRemoveRows();
    }

    const int first = static_cast<int>(m_rows.size());
    beginInsertRows(QModelIndex(), first, first + staged - 1);
    for (const auto& frame : m_pending)
      m_rows.push_back(frame);

    endInsertRows();
    m_pending.clear();
  }

  if (!m_countDirty)
    return;

  m_countDirty = false;
  Q_EMIT countChanged();
}

//--------------------------------------------------------------------------------------------------
// Timeline and hover
//--------------------------------------------------------------------------------------------------

/**
 * @brief The messages inside [fromMs, toMs] as flat bar geometry for a canvas @a pixels wide:
 *        x, lane (0 for requests and sends, 1 for the rest), severity and row, four numbers per
 *        bar, newest last and at most kBusMonitorMaxBars of them. Painted, not instantiated: one
 *        scene-graph item per message is what made the annotation strip crawl.
 */
QVariantList Console::BusMonitor::timelineBars(qint64 fromMs, qint64 toMs, qreal pixels) const
{
  QVariantList bars;
  if (m_rows.empty() || toMs <= fromMs || pixels <= 0.0)
    return bars;

  const auto first =
    std::lower_bound(m_rows.begin(), m_rows.end(), fromMs, [](const BusFrame& frame, qint64 time) {
      return frame.timeMs < time;
    });

  const qreal scale = pixels / static_cast<qreal>(toMs - fromMs);
  auto start        = first;
  const auto inside = std::count_if(
    first, m_rows.end(), [toMs](const BusFrame& frame) { return frame.timeMs <= toMs; });
  if (inside > kBusMonitorMaxBars)
    start = first + (inside - kBusMonitorMaxBars);

  bars.reserve(static_cast<qsizetype>(qMin<qint64>(inside, kBusMonitorMaxBars)) * 4);
  for (auto it = start; it != m_rows.end() && it->timeMs <= toMs; ++it) {
    const bool upper = it->role == BusRole::Master || it->role == BusRole::Sent;
    bars.append(static_cast<qreal>(it->timeMs - fromMs) * scale);
    bars.append(upper ? 0 : 1);
    bars.append(static_cast<int>(busSeverity(it->findings)));
    bars.append(static_cast<int>(it - m_rows.begin()));
  }

  return bars;
}

/**
 * @brief Everything the timeline's hover card shows about the message in row @a row.
 */
QVariantMap Console::BusMonitor::messageInfo(int row) const
{
  QVariantMap info;
  if (row < 0 || row >= static_cast<int>(m_rows.size()))
    return info;

  const auto& frame = m_rows[static_cast<size_t>(row)];
  info.insert(QStringLiteral("time"), cellText(frame, TimeColumn));
  info.insert(QStringLiteral("role"), cellText(frame, RoleColumn));
  info.insert(QStringLiteral("length"), static_cast<int>(frame.bytes.size()));
  info.insert(QStringLiteral("raw"), cellText(frame, RawColumn));
  info.insert(QStringLiteral("summary"), cellText(frame, SummaryColumn));
  info.insert(QStringLiteral("diagnosis"), cellText(frame, DiagnosisColumn));
  return info;
}

//--------------------------------------------------------------------------------------------------
// Table filter
//--------------------------------------------------------------------------------------------------

/**
 * @brief Filters @a monitor and re-evaluates when its "issues only" setting changes or a row
 *        gains a finding.
 */
Console::BusMonitorFilter::BusMonitorFilter(BusMonitor* monitor, QObject* parent)
  : QSortFilterProxyModel(parent), m_monitor(monitor), m_issuesOnly(false)
{
  SS_ASSERT(m_monitor != nullptr, return);

  m_issuesOnly = m_monitor->issuesOnly() && m_monitor->mode() == BusMonitor::ModbusRtu;
  setSourceModel(m_monitor);
  setDynamicSortFilter(true);
  connect(m_monitor, &BusMonitor::settingsChanged, this, &BusMonitorFilter::syncIssuesOnly);
}

/**
 * @brief Takes the monitor's "issues only" setting, which filters in Modbus RTU mode only:
 *        Messages rows carry no findings and the checkbox is hidden there. The filter keeps its
 *        own copy because the change has to happen between the proxy's begin and end calls.
 */
void Console::BusMonitorFilter::syncIssuesOnly()
{
  SS_ASSERT(m_monitor != nullptr, return);
  const bool wanted = m_monitor->issuesOnly() && m_monitor->mode() == BusMonitor::ModbusRtu;
  if (m_issuesOnly == wanted)
    return;

  beginFilterChange();
  m_issuesOnly = wanted;
  endFilterChange(QSortFilterProxyModel::Direction::Rows);
}

/**
 * @brief Keeps every row, or with "issues only" the rows carrying a warning or an error.
 */
bool Console::BusMonitorFilter::filterAcceptsRow(int sourceRow, const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  SS_ASSERT(m_monitor != nullptr, return true);
  if (!m_issuesOnly)
    return true;

  if (sourceRow < 0 || sourceRow >= m_monitor->count())
    return false;

  return busSeverity(m_monitor->at(sourceRow).findings) >= BusSeverity::Warning;
}
