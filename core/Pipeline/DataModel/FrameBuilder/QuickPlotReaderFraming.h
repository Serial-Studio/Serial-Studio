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

namespace DataModel {

/**
 * @brief Whether a Quick Plot frame takes the packed binary decode (spec 0111 R14): the builder
 *        holds a binary declaration AND the reader framed the batch with the declared delimiters.
 *        Until the reader is recreated, a line of exactly the packed length would decode as
 *        nonsense, so it keeps the line path.
 */
[[nodiscard]] constexpr bool packedLaneOpen(const bool framingActive,
                                            const bool readerDeclared) noexcept
{
  return framingActive && readerDeclared;
}

/**
 * @brief Whether the reader left the declared framing the builder still holds: the link was
 *        re-made behind a connect edge the builder never saw, so the declaration is stale and the
 *        device's next one must be taken as new.
 */
[[nodiscard]] constexpr bool declaredFramingLost(const bool wasDeclared,
                                                 const bool nowDeclared,
                                                 const bool framingActive) noexcept
{
  return framingActive && wasDeclared && !nowDeclared;
}

}  // namespace DataModel
