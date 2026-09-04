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

#include "IO/Drivers/OpcUa/OpcUaEndpointSelection.h"

#include <QCoreApplication>

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

/**
 * @brief The bonus score handed to the endpoint that matches the configured policy and mode, set
 *        above anything the ranking below can reach so an explicit choice always wins.
 */
static constexpr int kConfiguredScore = 1000;

/**
 * @brief Translates a picker label in the OPC UA driver's context, which is where the catalogs
 *        already carry these entries.
 */
[[nodiscard]] static QString trOpcUa(const char* text)
{
  return QCoreApplication::translate("IO::Drivers::OpcUa", text);
}

/**
 * @brief Every security policy this build can open, weakest first.
 */
const QStringList& IO::Drivers::OpcUaEndpointSelection::supportedPolicies()
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
bool IO::Drivers::OpcUaEndpointSelection::policyIsDeprecated(const QString& policyUri)
{
  return policyUri.endsWith(QLatin1String("#Basic128Rsa15"))
      || policyUri.endsWith(QLatin1String("#Basic256"));
}

/**
 * @brief True when this build can dial the endpoint. Every policy in kPolicyUris is supported, so
 *        an endpoint is usable when its policy is one of them and its mode is a real one.
 */
bool IO::Drivers::OpcUaEndpointSelection::endpointUsable(const OpcUaTypes::Endpoint& endpoint)
{
  if (endpoint.securityMode == OpcUaTypes::SecurityMode::Invalid)
    return false;

  return supportedPolicies().contains(endpoint.securityPolicyUri);
}

/**
 * @brief True when the endpoint advertises a user token the selected authentication mode can
 *        present; a server offering only Anonymous rejects a username session outright.
 */
bool IO::Drivers::OpcUaEndpointSelection::endpointAcceptsToken(const OpcUaTypes::Endpoint& endpoint,
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
 * @brief Picks the endpoint to dial after a discovery. A URL that was already selected keeps its
 *        place; otherwise the MOST secure usable endpoint wins. A deprecated policy is never
 *        scored: a server offering nothing else leaves the choice empty rather than being dialed
 *        over Basic128Rsa15, which is what "never auto-selected" has to mean to be true.
 */
IO::Drivers::OpcUaEndpointSelection::EndpointChoice IO::Drivers::OpcUaEndpointSelection::
  selectBestEndpoint(const QList<OpcUaTypes::Endpoint>& endpoints,
                     const int authMode,
                     const QString& preferredPolicy,
                     const int preferredMode,
                     const QString& previousUrl)
{
  EndpointChoice choice;

  int bestScore = -1;
  for (int i = 0; i < endpoints.size(); ++i) {
    const auto& candidate = endpoints.at(i);
    if (!endpointUsable(candidate) || !endpointAcceptsToken(candidate, authMode))
      continue;

    if (!previousUrl.isEmpty() && candidate.endpointUrl == previousUrl) {
      choice.index        = i;
      choice.keptPrevious = true;
      return choice;
    }

    const bool wanted = candidate.securityPolicyUri == preferredPolicy
                     && static_cast<int>(candidate.securityMode) == preferredMode;
    if (!wanted && policyIsDeprecated(candidate.securityPolicyUri))
      continue;

    const int score = wanted ? kConfiguredScore
                             : supportedPolicies().indexOf(candidate.securityPolicyUri) * 10
                                 + static_cast<int>(candidate.securityMode);
    if (score <= bestScore)
      continue;

    bestScore    = score;
    choice.index = i;
  }

  return choice;
}

//--------------------------------------------------------------------------------------------------
// Presentation
//--------------------------------------------------------------------------------------------------

/**
 * @brief The translated name of a message security mode.
 */
QString IO::Drivers::OpcUaEndpointSelection::describeMode(OpcUaTypes::SecurityMode mode)
{
  switch (mode) {
    case OpcUaTypes::SecurityMode::None:
      return trOpcUa("None");
    case OpcUaTypes::SecurityMode::Sign:
      return trOpcUa("Sign");
    case OpcUaTypes::SecurityMode::SignAndEncrypt:
      return trOpcUa("Sign and Encrypt");
    case OpcUaTypes::SecurityMode::Invalid:
      break;
  }

  return trOpcUa("Invalid");
}

/**
 * @brief "policy / mode / url" rows for the endpoint combo.
 */
QStringList IO::Drivers::OpcUaEndpointSelection::endpointRows(
  const QList<OpcUaTypes::Endpoint>& endpoints)
{
  QStringList list;
  for (const auto& endpoint : endpoints) {
    auto policy = endpoint.securityPolicyUri.section(QLatin1Char('#'), -1);
    if (policyIsDeprecated(endpoint.securityPolicyUri))
      policy += trOpcUa(" (deprecated)");

    list.append(QStringLiteral("%1 / %2 / %3")
                  .arg(policy, describeMode(endpoint.securityMode), endpoint.endpointUrl));
  }

  return list;
}

/**
 * @brief Parallel to endpointRows(): true where the row can be dialed by this build.
 */
QVariantList IO::Drivers::OpcUaEndpointSelection::endpointSelectable(
  const QList<OpcUaTypes::Endpoint>& endpoints, const int authMode)
{
  QVariantList list;
  for (const auto& endpoint : endpoints)
    list.append(endpointUsable(endpoint) && endpointAcceptsToken(endpoint, authMode));

  return list;
}

/**
 * @brief Short names of the supported policies, for the picker.
 */
QStringList IO::Drivers::OpcUaEndpointSelection::policyNames()
{
  QStringList out;
  for (const auto& uri : supportedPolicies()) {
    const auto name = uri.section(QLatin1Char('#'), -1);
    out.append(policyIsDeprecated(uri) ? trOpcUa("%1 (deprecated)").arg(name) : name);
  }

  return out;
}

/**
 * @brief Parallel to policyNames(): true where the row is a deprecated policy.
 */
QVariantList IO::Drivers::OpcUaEndpointSelection::policyDeprecationFlags()
{
  QVariantList out;
  for (const auto& uri : supportedPolicies())
    out.append(policyIsDeprecated(uri));

  return out;
}

/**
 * @brief Translated message-security-mode labels, indexed by the OPC UA enumeration.
 */
QStringList IO::Drivers::OpcUaEndpointSelection::securityModeNames()
{
  return {trOpcUa("Invalid"), trOpcUa("None"), trOpcUa("Sign"), trOpcUa("Sign and Encrypt")};
}
