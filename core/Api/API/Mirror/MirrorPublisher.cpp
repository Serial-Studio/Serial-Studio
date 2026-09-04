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

#include "API/Mirror/MirrorPublisher.h"

#include <algorithm>
#include <cmath>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>

#include "API/Server.h"
#include "AppInfo.h"
#include "AppState.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "SessionContext.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kRateSlackMs = 1;
constexpr int kDefaultUiHz = 60;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wire identity of one dataset: its persisted uniqueId, or the legacy positional value
 *        when the project never assigned one.
 */
static int wireUniqueId(const DataModel::Dataset& dataset)
{
  if (dataset.uniqueId >= 0)
    return dataset.uniqueId;

  return DataModel::dataset_unique_id(dataset.sourceId, dataset.groupId, dataset.datasetId);
}

/**
 * @brief Tag a non-finite double travels under, since JSON has no spelling for one.
 */
static QString nonFiniteTag(const double value)
{
  if (std::isnan(value))
    return QString::fromLatin1(API::Mirror::NonFinite::NaN);

  return QString::fromLatin1(value > 0 ? API::Mirror::NonFinite::Infinity
                                       : API::Mirror::NonFinite::NegInfinity);
}

/**
 * @brief Maps one live dataset onto its positional snapshot slot.
 */
static API::Mirror::SnapshotValue snapshotValue(const DataModel::Dataset& dataset)
{
  API::Mirror::SnapshotValue value;

  if (dataset.isNumeric && std::isfinite(dataset.numericValue)) {
    value.kind   = API::Mirror::SnapshotValue::Kind::Number;
    value.number = dataset.numericValue;
    return value;
  }

  if (dataset.isNumeric) {
    value.kind = API::Mirror::SnapshotValue::Kind::NonFinite;
    value.text = nonFiniteTag(dataset.numericValue);
    return value;
  }

  if (!dataset.value.isEmpty()) {
    value.kind = API::Mirror::SnapshotValue::Kind::Text;
    value.text = dataset.value;
  }

  return value;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the publisher. Nothing is wired here: the dashboard, epoch, and heartbeat
 *        links are made only while at least one viewer is subscribed.
 */
API::MirrorPublisher::MirrorPublisher(SessionContext& ctx)
  : m_ctx(ctx), m_epoch(1), m_seq(0), m_structureValid(false), m_lastSnapshot(0)
{
  m_clock.start();
  m_heartbeat.setInterval(Mirror::kHeartbeatIntervalMs);
  connect(&m_heartbeat, &QTimer::timeout, this, &MirrorPublisher::onHeartbeatTimeout);
}

/**
 * @brief Gets the singleton publisher, bound to the session the composition root published.
 */
API::MirrorPublisher& API::MirrorPublisher::instance()
{
  static MirrorPublisher singleton(SessionContext::current());
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Viewer accounting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Number of currently subscribed viewers.
 */
int API::MirrorPublisher::viewers() const noexcept
{
  return m_subscribers.size();
}

/**
 * @brief Viewer ceiling: the API/MaxViewers setting when configured, the server's own client
 *        cap otherwise. Zero refuses viewers outright while leaving the API server usable.
 */
int API::MirrorPublisher::maxViewers() const
{
  const int configured = m_settings.value(QStringLiteral("API/MaxViewers"), -1).toInt();
  if (configured < 0)
    return Server::maxClients();

  return qMin(configured, Server::maxClients());
}

/**
 * @brief Whether this instance accepts mirror viewers at all.
 */
bool API::MirrorPublisher::viewersAllowed() const
{
  return maxViewers() > 0;
}

/**
 * @brief Current structure epoch; every snapshot carries it and a client drops any mismatch.
 */
quint64 API::MirrorPublisher::epoch() const noexcept
{
  return m_epoch;
}

/**
 * @brief Whether the given API connection holds a mirror subscription.
 */
bool API::MirrorPublisher::subscribed(const QString& sessionId) const
{
  return m_subscribers.contains(sessionId);
}

/**
 * @brief Rate this publisher can actually serve: the request, capped by the local UI tick that
 *        wakes it, so a client sizes its staleness watchdog against reality.
 */
int API::MirrorPublisher::effectiveHz(const int hz) const
{
  const int uiHz = m_settings.value(QStringLiteral("uiRefreshRate"), kDefaultUiHz).toInt();
  return qBound(Mirror::kHzMin, qMin(hz, qMax(Mirror::kHzMin, uiHz)), Mirror::kHzMax);
}

//--------------------------------------------------------------------------------------------------
// Subscription management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers or renews a viewer. The first subscriber is what wires the publisher to the
 *        dashboard; refuses past the viewer ceiling.
 */
bool API::MirrorPublisher::subscribe(QTcpSocket* socket,
                                     const QString& sessionId,
                                     const int hz,
                                     const int precision)
{
  SS_ASSERT(socket != nullptr, return false);
  SS_ASSERT(!sessionId.isEmpty(), return false);

  const bool renewal = m_subscribers.contains(sessionId);
  if (!renewal && m_subscribers.size() >= maxViewers())
    return false;

  Subscriber subscriber;
  subscriber.socket    = socket;
  subscriber.sessionId = sessionId;
  subscriber.hz        = effectiveHz(hz);
  subscriber.precision = qBound(Mirror::kPrecisionMin, precision, Mirror::kPrecisionMax);

  const bool first = m_subscribers.isEmpty();
  m_subscribers.insert(sessionId, subscriber);

  if (first)
    activate();

  Q_EMIT viewersChanged();
  return true;
}

/**
 * @brief Renegotiates one viewer's cadence; false when it holds no subscription.
 */
bool API::MirrorPublisher::setRate(const QString& sessionId, const int hz)
{
  SS_ASSERT(!sessionId.isEmpty(), return false);
  SS_ASSERT_LOG(hz >= Mirror::kHzMin && hz <= Mirror::kHzMax);

  const auto it = m_subscribers.find(sessionId);
  if (it == m_subscribers.end())
    return false;

  it->hz = effectiveHz(hz);
  return true;
}

/**
 * @brief Drops one viewer. Never re-enables that connection's frame stream: only mirror.subscribe
 *        changes the opt-out, so unsubscribing cannot reopen the firehose on a slow reader.
 */
void API::MirrorPublisher::unsubscribe(const QString& sessionId)
{
  if (m_subscribers.remove(sessionId) == 0)
    return;

  if (m_subscribers.isEmpty())
    deactivate();

  Q_EMIT viewersChanged();
}

/**
 * @brief Drops every viewer, used when the server closes all of its sockets at once.
 */
void API::MirrorPublisher::clearSubscribers()
{
  if (m_subscribers.isEmpty())
    return;

  m_subscribers.clear();
  deactivate();

  Q_EMIT viewersChanged();
}

//--------------------------------------------------------------------------------------------------
// Activation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the publisher to the display tick and the two structural signals. Called on the
 *        transition to one viewer, so an unwatched instance runs no mirror code at all.
 */
void API::MirrorPublisher::activate()
{
  SS_ASSERT(!m_subscribers.isEmpty(), return);
  SS_ASSERT_LOG(!m_updatedLink);

  auto* dashboard = &m_ctx.dashboard();
  m_updatedLink =
    connect(dashboard, &UI::Dashboard::updated, this, &MirrorPublisher::onDashboardUpdated);
  m_widgetCountLink = connect(
    dashboard, &UI::Dashboard::widgetCountChanged, this, &MirrorPublisher::onStructureChanged);
  m_dataResetLink =
    connect(dashboard, &UI::Dashboard::dataReset, this, &MirrorPublisher::onStructureChanged);

  invalidateStructure();
  m_lastSnapshot = m_clock.elapsed();
  m_heartbeat.start();
}

/**
 * @brief Breaks every link made by activate() once the last viewer leaves.
 */
void API::MirrorPublisher::deactivate()
{
  SS_ASSERT(m_subscribers.isEmpty(), return);
  SS_ASSERT_LOG(m_heartbeat.isActive() || !m_updatedLink);

  m_heartbeat.stop();
  disconnect(m_updatedLink);
  disconnect(m_widgetCountLink);
  disconnect(m_dataResetLink);

  m_updatedLink     = QMetaObject::Connection();
  m_widgetCountLink = QMetaObject::Connection();
  m_dataResetLink   = QMetaObject::Connection();
}

//--------------------------------------------------------------------------------------------------
// Structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the epoch stale after a layout change; the next tick rebuilds and every viewer
 *        is sent the new structure before any snapshot carrying the new epoch.
 */
void API::MirrorPublisher::onStructureChanged()
{
  ++m_epoch;
  invalidateStructure();
}

/**
 * @brief Drops the cached structure payload and its encoded lines.
 */
void API::MirrorPublisher::invalidateStructure()
{
  m_structureValid = false;
  m_structure      = QJsonObject();
  m_structureLines.clear();
}

/**
 * @brief Guarantees a current structure. The cache is only trusted while viewers are attached,
 *        because the signals that invalidate it are wired only then.
 */
void API::MirrorPublisher::ensureStructure()
{
  if (m_subscribers.isEmpty())
    invalidateStructure();

  if (!m_structureValid)
    rebuildStructure();
}

/**
 * @brief Rebuilds the ordered identity list, the layout hash, and the structure payload from the
 *        dashboard's own combined frame, which is already ordered by ascending sourceId, then
 *        group, then dataset. A layout that changed while nobody watched bumps the epoch here,
 *        so a reattaching viewer can never apply values against the structure it used to hold.
 */
void API::MirrorPublisher::rebuildStructure()
{
  auto& dashboard   = m_ctx.dashboard();
  const auto& frame = dashboard.rawFrame();

  m_datasets.clear();
  m_sourceIds.clear();
  for (const auto& group : frame.groups)
    for (const auto& dataset : group.datasets) {
      m_datasets.push_back({dataset.sourceId, wireUniqueId(dataset)});
      m_sourceIds.push_back(dataset.sourceId);
    }

  std::sort(m_sourceIds.begin(), m_sourceIds.end());
  m_sourceIds.erase(std::unique(m_sourceIds.begin(), m_sourceIds.end()), m_sourceIds.end());

  const auto hash = Mirror::layoutHash(m_datasets);
  if (!m_layoutHash.isEmpty() && hash != m_layoutHash)
    ++m_epoch;

  m_layoutHash = hash;
  m_tNs.assign(m_sourceIds.size(), 0);
  m_sourceOrigins.assign(m_sourceIds.size(), -1);

  m_structure = Mirror::encodeStructure(m_epoch,
                                        m_datasets,
                                        m_sourceIds,
                                        m_ctx.projectModel().serializeToJson(),
                                        static_cast<int>(m_ctx.appState().operationMode()),
                                        dashboard.plotTimeRange(),
                                        dashboard.frozen(),
                                        QDateTime::currentMSecsSinceEpoch());

  const auto line = Mirror::encodeLine(m_structure);
  m_structureLines.clear();
  if (line.size() <= Mirror::kStructureChunkBytes)
    m_structureLines.push_back(line);
  else
    for (const auto& chunk : Mirror::chunkStructure(m_structure))
      m_structureLines.push_back(Mirror::encodeLine(chunk));

  m_structureValid = true;
}

/**
 * @brief Number of parts a structure fetch takes; zero when the project is too large to mirror.
 */
int API::MirrorPublisher::structureParts()
{
  ensureStructure();
  return static_cast<int>(m_structureLines.size());
}

/**
 * @brief The current epoch's structure payload, as returned by mirror.getStructure.
 */
QJsonObject API::MirrorPublisher::structure()
{
  ensureStructure();
  return m_structure;
}

/**
 * @brief One part of a chunked structure payload; empty when the part does not exist.
 */
QJsonObject API::MirrorPublisher::structureChunk(const int part)
{
  ensureStructure();

  const auto chunks = Mirror::chunkStructure(m_structure);
  if (part < 0 || part >= static_cast<int>(chunks.size()))
    return QJsonObject();

  return chunks[static_cast<std::size_t>(part)];
}

/**
 * @brief The mirror.getInfo payload: what a client needs before it decides to attach.
 */
QJsonObject API::MirrorPublisher::info()
{
  const int parts = structureParts();

  QJsonArray sources;
  for (const int sourceId : m_sourceIds)
    sources.append(sourceId);

  QJsonArray commands;
  commands.append(QLatin1String(Mirror::Command::Subscribe));
  commands.append(QLatin1String(Mirror::Command::SetRate));
  commands.append(QLatin1String(Mirror::Command::Unsubscribe));

  QJsonObject result;
  result.insert(QStringLiteral("wireVersion"), Mirror::kWireVersion);
  result.insert(QStringLiteral("appVersion"), QStringLiteral(APP_VERSION));
  result.insert(QStringLiteral("sessionId"), QString::number(m_ctx.sessionId()));
  result.insert(QStringLiteral("epoch"), static_cast<qint64>(m_epoch));
  result.insert(QStringLiteral("layoutHash"), m_layoutHash);
  result.insert(QStringLiteral("operationMode"),
                static_cast<int>(m_ctx.appState().operationMode()));
  result.insert(QStringLiteral("datasetCount"), static_cast<int>(m_datasets.size()));
  result.insert(QStringLiteral("sourceIds"), sources);
  result.insert(QStringLiteral("viewersAllowed"), viewersAllowed());
  result.insert(QStringLiteral("maxViewers"), maxViewers());
  result.insert(QStringLiteral("viewers"), viewers());
  result.insert(QStringLiteral("structureParts"), parts);
  result.insert(QStringLiteral("commands"), commands);
  return result;
}

//--------------------------------------------------------------------------------------------------
// Publishing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a viewer's rate divider is due at @p now, in publisher milliseconds.
 */
bool API::MirrorPublisher::due(const Subscriber& subscriber, const qint64 now) const
{
  SS_ASSERT_LOG(subscriber.hz >= Mirror::kHzMin);
  SS_ASSERT_LOG(now >= 0);

  return (now - subscriber.lastSnapshot + kRateSlackMs) * subscriber.hz >= 1000;
}

/**
 * @brief Encodes this tick's snapshot at the requested precision, once per distinct precision.
 */
QByteArray API::MirrorPublisher::snapshotLine(const int precision)
{
  const auto cached = m_tickLines.constFind(precision);
  if (cached != m_tickLines.constEnd())
    return cached.value();

  const auto line =
    Mirror::encodeLine(Mirror::encodeSnapshot(m_epoch, m_seq, m_tNs, m_values, precision));
  m_tickLines.insert(precision, line);
  return line;
}

/**
 * @brief Sends the current epoch's structure to one viewer before it sees a snapshot for it.
 */
void API::MirrorPublisher::publishStructure(Subscriber& subscriber)
{
  SS_ASSERT(m_structureValid, return);
  SS_ASSERT(!subscriber.sessionId.isEmpty(), return);

  if (m_structureLines.empty()) {
    qWarning() << "[Mirror] Structure needs more than" << Mirror::kMaxStructureParts
               << "parts - viewer" << subscriber.sessionId << "cannot be served";
    return;
  }

  for (const auto& line : m_structureLines)
    Q_EMIT payloadReady(subscriber.socket, subscriber.sessionId, line);

  subscriber.lastEpoch = m_epoch;
}

/**
 * @brief Collects the positional values for this tick and advances the per-source relative
 *        clocks. Returns false when the dashboard's walk no longer matches the epoch's identity
 *        list, which forces a new epoch instead of publishing values against a stale layout.
 */
bool API::MirrorPublisher::collectValues()
{
  const auto& frame = m_ctx.dashboard().rawFrame();

  m_values.clear();
  m_values.reserve(m_datasets.size());

  for (const auto& group : frame.groups)
    for (const auto& dataset : group.datasets) {
      const auto index = m_values.size();
      if (index >= m_datasets.size() || m_datasets[index].uniqueId != wireUniqueId(dataset))
        return false;

      m_values.push_back(snapshotValue(dataset));
    }

  if (m_values.size() != m_datasets.size())
    return false;

  const qint64 nowNs = m_clock.nsecsElapsed();
  for (std::size_t i = 0; i < m_sourceOrigins.size(); ++i) {
    if (m_sourceOrigins[i] < 0)
      m_sourceOrigins[i] = nowNs;

    m_tNs[i] = nowNs - m_sourceOrigins[i];
  }

  return true;
}

/**
 * @brief Builds one snapshot per display tick and fans it out to every viewer whose divider is
 *        due. The snapshot is encoded once per distinct precision, never once per viewer.
 */
void API::MirrorPublisher::onDashboardUpdated()
{
  SS_ASSERT(!m_subscribers.isEmpty(), return);
  SS_ASSERT_LOG(m_epoch > 0);

  const qint64 now = m_clock.elapsed();
  if (std::none_of(m_subscribers.cbegin(), m_subscribers.cend(), [this, now](const Subscriber& s) {
        return due(s, now);
      }))
    return;

  ensureStructure();
  if (!collectValues()) {
    onStructureChanged();
    return;
  }

  ++m_seq;
  m_lastSnapshot = now;
  m_tickLines.clear();

  for (auto& subscriber : m_subscribers) {
    if (!due(subscriber, now))
      continue;

    if (subscriber.lastEpoch != m_epoch)
      publishStructure(subscriber);

    subscriber.lastSnapshot = now;
    Q_EMIT payloadReady(
      subscriber.socket, subscriber.sessionId, snapshotLine(subscriber.precision));
  }
}

/**
 * @brief Emits a heartbeat when the capture has produced nothing for a second, so a viewer can
 *        tell an idle remote from a dead link instead of calling both stale.
 */
void API::MirrorPublisher::onHeartbeatTimeout()
{
  SS_ASSERT(!m_subscribers.isEmpty(), return);
  SS_ASSERT_LOG(m_heartbeat.isActive());

  if (m_clock.elapsed() - m_lastSnapshot < Mirror::kHeartbeatIntervalMs)
    return;

  const auto line = Mirror::encodeLine(Mirror::encodeHeartbeat(m_epoch, m_seq));
  for (const auto& subscriber : std::as_const(m_subscribers))
    Q_EMIT payloadReady(subscriber.socket, subscriber.sessionId, line);
}
