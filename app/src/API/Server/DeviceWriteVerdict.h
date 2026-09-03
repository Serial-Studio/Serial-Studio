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

namespace API {

/**
 * @brief Answer of the device-write consent gate. ConsentRequired is what keeps the modal off the
 *        receive path (spec 0075 I1): the write is refused now, the prompt is posted queued, and
 *        the client retries once the user answered. Lives in its own header so the reception
 *        machine can be driven by a stub host that links neither ServerAuth nor QtWidgets.
 */
enum class DeviceWriteVerdict {
  Allowed,
  Denied,
  ConsentRequired,
};

}  // namespace API
