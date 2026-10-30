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

#pragma once

#include <QByteArrayView>
#include <QtGlobal>

#include "DataModel/Scripting/LineFormat.h"

namespace DataModel {

/**
 * @brief Deepest object or array nesting the scanner follows.
 */
inline constexpr int kMaxJsonDepth = 8;

/**
 * @brief Longest leaf path (dotted names plus indices) the scanner builds.
 */
inline constexpr qsizetype kMaxJsonPathBytes = 256;

/**
 * @brief Scans one line holding a JSON object and appends one field per numeric leaf, in text
 *        order, with paths such as imu.ax and acc[0] written into the fields' path arena.
 *        Strings, booleans and nulls are skipped. Returns false, with no fields, on malformed
 *        input, a non-object root, a depth or arena overflow, a key containing a comma (the saved
 *        project lists field paths comma-separated) or zero numeric leaves. The scanner exists
 *        because QJsonObject sorts keys and the device's order is the channel order.
 */
[[nodiscard]] bool scanJsonLeaves(QByteArrayView text, LineFields& out) noexcept;

}  // namespace DataModel
