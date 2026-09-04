/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#  include "MQTT/Publisher.h"

#  include <algorithm>
#  include <QApplication>
#  include <QFileDialog>
#  include <QJsonDocument>
#  include <QLoggingCategory>
#  include <QRandomGenerator>
#  include <QStandardPaths>

#  include "Core/SSAssert.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/ProjectModel.h"
#  include "Licensing/CommercialToken.h"
#  include "Misc/Utilities.h"

//==================================================================================================
// Construction
//==================================================================================================

/**
 * @brief Constructs the publisher with safe broker defaults and starts the worker thread.
 */
MQTT::Publisher::Publisher()
  : DataModel::FrameConsumer<DataModel::DataBlockPtr>(
      {.queueCapacity = 8192, .flushThreshold = 1024, .timerIntervalMs = 100})
  , m_enabled(false)
  , m_publishNotifications(false)
  , m_cleanSession(true)
  , m_inApply(false)
  , m_skipNextSync(false)
  , m_savingToProjectModel(false)
  , m_reportConnectionErrors(false)
  , m_customClientId(false)
  , m_sparkplugEnabled(false)
  , m_mode(static_cast<int>(Mode::RawRxData))
  , m_scriptLanguage(0)
  , m_publishFrequencyHz(kDefaultPublishHz)
  , m_protocolVersion(QMqttClient::MQTT_5_0)
  , m_port(1883)
  , m_keepAlive(60)
  , m_hostname(QStringLiteral("127.0.0.1"))
  , m_rawBytesQueue(8192)
  , m_rawFramesQueue(8192)
  , m_hotEnabled(false)
  , m_hotSparkplug(false)
  , m_hotHasTopic(false)
  , m_workerMode(static_cast<int>(Mode::RawRxData))
  , m_workerScriptLanguage(0)
  , m_isConnected(false)
  , m_messagesSent(0)
  , m_bytesSent(0)
  , m_messagesSentSeen(0)
{
  qRegisterMetaType<MQTT::BrokerConfig>("MQTT::BrokerConfig");
  qRegisterMetaType<QMqttClient::ClientState>("QMqttClient::ClientState");
  qRegisterMetaType<QMqttClient::ClientError>("QMqttClient::ClientError");

  registerBrokerOptions();

  m_syncTimer.setSingleShot(true);
  m_syncTimer.setInterval(kSyncDebounceMs);
  connect(&m_syncTimer, &QTimer::timeout, this, &Publisher::syncToWorker);

  m_statsTimer.setInterval(kStatsTickMs);
  connect(&m_statsTimer, &QTimer::timeout, this, &Publisher::emitStatsIfChanged);
  m_statsTimer.start();

  regenerateClientId();

  initializeWorker();
  applyTimerInterval();

  auto* w = static_cast<PublisherWorker*>(m_worker);
  connect(w, &PublisherWorker::brokerStateChanged, this, &Publisher::onWorkerBrokerStateChanged);
  connect(w, &PublisherWorker::brokerErrorOccurred, this, &Publisher::onWorkerBrokerError);
  connect(w, &PublisherWorker::scriptErrorOccurred, this, &Publisher::onWorkerScriptError);
  connect(
    w, &PublisherWorker::testConnectionFinished, this, &Publisher::onWorkerTestConnectionFinished);

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  connect(&frameBuilder,
          &DataModel::FrameBuilder::structureGenerationChanged,
          w,
          &PublisherWorker::setStructureGeneration,
          Qt::QueuedConnection);
  connect(&frameBuilder,
          &DataModel::FrameBuilder::structurePublished,
          w,
          &PublisherWorker::setTemplateFrame,
          Qt::QueuedConnection);

  QMetaObject::invokeMethod(w, &PublisherWorker::bootstrap, Qt::QueuedConnection);
}

/**
 * @brief Destructor; the FrameConsumer base waits for the worker thread to finish.
 */
MQTT::Publisher::~Publisher() = default;

/**
 * @brief Returns the singleton instance of the MQTT Publisher.
 */
MQTT::Publisher& MQTT::Publisher::instance()
{
  static Publisher singleton;
  return singleton;
}

/**
 * @brief FrameConsumer factory method; creates the worker for our threading wrapper.
 */
DataModel::FrameConsumerWorkerBase* MQTT::Publisher::createWorker()
{
  return new PublisherWorker(&m_pendingQueue,
                             &m_consumerEnabled,
                             &m_queueSize,
                             &m_rawBytesQueue,
                             &m_rawFramesQueue,
                             &m_workerMode,
                             &m_workerScriptLanguage,
                             &m_messagesSent,
                             &m_bytesSent);
}

/**
 * @brief Fills the option tables the settings UI binds to. The labels are built here, in the
 *        publisher's own translation context, because a project file stores the index each table
 *        implies: moving these strings would renumber every saved selection.
 */
void MQTT::Publisher::registerBrokerOptions()
{
  m_options.setModes({tr("Raw RX Data"),
                      tr("Custom Script"),
                      tr("Dashboard Data (CSV)"),
                      tr("Dashboard Data (JSON)")});

  m_options.addMqttVersion(tr("MQTT 3.1"), QMqttClient::MQTT_3_1);
  m_options.addMqttVersion(tr("MQTT 3.1.1"), QMqttClient::MQTT_3_1_1);
  m_options.addMqttVersion(tr("MQTT 5.0"), QMqttClient::MQTT_5_0);

  m_options.addSslProtocol(tr("TLS 1.2"), QSsl::TlsV1_2);
  m_options.addSslProtocol(tr("TLS 1.3"), QSsl::TlsV1_3);
  m_options.addSslProtocol(tr("TLS 1.3 or Later"), QSsl::TlsV1_3OrLater);
  m_options.addSslProtocol(tr("DTLS 1.2 or Later"), QSsl::DtlsV1_2OrLater);
  m_options.addSslProtocol(tr("Any Protocol"), QSsl::AnyProtocol);
  m_options.addSslProtocol(tr("Secure Protocols Only"), QSsl::SecureProtocols);

  m_options.addPeerVerifyMode(tr("None"), QSslSocket::VerifyNone);
  m_options.addPeerVerifyMode(tr("Query Peer"), QSslSocket::QueryPeer);
  m_options.addPeerVerifyMode(tr("Verify Peer"), QSslSocket::VerifyPeer);
  m_options.addPeerVerifyMode(tr("Auto Verify Peer"), QSslSocket::AutoVerifyPeer);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the publisher is enabled for the current project.
 */
bool MQTT::Publisher::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Returns whether SSL/TLS is enabled.
 */
bool MQTT::Publisher::sslEnabled() const noexcept
{
  return m_tls.enabled();
}

/**
 * @brief Returns true while the broker connection is established.
 */
bool MQTT::Publisher::isConnected() const
{
  return m_isConnected.load(std::memory_order_relaxed);
}

/**
 * @brief Returns the broker clean-session flag.
 */
bool MQTT::Publisher::cleanSession() const noexcept
{
  return m_cleanSession;
}

/**
 * @brief Returns whether notifications are mirrored to MQTT.
 */
bool MQTT::Publisher::publishNotifications() const noexcept
{
  return m_publishNotifications;
}

/**
 * @brief Returns the configured publisher mode (Mode enum index).
 */
int MQTT::Publisher::mode() const noexcept
{
  return m_mode;
}

/**
 * @brief Returns the SSL peer-verification depth.
 */
int MQTT::Publisher::peerVerifyDepth() const noexcept
{
  return m_tls.peerVerifyDepth();
}

/**
 * @brief Returns the configured publish rate in Hz.
 */
int MQTT::Publisher::publishFrequency() const noexcept
{
  return m_publishFrequencyHz;
}

/**
 * @brief Returns the selected MQTT protocol version index.
 */
quint8 MQTT::Publisher::mqttVersion() const noexcept
{
  return m_options.mqttVersionIndex(m_protocolVersion);
}

/**
 * @brief Returns the selected SSL protocol index.
 */
quint8 MQTT::Publisher::sslProtocol() const noexcept
{
  return m_options.sslProtocolIndex(m_tls.protocol());
}

/**
 * @brief Returns the selected SSL peer-verification mode index.
 */
quint8 MQTT::Publisher::peerVerifyMode() const noexcept
{
  return m_options.peerVerifyModeIndex(m_tls.peerVerifyMode());
}

/**
 * @brief Returns the broker TCP port.
 */
quint16 MQTT::Publisher::port() const noexcept
{
  return m_port;
}

/**
 * @brief Returns the keep-alive interval in seconds.
 */
quint16 MQTT::Publisher::keepAlive() const noexcept
{
  return m_keepAlive;
}

/**
 * @brief Returns the MQTT client identifier.
 */
QString MQTT::Publisher::clientId() const
{
  return m_clientId;
}

/**
 * @brief Returns true when the user has opted to manage the client id manually.
 */
bool MQTT::Publisher::customClientId() const noexcept
{
  return m_customClientId;
}

/**
 * @brief Returns the broker hostname.
 */
QString MQTT::Publisher::hostname() const
{
  return m_hostname;
}

/**
 * @brief Returns the broker authentication username.
 */
QString MQTT::Publisher::username() const
{
  return m_username;
}

/**
 * @brief Returns the broker authentication password.
 */
QString MQTT::Publisher::password() const
{
  return m_password;
}

/**
 * @brief Returns the client certificate PEM path used for mutual TLS (empty = off).
 */
QString MQTT::Publisher::clientCertificatePath() const
{
  return m_tls.certificatePath();
}

/**
 * @brief Returns the private key PEM path (empty = look in the certificate file).
 */
QString MQTT::Publisher::privateKeyPath() const
{
  return m_tls.privateKeyPath();
}

/**
 * @brief Returns the private-key passphrase (kept obfuscated in the vault, never in projects).
 */
QString MQTT::Publisher::keyPassphrase() const
{
  return m_keyPassphrase;
}

/**
 * @brief Returns whether ALPN is requested during the TLS handshake (MQTT over port 443).
 */
bool MQTT::Publisher::alpnEnabled() const noexcept
{
  return m_tls.alpnEnabled();
}

/**
 * @brief Returns the ALPN protocol name announced when ALPN is enabled.
 */
QString MQTT::Publisher::alpnProtocol() const
{
  return m_tls.alpnProtocol();
}

/**
 * @brief Returns the base MQTT topic for dashboard or raw publishing.
 */
QString MQTT::Publisher::topicBase() const
{
  return m_topicBase;
}

/**
 * @brief Returns the dedicated MQTT topic for notifications.
 */
QString MQTT::Publisher::notificationTopic() const
{
  return m_notificationTopic;
}

/**
 * @brief Returns the user script used in ScriptDriven mode.
 */
QString MQTT::Publisher::scriptCode() const
{
  return m_scriptCode;
}

/**
 * @brief Returns the per-script override topic (empty == use topicBase).
 */
QString MQTT::Publisher::scriptTopic() const
{
  return m_scriptTopic;
}

/**
 * @brief Returns the script language enum (0=JavaScript, 1=Lua).
 */
int MQTT::Publisher::scriptLanguage() const noexcept
{
  return m_scriptLanguage;
}

/**
 * @brief Returns whether the publisher acts as a Sparkplug B edge node.
 */
bool MQTT::Publisher::sparkplugEnabled() const noexcept
{
  return m_sparkplugEnabled;
}

/**
 * @brief Returns the Sparkplug group id this node publishes under.
 */
QString MQTT::Publisher::sparkplugGroupId() const
{
  return m_sparkplugGroupId;
}

/**
 * @brief Returns the optional Sparkplug device id (empty publishes at node level).
 */
QString MQTT::Publisher::sparkplugDeviceId() const
{
  return m_sparkplugDeviceId;
}

/**
 * @brief Returns the Sparkplug edge node id.
 */
QString MQTT::Publisher::sparkplugEdgeNodeId() const
{
  return m_sparkplugEdgeNodeId;
}

/**
 * @brief Returns the localized label of the active mode.
 */
QString MQTT::Publisher::modeLabel() const
{
  const auto& list = modes();
  if (m_mode >= 0 && m_mode < list.size())
    return list.at(m_mode);

  return QString();
}

/**
 * @brief Returns "host:port" for compact display in the status popup.
 */
QString MQTT::Publisher::brokerEndpoint() const
{
  return QStringLiteral("%1:%2").arg(m_hostname, QString::number(m_port));
}

/**
 * @brief Returns the number of messages successfully published since the singleton booted.
 */
quint64 MQTT::Publisher::messagesSent() const noexcept
{
  return m_messagesSent.load(std::memory_order_relaxed);
}

/**
 * @brief Returns the canonical starter script shown in the editor when no code is saved.
 */
QString MQTT::Publisher::defaultScriptTemplate()
{
  return QStringLiteral(
    "// Called once per parsed frame. Return the payload to publish to the broker.\n"
    "// Return null or undefined to skip this frame.\n"
    "//\n"
    "// The `frame` argument is the JSON shape produced by Frame::serialize().\n"
    "function mqtt(frame) {\n"
    "  return JSON.stringify(frame);\n"
    "}\n");
}

/**
 * @brief Returns the available publisher modes (display names).
 */
const QStringList& MQTT::Publisher::modes() const
{
  return m_options.modes();
}

/**
 * @brief Returns the available MQTT protocol versions (display names).
 */
const QStringList& MQTT::Publisher::mqttVersions() const
{
  return m_options.mqttVersions();
}

/**
 * @brief Returns the available SSL/TLS protocols (display names).
 */
const QStringList& MQTT::Publisher::sslProtocols() const
{
  return m_options.sslProtocols();
}

/**
 * @brief Returns the available SSL peer-verification modes.
 */
const QStringList& MQTT::Publisher::peerVerifyModes() const
{
  return m_options.peerVerifyModes();
}

//--------------------------------------------------------------------------------------------------
// Project-config serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes the current configuration as a JSON object for project storage.
 */
QJsonObject MQTT::Publisher::toJson() const
{
  QJsonObject obj;
  obj.insert(kKeyEnabled, m_enabled);
  obj.insert(kKeyMode, m_mode);
  obj.insert(kKeyPublishNotifications, m_publishNotifications);
  obj.insert(kKeyPublishFrequency, m_publishFrequencyHz);
  obj.insert(kKeyTopicBase, m_topicBase);
  obj.insert(kKeyNotificationTopic, m_notificationTopic);
  obj.insert(kKeyScriptCode, m_scriptCode);
  obj.insert(kKeyScriptTopic, m_scriptTopic);
  obj.insert(kKeyScriptLanguage, m_scriptLanguage);
  obj.insert(kKeyHostname, m_hostname);
  obj.insert(kKeyPort, static_cast<int>(m_port));
  obj.insert(kKeyCustomClientId, m_customClientId);

  if (m_customClientId)
    obj.insert(kKeyClientId, m_clientId);

  obj.insert(kKeyCleanSession, m_cleanSession);
  obj.insert(kKeyKeepAlive, static_cast<int>(m_keepAlive));
  obj.insert(kKeyMqttVersion, static_cast<int>(mqttVersion()));
  obj.insert(kKeySslEnabled, m_tls.enabled());
  obj.insert(kKeySslProtocol, static_cast<int>(sslProtocol()));
  obj.insert(kKeyPeerVerifyMode, static_cast<int>(peerVerifyMode()));
  obj.insert(kKeyPeerVerifyDepth, m_tls.peerVerifyDepth());
  obj.insert(kKeyClientCertPath, m_tls.certificatePath());
  obj.insert(kKeyPrivateKeyPath, m_tls.privateKeyPath());
  obj.insert(kKeyAlpnEnabled, m_tls.alpnEnabled());
  obj.insert(kKeyAlpnProtocol, m_tls.alpnProtocol());
  obj.insert(kKeySparkplugEnabled, m_sparkplugEnabled);
  obj.insert(kKeySparkplugGroupId, m_sparkplugGroupId);
  obj.insert(kKeySparkplugEdgeNodeId, m_sparkplugEdgeNodeId);
  obj.insert(kKeySparkplugDeviceId, m_sparkplugDeviceId);
  return obj;
}

/**
 * @brief Loads configuration from a project-supplied JSON object.
 */
void MQTT::Publisher::applyProjectConfig(const QJsonObject& cfg)
{
  m_inApply = true;

  setEnabled(cfg.value(kKeyEnabled).toBool(false));
  setMode(cfg.value(kKeyMode).toInt(static_cast<int>(Mode::RawRxData)));
  setPublishNotifications(cfg.value(kKeyPublishNotifications).toBool(false));
  setPublishFrequency(cfg.value(kKeyPublishFrequency).toInt(kDefaultPublishHz));
  setTopicBase(cfg.value(kKeyTopicBase).toString());
  setNotificationTopic(cfg.value(kKeyNotificationTopic).toString());
  setScriptCode(cfg.value(kKeyScriptCode).toString());
  setScriptTopic(cfg.value(kKeyScriptTopic).toString());
  setScriptLanguage(cfg.value(kKeyScriptLanguage).toInt(0));

  setHostname(cfg.value(kKeyHostname).toString(QStringLiteral("127.0.0.1")));
  setPort(static_cast<quint16>(cfg.value(kKeyPort).toInt(1883)));

  const bool customCid = cfg.value(kKeyCustomClientId).toBool(false);
  setCustomClientId(customCid);

  const auto cid = cfg.value(kKeyClientId).toString();
  if (customCid && !cid.isEmpty())
    setClientId(cid);
  else
    regenerateClientId();

  reloadCredentialsFromVault();

  setCleanSession(cfg.value(kKeyCleanSession).toBool(true));
  setKeepAlive(static_cast<quint16>(cfg.value(kKeyKeepAlive).toInt(60)));
  setMqttVersion(static_cast<quint8>(cfg.value(kKeyMqttVersion).toInt(2)));

  setSslEnabled(cfg.value(kKeySslEnabled).toBool(false));
  setSslProtocol(static_cast<quint8>(cfg.value(kKeySslProtocol).toInt(5)));
  setPeerVerifyMode(static_cast<quint8>(cfg.value(kKeyPeerVerifyMode).toInt(3)));
  setPeerVerifyDepth(cfg.value(kKeyPeerVerifyDepth).toInt(10));

  setAlpnEnabled(cfg.value(kKeyAlpnEnabled).toBool(false));
  setAlpnProtocol(cfg.value(kKeyAlpnProtocol).toString(QStringLiteral("x-amzn-mqtt-ca")));

  setSparkplugEnabled(cfg.value(kKeySparkplugEnabled).toBool(false));
  setSparkplugGroupId(cfg.value(kKeySparkplugGroupId).toString());
  setSparkplugEdgeNodeId(cfg.value(kKeySparkplugEdgeNodeId).toString());
  setSparkplugDeviceId(cfg.value(kKeySparkplugDeviceId).toString());

  m_tls.setCertificatePath(cfg.value(kKeyClientCertPath).toString());
  m_tls.setPrivateKeyPath(cfg.value(kKeyPrivateKeyPath).toString());
  reloadTlsIdentity(false);

  m_inApply = false;

  m_savingToProjectModel    = true;
  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.setMqttPublisher(toJson());
  m_savingToProjectModel = false;

  m_skipNextSync = true;
  Q_EMIT configurationChanged();
  m_skipNextSync = false;

  m_reportConnectionErrors = m_enabled;
  m_syncTimer.stop();
  syncToWorker();
}

/**
 * @brief Resets the publisher to default state (used on project close / new).
 */
void MQTT::Publisher::resetProjectConfig()
{
  applyProjectConfig(QJsonObject{});
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Triggers a one-shot probe connection on the worker.
 */
void MQTT::Publisher::testConnection()
{
  if (!licenseValid()) {
    Misc::Utilities::showMessageBox(
      tr("MQTT publisher unavailable"),
      tr("A valid commercial license is required to use MQTT publishing."),
      QMessageBox::Warning,
      tr("MQTT Test Connection"));
    return;
  }

  if (m_syncTimer.isActive()) {
    m_syncTimer.stop();
    syncToWorker();
  }

  QMetaObject::invokeMethod(m_worker, "runTestConnection", Qt::QueuedConnection);
}

/**
 * @brief Opens a folder picker to load additional CA certificates. The scan runs through a queued
 *        invoke: on macOS fileSelected fires inside QFileDialog::done() and re-entering Qt
 *        synchronously can delete the dialog under the native panel.
 */
void MQTT::Publisher::addCaCertificates()
{
  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    tr("Select PEM Certificates Directory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        m_tls.addCaCertificatesFromDirectory(path);
        scheduleSyncToWorker();
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a PEM file picker and routes the selection into the given path setter. The work
 *        runs through a queued invoke: on macOS fileSelected fires inside QFileDialog::done()
 *        and re-entering Qt synchronously can delete the dialog under the native panel.
 */
void MQTT::Publisher::selectPemFile(const QString& title, void (Publisher::*setter)(const QString&))
{
  SS_ASSERT(setter != nullptr, return);
  SS_ASSERT(!title.isEmpty(), return);

  auto* dialog =
    new QFileDialog(qApp->activeWindow(),
                    title,
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                    tr("PEM files (*.pem *.crt *.cer *.key);;All files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this, setter](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, setter, path]() {
        (this->*setter)(path);
        reloadTlsIdentity(true);
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Opens a file picker for the mutual-TLS client certificate.
 */
void MQTT::Publisher::selectClientCertificate()
{
  selectPemFile(tr("Select Client Certificate"), &Publisher::setClientCertificatePath);
}

/**
 * @brief Opens a file picker for the mutual-TLS private key.
 */
void MQTT::Publisher::selectPrivateKey()
{
  selectPemFile(tr("Select Private Key"), &Publisher::setPrivateKeyPath);
}

/**
 * @brief Generates a fresh random 16-char client ID into the auto slot.
 */
void MQTT::Publisher::regenerateClientId()
{
  QString id;
  constexpr int length  = 16;
  const QString charset = QStringLiteral("abcdefghijklmnopqrstuvwxyz0123456789");
  for (int i = 0; i < length; ++i) {
    const int index = QRandomGenerator::global()->bounded(charset.length());
    id.append(charset.at(index));
  }

  m_autoClientId = id;
  if (!m_customClientId)
    setClientId(id);
}

//--------------------------------------------------------------------------------------------------
// External wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the publisher to NotificationCenter and ProjectModel so config loads on
 *        project open and edits round-trip back into the project file.
 */
void MQTT::Publisher::setupExternalConnections()
{
  connect(&DataModel::NotificationCenter::instance(),
          &DataModel::NotificationCenter::notificationPosted,
          this,
          &Publisher::onNotificationPosted);

  auto* projectModel = &DataModel::ProjectModel::instance();
  connect(projectModel, &DataModel::ProjectModel::mqttPublisherChanged, this, [this, projectModel] {
    if (m_savingToProjectModel)
      return;

    applyProjectConfig(projectModel->mqttPublisher());
  });
  connect(this, &Publisher::configurationChanged, this, [this, projectModel] {
    if (m_skipNextSync)
      return;

    m_savingToProjectModel = true;
    projectModel->setMqttPublisher(toJson());
    m_savingToProjectModel = false;
  });

  applyProjectConfig(projectModel->mqttPublisher());
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables MQTT publishing for the current project.
 */
void MQTT::Publisher::setEnabled(const bool enabled)
{
  if (m_enabled == enabled)
    return;

  m_enabled = enabled;
  m_hotEnabled.store(enabled, std::memory_order_relaxed);
  setConsumerEnabled(enabled);

  if (enabled) {
    m_messagesSent.store(0, std::memory_order_relaxed);
    m_bytesSent.store(0, std::memory_order_relaxed);
    m_messagesSentSeen = 0;
    Q_EMIT statsChanged();
  }

  markConfigChanged();
}

/**
 * @brief Sets the publisher mode (DashboardData / RawRxData).
 */
void MQTT::Publisher::setMode(const int mode)
{
  if (m_mode == mode)
    return;

  m_mode = mode;
  m_workerMode.store(mode, std::memory_order_relaxed);
  markConfigChanged();
}

/**
 * @brief Enables or disables SSL/TLS for the broker connection.
 */
void MQTT::Publisher::setSslEnabled(const bool enabled)
{
  if (m_tls.enabled() == enabled)
    return;

  m_tls.setEnabled(enabled);
  markConfigChanged();
}

/**
 * @brief Sets the broker clean-session flag.
 */
void MQTT::Publisher::setCleanSession(const bool cleanSession)
{
  if (m_cleanSession == cleanSession)
    return;

  m_cleanSession = cleanSession;
  markConfigChanged();
}

/**
 * @brief Toggles publishing of notifications to MQTT.
 */
void MQTT::Publisher::setPublishNotifications(const bool publish)
{
  if (m_publishNotifications == publish)
    return;

  m_publishNotifications = publish;
  markConfigChanged();
}

/**
 * @brief Sets the SSL peer-verification chain depth.
 */
void MQTT::Publisher::setPeerVerifyDepth(const int depth)
{
  if (m_tls.peerVerifyDepth() == depth)
    return;

  m_tls.setPeerVerifyDepth(depth);
  markConfigChanged();
}

/**
 * @brief Sets the publish frequency in Hz; clamped to [kMinPublishHz, kMaxPublishHz].
 */
void MQTT::Publisher::setPublishFrequency(const int hz)
{
  const int clamped = std::clamp(hz, kMinPublishHz, kMaxPublishHz);
  if (m_publishFrequencyHz == clamped)
    return;

  m_publishFrequencyHz = clamped;
  applyTimerInterval();
  markConfigChanged();
}

/**
 * @brief Sets the MQTT protocol version by index.
 */
void MQTT::Publisher::setMqttVersion(const quint8 version)
{
  QMqttClient::ProtocolVersion selected = m_protocolVersion;
  if (!m_options.mqttVersionAt(version, selected))
    return;

  if (selected == m_protocolVersion)
    return;

  m_protocolVersion = selected;
  markConfigChanged();
}

/**
 * @brief Sets the SSL protocol by index.
 */
void MQTT::Publisher::setSslProtocol(const quint8 protocol)
{
  QSsl::SslProtocol selected = m_tls.protocol();
  if (!m_options.sslProtocolAt(protocol, selected))
    return;

  if (selected == m_tls.protocol())
    return;

  m_tls.setProtocol(selected);
  markConfigChanged();
}

/**
 * @brief Sets the SSL peer-verification mode by index.
 */
void MQTT::Publisher::setPeerVerifyMode(const quint8 verifyMode)
{
  QSslSocket::PeerVerifyMode selected = m_tls.peerVerifyMode();
  if (!m_options.peerVerifyModeAt(verifyMode, selected))
    return;

  if (selected == m_tls.peerVerifyMode())
    return;

  if (selected == QSslSocket::VerifyNone) [[unlikely]]
    qWarning() << "[MQTT publisher] TLS peer verification disabled -- vulnerable to MITM";

  m_tls.setPeerVerifyMode(selected);
  markConfigChanged();
}

/**
 * @brief Sets the broker TCP port.
 */
void MQTT::Publisher::setPort(const quint16 port)
{
  if (m_port == port)
    return;

  m_port = port;

  if (!m_inApply) {
    reloadCredentialsFromVault();
    reloadTlsIdentity(false);
  }

  markConfigChanged();
}

/**
 * @brief Sets the keep-alive interval in seconds.
 */
void MQTT::Publisher::setKeepAlive(const quint16 keepAlive)
{
  if (m_keepAlive == keepAlive)
    return;

  m_keepAlive = keepAlive;
  markConfigChanged();
}

/**
 * @brief Sets the MQTT client identifier.
 */
void MQTT::Publisher::setClientId(const QString& id)
{
  if (m_clientId == id)
    return;

  m_clientId = id;
  markConfigChanged();
}

/**
 * @brief Toggles user management of the client id.
 */
void MQTT::Publisher::setCustomClientId(const bool custom)
{
  if (m_customClientId == custom)
    return;

  m_customClientId = custom;

  if (!custom) {
    if (m_autoClientId.isEmpty())
      regenerateClientId();
    else
      setClientId(m_autoClientId);
  }

  markConfigChanged();
}

/**
 * @brief Sets the broker hostname.
 */
void MQTT::Publisher::setHostname(const QString& hostname)
{
  if (m_hostname == hostname)
    return;

  m_hostname = hostname;

  if (!m_inApply) {
    reloadCredentialsFromVault();
    reloadTlsIdentity(false);
  }

  markConfigChanged();
}

/**
 * @brief Sets the broker authentication username.
 */
void MQTT::Publisher::setUsername(const QString& username)
{
  if (m_username == username)
    return;

  m_username = username;
  persistCredentialsToVault();
  markConfigChanged();
}

/**
 * @brief Sets the broker authentication password.
 */
void MQTT::Publisher::setPassword(const QString& password)
{
  if (m_password == password)
    return;

  m_password = password;
  persistCredentialsToVault();
  markConfigChanged();
}

/**
 * @brief Sets the client certificate PEM path and reloads the parsed TLS identity.
 */
void MQTT::Publisher::setClientCertificatePath(const QString& path)
{
  if (m_tls.certificatePath() == path)
    return;

  m_tls.setCertificatePath(path);
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Sets the private key PEM path and reloads the parsed TLS identity.
 */
void MQTT::Publisher::setPrivateKeyPath(const QString& path)
{
  if (m_tls.privateKeyPath() == path)
    return;

  m_tls.setPrivateKeyPath(path);
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Sets the private-key passphrase, persists it to the vault and re-parses the key.
 */
void MQTT::Publisher::setKeyPassphrase(const QString& passphrase)
{
  if (m_keyPassphrase == passphrase)
    return;

  m_keyPassphrase = passphrase;
  persistCredentialsToVault();
  reloadTlsIdentity(false);
  markConfigChanged();
}

/**
 * @brief Enables or disables ALPN announcement during the TLS handshake.
 */
void MQTT::Publisher::setAlpnEnabled(const bool enabled)
{
  if (m_tls.alpnEnabled() == enabled)
    return;

  m_tls.setAlpnEnabled(enabled);
  markConfigChanged();
}

/**
 * @brief Sets the ALPN protocol name (AWS IoT uses "x-amzn-mqtt-ca" on port 443).
 */
void MQTT::Publisher::setAlpnProtocol(const QString& protocol)
{
  if (m_tls.alpnProtocol() == protocol)
    return;

  m_tls.setAlpnProtocol(protocol);
  markConfigChanged();
}

/**
 * @brief Sets the base topic for frame/raw publishing.
 */
void MQTT::Publisher::setTopicBase(const QString& topic)
{
  if (m_topicBase == topic)
    return;

  m_topicBase = topic;
  m_hotHasTopic.store(!topic.isEmpty(), std::memory_order_relaxed);
  markConfigChanged();
}

/**
 * @brief Sets the notification publishing topic.
 */
void MQTT::Publisher::setNotificationTopic(const QString& topic)
{
  if (m_notificationTopic == topic)
    return;

  m_notificationTopic = topic;
  markConfigChanged();
}

/**
 * @brief Sets the user script source for ScriptDriven mode.
 */
void MQTT::Publisher::setScriptCode(const QString& code)
{
  if (m_scriptCode == code)
    return;

  m_scriptCode = code;
  markConfigChanged();
}

/**
 * @brief Sets the per-script override topic (empty == fall back to topicBase).
 */
void MQTT::Publisher::setScriptTopic(const QString& topic)
{
  if (m_scriptTopic == topic)
    return;

  m_scriptTopic = topic;
  markConfigChanged();
}

/**
 * @brief Sets the script language (0 = JavaScript, 1 = Lua).
 */
void MQTT::Publisher::setScriptLanguage(const int language)
{
  if (m_scriptLanguage == language)
    return;

  m_scriptLanguage = language;
  m_workerScriptLanguage.store(language, std::memory_order_relaxed);
  markConfigChanged();
}

/**
 * @brief Enables or disables Sparkplug B edge-node publishing for the current project.
 */
void MQTT::Publisher::setSparkplugEnabled(const bool enabled)
{
  if (m_sparkplugEnabled == enabled)
    return;

  m_sparkplugEnabled = enabled;
  m_hotSparkplug.store(enabled, std::memory_order_relaxed);
  markConfigChanged();
}

/**
 * @brief Sets the Sparkplug group id.
 */
void MQTT::Publisher::setSparkplugGroupId(const QString& groupId)
{
  if (m_sparkplugGroupId == groupId)
    return;

  m_sparkplugGroupId = groupId;
  markConfigChanged();
}

/**
 * @brief Sets the optional Sparkplug device id.
 */
void MQTT::Publisher::setSparkplugDeviceId(const QString& deviceId)
{
  if (m_sparkplugDeviceId == deviceId)
    return;

  m_sparkplugDeviceId = deviceId;
  markConfigChanged();
}

/**
 * @brief Sets the Sparkplug edge node id.
 */
void MQTT::Publisher::setSparkplugEdgeNodeId(const QString& edgeNodeId)
{
  if (m_sparkplugEdgeNodeId == edgeNodeId)
    return;

  m_sparkplugEdgeNodeId = edgeNodeId;
  markConfigChanged();
}

//--------------------------------------------------------------------------------------------------
// Data publishing hotpaths
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enqueues the frame for the worker; rate-limiting and broker I/O happen off-main. An edge
 *        node addresses its own spBv1.0 topics and declares datasets whatever the payload mode is,
 *        so Sparkplug takes the block on its own terms; with Sparkplug off the gate is unchanged.
 */
void MQTT::Publisher::ingestBlock(const DataModel::DataBlockPtr& block)
{
  if (!m_hotEnabled.load(std::memory_order_relaxed) || !licenseValid()) [[likely]]
    return;

  const bool sparkplug = m_hotSparkplug.load(std::memory_order_relaxed);
  const int mode       = m_workerMode.load(std::memory_order_relaxed);
  if (!sparkplug && mode != static_cast<int>(Mode::DashboardDataJson)
      && mode != static_cast<int>(Mode::DashboardDataCsv))
    return;

  if (!block || (!sparkplug && !m_hotHasTopic.load(std::memory_order_relaxed)))
    return;

  enqueueData(block);
}

/**
 * @brief Enqueues raw driver bytes for the worker (RawRxData mode only). A Sparkplug edge node
 *        publishes its own namespace, so the raw queue is dead under it: gate the enqueue off
 *        rather than filling a queue only discardSuppressedPayloads will drain.
 */
void MQTT::Publisher::hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_hotEnabled.load(std::memory_order_relaxed) || !licenseValid()) [[likely]]
    return;

  if (m_hotSparkplug.load(std::memory_order_relaxed)
      || m_workerMode.load(std::memory_order_relaxed) != static_cast<int>(Mode::RawRxData))
    return;

  if (!data || data->data.isEmpty() || !m_hotHasTopic.load(std::memory_order_relaxed))
    return;

  TimestampedRawBytes item{deviceId, data};
  m_rawBytesQueue.try_enqueue(std::move(item));
}

/**
 * @brief Enqueues a delimited frame's raw bytes for the script mode worker. Suppressed while a
 *        Sparkplug edge node owns the session, whose own namespace leaves the script queue dead.
 */
void MQTT::Publisher::hotpathTxRawFrame(int deviceId, const IO::CapturedDataPtr& data)
{
  if (!m_hotEnabled.load(std::memory_order_relaxed) || !licenseValid()) [[likely]]
    return;

  if (m_hotSparkplug.load(std::memory_order_relaxed)
      || m_workerMode.load(std::memory_order_relaxed) != static_cast<int>(Mode::ScriptDriven))
    return;

  if (!data || data->data.isEmpty() || !m_hotHasTopic.load(std::memory_order_relaxed))
    return;

  TimestampedRawBytes item{deviceId, data};
  m_rawFramesQueue.try_enqueue(std::move(item));
}

/**
 * @brief Forwards a posted notification to the worker.
 */
void MQTT::Publisher::onNotificationPosted(const QVariantMap& event)
{
  if (!m_enabled || !m_publishNotifications || !licenseValid()) [[likely]]
    return;

  const QString topic = m_notificationTopic.isEmpty() ? m_topicBase : m_notificationTopic;
  if (topic.isEmpty())
    return;

  const QJsonDocument doc(QJsonObject::fromVariantMap(event));
  const QByteArray payload = doc.toJson(QJsonDocument::Compact);

  QMetaObject::invokeMethod(m_worker,
                            "publishNotificationOnWorker",
                            Qt::QueuedConnection,
                            Q_ARG(QString, topic),
                            Q_ARG(QByteArray, payload));
}

/**
 * @brief Public publish slot used by scripts and tools.
 */
qint64 MQTT::Publisher::mqttPublish(const QString& topic,
                                    const QByteArray& payload,
                                    int qos,
                                    bool retain)
{
  if (!isConnected() || !licenseValid())
    return -1;

  QMetaObject::invokeMethod(m_worker,
                            "publishCustomOnWorker",
                            Qt::QueuedConnection,
                            Q_ARG(QString, topic),
                            Q_ARG(QByteArray, payload),
                            Q_ARG(int, qos),
                            Q_ARG(bool, retain));

  return 1;
}

//--------------------------------------------------------------------------------------------------
// Worker callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Mirrors the worker's broker state onto the main-thread atomic.
 */
void MQTT::Publisher::onWorkerBrokerStateChanged(int state)
{
  const bool connected = state == static_cast<int>(QMqttClient::Connected);
  m_isConnected.store(connected, std::memory_order_relaxed);
  Q_EMIT connectedChanged();

  if (connected)
    m_reportConnectionErrors = false;
}

/**
 * @brief Shows the test-connection probe result as a messagebox on the main thread.
 */
void MQTT::Publisher::onWorkerTestConnectionFinished(bool ok, const QString& detail)
{
  Misc::Utilities::showMessageBox(ok ? tr("MQTT broker reachable") : tr("MQTT broker unreachable"),
                                  detail,
                                  ok ? QMessageBox::Information : QMessageBox::Critical,
                                  tr("MQTT Test Connection"));
}

/**
 * @brief Displays the first error after a load attempt as a messagebox.
 */
void MQTT::Publisher::onWorkerBrokerError(const QString& message)
{
  if (m_reportConnectionErrors) {
    m_reportConnectionErrors = false;
    Misc::Utilities::showMessageBox(
      tr("MQTT broker connection failed"), message, QMessageBox::Critical, tr("MQTT Publisher"));
  }
}

/**
 * @brief Forwards a script compile/runtime error as a UI signal for the editor to surface.
 */
void MQTT::Publisher::onWorkerScriptError(const QString& message)
{
  Q_EMIT scriptError(message);
}

/**
 * @brief Emits statsChanged when the worker has published at least one new message.
 */
void MQTT::Publisher::emitStatsIfChanged()
{
  const auto current = m_messagesSent.load(std::memory_order_relaxed);
  if (current == m_messagesSentSeen)
    return;

  m_messagesSentSeen = current;
  Q_EMIT statsChanged();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the current commercial license grants MQTT publishing.
 */
bool MQTT::Publisher::licenseValid() const
{
  const auto& token = Licensing::CommercialToken::current();
  return token.isValid() && SS_LICENSE_GUARD();
}

/**
 * @brief Emits configurationChanged and schedules a debounced worker sync.
 */
void MQTT::Publisher::markConfigChanged()
{
  if (m_inApply)
    return;

  scheduleSyncToWorker();
  Q_EMIT configurationChanged();
}

/**
 * @brief Restarts the debounce timer; final fire flushes one snapshot to the worker.
 */
void MQTT::Publisher::scheduleSyncToWorker()
{
  m_syncTimer.start();
}

/**
 * @brief Builds a BrokerConfig snapshot from the current main-thread state.
 */
MQTT::BrokerConfig MQTT::Publisher::snapshotConfig() const
{
  BrokerConfig cfg;
  cfg.enabled              = m_enabled;
  cfg.sslEnabled           = m_tls.enabled();
  cfg.cleanSession         = m_cleanSession;
  cfg.publishNotifications = m_publishNotifications;
  cfg.mode                 = m_mode;
  cfg.peerVerifyDepth      = m_tls.peerVerifyDepth();
  cfg.port                 = m_port;
  cfg.keepAlive            = m_keepAlive;
  cfg.mqttVersion          = m_protocolVersion;
  cfg.sslProtocol          = m_tls.protocol();
  cfg.peerVerifyMode       = m_tls.peerVerifyMode();
  cfg.clientId             = m_clientId;
  cfg.hostname             = m_hostname;
  cfg.username             = m_username;
  cfg.password             = m_password;
  cfg.topicBase            = m_topicBase;
  cfg.notificationTopic    = m_notificationTopic;
  cfg.scriptCode           = m_scriptCode;
  cfg.scriptTopic          = m_scriptTopic;
  cfg.scriptLanguage       = m_scriptLanguage;
  cfg.sparkplugEnabled     = m_sparkplugEnabled;
  cfg.sparkplugGroupId     = m_sparkplugGroupId;
  cfg.sparkplugEdgeNode    = m_sparkplugEdgeNodeId;
  cfg.sparkplugDeviceId    = m_sparkplugDeviceId;
  cfg.caCertificates       = m_tls.caCertificates();
  cfg.clientCertificate    = m_tls.identity().certificate;
  cfg.clientPrivateKey     = m_tls.identity().privateKey;
  cfg.alpnProtocol         = m_tls.alpnPayload();
  return cfg;
}

/**
 * @brief Pushes a fresh BrokerConfig snapshot to the worker.
 */
void MQTT::Publisher::syncToWorker()
{
  if (!m_worker)
    return;

  QMetaObject::invokeMethod(m_worker,
                            "applyBrokerConfig",
                            Qt::QueuedConnection,
                            Q_ARG(MQTT::BrokerConfig, snapshotConfig()));
}

/**
 * @brief Updates the worker's drain timer interval to match the current publish frequency.
 */
void MQTT::Publisher::applyTimerInterval()
{
  const int interval = 1000 / std::clamp(m_publishFrequencyHz, kMinPublishHz, kMaxPublishHz);
  setTimerIntervalMs(interval);
}

/**
 * @brief Loads credentials for the current host:port from the vault into the in-memory fields.
 *        Callers re-parse the TLS identity themselves once their path state is final: parsing
 *        here would run against whatever paths happen to be current mid-restore.
 */
void MQTT::Publisher::reloadCredentialsFromVault()
{
  const auto creds = m_credentialVault.credentials(m_hostname, m_port);
  m_username       = creds.username;
  m_password       = creds.password;
  m_keyPassphrase  = m_credentialVault.keyPassphrase(m_hostname, m_port);
}

/**
 * @brief Writes the current in-memory credentials to the vault keyed by current host:port.
 */
void MQTT::Publisher::persistCredentialsToVault()
{
  if (m_hostname.isEmpty())
    return;

  m_credentialVault.setCredentials(m_hostname, m_port, m_username, m_password);
  m_credentialVault.setKeyPassphrase(m_hostname, m_port, m_keyPassphrase);
}

/**
 * @brief Re-parses the client certificate + key pair from the configured paths. A failed parse
 *        clears the identity (so a stale pair is never sent). Only the queued file-picker path
 *        passes interactive=true: a modal box from a plain setter would open a nested event
 *        loop inside the form model's itemChanged emission and re-enter the editing delegate.
 */
void MQTT::Publisher::reloadTlsIdentity(const bool interactive)
{
  const auto result = m_tls.reloadIdentity(m_keyPassphrase);
  if (result.ok())
    return;

  qCWarning(lcMqttPub) << "TLS identity rejected:" << tlsIdentityErrorString(result);
  if (interactive)
    Misc::Utilities::showMessageBox(tr("MQTT Client Certificate Error"),
                                    tlsIdentityErrorString(result),
                                    QMessageBox::Warning,
                                    tr("MQTT Publisher"));
}

#endif  // BUILD_COMMERCIAL
