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

namespace Misc::Diagnostics {

/**
 * @brief What this process can actually do with one device node, plus the group facts a remedy
 *        needs. accessKnown is false where the platform has no POSIX permission model, and only
 *        exists carries meaning there.
 */
struct DeviceAccess {
  bool exists          = false;
  bool readable        = false;
  bool writable        = false;
  bool accessKnown     = false;
  bool accountInGroup  = false;
  bool sessionHasGroup = false;
  QString ownerGroup;
};

/**
 * @brief Returns the login name of the account this process runs as, empty when unresolvable.
 */
[[nodiscard]] QString currentAccountName();

/**
 * @brief Probes @p path for existence, read/write access, and the owning group's membership
 *        facts. access(2) decides readable/writable, so a udev ACL or a logind uaccess grant is
 *        not reported as a failure the way a naive group comparison would.
 */
[[nodiscard]] DeviceAccess probeDeviceNode(const QString& path);

}  // namespace Misc::Diagnostics
