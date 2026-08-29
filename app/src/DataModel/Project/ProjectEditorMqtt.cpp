/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "Misc/IconRegistry.h"
#include "ProjectEditorItemIds.h"
#include "ProjectEditorShared.h"

/**
 * @brief Opens the native MQTT publisher script editor dialog.
 */
void DataModel::ProjectEditor::openMqttScriptEditor()
{
#ifdef BUILD_COMMERCIAL
  auto& pub = m_mqttPublisher;

  if (!m_mqttScriptEditor) {
    m_mqttScriptEditor = new MQTT::PublisherScriptEditor(nullptr);

    connect(m_mqttScriptEditor,
            &MQTT::PublisherScriptEditor::scriptApplied,
            this,
            [this](const QString& code, int language) {
              auto& publisher = m_mqttPublisher;
              publisher.setScriptLanguage(language);
              publisher.setScriptCode(code);
            });
  }

  m_mqttScriptEditor->displayDialog(pub.scriptCode(), pub.scriptLanguage());
#endif
}

/**
 * @brief Rebuilds the MQTT Publisher form model from the live Publisher singleton.
 */
void DataModel::ProjectEditor::buildMqttPublisherModel()
{
  if (m_mqttPublisherModel) {
    m_mqttPublisherModel->disconnect(this);
    m_mqttPublisherModel->deleteLater();
    m_mqttPublisherModel = nullptr;
  }

  m_mqttPublisherModel = new CustomModel(this);

#ifdef BUILD_COMMERCIAL
  const auto& pub    = m_mqttPublisher;
  const bool enabled = pub.enabled();

  buildMqttPublishingSection(pub, enabled);
  buildMqttSparkplugSection(pub, enabled);
  buildMqttBrokerSection(pub, enabled);
  buildMqttSslSection(pub, enabled);
#endif

  connect(m_mqttPublisherModel,
          &CustomModel::itemChanged,
          this,
          &DataModel::ProjectEditor::onMqttPublisherItemChanged);

  Q_EMIT mqttPublisherModelChanged();
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the Publishing section (master toggle, mode, rate, topic, notifications).
 */
void DataModel::ProjectEditor::buildMqttPublishingSection(const MQTT::Publisher& pub, bool enabled)
{
  auto* pubHdr = new QStandardItem();
  pubHdr->setData(SectionHeader, WidgetType);
  pubHdr->setData(tr("Publishing"), PlaceholderValue);
  static auto& registry = Misc::IconRegistry::instance();
  pubHdr->setData(registry.iconById(QStringLiteral("editor/output-range"), 16), ParameterIcon);
  m_mqttPublisherModel->appendRow(pubHdr);

  auto* enabledItem = new QStandardItem();
  enabledItem->setEditable(true);
  enabledItem->setData(true, Active);
  enabledItem->setData(CheckBox, WidgetType);
  enabledItem->setData(enabled, EditableValue);
  enabledItem->setData(kMqttPublisher_Enabled, ParameterType);
  enabledItem->setData(tr("Enable Publishing"), ParameterName);
  enabledItem->setData(tr("Broadcast frames, raw bytes and notifications to the broker"),
                       ParameterDescription);
  m_mqttPublisherModel->appendRow(enabledItem);

  auto* modeItem = new QStandardItem();
  modeItem->setEditable(true);
  modeItem->setData(enabled, Active);
  modeItem->setData(ComboBox, WidgetType);
  modeItem->setData(pub.modes(), ComboBoxData);
  modeItem->setData(pub.mode(), EditableValue);
  modeItem->setData(kMqttPublisher_Mode, ParameterType);
  modeItem->setData(tr("Payload"), ParameterName);
  modeItem->setData(tr("Selects what gets published: parsed dashboard data or raw RX bytes"),
                    ParameterDescription);
  m_mqttPublisherModel->appendRow(modeItem);

  auto* freqItem = new QStandardItem();
  freqItem->setEditable(true);
  freqItem->setData(enabled, Active);
  freqItem->setData(IntField, WidgetType);
  freqItem->setData(pub.publishFrequency(), EditableValue);
  freqItem->setData(kMqttPublisher_PublishFrequency, ParameterType);
  freqItem->setData(tr("Publish Rate (Hz)"), ParameterName);
  freqItem->setData(tr("How many times per second to publish (1-30 Hz). Higher rates increase "
                       "broker load; dashboard data is rate-limited so a slow broker never blocks "
                       "frame parsing."),
                    ParameterDescription);
  m_mqttPublisherModel->appendRow(freqItem);

  auto* topicItem = new QStandardItem();
  topicItem->setEditable(true);
  topicItem->setData(enabled, Active);
  topicItem->setData(TextField, WidgetType);
  topicItem->setData(pub.topicBase(), EditableValue);
  topicItem->setData(kMqttPublisher_TopicBase, ParameterType);
  topicItem->setData(tr("Topic Base"), ParameterName);
  topicItem->setData(tr("serial-studio/device"), PlaceholderValue);
  topicItem->setData(tr("Base topic used for frame and raw-byte publishing"), ParameterDescription);
  m_mqttPublisherModel->appendRow(topicItem);

  if (pub.mode() == static_cast<int>(MQTT::Publisher::Mode::ScriptDriven)) {
    auto* scriptTopicItem = new QStandardItem();
    scriptTopicItem->setEditable(true);
    scriptTopicItem->setData(enabled, Active);
    scriptTopicItem->setData(TextField, WidgetType);
    scriptTopicItem->setData(pub.scriptTopic(), EditableValue);
    scriptTopicItem->setData(kMqttPublisher_ScriptTopic, ParameterType);
    scriptTopicItem->setData(tr("Script Topic"), ParameterName);
    scriptTopicItem->setData(tr("Defaults to Topic Base when empty"), PlaceholderValue);
    scriptTopicItem->setData(tr("Topic the user script publishes to"), ParameterDescription);
    m_mqttPublisherModel->appendRow(scriptTopicItem);
  }

  auto* notifyItem = new QStandardItem();
  notifyItem->setEditable(true);
  notifyItem->setData(enabled, Active);
  notifyItem->setData(CheckBox, WidgetType);
  notifyItem->setData(pub.publishNotifications(), EditableValue);
  notifyItem->setData(kMqttPublisher_PublishNotifications, ParameterType);
  notifyItem->setData(tr("Publish Notifications"), ParameterName);
  notifyItem->setData(tr("Mirror dashboard notifications to a dedicated topic"),
                      ParameterDescription);
  m_mqttPublisherModel->appendRow(notifyItem);

  if (pub.publishNotifications()) {
    auto* notifyTopicItem = new QStandardItem();
    notifyTopicItem->setEditable(true);
    notifyTopicItem->setData(enabled, Active);
    notifyTopicItem->setData(TextField, WidgetType);
    notifyTopicItem->setData(pub.notificationTopic(), EditableValue);
    notifyTopicItem->setData(kMqttPublisher_NotificationTopic, ParameterType);
    notifyTopicItem->setData(tr("Notification Topic"), ParameterName);
    notifyTopicItem->setData(tr("Defaults to Topic Base when empty"), PlaceholderValue);
    notifyTopicItem->setData(tr("Topic where dashboard notifications are mirrored"),
                             ParameterDescription);
    m_mqttPublisherModel->appendRow(notifyTopicItem);
  }
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the Sparkplug section (edge-node toggle and identity). The identity rows appear
 *        only once the toggle is on, because an edge node without a group and a node id has no
 *        topic namespace to publish into.
 */
void DataModel::ProjectEditor::buildMqttSparkplugSection(const MQTT::Publisher& pub, bool enabled)
{
  auto* sparkplugHdr = new QStandardItem();
  sparkplugHdr->setData(SectionHeader, WidgetType);
  sparkplugHdr->setData(tr("Sparkplug"), PlaceholderValue);
  static auto& registry = Misc::IconRegistry::instance();
  sparkplugHdr->setData(registry.iconById(QStringLiteral("editor/mqtt-publisher"), 16),
                        ParameterIcon);
  m_mqttPublisherModel->appendRow(sparkplugHdr);

  auto* sparkplugItem = new QStandardItem();
  sparkplugItem->setEditable(true);
  sparkplugItem->setData(enabled, Active);
  sparkplugItem->setData(CheckBox, WidgetType);
  sparkplugItem->setData(pub.sparkplugEnabled(), EditableValue);
  sparkplugItem->setData(kMqttPublisher_SparkplugEnabled, ParameterType);
  sparkplugItem->setData(tr("Publish as Edge Node"), ParameterName);
  sparkplugItem->setData(tr("Publish datasets in the Sparkplug B namespace instead of the payload "
                            "selected above, with birth, death and rebirth handling"),
                         ParameterDescription);
  m_mqttPublisherModel->appendRow(sparkplugItem);

  if (!pub.sparkplugEnabled())
    return;

  auto* groupItem = new QStandardItem();
  groupItem->setEditable(true);
  groupItem->setData(enabled, Active);
  groupItem->setData(TextField, WidgetType);
  groupItem->setData(pub.sparkplugGroupId(), EditableValue);
  groupItem->setData(kMqttPublisher_SparkplugGroupId, ParameterType);
  groupItem->setData(tr("Group ID"), ParameterName);
  groupItem->setData(tr("Plant1"), PlaceholderValue);
  groupItem->setData(tr("Logical group this edge node belongs to"), ParameterDescription);
  m_mqttPublisherModel->appendRow(groupItem);

  auto* nodeItem = new QStandardItem();
  nodeItem->setEditable(true);
  nodeItem->setData(enabled, Active);
  nodeItem->setData(TextField, WidgetType);
  nodeItem->setData(pub.sparkplugEdgeNodeId(), EditableValue);
  nodeItem->setData(kMqttPublisher_SparkplugEdgeNode, ParameterType);
  nodeItem->setData(tr("Edge Node ID"), ParameterName);
  nodeItem->setData(tr("SerialStudio"), PlaceholderValue);
  nodeItem->setData(tr("Identifies this node inside the group"), ParameterDescription);
  m_mqttPublisherModel->appendRow(nodeItem);

  auto* deviceItem = new QStandardItem();
  deviceItem->setEditable(true);
  deviceItem->setData(enabled, Active);
  deviceItem->setData(TextField, WidgetType);
  deviceItem->setData(pub.sparkplugDeviceId(), EditableValue);
  deviceItem->setData(kMqttPublisher_SparkplugDevice, ParameterType);
  deviceItem->setData(tr("Device ID"), ParameterName);
  deviceItem->setData(tr("Optional"), PlaceholderValue);
  deviceItem->setData(tr("When set, datasets are published as a device of this node"),
                      ParameterDescription);
  m_mqttPublisherModel->appendRow(deviceItem);
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the Broker section (host, port, credentials, protocol, keep-alive).
 */
void DataModel::ProjectEditor::buildMqttBrokerSection(const MQTT::Publisher& pub, bool enabled)
{
  auto* brokerHdr = new QStandardItem();
  brokerHdr->setData(SectionHeader, WidgetType);
  brokerHdr->setData(tr("Broker"), PlaceholderValue);
  static auto& registry = Misc::IconRegistry::instance();
  brokerHdr->setData(registry.iconById(QStringLiteral("editor/mqtt-broker"), 16), ParameterIcon);
  m_mqttPublisherModel->appendRow(brokerHdr);

  auto* hostItem = new QStandardItem();
  hostItem->setEditable(true);
  hostItem->setData(enabled, Active);
  hostItem->setData(TextField, WidgetType);
  hostItem->setData(pub.hostname(), EditableValue);
  hostItem->setData(kMqttPublisher_Hostname, ParameterType);
  hostItem->setData(tr("Hostname"), ParameterName);
  hostItem->setData(tr("broker.hivemq.com"), PlaceholderValue);
  hostItem->setData(tr("Hostname or IP address of the MQTT broker"), ParameterDescription);
  m_mqttPublisherModel->appendRow(hostItem);

  auto* portItem = new QStandardItem();
  portItem->setEditable(true);
  portItem->setData(enabled, Active);
  portItem->setData(IntField, WidgetType);
  portItem->setData(static_cast<int>(pub.port()), EditableValue);
  portItem->setData(kMqttPublisher_Port, ParameterType);
  portItem->setData(tr("Port"), ParameterName);
  portItem->setData(tr("TCP port exposed by the broker (1883 plain, 8883 TLS)"),
                    ParameterDescription);
  m_mqttPublisherModel->appendRow(portItem);

  auto* customClientIdItem = new QStandardItem();
  customClientIdItem->setEditable(true);
  customClientIdItem->setData(enabled, Active);
  customClientIdItem->setData(CheckBox, WidgetType);
  customClientIdItem->setData(pub.customClientId(), EditableValue);
  customClientIdItem->setData(kMqttPublisher_CustomClientId, ParameterType);
  customClientIdItem->setData(tr("Custom Client ID"), ParameterName);
  customClientIdItem->setData(
    tr("Off: a fresh random id is generated on every project load. On: use the id below."),
    ParameterDescription);
  m_mqttPublisherModel->appendRow(customClientIdItem);

  if (pub.customClientId()) {
    auto* clientIdItem = new QStandardItem();
    clientIdItem->setEditable(true);
    clientIdItem->setData(enabled, Active);
    clientIdItem->setData(TextField, WidgetType);
    clientIdItem->setData(pub.clientId(), EditableValue);
    clientIdItem->setData(kMqttPublisher_ClientId, ParameterType);
    clientIdItem->setData(tr("Client ID"), ParameterName);
    clientIdItem->setData(tr("Identifier sent to the broker on CONNECT"), ParameterDescription);
    m_mqttPublisherModel->appendRow(clientIdItem);
  }

  buildMqttBrokerCredentials(pub, enabled);

  auto* versionItem = new QStandardItem();
  versionItem->setEditable(true);
  versionItem->setData(enabled, Active);
  versionItem->setData(ComboBox, WidgetType);
  versionItem->setData(pub.mqttVersions(), ComboBoxData);
  versionItem->setData(static_cast<int>(pub.mqttVersion()), EditableValue);
  versionItem->setData(kMqttPublisher_MqttVersion, ParameterType);
  versionItem->setData(tr("Protocol Version"), ParameterName);
  versionItem->setData(tr("MQTT protocol revision used on CONNECT"), ParameterDescription);
  m_mqttPublisherModel->appendRow(versionItem);

  auto* keepAliveItem = new QStandardItem();
  keepAliveItem->setEditable(true);
  keepAliveItem->setData(enabled, Active);
  keepAliveItem->setData(IntField, WidgetType);
  keepAliveItem->setData(static_cast<int>(pub.keepAlive()), EditableValue);
  keepAliveItem->setData(kMqttPublisher_KeepAlive, ParameterType);
  keepAliveItem->setData(tr("Keep Alive (s)"), ParameterName);
  keepAliveItem->setData(tr("Seconds between PINGREQ packets when idle"), ParameterDescription);
  m_mqttPublisherModel->appendRow(keepAliveItem);

  auto* cleanItem = new QStandardItem();
  cleanItem->setEditable(true);
  cleanItem->setData(enabled, Active);
  cleanItem->setData(CheckBox, WidgetType);
  cleanItem->setData(pub.cleanSession(), EditableValue);
  cleanItem->setData(kMqttPublisher_CleanSession, ParameterType);
  cleanItem->setData(tr("Clean Session"), ParameterName);
  cleanItem->setData(tr("Discard any persistent session state on CONNECT"), ParameterDescription);
  m_mqttPublisherModel->appendRow(cleanItem);
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the broker authentication rows (username, password).
 */
void DataModel::ProjectEditor::buildMqttBrokerCredentials(const MQTT::Publisher& pub, bool enabled)
{
  auto* userItem = new QStandardItem();
  userItem->setEditable(true);
  userItem->setData(enabled, Active);
  userItem->setData(TextField, WidgetType);
  userItem->setData(pub.username(), EditableValue);
  userItem->setData(kMqttPublisher_Username, ParameterType);
  userItem->setData(tr("Username"), ParameterName);
  userItem->setData(tr("Username for broker authentication (leave empty for anonymous)"),
                    ParameterDescription);
  m_mqttPublisherModel->appendRow(userItem);

  auto* passItem = new QStandardItem();
  passItem->setEditable(true);
  passItem->setData(enabled, Active);
  passItem->setData(PasswordField, WidgetType);
  passItem->setData(pub.password(), EditableValue);
  passItem->setData(kMqttPublisher_Password, ParameterType);
  passItem->setData(tr("Password"), ParameterName);
  passItem->setData(tr("Password for broker authentication"), ParameterDescription);
  m_mqttPublisherModel->appendRow(passItem);
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the mutual-TLS client identity rows (certificate, key, passphrase, ALPN).
 */
static void appendMqttClientIdentityRows(DataModel::CustomModel* model,
                                         const MQTT::Publisher& pub,
                                         bool enabled)
{
  using Editor = DataModel::ProjectEditor;

  auto* certPathItem = new QStandardItem();
  certPathItem->setEditable(true);
  certPathItem->setData(enabled, Editor::Active);
  certPathItem->setData(Editor::TextField, Editor::WidgetType);
  certPathItem->setData(pub.clientCertificatePath(), Editor::EditableValue);
  certPathItem->setData(kMqttPublisher_ClientCertPath, Editor::ParameterType);
  certPathItem->setData(Editor::tr("Client Certificate"), Editor::ParameterName);
  certPathItem->setData(Editor::tr("Optional (mutual TLS)"), Editor::PlaceholderValue);
  certPathItem->setData(Editor::tr("PEM certificate presented to brokers that require mutual "
                                   "TLS (AWS IoT Core)"),
                        Editor::ParameterDescription);
  model->appendRow(certPathItem);

  auto* keyPathItem = new QStandardItem();
  keyPathItem->setEditable(true);
  keyPathItem->setData(enabled, Editor::Active);
  keyPathItem->setData(Editor::TextField, Editor::WidgetType);
  keyPathItem->setData(pub.privateKeyPath(), Editor::EditableValue);
  keyPathItem->setData(kMqttPublisher_PrivateKeyPath, Editor::ParameterType);
  keyPathItem->setData(Editor::tr("Private Key"), Editor::ParameterName);
  keyPathItem->setData(Editor::tr("Defaults to the certificate file"), Editor::PlaceholderValue);
  keyPathItem->setData(Editor::tr("PEM private key matching the client certificate"),
                       Editor::ParameterDescription);
  model->appendRow(keyPathItem);

  auto* passphraseItem = new QStandardItem();
  passphraseItem->setEditable(true);
  passphraseItem->setData(enabled, Editor::Active);
  passphraseItem->setData(Editor::PasswordField, Editor::WidgetType);
  passphraseItem->setData(pub.keyPassphrase(), Editor::EditableValue);
  passphraseItem->setData(kMqttPublisher_KeyPassphrase, Editor::ParameterType);
  passphraseItem->setData(Editor::tr("Key Passphrase"), Editor::ParameterName);
  passphraseItem->setData(Editor::tr("Passphrase for an encrypted private key (stored in the "
                                     "encrypted vault, never in the project file)"),
                          Editor::ParameterDescription);
  model->appendRow(passphraseItem);

  auto* alpnItem = new QStandardItem();
  alpnItem->setEditable(true);
  alpnItem->setData(enabled, Editor::Active);
  alpnItem->setData(Editor::CheckBox, Editor::WidgetType);
  alpnItem->setData(pub.alpnEnabled(), Editor::EditableValue);
  alpnItem->setData(kMqttPublisher_AlpnEnabled, Editor::ParameterType);
  alpnItem->setData(Editor::tr("ALPN (MQTT over port 443)"), Editor::ParameterName);
  alpnItem->setData(Editor::tr("Announce an ALPN protocol so brokers can serve MQTT on port "
                               "443 behind firewalls"),
                    Editor::ParameterDescription);
  model->appendRow(alpnItem);

  if (!pub.alpnEnabled())
    return;

  auto* alpnProtoItem = new QStandardItem();
  alpnProtoItem->setEditable(true);
  alpnProtoItem->setData(enabled, Editor::Active);
  alpnProtoItem->setData(Editor::TextField, Editor::WidgetType);
  alpnProtoItem->setData(pub.alpnProtocol(), Editor::EditableValue);
  alpnProtoItem->setData(kMqttPublisher_AlpnProtocol, Editor::ParameterType);
  alpnProtoItem->setData(Editor::tr("ALPN Protocol"), Editor::ParameterName);
  alpnProtoItem->setData(QStringLiteral("x-amzn-mqtt-ca"), Editor::PlaceholderValue);
  alpnProtoItem->setData(Editor::tr("Protocol name announced during the TLS handshake (AWS "
                                    "IoT uses x-amzn-mqtt-ca)"),
                         Editor::ParameterDescription);
  model->appendRow(alpnProtoItem);
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Appends the SSL/TLS section (toggle, protocol, peer verification, depth).
 */
void DataModel::ProjectEditor::buildMqttSslSection(const MQTT::Publisher& pub, bool enabled)
{
  auto* sslHdr = new QStandardItem();
  sslHdr->setData(SectionHeader, WidgetType);
  sslHdr->setData(tr("SSL / TLS"), PlaceholderValue);
  static auto& registry = Misc::IconRegistry::instance();
  sslHdr->setData(registry.iconById(QStringLiteral("editor/mqtt-ssl"), 16), ParameterIcon);
  m_mqttPublisherModel->appendRow(sslHdr);

  auto* sslItem = new QStandardItem();
  sslItem->setEditable(true);
  sslItem->setData(enabled, Active);
  sslItem->setData(CheckBox, WidgetType);
  sslItem->setData(pub.sslEnabled(), EditableValue);
  sslItem->setData(kMqttPublisher_SslEnabled, ParameterType);
  sslItem->setData(tr("Use SSL/TLS"), ParameterName);
  sslItem->setData(tr("Tunnel the broker connection over TLS"), ParameterDescription);
  m_mqttPublisherModel->appendRow(sslItem);

  if (!pub.sslEnabled())
    return;

  auto* sslProtoItem = new QStandardItem();
  sslProtoItem->setEditable(true);
  sslProtoItem->setData(enabled, Active);
  sslProtoItem->setData(ComboBox, WidgetType);
  sslProtoItem->setData(pub.sslProtocols(), ComboBoxData);
  sslProtoItem->setData(static_cast<int>(pub.sslProtocol()), EditableValue);
  sslProtoItem->setData(kMqttPublisher_SslProtocol, ParameterType);
  sslProtoItem->setData(tr("Protocol"), ParameterName);
  sslProtoItem->setData(tr("Negotiated TLS protocol family"), ParameterDescription);
  m_mqttPublisherModel->appendRow(sslProtoItem);

  auto* peerModeItem = new QStandardItem();
  peerModeItem->setEditable(true);
  peerModeItem->setData(enabled, Active);
  peerModeItem->setData(ComboBox, WidgetType);
  peerModeItem->setData(pub.peerVerifyModes(), ComboBoxData);
  peerModeItem->setData(static_cast<int>(pub.peerVerifyMode()), EditableValue);
  peerModeItem->setData(kMqttPublisher_PeerVerifyMode, ParameterType);
  peerModeItem->setData(tr("Peer Verify"), ParameterName);
  peerModeItem->setData(tr("How strictly the broker's certificate chain is validated"),
                        ParameterDescription);
  m_mqttPublisherModel->appendRow(peerModeItem);

  auto* peerDepthItem = new QStandardItem();
  peerDepthItem->setEditable(true);
  peerDepthItem->setData(enabled, Active);
  peerDepthItem->setData(IntField, WidgetType);
  peerDepthItem->setData(pub.peerVerifyDepth(), EditableValue);
  peerDepthItem->setData(kMqttPublisher_PeerVerifyDepth, ParameterType);
  peerDepthItem->setData(tr("Verify Depth"), ParameterName);
  peerDepthItem->setData(tr("Maximum certificate chain length accepted (0 = unlimited)"),
                         ParameterDescription);
  m_mqttPublisherModel->appendRow(peerDepthItem);

  appendMqttClientIdentityRows(m_mqttPublisherModel, pub, enabled);
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Applies the mutual-TLS identity edits that need no model rebuild; returns true when
 *        the parameter was one of them.
 */
static bool applyMqttClientIdentityEdit(MQTT::Publisher& pub, int type, const QVariant& value)
{
  switch (type) {
    case kMqttPublisher_ClientCertPath:
      pub.setClientCertificatePath(value.toString());
      return true;
    case kMqttPublisher_PrivateKeyPath:
      pub.setPrivateKeyPath(value.toString());
      return true;
    case kMqttPublisher_KeyPassphrase:
      pub.setKeyPassphrase(value.toString());
      return true;
    case kMqttPublisher_AlpnProtocol:
      pub.setAlpnProtocol(value.toString());
      return true;
    default:
      return false;
  }
}
#endif

#ifdef BUILD_COMMERCIAL
/**
 * @brief Applies the Sparkplug identity edits that need no model rebuild; returns true when the
 *        parameter was one of them.
 */
static bool applyMqttSparkplugEdit(MQTT::Publisher& pub, int type, const QVariant& value)
{
  switch (type) {
    case kMqttPublisher_SparkplugGroupId:
      pub.setSparkplugGroupId(value.toString());
      return true;
    case kMqttPublisher_SparkplugEdgeNode:
      pub.setSparkplugEdgeNodeId(value.toString());
      return true;
    case kMqttPublisher_SparkplugDevice:
      pub.setSparkplugDeviceId(value.toString());
      return true;
    default:
      return false;
  }
}
#endif

/**
 * @brief Pushes MQTT Publisher form edits back into the live Publisher singleton.
 */
void DataModel::ProjectEditor::onMqttPublisherItemChanged(QStandardItem* item)
{
  if (!item)
    return;

#ifdef BUILD_COMMERCIAL
  auto& pub        = m_mqttPublisher;
  const auto type  = item->data(ParameterType).toInt();
  const auto value = item->data(EditableValue);

  if (applyMqttClientIdentityEdit(pub, type, value))
    return;

  if (applyMqttSparkplugEdit(pub, type, value))
    return;

  switch (type) {
    case kMqttPublisher_Enabled:
      pub.setEnabled(value.toBool());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_Mode:
      pub.setMode(value.toInt());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_PublishFrequency:
      pub.setPublishFrequency(value.toInt());
      break;
    case kMqttPublisher_TopicBase:
      pub.setTopicBase(value.toString());
      break;
    case kMqttPublisher_ScriptTopic:
      pub.setScriptTopic(value.toString());
      break;
    case kMqttPublisher_ScriptCode:
      pub.setScriptCode(value.toString());
      break;
    case kMqttPublisher_PublishNotifications:
      pub.setPublishNotifications(value.toBool());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_NotificationTopic:
      pub.setNotificationTopic(value.toString());
      break;
    case kMqttPublisher_SparkplugEnabled:
      pub.setSparkplugEnabled(value.toBool());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_Hostname:
      pub.setHostname(value.toString());
      break;
    case kMqttPublisher_Port:
      pub.setPort(static_cast<quint16>(value.toInt()));
      break;
    case kMqttPublisher_CustomClientId:
      pub.setCustomClientId(value.toBool());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_ClientId:
      pub.setClientId(value.toString());
      break;
    case kMqttPublisher_Username:
      pub.setUsername(value.toString());
      break;
    case kMqttPublisher_Password:
      pub.setPassword(value.toString());
      break;
    case kMqttPublisher_MqttVersion:
      pub.setMqttVersion(static_cast<quint8>(value.toInt()));
      break;
    case kMqttPublisher_KeepAlive:
      pub.setKeepAlive(static_cast<quint16>(value.toInt()));
      break;
    case kMqttPublisher_CleanSession:
      pub.setCleanSession(value.toBool());
      break;
    case kMqttPublisher_SslEnabled:
      pub.setSslEnabled(value.toBool());
      buildMqttPublisherModel();
      return;
    case kMqttPublisher_SslProtocol:
      pub.setSslProtocol(static_cast<quint8>(value.toInt()));
      break;
    case kMqttPublisher_PeerVerifyMode:
      pub.setPeerVerifyMode(static_cast<quint8>(value.toInt()));
      break;
    case kMqttPublisher_PeerVerifyDepth:
      pub.setPeerVerifyDepth(value.toInt());
      break;
    case kMqttPublisher_AlpnEnabled:
      pub.setAlpnEnabled(value.toBool());
      buildMqttPublisherModel();
      return;
    default:
      break;
  }
#else
  Q_UNUSED(item);
#endif
}
