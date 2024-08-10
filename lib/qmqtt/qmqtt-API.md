# qmqtt API 

```C++
enum ClientError
{
    UnknownError = 0,
    SocketConnectionRefusedError,
    SocketRemoteHostClosedError,
    SocketHostNotFoundError,
    SocketAccessError, 
    SocketResourceError, 
    SocketTimeoutError, 
    SocketDatagramTooLargeError,
    SocketNetworkError, 
    SocketAddressInUseError, 
    SocketAddressNotAvailableError,
    SocketUnsupportedSocketOperationError,
    SocketUnfinishedSocketOperationError,
    SocketProxyAuthenticationRequiredError,
    SocketSslHandshakeFailedError,
    SocketProxyConnectionRefusedError,
    SocketProxyConnectionClosedError,
    SocketProxyConnectionTimeoutError,
    SocketProxyNotFoundError,
    SocketProxyProtocolError,
    SocketOperationError,
    SocketSslInternalError,
    SocketSslInvalidUserDataError,
    SocketTemporaryError,
    MqttUnacceptableProtocolVersionError=1<<16,
    MqttIdentifierRejectedError,
    MqttServerUnavailableError,
    MqttBadUserNameOrPasswordError,
    MqttNotAuthorizedError,
    MqttNoPingResponse 
}
```

This enum describes the client errors that can occur.

| Constant                                 | Value   | Description                                                  |
| :--------------------------------------- | ------- | ------------------------------------------------------------ |
| `UnknownError `                          | `0`     | An unidentified error occurred.                              |
| `SocketConnectionRefusedError`           | `1`     | The connection was refused by the peer (or timed out).       |
| `SocketRemoteHostClosedError`            | `2`     | The remote host closed the connection. Note that the client socket (i.e., this socket) will be closed after the remote close notification has been sent. |
| `SocketHostNotFoundError`                | `3`     | The host address was not found.                              |
| `SocketAccessError`                      | `4`     | The socket operation failed because the application lacked the required privileges. |
| `SocketResourceError`                    | `5`     | The local system ran out of resources (e.g., too many sockets). |
| `SocketTimeoutError`                     | `6`     | The socket operation timed out.                              |
| `SocketDatagramTooLargeError`            | `7`     | The datagram was larger than the operating system's limit (which can be as low as 8192 bytes). |
| `SocketNetworkError`                     | `8`     | An error occurred with the network (e.g., the network cable was accidentally plugged out). |
| `SocketAddressInUseError`                | `9`     | The address is already in use and was set to be exclusive.   |
| `SocketAddressNotAvailableError`         | `10`    | The address does not belong to the host.                     |
| `SocketUnsupportedSocketOperationError`  | `11`    | The requested socket operation is not supported by the local operating system (e.g., lack of IPv6 support). |
| `SocketUnfinishedSocketOperationError`   | `12`    | Used by QAbstractSocketEngine only, The last operation attempted has not finished yet (still in progress in the background). |
| `SocketProxyAuthenticationRequiredError` | `13`    | The socket is using a proxy, and the proxy requires authentication. |
| `SocketSslHandshakeFailedError`          | `14`    | The SSL/TLS handshake failed, so the connection was closed.  |
| `SocketProxyConnectionRefusedError`      | `15`    | Could not contact the proxy server because the connection to that server was denied |
| `SocketProxyConnectionClosedError`       | `16`    | The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established) |
| `SocketProxyConnectionTimeoutError`      | `17`    | The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase. |
| `SocketProxyNotFoundError`               | `18`    | The proxy address set with (or the application proxy) was not found. |
| `SocketProxyProtocolError`               | `19`    | The connection negotiation with the proxy server failed, because the response from the proxy server could not be understood. |
| `SocketOperationError`                   | `20`    | An operation was attempted while the socket was in a state that did not permit it. |
| `SocketSslInternalError`                 | `21`    | The SSL library being used reported an internal error. This is probably the result of a bad installation or misconfiguration of the library. |
| `SocketSslInvalidUserDataError`          | `22`    | Invalid data (certificate, key, cypher, etc.) was provided and its use resulted in an error in the SSL library. |
| `SocketTemporaryError`                   | `23`    | A temporary error occurred (e.g., operation would block and socket is non-blocking). |
| `MqttUnacceptableProtocolVersionError`   | `65536` | The **Server** does **not** support the **level** **of** the MQTT protocol requested **by** the **Client**. |
| `MqttIdentifierRejectedError`            | `65537` | The **Client** identifier **is** correct UTF-8 but **not** allowed **by** the **Server**. |
| `MqttServerUnavailableError`             | `65538` | The Network **Connection** has been made but the MQTT service **is** unavailable. |
| `MqttBadUserNameOrPasswordError`         | `65539` | The **data** **in** the **user** **name** **or** **password** **is** malformed. |
| `MqttNotAuthorizedError`                 | `65540` | The **Client** **is** **not** authorized **to** **connect**. |
| `MqttNoPingResponse`                     | `65541` | Ping response timeout.                                       |

#### Client Class

---

##### Public signals

|      | Signals       | Brief                               |
| ---- | :------------ | ----------------------------------- |
| void | `connected()` | Receive when connected. |
| void | `disconnected()` | Receive when disconnected. |
| void | `error(const QMQTT::ClientError error)` | Receive when client error occured. |
| void | `subscribed(const QString& topic, const quint8 qos)` | Receive when subscribe is succeeded. |
| void | `unsubscribed(const QString& topic)` | Receive when unsubscribe is succeeded. |
| void | `published(const quint16 msgid, const quint8 qos)` | Receive when publish is succeeded. |
| void | `pingresp()` | Receive when ping response is received. |
| void | `received(const QMQTT::Message& message)` | Receive when message is received. |

##### Public Functions

| Return    | Function members                                             | Brief                             |
| :-------- | :----------------------------------------------------------- | --------------------------------- |
| `Client`  | `Client(const QHostAddress& host = QHostAddress::LocalHost, const quint16 port = 1883, QObject* parent = NULL);` | Constructor client object.        |
| `void`    | `setHost(const QHostAddress& host)`                          | Set host address.                 |
| `void`    | `setHostName(const QString& hostName)`                       | Set host name.                    |
| `void`    | `setPort(const quint16 port)`                                | Set port.                         |
| `void`    | `setClientId(const QString& clientId)`                       | Set client identify.              |
| `void`    | `setUsername(const QString& username)`                       | Set user name.                    |
| `void`    | `setPassword(const QByteArray& password)`                    | Set password.                     |
| `void`    | `setVersion(const MQTTVersion version)`                      | Set mqtt version.                 |
| `void`    | `setKeepAlive(const quint16 keepAlive)`                      | Set keep alive interval.          |
| `void`    | `setCleanSession(const bool cleanSession)`                   | Set clean session option.         |
| `void`    | `setAutoReconnect(const bool value)`                         | Set auto reconnect.               |
| `void`    | `setAutoReconnectInterval(const int autoReconnectInterval)`  | Set auto reconnect interval.      |
| `void`    | `setWillTopic(const QString& willTopic)`                     | Set will topic.                   |
| `void`    | `setWillQos(const quint8 willQos)`                           | Set will qos level.               |
| `void`    | `setWillRetain(const bool willRetain)`                       | Set will retain.                  |
| `void`    | `setWillMessage(const QByteArray& willMessage)`              | Set will message.                 |
| `void`    | `connectToHost()`                                            | Connect to host.                  |
| `void`    | `disconnectFromHost()`                                       | Disconnect from host.             |
| `void`    | `subscribe(const QString& topic, const quint8 qos = 0)`      | Subscribe topic with certain qos. |
| `void`    | `unsubscribe(const QString& topic)`                          | Unsubscribe a topic.              |
| `quint16` | `publish(const QMQTT::Message& message)`                     | Publish a Message.                |

Client
---
```c++
Client(const QHostAddress& host = QHostAddress::LocalHost,
       const quint16 port = 1883,
       QObject* parent = NULL);
```
Client constructor.
**Parameters**

- host: Host ip.
- port: Host port.
- parent: QObject.

**Returns**
- Client object.

setHost
---
`void setHost(const QHostAddress& host)`
Set host ip address. For example:

```C++
QMQTT::Client client;
const QHostAddress EXAMPLE_HOST = QHostAddress("34.211.84.46");
client.setHost(HOST);
```

**Parameters**

- host: Host ip.

**Returns**

setHostName
---
`setHostName(const QString& hostName)`
Set host name. For example:

```C++
QMQTT::Client client;
client.setHostName("broker.emqx.io");
```

**Parameters**

- hostName: Host name.

**Returns**

setPort
---
`setPort(const quint16 port)`
Set port num. For example:

```C++
QMQTT::Client client;
client.setPort(1883);
```

**Parameters**

- port: Port num.

**Returns**

setClientId
---
`setClientId(const QString& clientId)`
Set client identify. For example:

```C++
QMQTT::Client client;
client.setClientId("clientid");
```

**Parameters**

- clientId: Client identify.

**Returns**

setUsername
---
`void setUsername(const QString& username)`
Set username. For example:

```C++
QMQTT::Client client;
client.setUsername("username");
```

**Parameters**

- username: Username.

**Returns**

setVersion
---
`void setVersion(const MQTTVersion version)`
Set mqtt version. For example:

```C++
QMQTT::Client client;
client.setVersion(QMQTT::V3_1_1);
```

**Parameters**

- version: Mqtt version. For now only support 3.1.0 and 3.1.1.

**Returns**

setKeepAlive
---
`void setKeepAlive(const quint16 keepAlive);`
Set Keep alive interval. For example:

```C++
QMQTT::Client client;
client.setKeepAlive(100);
```

**Parameters**

- keepAlive: Keep alive interval, default is 300.

**Returns**

setCleanSession
---
`void setCleanSession(const bool cleanSession)`
Set clean session flag. For example:

```C++
QMQTT::Client client;
client.setCleanSession(true);
```

**Parameters**

- cleanSession: Set clean session, if true clean session, false keep session, default is false.

**Returns**

setAutoReconnect
---
`void setAutoReconnect(const bool value)`
Set auto reconnect. For example:

```C++
QMQTT::Client client;
client.setAutoReconnect(true);
```

**Parameters**

- value: Auto reconnect or not, true auto reconnect, false do not reconnect.

**Returns**

setAutoReconnectInterval
---
`void setAutoReconnectInterval(const int autoReconnectInterval)`
Set auto reconnect interval. For example:

```C++
QMQTT::Client client;
client.setAutoReconnectInterval(10);
```

**Parameters**

- autoReconnectInterval: Auto reconnect interval (second).

**Returns**

setWillTopic
---
`void setWillTopic(const QString& willTopic)`
Set will topic info. For example:

```C++
QMQTT::Client client;
client.setWillTopic("will topic");
```

**Parameters**

- willTopic: Will topic.

**Returns**

setWillQos
---
`void setWillQos(const quint8 willQos)`
Set will qos level. For example:

```C++
QMQTT::Client client;
client.setWillQos(1);
```

**Parameters**

- willQos: Will qos level.

**Returns**

setWillRetain
---
`void setWillRetain(const bool willRetain)`
Set will retain flag. For example:

```C++
QMQTT::Client client;
client.setWillRetain(true);
```

**Parameters**

- willRetain: Will retain flag.

**Returns**

setWillMessage
---
`void setWillMessage(const QByteArray& willMessage)`
Set will message. For example:

```C++
QMQTT::Client client;
QByteArray ba("Hello");
client.setWillMessage(ba);
```

**Parameters**

- willMessage: Will message. 

**Returns**

connectToHost
---
`void connectToHost()`
Connect to host. For example:

```C++
QMQTT::Client client;
client.connectToHost();
```

**Parameters**

**Returns**

disconnectFromHost
---
`void disconnectFromHost()`
Disconnect from host. For example:

```C++
QMQTT::Client client;
client.disconnectFromHost();
```

**Parameters**

**Returns**

subscribe
---
`void subscribe(const QString& topic, const quint8 qos = 0)`
Subscribe a topic with a certain qos level.

```C++
QMQTT::Client client;
client.subscribe("topic", 1);
```

**Parameters**

- topic: subscribe topic.
- qos: subscribe topic qos level, default is 0.

**Returns**

unsubscribe
---
`void unsubscribe(const QString& topic)`
Unsubscribe a topic.

```C++
QMQTT::Client client;
client.unsubscribe("topic");
```



**Parameters**

- topic: unsubscribe topic.

**Returns**

publish
---
`quint16 publish(const QMQTT::Message& message)`
Publish a Message.

```C++
QMQTT::Client client;
QMQTT::Message message(1, "topic", QString("Hello").toUtf8());
message.setQos(2);
client.publish(message);
```

**Parameters**

- message: Message info see below.

**Returns**


#### Message Class

---

##### Public Functions

| Return       | Function members                                             | Brief                     |
| :----------- | :----------------------------------------------------------- | ------------------------- |
| `Message`    | `Message()`                                                  | Construct Message object  |
| `Message`    | `Message(const quint16 id, const QString &topic, const QByteArray &payload, const quint8 qos = 0, const bool retain = false, const bool dup = false)` | Construct Message object  |
| `Message`    | `Message(const Message &other)`                              | Construct Message object  |
| `Message`    | `~Message()`                                                 | Destructor Message object |
| `quint16`    | `id() const`                                                 | Get id                    |
| `void`       | `setId(const quint16 id)`                                    | Set id                    |
| `quint8`     | `qos() const`                                                | Get qos                   |
| `void`       | `setQos(const quint8 qos)`                                   | Set Qos                   |
| `bool`       | `retain() const`                                             | Get retain                |
| `void`       | `setRetain(const bool retain)`                               | Set retain                |
| `bool`       | `dup() const`                                                | Get dup                   |
| `void`       | `setDup(const bool dup)`                                     | Set Dup                   |
| `QString`    | `topic() const`                                              | Get topic                 |
| `void`       | `setTopic(const QString &topic)`                             | Set topic                 |
| `QByteArray` | `payload() const`                                            | Get payload               |
| `void`       | `setPayload(const QByteArray &payload)`                      | Set payload               |

Message
---

```C++
Message()
Message(const quint16 id, const QString &topic, const QByteArray &payload,const quint8 qos = 0, const bool retain = false, const bool dup = false)
Message(const Message &other)
```

id
---
`quint16 id() const`
Get id from Message.
**Parameters**

**Returns**

- Message identify.

setId
---
`void setId(const quint16 id)`
Set id to Message.
**Parameters**

- id: Message identify

**Returns**

qos
---
`quint8 qos() const`
Get qos level from Message.
**Parameters**

**Returns**

- Message qos level.

setQos
---
`void setQos(const quint8 qos)`
Set Message Qos level.
**Parameters**

- qos: Message qos level.

**Returns**

retain
---
`bool retain() const`
Get retain flag from Message.
**Parameters**

**Returns**

- Had retain flag been set.

setRetain
---
`void setRetain(const bool retain)`
Set retain flag to Message.
**Parameters**

- retain: Set retain flag.

**Returns**

dup
--- 
`bool dup() const` 
Get dup flag from Message.
**Parameters**

**Returns**

- Dup flag been se.

setDup
---
`void setDup(const bool dup)`
Set dup flag for Message.
**Parameters**

- dup: Dup flag.

**Returns**

topic
---
`QString topic() const`
Get topic from Message.
**Parameters**

**Returns**

- Get Message topic.

setTopic
---
`void setTopic(const QString &topic)`
Set topic to Message.
**Parameters**

- topic: Set Message topic.

**Returns**

payload
---
`QByteArray payload() const`
Get payload form Message.
**Parameters**

**Returns**

- Message payload.

setPayload
---
`void setPayload(const QByteArray &payload)`
Set payload for Message.
**Parameters**

- payload: SetMessage payload.

**Returns**

