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
class ProjectModel;
}  // namespace DataModel

namespace DataModel {

/**
 * @brief Appends an output widget's state-feedback rows to its property form (spec 0080): where
 *        the control reads its displayed state from, the on-value truth rule for a two-state
 *        control, and how long a request stays outstanding. Lives beside EditorForms rather than
 *        inside it because that translation unit is already at its length limit.
 */
void appendOutputStateRows(QStandardItemModel* model,
                           const OutputWidget& widget,
                           const ProjectModel& project);

/**
 * @brief Resolves a picker index back onto @p widget. Lives beside appendOutputStateRows because
 *        it must walk the project in exactly the same order the picker listed it; splitting the
 *        two across files is how an index silently starts meaning a different source.
 */
void applyOutputStateTarget(OutputWidget& widget, const ProjectModel& project, const int index);

}  // namespace DataModel
