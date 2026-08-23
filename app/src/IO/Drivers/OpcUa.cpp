/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "IO/Drivers/OpcUa.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QOpcUaAuthenticationInformation>
#include <QOpcUaConnectionSettings>
#include <QOpcUaLocalizedText>
#include <QOpcUaMonitoringParameters>
#include <QOpcUaReadItem>
#include <QOpcUaUserTokenPolicy>
#include <QSet>
#include <QUrl>

#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/OpcUaTagModel.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

Q_LOGGING_CATEGORY(lcOpcUa, "serialstudio.io.opcua")

static constexpr int kOpcUaDialDeadlineMs    = 15000;
static constexpr int kMinIntervalMs          = 10;
static constexpr int kMaxIntervalMs          = 60000;
static constexpr int kOpcUaDefaultIntervalMs = 100;
static constexpr qint64 kNsPerMs             = 1000000LL;
static constexpr qint64 kMaxClockSkewMs      = 5000;
static constexpr int kDefaultPort            = 4840;
static constexpr int kDefaultReadLimit       = 200;
static constexpr int kRequestTimeoutMs       = 10000;
static constexpr int kSessionTimeoutMs       = 60000;
static constexpr int kChannelLifetimeMs      = 600000;
static constexpr int kWatchdogMs             = 1000;
static constexpr int kSilenceFactor          = 6;
static constexpr int kMinSilenceMs           = 3000;
static constexpr const char* kBackendName    = "open62541";
static constexpr const char* kPolicyNone     = "http://opcfoundation.org/UA/SecurityPolicy#None";

//--------------------------------------------------------------------------------------------------
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the driver, restores persisted settings and wires the configuration signals.
 *        The open62541 backend narrates every channel event through qt.opcua.*; that goes to the
 *        user's console, so it is filtered here (setFilterRules replaces the rule set, hence the
 *        font and canbus rules from main.cpp / CANBus.cpp ride along).
 */
IO::Drivers::OpcUa::OpcUa()
  : m_connecting(false)
  , m_discovering(false)
  , m_browsing(false)
  , m_pollMode(false)
  , m_persistent(true)
  , m_readInFlight(false)
  , m_subscribing(false)
  , m_hasDeferred(false)
  , m_pendingDial(PendingDial::None)
  , m_clockValid(false)
  , m_authMode(0)
  , m_endpointIndex(-1)
  , m_publishingInterval(kOpcUaDefaultIntervalMs)
  , m_pendingMonitors(0)
  , m_failedMonitors(0)
  , m_revisedInterval(0)
  , m_frameCursor(0)
  , m_readLimit(kDefaultReadLimit)
  , m_valuesReceived(0)
  , m_badStatusCount(0)
  , m_unstampedCount(0)
  , m_framesPublished(0)
  , m_linkDrops(0)
  , m_skippedPolls(0)
  , m_endpointUrl(QStringLiteral("opc.tcp://127.0.0.1:4840"))
  , m_dialTimer(new QTimer(this))
  , m_watchdog(new QTimer(this))
  , m_pollTimer(new QTimer(this))
  , m_frameTimer(new QTimer(this))
  , m_client(nullptr)
  , m_browseClient(nullptr)
  , m_discoveryClient(nullptr)
  , m_tagModel(nullptr)
  , m_frameBytes(0)
  , m_lastStampNs(0)
  , m_lastNotifyNs(0)
  , m_serverOffsetMs(0)
  , m_clockOffsetNs(0)
  , m_vault(QStringLiteral("opcua"))
{
  loadSettings();

  m_dialTimer->setSingleShot(true);
  connect(m_dialTimer, &QTimer::timeout, this, &IO::Drivers::OpcUa::onDialTimeout);
  connect(m_pollTimer, &QTimer::timeout, this, &IO::Drivers::OpcUa::onPollTick);
  connect(m_frameTimer, &QTimer::timeout, this, &IO::Drivers::OpcUa::onFrameTick);
  connect(m_watchdog, &QTimer::timeout, this, &IO::Drivers::OpcUa::onWatchdogTick);

  static constexpr void (OpcUa::* kConfigSignals[])() = {&OpcUa::endpointUrlChanged,
                                                         &OpcUa::endpointIndexChanged,
                                                         &OpcUa::endpointsChanged,
                                                         &OpcUa::authModeChanged,
                                                         &OpcUa::usernameChanged,
                                                         &OpcUa::passwordChanged,
                                                         &OpcUa::publishingIntervalChanged,
                                                         &OpcUa::tagsChanged};
  for (const auto signal : kConfigSignals)
    connect(this, signal, this, &IO::Drivers::OpcUa::configurationChanged);

  QLoggingCategory::setFilterRules(QStringLiteral("*font*=false\n"
                                                  "qt.canbus*=false\n"
                                                  "qt.opcua*=false"));

  Q_EMIT configurationChanged();
}

/**
 * @brief Wires the translator so the authentication labels refresh on a language change; runs
 *        from the composition root once every core module exists.
 */
void IO::Drivers::OpcUa::setupExternalConnections()
{
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::Drivers::OpcUa::languageChanged);
}

/**
 * @brief Tears down every client and timer. Retired clients and nodes are normally freed by
 *        deleteLater; here they die NOW, because a deferred delete would run after the driver
 *        is gone and reach into the backend through a freed parent.
 */
IO::Drivers::OpcUa::~OpcUa()
{
  doClose();
  teardownClient(m_browseClient);
  teardownClient(m_discoveryClient);
  delete m_tagModel;
  m_tagModel = nullptr;

  const auto nodes = findChildren<QOpcUaNode*>(QString(), Qt::FindDirectChildrenOnly);
  for (auto* node : nodes)
    delete node;

  const auto clients = findChildren<QOpcUaClient*>(QString(), Qt::FindDirectChildrenOnly);
  for (auto* client : clients)
    delete client;
}

/**
 * @brief Restores the persisted endpoint, authentication and tag list.
 */
void IO::Drivers::OpcUa::loadSettings()
{
  m_endpointUrl = m_settings.value("OpcUaDriver/endpointUrl", m_endpointUrl).toString();
  m_authMode    = m_settings.value("OpcUaDriver/authMode", 0).toInt();
  m_username    = m_settings.value("OpcUaDriver/username", QString()).toString();
  m_publishingInterval =
    m_settings.value("OpcUaDriver/publishingInterval", kOpcUaDefaultIntervalMs).toInt();
  m_publishingInterval = qBound(kMinIntervalMs, m_publishingInterval, kMaxIntervalMs);

  const QUrl url(m_endpointUrl);
  if (url.isValid() && !url.host().isEmpty())
    m_password =
      m_vault.credentials(url.host(), static_cast<quint16>(url.port(kDefaultPort))).password;

  const auto doc =
    QJsonDocument::fromJson(m_settings.value("OpcUaDriver/tags", QByteArray("[]")).toByteArray());
  if (doc.isArray())
    setTags(doc.array());
}

/**
 * @brief Persists the tag list as compact JSON.
 */
void IO::Drivers::OpcUa::saveTags()
{
  if (!m_persistent)
    return;

  m_settings.setValue("OpcUaDriver/tags", QJsonDocument(tagsJson()).toJson(QJsonDocument::Compact));
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Only the UI-config instance writes settings and the credential vault; the per-source
 *        live instance is fed by the manager and must never echo state back into QSettings.
 */
void IO::Drivers::OpcUa::setPersistent(const bool persistent) noexcept
{
  if (m_persistent == persistent)
    return;

  m_persistent = persistent;
}

/**
 * @brief Closes the live session and cancels an in-flight dial; a user's disconnect is final.
 */
void IO::Drivers::OpcUa::close()
{
  m_connecting = false;
  doClose();
  applyDeferredTags();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Applies a tag-list edit that arrived while the session held the wire layout, so an API
 *        or picker change made mid-session is not silently lost once the link closes.
 */
void IO::Drivers::OpcUa::applyDeferredTags()
{
  if (!m_hasDeferred)
    return;

  const auto pending = m_deferredTags;
  m_hasDeferred      = false;
  m_deferredTags     = QJsonArray();
  setTags(pending);
}

/**
 * @brief Non-virtual teardown shared by close(), failDial() and the destructor.
 */
void IO::Drivers::OpcUa::doClose()
{
  if (m_pendingDial == PendingDial::Live)
    m_pendingDial = PendingDial::None;

  m_dialTimer->stop();
  m_pollTimer->stop();
  m_frameTimer->stop();

  for (auto* node : m_nodes) {
    if (!node)
      continue;

    disconnect(node, nullptr, this, nullptr);
    node->deleteLater();
  }

  m_nodes.clear();
  m_nodeIndex.clear();
  m_slots.clear();
  m_watchdog->stop();
  m_polledTags.clear();
  m_pendingMonitors = 0;
  m_failedMonitors  = 0;
  m_revisedInterval = 0;
  m_frameCursor     = 0;
  m_readLimit       = kDefaultReadLimit;
  m_readInFlight    = false;
  m_subscribing     = false;
  m_pollMode        = false;
  m_clockValid      = false;
  m_lastNotifyNs    = 0;
  m_serverOffsetMs  = 0;

  teardownClient(m_client);
}

/**
 * @brief Disconnects every signal from a client and retires it; the pointer is nulled so a late
 *        callback can never reach a freed object.
 */
void IO::Drivers::OpcUa::teardownClient(QOpcUaClient*& client)
{
  if (!client)
    return;

  disconnect(client, nullptr, this, nullptr);
  if (client->state() != QOpcUaClient::Disconnected)
    client->disconnectFromEndpoint();

  client->deleteLater();
  client = nullptr;
}

/**
 * @brief Returns true while the session is established.
 */
bool IO::Drivers::OpcUa::isOpen() const noexcept
{
  return m_client && m_client->state() == QOpcUaClient::Connected;
}

/**
 * @brief Returns true while a dial is in flight.
 */
bool IO::Drivers::OpcUa::isConnecting() const noexcept
{
  return m_connecting;
}

/**
 * @brief Tag values arrive through the session, so readable means open.
 */
bool IO::Drivers::OpcUa::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Writing tags is out of scope (spec 0066 non-goal).
 */
bool IO::Drivers::OpcUa::isWritable() const noexcept
{
  return false;
}

/**
 * @brief A selectable None-policy endpoint, credentials when required, and at least one tag.
 */
bool IO::Drivers::OpcUa::configurationOk() const noexcept
{
  if (m_tags.isEmpty() || m_tags.size() > OpcUaWire::kMaxTags)
    return false;

  if (m_authMode == 1 && m_username.isEmpty())
    return false;

  if (m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size())
    return policyIsNone(m_endpoints.at(m_endpointIndex));

  return QUrl(m_endpointUrl).isValid() && !QUrl(m_endpointUrl).host().isEmpty();
}

/**
 * @brief The driver never writes (see isWritable()).
 */
qint64 IO::Drivers::OpcUa::write(const QByteArray& data)
{
  Q_UNUSED(data)
  return 0;
}

/**
 * @brief Starts the async dial: a fresh client and the 15 s last-resort timer. With a discovered
 *        endpoint selected it dials at once; otherwise it discovers first and dials the server's
 *        own None endpoint from onEndpointsFinished() (a synthetic description carries no user
 *        identity tokens and the backend refuses it). One verdict either way.
 */
bool IO::Drivers::OpcUa::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  close();
  if (!configurationOk())
    return false;

  m_client = makeClient();
  if (!m_client) {
    logDriverError(tr("OPC UA Initialization Failed"),
                   tr("The %1 backend is not available in this build.").arg(kBackendName));
    return false;
  }

  connect(m_client, &QOpcUaClient::stateChanged, this, &IO::Drivers::OpcUa::onStateChanged);
  connect(m_client, &QOpcUaClient::errorChanged, this, &IO::Drivers::OpcUa::onErrorChanged);
  connect(
    m_client, &QOpcUaClient::readNodeAttributesFinished, this, &IO::Drivers::OpcUa::onReadFinished);

  applyAuthentication(m_client);
  if (m_authMode == 1)
    qCWarning(lcOpcUa) << "Username/password selected on a None-policy channel: credentials "
                          "travel unencrypted to"
                       << selectedEndpointUrl();

  m_connecting = true;
  m_lastError.clear();
  m_dialTimer->start(kOpcUaDialDeadlineMs);

  if (hasSelectedEndpoint())
    m_client->connectToEndpoint(dialEndpoint());
  else {
    m_pendingDial = PendingDial::Live;
    discoverEndpoints();
  }

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  return m_connecting || isOpen();
}

/**
 * @brief True when a discovered, selectable endpoint row is current.
 */
bool IO::Drivers::OpcUa::hasSelectedEndpoint() const noexcept
{
  return m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size()
      && policyIsNone(m_endpoints.at(m_endpointIndex))
      && endpointAcceptsToken(m_endpoints.at(m_endpointIndex), m_authMode);
}

/**
 * @brief The ONE provider in the process, created on first use and never destroyed.
 *        ~QOpcUaProvider unloads the backend plugin Qt caches process-wide, so a second
 *        instance (the per-source live driver beside the UI-config one) would leave the
 *        survivor's next createClient() calling into a dangling plugin. Leaking one object
 *        also keeps the plugin alive past every client at shutdown.
 */
QOpcUaProvider& IO::Drivers::OpcUa::provider()
{
  static QOpcUaProvider* instance = new QOpcUaProvider;
  return *instance;
}

/**
 * @brief Creates a backend client, or nullptr when the plugin is missing.
 */
QOpcUaClient* IO::Drivers::OpcUa::makeClient()
{
  auto* client = provider().createClient(QString::fromLatin1(kBackendName));
  if (!client)
    return nullptr;

  QOpcUaConnectionSettings settings;
  settings.setConnectTimeout(std::chrono::milliseconds(kOpcUaDialDeadlineMs));
  settings.setRequestTimeout(std::chrono::milliseconds(kRequestTimeoutMs));
  settings.setSessionTimeout(std::chrono::milliseconds(kSessionTimeoutMs));
  settings.setSecureChannelLifeTime(std::chrono::milliseconds(kChannelLifetimeMs));
  client->setConnectionSettings(settings);
  client->setParent(this);
  return client;
}

/**
 * @brief Applies the anonymous or username token to a client.
 */
void IO::Drivers::OpcUa::applyAuthentication(QOpcUaClient* client) const
{
  SS_ASSERT(client != nullptr, return);

  QOpcUaAuthenticationInformation auth;
  if (m_authMode == 1)
    auth.setUsernameAuthentication(m_username, m_password);
  else
    auth.setAnonymousAuthentication();

  client->setAuthenticationInformation(auth);
}

/**
 * @brief The endpoint URL the dial targets (selected endpoint, else the typed one).
 */
QString IO::Drivers::OpcUa::selectedEndpointUrl() const
{
  if (m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size())
    return m_endpoints.at(m_endpointIndex).endpointUrl();

  return m_endpointUrl;
}

/**
 * @brief True for the one policy the shipped backend can open.
 */
bool IO::Drivers::OpcUa::policyIsNone(const QOpcUaEndpointDescription& endpoint)
{
  return endpoint.securityPolicy() == QLatin1String(kPolicyNone)
      && endpoint.securityMode() == QOpcUaEndpointDescription::None;
}

/**
 * @brief True when the endpoint advertises a user token the selected authentication mode can
 *        present; a server offering only Anonymous rejects a username session outright.
 */
bool IO::Drivers::OpcUa::endpointAcceptsToken(const QOpcUaEndpointDescription& endpoint,
                                              const int authMode)
{
  const auto wanted =
    authMode == 1 ? QOpcUaUserTokenPolicy::Username : QOpcUaUserTokenPolicy::Anonymous;

  const auto tokens = endpoint.userIdentityTokens();
  if (tokens.isEmpty())
    return true;

  for (const auto& token : tokens)
    if (token.tokenType() == wanted)
      return true;

  return false;
}

/**
 * @brief The description actually dialed: the discovered endpoint carrying the URL the user
 *        typed. Servers routinely advertise their own hostname (S7, Kepware, B&R) which does not
 *        resolve from the engineering laptop, so host and port are substituted while the policy
 *        and the user-token list the backend validates are preserved.
 */
QOpcUaEndpointDescription IO::Drivers::OpcUa::dialEndpoint() const
{
  QOpcUaEndpointDescription endpoint;
  if (!hasSelectedEndpoint()) {
    endpoint.setEndpointUrl(m_endpointUrl);
    endpoint.setSecurityPolicy(QString::fromLatin1(kPolicyNone));
    endpoint.setSecurityMode(QOpcUaEndpointDescription::None);
    return endpoint;
  }

  endpoint = m_endpoints.at(m_endpointIndex);

  const QUrl typed(m_endpointUrl);
  QUrl advertised(endpoint.endpointUrl());
  if (!typed.isValid() || typed.host().isEmpty() || !advertised.isValid())
    return endpoint;

  advertised.setHost(typed.host());
  advertised.setPort(typed.port(advertised.port(kDefaultPort)));
  endpoint.setEndpointUrl(advertised.toString());
  return endpoint;
}

/**
 * @brief Ends a failed dial exactly once: tears the client down, logs, reports the verdict and
 *        republishes the state. A user abort already cleared m_connecting, which makes it silent.
 */
void IO::Drivers::OpcUa::failDial(const QString& reason)
{
  if (!m_connecting)
    return;

  m_connecting = false;
  m_lastError  = reason;
  doClose();

  logDriverError(tr("OPC UA Connection Failed"),
                 reason.isEmpty() ? tr("Unable to connect to \"%1\".").arg(selectedEndpointUrl())
                                  : tr("\"%1\": %2").arg(selectedEndpointUrl(), reason));

  reportOpenFinished(false, reason);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief An established session dropped: report once, then let the manager close the device on
 *        the next event-loop turn so nothing tears the driver down from inside its own handler.
 */
void IO::Drivers::OpcUa::onLinkDropped(const QString& reason)
{
  m_lastError = reason;
  ++m_linkDrops;
  logDriverError(tr("OPC UA Connection Lost"), reason);

  static auto& manager = ConnectionManager::instance();
  QMetaObject::invokeMethod(this, [this] { manager.disconnectDevice(this); }, Qt::QueuedConnection);

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Last-resort dial deadline; the backend normally reports sooner.
 */
void IO::Drivers::OpcUa::onDialTimeout()
{
  failDial(tr("Timed out after %1 s").arg(kOpcUaDialDeadlineMs / 1000));
}

//--------------------------------------------------------------------------------------------------
// Client signal handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Connected settles the dial and starts the subscription; Disconnected during a dial is a
 *        failure, after one a link drop.
 */
void IO::Drivers::OpcUa::onStateChanged(QOpcUaClient::ClientState state)
{
  if (!m_client)
    return;

  if (state == QOpcUaClient::Connected) {
    m_dialTimer->stop();
    m_connecting = false;
    reportOpenFinished(true);
    subscribeAll();
    Q_EMIT statusChanged();
    Q_EMIT configurationChanged();
    return;
  }

  if (state != QOpcUaClient::Disconnected)
    return;

  if (m_connecting)
    failDial(m_lastError.isEmpty() ? tr("The server closed the connection") : m_lastError);
  else if (!m_nodes.isEmpty())
    onLinkDropped(m_lastError.isEmpty() ? tr("The server closed the session") : m_lastError);
}

/**
 * @brief Maps backend errors to a reason string; the state transition carries the verdict.
 */
void IO::Drivers::OpcUa::onErrorChanged(QOpcUaClient::ClientError error)
{
  switch (error) {
    case QOpcUaClient::NoError:
      return;
    case QOpcUaClient::InvalidUrl:
      m_lastError = tr("Invalid endpoint URL");
      break;
    case QOpcUaClient::AccessDenied:
      m_lastError = tr("Access denied (bad credentials or anonymous login refused)");
      break;
    case QOpcUaClient::ConnectionError:
      m_lastError = tr("Connection error (server unreachable or refused)");
      break;
    case QOpcUaClient::UnsupportedAuthenticationInformation:
    case QOpcUaClient::InvalidAuthenticationInformation:
      m_lastError = tr("The server does not accept this authentication mode");
      break;
    case QOpcUaClient::InvalidEndpointDescription:
      m_lastError = tr("The endpoint description was rejected (unsupported security policy)");
      break;
    case QOpcUaClient::NoMatchingUserIdentityTokenFound:
      m_lastError = tr("The endpoint offers no matching user identity token");
      break;
    case QOpcUaClient::UnsupportedSecurityPolicy:
      m_lastError = tr("The security policy is not supported by this build");
      break;
    case QOpcUaClient::InvalidPki:
      m_lastError = tr("The certificate store is invalid or unreadable");
      break;
    case QOpcUaClient::CertificateUntrusted:
      m_lastError = tr("The server certificate is not trusted");
      break;
    default:
      m_lastError = tr("Unexpected backend error");
      break;
  }

  if (m_connecting && m_client && m_client->state() == QOpcUaClient::Disconnected)
    failDial(m_lastError);
}

//--------------------------------------------------------------------------------------------------
// Endpoint discovery
//--------------------------------------------------------------------------------------------------

/**
 * @brief Asks the server at the typed URL for its endpoints; results land in endpointList.
 */
void IO::Drivers::OpcUa::discoverEndpoints()
{
  if (m_discovering)
    return;

  const QUrl url(m_endpointUrl);
  if (!url.isValid() || url.host().isEmpty()) {
    m_lastError = tr("\"%1\" is not a valid endpoint URL").arg(m_endpointUrl);
    logDriverError(tr("OPC UA Discovery"), m_lastError);
    continuePendingDial();
    return;
  }

  teardownClient(m_discoveryClient);
  m_discoveryClient = makeClient();
  if (!m_discoveryClient) {
    m_lastError = tr("The %1 backend is not available in this build").arg(kBackendName);
    continuePendingDial();
    return;
  }

  connect(m_discoveryClient,
          &QOpcUaClient::endpointsRequestFinished,
          this,
          [this](const QList<QOpcUaEndpointDescription>& endpoints,
                 QOpcUa::UaStatusCode status,
                 const QUrl&) { onEndpointsFinished(endpoints, status); });

  m_discovering = true;
  Q_EMIT discoveringChanged();

  if (!m_discoveryClient->requestEndpoints(url))
    onEndpointsFinished({}, QOpcUa::UaStatusCode::BadInternalError);
}

/**
 * @brief Stores the advertised endpoints, preferring the first None-policy one as the selection.
 */
void IO::Drivers::OpcUa::onEndpointsFinished(const QList<QOpcUaEndpointDescription>& endpoints,
                                             QOpcUa::UaStatusCode status)
{
  m_discovering = false;
  teardownClient(m_discoveryClient);

  if (status != QOpcUa::UaStatusCode::Good) {
    m_endpoints.clear();
    m_endpointIndex = -1;
    m_lastError     = tr("Discovery failed: %1").arg(QOpcUa::statusToString(status));
    logDriverError(tr("OPC UA Discovery Failed"), tr("\"%1\": %2").arg(m_endpointUrl, m_lastError));
  } else {
    const auto previous = m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size()
                          ? m_endpoints.at(m_endpointIndex).endpointUrl()
                          : QString();

    m_endpoints     = endpoints;
    m_endpointIndex = -1;

    for (int i = 0; i < m_endpoints.size(); ++i) {
      const auto& candidate = m_endpoints.at(i);
      if (!policyIsNone(candidate) || !endpointAcceptsToken(candidate, m_authMode))
        continue;

      if (m_endpointIndex < 0)
        m_endpointIndex = i;

      if (!previous.isEmpty() && candidate.endpointUrl() == previous) {
        m_endpointIndex = i;
        break;
      }
    }

    if (m_endpointIndex < 0) {
      m_lastError =
        tr("No None-policy endpoint; secure channels are not supported in this version");
      logDriverError(tr("OPC UA Discovery"), tr("\"%1\": %2").arg(m_endpointUrl, m_lastError));
    }
  }

  Q_EMIT discoveringChanged();
  Q_EMIT statusChanged();
  Q_EMIT endpointsChanged();
  Q_EMIT endpointIndexChanged();
  continuePendingDial();
}

/**
 * @brief Dials the endpoint discovery just selected for a live or browse session that asked for
 *        it, or fails that session when the server offers nothing this build can open.
 */
void IO::Drivers::OpcUa::continuePendingDial()
{
  const auto pending = m_pendingDial;
  m_pendingDial      = PendingDial::None;

  if (pending == PendingDial::Live) {
    if (m_client && hasSelectedEndpoint())
      m_client->connectToEndpoint(dialEndpoint());
    else
      failDial(m_lastError.isEmpty() ? tr("Endpoint discovery failed") : m_lastError);

    return;
  }

  if (pending != PendingDial::Browse)
    return;

  if (m_browseClient && hasSelectedEndpoint()) {
    m_browseClient->connectToEndpoint(dialEndpoint());
    return;
  }

  Q_EMIT browseFailed(m_lastError.isEmpty() ? tr("Endpoint discovery failed") : m_lastError);
  cancelBrowse();
}

//--------------------------------------------------------------------------------------------------
// Subscription, poll fallback and the publish tick
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the wire layout, reserves the frame once and asks for a monitored item per tag;
 *        the all-refused verdict arrives through onMonitoringEnabled().
 */
void IO::Drivers::OpcUa::subscribeAll()
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT(!m_tags.isEmpty(), return);

  const auto steady     = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 steadyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(steady).count();
  m_clockOffsetNs       = steadyNs - QDateTime::currentMSecsSinceEpoch() * kNsPerMs;
  m_clockValid          = true;
  m_lastStampNs         = 0;
  m_lastNotifyNs        = steadyNs;
  m_serverOffsetMs      = 0;

  reserveFrame();
  readServerLimits();

  m_pendingMonitors = m_tags.size();
  m_failedMonitors  = 0;
  m_revisedInterval = m_publishingInterval;
  m_pollMode        = false;
  m_subscribing     = true;
  m_polledTags.clear();

  QOpcUaMonitoringParameters params(static_cast<double>(m_publishingInterval));
  params.setSamplingInterval(static_cast<double>(m_publishingInterval));
  params.setQueueSize(1);
  params.setDiscardOldest(true);
  for (int i = 0; i < m_tags.size(); ++i) {
    auto* node = m_client->node(m_tags.at(i).nodeId);
    if (node)
      node->setParent(this);

    m_nodes.append(node);
    m_nodeIndex.insert(m_tags.at(i).nodeId, i);
    if (!node) {
      onMonitoringEnabled(i, QOpcUa::UaStatusCode::BadNodeIdInvalid);
      continue;
    }

    connect(node, &QOpcUaNode::valueAttributeUpdated, this, [this, i](const QVariant& value) {
      onValueUpdated(i, value);
    });
    connect(node,
            &QOpcUaNode::enableMonitoringFinished,
            this,
            [this, i](QOpcUa::NodeAttribute, QOpcUa::UaStatusCode status) {
              onMonitoringEnabled(i, status);
            });

    if (!node->enableMonitoring(QOpcUa::NodeAttribute::Value, params))
      onMonitoringEnabled(i, QOpcUa::UaStatusCode::BadInternalError);
  }

  m_frameTimer->start(m_publishingInterval);
  m_watchdog->start(kWatchdogMs);
}

/**
 * @brief Reads the server's MaxNodesPerRead once so a batched poll is never rejected wholesale
 *        with Bad_TooManyOperations; the default stands when the server does not publish it.
 */
void IO::Drivers::OpcUa::readServerLimits()
{
  SS_ASSERT(m_client != nullptr, return);

  auto* node = m_client->node(QStringLiteral("ns=0;i=11705"));
  if (!node)
    return;

  node->setParent(this);
  connect(node, &QOpcUaNode::attributeRead, this, [this, node](QOpcUa::NodeAttributes) {
    const int limit = node->attribute(QOpcUa::NodeAttribute::Value).toInt();
    if (limit > 0)
      m_readLimit = qBound(1, limit, kDefaultReadLimit * 10);

    node->deleteLater();
  });

  if (!node->readAttributes(QOpcUa::NodeAttribute::Value))
    node->deleteLater();
}

/**
 * @brief Sizes the slot cache from the tag layout and reserves the worst-case frame once.
 */
void IO::Drivers::OpcUa::reserveFrame()
{
  m_firstIndex.clear();
  m_slots.clear();

  m_slotCount.clear();

  qsizetype bytes = OpcUaWire::kHeaderBytes;
  for (const auto& tag : m_tags) {
    const auto type = wireTypeFor(tag);
    const int count = qMax(1, tag.arrayLen);
    m_firstIndex.append(m_slots.size());
    m_slotCount.append(count);
    bytes += static_cast<qsizetype>(count) * OpcUaWire::maxEntryBytes(type);
    for (int i = 0; i < count; ++i) {
      Slot slot;
      slot.type = type;
      m_slots.append(slot);
    }
  }

  SS_ASSERT_LOG(m_slots.size() <= OpcUaWire::kMaxTags);
  m_frameBytes = qMin<qsizetype>(bytes, OpcUaWire::kMaxFrameBytes);
  m_frame      = QByteArray();
  m_frame.reserve(m_frameBytes);
}

/**
 * @brief Counts monitored-item verdicts; when every tag was refused the session falls back to
 *        timed reads, and a partial refusal is logged but the subscription stays.
 */
void IO::Drivers::OpcUa::onMonitoringEnabled(int tag, QOpcUa::UaStatusCode status)
{
  SS_ASSERT(tag >= 0 && tag < m_tags.size(), return);
  if (m_pendingMonitors <= 0)
    return;

  --m_pendingMonitors;
  if (status != QOpcUa::UaStatusCode::Good) {
    ++m_failedMonitors;
    m_polledTags.append(tag);
    logDriverError(tr("OPC UA Monitored Item Refused"),
                   tr("\"%1\": %2").arg(m_tags.at(tag).nodeId, QOpcUa::statusToString(status)));
  } else if (m_revisedInterval == m_publishingInterval) {
    adoptRevisedInterval(tag);
  }

  if (m_pendingMonitors > 0)
    return;

  m_subscribing = false;
  if (m_failedMonitors == m_nodes.size()) {
    enterPollMode(tr("the server refused every monitored item"));
    return;
  }

  if (!m_polledTags.isEmpty()) {
    m_pollTimer->start(m_revisedInterval);
    onPollTick();
  }

  Q_EMIT statusChanged();
}

/**
 * @brief Adopts the interval the server revised the subscription to; a PLC that floors publishing
 *        at 100 ms must not leave the pane claiming the rate the user asked for.
 */
void IO::Drivers::OpcUa::adoptRevisedInterval(int tag)
{
  SS_ASSERT(tag >= 0 && tag < m_nodes.size(), return);
  auto* node = m_nodes.at(tag);
  SS_ASSERT(node != nullptr, return);

  const auto status = node->monitoringStatus(QOpcUa::NodeAttribute::Value);
  const int revised = static_cast<int>(status.publishingInterval());
  if (revised <= 0 || revised == m_revisedInterval)
    return;

  m_revisedInterval = qBound(kMinIntervalMs, revised, kMaxIntervalMs);
  if (m_frameTimer->isActive())
    m_frameTimer->start(m_revisedInterval);

  if (m_pollTimer->isActive())
    m_pollTimer->start(m_revisedInterval);
}

/**
 * @brief Switches to timed reads at the publishing interval.
 */
void IO::Drivers::OpcUa::enterPollMode(const QString& reason)
{
  if (m_pollMode)
    return;

  m_pollMode = true;
  m_polledTags.clear();
  for (int i = 0; i < m_tags.size(); ++i)
    m_polledTags.append(i);

  logDriverError(tr("OPC UA Subscription Unavailable"),
                 tr("Falling back to polling: %1.").arg(reason));

  m_pollTimer->start(m_revisedInterval > 0 ? m_revisedInterval : m_publishingInterval);
  onPollTick();
  Q_EMIT statusChanged();
}

/**
 * @brief Nothing has arrived for several publishing periods while the session is still up: a
 *        server that silently dropped the subscription (project reload, PLC stop) looks healthy
 *        otherwise, so the session falls back to polling rather than freezing the dashboard.
 */
void IO::Drivers::OpcUa::onWatchdogTick()
{
  if (!isOpen() || m_pollMode || m_subscribing || m_lastNotifyNs == 0)
    return;

  const auto now        = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 nowNs    = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
  const qint64 silentMs = (nowNs - m_lastNotifyNs) / kNsPerMs;
  const qint64 budgetMs =
    qMax<qint64>(kMinSilenceMs, static_cast<qint64>(m_revisedInterval) * kSilenceFactor);

  if (silentMs < budgetMs)
    return;

  enterPollMode(tr("no notification for %1 s").arg(silentMs / 1000));
}

/**
 * @brief Issues one batched read of every tag's Value attribute.
 */
void IO::Drivers::OpcUa::onPollTick()
{
  if (!isOpen() || m_polledTags.isEmpty())
    return;

  if (m_readInFlight) {
    ++m_skippedPolls;
    return;
  }

  issueRead(m_polledTags);
}

/**
 * @brief Issues the batched Value read for a tag subset, chunked to the server's MaxNodesPerRead.
 *        Exactly one read is outstanding at a time: queueing them behind a slow PLC grows latency
 *        without bound and eventually times the session out.
 */
void IO::Drivers::OpcUa::issueRead(const QList<int>& tags)
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT(!tags.isEmpty(), return);

  QList<QOpcUaReadItem> items;
  items.reserve(qMin(tags.size(), m_readLimit));
  for (const int tag : tags) {
    if (tag < 0 || tag >= m_tags.size())
      continue;

    items.append(QOpcUaReadItem(m_tags.at(tag).nodeId, QOpcUa::NodeAttribute::Value));
    if (items.size() < m_readLimit)
      continue;

    m_readInFlight = m_client->readNodeAttributes(items);
    items.clear();
    return;
  }

  if (!items.isEmpty())
    m_readInFlight = m_client->readNodeAttributes(items);
}

/**
 * @brief Routes batched read results into the slot cache by node id.
 */
void IO::Drivers::OpcUa::onReadFinished(const QList<QOpcUaReadResult>& results,
                                        QOpcUa::UaStatusCode status)
{
  m_readInFlight = false;
  if (status != QOpcUa::UaStatusCode::Good) {
    logDriverError(tr("OPC UA Read Failed"), QOpcUa::statusToString(status));
    return;
  }

  for (const auto& result : results) {
    const int tag = m_nodeIndex.value(result.nodeId(), -1);
    if (tag < 0)
      continue;

    storeValue(tag, result.value(), result.statusCode(), result.sourceTimestamp());
  }
}

/**
 * @brief Monitored-item update: pulls status and source time off the node and caches the value.
 */
void IO::Drivers::OpcUa::onValueUpdated(int tag, const QVariant& value)
{
  SS_ASSERT(tag >= 0 && tag < m_nodes.size(), return);
  auto* node = m_nodes.at(tag);
  SS_ASSERT(node != nullptr, return);

  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  m_lastNotifyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

  if (!m_serverOffsetMs) {
    const auto serverTs = node->serverTimestamp(QOpcUa::NodeAttribute::Value);
    if (serverTs.isValid())
      m_serverOffsetMs = serverTs.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
  }

  storeValue(
    tag, value, node->valueAttributeError(), node->sourceTimestamp(QOpcUa::NodeAttribute::Value));
}

/**
 * @brief Unwraps the one OPC UA-specific value type the wire vocabulary cannot see: LocalizedText
 *        becomes its text here so OpcUaWire.h stays Qt Core-only for the ctest tier.
 */
QVariant IO::Drivers::OpcUa::unwrapValue(const QVariant& value)
{
  if (value.canConvert<QOpcUaLocalizedText>())
    return value.value<QOpcUaLocalizedText>().text();

  return value;
}

/**
 * @brief Writes a value into its slot(s); a bad status keeps the last good value and counts.
 *        Arrays fan out element-wise, extra elements are dropped, missing ones left latched.
 */
void IO::Drivers::OpcUa::storeValue(int tag,
                                    const QVariant& value,
                                    QOpcUa::UaStatusCode status,
                                    const QDateTime& sourceTs)
{
  SS_ASSERT(tag >= 0 && tag < m_firstIndex.size(), return);
  ++m_valuesReceived;

  const auto severity = static_cast<quint32>(status) & 0xC0000000u;
  if (severity >= 0x80000000u) {
    ++m_badStatusCount;
    markBad(tag);
    return;
  }

  const int first = m_firstIndex.at(tag);
  const int count = m_slotCount.at(tag);
  if (value.typeId() == QMetaType::QVariantList) {
    const auto list = value.toList();
    for (int i = 0; i < count && i < list.size() && first + i < m_slots.size(); ++i) {
      auto& slot    = m_slots[first + i];
      slot.value    = unwrapValue(list.at(i));
      slot.sourceTs = sourceTs;
      slot.dirty    = true;
      slot.bad      = false;
    }

    return;
  }

  SS_ASSERT(first < m_slots.size(), return);
  auto& slot    = m_slots[first];
  slot.value    = unwrapValue(value);
  slot.sourceTs = sourceTs;
  slot.dirty    = true;
  slot.bad      = false;
}

/**
 * @brief Flags a tag's slots as stale after a Bad status; the dashboard keeps the last good value
 *        and the quality is reported through the diagnostics snapshot instead of vanishing.
 */
void IO::Drivers::OpcUa::markBad(int tag)
{
  SS_ASSERT(tag >= 0 && tag < m_firstIndex.size(), return);

  const int first = m_firstIndex.at(tag);
  const int count = m_slotCount.at(tag);
  for (int i = 0; i < count && first + i < m_slots.size(); ++i)
    m_slots[first + i].bad = true;
}

/**
 * @brief The node ids whose newest value carried a Bad status (R11 diagnostics).
 */
QStringList IO::Drivers::OpcUa::badTags() const
{
  QStringList out;
  for (int tag = 0; tag < m_firstIndex.size() && tag < m_tags.size(); ++tag) {
    const int first = m_firstIndex.at(tag);
    if (first < m_slots.size() && m_slots.at(first).bad)
      out.append(m_tags.at(tag).nodeId);
  }

  return out;
}

/**
 * @brief Maps a server source timestamp onto the steady clock through the per-connect offset;
 *        skew is measured against the server-to-local offset sampled at connect, so an un-NTP'd
 *        PLC is followed rather than rejected; a missing or wildly skewed stamp falls back to now
 *        and counts as unstamped. The result never goes backwards (previous stamp plus 1 ns).
 */
IO::CapturedData::SteadyTimePoint IO::Drivers::OpcUa::toSteady(const QDateTime& sourceTs)
{
  const auto now = CapturedData::SteadyClock::now();
  const qint64 nowNs =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

  qint64 stamp = nowNs;
  if (!m_clockValid || !sourceTs.isValid())
    ++m_unstampedCount;
  else {
    const qint64 skewMs =
      sourceTs.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch() - m_serverOffsetMs;
    if (skewMs > kMaxClockSkewMs || skewMs < -kMaxClockSkewMs)
      ++m_unstampedCount;
    else
      stamp = (sourceTs.toMSecsSinceEpoch() - m_serverOffsetMs) * kNsPerMs + m_clockOffsetNs;
  }

  stamp         = qMax(stamp, m_lastStampNs + 1);
  m_lastStampNs = stamp;
  return CapturedData::SteadyTimePoint(std::chrono::nanoseconds(stamp));
}

/**
 * @brief Publishing tick: encodes every dirty slot into one delta frame stamped with the earliest
 *        source time it carries. No dirty slot, no frame; slots that do not fit under the decoder
 *        cap stay dirty for the next tick. The buffer is handed to the pipeline and re-reserved,
 *        one allocation per tick at command rate.
 */
void IO::Drivers::OpcUa::onFrameTick()
{
  if (!isOpen())
    return;

  using namespace OpcUaWire;
  beginFrame(m_frame);

  QDateTime earliest;
  const int slotCount = m_slots.size();
  if (m_frameCursor >= slotCount)
    m_frameCursor = 0;

  for (int step = 0; step < slotCount; ++step) {
    const int index = (m_frameCursor + step) % slotCount;
    auto& slot      = m_slots[index];
    if (!slot.dirty)
      continue;

    if (m_frame.size() + maxEntryBytes(slot.type) > kMaxFrameBytes) {
      m_frameCursor = index;
      break;
    }

    if (!slot.warned && !valueFitsType(slot.value, slot.type)) {
      slot.warned = true;
      logDriverError(tr("OPC UA Type Mismatch"),
                     tr("Channel %1 is declared %2 but the server sends %3; the value is coerced.")
                       .arg(QString::number(index),
                            codeFromType(slot.type),
                            QString::fromLatin1(slot.value.typeName())));
    }

    appendEntry(m_frame, index, slot.type, slot.value);
    slot.dirty = false;
    if (!earliest.isValid() || (slot.sourceTs.isValid() && slot.sourceTs < earliest))
      earliest = slot.sourceTs;
  }

  if (m_frame.size() <= kHeaderBytes)
    return;

  ++m_framesPublished;
  publishReceivedData(std::move(m_frame), toSteady(earliest));
  m_frame = QByteArray();
  m_frame.reserve(m_frameBytes);
}

/**
 * @brief The wire type a tag encodes as (its declared type; strings for anything unmapped).
 */
IO::Drivers::OpcUaWire::Type IO::Drivers::OpcUa::wireTypeFor(const OpcUaTag& tag) noexcept
{
  if (tag.type == OpcUaWire::Type::Invalid)
    return OpcUaWire::Type::Str;

  return tag.type;
}

//--------------------------------------------------------------------------------------------------
// Browse session and project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The picker's model, created on first use and fed only while a browse session is up.
 */
IO::Drivers::OpcUaTagModel* IO::Drivers::OpcUa::tagModel()
{
  if (!m_tagModel) {
    m_tagModel = new OpcUaTagModel(this);
    connect(
      m_tagModel, &OpcUaTagModel::browseError, this, [this](const QString& id, const QString& r) {
        Q_EMIT browseFailed(tr("Browse of %1 failed: %2").arg(id, r));
      });
  }

  return m_tagModel;
}

/**
 * @brief QML-facing view of the tag model.
 */
QObject* IO::Drivers::OpcUa::tagModelObject()
{
  return tagModel();
}

/**
 * @brief Opens a browse-only session on a second client so the picker can walk the address
 *        space without touching the live link; stopBrowse() ends it.
 */
void IO::Drivers::OpcUa::startBrowse()
{
  if (m_browsing)
    return;

  if (!QUrl(m_endpointUrl).isValid() || QUrl(m_endpointUrl).host().isEmpty()) {
    Q_EMIT browseFailed(tr("\"%1\" is not a valid endpoint URL.").arg(m_endpointUrl));
    return;
  }

  teardownClient(m_browseClient);
  m_browseClient = makeClient();
  if (!m_browseClient) {
    Q_EMIT browseFailed(tr("The %1 backend is not available in this build.").arg(kBackendName));
    return;
  }

  connect(
    m_browseClient, &QOpcUaClient::stateChanged, this, &IO::Drivers::OpcUa::onBrowseClientState);
  applyAuthentication(m_browseClient);

  tagModel()->preselect(m_tags);
  m_browsing = true;
  Q_EMIT browsingChanged();

  if (hasSelectedEndpoint())
    m_browseClient->connectToEndpoint(dialEndpoint());
  else {
    m_pendingDial = PendingDial::Browse;
    discoverEndpoints();
  }
}

/**
 * @brief Commits the picker's selection as the tag list and ends the browse session. Tags the
 *        picker never fetched keep their place: an unexpanded folder is not an unchecked one.
 */
void IO::Drivers::OpcUa::stopBrowse()
{
  if (m_tagModel && m_browseClient && m_browseClient->state() == QOpcUaClient::Connected) {
    QJsonArray array;
    const auto selected = m_tagModel->selectedTags();
    for (const auto& tag : selected)
      array.append(tagToJson(tag));

    for (const auto& tag : m_tags)
      if (!m_tagModel->hasSeen(tag.nodeId))
        array.append(tagToJson(tag));

    setTags(array);
  }

  if (m_tagModel)
    m_tagModel->setClient(nullptr);

  teardownClient(m_browseClient);
  if (!m_browsing)
    return;

  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Ends the browse session WITHOUT committing the picker selection (the dialog's Cancel).
 */
void IO::Drivers::OpcUa::cancelBrowse()
{
  if (m_pendingDial == PendingDial::Browse)
    m_pendingDial = PendingDial::None;

  if (m_tagModel)
    m_tagModel->setClient(nullptr);

  teardownClient(m_browseClient);
  if (!m_browsing)
    return;

  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Lends the connected browse client to the model; a drop or refusal ends the session.
 */
void IO::Drivers::OpcUa::onBrowseClientState(QOpcUaClient::ClientState state)
{
  if (!m_browseClient)
    return;

  if (state == QOpcUaClient::Connected) {
    tagModel()->setClient(m_browseClient);
    tagModel()->fetchMore(QModelIndex());
    return;
  }

  if (state != QOpcUaClient::Disconnected || !m_browsing)
    return;

  m_lastError = tr("Could not open a browse session on %1").arg(selectedEndpointUrl());
  Q_EMIT browseFailed(m_lastError);
  Q_EMIT statusChanged();
  if (m_tagModel)
    m_tagModel->setClient(nullptr);

  teardownClient(m_browseClient);
  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Builds the project and loads it into the editor (no save dialog); the API path.
 *        Returns the model on success so callers hold no singleton of their own.
 */
DataModel::ProjectModel* IO::Drivers::OpcUa::loadGeneratedProject()
{
  if (m_tags.isEmpty())
    return nullptr;

  static auto& pm       = DataModel::ProjectModel::instance();
  static auto& appState = AppState::instance();
  appState.setOperationMode(SerialStudio::ProjectFile);
  if (!pm.loadFromJsonDocument(QJsonDocument(buildProject()), QString())) {
    logDriverError(tr("Failed to load generated project"),
                   tr("The generated project JSON could not be loaded."));
    return nullptr;
  }

  pm.setModified(true);
  return &pm;
}

/**
 * @brief Generates a project from the tag list and opens it in the editor.
 */
void IO::Drivers::OpcUa::generateProject()
{
  if (m_tags.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No tags selected"),
                                    tr("Browse the server and select at least one tag before "
                                       "generating a project."),
                                    QMessageBox::Warning,
                                    tr("OPC UA Project Generator"));
    return;
  }

  auto* pm = loadGeneratedProject();
  if (!pm)
    return;

  const int groupCount = buildProject().value(Keys::Groups).toArray().size();
  const int datasets   = wireSchema().size();
  QObject::connect(
    pm,
    &DataModel::ProjectModel::saveDialogCompleted,
    this,
    [groupCount, datasets](bool accepted) {
      if (!accepted)
        return;

      Misc::Utilities::showMessageBox(
        tr("Successfully generated project with %1 groups and %2 datasets.")
          .arg(groupCount)
          .arg(datasets),
        tr("The project editor is now open for customization."),
        QMessageBox::Information,
        tr("OPC UA Project Generator"));
    },
    Qt::SingleShotConnection);

  (void)pm->saveJsonFile(true);
}

/**
 * @brief One dataset for a tag (or one array element): LED for booleans, plot for numerics.
 */
DataModel::Dataset IO::Drivers::OpcUa::datasetFor(const OpcUaTag& tag, int element, int index)
{
  SS_ASSERT_LOG(index >= 1);
  const auto type = wireTypeFor(tag);

  DataModel::Dataset dataset;
  dataset.index = index;
  dataset.log   = true;
  dataset.units = tag.unit;
  dataset.title = qMax(1, tag.arrayLen) > 1
                  ? QStringLiteral("%1[%2]").arg(tag.name, QString::number(element))
                  : tag.name;

  if (type == OpcUaWire::Type::Bool) {
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.wgtMax  = 1;
  } else if (type != OpcUaWire::Type::Str) {
    dataset.plt = true;
    if (tag.max > tag.min) {
      dataset.wgtMin = tag.min;
      dataset.wgtMax = tag.max;
      dataset.pltMin = tag.min;
      dataset.pltMax = tag.max;
    }
  }

  return dataset;
}

/**
 * @brief One group per parent folder, one dataset per wire index, the opcua native template.
 */
QJsonObject IO::Drivers::OpcUa::buildProject() const
{
  QJsonObject project;
  project[Keys::Title]   = tr("OPC UA Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("OPC UA");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::OpcUa);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("opcua");
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
  int index = 0;
  for (const auto& tag : m_tags) {
    const QString key = tag.path.isEmpty() ? tr("Tags") : tag.path;
    if (!groups.contains(key)) {
      DataModel::Group group;
      group.groupId = order.size();
      group.widget  = QStringLiteral("datagrid");
      group.title   = key.section(QLatin1Char('/'), -1);
      groups.insert(key, group);
      order.append(key);
    }

    auto& group = groups[key];
    for (int i = 0; i < qMax(1, tag.arrayLen); ++i, ++index)
      group.datasets.push_back(datasetFor(tag, i, index + 1));
  }

  QJsonArray groupArray;
  for (const auto& key : order)
    groupArray.append(DataModel::serialize(groups.value(key)));

  project[Keys::Groups] = groupArray;
  return project;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the typed endpoint URL.
 */
QString IO::Drivers::OpcUa::endpointUrl() const
{
  return m_endpointUrl;
}

/**
 * @brief Returns the selected discovered endpoint row, or -1.
 */
int IO::Drivers::OpcUa::endpointIndex() const
{
  return m_endpointIndex;
}

/**
 * @brief "policy / mode / url" rows for the endpoint combo.
 */
QStringList IO::Drivers::OpcUa::endpointList() const
{
  QStringList list;
  for (const auto& endpoint : m_endpoints) {
    const auto policy = endpoint.securityPolicy().section(QLatin1Char('#'), -1);
    const auto mode   = endpoint.securityMode() == QOpcUaEndpointDescription::None ? tr("None")
                      : endpoint.securityMode() == QOpcUaEndpointDescription::Sign
                        ? tr("Sign")
                        : tr("Sign & Encrypt");
    list.append(QStringLiteral("%1 / %2 / %3").arg(policy, mode, endpoint.endpointUrl()));
  }

  return list;
}

/**
 * @brief Structured view of the discovered endpoints for API clients: policy, mode, URL, the
 *        advertised user tokens and whether this build can dial the row.
 */
QJsonArray IO::Drivers::OpcUa::endpointsJson() const
{
  QJsonArray out;
  for (int i = 0; i < m_endpoints.size(); ++i) {
    const auto& endpoint = m_endpoints.at(i);

    QJsonArray tokens;
    for (const auto& token : endpoint.userIdentityTokens())
      tokens.append(static_cast<int>(token.tokenType()));

    out.append(QJsonObject{
      {QStringLiteral("index"), i},
      {QStringLiteral("policy"), endpoint.securityPolicy().section(QLatin1Char('#'), -1)},
      {QStringLiteral("mode"), static_cast<int>(endpoint.securityMode())},
      {QStringLiteral("url"), endpoint.endpointUrl()},
      {QStringLiteral("tokens"), tokens},
      {QStringLiteral("selectable"),
       policyIsNone(endpoint) && endpointAcceptsToken(endpoint, m_authMode)},
    });
  }

  return out;
}

/**
 * @brief True when a tag edit is waiting for the live session to close.
 */
bool IO::Drivers::OpcUa::tagsDeferred() const
{
  return m_hasDeferred;
}

/**
 * @brief Parallel to endpointList(): true where the row can be dialed by this build.
 */
QVariantList IO::Drivers::OpcUa::endpointSelectable() const
{
  QVariantList list;
  for (const auto& endpoint : m_endpoints)
    list.append(policyIsNone(endpoint));

  return list;
}

/**
 * @brief Returns true while an endpoint request is in flight.
 */
bool IO::Drivers::OpcUa::discovering() const
{
  return m_discovering;
}

/**
 * @brief Returns the authentication mode (0 anonymous, 1 username).
 */
int IO::Drivers::OpcUa::authMode() const
{
  return m_authMode;
}

/**
 * @brief Returns the username for mode 1.
 */
QString IO::Drivers::OpcUa::username() const
{
  return m_username;
}

/**
 * @brief Returns the vault-backed password for mode 1.
 */
QString IO::Drivers::OpcUa::password() const
{
  return m_password;
}

/**
 * @brief Returns the publishing interval in milliseconds.
 */
int IO::Drivers::OpcUa::publishingInterval() const
{
  return m_publishingInterval;
}

/**
 * @brief Returns the number of selected tags.
 */
int IO::Drivers::OpcUa::tagCount() const
{
  return m_tags.size();
}

/**
 * @brief One-line session status for the pane.
 */
QString IO::Drivers::OpcUa::statusText() const
{
  if (m_connecting)
    return tr("Connecting to %1").arg(selectedEndpointUrl());

  if (!isOpen())
    return m_lastError.isEmpty() ? tr("Not connected") : m_lastError;

  const int interval = m_revisedInterval > 0 ? m_revisedInterval : m_publishingInterval;
  const double hz    = 1000.0 / qMax(1, interval);

  if (m_subscribing)
    return tr("Subscribing, %1 of %2 tags")
      .arg(m_tags.size() - m_pendingMonitors)
      .arg(m_tags.size());

  if (!m_pollMode && !m_polledTags.isEmpty())
    return tr("Subscribed %1 tags, polling %2 refused, %3 Hz")
      .arg(m_tags.size() - m_polledTags.size())
      .arg(m_polledTags.size())
      .arg(hz, 0, 'f', 1);

  if (m_pollMode)
    return tr("Polling (server refused subscriptions), %1 tags, %2 Hz")
      .arg(m_tags.size())
      .arg(hz, 0, 'f', 1);

  return tr("Subscribed, %1 tags, %2 Hz").arg(m_tags.size()).arg(hz, 0, 'f', 1);
}

/**
 * @brief Returns true once the session fell back to timed reads.
 */
bool IO::Drivers::OpcUa::pollMode() const
{
  return m_pollMode;
}

/**
 * @brief The publishing interval the server revised the subscription to (0 when not subscribed).
 */
int IO::Drivers::OpcUa::revisedInterval() const
{
  return m_revisedInterval;
}

/**
 * @brief Returns true while the browse session is up.
 */
bool IO::Drivers::OpcUa::browsing() const
{
  return m_browsing;
}

/**
 * @brief Returns the translated authentication mode labels.
 */
QStringList IO::Drivers::OpcUa::authModeList() const
{
  return {tr("Anonymous"), tr("Username / Password")};
}

/**
 * @brief Returns the selected tags in wire order.
 */
const QList<IO::Drivers::OpcUaTag>& IO::Drivers::OpcUa::tags() const noexcept
{
  return m_tags;
}

/**
 * @brief The tag list as JSON (conn-settings and API shape).
 */
QJsonArray IO::Drivers::OpcUa::tagsJson() const
{
  QJsonArray array;
  for (const auto& tag : m_tags)
    array.append(tagToJson(tag));

  return array;
}

/**
 * @brief The `opcua` native template schema: one {i, t} entry per wire index.
 */
QJsonArray IO::Drivers::OpcUa::wireSchema() const
{
  QJsonArray schema;
  int index = 0;
  for (const auto& tag : m_tags) {
    const auto code = OpcUaWire::codeFromType(wireTypeFor(tag));
    for (int i = 0; i < qMax(1, tag.arrayLen) && index < OpcUaWire::kMaxTags; ++i, ++index)
      schema.append(QJsonObject{
        { QStringLiteral("i"),      index},
        { QStringLiteral("t"),       code},
        {QStringLiteral("id"), tag.nodeId}
      });
  }

  return schema;
}

/**
 * @brief Pulled diagnostics snapshot (spec 0033: counters, never pushed).
 */
QJsonObject IO::Drivers::OpcUa::statusJson() const
{
  return QJsonObject{
    {      QStringLiteral("connected"),                               isOpen()},
    {     QStringLiteral("connecting"),                           m_connecting},
    {       QStringLiteral("pollMode"),                             m_pollMode},
    {    QStringLiteral("subscribing"),                          m_subscribing},
    {QStringLiteral("revisedInterval"),                      m_revisedInterval},
    {    QStringLiteral("refusedTags"),                    m_polledTags.size()},
    {   QStringLiteral("skippedPolls"),    static_cast<qint64>(m_skippedPolls)},
    {        QStringLiteral("badTags"),  QJsonArray::fromStringList(badTags())},
    {       QStringLiteral("endpoint"),                  selectedEndpointUrl()},
    {       QStringLiteral("tagCount"),                          m_tags.size()},
    { QStringLiteral("valuesReceived"),  static_cast<qint64>(m_valuesReceived)},
    {      QStringLiteral("badStatus"),  static_cast<qint64>(m_badStatusCount)},
    {      QStringLiteral("unstamped"),  static_cast<qint64>(m_unstampedCount)},
    {QStringLiteral("framesPublished"), static_cast<qint64>(m_framesPublished)},
    {     QStringLiteral("reconnects"),       static_cast<qint64>(m_linkDrops)},
    {      QStringLiteral("lastError"),                            m_lastError},
    {     QStringLiteral("statusText"),                           statusText()},
  };
}

/**
 * @brief Human-readable row for the pane's tag summary.
 */
QString IO::Drivers::OpcUa::tagInfo(const int index) const
{
  if (index < 0 || index >= m_tags.size())
    return {};

  const auto& tag = m_tags.at(index);
  return QStringLiteral("%1 (%2) %3")
    .arg(tag.name, OpcUaWire::codeFromType(wireTypeFor(tag)), tag.nodeId);
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Changes the typed URL; the discovered list no longer applies to it.
 */
void IO::Drivers::OpcUa::setEndpointUrl(const QString& url)
{
  const auto trimmed = url.trimmed();
  if (m_endpointUrl == trimmed)
    return;

  m_endpointUrl = trimmed;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/endpointUrl", m_endpointUrl);

  m_endpoints.clear();
  m_endpointIndex = -1;

  const QUrl parsed(m_endpointUrl);
  if (parsed.isValid() && !parsed.host().isEmpty())
    m_password =
      m_vault.credentials(parsed.host(), static_cast<quint16>(parsed.port(kDefaultPort))).password;

  Q_EMIT endpointUrlChanged();
  Q_EMIT endpointsChanged();
  Q_EMIT endpointIndexChanged();
  Q_EMIT passwordChanged();
}

/**
 * @brief Selects a discovered endpoint; non-None rows are refused.
 */
void IO::Drivers::OpcUa::setEndpointIndex(const int index)
{
  if (index == m_endpointIndex)
    return;

  if (index >= 0 && (index >= m_endpoints.size() || !policyIsNone(m_endpoints.at(index))))
    return;

  m_endpointIndex = index;
  Q_EMIT endpointIndexChanged();
}

/**
 * @brief Sets the authentication mode.
 */
void IO::Drivers::OpcUa::setAuthMode(const int mode)
{
  const int clamped = qBound(0, mode, 1);
  if (m_authMode == clamped)
    return;

  m_authMode = clamped;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/authMode", m_authMode);

  Q_EMIT authModeChanged();
}

/**
 * @brief Sets the username for mode 1.
 */
void IO::Drivers::OpcUa::setUsername(const QString& username)
{
  if (m_username == username)
    return;

  m_username = username;
  const QUrl url(m_endpointUrl);
  if (m_persistent) {
    m_settings.setValue("OpcUaDriver/username", m_username);
    if (url.isValid() && !url.host().isEmpty())
      m_vault.setCredentials(
        url.host(), static_cast<quint16>(url.port(kDefaultPort)), m_username, m_password);
  }

  Q_EMIT usernameChanged();
}

/**
 * @brief Stores the password in the encrypted vault keyed by the endpoint host:port.
 */
void IO::Drivers::OpcUa::setPassword(const QString& password)
{
  if (m_password == password)
    return;

  m_password = password;
  const QUrl url(m_endpointUrl);
  if (m_persistent && url.isValid() && !url.host().isEmpty())
    m_vault.setCredentials(
      url.host(), static_cast<quint16>(url.port(kDefaultPort)), m_username, m_password);

  Q_EMIT passwordChanged();
}

/**
 * @brief Sets the publishing interval (ms); a live session re-arms its timers in place.
 */
void IO::Drivers::OpcUa::setPublishingInterval(const int interval)
{
  const int clamped = qBound(kMinIntervalMs, interval, kMaxIntervalMs);
  if (m_publishingInterval == clamped)
    return;

  m_publishingInterval = clamped;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/publishingInterval", m_publishingInterval);

  for (auto* node : m_nodes)
    if (node)
      (void)node->modifyMonitoring(QOpcUa::NodeAttribute::Value,
                                   QOpcUaMonitoringParameters::Parameter::PublishingInterval,
                                   static_cast<double>(m_publishingInterval));

  m_revisedInterval = m_publishingInterval;
  if (m_frameTimer->isActive())
    m_frameTimer->start(m_publishingInterval);

  if (m_pollTimer->isActive())
    m_pollTimer->start(m_publishingInterval);

  Q_EMIT publishingIntervalChanged();
  Q_EMIT statusChanged();
}

/**
 * @brief Replaces the tag list from its JSON shape (bounded by kMaxTags).
 */
void IO::Drivers::OpcUa::setTags(const QJsonArray& tags)
{
  QList<OpcUaTag> list;
  QSet<QString> seen;
  int indices = 0;
  for (const auto& item : tags) {
    const auto tag  = tagFromJson(item.toObject());
    const int count = qMax(1, tag.arrayLen);
    if (tag.nodeId.isEmpty() || seen.contains(tag.nodeId) || count > OpcUaWire::kMaxTags - indices)
      continue;

    seen.insert(tag.nodeId);
    indices += count;
    list.append(tag);
  }

  if (list == m_tags)
    return;

  if (tagsFrozen()) {
    m_deferredTags = tags;
    m_hasDeferred  = true;
    return;
  }

  m_deferredTags = QJsonArray();
  m_hasDeferred  = false;
  m_tags         = list;
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief The wire layout is sized at subscribe time, so the list is immutable while a session
 *        or a dial is live. Silent on purpose: the UI-to-live property echo hits this on every
 *        configuration change.
 */
bool IO::Drivers::OpcUa::tagsFrozen() const
{
  return m_client != nullptr;
}

/**
 * @brief Appends a tag unless already present or over the index budget.
 */
void IO::Drivers::OpcUa::addTag(const OpcUaTag& tag)
{
  if (tag.nodeId.isEmpty() || tagsFrozen())
    return;

  for (const auto& existing : m_tags)
    if (existing.nodeId == tag.nodeId)
      return;

  int indices = 0;
  for (const auto& existing : m_tags)
    indices += qMax(1, existing.arrayLen);

  OpcUaTag bounded = tag;
  bounded.arrayLen = qBound(1, tag.arrayLen, OpcUaWire::kMaxTags);
  if (bounded.arrayLen > OpcUaWire::kMaxTags - indices)
    return;

  m_tags.append(bounded);
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief Removes the tag at the given position.
 */
void IO::Drivers::OpcUa::removeTag(const int index)
{
  if (index < 0 || index >= m_tags.size() || tagsFrozen())
    return;

  m_tags.removeAt(index);
  saveTags();
  Q_EMIT tagsChanged();
}

/**
 * @brief Drops every selected tag.
 */
void IO::Drivers::OpcUa::clearTags()
{
  if (m_tags.isEmpty() || tagsFrozen())
    return;

  m_tags.clear();
  saveTags();
  Q_EMIT tagsChanged();
}

//--------------------------------------------------------------------------------------------------
// Tag JSON shape
//--------------------------------------------------------------------------------------------------

/**
 * @brief {id, name, path, unit, t, n} -> OpcUaTag.
 */
IO::Drivers::OpcUaTag IO::Drivers::OpcUa::tagFromJson(const QJsonObject& obj)
{
  OpcUaTag tag;
  tag.nodeId   = obj.value(QStringLiteral("id")).toString();
  tag.name     = obj.value(QStringLiteral("name")).toString(tag.nodeId);
  tag.path     = obj.value(QStringLiteral("path")).toString();
  tag.unit     = obj.value(QStringLiteral("unit")).toString();
  tag.type     = OpcUaWire::typeFromCode(obj.value(QStringLiteral("t")).toString());
  tag.arrayLen = qBound(1, obj.value(QStringLiteral("n")).toInt(1), OpcUaWire::kMaxTags);
  tag.min      = SerialStudio::toDouble(obj.value(QStringLiteral("min")));
  tag.max      = SerialStudio::toDouble(obj.value(QStringLiteral("max")));
  return tag;
}

/**
 * @brief OpcUaTag -> {id, name, path, unit, t, n}.
 */
QJsonObject IO::Drivers::OpcUa::tagToJson(const OpcUaTag& tag)
{
  return QJsonObject{
    {QStringLiteral("id"), tag.nodeId},
    {QStringLiteral("name"), tag.name},
    {QStringLiteral("path"), tag.path},
    {QStringLiteral("unit"), tag.unit},
    {QStringLiteral("t"), OpcUaWire::codeFromType(wireTypeFor(tag))},
    {QStringLiteral("n"), qMax(1, tag.arrayLen)},
    {QStringLiteral("min"), tag.min},
    {QStringLiteral("max"), tag.max},
  };
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat editable property list; the password is typed Password so projects never store it.
 */
QList<IO::DriverProperty> IO::Drivers::OpcUa::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty url;
  url.key   = QStringLiteral("endpointUrl");
  url.label = tr("Endpoint URL");
  url.type  = IO::DriverProperty::Text;
  url.value = selectedEndpointUrl();
  props.append(url);

  IO::DriverProperty auth;
  auth.key     = QStringLiteral("authMode");
  auth.label   = tr("Authentication");
  auth.type    = IO::DriverProperty::ComboBox;
  auth.value   = m_authMode;
  auth.options = authModeList();
  props.append(auth);

  IO::DriverProperty user;
  user.key   = QStringLiteral("username");
  user.label = tr("Username");
  user.type  = IO::DriverProperty::Text;
  user.value = m_username;
  props.append(user);

  IO::DriverProperty pass;
  pass.key   = QStringLiteral("password");
  pass.label = tr("Password");
  pass.type  = IO::DriverProperty::Password;
  pass.value = m_password;
  props.append(pass);

  IO::DriverProperty interval;
  interval.key   = QStringLiteral("publishingInterval");
  interval.label = tr("Poll Interval (ms)");
  interval.type  = IO::DriverProperty::IntField;
  interval.value = m_publishingInterval;
  interval.min   = kMinIntervalMs;
  interval.max   = kMaxIntervalMs;
  props.append(interval);

  IO::DriverProperty tags;
  tags.key   = QStringLiteral("tags");
  tags.type  = IO::DriverProperty::Text;
  tags.value = QVariant::fromValue(tagsJson());
  props.append(tags);

  return props;
}

/**
 * @brief Applies a single configuration change by key.
 */
void IO::Drivers::OpcUa::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("endpointUrl")) {
    setEndpointUrl(value.toString());
    return;
  }

  if (key == QLatin1String("authMode")) {
    setAuthMode(value.toInt());
    return;
  }

  if (key == QLatin1String("username")) {
    setUsername(value.toString());
    return;
  }

  if (key == QLatin1String("password")) {
    setPassword(value.toString());
    return;
  }

  if (key == QLatin1String("publishingInterval")) {
    setPublishingInterval(value.toInt());
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
