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

#include "UI/Taskbar/TaskbarModel.h"

//--------------------------------------------------------------------------------------------------
// Taskbar model implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the taskbar model for representing dashboard widgets.
 */
UI::TaskbarModel::TaskbarModel(QObject* parent) : QStandardItemModel(parent) {}

/**
 * @brief Returns the role names used for QML data binding.
 */
QHash<int, QByteArray> UI::TaskbarModel::roleNames() const
{
#define BAL(x) QByteArrayLiteral(x)
  static const QHash<int, QByteArray> kNames = {
    {    GroupIdRole,     BAL("groupId")},
    {    IsGroupRole,     BAL("isGroup")},
    {   WindowIdRole,    BAL("windowId")},
    {  GroupNameRole,   BAL("groupName")},
    { WidgetTypeRole,  BAL("widgetType")},
    { WidgetNameRole,  BAL("widgetName")},
    { WidgetIconRole,  BAL("widgetIcon")},
    {WindowStateRole, BAL("windowState")},
    {     IconIdRole,      BAL("iconId")},
  };
#undef BAL

  return kNames;
}
