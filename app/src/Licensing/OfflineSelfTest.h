/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

namespace Licensing {

/**
 * @brief Runs headless offline-certificate verifier vectors; returns 0 on success.
 *
 * Uses an embedded throwaway TEST keypair (never the production key) and fixed
 * vectors so the signature, machine-binding, expiry, tier, and malformed-input
 * paths are checked deterministically with no Qt UI and no network access.
 */
[[nodiscard]] int runOfflineSelfTest();

}  // namespace Licensing
