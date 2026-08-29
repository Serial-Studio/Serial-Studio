/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QString>
#include <QTest>

#include "IO/Drivers/OpcUa/OpcUaEndpointSelection.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

using namespace IO::Drivers;
using namespace IO::Drivers::OpcUaEndpointSelection;

/**
 * @brief Builds an advertised endpoint description; `policy` is a short policy name, which is
 *        expanded to the full OPC Foundation URI unless it already looks like one.
 */
static OpcUaTypes::Endpoint makeEndpoint(const QString& url,
                                         const QString& policy,
                                         OpcUaTypes::SecurityMode mode,
                                         const QList<OpcUaTypes::UserTokenType>& tokens = {})
{
  OpcUaTypes::Endpoint endpoint;
  endpoint.endpointUrl = url;
  endpoint.securityPolicyUri =
    policy.startsWith(QLatin1String("http"))
      ? policy
      : QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#%1").arg(policy);
  endpoint.securityMode   = mode;
  endpoint.userTokenTypes = tokens;
  return endpoint;
}

/**
 * @brief Pins the OPC UA policy catalogue and the endpoint ranking a discovery settles on
 *        (spec 0067), including the two rules a weaker one would silently break: a deprecated
 *        policy is never auto-selected, and a previously selected URL keeps its place.
 */
class TstOpcUaEndpointSelection : public QObject {
  Q_OBJECT

private slots:
  void supportedPolicies_catalogue();

  void policyIsDeprecated_data();
  void policyIsDeprecated();

  void endpointUsable_rejectsInvalidMode();
  void endpointUsable_rejectsUnknownPolicy();
  void endpointUsable_acceptsCatalogue();

  void endpointAcceptsToken_emptyListIsPermissive();
  void endpointAcceptsToken_matchesAuthMode();

  void selectBestEndpoint_emptyList();
  void selectBestEndpoint_keepsPreviousUrl();
  void selectBestEndpoint_ignoresUnusablePrevious();
  void selectBestEndpoint_prefersConfiguredPolicy();
  void selectBestEndpoint_prefersStrongerPolicy();
  void selectBestEndpoint_avoidsDeprecatedPolicy();
  void selectBestEndpoint_skipsTokenMismatch();
  void selectBestEndpoint_noneUsable();
};

//--------------------------------------------------------------------------------------------------
// The policy catalogue
//--------------------------------------------------------------------------------------------------

/**
 * @brief The catalogue is ordered weakest first: the ranking scores a policy by its position in
 *        this list, so the order is behaviour and not presentation.
 */
void TstOpcUaEndpointSelection::supportedPolicies_catalogue()
{
  const auto& policies = supportedPolicies();

  QCOMPARE(policies.size(), qsizetype(6));
  QCOMPARE(policies.first(), QString::fromLatin1(kPolicyNoneUri));
  QVERIFY(
    policies.indexOf(QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"))
    > policies.indexOf(QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Basic256")));
  QCOMPARE(policies.last(),
           QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss"));
}

//--------------------------------------------------------------------------------------------------
// policyIsDeprecated
//--------------------------------------------------------------------------------------------------

void TstOpcUaEndpointSelection::policyIsDeprecated_data()
{
  QTest::addColumn<QString>("policyUri");
  QTest::addColumn<bool>("expected");

  const QString prefix = QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#");

  QTest::newRow("Basic128Rsa15 is deprecated") << prefix + QStringLiteral("Basic128Rsa15") << true;
  QTest::newRow("Basic256 is deprecated") << prefix + QStringLiteral("Basic256") << true;
  QTest::newRow("Basic256Sha256 is current") << prefix + QStringLiteral("Basic256Sha256") << false;
  QTest::newRow("None is current") << prefix + QStringLiteral("None") << false;
  QTest::newRow("Aes256 is current") << prefix + QStringLiteral("Aes256_Sha256_RsaPss") << false;
  QTest::newRow("empty") << QString() << false;
}

/**
 * @brief The suffix match must not swallow Basic256Sha256, which is a current policy whose name
 *        starts with a deprecated one.
 */
void TstOpcUaEndpointSelection::policyIsDeprecated()
{
  QFETCH(QString, policyUri);
  QFETCH(bool, expected);

  QCOMPARE(IO::Drivers::OpcUaEndpointSelection::policyIsDeprecated(policyUri), expected);
}

//--------------------------------------------------------------------------------------------------
// endpointUsable
//--------------------------------------------------------------------------------------------------

/**
 * @brief An Invalid message security mode is a malformed row, never a dialable one.
 */
void TstOpcUaEndpointSelection::endpointUsable_rejectsInvalidMode()
{
  const auto endpoint = makeEndpoint(QStringLiteral("opc.tcp://plc:4840"),
                                     QStringLiteral("Basic256Sha256"),
                                     OpcUaTypes::SecurityMode::Invalid);

  QVERIFY(!IO::Drivers::OpcUaEndpointSelection::endpointUsable(endpoint));
}

/**
 * @brief A policy outside the catalogue cannot be opened by this build.
 */
void TstOpcUaEndpointSelection::endpointUsable_rejectsUnknownPolicy()
{
  const auto endpoint = makeEndpoint(QStringLiteral("opc.tcp://plc:4840"),
                                     QStringLiteral("Aes128_Sha256_RsaOaep_Future"),
                                     OpcUaTypes::SecurityMode::SignAndEncrypt);

  QVERIFY(!IO::Drivers::OpcUaEndpointSelection::endpointUsable(endpoint));
}

/**
 * @brief Every catalogued policy is dialable, deprecated ones included: they are labelled, not
 *        refused.
 */
void TstOpcUaEndpointSelection::endpointUsable_acceptsCatalogue()
{
  const auto& policies = supportedPolicies();
  for (const auto& policy : policies) {
    const auto endpoint = makeEndpoint(
      QStringLiteral("opc.tcp://plc:4840"), policy, OpcUaTypes::SecurityMode::SignAndEncrypt);
    QVERIFY2(IO::Drivers::OpcUaEndpointSelection::endpointUsable(endpoint), qPrintable(policy));
  }
}

//--------------------------------------------------------------------------------------------------
// endpointAcceptsToken
//--------------------------------------------------------------------------------------------------

/**
 * @brief A server that advertises no user token at all is given the benefit of the doubt; the
 *        alternative would refuse to dial perfectly good endpoints on an empty list.
 */
void TstOpcUaEndpointSelection::endpointAcceptsToken_emptyListIsPermissive()
{
  const auto endpoint = makeEndpoint(
    QStringLiteral("opc.tcp://plc:4840"), QStringLiteral("None"), OpcUaTypes::SecurityMode::None);

  QVERIFY(endpointAcceptsToken(endpoint, 0));
  QVERIFY(endpointAcceptsToken(endpoint, 1));
  QVERIFY(endpointAcceptsToken(endpoint, 2));
}

/**
 * @brief Authentication modes map onto token types 0/1/2 = Anonymous/Username/Certificate, and a
 *        server offering only one of them rejects the others outright.
 */
void TstOpcUaEndpointSelection::endpointAcceptsToken_matchesAuthMode()
{
  const auto anonymous = makeEndpoint(QStringLiteral("opc.tcp://plc:4840"),
                                      QStringLiteral("None"),
                                      OpcUaTypes::SecurityMode::None,
                                      {OpcUaTypes::UserTokenType::Anonymous});
  QVERIFY(endpointAcceptsToken(anonymous, 0));
  QVERIFY(!endpointAcceptsToken(anonymous, 1));
  QVERIFY(!endpointAcceptsToken(anonymous, 2));

  const auto named =
    makeEndpoint(QStringLiteral("opc.tcp://plc:4840"),
                 QStringLiteral("Basic256Sha256"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt,
                 {OpcUaTypes::UserTokenType::Username, OpcUaTypes::UserTokenType::Certificate});
  QVERIFY(!endpointAcceptsToken(named, 0));
  QVERIFY(endpointAcceptsToken(named, 1));
  QVERIFY(endpointAcceptsToken(named, 2));
}

//--------------------------------------------------------------------------------------------------
// selectBestEndpoint
//--------------------------------------------------------------------------------------------------

/**
 * @brief Nothing advertised means nothing selected, and the caller is told it was not a carry-over.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_emptyList()
{
  const auto choice = selectBestEndpoint({}, 0, QString::fromLatin1(kPolicyNoneUri), 1, QString());

  QCOMPARE(choice.index, -1);
  QVERIFY(!choice.keptPrevious);
}

/**
 * @brief A URL the user already selected keeps its place across a re-discovery, and says so: the
 *        driver must NOT then adopt that row's policy and mode as the configured ones.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_keepsPreviousUrl()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Aes256_Sha256_RsaPss"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
    makeEndpoint(
      QStringLiteral("opc.tcp://b:4840"), QStringLiteral("None"), OpcUaTypes::SecurityMode::None),
  };

  const auto choice = selectBestEndpoint(
    endpoints, 0, QString::fromLatin1(kPolicyNoneUri), 1, QStringLiteral("opc.tcp://b:4840"));

  QCOMPARE(choice.index, 1);
  QVERIFY(choice.keptPrevious);
}

/**
 * @brief The carry-over only applies to a row this build can still dial; an endpoint that became
 *        unusable is ranked away rather than kept.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_ignoresUnusablePrevious()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Basic256Sha256"),
                 OpcUaTypes::SecurityMode::Sign),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("Unknown_Policy"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
  };

  const auto choice = selectBestEndpoint(
    endpoints, 0, QString::fromLatin1(kPolicyNoneUri), 1, QStringLiteral("opc.tcp://b:4840"));

  QCOMPARE(choice.index, 0);
  QVERIFY(!choice.keptPrevious);
}

/**
 * @brief An explicit policy and mode the user configured outrank every other row, stronger ones
 *        included.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_prefersConfiguredPolicy()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Aes256_Sha256_RsaPss"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("Basic256Sha256"),
                 OpcUaTypes::SecurityMode::Sign),
  };

  const auto configured =
    QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256");
  const auto choice = selectBestEndpoint(
    endpoints, 0, configured, static_cast<int>(OpcUaTypes::SecurityMode::Sign), QString());

  QCOMPARE(choice.index, 1);
  QVERIFY(!choice.keptPrevious);
}

/**
 * @brief With nothing matching the configuration, the most secure usable row wins.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_prefersStrongerPolicy()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(
      QStringLiteral("opc.tcp://a:4840"), QStringLiteral("None"), OpcUaTypes::SecurityMode::None),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("Basic256Sha256"),
                 OpcUaTypes::SecurityMode::Sign),
    makeEndpoint(QStringLiteral("opc.tcp://c:4840"),
                 QStringLiteral("Aes256_Sha256_RsaPss"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
  };

  const auto choice =
    selectBestEndpoint(endpoints,
                       0,
                       QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Nothing"),
                       3,
                       QString());

  QCOMPARE(choice.index, 2);
}

/**
 * @brief A deprecated policy scores zero, so it loses even to the unencrypted None endpoint and is
 *        only ever dialed when the user asked for it by name.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_avoidsDeprecatedPolicy()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Basic128Rsa15"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("Basic256"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
    makeEndpoint(
      QStringLiteral("opc.tcp://c:4840"), QStringLiteral("None"), OpcUaTypes::SecurityMode::None),
  };

  const auto ranked =
    selectBestEndpoint(endpoints,
                       0,
                       QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Nothing"),
                       3,
                       QString());
  QCOMPARE(ranked.index, 2);

  const auto asked =
    selectBestEndpoint(endpoints,
                       0,
                       QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Basic256"),
                       static_cast<int>(OpcUaTypes::SecurityMode::SignAndEncrypt),
                       QString());
  QCOMPARE(asked.index, 1);
}

/**
 * @brief A row whose advertised tokens cannot carry the selected identity is skipped, however
 *        strong its policy is.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_skipsTokenMismatch()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Aes256_Sha256_RsaPss"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt,
                 {OpcUaTypes::UserTokenType::Anonymous}),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("Basic256Sha256"),
                 OpcUaTypes::SecurityMode::Sign,
                 {OpcUaTypes::UserTokenType::Username}),
  };

  const auto choice =
    selectBestEndpoint(endpoints,
                       1,
                       QStringLiteral("http://opcfoundation.org/UA/SecurityPolicy#Nothing"),
                       3,
                       QString());

  QCOMPARE(choice.index, 1);
}

/**
 * @brief Nothing dialable is reported as no selection at all, which is what makes the driver raise
 *        "no endpoint this build can open" instead of dialing something it cannot.
 */
void TstOpcUaEndpointSelection::selectBestEndpoint_noneUsable()
{
  const QList<OpcUaTypes::Endpoint> endpoints = {
    makeEndpoint(QStringLiteral("opc.tcp://a:4840"),
                 QStringLiteral("Unknown_Policy"),
                 OpcUaTypes::SecurityMode::SignAndEncrypt),
    makeEndpoint(QStringLiteral("opc.tcp://b:4840"),
                 QStringLiteral("None"),
                 OpcUaTypes::SecurityMode::Invalid),
  };

  const auto choice =
    selectBestEndpoint(endpoints, 0, QString::fromLatin1(kPolicyNoneUri), 1, QString());

  QCOMPARE(choice.index, -1);
  QVERIFY(!choice.keptPrevious);
}

QTEST_APPLESS_MAIN(TstOpcUaEndpointSelection)

#include "tst_opcua_endpoint_selection.moc"
