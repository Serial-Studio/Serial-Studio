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

#include "IO/Drivers/OpcUaSession.h"

#include <open62541.h>

#include <QFile>
#include <QUrl>
#include <vector>

#include "IO/Drivers/OpcUaMarshal.h"
#include "IO/Drivers/OpcUaSecurity.h"
#include "SSAssert.h"

using namespace IO::Drivers::OpcUaTypes;

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr int kSessionPumpBusyMs         = 10;
static constexpr int kSessionPumpIdleMs         = 100;
static constexpr int kSessionResolveDeadlineMs  = 5000;
static constexpr int kSessionRequestTimeoutMs   = 10000;
static constexpr int kSessionDefaultReadLimit   = 200;
static constexpr int kSessionMinIntervalMs      = 10;
static constexpr int kSessionMaxIntervalMs      = 60000;
static constexpr quint32 kSessionMaxRefsPerNode = 0;
static constexpr quint32 kSessionKeepAliveCount = 10;
static constexpr quint32 kSessionLifetimeCount  = 10000;

/**
 * @brief Maps a certificate group back to the session that installed it. The verification hook is
 *        a plain C function pointer with no user pointer of its own, and the group's `context`
 *        belongs to open62541's own memory store, so this small registry is what carries the
 *        session across. Bounded by the number of live sessions (live, browse, discovery).
 */
static QHash<const UA_CertificateGroup*, IO::Drivers::OpcUaSession*> s_verifiers;

/**
 * @brief Raises a session's re-entrancy depth for the duration of ONE open62541 call.
 *
 *        open62541 dispatches its C callbacks synchronously from whichever entry point changed
 *        the client state, not only from UA_Client_run_iterate(): a connectAsync() that fails on
 *        the spot (unresolvable host, a security policy that will not initialise) invokes the
 *        state callback before it returns, and then keeps using the client as its own stack
 *        unwinds. Those callbacks emit the signals the driver turns into close(), so the depth
 *        has to cover every such call, per session -- a flag that only covered the pump let a
 *        failed dial destroy the client that connectAsync() was still standing on.
 */
class ClientCallScope {
public:
  /**
   * @brief Enters one open62541 call on the session owning @p depth.
   */
  explicit ClientCallScope(int& depth) : m_depth(depth) { ++m_depth; }

  /**
   * @brief Leaves it, restoring the previous depth rather than assuming zero.
   */
  ~ClientCallScope() { --m_depth; }

  /**
   * @brief The guard is a stack marker; copying or moving one would misplace the depth.
   */
  ClientCallScope(ClientCallScope&&) = delete;

  /**
   * @brief As above, for the copy constructor.
   */
  ClientCallScope(const ClientCallScope&) = delete;

  ClientCallScope& operator=(ClientCallScope&&)      = delete;
  ClientCallScope& operator=(const ClientCallScope&) = delete;

private:
  int& m_depth;
};

//--------------------------------------------------------------------------------------------------
// Construction and teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an idle session; the client is created on the first dial.
 */
IO::Drivers::OpcUaSession::OpcUaSession(QObject* parent)
  : QObject(parent)
  , m_connecting(false)
  , m_open(false)
  , m_readInFlight(false)
  , m_stackDepth(0)
  , m_intent(Intent::Idle)
  , m_readLimit(kSessionDefaultReadLimit)
  , m_revisedInterval(0)
  , m_subscriptionId(0)
  , m_pump(new QTimer(this))
  , m_client(nullptr)
  , m_securityMode(OpcUaTypes::SecurityMode::None)
  , m_trustFailure(OpcUaTypes::TrustFailure::None)
  , m_pollCursor(0)
{
  m_pump->setInterval(kSessionPumpBusyMs);
  m_pump->setTimerType(Qt::CoarseTimer);
  connect(m_pump, &QTimer::timeout, this, &IO::Drivers::OpcUaSession::pump);
  connect(
    &m_resolver, &IO::AsyncTcpDial::finished, this, &IO::Drivers::OpcUaSession::onResolveFinished);
}

/**
 * @brief Retires the client; teardown() carries the ordering that keeps a late callback from
 *        reaching a freed object.
 */
IO::Drivers::OpcUaSession::~OpcUaSession()
{
  teardown();
}

/**
 * @brief Stops the pump, detaches the context and destroys the client, in that order. The context
 *        is nulled FIRST so any callback still queued inside the stack finds nothing to call back
 *        into, and the pump is stopped before the delete so no new iterate can start.
 */
void IO::Drivers::OpcUaSession::teardown()
{
  m_pump->stop();
  m_resolver.cancel();

  if (!m_client) {
    m_open           = false;
    m_connecting     = false;
    m_intent         = Intent::Idle;
    m_readInFlight   = false;
    m_subscriptionId = 0;
    return;
  }

  auto* client = m_client;
  m_client     = nullptr;

  UA_ClientConfig* config = UA_Client_getConfig(client);
  if (config) {
    config->clientContext = nullptr;
    config->stateCallback = nullptr;
    s_verifiers.remove(&config->certificateVerification);
  }

  UA_Client_delete(client);

  m_open           = false;
  m_connecting     = false;
  m_intent         = Intent::Idle;
  m_readInFlight   = false;
  m_subscriptionId = 0;
  m_monitorTags.clear();
  m_monitoredNodes.clear();
  m_namespaceArray.clear();
  m_pollCursor = 0;
  m_readRequests.clear();
  m_browseRequests.clear();
}

/**
 * @brief Ends the session. A close arriving from inside a callback defers the destruction to the
 *        end of the current pump, because the stack is still executing on the client.
 */
void IO::Drivers::OpcUaSession::close()
{
  if (!m_client)
    return;

  if (m_stackDepth > 0) {
    m_open       = false;
    m_connecting = false;
    m_intent     = Intent::Idle;

    auto* pending = m_client;
    m_pump->stop();
    m_resolver.cancel();
    QMetaObject::invokeMethod(
      this,
      [this, pending] {
        if (m_client == pending)
          teardown();
      },
      Qt::QueuedConnection);
    return;
  }

  teardown();
}

//--------------------------------------------------------------------------------------------------
// State
//--------------------------------------------------------------------------------------------------

/**
 * @brief True once the session is activated, not merely once the channel is up.
 */
bool IO::Drivers::OpcUaSession::isOpen() const noexcept
{
  return m_open;
}

/**
 * @brief True while a dial is in flight and its verdict has not been reported.
 */
bool IO::Drivers::OpcUaSession::isConnecting() const noexcept
{
  return m_connecting;
}

/**
 * @brief The server's MaxNodesPerRead, or a conservative default; a batched poll larger than this
 *        is rejected wholesale with Bad_TooManyOperations.
 */
int IO::Drivers::OpcUaSession::readLimit() const noexcept
{
  return m_readLimit;
}

/**
 * @brief The publishing interval the server actually granted, which a PLC is free to floor well
 *        above the one that was asked for. Valid once subscribed() has fired.
 */
int IO::Drivers::OpcUaSession::revisedInterval() const noexcept
{
  return m_revisedInterval;
}

//--------------------------------------------------------------------------------------------------
// The iterate pump
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gives the stack a slice to read the socket and dispatch callbacks. Everything open62541
 *        reports arrives from inside this call, on this thread.
 */
void IO::Drivers::OpcUaSession::pump()
{
  if (!m_client)
    return;

  UA_StatusCode status = UA_STATUSCODE_GOOD;
  {
    const ClientCallScope scope(m_stackDepth);
    status = UA_Client_run_iterate(m_client, 0);
  }

  if (!m_client)
    return;

  applyPumpCadence();

  if (status == UA_STATUSCODE_GOOD || !m_open)
    return;

  m_open = false;
  Q_EMIT disconnected();
}

/**
 * @brief Paces the iterate slice by what the session owes an answer to. A session with nothing
 *        outstanding needs the socket read often enough to notice a drop, not 100 times a second:
 *        three idle sessions used to cost 300 wake-ups per second between them.
 */
void IO::Drivers::OpcUaSession::applyPumpCadence()
{
  const bool busy = m_connecting || m_readInFlight || m_subscriptionId != 0
                 || !m_readRequests.isEmpty() || !m_browseRequests.isEmpty();

  const int interval = busy ? kSessionPumpBusyMs : kSessionPumpIdleMs;
  if (m_pump->interval() != interval)
    m_pump->setInterval(interval);
}

/**
 * @brief Arms the pump; harmless to call when it is already running.
 */
void IO::Drivers::OpcUaSession::startPump()
{
  applyPumpCadence();

  if (!m_pump->isActive())
    m_pump->start();
}

//--------------------------------------------------------------------------------------------------
// Client construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Recovers the session from a client, or nullptr once the context has been detached by
 *        teardown(). Every trampoline below goes through this.
 */
static IO::Drivers::OpcUaSession* sessionOf(UA_Client* client)
{
  if (!client)
    return nullptr;

  UA_ClientConfig* config = UA_Client_getConfig(client);
  if (!config)
    return nullptr;

  return static_cast<IO::Drivers::OpcUaSession*>(config->clientContext);
}

/**
 * @brief The stack's state callback, forwarded to the owning session.
 */
static void stateTrampoline(UA_Client* client,
                            UA_SecureChannelState channelState,
                            UA_SessionState sessionState,
                            UA_StatusCode status)
{
  auto* session = sessionOf(client);
  if (!session)
    return;

  QMetaObject::invokeMethod(
    session,
    [session, channelState, sessionState, status] {
      session->handleStateChanged(
        static_cast<int>(channelState), static_cast<int>(sessionState), status);
    },
    Qt::DirectConnection);
}

/**
 * @brief The stack's server-certificate check, answered by the session's trust store.
 */
static UA_StatusCode verifyTrampoline(UA_CertificateGroup* group, const UA_ByteString* certificate)
{
  auto* session = s_verifiers.value(group, nullptr);
  if (!session || !certificate || !certificate->data)
    return UA_STATUSCODE_BADCERTIFICATEINVALID;

  return session->verifyServerCertificate(QByteArray(
    reinterpret_cast<const char*>(certificate->data), static_cast<qsizetype>(certificate->length)));
}

/**
 * @brief Creates the client and installs the configuration for @p endpoint plus our callbacks.
 *        A None-policy endpoint takes the plain default; anything signed or encrypted needs the
 *        installation's own certificate, which is generated here on first secure use.
 */
bool IO::Drivers::OpcUaSession::ensureClient(const OpcUaTypes::Endpoint& endpoint,
                                             const Identity& identity)
{
  teardown();

  m_client = UA_Client_new();
  if (!m_client)
    return false;

  UA_ClientConfig* config = UA_Client_getConfig(m_client);
  if (!config) {
    teardown();
    return false;
  }

  if (!applySecurity(endpoint) || !applyIdentity(identity)) {
    teardown();
    return false;
  }

  config->clientContext = this;
  config->stateCallback = stateTrampoline;
  config->timeout       = static_cast<UA_UInt32>(kSessionRequestTimeoutMs);
  return true;
}

/**
 * @brief Installs the security policy, mode, client certificate and verification hook. The hook
 *        REPLACES open62541's trust-list check rather than adding to it: our trust store is the
 *        single authority, and a user's accept must be able to override a chain that no CA signs.
 */
bool IO::Drivers::OpcUaSession::applySecurity(const OpcUaTypes::Endpoint& endpoint)
{
  SS_ASSERT(m_client != nullptr, return false);
  UA_ClientConfig* config = UA_Client_getConfig(m_client);
  SS_ASSERT(config != nullptr, return false);

  m_securityMode      = endpoint.securityMode;
  m_securityPolicyUri = endpoint.securityPolicyUri;
  m_trustFailure      = OpcUaTypes::TrustFailure::None;
  m_serverCertificate = OpcUaTypes::CertInfo();

  const bool secure = endpoint.securityMode == OpcUaTypes::SecurityMode::Sign
                   || endpoint.securityMode == OpcUaTypes::SecurityMode::SignAndEncrypt;
  if (!secure)
    return UA_ClientConfig_setDefault(config) == UA_STATUSCODE_GOOD;

  QByteArray certificate;
  QByteArray privateKey;
  if (!OpcUaSecurity::ensureClientIdentity(certificate, privateKey))
    return false;

  UA_ByteString cert = {static_cast<size_t>(certificate.size()),
                        reinterpret_cast<UA_Byte*>(const_cast<char*>(certificate.constData()))};
  UA_ByteString key  = {static_cast<size_t>(privateKey.size()),
                        reinterpret_cast<UA_Byte*>(const_cast<char*>(privateKey.constData()))};

  if (UA_ClientConfig_setDefaultEncryption(config, cert, key, nullptr, 0, nullptr, 0)
      != UA_STATUSCODE_GOOD)
    return false;

  const auto uri = OpcUaSecurity::applicationUri().toUtf8();
  UA_String_clear(&config->clientDescription.applicationUri);
  config->clientDescription.applicationUri = UA_STRING_ALLOC(uri.constData());

  const auto policy = endpoint.securityPolicyUri.toUtf8();
  UA_String_clear(&config->securityPolicyUri);
  config->securityPolicyUri = UA_STRING_ALLOC(policy.constData());
  config->securityMode      = static_cast<UA_MessageSecurityMode>(endpoint.securityMode);

  s_verifiers.insert(&config->certificateVerification, this);
  config->certificateVerification.verifyCertificate = verifyTrampoline;
  return true;
}

/**
 * @brief Installs the user identity token: anonymous, username/password, or an X.509 certificate
 *        the user supplied. `allowNonePolicyPassword` lets a password travel over an unencrypted
 *        channel at all; open62541 refuses that by default, and the override is the user's own
 *        per-installation answer, so nothing goes in the clear until someone accepts that it will.
 */
bool IO::Drivers::OpcUaSession::applyIdentity(const Identity& identity)
{
  SS_ASSERT(m_client != nullptr, return false);
  UA_ClientConfig* config = UA_Client_getConfig(m_client);
  SS_ASSERT(config != nullptr, return false);

  if (identity.mode == 1) {
    config->allowNonePolicyPassword = identity.allowPlaintextPassword;
    applyUsernameIdentity(identity);
    return true;
  }

  if (identity.mode != 2)
    return true;

  const auto certificate = OpcUaSecurity::readCertificateFile(identity.certificatePath);
  QFile keyFile(identity.privateKeyPath);
  if (certificate.isEmpty() || !keyFile.open(QIODevice::ReadOnly))
    return false;

  const auto privateKey = keyFile.readAll();
  UA_ByteString cert    = {static_cast<size_t>(certificate.size()),
                           reinterpret_cast<UA_Byte*>(const_cast<char*>(certificate.constData()))};
  UA_ByteString key     = {static_cast<size_t>(privateKey.size()),
                           reinterpret_cast<UA_Byte*>(const_cast<char*>(privateKey.constData()))};

  return UA_ClientConfig_setAuthenticationCert(config, cert, key) == UA_STATUSCODE_GOOD;
}

/**
 * @brief Answers the stack's certificate check from the installation's trust store, recording WHY
 *        a certificate was refused: the four causes have four different fixes. TRUST is read
 *        FIRST, because it pins these exact bytes by SHA-256; checking the name or the clock ahead
 *        of it made Trust a button that could not accept a self-signed server dialed by IP.
 */
IO::Drivers::OpcUaTypes::StatusCode IO::Drivers::OpcUaSession::verifyServerCertificate(
  const QByteArray& certificate)
{
  const auto host     = QUrl(m_endpointUrl).host();
  m_serverCertificate = OpcUaSecurity::inspect(certificate, host);

  if (!m_serverCertificate.valid) {
    m_trustFailure = OpcUaTypes::TrustFailure::Unreadable;
    return UA_STATUSCODE_BADCERTIFICATEINVALID;
  }

  if (m_serverCertificate.trusted) {
    m_trustFailure = OpcUaTypes::TrustFailure::None;
    return UA_STATUSCODE_GOOD;
  }

  if (m_serverCertificate.expired) {
    m_trustFailure = OpcUaTypes::TrustFailure::Expired;
    return UA_STATUSCODE_BADCERTIFICATETIMEINVALID;
  }

  if (m_serverCertificate.notYetValid) {
    m_trustFailure = OpcUaTypes::TrustFailure::NotYetValid;
    return UA_STATUSCODE_BADCERTIFICATETIMEINVALID;
  }

  if (!m_serverCertificate.hostnameMatches) {
    m_trustFailure = OpcUaTypes::TrustFailure::HostnameMismatch;
    return UA_STATUSCODE_BADCERTIFICATEHOSTNAMEINVALID;
  }

  m_trustFailure = OpcUaTypes::TrustFailure::Untrusted;
  return UA_STATUSCODE_BADCERTIFICATEUNTRUSTED;
}

/**
 * @brief The security policy URI of the current or last attempted channel.
 */
QString IO::Drivers::OpcUaSession::securityPolicyUri() const
{
  return m_securityPolicyUri;
}

/**
 * @brief The message security mode of the current or last attempted channel.
 */
IO::Drivers::OpcUaTypes::SecurityMode IO::Drivers::OpcUaSession::securityMode() const noexcept
{
  return m_securityMode;
}

/**
 * @brief What the server presented on the last attempt, for the trust prompt.
 */
IO::Drivers::OpcUaTypes::CertInfo IO::Drivers::OpcUaSession::serverCertificate() const
{
  return m_serverCertificate;
}

/**
 * @brief Why the last attempt's certificate was refused, or None.
 */
IO::Drivers::OpcUaTypes::TrustFailure IO::Drivers::OpcUaSession::trustFailure() const noexcept
{
  return m_trustFailure;
}

//--------------------------------------------------------------------------------------------------
// Dial and discovery
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a failed attempt exactly once and drops the client. The driver owns the verdict
 *        contract; this is the single place the session can say "the dial did not happen", so a
 *        second state transition for the same attempt stays silent.
 */
void IO::Drivers::OpcUaSession::failDial(const QString& reason)
{
  if (!m_connecting)
    return;

  m_connecting = false;
  m_intent     = Intent::Idle;
  m_lastReason = reason;
  close();

  Q_EMIT connectFailed(reason);
}

/**
 * @brief Translates the stack's channel and session transitions into the three outcomes the
 *        driver understands: endpoints ready to fetch, session up, or attempt over. Only a BAD
 *        status is a verdict, since open62541 reopens the channel mid-handshake on purpose; a bad
 *        one under Discovering ends the discovery, which nothing else ends before the deadline.
 */
void IO::Drivers::OpcUaSession::handleStateChanged(int channelState,
                                                   int sessionState,
                                                   OpcUaTypes::StatusCode status)
{
  if (m_intent == Intent::Discovering && channelState == UA_SECURECHANNELSTATE_OPEN) {
    requestEndpoints();
    return;
  }

  if (m_intent == Intent::Discovering && channelState == UA_SECURECHANNELSTATE_CLOSED
      && !isGood(status)) {
    handleEndpoints({}, status);
    return;
  }

  if (sessionState == UA_SESSIONSTATE_ACTIVATED && !m_open) {
    m_open       = true;
    m_connecting = false;
    m_intent     = Intent::Idle;
    requestServerLimits();
    requestNamespaceArray();
    Q_EMIT connected();
    return;
  }

  if (channelState != UA_SECURECHANNELSTATE_CLOSED)
    return;

  if (m_connecting) {
    if (isGood(status))
      return;

    failDial(OpcUaMarshal::statusText(status));
    return;
  }

  if (m_open) {
    m_open = false;
    Q_EMIT disconnected();
  }
}

/**
 * @brief Opens a bare secure channel and asks for the server's endpoints over it. A channel needs
 *        no session, which is what lets discovery run before any identity is chosen.
 */
bool IO::Drivers::OpcUaSession::discoverEndpoints(const QString& url)
{
  OpcUaTypes::Endpoint bare;
  bare.endpointUrl  = url;
  bare.securityMode = OpcUaTypes::SecurityMode::None;
  if (!ensureClient(bare, Identity()))
    return false;

  m_endpointUrl = url;
  m_intent      = Intent::Discovering;

  startResolution();
  return true;
}

/**
 * @brief Resolves the endpoint's host before the stack is asked to dial it. open62541 resolves
 *        inside UA_Client_connectAsync() with a synchronous getaddrinfo ("TODO: Make this
 *        non-blocking" upstream), so an unresolvable host froze the window for the resolver's own
 *        timeout, which no dial deadline of ours could shorten.
 */
void IO::Drivers::OpcUaSession::startResolution()
{
  const QUrl parsed(m_endpointUrl);
  const auto port = static_cast<quint16>(parsed.port(OpcUaTypes::kDefaultPort));

  m_resolver.setDeadline(kSessionResolveDeadlineMs);
  m_resolver.startResolve(parsed.host(), port);
}

/**
 * @brief The URL handed to the stack: the resolved literal with the configured port, so the dial
 *        runs no resolver of its own. m_endpointUrl keeps the hostname the user typed, which is
 *        what the certificate hostname check has to see.
 */
QString IO::Drivers::OpcUaSession::dialUrl() const
{
  const auto address = m_resolver.resolvedAddress();
  if (address.isNull())
    return m_endpointUrl;

  QUrl numeric(m_endpointUrl);
  numeric.setHost(address.toString());
  if (numeric.port() < 0)
    numeric.setPort(OpcUaTypes::kDefaultPort);

  return numeric.toString();
}

/**
 * @brief Continues the attempt once the host resolved, or ends it when it did not. The two
 *        intents settle through their own funnels: a discovery through handleEndpoints(), a live
 *        dial through failDial(), so the verdict still has exactly one owner.
 */
void IO::Drivers::OpcUaSession::onResolveFinished(bool ok, const QString& reason)
{
  if (m_intent == Intent::Discovering) {
    if (!ok || !m_client) {
      m_intent     = Intent::Idle;
      m_lastReason = reason;
      handleEndpoints({}, OpcUaTypes::kStatusBadInternal);
      return;
    }

    const auto utf8       = dialUrl().toUtf8();
    UA_StatusCode dialing = UA_STATUSCODE_GOOD;
    {
      const ClientCallScope scope(m_stackDepth);
      dialing = UA_Client_connectSecureChannelAsync(m_client, utf8.constData());
    }

    if (dialing != UA_STATUSCODE_GOOD) {
      m_intent = Intent::Idle;
      handleEndpoints({}, OpcUaTypes::kStatusBadInternal);
      return;
    }

    startPump();
    return;
  }

  if (m_intent != Intent::Connecting)
    return;

  if (!ok || !m_client) {
    failDial(reason.isEmpty() ? tr("Host not found") : reason);
    return;
  }

  const auto utf8      = dialUrl().toUtf8();
  UA_StatusCode status = UA_STATUSCODE_GOOD;
  {
    const ClientCallScope scope(m_stackDepth);
    status = UA_Client_connectAsync(m_client, utf8.constData());
  }

  if (status != UA_STATUSCODE_GOOD) {
    failDial(OpcUaMarshal::statusText(status));
    return;
  }

  startPump();
}

/**
 * @brief The GetEndpoints reply, converted into the driver's endpoint rows.
 */
static void endpointsTrampoline(UA_Client* client, void* userdata, UA_UInt32, void* response)
{
  Q_UNUSED(userdata)
  auto* session = sessionOf(client);
  if (!session || !response)
    return;

  auto* reply = static_cast<UA_GetEndpointsResponse*>(response);
  QList<Endpoint> endpoints;
  endpoints.reserve(static_cast<qsizetype>(reply->endpointsSize));

  for (size_t i = 0; i < reply->endpointsSize; ++i) {
    const auto& source = reply->endpoints[i];

    Endpoint endpoint;
    endpoint.endpointUrl       = IO::Drivers::OpcUaMarshal::toQString(source.endpointUrl);
    endpoint.securityPolicyUri = IO::Drivers::OpcUaMarshal::toQString(source.securityPolicyUri);
    endpoint.applicationUri    = IO::Drivers::OpcUaMarshal::toQString(source.server.applicationUri);
    endpoint.securityMode =
      IO::Drivers::OpcUaMarshal::toSecurityMode(static_cast<int>(source.securityMode));
    endpoint.securityLevel = source.securityLevel;

    for (size_t t = 0; t < source.userIdentityTokensSize; ++t)
      endpoint.userTokenTypes.append(IO::Drivers::OpcUaMarshal::toUserTokenType(
        static_cast<int>(source.userIdentityTokens[t].tokenType)));

    endpoints.append(endpoint);
  }

  session->handleEndpoints(endpoints, reply->responseHeader.serviceResult);
}

/**
 * @brief Issues the GetEndpoints service on the open channel.
 */
void IO::Drivers::OpcUaSession::requestEndpoints()
{
  SS_ASSERT(m_client != nullptr, return);
  m_intent = Intent::Idle;

  const auto utf8 = m_endpointUrl.toUtf8();

  UA_GetEndpointsRequest request;
  UA_GetEndpointsRequest_init(&request);
  request.endpointUrl.length = static_cast<size_t>(utf8.size());
  request.endpointUrl.data   = reinterpret_cast<UA_Byte*>(const_cast<char*>(utf8.constData()));

  const auto status = __UA_Client_AsyncService(m_client,
                                               &request,
                                               &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST],
                                               endpointsTrampoline,
                                               &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE],
                                               this,
                                               nullptr);
  if (status != UA_STATUSCODE_GOOD)
    handleEndpoints({}, status);
}

/**
 * @brief Publishes the discovery result and retires the channel it was fetched over.
 */
void IO::Drivers::OpcUaSession::handleEndpoints(const QList<OpcUaTypes::Endpoint>& endpoints,
                                                OpcUaTypes::StatusCode status)
{
  Q_EMIT endpointsReady(endpoints, status);
  close();
}

/**
 * @brief Dials the endpoint with the chosen identity. The verdict arrives through the state
 *        callback, never from this return value: false here means the attempt never started.
 */
bool IO::Drivers::OpcUaSession::connectToEndpoint(const OpcUaTypes::Endpoint& endpoint,
                                                  const Identity& identity)
{
  m_endpointUrl = endpoint.endpointUrl;
  if (!ensureClient(endpoint, identity))
    return false;

  m_intent     = Intent::Connecting;
  m_connecting = true;

  startResolution();
  return true;
}

/**
 * @brief Installs a username token on the config so the dial can stay asynchronous.
 *        UA_Client_connectUsername() exists but is a wrapper that sets this same token and then
 *        calls the BLOCKING connect, which would freeze the GUI for the length of the handshake
 *        (and for the full timeout against an unreachable host).
 */
void IO::Drivers::OpcUaSession::applyUsernameIdentity(const Identity& identity)
{
  SS_ASSERT(m_client != nullptr, return);

  UA_ClientConfig* config = UA_Client_getConfig(m_client);
  SS_ASSERT(config != nullptr, return);

  auto* token = UA_UserNameIdentityToken_new();
  if (!token)
    return;

  UA_UserNameIdentityToken_init(token);
  token->userName = UA_STRING_ALLOC(identity.username.toUtf8().constData());
  token->password = UA_STRING_ALLOC(identity.password.toUtf8().constData());

  UA_ExtensionObject_clear(&config->userIdentityToken);
  UA_ExtensionObject_setValue(
    &config->userIdentityToken, token, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]);
}

//--------------------------------------------------------------------------------------------------
// Subscription
//--------------------------------------------------------------------------------------------------

/**
 * @brief The CreateSubscription reply, forwarded with the interval the server granted.
 */
static void subscriptionTrampoline(UA_Client* client,
                                   void* userdata,
                                   UA_UInt32,
                                   UA_CreateSubscriptionResponse* reply)
{
  Q_UNUSED(userdata)
  auto* session = sessionOf(client);
  if (!session || !reply)
    return;

  session->handleSubscriptionCreated(reply->subscriptionId,
                                     static_cast<int>(reply->revisedPublishingInterval),
                                     reply->responseHeader.serviceResult);
}

/**
 * @brief The CreateMonitoredItems reply. The per-item status array is the whole point of the
 *        batch: a server that refuses one tag must not cost the other tags their subscription.
 */
static void monitoredItemsTrampoline(UA_Client* client,
                                     void* userdata,
                                     UA_UInt32,
                                     UA_CreateMonitoredItemsResponse* reply)
{
  Q_UNUSED(userdata)
  auto* session = sessionOf(client);
  if (!session || !reply)
    return;

  QList<StatusCode> perItemStatus;
  perItemStatus.reserve(static_cast<qsizetype>(reply->resultsSize));
  for (size_t i = 0; i < reply->resultsSize; ++i)
    perItemStatus.append(reply->results[i].statusCode);

  session->handleSubscribed(perItemStatus, reply->responseHeader.serviceResult);
}

/**
 * @brief A subscription the server dropped on its own (keep-alive expiry, session timeout). The
 *        driver treats this as a reason to fall back to timed reads, not as a link drop.
 */
static void subscriptionLostTrampoline(UA_Client* client,
                                       UA_UInt32,
                                       void*,
                                       UA_StatusChangeNotification* notification)
{
  auto* session = sessionOf(client);
  if (!session || !notification)
    return;

  session->handleSubscriptionLost(IO::Drivers::OpcUaMarshal::statusText(notification->status));
}

/**
 * @brief One data-change notification. `monContext` points into m_monitorTags, so the tag index
 *        is known from the first notification onwards: open62541 registers monitored items
 *        locally BEFORE the CreateMonitoredItems reply arrives, and a map keyed on the returned
 *        monitored-item id would drop the initial value of every slow-changing tag.
 */
static void valueTrampoline(
  UA_Client* client, UA_UInt32, void*, UA_UInt32, void* monContext, UA_DataValue* value)
{
  auto* session = sessionOf(client);
  if (!session || !monContext || !value)
    return;

  using namespace IO::Drivers::OpcUaMarshal;

  MonitoredValue notification;
  notification.tag             = *static_cast<const int*>(monContext);
  notification.status          = value->hasStatus ? value->status : kStatusGood;
  notification.sourceTimestamp = sourceTimeOf(*value);
  if (value->hasServerTimestamp)
    notification.serverTimestamp = toDateTime(value->serverTimestamp);

  if (value->hasValue)
    notification.value = toVariant(value->value);

  session->handleValue(notification);
}

/**
 * @brief Creates ONE subscription for every tag. The monitored items follow in a single batch
 *        once the server has granted the subscription, because open62541 refuses to register an
 *        item against a subscription id it does not know yet.
 */
bool IO::Drivers::OpcUaSession::subscribe(const QStringList& nodeIds, int publishingIntervalMs)
{
  SS_ASSERT(!nodeIds.isEmpty(), return false);
  if (!m_client || !m_open)
    return false;

  m_monitoredNodes  = nodeIds;
  m_subscriptionId  = 0;
  m_revisedInterval = qBound(kSessionMinIntervalMs, publishingIntervalMs, kSessionMaxIntervalMs);

  m_monitorTags.resize(nodeIds.size());
  for (qsizetype i = 0; i < m_monitorTags.size(); ++i)
    m_monitorTags[i] = static_cast<int>(i);

  auto request                        = UA_CreateSubscriptionRequest_default();
  request.requestedPublishingInterval = static_cast<UA_Double>(m_revisedInterval);
  request.requestedLifetimeCount      = kSessionLifetimeCount;
  request.requestedMaxKeepAliveCount  = kSessionKeepAliveCount;

  const auto status = UA_Client_Subscriptions_create_async(m_client,
                                                           request,
                                                           this,
                                                           subscriptionLostTrampoline,
                                                           nullptr,
                                                           subscriptionTrampoline,
                                                           this,
                                                           nullptr);
  if (status != UA_STATUSCODE_GOOD) {
    handleSubscribed({}, status);
    return false;
  }

  return true;
}

/**
 * @brief Adopts the granted subscription and issues the monitored items, or reports the refusal
 *        against every tag when the server declined the subscription itself.
 */
void IO::Drivers::OpcUaSession::handleSubscriptionCreated(quint32 subscriptionId,
                                                          int revisedIntervalMs,
                                                          OpcUaTypes::StatusCode status)
{
  if (!isGood(status)) {
    handleSubscribed({}, status);
    return;
  }

  m_subscriptionId = subscriptionId;
  if (revisedIntervalMs > 0)
    m_revisedInterval = qBound(kSessionMinIntervalMs, revisedIntervalMs, kSessionMaxIntervalMs);

  createMonitoredItems();
}

/**
 * @brief Sends every tag as one CreateMonitoredItems request. A node id that does not parse is
 *        still sent, as a null node id: the server refuses THAT item and the reply stays aligned
 *        with the tag order, which is what keeps a refusal individual.
 */
void IO::Drivers::OpcUaSession::createMonitoredItems()
{
  SS_ASSERT(m_client != nullptr, return);
  SS_ASSERT(!m_monitoredNodes.isEmpty(), return);

  const auto count = static_cast<size_t>(m_monitoredNodes.size());
  std::vector<UA_MonitoredItemCreateRequest> items(count);
  std::vector<void*> contexts(count, nullptr);
  std::vector<UA_Client_DataChangeNotificationCallback> callbacks(count, valueTrampoline);

  for (size_t i = 0; i < count; ++i) {
    const auto index = static_cast<qsizetype>(i);
    UA_NodeId nodeId;
    const bool parsed = OpcUaMarshal::nodeIdFromString(m_monitoredNodes.at(index), nodeId);
    SS_ASSERT_LOG(parsed);

    items[i]                                      = UA_MonitoredItemCreateRequest_default(nodeId);
    items[i].requestedParameters.samplingInterval = static_cast<UA_Double>(m_revisedInterval);
    contexts[i]                                   = &m_monitorTags[index];
  }

  UA_CreateMonitoredItemsRequest request;
  UA_CreateMonitoredItemsRequest_init(&request);
  request.subscriptionId     = m_subscriptionId;
  request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
  request.itemsToCreate      = items.data();
  request.itemsToCreateSize  = count;

  const auto status = UA_Client_MonitoredItems_createDataChanges_async(m_client,
                                                                       request,
                                                                       contexts.data(),
                                                                       callbacks.data(),
                                                                       nullptr,
                                                                       monitoredItemsTrampoline,
                                                                       this,
                                                                       nullptr);
  for (auto& item : items)
    UA_MonitoredItemCreateRequest_clear(&item);

  if (status != UA_STATUSCODE_GOOD)
    handleSubscribed({}, status);
}

/**
 * @brief Publishes the batch verdict, one status per tag in tag order. A reply that carries no
 *        results at all is padded with the service-level status so the driver still sees a
 *        verdict for every tag it asked for, rather than a short list it would mis-index.
 */
void IO::Drivers::OpcUaSession::handleSubscribed(const QList<OpcUaTypes::StatusCode>& perItemStatus,
                                                 OpcUaTypes::StatusCode serviceStatus)
{
  auto rows               = perItemStatus;
  const qsizetype missing = m_monitoredNodes.size() - rows.size();
  if (missing > 0)
    rows.append(QList<StatusCode>(missing, serviceStatus));

  Q_EMIT subscribed(rows);
}

/**
 * @brief The server retired the subscription; the driver decides whether to poll instead.
 */
void IO::Drivers::OpcUaSession::handleSubscriptionLost(const QString& reason)
{
  m_subscriptionId = 0;
  Q_EMIT subscriptionLost(reason);
}

/**
 * @brief Forwards one monitored value. Nothing is stamped here: the timestamp on it is the
 *        server's own and travels unmodified to the driver's slot cache.
 */
void IO::Drivers::OpcUaSession::handleValue(const OpcUaTypes::MonitoredValue& value)
{
  Q_EMIT valueChanged(value);
}

//--------------------------------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------------------------------

/**
 * @brief The Read reply. Results are positional: the service answers in request order and carries
 *        no node id, so the staged rows are what says which value belongs to which node.
 */
static void readTrampoline(UA_Client* client,
                           void* userdata,
                           UA_UInt32 requestId,
                           UA_ReadResponse* reply)
{
  Q_UNUSED(userdata)
  auto* session = sessionOf(client);
  if (!session || !reply)
    return;

  using namespace IO::Drivers::OpcUaMarshal;

  QList<ReadRow> rows;
  rows.reserve(static_cast<qsizetype>(reply->resultsSize));

  for (size_t i = 0; i < reply->resultsSize; ++i) {
    const auto& result = reply->results[i];

    ReadRow row;
    row.status          = result.hasStatus ? result.status : kStatusGood;
    row.sourceTimestamp = sourceTimeOf(result);
    if (result.hasServerTimestamp)
      row.serverTimestamp = toDateTime(result.serverTimestamp);

    if (result.hasValue)
      row.value = toVariant(result.value);

    rows.append(row);
  }

  session->handleRead(requestId, rows, reply->responseHeader.serviceResult);
}

/**
 * @brief Reads the Value attribute of every node in one request, chunked to the server's
 *        MaxNodesPerRead. Exactly ONE value read is outstanding at a time, and a list longer
 *        than the limit is served through a ROTATING window: a fixed leading slice would starve
 *        the tail forever, since the caller passes the same order every tick.
 */
bool IO::Drivers::OpcUaSession::readValues(const QStringList& nodeIds)
{
  SS_ASSERT(!nodeIds.isEmpty(), return false);
  if (!m_client || !m_open || m_readInFlight)
    return false;

  const auto total = nodeIds.size();
  const auto span  = qMin<qsizetype>(total, m_readLimit);
  if (m_pollCursor >= total)
    m_pollCursor = 0;

  QList<ReadRow> rows;
  rows.reserve(span);
  for (qsizetype i = 0; i < span; ++i) {
    ReadRow row;
    row.nodeId    = nodeIds.at((m_pollCursor + i) % total);
    row.attribute = NodeAttribute::Value;
    rows.append(row);
  }

  m_pollCursor   = (m_pollCursor + span) % total;
  m_readInFlight = sendRead(rows, InternalRead::No, true, 0);
  return m_readInFlight;
}

/**
 * @brief Reads a set of attributes for a set of nodes in one request; the reply keeps the node
 *        and attribute each value belongs to. Not gated on the value-read slot: this is the tag
 *        browser's per-level read, which is bounded by the level and not repeated on a timer.
 */
bool IO::Drivers::OpcUaSession::readAttributes(const QStringList& nodeIds,
                                               const QList<OpcUaTypes::NodeAttribute>& attributes,
                                               quint32 token)
{
  SS_ASSERT(!nodeIds.isEmpty(), return false);
  SS_ASSERT(!attributes.isEmpty(), return false);
  if (!m_client || !m_open)
    return false;

  QList<ReadRow> rows;
  rows.reserve(nodeIds.size() * attributes.size());
  for (const auto& nodeId : nodeIds)
    for (const auto attribute : attributes) {
      ReadRow row;
      row.nodeId    = nodeId;
      row.attribute = attribute;
      rows.append(row);
    }

  return sendRead(rows, InternalRead::No, false, token);
}

/**
 * @brief Encodes and sends one Read request for already-staged rows.
 */
bool IO::Drivers::OpcUaSession::sendRead(const QList<OpcUaTypes::ReadRow>& rows,
                                         InternalRead internalRead,
                                         bool valueRead,
                                         quint32 token)
{
  SS_ASSERT(m_client != nullptr, return false);
  SS_ASSERT(!rows.isEmpty(), return false);

  const auto count = static_cast<size_t>(rows.size());
  std::vector<UA_ReadValueId> items(count);
  for (size_t i = 0; i < count; ++i) {
    const auto& row = rows.at(static_cast<qsizetype>(i));
    UA_ReadValueId_init(&items[i]);
    const bool parsed = OpcUaMarshal::nodeIdFromString(row.nodeId, items[i].nodeId);
    SS_ASSERT_LOG(parsed);
    items[i].attributeId = static_cast<UA_UInt32>(row.attribute);
  }

  UA_ReadRequest request;
  UA_ReadRequest_init(&request);
  request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
  request.nodesToRead        = items.data();
  request.nodesToReadSize    = count;

  UA_UInt32 requestId = 0;
  const auto status =
    UA_Client_sendAsyncReadRequest(m_client, &request, readTrampoline, this, &requestId);

  for (auto& item : items)
    UA_ReadValueId_clear(&item);

  if (status != UA_STATUSCODE_GOOD)
    return false;

  PendingRead pending;
  pending.rows         = rows;
  pending.internalRead = internalRead;
  pending.valueRead    = valueRead;
  pending.token        = token;
  m_readRequests.insert(requestId, pending);
  applyPumpCadence();
  return true;
}

/**
 * @brief Merges the positional reply back onto the staged rows and publishes them. A request the
 *        registry no longer knows was cancelled by a teardown and is dropped.
 */
void IO::Drivers::OpcUaSession::handleRead(quint32 requestId,
                                           const QList<OpcUaTypes::ReadRow>& rows,
                                           OpcUaTypes::StatusCode status)
{
  auto pending = m_readRequests.take(requestId);
  if (pending.rows.isEmpty())
    return;

  auto merged = std::move(pending.rows);
  for (qsizetype i = 0; i < merged.size() && i < rows.size(); ++i) {
    merged[i].value           = rows.at(i).value;
    merged[i].status          = rows.at(i).status;
    merged[i].sourceTimestamp = rows.at(i).sourceTimestamp;
    merged[i].serverTimestamp = rows.at(i).serverTimestamp;
  }

  if (pending.internalRead == InternalRead::No) {
    if (pending.valueRead)
      m_readInFlight = false;

    Q_EMIT readFinished(pending.token, merged, status);
    return;
  }

  if (pending.internalRead == InternalRead::NamespaceArray) {
    m_namespaceArray.clear();
    const auto entries = merged.first().value.toList();
    for (const auto& entry : entries)
      m_namespaceArray.append(entry.toString());

    return;
  }

  const int limit = merged.first().value.toInt();
  if (limit > 0)
    m_readLimit = qBound(1, limit, kSessionDefaultReadLimit * 10);
}

/**
 * @brief Asks the server for its MaxNodesPerRead once per session. A batched poll larger than
 *        that limit is rejected wholesale with Bad_TooManyOperations, so the default stands only
 *        until the server says otherwise.
 */
void IO::Drivers::OpcUaSession::requestServerLimits()
{
  SS_ASSERT(m_client != nullptr, return);

  ReadRow row;
  row.nodeId      = QString::fromLatin1(kNodeMaxNodesPerRead);
  row.attribute   = NodeAttribute::Value;
  const bool sent = sendRead({row}, InternalRead::ReadLimit, false, 0);
  SS_ASSERT_LOG(sent);
}

/**
 * @brief Asks the server for its namespace table once per session. An aggregating server answers
 *        a browse with references that name a namespace URI rather than an index, and this table
 *        is what turns those into a node id that means the same thing on the next connect.
 */
void IO::Drivers::OpcUaSession::requestNamespaceArray()
{
  SS_ASSERT(m_client != nullptr, return);

  ReadRow row;
  row.nodeId    = QString::fromLatin1(kNodeNamespaceArray);
  row.attribute = NodeAttribute::Value;

  const bool sent = sendRead({row}, InternalRead::NamespaceArray, false, 0);
  SS_ASSERT_LOG(sent);
}

/**
 * @brief The server's namespace table, empty until the probe lands.
 */
QStringList IO::Drivers::OpcUaSession::namespaceArray() const
{
  return m_namespaceArray;
}

//--------------------------------------------------------------------------------------------------
// Browse
//--------------------------------------------------------------------------------------------------

/**
 * @brief The Browse reply, converted into the picker's reference rows.
 */
static void browseTrampoline(UA_Client* client,
                             void* userdata,
                             UA_UInt32 requestId,
                             UA_BrowseResponse* reply)
{
  Q_UNUSED(userdata)
  auto* session = sessionOf(client);
  if (!session || !reply)
    return;

  using namespace IO::Drivers::OpcUaMarshal;

  auto status = reply->responseHeader.serviceResult;

  QList<ReferenceRow> children;
  for (size_t r = 0; r < reply->resultsSize; ++r) {
    const auto& result = reply->results[r];
    if (!isGood(result.statusCode))
      status = result.statusCode;

    for (size_t i = 0; i < result.referencesSize; ++i) {
      const auto& reference = result.references[i];

      ReferenceRow row;
      row.nodeId       = nodeIdToString(reference.nodeId.nodeId);
      row.namespaceUri = toQString(reference.nodeId.namespaceUri);
      row.browseName   = toQString(reference.browseName.name);
      row.displayName  = toQString(reference.displayName.text);
      row.nodeClass    = toNodeClass(static_cast<quint32>(reference.nodeClass));
      children.append(row);
    }
  }

  session->handleBrowse(requestId, children, status);
}

/**
 * @brief Browses ONE node. The reply carries the caller's token back, because the picker browses
 *        the same node for up to three different reasons (level expansion, the has-children
 *        probe, the units lookup) and the node id alone cannot say which reply this is.
 */
bool IO::Drivers::OpcUaSession::browse(const QString& nodeId, const OpcUaTypes::BrowseQuery& query)
{
  SS_ASSERT(!nodeId.isEmpty(), return false);
  if (!m_client || !m_open)
    return false;

  UA_BrowseDescription description;
  UA_BrowseDescription_init(&description);
  const bool parsed = OpcUaMarshal::nodeIdFromString(nodeId, description.nodeId);
  SS_ASSERT_LOG(parsed);

  description.browseDirection = UA_BROWSEDIRECTION_FORWARD;
  description.includeSubtypes = true;
  description.nodeClassMask   = static_cast<UA_UInt32>(query.nodeClassMask);
  description.resultMask      = UA_BROWSERESULTMASK_ALL;
  description.referenceTypeId = query.kind == OpcUaTypes::ReferenceKind::HasProperty
                                ? UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY)
                                : UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES);

  UA_BrowseRequest request;
  UA_BrowseRequest_init(&request);
  request.requestedMaxReferencesPerNode = kSessionMaxRefsPerNode;
  request.nodesToBrowse                 = &description;
  request.nodesToBrowseSize             = 1;

  UA_UInt32 requestId = 0;
  const auto status =
    UA_Client_sendAsyncBrowseRequest(m_client, &request, browseTrampoline, this, &requestId);

  UA_BrowseDescription_clear(&description);
  if (status != UA_STATUSCODE_GOOD)
    return false;

  PendingBrowse pending;
  pending.nodeId = nodeId;
  pending.token  = query.token;
  m_browseRequests.insert(requestId, pending);
  applyPumpCadence();
  return true;
}

/**
 * @brief Publishes a browse reply against the node and token that asked for it.
 */
void IO::Drivers::OpcUaSession::handleBrowse(quint32 requestId,
                                             const QList<OpcUaTypes::ReferenceRow>& children,
                                             OpcUaTypes::StatusCode status)
{
  const auto pending = m_browseRequests.take(requestId);
  if (pending.nodeId.isEmpty())
    return;

  Q_EMIT browseFinished(pending.token, pending.nodeId, children, status);
}

/**
 * @brief The stack's own text for a status code, so the driver can name a refusal without
 *        including open62541 itself.
 */
QString IO::Drivers::OpcUaSession::describeStatus(OpcUaTypes::StatusCode status)
{
  return OpcUaMarshal::statusText(status);
}

/**
 * @brief Asks the server to revise the subscription's publishing interval in place; a rate the
 *        user changes mid-session must not cost the subscription and its monitored items.
 */
bool IO::Drivers::OpcUaSession::modifyPublishingInterval(int intervalMs)
{
  if (!m_client || !m_open || m_subscriptionId == 0)
    return false;

  m_revisedInterval = qBound(kSessionMinIntervalMs, intervalMs, kSessionMaxIntervalMs);

  UA_ModifySubscriptionRequest request;
  UA_ModifySubscriptionRequest_init(&request);
  request.subscriptionId              = m_subscriptionId;
  request.requestedPublishingInterval = static_cast<UA_Double>(m_revisedInterval);
  request.requestedLifetimeCount      = kSessionLifetimeCount;
  request.requestedMaxKeepAliveCount  = kSessionKeepAliveCount;

  return UA_Client_Subscriptions_modify_async(m_client, request, nullptr, nullptr, nullptr)
      == UA_STATUSCODE_GOOD;
}
