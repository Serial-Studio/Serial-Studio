#include "networkmock.h"
#include <qmqtt_client.h>
#include <qmqtt_message.h>
#include <qmqtt_frame.h>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QDataStream>
#include <gmock/gmock.h>

using namespace testing;

const quint8 CONNECT_TYPE = 0x10;
const quint8 CONNACK_TYPE = 0x20;
const quint8 PUBLISH_TYPE = 0x30;
const quint8 PUBACK_TYPE = 0x40;
const quint8 PUBREC_TYPE = 0x50;
const quint8 PUBREL_TYPE = 0x60;
const quint8 PUBCOMP_TYPE = 0x70;
const quint8 SUBSCRIBE_TYPE = 0x80;
const quint8 SUBACK_TYPE = 0x90;
const quint8 UNSUBSCRIBE_TYPE = 0xA0;
const quint8 UNSUBACK_TYPE = 0xB0;
const quint8 PINGREQ_TYPE = 0xC0;
const quint8 PINGRESP_TYPE = 0xD0;
const quint8 DISCONNECT_TYPE = 0xE0;

const quint8 QOS0 = 0x00;
const quint8 QOS1 = 0x02;
const quint8 QOS2 = 0x04;

const QHostAddress HOST_ADDRESS = QHostAddress("8.8.8.8");
const QString HOST_NAME = QStringLiteral("test.mosquitto.org");
const quint16 PORT = 8883;

class ClientTest : public Test
{
public:
  explicit ClientTest()
    : _networkMock(new NetworkMock)
    , _client(new QMQTT::Client(_networkMock))
  {
    qRegisterMetaType<QMQTT::ClientError>("QMQTT::ClientError");
  }
  virtual ~ClientTest() {}

  NetworkMock *_networkMock;
  QSharedPointer<QMQTT::Client> _client;

  quint8 getHeaderType(const quint8 header) { return header & 0xF0; }
};

TEST_F(ClientTest, constructorWithNoParameters_Test)
{
  EXPECT_EQ(QHostAddress::LocalHost, _client->host());
  EXPECT_EQ(1883, _client->port());
  EXPECT_THAT(_client->parent(), IsNull());
}

TEST_F(ClientTest, constructorWithHost_Test)
{
  _client.reset(new QMQTT::Client(HOST_ADDRESS));

  EXPECT_EQ(HOST_ADDRESS, _client->host());
  EXPECT_EQ(1883, _client->port());
  EXPECT_THAT(_client->parent(), IsNull());
}

TEST_F(ClientTest, constructorWithHostAndPort_Test)
{
  _client.reset(new QMQTT::Client(HOST_ADDRESS, PORT));

  EXPECT_EQ(HOST_ADDRESS, _client->host());
  EXPECT_EQ(PORT, _client->port());
  EXPECT_THAT(_client->parent(), IsNull());
}

TEST_F(ClientTest, constructorWithHostPortAndParent_Test)
{
  QObject parent;
  _client.reset(new QMQTT::Client(HOST_ADDRESS, PORT, &parent));

  EXPECT_EQ(HOST_ADDRESS, _client->host());
  EXPECT_EQ(PORT, _client->port());
  EXPECT_EQ(&parent, _client->parent());
  _client.reset();
}

TEST_F(ClientTest, constructorWithHostPortAndSsl_Test)
{
  _client.reset(new QMQTT::Client(HOST_NAME, PORT, true, false));

  EXPECT_EQ(HOST_NAME, _client->hostName());
  EXPECT_EQ(PORT, _client->port());
  EXPECT_THAT(_client->parent(), IsNull());
}

TEST_F(ClientTest, constructorWithHostPortSslAndParent_Test)
{
  QObject parent;
  _client.reset(new QMQTT::Client(HOST_NAME, PORT, true, false, &parent));

  EXPECT_EQ(HOST_NAME, _client->hostName());
  EXPECT_EQ(PORT, _client->port());
  EXPECT_EQ(&parent, _client->parent());
  _client.reset();
}

TEST_F(ClientTest, hostReturnsHostValue_Test)
{
  EXPECT_EQ(QHostAddress::LocalHost, _client->host());
}

TEST_F(ClientTest, setHostSetsHostValue_Test)
{
  _client->setHost(HOST_ADDRESS);
  EXPECT_EQ(HOST_ADDRESS, _client->host());
}

TEST_F(ClientTest, hostNameReturnsHostNameValue_Test)
{
  EXPECT_EQ(QString(), _client->hostName());
}

TEST_F(ClientTest, setHostNameSetsHostNameValue_Test)
{
  _client->setHostName(HOST_NAME);
  EXPECT_EQ(HOST_NAME, _client->hostName());
}

TEST_F(ClientTest, portReturnsPortValue_Test)
{
  EXPECT_EQ(1883, _client->port());
}

TEST_F(ClientTest, setPortSetsPortValue_Test)
{
  _client->setPort(PORT);
  EXPECT_EQ(PORT, _client->port());
}

TEST_F(ClientTest, clientIdReturnsClientId_Test)
{
  EXPECT_FALSE(_client->clientId().isEmpty());
}

TEST_F(ClientTest, setClientIdSetsClientId_Test)
{
  _client->setClientId("Client");
  EXPECT_EQ("Client", _client->clientId());
}

TEST_F(ClientTest, setClientIdCannotSetEmptyString_Test)
{
  _client->setClientId("");
  EXPECT_FALSE(_client->clientId().isEmpty());
}

TEST_F(ClientTest, usernameReturnsUsername_Test)
{
  EXPECT_EQ(QString(), _client->username());
}

TEST_F(ClientTest, setUsernameSetsUsername_Test)
{
  _client->setUsername("Username");
  EXPECT_EQ("Username", _client->username());
}

TEST_F(ClientTest, passwordReturnsPassword_Test)
{
  EXPECT_EQ(QString(), _client->password());
}

TEST_F(ClientTest, setPasswordSetsPassword_Test)
{
  _client->setPassword("Password");
  EXPECT_EQ("Password", _client->password());
}

TEST_F(ClientTest, keepAliveReturnsKeepAlive_Test)
{
  EXPECT_EQ(300, _client->keepAlive());
}

TEST_F(ClientTest, setKeepAliveSetsKeepAlive_Test)
{
  _client->setKeepAlive(400);
  EXPECT_EQ(400, _client->keepAlive());
}

TEST_F(ClientTest, CleanSessionReturnsCleanSession_Test)
{
  EXPECT_FALSE(_client->cleanSession());
}

TEST_F(ClientTest, setCleanSessionSetsCleanSession_Test)
{
  _client->setCleanSession(true);
  EXPECT_TRUE(_client->cleanSession());
}

TEST_F(ClientTest, connectToHostWillCallNetworkConnectToHost)
{
  EXPECT_CALL(*_networkMock, connectToHost(TypedEq<const QHostAddress &>(
                                               QHostAddress::LocalHost),
                                           Eq(1883)));
  _client->connectToHost();
}

TEST_F(ClientTest, isConnectedToHostIsFalseWhenNetworkIsNotConnected_Test)
{
  EXPECT_CALL(*_networkMock, isConnectedToHost()).WillOnce(Return(false));
  EXPECT_FALSE(_client->isConnectedToHost());
}

TEST_F(ClientTest, isConnectedToHostIsTrueWhenNetworkIsConnectedToHost_Test)
{
  EXPECT_CALL(*_networkMock, isConnectedToHost()).WillOnce(Return(true));
  EXPECT_TRUE(_client->isConnectedToHost());
}

TEST_F(ClientTest, autoReconnectIsTrueIfNetworkAutoReconnectIsTrue_Test)
{
  EXPECT_CALL(*_networkMock, autoReconnect()).WillOnce(Return(true));
  EXPECT_TRUE(_client->autoReconnect());
}

TEST_F(ClientTest, autoReconnectIsFalseIfNetworkAutoReconnectIsFalse_Test)
{
  EXPECT_CALL(*_networkMock, autoReconnect()).WillOnce(Return(false));
  EXPECT_FALSE(_client->autoReconnect());
}

TEST_F(ClientTest, setAutoReconnectSetsNetworkAutoReconnect_Test)
{
  EXPECT_CALL(*_networkMock, setAutoReconnect(true));
  _client->setAutoReconnect(true);
}

TEST_F(ClientTest,
       setAutoReconnectIntervalSetsNetworkAutoReconnectInterval_Test)
{
  EXPECT_CALL(*_networkMock, setAutoReconnectInterval(10000));
  _client->setAutoReconnectInterval(10000);
}

TEST_F(ClientTest, autoReconnectIntervalIsValueOfNetworkAutoReconnect_Test)
{
  EXPECT_CALL(*_networkMock, autoReconnectInterval()).WillOnce(Return(10000));
  EXPECT_TRUE(_client->autoReconnectInterval());
}

TEST_F(ClientTest, willTopicDefaultsToNull_Test)
{
  EXPECT_TRUE(_client->willTopic().isNull());
}

TEST_F(ClientTest, setWillTopicSetsAWillTopic_Test)
{
  _client->setWillTopic("topic");
  EXPECT_EQ("topic", _client->willTopic());
}

TEST_F(ClientTest, willQosDefaultsToZero_Test)
{
  EXPECT_EQ(QOS0, _client->willQos());
}

TEST_F(ClientTest, setWillQosSetsAWillQos_Test)
{
  _client->setWillQos(QOS1);
  EXPECT_EQ(QOS1, _client->willQos());
}

TEST_F(ClientTest, willRetainDefaultsToFalse_Test)
{
  EXPECT_FALSE(_client->willRetain());
}

TEST_F(ClientTest, setWillRetainSetsAWillRetainToTrue_Test)
{
  _client->setWillRetain(true);
  EXPECT_TRUE(_client->willRetain());
}

TEST_F(ClientTest, willMessageDefaultsToNull_Test)
{
  EXPECT_TRUE(_client->willMessage().isNull());
}

TEST_F(ClientTest, setWillMessageSetsAWillMessageTest)
{
  _client->setWillMessage("message");
  EXPECT_EQ("message", _client->willMessage());
}

TEST_F(ClientTest, connectionStateReturnsStateInit_Test)
{
  EXPECT_EQ(QMQTT::STATE_INIT, _client->connectionState());
}

TEST_F(ClientTest, connectionStateReturnsStateInitEvenAfterConnected_Test)
{
  EXPECT_CALL(*_networkMock, sendFrame(_));

  emit _networkMock->connected();

  EXPECT_EQ(QMQTT::STATE_INIT, _client->connectionState());
}

TEST_F(ClientTest, connectSendsConnectMessage_Test)
{
  QMQTT::Frame frame;
  EXPECT_CALL(*_networkMock, sendFrame(_)).WillOnce(SaveArg<0>(&frame));

  emit _networkMock->connected();

  EXPECT_EQ(CONNECT_TYPE, frame.header() & CONNECT_TYPE);
}

TEST_F(ClientTest, publishSendsPublishMessage_Test)
{
  QMQTT::Frame frame;
  EXPECT_CALL(*_networkMock, sendFrame(_)).WillOnce(SaveArg<0>(&frame));

  QMQTT::Message message(222, "topic", QByteArray("payload"));
  _client->publish(message);

  EXPECT_EQ(PUBLISH_TYPE, getHeaderType(frame.header()));
}

TEST_F(ClientTest, subscribeSendsSubscribeMessage_Test)
{
  QMQTT::Frame frame;
  EXPECT_CALL(*_networkMock, sendFrame(_)).WillOnce(SaveArg<0>(&frame));

  _client->subscribe("topic", QOS2);

  EXPECT_EQ(SUBSCRIBE_TYPE, getHeaderType(frame.header()));
}

// todo: these are internal, test them as we can (all are puback types?)
//    void puback(quint8 type, quint16 msgid);
//    void pubrec(int msgid);
//    void pubrel(int msgid);
//    void pubcomp(int msgid);

// todo: what happens if not already subscribed?
TEST_F(ClientTest, unsubscribeSendsUnsubscribeMessage_Test)
{
  QMQTT::Frame frame;
  EXPECT_CALL(*_networkMock, sendFrame(_)).WillOnce(SaveArg<0>(&frame));

  _client->unsubscribe("topic");

  EXPECT_EQ(UNSUBSCRIBE_TYPE, getHeaderType(frame.header()));
  // todo: test the topic
}

TEST_F(ClientTest, disconnectSendsDisconnectMessageAndNetworkDisconnect_Test)
{
  QMQTT::Frame frame;
  EXPECT_CALL(*_networkMock, sendFrame(_)).WillOnce(SaveArg<0>(&frame));
  EXPECT_CALL(*_networkMock, disconnectFromHost());

  _client->disconnectFromHost();

  EXPECT_EQ(DISCONNECT_TYPE, getHeaderType(frame.header()));
}

// todo: verify pingreq sent from client, will require timer interface and mock

TEST_F(ClientTest, networkConnectEmitsConnectedSignal_Test)
{
  EXPECT_CALL(*_networkMock, sendFrame(_));
  QSignalSpy spy(_client.data(), &QMQTT::Client::connected);

  emit _networkMock->connected();

  EXPECT_EQ(0, spy.count());
}

TEST_F(ClientTest, networkReceivedSendsConnackDoesNotEmitConnectedSignal_Test)
{
  QSignalSpy spy(_client.data(), &QMQTT::Client::connected);

  QMQTT::Frame frame(CONNACK_TYPE, QByteArray(2, 0x00));
  emit _networkMock->received(frame);

  EXPECT_EQ(1, spy.count());
}

// todo: receive connack_type should start keepalive
// todo: receive disconnect_type message should stop keepalive
//  we need a timer interface to test timer start and stop correctly

// todo: connack, connection accepted
// todo: connack, connection refused, unnacceptable protocol
// todo: connack, connection refused, identifier rejected
TEST_F(ClientTest, publishEmitsPublishedSignal_Test)
{
  EXPECT_CALL(*_networkMock, sendFrame(_));
  qRegisterMetaType<QMQTT::Message>("QMQTT::Message&");
  QSignalSpy spy(_client.data(), &QMQTT::Client::published);
  QMQTT::Message message(222, "topic", QByteArray("payload"));

  quint16 msgid = _client->publish(message);

  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(message, spy.at(0).at(0).value<QMQTT::Message>());
  EXPECT_EQ(msgid, spy.at(0).at(1).value<quint16>());
}

// todo: network received sends a puback, test what happens
// todo: client sends puback, test what happens

// todo: two different response types for different QoS levels
TEST_F(ClientTest, networkReceivedSendsPublishEmitsReceivedSignal_Test)
{
  QSignalSpy spy(_client.data(), &QMQTT::Client::received);

  QMQTT::Frame frame(PUBLISH_TYPE, QByteArray(2, 0x00));
  emit _networkMock->received(frame);

  EXPECT_EQ(1, spy.count());
}

TEST_F(ClientTest, subscribeEmitsSubscribedSignal_Test)
{
  EXPECT_CALL(*_networkMock, sendFrame(_));
  QSignalSpy spy(_client.data(), &QMQTT::Client::subscribed);

  _client->subscribe("topic", QOS2);

  QByteArray payLoad;
  payLoad.append((char)0x00); // message ID MSB
  payLoad.append((char)0x01); // message ID LSB
  payLoad.append((char)QOS2); // QOS
  QMQTT::Frame frame(SUBACK_TYPE, payLoad);
  emit _networkMock->received(frame);

  ASSERT_EQ(1, spy.count());
  EXPECT_EQ("topic", spy.at(0).at(0).toString());
  EXPECT_EQ(QOS2, spy.at(0).at(1).toInt());
}

// todo: network received sends suback triggers a subscribed signal (other
// things?)

TEST_F(ClientTest, unsubscribeEmitsUnsubscribedSignal_Test)
{
  EXPECT_CALL(*_networkMock, sendFrame(_));
  QSignalSpy spy(_client.data(), &QMQTT::Client::unsubscribed);

  _client->unsubscribe("topic");

  QByteArray payLoad;
  payLoad.append((char)0x00); // message ID MSB
  payLoad.append((char)0x01); // message ID LSB
  QMQTT::Frame frame(UNSUBACK_TYPE, payLoad);
  emit _networkMock->received(frame);

  ASSERT_EQ(1, spy.count());
  EXPECT_EQ("topic", spy.at(0).at(0).toString());
}

// todo: network received sends unsuback then emit unsubscribed signal (only
// then?)

TEST_F(ClientTest, networkDisconnectedEmitsDisconnectedSignal_Test)
{
  QSignalSpy spy(_client.data(), &QMQTT::Client::disconnected);

  emit _networkMock->disconnected();

  EXPECT_EQ(1, spy.count());
}

TEST_F(ClientTest, clientEmitsErrorWhenNetworkEmitsError_Test)
{
  QSignalSpy spy(_client.data(), &QMQTT::Client::error);
  emit _networkMock->error(QAbstractSocket::ConnectionRefusedError);
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(QMQTT::SocketConnectionRefusedError,
            spy.at(0).at(0).value<QMQTT::ClientError>());
}
