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

#include "DataModel/Editors/CodeFormatter.h"

class QCodeEditor;

namespace DataModel {

/**
 * @brief Applies CodeFormatter to a live QCodeEditor document: one undo step, caret preserved.
 *        Every editor in the app used to carry its own copy of the same cursor dance.
 */
namespace EditorFormatting {

void formatDocument(QCodeEditor& editor, CodeFormatter::Language language);

void formatSelection(QCodeEditor& editor, CodeFormatter::Language language);

}  // namespace EditorFormatting
}  // namespace DataModel
