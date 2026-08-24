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

#include <open62541.h>

#include <QDateTime>
#include <QString>
#include <QVariant>

#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

/**
 * @brief Conversion between open62541's C vocabulary and the Qt types the driver speaks
 *        (spec 0067). Every `UA_` value crossing into Serial Studio passes through here, which
 *        is what keeps the stack out of the driver and gives the ctest tier one seam to pin.
 */
namespace OpcUaMarshal {

[[nodiscard]] QString toQString(const UA_String& string);
[[nodiscard]] QString nodeIdToString(const UA_NodeId& id);
[[nodiscard]] bool nodeIdFromString(const QString& text, UA_NodeId& out);

[[nodiscard]] QDateTime toDateTime(UA_DateTime timestamp);
[[nodiscard]] QDateTime sourceTimeOf(const UA_DataValue& value);

[[nodiscard]] QVariant toVariant(const UA_Variant& variant);
[[nodiscard]] QString statusText(OpcUaTypes::StatusCode status);

[[nodiscard]] OpcUaTypes::NodeClass toNodeClass(quint32 mask) noexcept;
[[nodiscard]] OpcUaTypes::SecurityMode toSecurityMode(int mode) noexcept;
[[nodiscard]] OpcUaTypes::UserTokenType toUserTokenType(int type) noexcept;

}  // namespace OpcUaMarshal
}  // namespace Drivers
}  // namespace IO
