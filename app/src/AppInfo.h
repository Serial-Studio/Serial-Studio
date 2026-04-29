/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <qglobal.h>

// clang-format off
#define APP_NAME        PROJECT_DISPNAME
#define APP_VERSION     PROJECT_VERSION
#define APP_DEVELOPER   PROJECT_VENDOR
#define APP_SUPPORT_URL PROJECT_CONTACT
#define APP_UPDATER_URL PROJECT_APPCAST
// clang-format on

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#  define APP_EXECUTABLE QStringLiteral("Serial-Studio")
#else
#  define APP_EXECUTABLE QStringLiteral("serial-studio")
#endif
