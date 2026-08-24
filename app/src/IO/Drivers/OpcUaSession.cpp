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

#include "IO/Drivers/OpcUaMarshal.h"
#include "SSAssert.h"

using namespace IO::Drivers::OpcUaTypes;

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr int kPumpIntervalMs     = 10;
static constexpr int kRequestTimeoutMs   = 10000;
static constexpr int kDefaultReadLimit   = 200;
static constexpr quint32 kMaxRefsPerNode = 0;

/**
 * @brief True while a UA_Client_run_iterate() call is on the stack. open62541 invokes its C
 *        callbacks from inside iterate, and those callbacks emit signals the driver reacts to,
 *        so a close() can arrive while the client is still executing. Deleting it there frees the
 *        object the stack is standing on.
 */
static bool s_inPump = false;

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
  , m_intent(Intent::Idle)
  , m_readLimit(kDefaultReadLimit)
  , m_subscriptionId(0)
  , m_pump(new QTimer(this))
  , m_client(nullptr)
{
  m_pump->setInterval(kPumpIntervalMs);
  m_pump->setTimerType(Qt::CoarseTimer);
  connect(m_pump, &QTimer::timeout, this, &IO::Drivers::OpcUaSession::pump);
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
  }

  UA_Client_delete(client);

  m_open           = false;
  m_connecting     = false;
  m_intent         = Intent::Idle;
  m_readInFlight   = false;
  m_subscriptionId = 0;
  m_monitoredNodes.clear();
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

  if (s_inPump) {
    m_open       = false;
    m_connecting = false;
    m_intent     = Intent::Idle;
    QMetaObject::invokeMethod(this, [this] { teardown(); }, Qt::QueuedConnection);
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

  s_inPump          = true;
  const auto status = UA_Client_run_iterate(m_client, 0);
  s_inPump          = false;

  if (status == UA_STATUSCODE_GOOD || !m_open)
    return;

  m_open = false;
  Q_EMIT disconnected();
}

/**
 * @brief Arms the pump; harmless to call when it is already running.
 */
void IO::Drivers::OpcUaSession::startPump()
{
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
 * @brief Creates the client and installs the default configuration plus our state callback.
 */
bool IO::Drivers::OpcUaSession::ensureClient()
{
  teardown();

  m_client = UA_Client_new();
  if (!m_client)
    return false;

  UA_ClientConfig* config = UA_Client_getConfig(m_client);
  if (!config || UA_ClientConfig_setDefault(config) != UA_STATUSCODE_GOOD) {
    teardown();
    return false;
  }

  config->clientContext = this;
  config->stateCallback = stateTrampoline;
  config->timeout       = static_cast<UA_UInt32>(kRequestTimeoutMs);
  return true;
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
 *        driver understands: the endpoints are ready to fetch, the session is up, or the attempt
 *        is over. A channel that closes while connecting is a failure; one that closes after the
 *        session was established is a link drop.
 */
void IO::Drivers::OpcUaSession::handleStateChanged(int channelState,
                                                   int sessionState,
                                                   OpcUaTypes::StatusCode status)
{
  if (m_intent == Intent::Discovering && channelState == UA_SECURECHANNELSTATE_OPEN) {
    requestEndpoints();
    return;
  }

  if (sessionState == UA_SESSIONSTATE_ACTIVATED && !m_open) {
    m_open       = true;
    m_connecting = false;
    m_intent     = Intent::Idle;
    Q_EMIT connected();
    return;
  }

  if (channelState != UA_SECURECHANNELSTATE_CLOSED)
    return;

  if (m_connecting) {
    const auto reason =
      isGood(status) ? tr("The server closed the connection") : OpcUaMarshal::statusText(status);
    failDial(reason);
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
  if (!ensureClient())
    return false;

  m_endpointUrl = url;
  m_intent      = Intent::Discovering;

  const auto utf8 = url.toUtf8();
  if (UA_Client_connectSecureChannelAsync(m_client, utf8.constData()) != UA_STATUSCODE_GOOD) {
    m_intent = Intent::Idle;
    teardown();
    return false;
  }

  startPump();
  return true;
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
bool IO::Drivers::OpcUaSession::connectToEndpoint(const QString& url, const Identity& identity)
{
  if (!ensureClient())
    return false;

  m_endpointUrl = url;
  m_intent      = Intent::Connecting;
  m_connecting  = true;

  if (identity.mode == 1)
    applyUsernameIdentity(identity);

  const auto utf8   = url.toUtf8();
  const auto status = UA_Client_connectAsync(m_client, utf8.constData());
  if (status != UA_STATUSCODE_GOOD) {
    failDial(OpcUaMarshal::statusText(status));
    return false;
  }

  startPump();
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
