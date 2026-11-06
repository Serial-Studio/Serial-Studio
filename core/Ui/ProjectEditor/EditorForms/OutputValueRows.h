/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "Core/DataModel/Frame.h"

class QStandardItemModel;

namespace DataModel {

/**
 * @brief Appends an output widget's Value Range rows to its property form (spec 0109): the Input
 *        kind of a text field, then Min, Max, Step, Decimals and Units for every numeric control
 *        and Quantize to Step for a Number field. Lives beside EditorForms rather than inside it
 *        because that translation unit is already at its length limit.
 */
void appendOutputValueRows(QStandardItemModel* model, const OutputWidget& widget);

/**
 * @brief Appends the presentation rows of spec 0113: Show Title for every control, Knob Style and
 *        Position Labels for a knob, and a text field's placeholder and send-button dress.
 */
void appendOutputPresentationRows(QStandardItemModel* model, const OutputWidget& widget);

}  // namespace DataModel
