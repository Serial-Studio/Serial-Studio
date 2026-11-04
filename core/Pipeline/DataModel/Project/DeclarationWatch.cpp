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

#include "DataModel/Project/DeclarationWatch.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/IO/IDeviceWriter.h"
#include "Core/Runtime.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/Project/QuickPlotProject.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/DeviceWriteApi.h"
#include "DataModel/Scripting/LineFormat.h"

namespace Declaration = DataModel::Declaration;

/**
 * @brief Most (project, declaration) pairs one session remembers having noticed or kept, so a
 *        device cycling through declarations cannot grow either set or flood the notifications.
 */
static constexpr qsizetype kMaxNotices = 64;

/**
 * @brief Quiet time after a project edit before the comparison runs again, so a burst of typing
 *        costs one projection of the loaded project.
 */
static constexpr int kReevaluateDelayMs = 200;

/**
 * @brief Gaps a binary session keeps between two lines it posts to the pipeline, and between two
 *        posts of the same line (spec 0111 R14): a re-sent held declaration still arrives, and a
 *        flood of declaration lines never reaches the pipeline queue at line rate.
 */
static constexpr std::chrono::milliseconds kForwardSpacing(100);
static constexpr std::chrono::seconds kForwardRepeat(1);

/**
 * @brief The wire names of DeclarationStatus in enumerator order, for project.declaration.get.
 */
static constexpr QLatin1StringView kStatusNames[] = {
  QLatin1StringView("none"),
  QLatin1StringView("matches"),
  QLatin1StringView("differs"),
  QLatin1StringView("kept"),
  QLatin1StringView("rejected"),
};

/**
 * @brief The QSettings key of the Quick Plot request toggle (spec 0111 R6).
 */
static constexpr QLatin1StringView kRequestSetting("QuickPlot/RequestDeclaration");

//--------------------------------------------------------------------------------------------------
// Construction and wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Holds the references and reads the request toggle; every subscription waits for
 *        setupExternalConnections(), so the model's ctor closure reaches no other module.
 */
DataModel::DeclarationWatch::DeclarationWatch(ProjectModel& model, Core::Bus::MessageBus& bus)
  : m_mode(-1)
  , m_connected(false)
  , m_binarySession(false)
  , m_requestOnConnect(false)
  , m_status(DeclarationStatus::None)
  , m_model(model)
  , m_bus(bus)
{
  m_requestOnConnect = m_settings.value(kRequestSetting, false).toBool();
  m_reevaluateTimer.setSingleShot(true);
  m_reevaluateTimer.setInterval(kReevaluateDelayMs);
}

/**
 * @brief Follows the operation mode, the link state and the declared framing, and re-compares on
 *        project edits. The link handler is queued, so the request is written after the whole
 *        connect publish: the frame builder's connect-edge reset is already posted to the
 *        pipeline before the device can answer it.
 */
void DataModel::DeclarationWatch::setupExternalConnections()
{
  m_modeState = m_bus.subscribe<Core::Bus::OperationModeChanged>(
    this,
    [this](const std::shared_ptr<const Core::Bus::OperationModeChanged>& mode) {
      onOperationMode(mode->mode);
    },
    Qt::AutoConnection,
    true);
  m_linkState = m_bus.subscribe<Core::Bus::ConnectionStateChanged>(
    this,
    [this](const std::shared_ptr<const Core::Bus::ConnectionStateChanged>& link) {
      onLinkState(*link);
    },
    Qt::QueuedConnection);
  m_framingState = m_bus.subscribe<Core::Bus::DeclaredFraming>(
    this, [this](const std::shared_ptr<const Core::Bus::DeclaredFraming>& framing) {
      m_binarySession = framing->active;
      m_carry.clear();
      m_lastLine.clear();
      m_lastForward = {};
    });

  connect(&m_reevaluateTimer, &QTimer::timeout, this, &DeclarationWatch::reevaluate);
  connect(&m_model, &ProjectModel::groupsChanged, this, &DeclarationWatch::scheduleReevaluate);
  connect(&m_model, &ProjectModel::groupDataChanged, this, &DeclarationWatch::scheduleReevaluate);
  connect(&m_model, &ProjectModel::modifiedChanged, this, &DeclarationWatch::scheduleReevaluate);
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief The last declaration this watch read (Project File mode): its status, the refusal
 *        reason, a summary, and whether a difference is pending.
 */
QJsonObject DataModel::DeclarationWatch::status() const
{
  const auto index = static_cast<size_t>(m_status);
  SS_ASSERT(index < std::size(kStatusNames), return QJsonObject());

  int datasets = 0;
  for (const auto& group : m_declaration.groups)
    datasets += static_cast<int>(group.datasets.size());

  QJsonObject out;
  out[QStringLiteral("status")]            = kStatusNames[index];
  out[QStringLiteral("reason")]            = m_reason;
  out[QStringLiteral("title")]             = m_declaration.title;
  out[QStringLiteral("groups")]            = static_cast<int>(m_declaration.groups.size());
  out[QStringLiteral("datasets")]          = datasets;
  out[QStringLiteral("notes")]             = QJsonArray::fromStringList(m_declaration.notes);
  out[QStringLiteral("pendingDifference")] = pendingDifference();
  out[QStringLiteral("requestOnConnect")]  = m_requestOnConnect;
  return out;
}

/**
 * @brief Turns the Quick Plot request on or off and remembers the choice.
 */
void DataModel::DeclarationWatch::setRequestOnConnect(const bool enabled)
{
  if (m_requestOnConnect == enabled)
    return;

  m_requestOnConnect = enabled;
  m_settings.setValue(kRequestSetting, enabled);
  Q_EMIT requestOnConnectChanged();
}

/**
 * @brief Moves to @p status. The pending difference the Setup buttons and the API offer is the
 *        Differs status, so its NOTIFY fires only on an edge.
 */
void DataModel::DeclarationWatch::setStatus(const DeclarationStatus status)
{
  SS_ASSERT(static_cast<size_t>(status) < std::size(kStatusNames), return);
  const bool wasPending = pendingDifference();
  m_status              = status;
  if (wasPending != pendingDifference())
    Q_EMIT pendingDifferenceChanged();
}

/**
 * @brief Drops the remembered declaration, the partial line and the pending difference.
 */
void DataModel::DeclarationWatch::forget()
{
  m_reevaluateTimer.stop();
  m_carry.clear();
  m_reason.clear();
  m_lastLine.clear();
  m_declaredContent.clear();
  m_declaredProjection = QJsonObject();
  m_lastForward        = {};
  m_state.reset();
  m_declaration = DeviceDeclaration();
  setStatus(DeclarationStatus::None);
}

//--------------------------------------------------------------------------------------------------
// Request on connect
//--------------------------------------------------------------------------------------------------

/**
 * @brief Tracks the operation mode; leaving Project File mode forgets what was read there.
 */
void DataModel::DeclarationWatch::onOperationMode(const int mode)
{
  SS_ASSERT(mode >= SerialStudio::ProjectFile && mode <= SerialStudio::QuickPlot, return);
  if (m_mode == mode)
    return;

  m_mode = mode;
  forget();
}

/**
 * @brief On a connect edge in Quick Plot with the toggle on, asks the device for its declaration
 *        (spec 0111 R6). Console Only and Project File never send it, and neither does a sealed
 *        session. A disconnect drops the partial line, and in Project File mode the declaration
 *        too: it described that device, and the next link may be another one.
 */
void DataModel::DeclarationWatch::onLinkState(const Core::Bus::ConnectionStateChanged& link)
{
  SS_ASSERT(link.sourceId >= 0, return);
  if (link.connected == m_connected)
    return;

  m_connected = link.connected;
  m_carry.clear();
  if (!m_connected && m_mode == SerialStudio::ProjectFile)
    forget();

  if (!m_connected || !m_requestOnConnect || m_mode != SerialStudio::QuickPlot
      || Core::Runtime::sealed())
    return;

  const QByteArray request = QByteArray(Declaration::kRequest.data(), Declaration::kRequest.size())
                           + QByteArrayLiteral("\n");
  SS_ASSERT(request.size() <= Declaration::kMaxLineBytes, return);
  const auto written = DataModel::requireDeviceWriter().writeDataToDevice(link.sourceId, request);
  if (written <= 0)
    qInfo().noquote() << QStringLiteral("Device declaration request not sent: the write failed");
}

//--------------------------------------------------------------------------------------------------
// Project File watch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the primary device's bytes for declaration lines in Project File mode (spec 0111
 *        R3), and in a Quick Plot binary session, where the reader frames on the binary delimiters
 *        and only this tap can still see a re-declaration (R14). The loaded project is never
 *        touched from here.
 */
void DataModel::DeclarationWatch::onDeviceBytes(const int deviceId, const IO::CapturedDataPtr& data)
{
  SS_ASSERT(deviceId >= 0, return);
  const bool binary = m_binarySession && m_mode == SerialStudio::QuickPlot;
  if (deviceId != 0 || !data || Core::Runtime::sealed()
      || (m_mode != SerialStudio::ProjectFile && !binary))
    return;

  m_chunkTime = data->timestamp;
  scanChunk(QByteArrayView(data->data));
}

/**
 * @brief Finds prefixed lines in one chunk. A candidate is a '#' whose next bytes match the prefix,
 *        compared in place, so a stream of other '#' frames costs one search per '#' and copies
 *        nothing; only a matching line is collected (collectLine), and a prefix the chunk ends
 *        inside waits in the carry for the next chunk.
 */
void DataModel::DeclarationWatch::scanChunk(QByteArrayView chunk)
{
  const QByteArrayView prefix(Declaration::kPrefix.data(), Declaration::kPrefix.size());
  SS_ASSERT(!prefix.isEmpty(), return);

  qsizetype cursor      = resumeCarry(chunk, prefix);
  const qsizetype limit = chunk.size() + 1;
  for (qsizetype step = 0; cursor >= 0 && cursor < chunk.size() && step < limit; ++step) {
    cursor = chunk.indexOf('#', cursor);
    if (cursor < 0)
      return;

    const auto avail = (std::min)(prefix.size(), chunk.size() - cursor);
    if (chunk.sliced(cursor, avail) != prefix.first(avail)) {
      ++cursor;
      continue;
    }

    if (avail < prefix.size()) {
      m_carry = chunk.sliced(cursor).toByteArray();
      return;
    }

    cursor = collectLine(chunk, cursor);
  }
}

/**
 * @brief Continues a candidate the previous chunk ended inside: completes a partial prefix, or
 *        drops it on a mismatch, then collects the rest of a matching line. Returns where the
 *        scan resumes, or -1 when this chunk is consumed.
 */
qsizetype DataModel::DeclarationWatch::resumeCarry(QByteArrayView chunk, QByteArrayView prefix)
{
  SS_ASSERT(!prefix.isEmpty(), return -1);
  SS_ASSERT(m_carry.size() <= Declaration::kMaxLineBytes + 1, return -1);
  if (m_carry.isEmpty())
    return 0;

  qsizetype cursor = 0;
  if (m_carry.size() < prefix.size()) {
    const auto take = (std::min)(prefix.size() - m_carry.size(), chunk.size());
    if (chunk.first(take) != prefix.sliced(m_carry.size(), take)) {
      m_carry.clear();
      return 0;
    }

    m_carry.append(chunk.first(take));
    if (m_carry.size() < prefix.size())
      return -1;

    cursor = take;
  }

  return collectLine(chunk, cursor);
}

/**
 * @brief Appends a matching line from @p from to its newline, at most one byte past the line cap
 *        so an oversized line is still refused by the parser with its reason, and offers it once
 *        complete. Returns the byte after the newline, or -1 when the line continues.
 */
qsizetype DataModel::DeclarationWatch::collectLine(QByteArrayView chunk, const qsizetype from)
{
  SS_ASSERT(from >= 0 && from <= chunk.size(), return -1);
  const qsizetype room = Declaration::kMaxLineBytes + 1 - m_carry.size();
  SS_ASSERT(room >= 0, return -1);

  const auto newline = chunk.indexOf('\n', from);
  const auto stop    = newline < 0 ? chunk.size() : newline;
  m_carry.append(chunk.sliced(from, (std::min)(stop - from, room)));
  if (newline < 0)
    return -1;

  offerLine(m_carry);
  m_carry.clear();
  return newline + 1;
}

/**
 * @brief Routes one complete prefixed line. The app's own request and command lines, echoed
 *        back, are not declarations; a binary session's line goes to the frame builder, which
 *        owns the Quick Plot declaration.
 */
void DataModel::DeclarationWatch::offerLine(QByteArrayView raw)
{
  const auto line = trimLineAscii(raw);
  SS_ASSERT(raw.size() >= line.size(), return);
  if (!isDeclarationContent(line))
    return;

  if (m_mode == SerialStudio::QuickPlot)
    forwardLine(line);
  else
    acceptLine(line);
}

/**
 * @brief Posts a binary session's declaration line to the frame builder, whose DeclarationState
 *        dedups and holds it (spec 0111 R4/R14). A repeat waits a second, so a held line the
 *        device sends again still arrives; any line waits a tenth of a second after the last.
 */
void DataModel::DeclarationWatch::forwardLine(QByteArrayView line)
{
  SS_ASSERT(m_mode == SerialStudio::QuickPlot, return);
  SS_ASSERT(isDeclarationLine(line), return);
  const auto elapsed = m_chunkTime - m_lastForward;
  const bool repeat  = line == QByteArrayView(m_lastLine);
  if (elapsed < kForwardSpacing || (repeat && elapsed < kForwardRepeat))
    return;

  m_lastLine    = line.toByteArray();
  m_lastForward = m_chunkTime;
  DataModel::pipelineModules().frameBuilder.submitDeclarationLine(m_lastLine, m_chunkTime);
}

/**
 * @brief Parses a Project File declaration at most once per content, with the Quick Plot lane's
 *        one-second hold (spec 0111 R4/R10): a refused one is logged with its reason, an accepted
 *        one is projected once and compared with the loaded project.
 */
void DataModel::DeclarationWatch::acceptLine(QByteArrayView line)
{
  SS_ASSERT(m_mode == SerialStudio::ProjectFile, return);
  const auto outcome = m_state.offer(line, m_chunkTime);
  if (outcome == DeclarationOutcome::Unchanged)
    return;

  qInfo().noquote() << m_state.message();
  if (outcome == DeclarationOutcome::Held)
    return;

  m_reevaluateTimer.stop();
  if (outcome == DeclarationOutcome::Rejected) {
    m_reason      = m_state.reason();
    m_declaration = DeviceDeclaration();
    m_declaredContent.clear();
    m_declaredProjection = QJsonObject();
    setStatus(DeclarationStatus::Rejected);
    return;
  }

  m_reason.clear();
  m_declaration        = m_state.current();
  m_declaredProjection = wireProjection(QString(), m_declaration.groups);
  m_declaredContent    = QJsonDocument(wireProjection(m_declaration.title, m_declaration.groups))
                           .toJson(QJsonDocument::Compact);
  reevaluate();
}

/**
 * @brief Coalesces project edits into one comparison: a dataset rename emits only
 *        modifiedChanged, a group edit groupsChanged or groupDataChanged, and a burst of either
 *        compares once.
 */
void DataModel::DeclarationWatch::scheduleReevaluate()
{
  if (m_declaration.valid && m_mode == SerialStudio::ProjectFile)
    m_reevaluateTimer.start();
}

/**
 * @brief Compares the held declaration with the loaded project on the keys a device can say,
 *        ignoring the title (a saved project takes its file's name), so an adopt or a hand edit
 *        that matches clears the offer and a kept pair stays quiet.
 */
void DataModel::DeclarationWatch::reevaluate()
{
  m_reevaluateTimer.stop();
  if (!m_declaration.valid || m_mode != SerialStudio::ProjectFile)
    return;

  SS_ASSERT(!m_declaration.groups.empty(), return);
  SS_ASSERT(!m_declaredProjection.isEmpty(), return);
  if (m_declaredProjection == wireProjection(QString(), m_model.groups())) {
    setStatus(DeclarationStatus::Matches);
    return;
  }

  if (m_kept.contains(noticeKey())) {
    setStatus(DeclarationStatus::Kept);
    return;
  }

  setStatus(DeclarationStatus::Differs);
  notifyDifference();
}

/**
 * @brief The (loaded project, declaration content) pair a notice or a keep is remembered by.
 */
QByteArray DataModel::DeclarationWatch::noticeKey() const
{
  SS_ASSERT(!m_declaredContent.isEmpty(), return QByteArray());
  QCryptographicHash hash(QCryptographicHash::Sha1);
  hash.addData(m_model.jsonFilePath().toUtf8());
  hash.addData(QByteArrayView("\n"));
  hash.addData(m_declaredContent);
  return hash.result();
}

/**
 * @brief Says so once per loaded project and declaration content (spec 0111 R3), through the bus
 *        the notification center listens on.
 */
void DataModel::DeclarationWatch::notifyDifference()
{
  SS_ASSERT(m_declaration.valid, return);
  SS_ASSERT(m_noticed.size() <= kMaxNotices, return);
  const auto key = noticeKey();
  if (m_noticed.contains(key) || m_noticed.size() >= kMaxNotices)
    return;

  m_noticed.insert(key);
  m_bus.publish<Core::Bus::NotificationRaised>(
    Core::Bus::kSeverityInfo,
    QStringLiteral("Device Declaration"),
    QString(),
    tr("The device declares a different dashboard"),
    tr("\"%1\" differs from the loaded project. In the Setup panel, Use Device Dashboard opens it "
       "as a new, unsaved project, and Keep Project leaves the loaded project as it is.")
      .arg(m_declaration.title));
}

//--------------------------------------------------------------------------------------------------
// Adopt and keep
//--------------------------------------------------------------------------------------------------

/**
 * @brief Keeps the loaded project (spec 0111 R3): the pending difference is dropped for this
 *        project and declaration content, and returns only for another content or project.
 */
void DataModel::DeclarationWatch::keep()
{
  if (!pendingDifference())
    return;

  SS_ASSERT(m_kept.size() <= kMaxNotices, return);
  if (m_kept.size() >= kMaxNotices)
    m_kept.clear();

  m_kept.insert(noticeKey());
  setStatus(DeclarationStatus::Kept);
}

/**
 * @brief Opens the pending declaration as a new, unsaved project (spec 0111 R3) with the loaded
 *        project's primary connection, after the save prompt and a disconnect. The load switches
 *        no mode and leaves the loaded file untouched; the project is built before the prompt,
 *        whose event loop may deliver a newer declaration.
 */
bool DataModel::DeclarationWatch::adopt()
{
  if (!pendingDifference() || !m_declaration.valid || m_mode != SerialStudio::ProjectFile
      || Core::Runtime::sealed())
    return false;

  SS_ASSERT(!m_declaration.groups.empty(), return false);
  SS_ASSERT(std::ssize(m_declaration.groups) <= Declaration::kMaxGroups, return false);
  int busType = static_cast<int>(SerialStudio::BusType::UART);
  QJsonObject settings;
  const auto& sources = m_model.sources();
  const auto driver   = m_bus.latest<Core::Bus::ActiveUiDriverSettings>();
  if (!sources.empty()) {
    busType  = sources.front().busType;
    settings = sources.front().connectionSettings;
  } else if (driver) {
    busType  = driver->busType;
    settings = driver->settings;
  }

  const auto project = QuickPlotProject::build(m_declaration.groups,
                                               busType,
                                               settings,
                                               LineFormat(),
                                               m_declaration.title,
                                               m_declaration.source);
  if (project.isEmpty() || !m_model.askSave())
    return false;

  m_bus.publish<Core::Bus::DisconnectRequested>(0);
  if (!m_model.loadGeneratedProject(QJsonDocument(project)))
    return false;

  reevaluate();
  return true;
}
