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
#include <QSet>
#include <QUrl>

#include "API/EnumLabels.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/OpcUaSecurity.h"
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
static constexpr int kWatchdogMs             = 1000;
static constexpr int kSilenceFactor          = 6;
static constexpr int kMinSilenceMs           = 3000;
static constexpr const char* kBackendName    = "open62541";
static constexpr const char* kPolicyNone     = "http://opcfoundation.org/UA/SecurityPolicy#None";

/**
 * @brief Every security policy this build can open, weakest first. Basic128Rsa15 and Basic256 are
 *        deprecated by the OPC Foundation (SHA-1 and RSA-1.5); they stay reachable because field
 *        controllers still ship them, but they are labelled and never auto-selected.
 */
static constexpr const char* kPolicyUris[] = {
  "http://opcfoundation.org/UA/SecurityPolicy#None",
  "http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15",
  "http://opcfoundation.org/UA/SecurityPolicy#Basic256",
  "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
  "http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep",
  "http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss",
};

//--------------------------------------------------------------------------------------------------
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the driver, restores persisted settings and wires the configuration signals.
 *        setFilterRules REPLACES the rule set rather than adding to it, so the font and canbus
 *        rules from main.cpp / CANBus.cpp have to ride along here.
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
  , m_valuesReceived(0)
  , m_badStatusCount(0)
  , m_unstampedCount(0)
  , m_framesPublished(0)
  , m_linkDrops(0)
  , m_skippedPolls(0)
  , m_endpointUrl(QStringLiteral("opc.tcp://127.0.0.1:4840"))
  , m_securityMode(static_cast<int>(OpcUaTypes::SecurityMode::None))
  , m_dialTimer(new QTimer(this))
  , m_watchdog(new QTimer(this))
  , m_pollTimer(new QTimer(this))
  , m_frameTimer(new QTimer(this))
  , m_session(nullptr)
  , m_browseSession(nullptr)
  , m_discoverySession(nullptr)
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
                                                         &OpcUa::securityChanged,
                                                         &OpcUa::tagsChanged};
  for (const auto signal : kConfigSignals)
    connect(this, signal, this, &IO::Drivers::OpcUa::configurationChanged);

  QLoggingCategory::setFilterRules(QStringLiteral("*font*=false\n"
                                                  "qt.canbus*=false"));

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
  teardownSession(m_browseSession);
  teardownSession(m_discoverySession);
  delete m_tagModel;
  m_tagModel = nullptr;
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

  m_securityPolicy =
    m_settings.value("OpcUaDriver/securityPolicy", QString::fromLatin1(kPolicyNone)).toString();
  if (!supportedPolicies().contains(m_securityPolicy))
    m_securityPolicy = QString::fromLatin1(kPolicyNone);

  m_securityMode =
    m_settings.value("OpcUaDriver/securityMode", static_cast<int>(OpcUaTypes::SecurityMode::None))
      .toInt();
  m_userCertificatePath = m_settings.value("OpcUaDriver/userCertificate", QString()).toString();
  m_userKeyPath         = m_settings.value("OpcUaDriver/userKey", QString()).toString();

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
 * @brief Points this UI-config instance at the per-source instance that owns the live session.
 *        The pane and the API server both read the UI instance, but only the live one ever dials,
 *        so without this hop every session counter and error they show is a default value.
 */
void IO::Drivers::OpcUa::setSessionPeer(OpcUa* peer)
{
  SS_ASSERT(peer != this, return);
  if (m_sessionPeer == peer)
    return;

  m_sessionPeer = peer;
  if (peer)
    connect(
      peer, &QObject::destroyed, this, &IO::Drivers::OpcUa::statusChanged, Qt::UniqueConnection);

  Q_EMIT statusChanged();
}

/**
 * @brief The live session whose state answers a status query, or nullptr when this instance is
 *        itself the session. Only the persistent instance delegates, so the hop is always one
 *        deep and can never recurse.
 */
const IO::Drivers::OpcUa* IO::Drivers::OpcUa::sessionPeer() const
{
  return m_persistent ? m_sessionPeer.data() : nullptr;
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

  m_nodeIndex.clear();
  m_slots.clear();
  m_watchdog->stop();
  m_polledTags.clear();
  m_pendingMonitors = 0;
  m_failedMonitors  = 0;
  m_revisedInterval = 0;
  m_frameCursor     = 0;
  m_readInFlight    = false;
  m_subscribing     = false;
  m_pollMode        = false;
  m_clockValid      = false;
  m_lastNotifyNs    = 0;
  m_serverOffsetMs  = 0;

  teardownSession(m_session);
}

/**
 * @brief Disconnects every signal from a session and retires it; the pointer is nulled so a late
 *        callback can never reach a freed object. The session's own close() is what stops the
 *        iterate pump before the open62541 client is destroyed.
 */
void IO::Drivers::OpcUa::teardownSession(OpcUaSession*& session)
{
  if (!session)
    return;

  disconnect(session, nullptr, this, nullptr);
  session->close();
  session->deleteLater();
  session = nullptr;
}

/**
 * @brief Returns true while the session is established.
 */
bool IO::Drivers::OpcUa::isOpen() const noexcept
{
  return m_session && m_session->isOpen();
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
    return endpointUsable(m_endpoints.at(m_endpointIndex));

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
 * @brief Starts the async dial: a fresh session and the 15 s last-resort timer. With a discovered
 *        endpoint selected it dials at once; otherwise it discovers first and dials the endpoint
 *        chosen in onEndpointsFinished(), because a synthetic description carries no user
 *        identity tokens and the server refuses it. One verdict either way.
 */
bool IO::Drivers::OpcUa::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)

  close();
  if (!configurationOk()) {
    m_lastError = tr("The connection is not configured: check the endpoint and the tag list");
    Q_EMIT statusChanged();
    return false;
  }

  m_session = makeSession();
  if (!m_session) {
    m_lastError = tr("The %1 stack is not available in this build").arg(kBackendName);
    logDriverError(tr("OPC UA Initialization Failed"), m_lastError);
    Q_EMIT statusChanged();
    return false;
  }

  connect(m_session, &OpcUaSession::connected, this, &IO::Drivers::OpcUa::onSessionConnected);
  connect(m_session, &OpcUaSession::disconnected, this, &IO::Drivers::OpcUa::onSessionDisconnected);
  connect(m_session, &OpcUaSession::connectFailed, this, &IO::Drivers::OpcUa::onConnectFailed);
  connect(m_session, &OpcUaSession::subscribed, this, &IO::Drivers::OpcUa::onSubscribed);
  connect(m_session, &OpcUaSession::valueChanged, this, &IO::Drivers::OpcUa::onValueChanged);
  connect(
    m_session, &OpcUaSession::subscriptionLost, this, &IO::Drivers::OpcUa::onSubscriptionLost);
  connect(m_session, &OpcUaSession::readFinished, this, &IO::Drivers::OpcUa::onReadFinished);

  m_connecting = true;
  m_lastError.clear();
  m_dialTimer->start(kOpcUaDialDeadlineMs);
  warnAboutPlaintextCredentials();
  prepareClientIdentity();

  if (hasSelectedEndpoint())
    startDial();
  else {
    m_pendingDial = PendingDial::Live;
    discoverEndpoints();
  }

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
  return m_connecting || isOpen();
}

/**
 * @brief Hands the chosen endpoint to the session. A false return means the attempt never
 *        started, so the verdict is owed here rather than through the state callback.
 */
void IO::Drivers::OpcUa::startDial()
{
  SS_ASSERT(m_session != nullptr, return);

  const auto endpoint = dialEndpoint();
  if (!m_session->connectToEndpoint(endpoint, identity()))
    failDial(m_lastError.isEmpty() ? tr("The connection attempt could not be started")
                                   : m_lastError);
}

/**
 * @brief Materializes the installation's client certificate before a secure dial needs it, and
 *        republishes it. Generating an RSA-2048 identity is a prime search that can run for
 *        seconds, and leaving it to the first handshake put that inside DeviceManager's
 *        synchronous open() call, freezing the window mid-dial.
 */
void IO::Drivers::OpcUa::prepareClientIdentity()
{
  if (m_securityPolicy == QLatin1String(kPolicyNone))
    return;

  QByteArray certificate;
  QByteArray key;
  if (!OpcUaSecurity::ensureClientIdentity(certificate, key))
    logDriverError(tr("OPC UA Certificate"),
                   tr("The client certificate could not be generated; secure channels will be "
                      "refused."));

  Q_EMIT certificateChanged();
}

/**
 * @brief Warns once per dial when credentials would travel in the clear. Only an unencrypted
 *        channel exposes them: on Sign or SignAndEncrypt the token is protected, so the warning
 *        is conditional rather than permanent.
 */
void IO::Drivers::OpcUa::warnAboutPlaintextCredentials() const
{
  if (!credentialsAreExposed())
    return;

  qCWarning(lcOpcUa) << "Credentials selected on an unencrypted channel: they travel in the "
                        "clear to"
                     << selectedEndpointUrl();
}

/**
 * @brief True when the chosen identity sends a secret over a channel that does not encrypt it.
 */
bool IO::Drivers::OpcUa::credentialsAreExposed() const
{
  if (m_authMode != 1)
    return false;

  return dialEndpoint().securityMode != OpcUaTypes::SecurityMode::SignAndEncrypt;
}

/**
 * @brief True when a discovered endpoint row is current and this build can dial it.
 */
bool IO::Drivers::OpcUa::hasSelectedEndpoint() const noexcept
{
  return m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size()
      && endpointUsable(m_endpoints.at(m_endpointIndex))
      && endpointAcceptsToken(m_endpoints.at(m_endpointIndex), m_authMode);
}

/**
 * @brief Creates a session bound to this driver's thread.
 */
IO::Drivers::OpcUaSession* IO::Drivers::OpcUa::makeSession()
{
  return new OpcUaSession(this);
}

/**
 * @brief The identity the session presents: anonymous, username/password, or an X.509
 *        certificate the user supplied.
 */
IO::Drivers::OpcUaSession::Identity IO::Drivers::OpcUa::identity() const
{
  OpcUaSession::Identity out;
  out.mode            = m_authMode;
  out.username        = m_username;
  out.password        = m_password;
  out.certificatePath = m_userCertificatePath;
  out.privateKeyPath  = m_userKeyPath;
  return out;
}

/**
 * @brief The endpoint URL the dial targets (selected endpoint, else the typed one).
 */
QString IO::Drivers::OpcUa::selectedEndpointUrl() const
{
  if (m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size())
    return m_endpoints.at(m_endpointIndex).endpointUrl;

  return m_endpointUrl;
}

/**
 * @brief True when the endpoint advertises a user token the selected authentication mode can
 *        present; a server offering only Anonymous rejects a username session outright.
 */
bool IO::Drivers::OpcUa::endpointAcceptsToken(const OpcUaTypes::Endpoint& endpoint,
                                              const int authMode)
{
  const auto wanted = authMode == 1 ? OpcUaTypes::UserTokenType::Username
                    : authMode == 2 ? OpcUaTypes::UserTokenType::Certificate
                                    : OpcUaTypes::UserTokenType::Anonymous;

  if (endpoint.userTokenTypes.isEmpty())
    return true;

  return endpoint.userTokenTypes.contains(wanted);
}

/**
 * @brief The endpoint actually dialed: the discovered row carrying the URL the user typed, since
 *        servers advertise their own hostname (S7, Kepware, B&R) which rarely resolves from the
 *        engineering laptop. With no row selected the CONFIGURED policy and mode are used;
 *        falling back to None there would dial unencrypted whenever Discover was not pressed.
 */
IO::Drivers::OpcUaTypes::Endpoint IO::Drivers::OpcUa::dialEndpoint() const
{
  OpcUaTypes::Endpoint endpoint;
  if (!hasSelectedEndpoint()) {
    endpoint.endpointUrl = m_endpointUrl;
    endpoint.securityPolicyUri =
      m_securityPolicy.isEmpty() ? QString::fromLatin1(kPolicyNone) : m_securityPolicy;
    endpoint.securityMode = static_cast<OpcUaTypes::SecurityMode>(m_securityMode);
    return endpoint;
  }

  endpoint = m_endpoints.at(m_endpointIndex);

  const QUrl typed(m_endpointUrl);
  QUrl advertised(endpoint.endpointUrl);
  if (!typed.isValid() || typed.host().isEmpty() || !advertised.isValid())
    return endpoint;

  advertised.setHost(typed.host());
  advertised.setPort(typed.port(advertised.port(kDefaultPort)));
  endpoint.endpointUrl = advertised.toString();
  return endpoint;
}

/**
 * @brief Ends a failed dial exactly once: tears the session down, logs, reports the verdict and
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
 * @brief Last-resort dial deadline; the stack normally reports sooner.
 */
void IO::Drivers::OpcUa::onDialTimeout()
{
  failDial(tr("Timed out after %1 s").arg(kOpcUaDialDeadlineMs / 1000));
}

//--------------------------------------------------------------------------------------------------
// Session signal handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The session activated: settle the dial and start the subscription.
 */
void IO::Drivers::OpcUa::onSessionConnected()
{
  if (!m_session)
    return;

  m_dialTimer->stop();
  m_connecting = false;
  reportOpenFinished(true);
  subscribeAll();

  Q_EMIT statusChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief The session went down after it had been established: a link drop, never a dial verdict.
 */
void IO::Drivers::OpcUa::onSessionDisconnected()
{
  if (m_connecting || m_slots.isEmpty())
    return;

  onLinkDropped(m_lastError.isEmpty() ? tr("The server closed the session") : m_lastError);
}

/**
 * @brief The session's single verdict for a failed attempt.
 */
void IO::Drivers::OpcUa::onConnectFailed(const QString& reason)
{
  reportTrustFailure(m_session);
  failDial(m_lastError.isEmpty() ? reason : m_lastError);
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

  teardownSession(m_discoverySession);
  m_discoverySession = makeSession();
  if (!m_discoverySession) {
    m_lastError = tr("The %1 stack is not available in this build").arg(kBackendName);
    continuePendingDial();
    return;
  }

  connect(m_discoverySession,
          &OpcUaSession::endpointsReady,
          this,
          &IO::Drivers::OpcUa::onEndpointsFinished);

  m_discovering = true;
  Q_EMIT discoveringChanged();

  if (!m_discoverySession->discoverEndpoints(m_endpointUrl))
    onEndpointsFinished({}, OpcUaTypes::kStatusBadInternal);
}

/**
 * @brief Stores the advertised endpoints and selects the best one this build can dial with the
 *        chosen identity; a previously selected URL keeps its place across a re-discovery.
 */
void IO::Drivers::OpcUa::onEndpointsFinished(const QList<OpcUaTypes::Endpoint>& endpoints,
                                             OpcUaTypes::StatusCode status)
{
  m_discovering = false;

  if (!OpcUaTypes::isGood(status)) {
    m_endpoints.clear();
    publishEndpointSelection(-1);
    m_lastError = tr("Discovery failed: %1").arg(OpcUaSession::describeStatus(status));
    logDriverError(tr("OPC UA Discovery Failed"), tr("\"%1\": %2").arg(m_endpointUrl, m_lastError));
  } else {
    const auto previous = m_endpointIndex >= 0 && m_endpointIndex < m_endpoints.size()
                          ? m_endpoints.at(m_endpointIndex).endpointUrl
                          : QString();

    m_endpoints = endpoints;
    selectBestEndpoint(previous);
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
    if (m_session && hasSelectedEndpoint())
      startDial();
    else
      failDial(m_lastError.isEmpty() ? tr("Endpoint discovery failed") : m_lastError);

    return;
  }

  if (pending != PendingDial::Browse)
    return;

  if (m_browseSession && hasSelectedEndpoint()) {
    if (m_browseSession->connectToEndpoint(dialEndpoint(), identity()))
      return;
  }

  Q_EMIT browseFailed(m_lastError.isEmpty() ? tr("Endpoint discovery failed") : m_lastError);
  cancelBrowse();
}

//--------------------------------------------------------------------------------------------------
// Subscription, poll fallback and the publish tick
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes the wire layout, reserves the frame once and asks the session for ONE
 *        subscription carrying every tag; the per-item verdicts arrive together in onSubscribed().
 */
void IO::Drivers::OpcUa::subscribeAll()
{
  SS_ASSERT(m_session != nullptr, return);
  SS_ASSERT(!m_tags.isEmpty(), return);

  const auto steady     = std::chrono::steady_clock::now().time_since_epoch();
  const qint64 steadyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(steady).count();
  m_clockOffsetNs       = steadyNs - QDateTime::currentMSecsSinceEpoch() * kNsPerMs;
  m_clockValid          = true;
  m_lastStampNs         = 0;
  m_lastNotifyNs        = steadyNs;
  m_serverOffsetMs      = 0;

  reserveFrame();

  m_pendingMonitors = m_tags.size();
  m_failedMonitors  = 0;
  m_revisedInterval = m_publishingInterval;
  m_pollMode        = false;
  m_subscribing     = true;
  m_polledTags.clear();

  QStringList nodeIds;
  nodeIds.reserve(m_tags.size());
  for (int i = 0; i < m_tags.size(); ++i) {
    nodeIds.append(m_tags.at(i).nodeId);
    m_nodeIndex.insert(m_tags.at(i).nodeId, i);
  }

  if (!m_session->subscribe(nodeIds, m_publishingInterval))
    onSubscribed(QList<OpcUaTypes::StatusCode>(m_tags.size(), OpcUaTypes::kStatusBadInternal));

  m_frameTimer->start(m_publishingInterval);
  m_watchdog->start(kWatchdogMs);
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
 * @brief The batch verdict, one status per tag. A refusal stays INDIVIDUAL: the refused tags move
 *        to timed reads and the rest keep their subscription, and only an all-refused verdict
 *        flips the whole session into poll mode.
 */
void IO::Drivers::OpcUa::onSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus)
{
  m_subscribing     = false;
  m_pendingMonitors = 0;
  m_failedMonitors  = 0;
  m_polledTags.clear();

  for (int i = 0; i < perItemStatus.size() && i < m_tags.size(); ++i) {
    if (OpcUaTypes::isGood(perItemStatus.at(i)))
      continue;

    ++m_failedMonitors;
    m_polledTags.append(i);
    logDriverError(
      tr("OPC UA Monitored Item Refused"),
      tr("\"%1\": %2").arg(m_tags.at(i).nodeId, OpcUaSession::describeStatus(perItemStatus.at(i))));
  }

  adoptRevisedInterval();

  if (m_failedMonitors >= m_tags.size()) {
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
 * @brief The server dropped the subscription on its own; timed reads take over.
 */
void IO::Drivers::OpcUa::onSubscriptionLost(const QString& reason)
{
  if (!isOpen())
    return;

  enterPollMode(reason.isEmpty() ? tr("the server retired the subscription") : reason);
}

/**
 * @brief Adopts the interval the server revised the subscription to; a PLC that floors publishing
 *        at 100 ms must not leave the pane claiming the rate the user asked for.
 */
void IO::Drivers::OpcUa::adoptRevisedInterval()
{
  SS_ASSERT(m_session != nullptr, return);

  const int revised = m_session->revisedInterval();
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
 * @brief Issues the batched Value read for a tag subset. Exactly one read is outstanding at a
 *        time: queueing them behind a slow PLC grows latency without bound and eventually times
 *        the session out, so the session drops the overflow rather than queueing it.
 */
void IO::Drivers::OpcUa::issueRead(const QList<int>& tags)
{
  SS_ASSERT(m_session != nullptr, return);
  SS_ASSERT(!tags.isEmpty(), return);

  QStringList nodeIds;
  nodeIds.reserve(tags.size());
  for (const int tag : tags) {
    if (tag < 0 || tag >= m_tags.size())
      continue;

    nodeIds.append(m_tags.at(tag).nodeId);
  }

  if (!nodeIds.isEmpty())
    m_readInFlight = m_session->readValues(nodeIds);
}

/**
 * @brief Routes batched read results into the slot cache by node id.
 */
void IO::Drivers::OpcUa::onReadFinished(quint32 token,
                                        const QList<OpcUaTypes::ReadRow>& rows,
                                        OpcUaTypes::StatusCode status)
{
  Q_UNUSED(token)

  m_readInFlight = false;
  if (!OpcUaTypes::isGood(status)) {
    logDriverError(tr("OPC UA Read Failed"), OpcUaSession::describeStatus(status));
    return;
  }

  for (const auto& row : rows) {
    const int tag = m_nodeIndex.value(row.nodeId, -1);
    if (tag < 0)
      continue;

    storeValue(tag, row.value, row.status, row.sourceTimestamp);
  }
}

/**
 * @brief Monitored-item update: the tag index and the server's own timestamps travel with the
 *        notification, so nothing has to be looked up or re-stamped here.
 */
void IO::Drivers::OpcUa::onValueChanged(const OpcUaTypes::MonitoredValue& value)
{
  SS_ASSERT(value.tag >= 0, return);
  if (value.tag >= m_tags.size())
    return;

  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  m_lastNotifyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

  if (!m_serverOffsetMs && value.serverTimestamp.isValid())
    m_serverOffsetMs =
      value.serverTimestamp.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();

  storeValue(value.tag, value.value, value.status, value.sourceTimestamp);
}

/**
 * @brief Writes a value into its slot(s); a bad status keeps the last good value and counts.
 *        Arrays fan out element-wise, extra elements are dropped, missing ones left latched.
 */
void IO::Drivers::OpcUa::storeValue(int tag,
                                    const QVariant& value,
                                    OpcUaTypes::StatusCode status,
                                    const QDateTime& sourceTs)
{
  SS_ASSERT(tag >= 0 && tag < m_firstIndex.size(), return);
  ++m_valuesReceived;

  if (OpcUaTypes::isBad(status)) {
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
      slot.value    = list.at(i);
      slot.sourceTs = sourceTs;
      slot.dirty    = true;
      slot.bad      = false;
    }

    return;
  }

  SS_ASSERT(first < m_slots.size(), return);
  auto& slot    = m_slots[first];
  slot.value    = value;
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
 * @brief Opens a browse-only session beside the live one so the picker can walk the address
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

  teardownSession(m_browseSession);
  m_browseSession = makeSession();
  if (!m_browseSession) {
    Q_EMIT browseFailed(tr("The %1 stack is not available in this build.").arg(kBackendName));
    return;
  }

  connect(m_browseSession, &OpcUaSession::connected, this, &IO::Drivers::OpcUa::onBrowseConnected);
  connect(m_browseSession, &OpcUaSession::connectFailed, this, &IO::Drivers::OpcUa::onBrowseFailed);
  connect(m_browseSession, &OpcUaSession::disconnected, this, [this] {
    onBrowseFailed(tr("The browse session was closed by the server"));
  });

  tagModel()->preselect(m_tags);
  m_browsing = true;
  Q_EMIT browsingChanged();
  prepareClientIdentity();

  if (hasSelectedEndpoint()) {
    if (!m_browseSession->connectToEndpoint(dialEndpoint(), identity()))
      onBrowseFailed(tr("The browse session could not be started"));

    return;
  }

  m_pendingDial = PendingDial::Browse;
  discoverEndpoints();
}

/**
 * @brief Commits the picker's selection as the tag list and ends the browse session. Tags the
 *        picker never fetched keep their place: an unexpanded folder is not an unchecked one.
 */
void IO::Drivers::OpcUa::stopBrowse()
{
  if (m_tagModel && m_browseSession && m_browseSession->isOpen()) {
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
    m_tagModel->setSession(nullptr);

  teardownSession(m_browseSession);
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
    m_tagModel->setSession(nullptr);

  teardownSession(m_browseSession);
  if (!m_browsing)
    return;

  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Lends the connected browse session to the model and fetches the root level.
 */
void IO::Drivers::OpcUa::onBrowseConnected()
{
  if (!m_browseSession)
    return;

  tagModel()->setSession(m_browseSession);
  tagModel()->fetchMore(QModelIndex());
}

/**
 * @brief A browse session that could not be opened, or one the server closed.
 */
void IO::Drivers::OpcUa::onBrowseFailed(const QString& reason)
{
  if (!m_browsing)
    return;

  reportTrustFailure(m_browseSession);
  m_lastError = reason.isEmpty()
                ? tr("Could not open a browse session on %1").arg(selectedEndpointUrl())
                : reason;

  Q_EMIT browseFailed(m_lastError);
  Q_EMIT statusChanged();

  if (m_tagModel)
    m_tagModel->setSession(nullptr);

  teardownSession(m_browseSession);
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
    auto policy = endpoint.securityPolicyUri.section(QLatin1Char('#'), -1);
    if (policyIsDeprecated(endpoint.securityPolicyUri))
      policy += tr(" (deprecated)");

    list.append(QStringLiteral("%1 / %2 / %3")
                  .arg(policy, describeMode(endpoint.securityMode), endpoint.endpointUrl));
  }

  return list;
}

/**
 * @brief The translated name of a message security mode.
 */
QString IO::Drivers::OpcUa::describeMode(OpcUaTypes::SecurityMode mode)
{
  switch (mode) {
    case OpcUaTypes::SecurityMode::None:
      return tr("None");
    case OpcUaTypes::SecurityMode::Sign:
      return tr("Sign");
    case OpcUaTypes::SecurityMode::SignAndEncrypt:
      return tr("Sign and Encrypt");
    case OpcUaTypes::SecurityMode::Invalid:
      break;
  }

  return tr("Invalid");
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
    QJsonArray tokenSlugs;
    for (const auto token : endpoint.userTokenTypes) {
      tokens.append(static_cast<int>(token));
      tokenSlugs.append(API::EnumLabels::identityTokenSlug(static_cast<int>(token)));
    }

    out.append(QJsonObject{
      {QStringLiteral("index"), i},
      {QStringLiteral("policy"), endpoint.securityPolicyUri.section(QLatin1Char('#'), -1)},
      {QStringLiteral("policyUri"), endpoint.securityPolicyUri},
      {QStringLiteral("deprecated"), policyIsDeprecated(endpoint.securityPolicyUri)},
      {QStringLiteral("securityLevel"), static_cast<int>(endpoint.securityLevel)},
      {QStringLiteral("mode"), static_cast<int>(endpoint.securityMode)},
      {QStringLiteral("modeSlug"),
       API::EnumLabels::securityModeSlug(static_cast<int>(endpoint.securityMode))},
      {QStringLiteral("modeLabel"),
       API::EnumLabels::securityModeLabel(static_cast<int>(endpoint.securityMode))},
      {QStringLiteral("policySlug"),
       API::EnumLabels::securityPolicySlug(
         supportedPolicies().indexOf(endpoint.securityPolicyUri))},
      {QStringLiteral("url"), endpoint.endpointUrl},
      {QStringLiteral("tokens"), tokens},
      {QStringLiteral("tokenSlugs"), tokenSlugs},
      {QStringLiteral("selectable"),
       endpointUsable(endpoint) && endpointAcceptsToken(endpoint, m_authMode)},
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
    list.append(endpointUsable(endpoint) && endpointAcceptsToken(endpoint, m_authMode));

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
 * @brief One-line session status for the pane, read from the live session when this instance
 *        is the UI-config one.
 */
QString IO::Drivers::OpcUa::statusText() const
{
  if (const auto* peer = sessionPeer())
    return peer->statusText();

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
  if (const auto* peer = sessionPeer())
    return peer->pollMode();

  return m_pollMode;
}

/**
 * @brief The publishing interval the server revised the subscription to (0 when not subscribed).
 */
int IO::Drivers::OpcUa::revisedInterval() const
{
  if (const auto* peer = sessionPeer())
    return peer->revisedInterval();

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
  return {tr("Anonymous"), tr("Username / Password"), tr("X.509 Certificate")};
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
 * @brief Pulled diagnostics snapshot (spec 0033: counters, never pushed), read from the live
 *        session when this instance is the UI-config one.
 */
QJsonObject IO::Drivers::OpcUa::statusJson() const
{
  if (const auto* peer = sessionPeer()) {
    auto snapshot = peer->statusJson();
    if (snapshot.value(QStringLiteral("lastError")).toString().isEmpty() && !m_lastError.isEmpty())
      snapshot.insert(QStringLiteral("lastError"), m_lastError);

    return snapshot;
  }

  return QJsonObject{
    {         QStringLiteral("connected"),                               isOpen()},
    {        QStringLiteral("connecting"),                           m_connecting},
    {          QStringLiteral("pollMode"),                             m_pollMode},
    {       QStringLiteral("subscribing"),                          m_subscribing},
    {   QStringLiteral("revisedInterval"),                      m_revisedInterval},
    {       QStringLiteral("refusedTags"),                    m_polledTags.size()},
    {      QStringLiteral("skippedPolls"),    static_cast<qint64>(m_skippedPolls)},
    {           QStringLiteral("badTags"),  QJsonArray::fromStringList(badTags())},
    {          QStringLiteral("endpoint"),                  selectedEndpointUrl()},
    {    QStringLiteral("securityPolicy"),                     negotiatedPolicy()},
    {      QStringLiteral("securityMode"),                       negotiatedMode()},
    {  QStringLiteral("configuredPolicy"),                       m_securityPolicy},
    {    QStringLiteral("configuredMode"),                         m_securityMode},
    {QStringLiteral("credentialsExposed"),                   credentialsExposed()},
    { QStringLiteral("serverCertificate"),      certificateObject(m_pendingTrust)},
    {          QStringLiteral("tagCount"),                          m_tags.size()},
    {    QStringLiteral("valuesReceived"),  static_cast<qint64>(m_valuesReceived)},
    {         QStringLiteral("badStatus"),  static_cast<qint64>(m_badStatusCount)},
    {         QStringLiteral("unstamped"),  static_cast<qint64>(m_unstampedCount)},
    {   QStringLiteral("framesPublished"), static_cast<qint64>(m_framesPublished)},
    {        QStringLiteral("reconnects"),       static_cast<qint64>(m_linkDrops)},
    {         QStringLiteral("lastError"),                            m_lastError},
    {        QStringLiteral("statusText"),                           statusText()},
  };
}

/**
 * @brief The policy the live channel negotiated, falling back to the configured one when no
 *        session is up. A status reporting only what was asked for cannot show that a dial
 *        settled on something weaker.
 */
QString IO::Drivers::OpcUa::negotiatedPolicy() const
{
  if (m_session && m_session->isOpen())
    return m_session->securityPolicyUri();

  return m_securityPolicy;
}

/**
 * @brief The message security mode the live channel negotiated, else the configured one.
 */
int IO::Drivers::OpcUa::negotiatedMode() const
{
  if (m_session && m_session->isOpen())
    return static_cast<int>(m_session->securityMode());

  return m_securityMode;
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
  publishEndpointSelection(-1);

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

  if (index >= 0 && (index >= m_endpoints.size() || !endpointUsable(m_endpoints.at(index))))
    return;

  publishEndpointSelection(index);
}

/**
 * @brief Adopts an endpoint selection and republishes everything derived from it. The security
 *        posture of a connection follows the SELECTED endpoint, so credentialsExposed goes stale
 *        (banner shown on an encrypted channel, hidden on a clear one) whenever a selection
 *        moves without securityChanged.
 */
void IO::Drivers::OpcUa::publishEndpointSelection(const int index)
{
  m_endpointIndex = index;
  Q_EMIT endpointIndexChanged();
  Q_EMIT securityChanged();
}

/**
 * @brief Sets the authentication mode.
 */
void IO::Drivers::OpcUa::setAuthMode(const int mode)
{
  const int clamped = qBound(0, mode, 2);
  if (m_authMode == clamped)
    return;

  m_authMode = clamped;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/authMode", m_authMode);

  Q_EMIT authModeChanged();
  Q_EMIT securityChanged();
  Q_EMIT endpointsChanged();
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

  if (m_session)
    (void)m_session->modifyPublishingInterval(m_publishingInterval);

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
  return m_session != nullptr;
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
// Security configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every security policy this build can open, weakest first.
 */
const QStringList& IO::Drivers::OpcUa::supportedPolicies()
{
  static const QStringList k_policies = [] {
    QStringList out;
    for (const auto* uri : kPolicyUris)
      out.append(QString::fromLatin1(uri));

    return out;
  }();

  return k_policies;
}

/**
 * @brief True for the two policies the OPC Foundation has deprecated. They stay reachable because
 *        field controllers still ship them, but the UI labels them and nothing auto-selects one.
 */
bool IO::Drivers::OpcUa::policyIsDeprecated(const QString& policyUri)
{
  return policyUri.endsWith(QLatin1String("#Basic128Rsa15"))
      || policyUri.endsWith(QLatin1String("#Basic256"));
}

/**
 * @brief True when this build can dial the endpoint. Every policy in kPolicyUris is supported, so
 *        an endpoint is usable when its policy is one of them and its mode is a real one.
 */
bool IO::Drivers::OpcUa::endpointUsable(const OpcUaTypes::Endpoint& endpoint) const
{
  if (endpoint.securityMode == OpcUaTypes::SecurityMode::Invalid)
    return false;

  return supportedPolicies().contains(endpoint.securityPolicyUri);
}

/**
 * @brief Picks the endpoint to dial after a discovery. A URL that was already selected keeps its
 *        place; otherwise the MOST secure usable endpoint wins, and a deprecated policy is only
 *        ever chosen when the user asked for it by name.
 */
void IO::Drivers::OpcUa::selectBestEndpoint(const QString& previousUrl)
{
  m_endpointIndex = -1;

  int bestScore = -1;
  for (int i = 0; i < m_endpoints.size(); ++i) {
    const auto& candidate = m_endpoints.at(i);
    if (!endpointUsable(candidate) || !endpointAcceptsToken(candidate, m_authMode))
      continue;

    if (!previousUrl.isEmpty() && candidate.endpointUrl == previousUrl) {
      m_endpointIndex = i;
      return;
    }

    const bool wanted = candidate.securityPolicyUri == m_securityPolicy
                     && static_cast<int>(candidate.securityMode) == m_securityMode;
    const int score = wanted ? 1000
                    : policyIsDeprecated(candidate.securityPolicyUri)
                      ? 0
                      : supportedPolicies().indexOf(candidate.securityPolicyUri) * 10
                          + static_cast<int>(candidate.securityMode);
    if (score <= bestScore)
      continue;

    bestScore       = score;
    m_endpointIndex = i;
  }

  if (m_endpointIndex >= 0) {
    m_securityPolicy = m_endpoints.at(m_endpointIndex).securityPolicyUri;
    m_securityMode   = static_cast<int>(m_endpoints.at(m_endpointIndex).securityMode);
  }

  Q_EMIT securityChanged();
  if (m_endpointIndex >= 0)
    return;

  m_lastError = tr("No endpoint this build can open with the selected identity");
  logDriverError(tr("OPC UA Discovery"), tr("\"%1\": %2").arg(m_endpointUrl, m_lastError));
}

/**
 * @brief The configured security policy URI.
 */
QString IO::Drivers::OpcUa::securityPolicy() const
{
  return m_securityPolicy;
}

/**
 * @brief The configured policy as a row of securityPolicyList().
 */
int IO::Drivers::OpcUa::securityPolicyIndex() const
{
  return qMax(0, supportedPolicies().indexOf(m_securityPolicy));
}

/**
 * @brief Short names of the supported policies, for the picker.
 */
QStringList IO::Drivers::OpcUa::securityPolicyList() const
{
  QStringList out;
  for (const auto& uri : supportedPolicies()) {
    const auto name = uri.section(QLatin1Char('#'), -1);
    out.append(policyIsDeprecated(uri) ? tr("%1 (deprecated)").arg(name) : name);
  }

  return out;
}

/**
 * @brief Parallel to securityPolicyList(): true where the row is a deprecated policy.
 */
QVariantList IO::Drivers::OpcUa::securityPolicyDeprecated() const
{
  QVariantList out;
  for (const auto& uri : supportedPolicies())
    out.append(policyIsDeprecated(uri));

  return out;
}

/**
 * @brief The configured message security mode.
 */
int IO::Drivers::OpcUa::securityMode() const
{
  return m_securityMode;
}

/**
 * @brief Translated message-security-mode labels, indexed by the OPC UA enumeration.
 */
QStringList IO::Drivers::OpcUa::securityModeList() const
{
  return {tr("Invalid"), tr("None"), tr("Sign"), tr("Sign and Encrypt")};
}

/**
 * @brief Selects the security policy; a change invalidates the discovered endpoint selection.
 */
void IO::Drivers::OpcUa::setSecurityPolicy(const QString& policyUri)
{
  if (m_securityPolicy == policyUri || !supportedPolicies().contains(policyUri))
    return;

  m_securityPolicy = policyUri;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/securityPolicy", m_securityPolicy);

  if (m_securityPolicy == QLatin1String(kPolicyNone))
    m_securityMode = static_cast<int>(OpcUaTypes::SecurityMode::None);
  else if (m_securityMode <= static_cast<int>(OpcUaTypes::SecurityMode::None))
    m_securityMode = static_cast<int>(OpcUaTypes::SecurityMode::SignAndEncrypt);

  m_endpointIndex = -1;
  Q_EMIT securityChanged();
  Q_EMIT endpointIndexChanged();
}

/**
 * @brief Selects the security policy by row of securityPolicyList().
 */
void IO::Drivers::OpcUa::setSecurityPolicyIndex(const int index)
{
  const auto policies = supportedPolicies();
  if (index < 0 || index >= policies.size())
    return;

  setSecurityPolicy(policies.at(index));
}

/**
 * @brief Selects the message security mode. Dropping to None drops the policy with it: a strong
 *        policy left on a None channel dials in the clear while the pane still names the policy,
 *        the same silent-plaintext trap dialEndpoint() guards on the other side.
 */
void IO::Drivers::OpcUa::setSecurityMode(const int mode)
{
  const int clamped = qBound(static_cast<int>(OpcUaTypes::SecurityMode::None),
                             mode,
                             static_cast<int>(OpcUaTypes::SecurityMode::SignAndEncrypt));
  if (m_securityMode == clamped)
    return;

  m_securityMode = clamped;
  if (clamped == static_cast<int>(OpcUaTypes::SecurityMode::None))
    m_securityPolicy = QString::fromLatin1(kPolicyNone);

  if (m_persistent) {
    m_settings.setValue("OpcUaDriver/securityMode", m_securityMode);
    m_settings.setValue("OpcUaDriver/securityPolicy", m_securityPolicy);
  }

  m_endpointIndex = -1;
  Q_EMIT securityChanged();
  Q_EMIT endpointIndexChanged();
}

/**
 * @brief The user certificate presented for X.509 identity (authentication mode 2).
 */
QString IO::Drivers::OpcUa::userCertificatePath() const
{
  return m_userCertificatePath;
}

/**
 * @brief The private key that proves ownership of the user certificate.
 */
QString IO::Drivers::OpcUa::userKeyPath() const
{
  return m_userKeyPath;
}

/**
 * @brief Turns whatever a caller hands us into a local path. QML file dialogs deliver a
 *        `file://` URL, the API delivers a plain path, and both reach the same setter.
 */
static QString localPath(const QString& value)
{
  if (!value.startsWith(QLatin1String("file:")))
    return value;

  return QUrl(value).toLocalFile();
}

/**
 * @brief Sets the user certificate path.
 */
void IO::Drivers::OpcUa::setUserCertificatePath(const QString& value)
{
  const auto path = localPath(value);
  if (m_userCertificatePath == path)
    return;

  m_userCertificatePath = path;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/userCertificate", m_userCertificatePath);

  Q_EMIT securityChanged();
}

/**
 * @brief Sets the user private-key path. Only the PATH is stored: the key itself never enters
 *        the project file or QSettings.
 */
void IO::Drivers::OpcUa::setUserKeyPath(const QString& value)
{
  const auto path = localPath(value);
  if (m_userKeyPath == path)
    return;

  m_userKeyPath = path;
  if (m_persistent)
    m_settings.setValue("OpcUaDriver/userKey", m_userKeyPath);

  Q_EMIT securityChanged();
}

/**
 * @brief True when the chosen identity would send a secret over a channel that does not encrypt
 *        it, which is what the pane's warning banner is conditional on.
 */
bool IO::Drivers::OpcUa::credentialsExposed() const
{
  return credentialsAreExposed();
}

//--------------------------------------------------------------------------------------------------
// Certificates and trust
//--------------------------------------------------------------------------------------------------

/**
 * @brief A certificate rendered for QML.
 */
QVariantMap IO::Drivers::OpcUa::certificateMap(const OpcUaTypes::CertInfo& info)
{
  return QVariantMap{
    {          QStringLiteral("valid"),           info.valid},
    {        QStringLiteral("subject"),         info.subject},
    {         QStringLiteral("issuer"),          info.issuer},
    {    QStringLiteral("fingerprint"),     info.fingerprint},
    { QStringLiteral("applicationUri"),  info.applicationUri},
    {      QStringLiteral("notBefore"),       info.notBefore},
    {       QStringLiteral("notAfter"),        info.notAfter},
    {        QStringLiteral("trusted"),         info.trusted},
    {        QStringLiteral("expired"),         info.expired},
    {    QStringLiteral("notYetValid"),     info.notYetValid},
    {QStringLiteral("hostnameMatches"), info.hostnameMatches},
  };
}

/**
 * @brief A certificate rendered for the API.
 */
QJsonObject IO::Drivers::OpcUa::certificateObject(const OpcUaTypes::CertInfo& info)
{
  return QJsonObject{
    {          QStringLiteral("valid"),                           info.valid},
    {        QStringLiteral("subject"),                         info.subject},
    {         QStringLiteral("issuer"),                          info.issuer},
    {    QStringLiteral("fingerprint"),                     info.fingerprint},
    { QStringLiteral("applicationUri"),                  info.applicationUri},
    {      QStringLiteral("notBefore"), info.notBefore.toString(Qt::ISODate)},
    {       QStringLiteral("notAfter"),  info.notAfter.toString(Qt::ISODate)},
    {        QStringLiteral("trusted"),                         info.trusted},
    {        QStringLiteral("expired"),                         info.expired},
    {    QStringLiteral("notYetValid"),                     info.notYetValid},
    {QStringLiteral("hostnameMatches"),                 info.hostnameMatches},
  };
}

/**
 * @brief Why a certificate was refused, in the user's words. Kept distinct on purpose: trust it,
 *        renew it, wait for it and dial the right name are four different fixes.
 */
QString IO::Drivers::OpcUa::describeTrustFailure(OpcUaTypes::TrustFailure failure)
{
  switch (failure) {
    case OpcUaTypes::TrustFailure::Untrusted:
      return tr("The server certificate is not trusted");
    case OpcUaTypes::TrustFailure::Expired:
      return tr("The server certificate has expired");
    case OpcUaTypes::TrustFailure::NotYetValid:
      return tr("The server certificate is not valid yet");
    case OpcUaTypes::TrustFailure::HostnameMismatch:
      return tr("The server certificate was not issued for this host");
    case OpcUaTypes::TrustFailure::Unreadable:
      return tr("The server certificate could not be parsed");
    case OpcUaTypes::TrustFailure::None:
      break;
  }

  return {};
}

/**
 * @brief Publishes a rejected server certificate so the pane can offer the trust prompt. Emitted
 *        QUEUED: a modal opened synchronously from inside the dial's error path would spin a
 *        nested event loop in the middle of an emission (the macOS file-dialog reentrancy class).
 */
void IO::Drivers::OpcUa::reportTrustFailure(const OpcUaSession* session)
{
  if (!session || session->trustFailure() == OpcUaTypes::TrustFailure::None)
    return;

  m_pendingTrust    = session->serverCertificate();
  const auto detail = describeTrustFailure(session->trustFailure());
  const auto map    = certificateMap(m_pendingTrust);

  m_lastError = detail;
  QMetaObject::invokeMethod(
    this,
    [this, map, detail] { Q_EMIT serverCertificateUntrusted(map, detail); },
    Qt::QueuedConnection);
}

/**
 * @brief The installation's own client certificate, generated on first secure use.
 */
QVariantMap IO::Drivers::OpcUa::clientCertificate() const
{
  return certificateMap(OpcUaSecurity::inspect(OpcUaSecurity::clientCertificate(), QString()));
}

/**
 * @brief The installation's client certificate, for the API.
 */
QJsonObject IO::Drivers::OpcUa::certificateJson() const
{
  return certificateObject(OpcUaSecurity::inspect(OpcUaSecurity::clientCertificate(), QString()));
}

/**
 * @brief Every server certificate the user has accepted.
 */
QVariantList IO::Drivers::OpcUa::trustedCertificates() const
{
  QVariantList out;
  const auto certificates = OpcUaSecurity::trustedCertificates();
  for (const auto& certificate : certificates)
    out.append(certificateMap(OpcUaSecurity::inspect(certificate, QString())));

  return out;
}

/**
 * @brief Every accepted server certificate, for the API.
 */
QJsonArray IO::Drivers::OpcUa::trustedJson() const
{
  QJsonArray out;
  const auto certificates = OpcUaSecurity::trustedCertificates();
  for (const auto& certificate : certificates)
    out.append(certificateObject(OpcUaSecurity::inspect(certificate, QString())));

  return out;
}

/**
 * @brief Replaces the installation's client certificate and key. Every server that trusted the
 *        old one has to trust the new one, so this is a deliberate action and never automatic.
 */
bool IO::Drivers::OpcUa::regenerateCertificate()
{
  const bool ok = OpcUaSecurity::regenerateClientIdentity();
  if (ok)
    Q_EMIT certificateChanged();

  return ok;
}

/**
 * @brief Writes the client certificate where the user asked, to hand to the server's trust store.
 */
bool IO::Drivers::OpcUa::exportCertificate(const QString& path)
{
  return OpcUaSecurity::exportClientCertificate(localPath(path));
}

/**
 * @brief Accepts the server certificate the last attempt was refused over, and only when the
 *        caller names it: the fingerprint is the confirmation token for a security decision, so an
 *        empty one is a mismatch rather than a wildcard. This does NOT retry -- a trust decision
 *        followed by a connect is a NEW attempt with its own single verdict.
 */
bool IO::Drivers::OpcUa::trustServerCertificate(const QString& fingerprint)
{
  const auto* peer   = sessionPeer();
  const auto pending = peer ? peer->m_pendingTrust : m_pendingTrust;
  if (fingerprint.isEmpty() || pending.fingerprint.compare(fingerprint, Qt::CaseInsensitive) != 0)
    return false;

  if (!OpcUaSecurity::trustCertificate(pending.certificate))
    return false;

  Q_EMIT certificateChanged();
  return true;
}

/**
 * @brief Withdraws a previously accepted server certificate.
 */
bool IO::Drivers::OpcUa::revokeServerCertificate(const QString& fingerprint)
{
  const bool ok = OpcUaSecurity::revokeTrust(fingerprint);
  if (ok)
    Q_EMIT certificateChanged();

  return ok;
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

  IO::DriverProperty policy;
  policy.key     = QStringLiteral("securityPolicy");
  policy.label   = tr("Security Policy");
  policy.type    = IO::DriverProperty::ComboBox;
  policy.value   = securityPolicyIndex();
  policy.options = securityPolicyList();
  props.append(policy);

  IO::DriverProperty mode;
  mode.key     = QStringLiteral("securityMode");
  mode.label   = tr("Security Mode");
  mode.type    = IO::DriverProperty::ComboBox;
  mode.value   = m_securityMode;
  mode.options = securityModeList();
  props.append(mode);

  IO::DriverProperty userCert;
  userCert.key   = QStringLiteral("userCertificatePath");
  userCert.label = tr("User Certificate");
  userCert.type  = IO::DriverProperty::Text;
  userCert.value = m_userCertificatePath;
  props.append(userCert);

  IO::DriverProperty userKey;
  userKey.key   = QStringLiteral("userKeyPath");
  userKey.label = tr("User Private Key");
  userKey.type  = IO::DriverProperty::Text;
  userKey.value = m_userKeyPath;
  props.append(userKey);

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

  if (key == QLatin1String("securityPolicy")) {
    if (value.typeId() == QMetaType::QString)
      setSecurityPolicy(value.toString());
    else
      setSecurityPolicyIndex(value.toInt());

    return;
  }

  if (key == QLatin1String("securityMode")) {
    setSecurityMode(value.toInt());
    return;
  }

  if (key == QLatin1String("userCertificatePath")) {
    setUserCertificatePath(value.toString());
    return;
  }

  if (key == QLatin1String("userKeyPath")) {
    setUserKeyPath(value.toString());
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
