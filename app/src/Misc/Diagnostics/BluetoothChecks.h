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

#include <QList>

#include "Misc/Diagnostics/DiagnosticsShared.h"

namespace Misc::Diagnostics::BluetoothChecks {

/**
 * @brief Appends the instant Bluetooth verdicts: platform support, adapter powered on, and the
 *        permission status. Every one of them reads state that already exists -- no discovery
 *        agent is constructed, no scan is started, and no permission is ever requested.
 */
void collect(QList<Result>& out);

}  // namespace Misc::Diagnostics::BluetoothChecks
