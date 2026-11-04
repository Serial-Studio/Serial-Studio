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
#include <QStringList>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"

class QStandardItemModel;

namespace IO {
struct DriverProperty;
}  // namespace IO

namespace DataModel {

/**
 * @brief Appends a source's Frame Detection section to its property form: the method picker
 *        (@p methods labels, @p methodValues the enum behind each), then either the Fixed Length
 *        rows (length, header and body rules) or the delimiter rows the method uses. Lives beside
 *        EditorForms rather than inside it because that translation unit is at its length limit.
 */
void appendSourceFramingRows(QStandardItemModel* model,
                             const Source& source,
                             const QStringList& methods,
                             const QList<SerialStudio::FrameDetection>& methodValues);

/**
 * @brief Appends a UART source's Connection Sequence button row, labelled with its step count; a
 *        click reaches EditorCommit as a sentinel and opens the shared editor dialog.
 */
void appendConnectionSequenceRow(QStandardItemModel* model, const QList<IO::DriverProperty>& props);

}  // namespace DataModel
