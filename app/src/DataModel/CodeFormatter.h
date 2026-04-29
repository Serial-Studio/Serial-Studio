/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QString>

namespace DataModel {

/**
 * @brief Lightweight indentation-only formatter for JS and Lua source code.
 */
namespace CodeFormatter {

/** @brief Selects which language grammar drives indentation. */
enum class Language { JavaScript, Lua };

[[nodiscard]] QString formatDocument(const QString& source, Language language,
                                     int indentSpaces = 2);

[[nodiscard]] QString formatLineRange(const QString& source, Language language,
                                      int firstLine, int lastLine,
                                      int indentSpaces = 2);

}  // namespace CodeFormatter
}  // namespace DataModel
