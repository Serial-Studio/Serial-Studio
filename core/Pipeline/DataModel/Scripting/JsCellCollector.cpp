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

#include "DataModel/Scripting/JsCellCollector.h"

#include <QDebug>
#include <QJSValue>
#include <QString>

#include "Core/SSAssert.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kLengthProperty = QStringLiteral("length");

//--------------------------------------------------------------------------------------------------
// Element conversion
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends one JS value as a cell: numbers keep their value with ECMAScript text, everything
 *        else goes through toString() exactly as the list path did (booleans, null, undefined and
 *        objects included), encoded straight into the scratch.
 */
void JsCellCollector::appendValue(const QJSValue& value, ScriptCellRows& rows)
{
  if (value.isNumber()) {
    char text[ScriptCellRows::kNumberTextCapacity];
    const double number = value.toNumber();
    const qsizetype len = formatJsNumber(number, text, ScriptCellRows::kNumberTextCapacity);
    SS_ASSERT_LOG(len > 0);
    rows.appendNumber(number, text, len);
    return;
  }

  const QString text = value.toString();
  SS_ASSERT_LOG(!value.isString() || !text.isNull());
  rows.appendUtf16(text);
}

/**
 * @brief The array's length as a count, 0 for a negative or absent one.
 */
qsizetype JsCellCollector::arrayLength(const QJSValue& array)
{
  SS_ASSERT(array.isArray(), return 0);

  const int length = array.property(kLengthProperty).toInt();
  return length > 0 ? static_cast<qsizetype>(length) : 0;
}

/**
 * @brief Appends the first @p count elements of @p array as one row. With @p rejectArrays (a
 *        top-level flat row) a nested array means the scalar-plus-array shape the unzip path owns:
 *        false, nothing further appended.
 */
bool JsCellCollector::appendArrayRow(const QJSValue& array,
                                     qsizetype count,
                                     ScriptCellRows& rows,
                                     bool rejectArrays)
{
  SS_ASSERT(array.isArray(), return false);
  SS_ASSERT(count >= 0, return false);

  rows.beginRow();
  for (qsizetype i = 0; i < count; ++i) {
    const QJSValue value = array.property(static_cast<quint32>(i));
    if (rejectArrays && value.isArray()) [[unlikely]]
      return false;

    appendValue(value, rows);
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat array -> one row; array of arrays -> one row each, decided by the first element and
 *        checked on the same single walk that appends (every property() read of a non-integer
 *        number costs a heap double inside QJSValue, so the array is read once); a non-array
 *        result or a mixed shape -> false, so the list path keeps its legacy handling of those.
 */
bool JsCellCollector::collect(const QJSValue& result, ScriptCellRows& rows, qsizetype maxElements)
{
  SS_ASSERT(maxElements > 0, return false);

  rows.clear();
  if (!result.isArray())
    return false;

  const auto count = qMin(arrayLength(result), maxElements);
  if (count == 0)
    return true;

  rows.reserve(count, count * ScriptCellRows::kBytesPerCellGuess);
  const bool nested = result.property(0u).isArray();
  if (!nested) {
    if (appendArrayRow(result, count, rows, true))
      return true;

    rows.clear();
    return false;
  }

  for (qsizetype i = 0; i < count; ++i) {
    const QJSValue row = result.property(static_cast<quint32>(i));
    if (!row.isArray()) [[unlikely]] {
      rows.clear();
      return false;
    }

    (void)appendArrayRow(row, qMin(arrayLength(row), maxElements), rows, false);
  }

  return true;
}

}  // namespace DataModel
