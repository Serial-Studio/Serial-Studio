/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

namespace AI {

/**
 * @brief Status pill rendered by QML for each tool-call card. The integer values are the
 *        contract with ToolCallCard.qml, which compares raw ints; never renumber them.
 */
enum class ToolCallStatus : int {
  Running         = 0,
  AwaitingConfirm = 1,
  Done            = 2,
  Error           = 3,
  Denied          = 4,
  Blocked         = 5,
};

}  // namespace AI
