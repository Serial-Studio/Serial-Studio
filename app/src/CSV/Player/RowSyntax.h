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

#include <optional>
#include <QByteArrayView>
#include <QString>

namespace CSV {

/**
 * @brief Index of the first top-level @p separator of a raw row (quote-aware, mirroring the
 *        replay splitter's machine), or -1 when the row has a single cell.
 */
[[nodiscard]] qsizetype firstTopLevelSeparator(QByteArrayView row, char separator);

/**
 * @brief Top-level occurrences of @p separator in @p row using a cell-position-independent
 *        quote scanner (any double quote toggles quoted mode, "" escapes): unlike the RFC
 *        splitter's machine, quoted content never scores for ANY candidate, so a quoted cell
 *        cannot leak separators into the sniff of a differently-separated file.
 */
[[nodiscard]] qsizetype topLevelSeparatorCount(QByteArrayView row, char separator);

/**
 * @brief Detects the row separator (spec 0048): candidates scored by summed top-level
 *        occurrence count over header + first data row, comma scored first and winning
 *        ties. A non-comma winner needs a data-row hit AND header count == data count,
 *        so text-cell separators ("1,a;b;c") never re-interpret a comma file.
 */
[[nodiscard]] char sniffSeparator(QByteArrayView headerRow, QByteArrayView dataRow);

/**
 * @brief Seconds-per-tick for a numeric timestamp header (spec 0048 R7): a bracketed unit
 *        ("time(ms)", "t [us]") or a "_unit" suffix ("time_ms") selects the scale; a
 *        header naming no recognizable unit returns nullopt so the caller can ask.
 */
[[nodiscard]] std::optional<double> timestampUnitScale(const QString& header);

}  // namespace CSV
