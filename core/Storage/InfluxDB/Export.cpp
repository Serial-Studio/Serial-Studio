/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "InfluxDB/Export.h"

#  include <chrono>
#  include <QNetworkAccessManager>
#  include <QNetworkReply>
#  include <QNetworkRequest>
#  include <QSslError>
#  include <QUrlQuery>
#  include <utility>

#  include "Core/Bus/MessageBus.h"
#  include "Core/Bus/Messages.h"
#  include "Core/Licensing/CommercialToken.h"
#  include "Core/SerialStudio.h"
#  include "Core/Services.h"
#  include "Core/SSAssert.h"
#  include "Core/TimerEvents.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/PipelineModules.h"
#  include "DataModel/ProjectModel.h"
#  include "Replay/PlayerState.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// A hung endpoint must not pin the single in-flight slot forever
static constexpr int kInfluxTransferTimeoutMs = 30000;

// Cap on the HTTP error body echoed into the failure message, so a verbose 400/413 stays bounded
static constexpr qsizetype kInfluxMaxErrorBodyBytes = 1024;

// Default measurement when the project names none
static const QString kInfluxDefaultMeasurement = QStringLiteral("serial_studio");

// Project-file keys of the sink configuration; the token is deliberately absent
static constexpr QLatin1StringView kInfluxKeyEnabled{"enabled"};
static constexpr QLatin1StringView kInfluxKeyUrl{"url"};
static constexpr QLatin1StringView kInfluxKeyOrg{"org"};
static constexpr QLatin1StringView kInfluxKeyBucket{"bucket"};
static constexpr QLatin1StringView kInfluxKeyMeasurement{"measurement"};

// Credential-vault scope; the token is stored as the password of the endpoint's host:port
static const QString kInfluxVaultScope = QStringLiteral("influxdb");
static const QString kInfluxVaultUser  = QStringLiteral("token");

//--------------------------------------------------------------------------------------------------
// ExportWorker implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker; the network manager is built on the worker thread by bootstrap().
 */
InfluxDB::ExportWorker::ExportWorker(moodycamel::ReaderWriterQueue<DataModel::DataBlockPtr>* queue,
                                     std::atomic<bool>* enabled,
                                     std::atomic<size_t>* queueSize,
                                     std::atomic<quint64>* pointsWritten,
                                     std::atomic<quint64>* pointsDropped,
                                     std::atomic<quint64>* fieldsSkipped,
                                     std::atomic<quint64>* httpErrors)
  : DataModel::FrameConsumerWorker<DataModel::DataBlockPtr>(queue, enabled, queueSize)
  , m_manager(nullptr)
  , m_inFlightPoints(0)
  , m_epochOffsetNs(0)
  , m_healthy(false)
  , m_pointsWritten(pointsWritten)
  , m_pointsDropped(pointsDropped)
  , m_fieldsSkipped(fieldsSkipped)
  , m_httpErrors(httpErrors)
{
  SS_ASSERT_LOG(pointsWritten != nullptr);
  SS_ASSERT_LOG(pointsDropped != nullptr);
  SS_ASSERT_LOG(fieldsSkipped != nullptr);
  SS_ASSERT_LOG(httpErrors != nullptr);
}

/**
 * @brief Destroys the worker, abandoning whatever request is still in flight.
 */
InfluxDB::ExportWorker::~ExportWorker()
{
  closeResources();
}

/**
 * @brief Worker-thread bootstrap: builds the network manager and anchors the wall clock. Both
 *        belong here rather than in the constructor -- the manager because its replies must be
 *        served on the thread that drives them, the anchor because it must be taken once.
 */
void InfluxDB::ExportWorker::bootstrap()
{
  if (m_manager)
    return;

  m_manager = new QNetworkAccessManager(this);
  m_manager->setTransferTimeout(kInfluxTransferTimeoutMs);

  m_batch.reserve(kDefaultBatchBytes);
  m_postBuffer.reserve(kDefaultBatchBytes);

  sampleEpochOffset();
}

/**
 * @brief Re-reads the distance between the steady clock the driver stamped with and the wall clock
 *        the wire carries. Sampled once at bootstrap and again whenever the sink re-opens: a
 *        machine that steps its clock (NTP, a laptop waking in another timezone) would otherwise
 *        keep writing every later point at the offset that was true when the app started.
 */
void InfluxDB::ExportWorker::sampleEpochOffset()
{
  const auto wall   = std::chrono::system_clock::now().time_since_epoch();
  const auto steady = std::chrono::steady_clock::now().time_since_epoch();
  m_epochOffsetNs   = std::chrono::duration_cast<std::chrono::nanoseconds>(wall).count()
                  - std::chrono::duration_cast<std::chrono::nanoseconds>(steady).count();
}

/**
 * @brief Adopts a fresh endpoint configuration; a changed endpoint invalidates the health state.
 */
void InfluxDB::ExportWorker::applyConfig(const InfluxDB::SinkConfig& config)
{
  const bool endpointMoved =
    config.url != m_config.url || config.org != m_config.org || config.bucket != m_config.bucket;

  m_config = config;
  rebuildWriteCache();
  if (!endpointMoved)
    return;

  sampleEpochOffset();

  const bool wasOpen = m_healthy;
  m_healthy          = false;
  if (wasOpen)
    Q_EMIT resourceOpenChanged();
}

/**
 * @brief Recomputes the write URL and the authorization header from the current configuration.
 *        Both change only here, so caching them keeps the per-flush path from rebuilding a URL and
 *        re-encoding the token on every request.
 */
void InfluxDB::ExportWorker::rebuildWriteCache()
{
  m_writeUrl   = writeUrl();
  m_authHeader = QByteArrayLiteral("Token ") + m_config.token.toUtf8();
}

/**
 * @brief Adopts @p sourceId's published structure so field keys can carry dataset titles: a block
 *        carries dataset identities and values, never names.
 */
void InfluxDB::ExportWorker::setTemplateFrame(int sourceId, const DataModel::Frame& frame)
{
  RenderTemplate& rt = m_templates[sourceId];
  DataModel::bind_frame_template(rt.tpl, frame);
  buildRenderFields(rt);
}

/**
 * @brief Escapes each dataset's field key once, qualifying a duplicate (or empty) title with the
 *        dataset's uniqueId so two datasets can never collapse onto one field key and silently
 *        overwrite each other. The per-dataset type latch is reset so the next block re-decides it.
 */
void InfluxDB::ExportWorker::buildRenderFields(RenderTemplate& tpl)
{
  tpl.fields.clear();

  QHash<QString, int> titleCounts;
  for (const auto& group : tpl.tpl.frame.groups)
    for (const auto& dataset : group.datasets)
      ++titleCounts[dataset.title];

  for (const auto& group : tpl.tpl.frame.groups)
    for (const auto& dataset : group.datasets) {
      QString key = dataset.title;
      if (key.isEmpty() || titleCounts.value(key) > 1)
        key = (key.isEmpty() ? QStringLiteral("field") : key) + QStringLiteral("_")
            + QString::number(dataset.uniqueId);

      RenderField field;
      appendEscaped(field.key, key, kKeySpecials);
      tpl.fields.insert(dataset.uniqueId, field);
    }
}

/**
 * @brief Reports whether the last write to the configured endpoint succeeded.
 */
bool InfluxDB::ExportWorker::isResourceOpen() const
{
  return m_healthy;
}

/**
 * @brief Cancels the in-flight request and discards the pending batch. The finished handler is
 *        disconnected first so a cancelled write cannot report a verdict into a closed session.
 */
void InfluxDB::ExportWorker::closeResources()
{
  if (!m_reply.isNull()) {
    disconnect(m_reply, &QNetworkReply::finished, this, &InfluxDB::ExportWorker::onReplyFinished);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply.clear();
  }

  dropBatch();
  m_inFlightPoints = 0;

  const bool wasOpen = m_healthy;
  m_healthy          = false;
  if (wasOpen)
    Q_EMIT resourceOpenChanged();
}

/**
 * @brief Renders one drained batch of blocks and posts what fits, one request at a time.
 */
void InfluxDB::ExportWorker::processItems(const std::vector<DataModel::DataBlockPtr>& items)
{
  if (!configured())
    return;

  if (!m_reply.isNull()) {
    quint64 outstanding = 0;
    for (const auto& block : items)
      if (block && block->samples > 0)
        outstanding += static_cast<quint64>(block->samples);

    m_pointsDropped->fetch_add(outstanding, std::memory_order_relaxed);
    return;
  }

  for (const auto& block : items) {
    if (!block || block->samples <= 0)
      continue;

    renderBlock(*block);
  }

  flushBatch();
}

/**
 * @brief Renders every sample of @p block as one point, keeping the block's own timebase.
 */
void InfluxDB::ExportWorker::renderBlock(const DataModel::DataBlock& block)
{
  const auto it = m_templates.find(block.sourceId);
  if (it == m_templates.end())
    return;

  QByteArray head;
  appendEscaped(head, m_config.measurement, kMeasurementSpecials);
  head.append(QByteArrayLiteral(",source="));
  head.append(QByteArray::number(block.sourceId));

  for (qsizetype i = 0; i < block.samples; ++i) {
    renderSample(block, it->second, head, i);
    if (m_batch.full())
      flushBatch();
  }

  const auto skipped = static_cast<quint64>(m_batch.skippedFields());
  m_fieldsSkipped->store(skipped, std::memory_order_relaxed);
}

/**
 * @brief Renders sample @p index onto the pre-escaped @p head, one field per dataset. A dataset
 *        the published structure does not know is skipped; its line-protocol field type is latched
 *        on first use so a later sample of the opposite kind cannot contradict InfluxDB's schema.
 */
void InfluxDB::ExportWorker::renderSample(const DataModel::DataBlock& block,
                                          RenderTemplate& tpl,
                                          const QByteArray& head,
                                          qsizetype index)
{
  SS_ASSERT(index >= 0 && index < block.samples, return);

  m_batch.beginPointRaw(head);

  const auto slot = static_cast<std::size_t>(index);
  for (const auto& column : block.columns) {
    const auto field = tpl.fields.find(column.uniqueId);
    if (field == tpl.fields.end())
      continue;

    RenderField& info = field.value();
    if (!info.typeKnown) {
      info.isString  = column.hasText;
      info.typeKnown = true;
    }

    if (!info.isString)
      (void)m_batch.addFieldFloatRaw(info.key, column.values[slot]);
    else if (column.hasText)
      (void)m_batch.addFieldStringRaw(info.key, column.text[slot]);
  }

  (void)m_batch.endPoint(epochNs(block, index));
}

/**
 * @brief Posts the accumulated batch, or drops it when a request is still outstanding. Dropping
 *        is the bounded-buffer contract: queueing behind a slow server would grow latency without
 *        bound and eventually stall the consumer thread the pipeline hands blocks to.
 */
void InfluxDB::ExportWorker::flushBatch()
{
  if (m_batch.isEmpty())
    return;

  if (!configured() || !m_manager) {
    dropBatch();
    return;
  }

  if (!m_reply.isNull()) {
    dropBatch();
    return;
  }

  QNetworkRequest request(m_writeUrl);
  request.setTransferTimeout(kInfluxTransferTimeoutMs);
  request.setMaximumRedirectsAllowed(0);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::ManualRedirectPolicy);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("text/plain; charset=utf-8"));
  request.setRawHeader(QByteArrayLiteral("Authorization"), m_authHeader);

  m_sslFailure.clear();
  m_inFlightPoints = m_batch.points();
  m_batch.takePayload(m_postBuffer);
  m_reply = m_manager->post(request, m_postBuffer);

  if (m_reply.isNull()) {
    m_inFlightPoints = 0;
    noteHttpFailure(tr("The write request could not be sent"));
    return;
  }

  connect(m_reply, &QNetworkReply::finished, this, &InfluxDB::ExportWorker::onReplyFinished);
  connect(m_reply, &QNetworkReply::sslErrors, this, &InfluxDB::ExportWorker::onSslErrors);
}

/**
 * @brief Accounts the pending batch as dropped and empties it.
 */
void InfluxDB::ExportWorker::dropBatch()
{
  m_pointsDropped->fetch_add(static_cast<quint64>(m_batch.points()), std::memory_order_relaxed);
  m_batch.clear();
}

/**
 * @brief Consumes one write verdict: a 2xx credits the points, anything else counts an error and
 *        marks the sink closed so the UI stops claiming a live endpoint.
 */
void InfluxDB::ExportWorker::onReplyFinished()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr || reply != m_reply)
    return;

  m_reply.clear();
  reply->deleteLater();

  const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const bool ok    = reply->error() == QNetworkReply::NoError && status >= 200 && status < 300;
  const auto sent  = m_inFlightPoints;
  m_inFlightPoints = 0;

  if (!ok) {
    m_pointsDropped->fetch_add(static_cast<quint64>(sent), std::memory_order_relaxed);
    noteHttpFailure(failureMessage(*reply, status));
    return;
  }

  m_pointsWritten->fetch_add(static_cast<quint64>(sent), std::memory_order_relaxed);
  if (m_healthy)
    return;

  sampleEpochOffset();
  m_healthy = true;
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Why a write failed, in the words the operator can act on. A TLS refusal is recorded by
 *        the sslErrors handler and reported HERE, once: the handler used to count a failure of its
 *        own and the finished handler counted a second one, so one bad certificate read as two
 *        errors on a diagnostics pane that is meant to count attempts.
 */
QString InfluxDB::ExportWorker::failureMessage(QNetworkReply& reply, int status)
{
  if (!m_sslFailure.isEmpty())
    return tr("TLS verification failed: %1").arg(std::exchange(m_sslFailure, QString()));

  QString message = status > 0 ? tr("Server answered HTTP %1").arg(status) : reply.errorString();
  const QByteArray body = reply.readAll().left(kInfluxMaxErrorBodyBytes).trimmed();
  if (!body.isEmpty())
    message += QStringLiteral(": ") + QString::fromUtf8(body);

  return message;
}

/**
 * @brief Records why the peer was refused, without trusting it: ignoreSslErrors() is deliberately
 *        never called, so the request fails closed and its finished handler reports this reason
 *        instead of an opaque transport error. Counting happens THERE, once per failed reply.
 */
void InfluxDB::ExportWorker::onSslErrors(const QList<QSslError>& errors)
{
  QString reason;
  for (const QSslError& error : errors) {
    if (!reason.isEmpty())
      reason += QStringLiteral("; ");

    reason += error.errorString();
  }

  m_sslFailure = reason;
}

/**
 * @brief Counts one transport or status failure and reports its message to the controller.
 */
void InfluxDB::ExportWorker::noteHttpFailure(const QString& message)
{
  m_httpErrors->fetch_add(1, std::memory_order_relaxed);
  Q_EMIT errorOccurred(message);

  if (!m_healthy)
    return;

  m_healthy = false;
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Returns whether the configuration names a usable write endpoint.
 */
bool InfluxDB::ExportWorker::configured() const
{
  return m_config.enabled && !m_config.url.isEmpty() && !m_config.org.isEmpty()
      && !m_config.bucket.isEmpty() && !m_config.token.isEmpty();
}

/**
 * @brief Builds the v2 write URL for the configured organization and bucket, asking the server
 *        for nanosecond precision so the block's own timestamps survive the round trip.
 */
QUrl InfluxDB::ExportWorker::writeUrl() const
{
  QString base = m_config.url.trimmed();
  while (base.endsWith(QLatin1Char('/')))
    base.chop(1);

  QUrl url(base + QStringLiteral("/api/v2/write"));

  QUrlQuery query;
  query.addQueryItem(QStringLiteral("org"), m_config.org);
  query.addQueryItem(QStringLiteral("bucket"), m_config.bucket);
  query.addQueryItem(QStringLiteral("precision"), QStringLiteral("ns"));
  url.setQuery(query);

  return url;
}

/**
 * @brief Maps sample @p index onto wall-clock nanoseconds. The offset between the steady clock
 *        the driver stamped with and the wall clock is taken once at bootstrap, so the sample
 *        spacing the source produced is preserved exactly and nothing is re-stamped here.
 */
qint64 InfluxDB::ExportWorker::epochNs(const DataModel::DataBlock& block, qsizetype index) const
{
  const auto steadyNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(block.t0.time_since_epoch()).count();

  return steadyNs + DataModel::sample_offset_ns(block, index) + m_epochOffsetNs;
}

//--------------------------------------------------------------------------------------------------
// Export singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the sink controller and arms the late-activation guard, which re-derives the
 *        enabled flag from the project on every entitlement transition. It runs both ways: a
 *        revoked licence stops writing without a project reload, and a trial token that lands
 *        after the project loaded switches a sink the project asked for back on.
 */
InfluxDB::Export::Export()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      DataModel::FrameConsumerConfig{8192, 1024, 1000})
  , m_inApply(false)
  , m_exportRequested(false)
  , m_savingToProjectModel(false)
  , m_measurement(kInfluxDefaultMeasurement)
  , m_vault(kInfluxVaultScope)
  , m_isOpen(false)
  , m_exportEnabled(false)
  , m_pointsWritten(0)
  , m_pointsDropped(0)
  , m_fieldsSkipped(0)
  , m_httpErrors(0)
  , m_lastPointsWrittenSeen(0)
  , m_lastPointsDroppedSeen(0)
  , m_lastHttpErrorsSeen(0)
  , m_bus(nullptr)
{
  connect(
    this, &InfluxDB::Export::enabledChanged, this, &DataModel::IBlockSink::sinkActivityChanged);
  initializeWorker();

  auto* worker = static_cast<ExportWorker*>(m_worker);
  connect(worker,
          &DataModel::FrameConsumerWorkerBase::resourceOpenChanged,
          this,
          &Export::onWorkerOpenChanged);
  connect(worker, &ExportWorker::errorOccurred, this, &Export::onWorkerError);
  QMetaObject::invokeMethod(worker, &ExportWorker::bootstrap, Qt::QueuedConnection);

  m_licenseWatch = Core::services().bus.subscribe<Core::Bus::LicenseStateChanged>(
    this, [this](const std::shared_ptr<const Core::Bus::LicenseStateChanged>&) {
      setExportEnabled(m_exportRequested);
    });
}

/**
 * @brief Default destructor; the FrameConsumer base stops the worker thread.
 */
InfluxDB::Export::~Export() = default;

/**
 * @brief Returns the singleton instance.
 */
InfluxDB::Export& InfluxDB::Export::instance()
{
  static Export singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the last write to the endpoint succeeded.
 */
bool InfluxDB::Export::isOpen() const
{
  return m_isOpen.load(std::memory_order_relaxed);
}

/**
 * @brief Returns whether a token is stored for the current endpoint; the token itself never
 *        leaves this object, so the UI and the API can only ask whether one exists.
 */
bool InfluxDB::Export::hasToken() const
{
  return !m_token.isEmpty();
}

/**
 * @brief Returns whether the sink is enabled for the current project.
 */
bool InfluxDB::Export::exportEnabled() const
{
  return m_exportEnabled.load(std::memory_order_relaxed);
}

/**
 * @brief The publisher's verdict for this sink: the InfluxDB sink consumes blocks while enabled.
 */
bool InfluxDB::Export::sinkActive() const noexcept
{
  return exportEnabled();
}

/**
 * @brief Returns the configured server URL.
 */
QString InfluxDB::Export::url() const
{
  return m_url;
}

/**
 * @brief Returns the configured organization.
 */
QString InfluxDB::Export::organization() const
{
  return m_organization;
}

/**
 * @brief Returns the configured bucket.
 */
QString InfluxDB::Export::bucket() const
{
  return m_bucket;
}

/**
 * @brief Returns the measurement every point is written under.
 */
QString InfluxDB::Export::measurement() const
{
  return m_measurement;
}

/**
 * @brief Returns the most recent write failure message, or an empty string.
 */
QString InfluxDB::Export::lastError() const
{
  return m_lastError;
}

/**
 * @brief Returns how many points the server has accepted this session.
 */
quint64 InfluxDB::Export::pointsWritten() const
{
  return m_pointsWritten.load(std::memory_order_relaxed);
}

/**
 * @brief Returns how many points were dropped: batches refused by backpressure or by the server.
 */
quint64 InfluxDB::Export::pointsDropped() const
{
  return m_pointsDropped.load(std::memory_order_relaxed);
}

/**
 * @brief Returns how many field values were unrepresentable in line protocol (NaN, infinities).
 */
quint64 InfluxDB::Export::fieldsSkipped() const
{
  return m_fieldsSkipped.load(std::memory_order_relaxed);
}

/**
 * @brief Returns how many write requests failed at transport or status level.
 */
quint64 InfluxDB::Export::httpErrors() const
{
  return m_httpErrors.load(std::memory_order_relaxed);
}

/**
 * @brief Serializes the configuration for the project file. The token is never part of it.
 */
QJsonObject InfluxDB::Export::toJson() const
{
  QJsonObject json;
  json.insert(kInfluxKeyEnabled, exportEnabled());
  json.insert(kInfluxKeyUrl, m_url);
  json.insert(kInfluxKeyOrg, m_organization);
  json.insert(kInfluxKeyBucket, m_bucket);
  json.insert(kInfluxKeyMeasurement, m_measurement);
  return json;
}

//--------------------------------------------------------------------------------------------------
// External wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts the root-owned message bus this module publishes on and subscribes to.
 */
void InfluxDB::Export::attachMessageBus(Core::Bus::MessageBus& bus)
{
  SS_ASSERT(m_bus == nullptr, return);
  m_bus = &bus;
}

/**
 * @brief Wires the project round-trip, the published-structure feed the field names come from,
 *        and the 1 Hz statistics pull.
 */
void InfluxDB::Export::setupExternalConnections()
{
  auto* projectModel = &DataModel::pipelineModules().projectModel;
  connect(projectModel, &DataModel::ProjectModel::influxSinkChanged, this, [this, projectModel] {
    if (m_savingToProjectModel)
      return;

    applyProjectConfig(projectModel->influxSink());
  });

  connect(this, &Export::configurationChanged, this, [this, projectModel] {
    if (m_inApply)
      return;

    m_savingToProjectModel = true;
    projectModel->setInfluxSink(toJson());
    m_savingToProjectModel = false;
  });

  auto* worker = static_cast<ExportWorker*>(m_worker);
  SS_ASSERT(worker != nullptr, return);
  connect(&DataModel::pipelineModules().frameBuilder,
          &DataModel::FrameBuilder::structurePublished,
          worker,
          &ExportWorker::setTemplateFrame,
          Qt::QueuedConnection);

  connect(
    &Core::services().timerEvents, &Misc::TimerEvents::timeout1Hz, this, &Export::sampleStats);

  applyProjectConfig(projectModel->influxSink());
}

/**
 * @brief Adopts a project's sink configuration wholesale, then hands it to the worker once.
 */
void InfluxDB::Export::applyProjectConfig(const QJsonObject& config)
{
  m_inApply = true;

  m_url          = config.value(kInfluxKeyUrl).toString();
  m_organization = config.value(kInfluxKeyOrg).toString();
  m_bucket       = config.value(kInfluxKeyBucket).toString();
  setMeasurement(config.value(kInfluxKeyMeasurement).toString());
  reloadTokenFromVault();

  setExportEnabled(config.value(kInfluxKeyEnabled).toBool(false));

  m_inApply = false;

  Q_EMIT configurationChanged();
  syncToWorker();
}

/**
 * @brief Resets the sink to its default state (project close / new document).
 */
void InfluxDB::Export::resetProjectConfig()
{
  applyProjectConfig(QJsonObject{});
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables the sink. Licensing is re-checked here, not cached at load time, so
 *        a project that ships with the sink on cannot turn it on in an unlicensed build. The
 *        request is recorded before the licence AND so the entitlement hook can replay it: a
 *        trial or activation arriving after the project loaded then honours what was asked for.
 */
void InfluxDB::Export::setExportEnabled(const bool enabled)
{
  m_exportRequested = enabled;

  const bool allow = enabled && licenseValid();
  if (m_exportEnabled.load(std::memory_order_relaxed) == allow)
    return;

  m_exportEnabled.store(allow, std::memory_order_relaxed);
  setConsumerEnabled(allow);

  if (!allow)
    closeWorkerResources();

  Q_EMIT enabledChanged();
  markConfigChanged();
}

/**
 * @brief Sets the server URL, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com.
 */
void InfluxDB::Export::setUrl(const QString& url)
{
  if (m_url == url)
    return;

  if (!url.trimmed().isEmpty() && !urlSchemeAllowed(url)) {
    m_lastError =
      tr("Refused an insecure InfluxDB URL: use https, or http only for a loopback host");
    Q_EMIT statsChanged();
    return;
  }

  m_url = url;
  reloadTokenFromVault();
  markConfigChanged();
}

/**
 * @brief Sets the InfluxDB organization the bucket belongs to.
 */
void InfluxDB::Export::setOrganization(const QString& organization)
{
  if (m_organization == organization)
    return;

  m_organization = organization;
  markConfigChanged();
}

/**
 * @brief Sets the destination bucket.
 */
void InfluxDB::Export::setBucket(const QString& bucket)
{
  if (m_bucket == bucket)
    return;

  m_bucket = bucket;
  markConfigChanged();
}

/**
 * @brief Sets the measurement name. An empty name, or one starting with '#' (a line-protocol
 *        comment marker the server would reject), falls back to the default.
 */
void InfluxDB::Export::setMeasurement(const QString& measurement)
{
  QString resolved = measurement.trimmed();
  if (resolved.isEmpty() || resolved.startsWith(QLatin1Char('#')))
    resolved = kInfluxDefaultMeasurement;

  if (m_measurement == resolved)
    return;

  m_measurement = resolved;
  markConfigChanged();
}

/**
 * @brief Stores the API token in the credential vault, keyed by the endpoint. Write-only by
 *        design: nothing reads it back out of this object, so it cannot leak through the project
 *        file, the API surface or the UI.
 */
void InfluxDB::Export::setToken(const QString& token)
{
  if (m_token == token)
    return;

  m_token = token;
  m_vault.setCredentials(vaultHost(), vaultPort(), kInfluxVaultUser, token);

  Q_EMIT configurationChanged();
  syncToWorker();
}

//--------------------------------------------------------------------------------------------------
// Hotpath data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues one published block. The single producer for this SPSC queue is the pipeline
 *        thread, for both lanes (spec 0055 D8); a full queue drops the block, by design.
 */
void InfluxDB::Export::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!block || !exportEnabled() || SerialStudio::isAnyPlayerOpen())
    return;

  enqueueData(block);
}

//--------------------------------------------------------------------------------------------------
// Worker plumbing
//--------------------------------------------------------------------------------------------------

/**
 * @brief FrameConsumer factory method; creates the worker for the threading wrapper.
 */
DataModel::FrameConsumerWorkerBase* InfluxDB::Export::createWorker()
{
  return new ExportWorker(&m_pendingQueue,
                          &m_consumerEnabled,
                          &m_queueSize,
                          &m_pointsWritten,
                          &m_pointsDropped,
                          &m_fieldsSkipped,
                          &m_httpErrors);
}

/**
 * @brief Mirrors the worker's endpoint health onto the controller.
 */
void InfluxDB::Export::onWorkerOpenChanged()
{
  auto* worker = static_cast<ExportWorker*>(m_worker);
  SS_ASSERT(worker != nullptr, return);

  const bool state = worker->isResourceOpen();
  if (m_isOpen.exchange(state, std::memory_order_relaxed) == state)
    return;

  Q_EMIT openChanged();
}

/**
 * @brief Retains the worker's most recent failure message for the UI and the status verb.
 */
void InfluxDB::Export::onWorkerError(const QString& message)
{
  if (m_lastError == message)
    return;

  m_lastError = message;
  Q_EMIT statsChanged();
}

/**
 * @brief 1 Hz statistics pull (specs 0033/0035): the worker only increments plain counters, and
 *        the UI-facing signal is emitted here when one of them actually moved.
 */
void InfluxDB::Export::sampleStats()
{
  const quint64 written = pointsWritten();
  const quint64 dropped = pointsDropped();
  const quint64 errors  = httpErrors();

  if (written == m_lastPointsWrittenSeen && dropped == m_lastPointsDroppedSeen
      && errors == m_lastHttpErrorsSeen)
    return;

  m_lastPointsWrittenSeen = written;
  m_lastPointsDroppedSeen = dropped;
  m_lastHttpErrorsSeen    = errors;
  Q_EMIT statsChanged();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether this build carries a valid commercial licence right now.
 */
bool InfluxDB::Export::licenseValid() const
{
  const auto& token = Licensing::CommercialToken::current();
  return token.isValid() && SS_LICENSE_GUARD();
}

/**
 * @brief Whether @p url is safe to carry a bearer token: https anywhere, or http only to a
 *        loopback host. A cleartext token to a remote host is refused so it cannot ride the wire.
 */
bool InfluxDB::Export::urlSchemeAllowed(const QString& url)
{
  const QUrl parsed(url.trimmed());
  const QString scheme = parsed.scheme().toLower();
  if (scheme == QStringLiteral("https"))
    return true;

  if (scheme != QStringLiteral("http"))
    return false;

  const QString host = parsed.host().toLower();
  return host == QStringLiteral("127.0.0.1") || host == QStringLiteral("localhost")
      || host == QStringLiteral("::1");
}

/**
 * @brief Snapshots the configuration the worker thread needs, token included.
 */
InfluxDB::SinkConfig InfluxDB::Export::snapshotConfig() const
{
  SinkConfig config;
  config.enabled     = exportEnabled();
  config.url         = m_url;
  config.org         = m_organization;
  config.bucket      = m_bucket;
  config.measurement = m_measurement;
  config.token       = m_token;
  return config;
}

/**
 * @brief Returns the vault key host for the current endpoint.
 */
QString InfluxDB::Export::vaultHost() const
{
  const QUrl parsed(m_url);
  const QString host = parsed.host();
  return host.isEmpty() ? m_url.trimmed() : host;
}

/**
 * @brief Returns the vault key port for the current endpoint, defaulting to the InfluxDB port.
 */
quint16 InfluxDB::Export::vaultPort() const
{
  const QUrl parsed(m_url);
  const int port = parsed.port();
  return port > 0 ? static_cast<quint16>(port) : quint16(8086);
}

/**
 * @brief Loads the token stored for the current endpoint, if any.
 */
void InfluxDB::Export::reloadTokenFromVault()
{
  m_token = m_vault.credentials(vaultHost(), vaultPort()).password;
}

/**
 * @brief Announces a configuration change and pushes it to the worker, unless a wholesale apply
 *        is running -- that one notifies and syncs once at the end instead of per field.
 */
void InfluxDB::Export::markConfigChanged()
{
  if (m_inApply)
    return;

  Q_EMIT configurationChanged();
  syncToWorker();
}

/**
 * @brief Hands the current configuration to the worker thread.
 */
void InfluxDB::Export::syncToWorker()
{
  if (!m_worker)
    return;

  QMetaObject::invokeMethod(
    m_worker, "applyConfig", Qt::QueuedConnection, Q_ARG(InfluxDB::SinkConfig, snapshotConfig()));
}

#endif  // BUILD_COMMERCIAL
