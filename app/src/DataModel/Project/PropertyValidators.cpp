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

#include <QColor>

#include "DataModel/Project/PropertyHooks.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Validators that reach no project state; apart from PropertyHooks.cpp so the unit suites linking
// the generated readers do not pull ProjectModel, and so the whole application, in with them
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when @p color is empty (automatic) or a colour Qt can parse.
 */
bool DataModel::PropertyHooks::isValidColor(const QString& color)
{
  if (color.isEmpty())
    return true;

  return QColor::fromString(color).isValid();
}

/**
 * @brief Returns true when @p index is a usable frame slot (0 = unassigned, 1+ = parser slot).
 */
bool DataModel::PropertyHooks::isValidDatasetIndex(int index)
{
  return index >= 0;
}

/**
 * @brief Returns true when @p window names a SerialStudio::FFTWindow value.
 */
bool DataModel::PropertyHooks::isValidFftWindow(int window)
{
  return window >= SerialStudio::FFTWindowRectangular && window <= SerialStudio::FFTWindowParzen;
}

/**
 * @brief Returns true when @p language is inherit (-1), JavaScript, Lua or a compiled Expression
 *        (spec 0060). Native is a frame-parser language only.
 */
bool DataModel::PropertyHooks::isValidTransformLanguage(int language)
{
  return language == -1 || language == SerialStudio::JavaScript || language == SerialStudio::Lua
      || language == SerialStudio::Expression;
}
