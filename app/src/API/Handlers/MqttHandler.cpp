/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/MqttHandler.h"

#  include <optional>
#  include <QJsonArray>
#  include <QJsonObject>

#  include "API/CommandRegistry.h"
#  include "API/SchemaBuilder.h"
#  include "IO/ConnectionManager.h"
#  include "IO/Drivers/MQTT.h"
#  include "MQTT/CredentialVault.h"
#  include "MQTT/Publisher.h"

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when port is within the valid [1, 65535] TCP range.
 */
[[nodiscard]] static bool portIsValid(int port)
{
  return port >= 1 && port <= 65535;
}

/**
 * @brief Validates that an integer falls within [0, modeCount).
 */
[[nodiscard]] static bool indexInRange(int v, int count)
{
  return v >= 0 && v < count;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers MQTT publisher and subscriber commands with the registry.
 */
void API::Handlers::MqttHandler::registerCommands()
{
  registerPublisherCommands();
  registerSubscriberCommands();
}

/**
 * @brief Registers publisher get/set/status commands with the registry.
 */
void API::Handlers::MqttHandler::registerPublisherCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.mqtt.publisher.getConfig"),
    QStringLiteral("Read the MQTT publisher configuration (broker, mode, TLS, etc.). "
                   "The password is NEVER returned -- inspect 'hasCredentials' instead. "
                   "Use 'modes', 'mqttVersions', 'sslProtocols', 'peerVerifyModes' as "
                   "the canonical enum tables when interpreting the integer fields."),
    API::emptySchema(),
    &publisherGetConfig);

  registry.registerCommand(
    QStringLiteral("project.mqtt.publisher.setConfig"),
    QStringLiteral("Patch one or more MQTT publisher fields. Pass only the keys you "
                   "want to change. Setting 'password' requires 'username' in the "
                   "same call -- the credential pair is persisted to the encrypted "
                   "vault, NOT to the project file. To clear credentials, pass empty "
                   "strings for both. Setting 'enabled:true' starts publishing."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("enabled"),
        QStringLiteral("boolean"),
        QStringLiteral("Start/stop publishing")},
       {QStringLiteral("hostname"), QStringLiteral("string"), QStringLiteral("Broker hostname")},
       {QStringLiteral("port"),
        QStringLiteral("integer"),
        QStringLiteral("Broker TCP port (1-65535)")},
       {QStringLiteral("clientId"), QStringLiteral("string"), QStringLiteral("MQTT client id")},
       {QStringLiteral("customClientId"),
        QStringLiteral("boolean"),
        QStringLiteral("If false, regenerate clientId automatically")},
       {QStringLiteral("username"),
        QStringLiteral("string"),
        QStringLiteral("Vault username (paired with password)")},
       {QStringLiteral("password"),
        QStringLiteral("string"),
        QStringLiteral("Vault password (requires username in same call)")},
       {QStringLiteral("topicBase"),
        QStringLiteral("string"),
        QStringLiteral("Publish topic root")},
       {QStringLiteral("notificationTopic"),
        QStringLiteral("string"),
        QStringLiteral("Topic for app notifications")},
       {QStringLiteral("publishNotifications"),
        QStringLiteral("boolean"),
        QStringLiteral("Forward app notifications to MQTT")},
       {QStringLiteral("mode"),
        QStringLiteral("integer"),
        QStringLiteral("0=RawRxData, 1=ScriptDriven, 2=DashboardCsv, 3=DashboardJson")},
       {QStringLiteral("publishFrequency"),
        QStringLiteral("integer"),
        QStringLiteral("Publish rate in Hz (1-30)")},
       {QStringLiteral("cleanSession"),
        QStringLiteral("boolean"),
        QStringLiteral("Use clean MQTT session")},
       {QStringLiteral("keepAlive"),
        QStringLiteral("integer"),
        QStringLiteral("Keepalive seconds (0-65535)")},
       {QStringLiteral("mqttVersion"),
        QStringLiteral("integer"),
        QStringLiteral("Protocol version index (see 'mqttVersions' from getConfig)")},
       {QStringLiteral("sslEnabled"), QStringLiteral("boolean"), QStringLiteral("Enable TLS")},
       {QStringLiteral("sslProtocol"),
        QStringLiteral("integer"),
        QStringLiteral("TLS protocol index (see 'sslProtocols' from getConfig)")},
       {QStringLiteral("peerVerifyMode"),
        QStringLiteral("integer"),
        QStringLiteral("Peer verify mode (see 'peerVerifyModes' from getConfig)")},
       {QStringLiteral("peerVerifyDepth"),
        QStringLiteral("integer"),
        QStringLiteral("Peer verify chain depth (0-65535)")}}),
    &publisherSetConfig);

  registry.registerCommand(
    QStringLiteral("project.mqtt.publisher.getStatus"),
    QStringLiteral("Snapshot publisher live state (connected, messagesSent, broker "
                   "endpoint). Does NOT trigger a new connection attempt."),
    API::emptySchema(),
    &publisherGetStatus);
}

/**
 * @brief Registers subscriber get/set/status commands with the registry.
 */
void API::Handlers::MqttHandler::registerSubscriberCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("project.mqtt.subscriber.getConfig"),
    QStringLiteral("Read the MQTT subscriber driver configuration (the UI driver -- "
                   "this matches what would be used the next time io.connect runs "
                   "with busType=mqtt). Password is never returned."),
    API::emptySchema(),
    &subscriberGetConfig);

  registry.registerCommand(
    QStringLiteral("project.mqtt.subscriber.setConfig"),
    QStringLiteral("Patch one or more MQTT subscriber driver fields. Pass only the "
                   "keys you want to change. Setting 'password' requires 'username' "
                   "in the same call. Mutating fields while connected schedules a "
                   "reconnect."),
    API::makeSchema(
      {
  },
      {{QStringLiteral("hostname"), QStringLiteral("string"), QStringLiteral("Broker hostname")},
       {QStringLiteral("port"),
        QStringLiteral("integer"),
        QStringLiteral("Broker TCP port (1-65535)")},
       {QStringLiteral("clientId"), QStringLiteral("string"), QStringLiteral("MQTT client id")},
       {QStringLiteral("username"),
        QStringLiteral("string"),
        QStringLiteral("Subscriber username")},
       {QStringLiteral("password"),
        QStringLiteral("string"),
        QStringLiteral("Subscriber password (requires username in same call)")},
       {QStringLiteral("topicFilter"),
        QStringLiteral("string"),
        QStringLiteral("Subscribe topic filter (supports + and # wildcards)")},
       {QStringLiteral("cleanSession"),
        QStringLiteral("boolean"),
        QStringLiteral("Use clean MQTT session")},
       {QStringLiteral("keepAlive"),
        QStringLiteral("integer"),
        QStringLiteral("Keepalive seconds (0-65535)")},
       {QStringLiteral("autoKeepAlive"),
        QStringLiteral("boolean"),
        QStringLiteral("Let the client manage keepalive automatically")},
       {QStringLiteral("mqttVersion"),
        QStringLiteral("integer"),
        QStringLiteral("Protocol version index (see 'mqttVersions' from getConfig)")},
       {QStringLiteral("sslEnabled"), QStringLiteral("boolean"), QStringLiteral("Enable TLS")},
       {QStringLiteral("sslProtocol"),
        QStringLiteral("integer"),
        QStringLiteral("TLS protocol index (see 'sslProtocols' from getConfig)")},
       {QStringLiteral("peerVerifyMode"),
        QStringLiteral("integer"),
        QStringLiteral("Peer verify mode (see 'peerVerifyModes' from getConfig)")},
       {QStringLiteral("peerVerifyDepth"),
        QStringLiteral("integer"),
        QStringLiteral("Peer verify chain depth (0-65535)")}}),
    &subscriberSetConfig);

  registry.registerCommand(
    QStringLiteral("project.mqtt.subscriber.getStatus"),
    QStringLiteral("Snapshot subscriber driver live state (isOpen, hostname, port)."),
    API::emptySchema(),
    &subscriberGetStatus);
}

//--------------------------------------------------------------------------------------------------
// Publisher
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the publisher configuration; password is never included.
 */
API::CommandResponse API::Handlers::MqttHandler::publisherGetConfig(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& p = MQTT::Publisher::instance();

  MQTT::CredentialVault vault;
  const bool hasCreds = vault.hasCredentials(p.hostname(), p.port());

  QJsonObject result;
  result[QStringLiteral("enabled")]              = p.enabled();
  result[QStringLiteral("hostname")]             = p.hostname();
  result[QStringLiteral("port")]                 = p.port();
  result[QStringLiteral("clientId")]             = p.clientId();
  result[QStringLiteral("customClientId")]       = p.customClientId();
  result[QStringLiteral("username")]             = p.username();
  result[QStringLiteral("hasCredentials")]       = hasCreds;
  result[QStringLiteral("topicBase")]            = p.topicBase();
  result[QStringLiteral("notificationTopic")]    = p.notificationTopic();
  result[QStringLiteral("publishNotifications")] = p.publishNotifications();
  result[QStringLiteral("mode")]                 = p.mode();
  result[QStringLiteral("modeLabel")]            = p.modeLabel();
  result[QStringLiteral("publishFrequency")]     = p.publishFrequency();
  result[QStringLiteral("cleanSession")]         = p.cleanSession();
  result[QStringLiteral("keepAlive")]            = p.keepAlive();
  result[QStringLiteral("mqttVersion")]          = p.mqttVersion();
  result[QStringLiteral("sslEnabled")]           = p.sslEnabled();
  result[QStringLiteral("sslProtocol")]          = p.sslProtocol();
  result[QStringLiteral("peerVerifyMode")]       = p.peerVerifyMode();
  result[QStringLiteral("peerVerifyDepth")]      = p.peerVerifyDepth();

  result[QStringLiteral("modes")]           = QJsonArray::fromStringList(p.modes());
  result[QStringLiteral("mqttVersions")]    = QJsonArray::fromStringList(p.mqttVersions());
  result[QStringLiteral("sslProtocols")]    = QJsonArray::fromStringList(p.sslProtocols());
  result[QStringLiteral("peerVerifyModes")] = QJsonArray::fromStringList(p.peerVerifyModes());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Applies endpoint + credential fields to the publisher; returns error on failure.
 */
[[nodiscard]] static std::optional<QString> applyPublisherEndpoint(MQTT::Publisher& p,
                                                                   const QJsonObject& params)
{
  if (params.contains(QStringLiteral("hostname"))) {
    const auto host = params.value(QStringLiteral("hostname")).toString();
    if (host.trimmed().isEmpty())
      return QStringLiteral("hostname must not be empty");

    p.setHostname(host);
  }

  if (params.contains(QStringLiteral("port"))) {
    const int port = params.value(QStringLiteral("port")).toInt(-1);
    if (!portIsValid(port))
      return QStringLiteral("port must be in [1, 65535] (got %1)").arg(port);

    p.setPort(static_cast<quint16>(port));
  }

  if (params.contains(QStringLiteral("clientId")))
    p.setClientId(params.value(QStringLiteral("clientId")).toString());

  if (params.contains(QStringLiteral("customClientId")))
    p.setCustomClientId(params.value(QStringLiteral("customClientId")).toBool());

  const bool hasUser = params.contains(QStringLiteral("username"));
  const bool hasPass = params.contains(QStringLiteral("password"));
  if (hasPass && !hasUser)
    return QStringLiteral("Setting 'password' requires 'username' in the same call");

  if (hasUser)
    p.setUsername(params.value(QStringLiteral("username")).toString());

  if (hasPass)
    p.setPassword(params.value(QStringLiteral("password")).toString());

  return std::nullopt;
}

/**
 * @brief Applies topic + mode + frequency fields to the publisher; returns error on failure.
 */
[[nodiscard]] static std::optional<QString> applyPublisherTopics(MQTT::Publisher& p,
                                                                 const QJsonObject& params)
{
  if (params.contains(QStringLiteral("topicBase")))
    p.setTopicBase(params.value(QStringLiteral("topicBase")).toString());

  if (params.contains(QStringLiteral("notificationTopic")))
    p.setNotificationTopic(params.value(QStringLiteral("notificationTopic")).toString());

  if (params.contains(QStringLiteral("publishNotifications")))
    p.setPublishNotifications(params.value(QStringLiteral("publishNotifications")).toBool());

  if (params.contains(QStringLiteral("mode"))) {
    const int mode = params.value(QStringLiteral("mode")).toInt(-1);
    if (!indexInRange(mode, p.modes().size()))
      return QStringLiteral("mode index out of range (got %1, valid 0..%2)")
        .arg(mode)
        .arg(p.modes().size() - 1);

    p.setMode(mode);
  }

  if (params.contains(QStringLiteral("publishFrequency"))) {
    const int hz = params.value(QStringLiteral("publishFrequency")).toInt(-1);
    if (hz < MQTT::Publisher::kMinPublishHz || hz > MQTT::Publisher::kMaxPublishHz)
      return QStringLiteral("publishFrequency must be in [%1, %2] (got %3)")
        .arg(MQTT::Publisher::kMinPublishHz)
        .arg(MQTT::Publisher::kMaxPublishHz)
        .arg(hz);

    p.setPublishFrequency(hz);
  }

  return std::nullopt;
}

/**
 * @brief Applies session + TLS fields to the publisher; returns error on failure.
 */
[[nodiscard]] static std::optional<QString> applyPublisherSessionAndTls(MQTT::Publisher& p,
                                                                        const QJsonObject& params)
{
  if (params.contains(QStringLiteral("cleanSession")))
    p.setCleanSession(params.value(QStringLiteral("cleanSession")).toBool());

  if (params.contains(QStringLiteral("keepAlive"))) {
    const int ka = params.value(QStringLiteral("keepAlive")).toInt(-1);
    if (ka < 0 || ka > 65535)
      return QStringLiteral("keepAlive must be in [0, 65535] (got %1)").arg(ka);

    p.setKeepAlive(static_cast<quint16>(ka));
  }

  if (params.contains(QStringLiteral("mqttVersion"))) {
    const int v = params.value(QStringLiteral("mqttVersion")).toInt(-1);
    if (!indexInRange(v, p.mqttVersions().size()))
      return QStringLiteral("mqttVersion index out of range (got %1)").arg(v);

    p.setMqttVersion(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("sslEnabled")))
    p.setSslEnabled(params.value(QStringLiteral("sslEnabled")).toBool());

  if (params.contains(QStringLiteral("sslProtocol"))) {
    const int v = params.value(QStringLiteral("sslProtocol")).toInt(-1);
    if (!indexInRange(v, p.sslProtocols().size()))
      return QStringLiteral("sslProtocol index out of range (got %1)").arg(v);

    p.setSslProtocol(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("peerVerifyMode"))) {
    const int v = params.value(QStringLiteral("peerVerifyMode")).toInt(-1);
    if (!indexInRange(v, p.peerVerifyModes().size()))
      return QStringLiteral("peerVerifyMode index out of range (got %1)").arg(v);

    p.setPeerVerifyMode(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("peerVerifyDepth"))) {
    const int d = params.value(QStringLiteral("peerVerifyDepth")).toInt(-1);
    if (d < 0 || d > 65535)
      return QStringLiteral("peerVerifyDepth must be in [0, 65535] (got %1)").arg(d);

    p.setPeerVerifyDepth(d);
  }

  return std::nullopt;
}

/**
 * @brief Applies a partial config patch to the publisher; rejects out-of-range values.
 */
API::CommandResponse API::Handlers::MqttHandler::publisherSetConfig(const QString& id,
                                                                    const QJsonObject& params)
{
  auto& p = MQTT::Publisher::instance();

  if (auto err = applyPublisherEndpoint(p, params); err)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  if (auto err = applyPublisherTopics(p, params); err)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  if (auto err = applyPublisherSessionAndTls(p, params); err)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  if (params.contains(QStringLiteral("enabled")))
    p.setEnabled(params.value(QStringLiteral("enabled")).toBool());

  QJsonObject result;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Snapshots publisher live state (connection, broker endpoint, counters).
 */
API::CommandResponse API::Handlers::MqttHandler::publisherGetStatus(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& p = MQTT::Publisher::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")]        = p.enabled();
  result[QStringLiteral("isConnected")]    = p.isConnected();
  result[QStringLiteral("brokerEndpoint")] = p.brokerEndpoint();
  result[QStringLiteral("messagesSent")]   = static_cast<qint64>(p.messagesSent());
  result[QStringLiteral("modeLabel")]      = p.modeLabel();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Subscriber driver
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a pointer to the UI MQTT driver, or nullptr if unavailable.
 */
[[nodiscard]] static IO::Drivers::MQTT* subscriber()
{
  return IO::ConnectionManager::instance().mqtt();
}

/**
 * @brief Returns the subscriber driver configuration; password is never included.
 */
API::CommandResponse API::Handlers::MqttHandler::subscriberGetConfig(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto* d = subscriber();
  if (!d)
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("MQTT subscriber driver unavailable"));

  MQTT::CredentialVault vault;
  const bool hasCreds = vault.hasCredentials(d->hostname(), d->port());

  QJsonObject result;
  result[QStringLiteral("hostname")]        = d->hostname();
  result[QStringLiteral("port")]            = d->port();
  result[QStringLiteral("clientId")]        = d->clientId();
  result[QStringLiteral("username")]        = d->username();
  result[QStringLiteral("hasCredentials")]  = hasCreds;
  result[QStringLiteral("topicFilter")]     = d->topicFilter();
  result[QStringLiteral("cleanSession")]    = d->cleanSession();
  result[QStringLiteral("autoKeepAlive")]   = d->autoKeepAlive();
  result[QStringLiteral("keepAlive")]       = d->keepAlive();
  result[QStringLiteral("mqttVersion")]     = d->mqttVersion();
  result[QStringLiteral("sslEnabled")]      = d->sslEnabled();
  result[QStringLiteral("sslProtocol")]     = d->sslProtocol();
  result[QStringLiteral("peerVerifyMode")]  = d->peerVerifyMode();
  result[QStringLiteral("peerVerifyDepth")] = d->peerVerifyDepth();

  result[QStringLiteral("mqttVersions")]    = QJsonArray::fromStringList(d->mqttVersions());
  result[QStringLiteral("sslProtocols")]    = QJsonArray::fromStringList(d->sslProtocols());
  result[QStringLiteral("peerVerifyModes")] = QJsonArray::fromStringList(d->peerVerifyModes());
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Applies endpoint + credential + topic fields to the subscriber.
 */
[[nodiscard]] static std::optional<QString> applySubscriberEndpoint(IO::Drivers::MQTT* d,
                                                                    const QJsonObject& params)
{
  if (params.contains(QStringLiteral("hostname"))) {
    const auto host = params.value(QStringLiteral("hostname")).toString();
    if (host.trimmed().isEmpty())
      return QStringLiteral("hostname must not be empty");

    d->setHostname(host);
  }

  if (params.contains(QStringLiteral("port"))) {
    const int port = params.value(QStringLiteral("port")).toInt(-1);
    if (!portIsValid(port))
      return QStringLiteral("port must be in [1, 65535] (got %1)").arg(port);

    d->setPort(static_cast<quint16>(port));
  }

  if (params.contains(QStringLiteral("clientId")))
    d->setClientId(params.value(QStringLiteral("clientId")).toString());

  const bool hasUser = params.contains(QStringLiteral("username"));
  const bool hasPass = params.contains(QStringLiteral("password"));
  if (hasPass && !hasUser)
    return QStringLiteral("Setting 'password' requires 'username' in the same call");

  if (hasUser)
    d->setUsername(params.value(QStringLiteral("username")).toString());

  if (hasPass)
    d->setPassword(params.value(QStringLiteral("password")).toString());

  if (params.contains(QStringLiteral("topicFilter")))
    d->setTopicFilter(params.value(QStringLiteral("topicFilter")).toString());

  return std::nullopt;
}

/**
 * @brief Applies session + TLS fields to the subscriber.
 */
[[nodiscard]] static std::optional<QString> applySubscriberSessionAndTls(IO::Drivers::MQTT* d,
                                                                         const QJsonObject& params)
{
  if (params.contains(QStringLiteral("cleanSession")))
    d->setCleanSession(params.value(QStringLiteral("cleanSession")).toBool());

  if (params.contains(QStringLiteral("autoKeepAlive")))
    d->setAutoKeepAlive(params.value(QStringLiteral("autoKeepAlive")).toBool());

  if (params.contains(QStringLiteral("keepAlive"))) {
    const int ka = params.value(QStringLiteral("keepAlive")).toInt(-1);
    if (ka < 0 || ka > 65535)
      return QStringLiteral("keepAlive must be in [0, 65535] (got %1)").arg(ka);

    d->setKeepAlive(static_cast<quint16>(ka));
  }

  if (params.contains(QStringLiteral("mqttVersion"))) {
    const int v = params.value(QStringLiteral("mqttVersion")).toInt(-1);
    if (!indexInRange(v, d->mqttVersions().size()))
      return QStringLiteral("mqttVersion index out of range (got %1)").arg(v);

    d->setMqttVersion(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("sslEnabled")))
    d->setSslEnabled(params.value(QStringLiteral("sslEnabled")).toBool());

  if (params.contains(QStringLiteral("sslProtocol"))) {
    const int v = params.value(QStringLiteral("sslProtocol")).toInt(-1);
    if (!indexInRange(v, d->sslProtocols().size()))
      return QStringLiteral("sslProtocol index out of range (got %1)").arg(v);

    d->setSslProtocol(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("peerVerifyMode"))) {
    const int v = params.value(QStringLiteral("peerVerifyMode")).toInt(-1);
    if (!indexInRange(v, d->peerVerifyModes().size()))
      return QStringLiteral("peerVerifyMode index out of range (got %1)").arg(v);

    d->setPeerVerifyMode(static_cast<quint8>(v));
  }

  if (params.contains(QStringLiteral("peerVerifyDepth"))) {
    const int dp = params.value(QStringLiteral("peerVerifyDepth")).toInt(-1);
    if (dp < 0 || dp > 65535)
      return QStringLiteral("peerVerifyDepth must be in [0, 65535] (got %1)").arg(dp);

    d->setPeerVerifyDepth(dp);
  }

  return std::nullopt;
}

/**
 * @brief Applies a partial config patch to the subscriber driver.
 */
API::CommandResponse API::Handlers::MqttHandler::subscriberSetConfig(const QString& id,
                                                                     const QJsonObject& params)
{
  auto* d = subscriber();
  if (!d)
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("MQTT subscriber driver unavailable"));

  if (auto err = applySubscriberEndpoint(d, params); err)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  if (auto err = applySubscriberSessionAndTls(d, params); err)
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, *err);

  QJsonObject result;
  result[QStringLiteral("updated")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Snapshots subscriber driver live state.
 */
API::CommandResponse API::Handlers::MqttHandler::subscriberGetStatus(const QString& id,
                                                                     const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto* d = subscriber();
  if (!d)
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("MQTT subscriber driver unavailable"));

  QJsonObject result;
  result[QStringLiteral("isOpen")]   = d->isOpen();
  result[QStringLiteral("hostname")] = d->hostname();
  result[QStringLiteral("port")]     = d->port();
  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
