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

#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

/**
 * @brief The policy catalogue, the endpoint ranking and the human-readable rendering of both
 *        behind an OPC UA dial (spec 0067). Pure functions over advertised endpoint descriptions:
 *        no session, no settings and no driver state, so the ranking and the picker rows can be
 *        pinned by the unit tier without the protocol stack.
 */
namespace OpcUaEndpointSelection {

/**
 * @brief The unencrypted policy, which is both the default and the only one a build without a
 *        client identity can open.
 */
inline constexpr const char* kPolicyNoneUri = "http://opcfoundation.org/UA/SecurityPolicy#None";

/**
 * @brief Where a ranking landed. `keptPrevious` says the row was carried over from an earlier
 *        discovery rather than chosen, which is what tells the caller NOT to adopt the row's
 *        policy and mode as the configured ones.
 */
struct EndpointChoice {
  int index;
  bool keptPrevious;

  EndpointChoice() : index(-1), keptPrevious(false) {}
};

[[nodiscard]] const QStringList& supportedPolicies();
[[nodiscard]] bool policyIsDeprecated(const QString& policyUri);
[[nodiscard]] bool endpointUsable(const OpcUaTypes::Endpoint& endpoint);
[[nodiscard]] bool endpointAcceptsToken(const OpcUaTypes::Endpoint& endpoint, int authMode);

[[nodiscard]] QStringList policyNames();
[[nodiscard]] QStringList securityModeNames();
[[nodiscard]] QVariantList policyDeprecationFlags();
[[nodiscard]] QString describeMode(OpcUaTypes::SecurityMode mode);
[[nodiscard]] QStringList endpointRows(const QList<OpcUaTypes::Endpoint>& endpoints);
[[nodiscard]] QVariantList endpointSelectable(const QList<OpcUaTypes::Endpoint>& endpoints,
                                              int authMode);
[[nodiscard]] EndpointChoice selectBestEndpoint(const QList<OpcUaTypes::Endpoint>& endpoints,
                                                int authMode,
                                                const QString& preferredPolicy,
                                                int preferredMode,
                                                const QString& previousUrl);

}  // namespace OpcUaEndpointSelection
}  // namespace Drivers
}  // namespace IO
