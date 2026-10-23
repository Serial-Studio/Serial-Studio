/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QTest>

#include "API/CommandCapabilities.h"
#include "API/Server/AuthPrimitives.h"
#include "API/Server/ClientReception.h"
#include "API/Server/ServerAuth.h"
#include "Core/Prompt/IUserPrompter.h"
#include "Core/Prompt/UserPrompt.h"

// Mirrors the caps ClientReception.cpp keeps file-private; restating them here is the point of a
// KAT: a silent widening of a limit has to break this suite.
constexpr int kMaxApiJsonDepth         = 64;
constexpr int kMaxApiRawBytes          = 1024 * 1024;
constexpr int kMaxApiMessagesPerWindow = 200;
constexpr int kMaxApiMessageBytes      = 1024 * 1024;
constexpr int kMaxApiBufferBytes       = 4 * 1024 * 1024;
constexpr int kMaxApiBytesPerWindow    = 128 * 1024 * 1024;
constexpr int kMaxAuthAttempts         = 3;

/**
 * @brief Records every effect the reception machine asks its host for, so a test can assert on
 *        what the machine decided without a server, a worker thread, or a live socket.
 */
class StubHost : public API::ReceptionHost {
public:
  bool deviceConnected() const override { return connected; }

  API::DeviceWriteVerdict authorizeDeviceWrite() override
  {
    return consent ? API::DeviceWriteVerdict::Allowed : API::DeviceWriteVerdict::Denied;
  }

  bool verifyToken(const QByteArray& provided) const override { return provided == expectedToken; }

  API::ConnectionState* stateFor(QTcpSocket* socket, const QString& sessionId) override
  {
    Q_UNUSED(socket);
    if (!lastState || lastState->sessionId != sessionId)
      return nullptr;

    return lastState;
  }

  qint64 writeToDevice(const QByteArray& data) override
  {
    deviceWrites.append(data);
    return data.size();
  }

  QByteArray dispatchCommand(const QByteArray& jsonBytes) override
  {
    dispatchedCommands.append(jsonBytes);
    return QByteArrayLiteral("{\"type\":\"response\",\"success\":true}\n");
  }

  QByteArray dispatchMcp(const QByteArray& jsonBytes, const QString& sessionId) override
  {
    Q_UNUSED(sessionId);
    dispatchedMcp.append(jsonBytes);
    return QByteArrayLiteral("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{}}\n");
  }

  bool routeConnectionCommand(QTcpSocket* socket,
                              API::ConnectionState& state,
                              const QJsonObject& json) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    routedCommands.append(json.value(QStringLiteral("command")).toString());
    return false;
  }

  void sendResponse(QTcpSocket* socket, const QByteArray& response) override
  {
    Q_UNUSED(socket);
    responses.append(response);
  }

  void closeSocket(QTcpSocket* socket, API::ConnectionState& state) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(state);
    ++closes;
  }

  void disconnectClient(QTcpSocket* socket,
                        API::ConnectionState& state,
                        const QString& errorCode,
                        const QString& errorMessage) override
  {
    Q_UNUSED(socket);
    Q_UNUSED(errorMessage);
    state.buffer.clear();
    disconnectCodes.append(errorCode);
  }

  bool consent                    = true;
  bool connected                  = true;
  int closes                      = 0;
  API::ConnectionState* lastState = nullptr;
  QByteArray expectedToken;
  QList<QByteArray> responses;
  QList<QByteArray> deviceWrites;
  QList<QByteArray> dispatchedMcp;
  QList<QByteArray> dispatchedCommands;
  QStringList routedCommands;
  QStringList disconnectCodes;
};

/**
 * @brief A prompter that says yes to one consent title and no to every other, so a case can grant
 *        a single gate the way a user would and watch whether another gate rides on it.
 */
class OneConsentPrompter : public Core::Prompt::IUserPrompter {
public:
  explicit OneConsentPrompter(const QString& yesTitle) : m_yesTitle(yesTitle) {}

  int promptMessage(const QString& text,
                    const QString& informativeText,
                    Core::Prompt::Icon icon,
                    const QString& windowTitle,
                    Core::Prompt::Buttons buttons,
                    Core::Prompt::Button defaultButton,
                    const Core::Prompt::ButtonLabels& buttonLabels) override
  {
    Q_UNUSED(informativeText);
    Q_UNUSED(icon);
    Q_UNUSED(windowTitle);
    Q_UNUSED(buttons);
    Q_UNUSED(defaultButton);
    Q_UNUSED(buttonLabels);
    titles.append(text);
    return text == m_yesTitle ? Core::Prompt::Yes : Core::Prompt::No;
  }

  void promptDirectory(const QString& title,
                       const QString& initialPath,
                       DirectoryHandler onSelected) override
  {
    Q_UNUSED(title);
    Q_UNUSED(initialPath);
    Q_UNUSED(onSelected);
  }

  void revealInFileManager(const QString& path) override { Q_UNUSED(path); }

  QStringList titles;

private:
  QString m_yesTitle;
};

/**
 * @brief Binds a prompter for the scope of one case and unbinds it on every exit path, so an
 *        early QVERIFY return never leaves a dangling stack prompter behind.
 */
class PrompterScope {
public:
  explicit PrompterScope(Core::Prompt::IUserPrompter* prompter)
  {
    Core::Prompt::setPrompter(prompter);
  }

  ~PrompterScope() { Core::Prompt::setPrompter(nullptr); }

  PrompterScope(PrompterScope&&)                 = delete;
  PrompterScope(const PrompterScope&)            = delete;
  PrompterScope& operator=(PrompterScope&&)      = delete;
  PrompterScope& operator=(const PrompterScope&) = delete;
};

/**
 * @brief Auth primitives and the API socket's reception guards, both driven without a server.
 */
class TstServerAuth : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void constantTimeEquals_data();
  void constantTimeEquals();

  void normalizeToken_data();
  void normalizeToken();

  void generatedTokenIsUsable();

  void commandClassification_data();
  void commandClassification();

  void scriptInstallClassification_data();
  void scriptInstallClassification();

  void remoteCommandPolicy();
  void remoteCommandPolicyHeadlessGrant();

  void uiDriveConsentDoesNotCompose();
  void uiDriveHeadlessGrant();

  void authHandshakeRejectsBadToken();
  void authHandshakeAcceptsAndReplaysPipeline();

  void jsonMessageSizeLimit();
  void jsonMessageDepthLimit();
  void jsonMessageRateLimit();

  void rateLimitsByteWindow();
  void rateLimitsBufferCap();

  void noNewlineRawBufferReachesDevice();
  void rawLineOversizeClosesSocket();

private:
  [[nodiscard]] static API::ConnectionState authenticatedState();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Test-mode settings so ServerAuth's token lives in a throwaway store (consent is
 *        session-scoped since 2026-10-04 and never persists).
 */
void TstServerAuth::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_server_auth"));
  QSettings settings;
  settings.clear();
}

/**
 * @brief A connection that already cleared the handshake, as every post-auth path expects.
 */
API::ConnectionState TstServerAuth::authenticatedState()
{
  API::ConnectionState state;
  state.sessionId      = QStringLiteral("7");
  state.peerAddress    = QStringLiteral("127.0.0.1");
  state.peerPort       = 55000;
  state.authenticated  = true;
  state.handshakeSeen  = true;
  state.firstBytesSeen = true;
  return state;
}

//--------------------------------------------------------------------------------------------------
// constantTimeEquals
//--------------------------------------------------------------------------------------------------

void TstServerAuth::constantTimeEquals_data()
{
  QTest::addColumn<QByteArray>("lhs");
  QTest::addColumn<QByteArray>("rhs");
  QTest::addColumn<bool>("expected");

  const QByteArray token(64, 'a');

  QTest::newRow("identical") << token << token << true;
  QTest::newRow("empty pair") << QByteArray() << QByteArray() << true;
  QTest::newRow("first byte differs")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("zbcd") << false;
  QTest::newRow("last byte differs")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("abcz") << false;
  QTest::newRow("one bit differs")
    << QByteArrayLiteral("\x01") << QByteArrayLiteral("\x03") << false;
  QTest::newRow("length mismatch prefix")
    << QByteArrayLiteral("abcd") << QByteArrayLiteral("abcde") << false;
  QTest::newRow("empty against value") << QByteArray() << QByteArrayLiteral("a") << false;
  QTest::newRow("high bytes equal")
    << QByteArrayLiteral("\xff\x80") << QByteArrayLiteral("\xff\x80") << true;
  QTest::newRow("high bytes differ")
    << QByteArrayLiteral("\xff\x80") << QByteArrayLiteral("\xff\x81") << false;
}

/**
 * @brief The comparison must agree with byte equality on every row; the constant-time property
 *        itself is structural (no early exit inside the loop) and is guarded by review.
 */
void TstServerAuth::constantTimeEquals()
{
  QFETCH(QByteArray, lhs);
  QFETCH(QByteArray, rhs);
  QFETCH(bool, expected);

  QCOMPARE(API::Auth::constantTimeEquals(lhs, rhs), expected);
  QCOMPARE(API::Auth::constantTimeEquals(rhs, lhs), expected);
}

//--------------------------------------------------------------------------------------------------
// normalizeToken
//--------------------------------------------------------------------------------------------------

void TstServerAuth::normalizeToken_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  const QString hex32(32, QLatin1Char('a'));
  const QString hex64(64, QLatin1Char('f'));

  QTest::newRow("exactly 32 hex") << hex32 << hex32;
  QTest::newRow("64 hex") << hex64 << hex64;
  QTest::newRow("uppercase folds") << QString(32, QLatin1Char('A')) << hex32;
  QTest::newRow("surrounding whitespace")
    << (QStringLiteral("  ") + hex32 + QStringLiteral("\n")) << hex32;
  QTest::newRow("digits are hex") << QString(32, QLatin1Char('9')) << QString(32, QLatin1Char('9'));
  QTest::newRow("31 chars refused") << QString(31, QLatin1Char('a')) << QString();
  QTest::newRow("non-hex letter refused") << QString(32, QLatin1Char('g')) << QString();
  QTest::newRow("punctuation refused")
    << (QString(31, QLatin1Char('a')) + QStringLiteral("-")) << QString();
  QTest::newRow("empty refused") << QString() << QString();
  QTest::newRow("whitespace only refused") << QString(40, QLatin1Char(' ')) << QString();
}

/**
 * @brief Anything shorter than 32 characters or outside [0-9a-f] is refused rather than accepted
 *        as a weaker credential; a valid token folds to lower case and loses its whitespace.
 */
void TstServerAuth::normalizeToken()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);

  QCOMPARE(API::Auth::normalizeToken(input), expected);
}

/**
 * @brief A generated token is 32 random bytes in hex, and survives its own validator unchanged.
 */
void TstServerAuth::generatedTokenIsUsable()
{
  const auto token = API::Auth::generateToken();
  QCOMPARE(token.size(), qsizetype(64));
  QCOMPARE(API::Auth::normalizeToken(token), token);
  QVERIFY(token != API::Auth::generateToken());
  QVERIFY(API::Auth::constantTimeEquals(token.toUtf8(), token.toUtf8()));
}

//--------------------------------------------------------------------------------------------------
// Command classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p command, with @p params, has @p capability; an unclassified one has none.
 */
static bool hasCapability(const QString& command,
                          const QJsonObject& params,
                          const API::CommandCapabilities::Capability capability)
{
  const auto capabilities = API::CommandCapabilities::of(command, params);
  return capabilities.has_value() && capabilities->testFlag(capability);
}

void TstServerAuth::commandClassification_data()
{
  QTest::addColumn<QString>("command");
  QTest::addColumn<bool>("scriptOnly");
  QTest::addColumn<bool>("writesToDevice");

  QTest::newRow("system.exec") << QStringLiteral("system.exec") << true << false;
  QTest::newRow("system.kill") << QStringLiteral("system.kill") << true << false;
  QTest::newRow("system.runningProcesses")
    << QStringLiteral("system.runningProcesses") << true << false;
  QTest::newRow("io.writeData") << QStringLiteral("io.writeData") << false << true;
  QTest::newRow("io.ble.writeCharacteristic")
    << QStringLiteral("io.ble.writeCharacteristic") << false << true;
  QTest::newRow("console.send") << QStringLiteral("console.send") << false << true;
  QTest::newRow("project.new") << QStringLiteral("project.new") << false << false;
  QTest::newRow("io.connect") << QStringLiteral("io.connect") << false << false;
  QTest::newRow("ui.automation.click") << QStringLiteral("ui.automation.click") << false << false;
  QTest::newRow("ui.automation.listWindows")
    << QStringLiteral("ui.automation.listWindows") << false << false;
  QTest::newRow("unknown command") << QStringLiteral("nope.nope") << false << false;
  QTest::newRow("case sensitive") << QStringLiteral("IO.writeData") << false << false;
}

/**
 * @brief The capabilities behind authorizeRemoteCommand(): a control-script-only command is
 *        refused outright, a device-write command falls through to the consent gate, and an
 *        unknown or miscased name has neither (the gate refuses it as unclassified instead).
 */
void TstServerAuth::commandClassification()
{
  QFETCH(QString, command);
  QFETCH(bool, scriptOnly);
  QFETCH(bool, writesToDevice);

  namespace Caps = API::CommandCapabilities;
  QCOMPARE(hasCapability(command, QJsonObject(), Caps::ControlScriptOnly), scriptOnly);
  QCOMPARE(hasCapability(command, QJsonObject(), Caps::WritesDevice), writesToDevice);
}

//--------------------------------------------------------------------------------------------------
// Script-change classification
//--------------------------------------------------------------------------------------------------

void TstServerAuth::scriptInstallClassification_data()
{
  QTest::addColumn<QString>("command");
  QTest::addColumn<QJsonObject>("params");
  QTest::addColumn<bool>("installs");

  const QJsonObject none;
  const QJsonObject transform{
    {QStringLiteral("transformCode"), QStringLiteral("return v")}
  };
  const QJsonObject painter{
    {QStringLiteral("painterCode"), QStringLiteral("function paint(){}")}
  };
  const QJsonObject transmit{
    {QStringLiteral("transmitFunction"), QStringLiteral("x")}
  };
  const QJsonObject title{
    {QStringLiteral("title"), QStringLiteral("Voltage")}
  };

  QTest::newRow("controlScript.set") << QStringLiteral("controlScript.set") << none << true;
  QTest::newRow("controlScript.setCode") << QStringLiteral("controlScript.setCode") << none << true;
  QTest::newRow("frameParser.setCode")
    << QStringLiteral("project.frameParser.setCode") << none << true;
  QTest::newRow("source.setFrameParserCode")
    << QStringLiteral("project.source.setFrameParserCode") << none << true;
  QTest::newRow("dataset.setTransformCode")
    << QStringLiteral("project.dataset.setTransformCode") << none << true;
  QTest::newRow("transformLibrary.set")
    << QStringLiteral("project.transformLibrary.set") << none << true;
  QTest::newRow("painter.setCode") << QStringLiteral("project.painter.setCode") << none << true;
  QTest::newRow("extensions.install") << QStringLiteral("extensions.install") << none << true;
  QTest::newRow("extensions.addRepository")
    << QStringLiteral("extensions.addRepository") << none << true;
  QTest::newRow("project.loadJson") << QStringLiteral("project.loadJson") << none << true;
  QTest::newRow("project.open") << QStringLiteral("project.open") << none << true;
  QTest::newRow("template.apply") << QStringLiteral("project.template.apply") << none << true;
  QTest::newRow("assistant.restore") << QStringLiteral("assistant.restore") << none << true;
  QTest::newRow("sessions.regress") << QStringLiteral("sessions.regress") << none << true;
  QTest::newRow("sessions.verify") << QStringLiteral("sessions.verify") << none << true;
  QTest::newRow("sessions.replay") << QStringLiteral("sessions.replay") << none << true;
  QTest::newRow("dataset.update with code")
    << QStringLiteral("project.dataset.update") << transform << true;
  QTest::newRow("dataset.update without code")
    << QStringLiteral("project.dataset.update") << title << false;
  QTest::newRow("group.update with painter")
    << QStringLiteral("project.group.update") << painter << true;
  QTest::newRow("group.update without painter")
    << QStringLiteral("project.group.update") << title << false;
  QTest::newRow("outputWidget.update with transmit")
    << QStringLiteral("project.outputWidget.update") << transmit << true;
  QTest::newRow("outputWidget.update without transmit")
    << QStringLiteral("project.outputWidget.update") << title << false;
  QTest::newRow("controlScript.dryRun") << QStringLiteral("controlScript.dryRun") << none << false;
  QTest::newRow("controlScript.get") << QStringLiteral("controlScript.get") << none << false;
  QTest::newRow("project.new") << QStringLiteral("project.new") << none << false;
  QTest::newRow("io.connect") << QStringLiteral("io.connect") << none << false;
  QTest::newRow("case sensitive") << QStringLiteral("ControlScript.set") << none << false;
}

/**
 * @brief The script-change classification the script consent guards: every setter and project
 *        loader unconditionally, the generic updaters only when the call carries their code
 *        field, and no dry run (dry runs execute on an inert surface instead).
 */
void TstServerAuth::scriptInstallClassification()
{
  QFETCH(QString, command);
  QFETCH(QJsonObject, params);
  QFETCH(bool, installs);

  namespace Caps    = API::CommandCapabilities;
  const bool script = hasCapability(command, params, Caps::InstallsScript);
  const bool load   = hasCapability(command, params, Caps::LoadsProject);
  QCOMPARE(script || load, installs);
}

//--------------------------------------------------------------------------------------------------
// Remote command policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief The ONE remote policy: unclassified and control-script-only refused outright, a script
 *        change, a project load and a device write each refused with CONSENT_REQUIRED while their
 *        prompt is pending and denied once it resolved to "no" (no prompter bound answers the
 *        default), the rest allowed. Only a command that cleared every gate announces a change.
 */
void TstServerAuth::remoteCommandPolicy()
{
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");
  QSettings settings;
  settings.clear();
  API::ServerAuth auth(settings);
  QSignalSpy changes(&auth, &API::ServerAuth::remoteProgramChanged);
  const QString id = QStringLiteral("7");

  const auto unknown = auth.authorizeRemoteCommand(id, QStringLiteral("no.such.command"), {});
  QVERIFY(unknown.has_value());
  QCOMPARE(unknown->errorCode, QString::fromLatin1(API::ErrorCode::ExecutionError));
  QVERIFY(unknown->errorMessage.contains(QStringLiteral("no capability classification")));

  const auto scriptOnly = auth.authorizeRemoteCommand(id, QStringLiteral("system.exec"), {});
  QVERIFY(scriptOnly.has_value());
  QCOMPARE(scriptOnly->id, id);
  QCOMPARE(scriptOnly->errorCode, QString::fromLatin1(API::ErrorCode::ExecutionError));
  QVERIFY(scriptOnly->errorMessage.contains(QStringLiteral("control-script only")));

  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("project.new"), {}).has_value());
  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("controlScript.dryRun"), {}).has_value());
  QCOMPARE(changes.count(), 0);

  const auto install = auth.authorizeRemoteCommand(id, QStringLiteral("controlScript.set"), {});
  QVERIFY(install.has_value());
  QCOMPARE(install->errorCode, QString::fromLatin1(API::ErrorCode::ConsentRequired));

  const auto restore = auth.authorizeRemoteCommand(id, QStringLiteral("assistant.restore"), {});
  QVERIFY(restore.has_value());
  QCOMPARE(restore->errorCode, QString::fromLatin1(API::ErrorCode::ConsentRequired));

  const auto write = auth.authorizeRemoteCommand(id, QStringLiteral("io.writeData"), {});
  QVERIFY(write.has_value());
  QCOMPARE(write->errorCode, QString::fromLatin1(API::ErrorCode::ConsentRequired));
  QCOMPARE(auth.authorizeDeviceWrite(), API::DeviceWriteVerdict::ConsentRequired);
  QCOMPARE(changes.count(), 0);

  const QJsonObject program{
    {QStringLiteral("key"), QStringLiteral("executable")}
  };
  const auto choose =
    auth.authorizeRemoteCommand(id, QStringLiteral("project.source.setProperty"), program);
  QVERIFY(!choose.has_value());
  QCOMPARE(changes.count(), 1);

  QCoreApplication::processEvents();

  const auto denied = auth.authorizeRemoteCommand(id, QStringLiteral("controlScript.set"), {});
  QVERIFY(denied.has_value());
  QCOMPARE(denied->errorCode, QString::fromLatin1(API::ErrorCode::ExecutionError));
  QVERIFY(denied->errorMessage.contains(QStringLiteral("denied by the user")));
  QCOMPARE(auth.authorizeDeviceWrite(), API::DeviceWriteVerdict::Denied);
  QCOMPARE(changes.count(), 1);
  QVERIFY(!settings.contains(QStringLiteral("API/ScriptInstallConsent")));
  QVERIFY(!settings.contains(QStringLiteral("API/DeviceWriteConsent")));
}

/**
 * @brief The headless override grants both consents in memory: nothing persists, the
 *        control-script-only refusal still stands, and an allowed script change still announces
 *        itself.
 */
void TstServerAuth::remoteCommandPolicyHeadlessGrant()
{
  qputenv("SERIAL_STUDIO_API_AUTO_CONSENT", QByteArrayLiteral("1"));
  QSettings settings;
  settings.clear();
  API::ServerAuth auth(settings);
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");
  QSignalSpy changes(&auth, &API::ServerAuth::remoteProgramChanged);
  const QString id = QStringLiteral("8");

  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("controlScript.set"), {}).has_value());
  QCOMPARE(changes.count(), 1);
  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("io.writeData"), {}).has_value());
  QCOMPARE(auth.authorizeDeviceWrite(), API::DeviceWriteVerdict::Allowed);
  QVERIFY(auth.authorizeRemoteCommand(id, QStringLiteral("system.kill"), {}).has_value());
  QCOMPARE(changes.count(), 1);
  QVERIFY(!settings.contains(QStringLiteral("API/ScriptInstallConsent")));
  QVERIFY(!settings.contains(QStringLiteral("API/DeviceWriteConsent")));
}

//--------------------------------------------------------------------------------------------------
// Interface-driving consent
//--------------------------------------------------------------------------------------------------

/**
 * @brief Consents do not compose (spec 0101 R8): a user who granted device writes has not granted
 *        interface driving, so a click stays behind its own prompt and a "no" there holds for the
 *        session while device writes stay allowed. The read-only automation commands need neither.
 */
void TstServerAuth::uiDriveConsentDoesNotCompose()
{
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");
  QSettings settings;
  settings.clear();
  API::ServerAuth auth(settings);
  OneConsentPrompter prompter(QStringLiteral("Allow API device control?"));
  const PrompterScope bound(&prompter);
  const QString id    = QStringLiteral("9");
  const QString click = QStringLiteral("ui.automation.click");
  const QString key   = QStringLiteral("ui.automation.key");

  namespace Caps = API::CommandCapabilities;
  QVERIFY(hasCapability(click, QJsonObject(), Caps::DrivesUi));
  QVERIFY(hasCapability(key, QJsonObject(), Caps::DrivesUi));
  QVERIFY(!hasCapability(click, QJsonObject(), Caps::WritesDevice));

  const auto write = auth.authorizeRemoteCommand(id, QStringLiteral("io.writeData"), {});
  QVERIFY(write.has_value());
  QCOMPARE(write->errorCode, QString::fromLatin1(API::ErrorCode::ConsentRequired));
  QCoreApplication::processEvents();
  QCOMPARE(auth.authorizeDeviceWrite(), API::DeviceWriteVerdict::Allowed);
  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("io.writeData"), {}).has_value());
  QCOMPARE(prompter.titles.size(), qsizetype(1));

  const auto pending = auth.authorizeRemoteCommand(id, click, {});
  QVERIFY(pending.has_value());
  QCOMPARE(pending->errorCode, QString::fromLatin1(API::ErrorCode::ConsentRequired));
  QCoreApplication::processEvents();
  QCOMPARE(prompter.titles.size(), qsizetype(2));
  QCOMPARE(prompter.titles.last(),
           QStringLiteral("Allow API clients to drive the user interface?"));

  const auto denied = auth.authorizeRemoteCommand(id, key, {});
  QVERIFY(denied.has_value());
  QCOMPARE(denied->errorCode, QString::fromLatin1(API::ErrorCode::ExecutionError));
  QVERIFY(denied->errorMessage.contains(QStringLiteral("denied by the user")));
  QCOMPARE(auth.authorizeDeviceWrite(), API::DeviceWriteVerdict::Allowed);

  const QString list = QStringLiteral("ui.automation.listWindows");
  const QString shot = QStringLiteral("ui.automation.screenshot");
  QVERIFY(!auth.authorizeRemoteCommand(id, list, {}).has_value());
  QVERIFY(!auth.authorizeRemoteCommand(id, shot, {}).has_value());
  QCOMPARE(prompter.titles.size(), qsizetype(2));
  QVERIFY(settings.allKeys().filter(QStringLiteral("Consent")).isEmpty());
}

/**
 * @brief The headless override grants interface driving beside the other two consents, in memory
 *        only; a driving command announces no program change, because it chose none.
 */
void TstServerAuth::uiDriveHeadlessGrant()
{
  qputenv("SERIAL_STUDIO_API_AUTO_CONSENT", QByteArrayLiteral("1"));
  QSettings settings;
  settings.clear();
  API::ServerAuth auth(settings);
  qunsetenv("SERIAL_STUDIO_API_AUTO_CONSENT");
  QSignalSpy changes(&auth, &API::ServerAuth::remoteProgramChanged);
  const QString id = QStringLiteral("10");

  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("ui.automation.click"), {}).has_value());
  QVERIFY(!auth.authorizeRemoteCommand(id, QStringLiteral("ui.automation.key"), {}).has_value());
  QCOMPARE(changes.count(), 0);
  QVERIFY(settings.allKeys().filter(QStringLiteral("Consent")).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Authentication handshake
//--------------------------------------------------------------------------------------------------

/**
 * @brief A wrong token is answered with the challenge until the attempt budget runs out, and the
 *        connection is dropped on the third failure without ever reaching the command dispatcher.
 */
void TstServerAuth::authHandshakeRejectsBadToken()
{
  QTcpSocket socket;
  StubHost host;
  host.expectedToken = QByteArrayLiteral("goodtoken");

  API::ClientReception reception(host);
  auto state          = authenticatedState();
  state.authenticated = false;
  host.lastState      = &state;

  const QByteArray attempt = QByteArrayLiteral("{\"type\":\"auth\",\"token\":\"bad\"}\n");
  for (int i = 0; i < kMaxAuthAttempts; ++i)
    reception.consumeBytes(&socket, state.sessionId, attempt);

  QVERIFY(!state.authenticated);
  QCOMPARE(state.authAttempts, kMaxAuthAttempts);
  QCOMPARE(host.responses.size(), qsizetype(kMaxAuthAttempts - 1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
  QVERIFY(host.dispatchedCommands.isEmpty());
}

/**
 * @brief The correct token authenticates the connection and the bytes pipelined behind the
 *        handshake line are processed in the same pass, not stranded in the buffer.
 */
void TstServerAuth::authHandshakeAcceptsAndReplaysPipeline()
{
  QTcpSocket socket;
  StubHost host;
  host.expectedToken = QByteArrayLiteral("goodtoken");

  API::ClientReception reception(host);
  auto state          = authenticatedState();
  state.authenticated = false;
  host.lastState      = &state;

  reception.consumeBytes(
    &socket,
    state.sessionId,
    QByteArrayLiteral("{\"type\":\"auth\",\"token\":\"goodtoken\"}\n"
                      "{\"type\":\"command\",\"command\":\"api.getCommands\"}\n"));

  QVERIFY(state.authenticated);
  QVERIFY(state.buffer.isEmpty());
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
  QCOMPARE(host.dispatchedCommands.size(), qsizetype(1));
  QCOMPARE(host.routedCommands, QStringList{QStringLiteral("api.getCommands")});
  QCOMPARE(host.responses.size(), qsizetype(2));

  const auto handshake = QJsonDocument::fromJson(host.responses.first()).object();
  QCOMPARE(handshake.value(QStringLiteral("success")).toBool(), true);
  QCOMPARE(handshake.value(QStringLiteral("result"))
             .toObject()
             .value(QStringLiteral("authenticated"))
             .toBool(),
           true);
}

//--------------------------------------------------------------------------------------------------
// JSON message validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A message past the size cap is refused with an error response, not a disconnect: an
 *        oversized single message is a client bug, not an attack on the buffer.
 */
void TstServerAuth::jsonMessageSizeLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  QByteArray oversized(kMaxApiMessageBytes + 1, 'x');
  QVERIFY(!reception.validateJsonMessage(&socket, state, oversized));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
  QCOMPARE(state.messageCount, 0);

  const QByteArray atCap =
    QByteArrayLiteral("{") + QByteArray(kMaxApiMessageBytes - 2, 'x') + QByteArrayLiteral("}");
  QCOMPARE(atCap.size(), qsizetype(kMaxApiMessageBytes));
  QVERIFY(reception.validateJsonMessage(&socket, state, atCap));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(state.messageCount, 1);
}

/**
 * @brief Nesting past the depth cap is refused before any JSON parser sees the payload, which is
 *        what keeps a deeply nested document from recursing inside QJsonDocument.
 */
void TstServerAuth::jsonMessageDepthLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  const QByteArray shallow = QByteArray(kMaxApiJsonDepth, '[') + QByteArray(kMaxApiJsonDepth, ']');
  QVERIFY(reception.validateJsonMessage(&socket, state, shallow));
  QCOMPARE(state.messageCount, 1);
  QCOMPARE(host.responses.size(), qsizetype(0));

  const QByteArray tooDeep =
    QByteArray(kMaxApiJsonDepth + 1, '[') + QByteArray(kMaxApiJsonDepth + 1, ']');
  QVERIFY(!reception.validateJsonMessage(&socket, state, tooDeep));
  QCOMPARE(host.responses.size(), qsizetype(1));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));
}

/**
 * @brief The per-window message counter disconnects the client on the first message past the
 *        budget, and every accepted message before it advances the counter by exactly one.
 */
void TstServerAuth::jsonMessageRateLimit()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  const QByteArray message = QByteArrayLiteral("{\"type\":\"command\",\"command\":\"api.ping\"}");
  for (int i = 0; i < kMaxApiMessagesPerWindow; ++i)
    QVERIFY(reception.validateJsonMessage(&socket, state, message));

  QCOMPARE(state.messageCount, kMaxApiMessagesPerWindow);
  QVERIFY(!reception.validateJsonMessage(&socket, state, message));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
}

//--------------------------------------------------------------------------------------------------
// Transport rate limits
//--------------------------------------------------------------------------------------------------

/**
 * @brief The byte budget is per window, so a client that already spent it is dropped on the next
 *        chunk however small that chunk is.
 */
void TstServerAuth::rateLimitsByteWindow()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  state.window.start();
  state.byteCount = kMaxApiBytesPerWindow - 1;
  QVERIFY(reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(0));

  QVERIFY(!reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
}

/**
 * @brief A buffer that cannot absorb the incoming chunk drops the connection instead of growing:
 *        the cap is on the accumulated buffer, not on any single message.
 */
void TstServerAuth::rateLimitsBufferCap()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  state.window.start();
  state.buffer = QByteArray(kMaxApiBufferBytes, 'x');
  QVERIFY(!reception.validateRateLimits(&socket, state, QByteArrayLiteral("a")));
  QCOMPARE(host.disconnectCodes.size(), qsizetype(1));
  QVERIFY(state.buffer.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Raw byte paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bytes with no newline and no JSON opener are the raw device lane: they reach the device
 *        once consent is granted and the buffer is cleared, so nothing is written twice.
 */
void TstServerAuth::noNewlineRawBufferReachesDevice()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("AT+RESET"));
  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("AT+RESET")});
  QVERIFY(state.buffer.isEmpty());

  host.consent = false;
  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("AT+AGAIN"));
  QCOMPARE(host.deviceWrites.size(), qsizetype(1));
  QVERIFY(state.buffer.isEmpty());
}

/**
 * @brief A newline-terminated raw line past the payload cap is answered and then closed, and the
 *        oversized bytes never reach the device.
 */
void TstServerAuth::rawLineOversizeClosesSocket()
{
  QTcpSocket socket;
  StubHost host;
  API::ClientReception reception(host);
  auto state     = authenticatedState();
  host.lastState = &state;

  reception.consumeBytes(&socket, state.sessionId, QByteArray(kMaxApiRawBytes + 1, 'x') + '\n');
  QCOMPARE(host.closes, 1);
  QCOMPARE(host.responses.size(), qsizetype(1));
  QVERIFY(host.deviceWrites.isEmpty());

  reception.consumeBytes(&socket, state.sessionId, QByteArrayLiteral("short\n"));
  QCOMPARE(host.deviceWrites, QList<QByteArray>{QByteArrayLiteral("short")});
}

QTEST_GUILESS_MAIN(TstServerAuth)

#include "tst_server_auth.moc"
