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

#include "API/Mirror/MirrorSession.h"

#include <algorithm>
#include <limits>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include "API/Mirror/MirrorClient.h"
#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "SerialStudio.h"
#include "SessionContext.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constants & module state
//--------------------------------------------------------------------------------------------------

constexpr int kMaxRecentEndpoints = 12;
constexpr int kValueDigits        = 15;

/**
 * @brief Band the mirror stamps its structure generations in. The dashboard compares that number
 *        against the local frame pool's generation to decide whether to reconfigure, and the two
 *        counters are independent, so a mirrored epoch must not be able to alias a local one.
 */
constexpr quint64 kMirrorGenerationBase = 1ULL << 48;

/**
 * @brief Whether a viewer session is attached. A plain module flag rather than a query on the
 *        session object, because UI::Dashboard::streamAvailable() is reached from the Dashboard
 *        constructor and must not construct a module that is built after the pinned order.
 */
static bool s_mirroring = false;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves a sparse side-map entry for one positional slot, or an empty string when the
 *        slot carries no out-of-band value.
 */
static QString sideValue(const QJsonObject& map, const int index)
{
  if (map.isEmpty())
    return QString();

  return map.value(QString::number(index)).toString();
}

/**
 * @brief Position of @p sourceId in the structure's sourceIds array, which is the array a
 *        snapshot's per-source timestamps are parallel to. Zero when absent, so a malformed
 *        structure reads the first stamp rather than indexing out of the array.
 */
static int wireIndexOf(const QJsonArray& sourceIds, const int sourceId)
{
  for (int i = 0; i < sourceIds.size(); ++i)
    if (sourceIds.at(i).toInt(-1) == sourceId)
      return i;

  return 0;
}

/**
 * @brief Turns a wire non-finite tag back into the double it stands for; JSON has no spelling for
 *        one, so they travel as tags and are restored here.
 */
static double nonFiniteValue(const QString& tag)
{
  if (tag == QLatin1String(API::Mirror::NonFinite::NaN))
    return std::numeric_limits<double>::quiet_NaN();

  if (tag == QLatin1String(API::Mirror::NonFinite::NegInfinity))
    return -std::numeric_limits<double>::infinity();

  return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the viewer session. The dashboard link is the one cached-flag input this spec
 *        adds: it is direct so a snapshot arriving in the same event-loop turn as the attach is
 *        not dropped by a stream-available flag that is still a turn behind.
 */
API::MirrorSession::MirrorSession(SessionContext& ctx)
  : m_ctx(ctx)
  , m_client(new MirrorClient(this))
  , m_attached(false)
  , m_canAttach(true)
  , m_anchored(false)
  , m_epoch(0)
{
  connect(m_client, &MirrorClient::failed, this, &MirrorSession::onFailed);
  connect(m_client, &MirrorClient::snapshotReceived, this, &MirrorSession::onSnapshot);
  connect(m_client, &MirrorClient::structureReceived, this, &MirrorSession::onStructure);
  connect(m_client, &MirrorClient::liveChanged, this, &MirrorSession::onLinkStatusChanged);
  connect(m_client, &MirrorClient::staleChanged, this, &MirrorSession::onLinkStatusChanged);
  connect(m_client, &MirrorClient::linkedChanged, this, &MirrorSession::onLinkStatusChanged);

  connect(this,
          &MirrorSession::attachedChanged,
          &m_ctx.dashboard(),
          &UI::Dashboard::updateStreamAvailable,
          Qt::DirectConnection);

  connect(&m_ctx.connectionManager(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &MirrorSession::refreshCanAttach);
  connect(&m_ctx.dashboard(), &UI::Dashboard::dataReset, this, &MirrorSession::refreshCanAttach);

  refreshCanAttach();
}

/**
 * @brief Gets the singleton viewer session, bound to the session the composition root published.
 */
API::MirrorSession& API::MirrorSession::instance()
{
  static MirrorSession singleton(SessionContext::current());
  return singleton;
}

/**
 * @brief Whether a viewer session is attached, without constructing anything.
 */
bool API::MirrorSession::mirroring() noexcept
{
  return s_mirroring;
}

//--------------------------------------------------------------------------------------------------
// State access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Negotiated mirror cadence, in hertz.
 */
int API::MirrorSession::hz() const
{
  return m_client->hz();
}

/**
 * @brief Whether value snapshots are still arriving from the remote.
 */
bool API::MirrorSession::live() const
{
  return m_client->live();
}

/**
 * @brief Whether the link has gone silent past the watchdog; the dashboard must not present its
 *        last values as current while this is true.
 */
bool API::MirrorSession::stale() const
{
  return m_client->stale();
}

/**
 * @brief Whether a remote dashboard is attached.
 */
bool API::MirrorSession::attached() const noexcept
{
  return m_attached;
}

/**
 * @brief Whether attaching is allowed right now. v1 keeps one dashboard and swaps its feed, so a
 *        local capture and a remote view cannot both own it; refusing is the only honest answer.
 */
bool API::MirrorSession::canAttach() const
{
  return m_canAttach;
}

/**
 * @brief Number of mirrored datasets in the current epoch.
 */
int API::MirrorSession::datasetCount() const noexcept
{
  return static_cast<int>(m_slots.size());
}

/**
 * @brief host:port of the attached remote, empty when detached.
 */
QString API::MirrorSession::endpoint() const
{
  return m_attached ? m_client->endpoint() : QString();
}

/**
 * @brief Application version the remote reported, for the attach dialog's status line.
 */
QString API::MirrorSession::remoteVersion() const
{
  return m_client->info().value(QStringLiteral("appVersion")).toString();
}

/**
 * @brief Message of the last attach or link failure.
 */
const QString& API::MirrorSession::lastError() const noexcept
{
  return m_lastError;
}

/**
 * @brief Code of the last failure: refused, unauthorized, version mismatch, unreachable, or the
 *        local refusal that a capture is running.
 */
const QString& API::MirrorSession::lastErrorCode() const noexcept
{
  return m_lastErrorCode;
}

/**
 * @brief Previously used endpoints, most recent first. Tokens are never persisted.
 */
QStringList API::MirrorSession::recentEndpoints() const
{
  return m_settings.value(QStringLiteral("Mirror/RecentEndpoints")).toStringList();
}

//--------------------------------------------------------------------------------------------------
// Attach & detach
//--------------------------------------------------------------------------------------------------

/**
 * @brief Attaches to a remote dashboard. The local session is snapshotted before anything is
 *        written, and the attached flag is raised before the first frame can arrive so the
 *        dashboard's cached stream flag is already true when it does.
 */
void API::MirrorSession::attach(const QString& host,
                                const int port,
                                const QString& token,
                                const int hz)
{
  if (host.isEmpty() || port <= 0 || port > 65535) {
    setError(QStringLiteral("MIRROR_BAD_ENDPOINT"), tr("Enter a host name and a port to attach"));
    return;
  }

  refreshCanAttach();
  if (!m_canAttach) {
    setError(QStringLiteral("MIRROR_LOCAL_BUSY"),
             tr("Disconnect the local device or close the open recording before attaching to a "
                "remote dashboard"));
    return;
  }

  if (m_attached)
    detach();

  captureLocalState();
  setError(QString(), QString());
  setAttached(true);

  m_client->open(host, static_cast<quint16>(port), token, hz);
  rememberEndpoint(QStringLiteral("%1:%2").arg(host, QString::number(port)));
}

/**
 * @brief Detaches and puts the local session back. The remote is told nothing beyond the socket
 *        closing, so its capture is unaffected.
 */
void API::MirrorSession::detach()
{
  m_client->close();

  if (!m_attached)
    return;

  setAttached(false);
  restoreLocalState();
  refreshCanAttach();
}

/**
 * @brief Publishes the attached flag. The module flag is written first so the direct dashboard
 *        refresh that follows already reads the new value.
 */
void API::MirrorSession::setAttached(const bool value)
{
  if (m_attached == value)
    return;

  m_attached  = value;
  s_mirroring = value;

  if (!value) {
    m_epoch    = 0;
    m_anchored = false;
    m_slots.clear();
    m_frames.clear();
    m_tNsIndex.clear();
    m_tNsAnchor.clear();
  }

  Q_EMIT attachedChanged();
  Q_EMIT statusChanged();
}

/**
 * @brief Recomputes whether the local session is free to be replaced by a remote view.
 */
void API::MirrorSession::refreshCanAttach()
{
  const bool value = m_attached || !m_ctx.dashboard().streamAvailable();
  if (m_canAttach == value)
    return;

  m_canAttach = value;
  Q_EMIT canAttachChanged();
}

//--------------------------------------------------------------------------------------------------
// Local session snapshot
//--------------------------------------------------------------------------------------------------

/**
 * @brief Captures everything attach is about to replace, before the first write.
 */
void API::MirrorSession::captureLocalState()
{
  auto& project = m_ctx.projectModel();

  m_local.valid         = true;
  m_local.path          = project.jsonFilePath();
  m_local.project       = project.serializeToJson();
  m_local.frozen        = project.frozen();
  m_local.modified      = project.modified();
  m_local.plotRange     = project.plotTimeRange();
  m_local.operationMode = static_cast<int>(m_ctx.appState().operationMode());
}

/**
 * @brief Puts the local session back. Runs on every exit path, including an abnormal disconnect,
 *        because losing the user's open project to a dropped socket is the failure this feature
 *        would otherwise be remembered for.
 */
void API::MirrorSession::restoreLocalState()
{
  if (!m_local.valid)
    return;

  auto& project   = m_ctx.projectModel();
  auto& appState  = m_ctx.appState();
  const auto mode = static_cast<SerialStudio::OperationMode>(m_local.operationMode);

  appState.setOperationMode(SerialStudio::ProjectFile);
  if (!project.loadFromJsonDocument(QJsonDocument(m_local.project), m_local.path))
    project.newJsonFile();

  project.setPlotTimeRange(m_local.plotRange);
  project.setFrozen(m_local.frozen);
  project.setModified(m_local.modified);
  appState.setOperationMode(mode);

  m_local = LocalState();
}

//--------------------------------------------------------------------------------------------------
// Endpoint history
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves @p endpoint to the front of the remembered list. Only the address is stored; a
 *        token is a credential and this viewer never writes one to disk.
 */
void API::MirrorSession::rememberEndpoint(const QString& endpoint)
{
  SS_ASSERT(!endpoint.isEmpty(), return);

  auto list = recentEndpoints();
  list.removeAll(endpoint);
  list.prepend(endpoint);
  while (list.size() > kMaxRecentEndpoints)
    list.removeLast();

  m_settings.setValue(QStringLiteral("Mirror/RecentEndpoints"), list);
  Q_EMIT recentEndpointsChanged();
}

/**
 * @brief Drops one endpoint from the remembered list.
 */
void API::MirrorSession::forgetEndpoint(const QString& endpoint)
{
  auto list = recentEndpoints();
  if (list.removeAll(endpoint) <= 0)
    return;

  m_settings.setValue(QStringLiteral("Mirror/RecentEndpoints"), list);
  Q_EMIT recentEndpointsChanged();
}

//--------------------------------------------------------------------------------------------------
// Structure adoption
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts a new epoch: loads the mirrored project through the ordinary in-memory load path
 *        and rebuilds the per-source frame templates the snapshots are assigned into.
 */
void API::MirrorSession::onStructure(const QJsonObject& structure)
{
  if (!m_attached)
    return;

  m_epoch    = static_cast<quint64>(structure.value(QStringLiteral("epoch")).toInteger(0));
  m_anchored = false;

  if (!applyRemoteState(structure) || !buildTemplates(structure)) {
    detach();
    return;
  }

  Q_EMIT attachedChanged();
  Q_EMIT statusChanged();
}

/**
 * @brief Loads the remote's project document and display state. The source path is deliberately
 *        empty: a mirrored project is not the viewer's file, and an empty path is what keeps the
 *        debounced autosave from writing the remote's layout over the user's own project.
 */
bool API::MirrorSession::applyRemoteState(const QJsonObject& structure)
{
  auto& project = m_ctx.projectModel();

  m_ctx.appState().setOperationMode(SerialStudio::ProjectFile);
  const auto document = QJsonDocument(structure.value(QStringLiteral("project")).toObject());
  if (!project.loadFromJsonDocument(document)) {
    setError(QStringLiteral("MIRROR_BAD_PROJECT"),
             tr("The remote sent a project this build cannot load"));
    return false;
  }

  const auto range = SerialStudio::toDouble(structure.value(QStringLiteral("plotTimeRange")));
  if (range > 0)
    project.setPlotTimeRange(range);

  project.setFrozen(structure.value(QStringLiteral("frozen")).toBool(false));
  return true;
}

/**
 * @brief Builds one template frame for the given source, filtered exactly as the capture side
 *        filters it, so the dataset walk order the positional wire format is defined over is the
 *        same on both ends.
 */
DataModel::Frame API::MirrorSession::buildSourceFrame(const int sourceId) const
{
  const auto& project = m_ctx.projectModel();

  DataModel::Frame frame;
  frame.sourceId = sourceId;
  frame.title    = project.title();
  frame.actions  = project.actions();

  for (const auto& group : project.groups()) {
    if (!group.enabled || group.sourceId != sourceId)
      continue;

    DataModel::Group runtime = group;
    runtime.datasets.clear();
    for (const auto& dataset : group.datasets)
      if (dataset.enabled)
        runtime.datasets.push_back(dataset);

    frame.groups.push_back(std::move(runtime));
  }

  DataModel::finalize_frame(frame);
  return frame;
}

/**
 * @brief Rebuilds the per-source templates and the positional slot table for the current epoch,
 *        then refuses the epoch unless the layout hash computed over the templates this viewer
 *        built equals the one the remote announced. That equality is the whole safety argument
 *        for assigning bare numbers by position.
 */
bool API::MirrorSession::buildTemplates(const QJsonObject& structure)
{
  const auto wireSources = structure.value(QStringLiteral("sourceIds")).toArray();

  std::vector<int> sourceIds;
  for (const auto& value : wireSources)
    sourceIds.push_back(value.toInt(0));

  std::sort(sourceIds.begin(), sourceIds.end());
  sourceIds.erase(std::unique(sourceIds.begin(), sourceIds.end()), sourceIds.end());

  m_slots.clear();
  m_frames.clear();
  m_tNsIndex.clear();

  std::vector<Mirror::DatasetId> identities;
  for (const int sourceId : sourceIds) {
    auto frame = buildSourceFrame(sourceId);
    if (frame.groups.empty())
      continue;

    const int frameIndex = static_cast<int>(m_frames.size());
    for (std::size_t g = 0; g < frame.groups.size(); ++g)
      for (std::size_t d = 0; d < frame.groups[g].datasets.size(); ++d) {
        m_slots.push_back({frameIndex, static_cast<int>(g), static_cast<int>(d)});
        identities.push_back({sourceId, frame.groups[g].datasets[d].uniqueId});
      }

    auto pointer                 = std::make_shared<DataModel::TimestampedFrame>(std::move(frame));
    pointer->structureGeneration = kMirrorGenerationBase + m_epoch;
    m_frames.push_back(std::move(pointer));
    m_tNsIndex.push_back(wireIndexOf(wireSources, sourceId));
  }

  m_tNsAnchor.assign(m_frames.size(), 0);

  const auto announced = structure.value(QStringLiteral("layoutHash")).toString();
  if (announced == Mirror::layoutHash(identities))
    return true;

  setError(QStringLiteral("MIRROR_LAYOUT_MISMATCH"),
           tr("This build resolves the remote project to a different dataset layout"));
  return false;
}

//--------------------------------------------------------------------------------------------------
// Snapshot injection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one snapshot and publishes it. The epoch and slot-count checks repeat what the
 *        transport already enforced: there must be no path anywhere that assigns values against a
 *        structure they do not match.
 */
void API::MirrorSession::onSnapshot(const QJsonObject& snapshot)
{
  const auto epoch  = static_cast<quint64>(snapshot.value(QStringLiteral("epoch")).toInteger(0));
  const auto values = snapshot.value(QStringLiteral("values")).toArray();

  if (!m_attached || m_frames.empty() || epoch != m_epoch)
    return;

  if (static_cast<std::size_t>(values.size()) != m_slots.size())
    return;

  assignValues(snapshot);
  publishFrames(snapshot);
}

/**
 * @brief Resolves the dataset behind one positional slot.
 */
DataModel::Dataset* API::MirrorSession::datasetAt(const std::size_t index)
{
  SS_ASSERT(index < m_slots.size(), return nullptr);

  const auto& slot = m_slots[index];
  auto& groups     = m_frames[static_cast<std::size_t>(slot.frame)]->data.groups;
  auto& datasets   = groups[static_cast<std::size_t>(slot.group)].datasets;
  return &datasets[static_cast<std::size_t>(slot.dataset)];
}

/**
 * @brief Writes the snapshot's values into the template frames. A null slot with no side-map
 *        entry means the remote dataset has no current value, so the previous one is kept rather
 *        than being replaced by a zero that would read as a live measurement.
 */
void API::MirrorSession::assignValues(const QJsonObject& snapshot)
{
  const auto values    = snapshot.value(QStringLiteral("values")).toArray();
  const auto strings   = snapshot.value(QStringLiteral("strings")).toObject();
  const auto nonFinite = snapshot.value(QStringLiteral("nonFinite")).toObject();

  for (int i = 0; i < values.size(); ++i) {
    auto* dataset = datasetAt(static_cast<std::size_t>(i));
    if (!dataset)
      return;

    const auto entry = values.at(i);
    if (entry.isDouble()) {
      dataset->isNumeric       = true;
      dataset->numericValue    = SerialStudio::toDouble(entry);
      dataset->rawNumericValue = dataset->numericValue;
      dataset->value           = QString::number(dataset->numericValue, 'g', kValueDigits);
      dataset->rawValue        = dataset->value;
      continue;
    }

    const auto tag = sideValue(nonFinite, i);
    if (!tag.isEmpty()) {
      dataset->isNumeric       = true;
      dataset->numericValue    = nonFiniteValue(tag);
      dataset->rawNumericValue = dataset->numericValue;
      dataset->value           = tag;
      dataset->rawValue        = tag;
      continue;
    }

    const auto text = sideValue(strings, i);
    if (!text.isEmpty()) {
      dataset->isNumeric    = false;
      dataset->value        = text;
      dataset->rawValue     = text;
      dataset->numericValue = (dataset->wgtMax > dataset->wgtMin) ? dataset->wgtMin : 0.0;
    }
  }
}

/**
 * @brief Publishes one frame per mirrored source, rebuilding each source's timestamp from the
 *        wire's relative nanoseconds against a local anchor taken at the epoch's first snapshot,
 *        so plot geometry stays the remote's and only the absolute origin is local. Nothing here
 *        reaches FrameBuilder::hotpathTxFrame: a viewer's export sinks never see a mirrored frame.
 */
void API::MirrorSession::publishFrames(const QJsonObject& snapshot)
{
  const auto times = snapshot.value(QStringLiteral("tNs")).toArray();

  if (!m_anchored) {
    m_anchored    = true;
    m_localAnchor = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < m_frames.size(); ++i)
      m_tNsAnchor[i] = times.at(m_tNsIndex[i]).toInteger(0);
  }

  auto& dashboard = m_ctx.dashboard();
  for (std::size_t i = 0; i < m_frames.size(); ++i) {
    const qint64 stamp = times.at(m_tNsIndex[i]).toInteger(m_tNsAnchor[i]);
    const auto delta   = std::chrono::nanoseconds(qMax<qint64>(0, stamp - m_tNsAnchor[i]));

    m_frames[i]->timestamp = m_localAnchor + delta;
    dashboard.hotpathRxFrame(m_frames[i]);
  }
}

//--------------------------------------------------------------------------------------------------
// Link status
//--------------------------------------------------------------------------------------------------

/**
 * @brief Republishes the link's liveness to QML.
 */
void API::MirrorSession::onLinkStatusChanged()
{
  Q_EMIT statusChanged();
}

/**
 * @brief Handles a link failure. A failure the transport cannot retry past leaves the viewer
 *        detached with the local session already restored, rather than parked on a dead endpoint.
 */
void API::MirrorSession::onFailed(const QString& code, const QString& message, const bool fatal)
{
  setError(code, message);

  if (fatal && m_attached) {
    detach();
    return;
  }

  Q_EMIT statusChanged();
}

/**
 * @brief Records and publishes the last failure.
 */
void API::MirrorSession::setError(const QString& code, const QString& message)
{
  if (m_lastErrorCode == code && m_lastError == message)
    return;

  m_lastErrorCode = code;
  m_lastError     = message;
  Q_EMIT lastErrorChanged();
}
