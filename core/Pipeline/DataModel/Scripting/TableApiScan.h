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

#include <QRegularExpression>
#include <QString>

#include "Core/SSAssert.h"

namespace DataModel::TableApiScan {

/**
 * @brief True when @p source names any table-API helper (spec 0086), which is what arms per-dataset
 *        capture for its engine. Conservative on purpose: a name inside a comment or string literal
 *        counts, a name assembled at runtime does not. Header-only so the unit tier links no
 * engine.
 */
[[nodiscard]] inline bool referencesTableApi(const QString& source)
{
  static const QRegularExpression s_tableApiName(
    QStringLiteral("\\b(?:tableGet|tableSet|tableHandle|tableHandleMany|tableGetH|tableSetH|"
                   "datasetGetRaw|datasetGetFinal|__ss)\\b"));
  SS_ASSERT_LOG(s_tableApiName.isValid());
  return !source.isEmpty() && s_tableApiName.match(source).hasMatch();
}

}  // namespace DataModel::TableApiScan
