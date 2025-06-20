/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
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
