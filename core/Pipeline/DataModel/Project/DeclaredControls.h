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

#pragma once

#include <QString>

#include "Core/DataModel/Frame.h"
#include "DataModel/Project/DeviceDeclaration.h"

/**
 * @brief Declared controls (spec 0111 R7): every transmit function of a declared output group is
 *        the app's command template, and the control's name is the only device text in it, checked
 *        against a fixed character set and JSON-encoded into one marked line. Pure: no Qt object.
 */
namespace DataModel::DeclaredControls {

[[nodiscard]] bool validName(const QString& name);
[[nodiscard]] QString transmitFunction(const QString& name);
[[nodiscard]] bool attach(Group& group, DeviceDeclaration& out);
[[nodiscard]] bool appendReplies(DeviceDeclaration& out);

}  // namespace DataModel::DeclaredControls
