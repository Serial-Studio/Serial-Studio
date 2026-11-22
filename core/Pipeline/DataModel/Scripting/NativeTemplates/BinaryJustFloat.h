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

#include <QByteArray>
#include <QByteArrayView>

#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"

namespace DataModel {

/**
 * @brief Bytes of the JustFloat end marker (spec 0127 R18): the little-endian image of a
 *        single-precision positive infinity, 00 00 80 7F.
 */
inline constexpr qsizetype kJustFloatMarkerBytes = 4;

[[nodiscard]] bool endsWithJustFloatMarker(QByteArrayView frame) noexcept;
[[nodiscard]] qsizetype justFloatChannelCount(QByteArrayView frame, bool stripMarker) noexcept;
[[nodiscard]] const INativeTemplate& justFloatTemplate();

}  // namespace DataModel
