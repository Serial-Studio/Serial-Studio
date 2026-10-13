/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "Core/DataModel/PropertyValidators.h"

#include <cmath>

#include "Core/SerialStudio.h"

static DataModel::PropertyHooks::ColorValidator s_colorValidator         = nullptr;
static DataModel::PropertyHooks::ActionIconResolver s_actionIconResolver = nullptr;

/**
 * @brief Installs the colour parser (the composition root binds QColor::fromString); a null
 *        validator returns the default, which accepts every colour.
 */
void DataModel::PropertyHooks::setColorValidator(ColorValidator validator) noexcept
{
  s_colorValidator = validator;
}

/**
 * @brief Installs the resolver that turns an action's icon string into the URL the QML image
 *        loads (the composition root binds the icon engine); unbound, the string passes through.
 */
void DataModel::PropertyHooks::setActionIconResolver(ActionIconResolver resolver) noexcept
{
  s_actionIconResolver = resolver;
}

/**
 * @brief Returns the icon URL for @p icon through the installed resolver, or @p icon itself.
 */
QString DataModel::PropertyHooks::resolveActionIcon(const QString& icon)
{
  if (!s_actionIconResolver)
    return icon;

  return s_actionIconResolver(icon);
}

/**
 * @brief Returns true when @p color is empty (automatic) or the installed parser accepts it.
 */
bool DataModel::PropertyHooks::isValidColor(const QString& color)
{
  if (color.isEmpty() || !s_colorValidator)
    return true;

  return s_colorValidator(color);
}

/**
 * @brief Returns true when @p index is a usable frame slot (0 = unassigned, 1+ = parser slot).
 */
bool DataModel::PropertyHooks::isValidDatasetIndex(int index)
{
  return index >= 0;
}

/**
 * @brief Returns true when @p deadband is a usable hysteresis width: finite and not negative.
 */
bool DataModel::PropertyHooks::isValidAlarmDeadband(double deadband)
{
  return std::isfinite(deadband) && deadband >= 0;
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
