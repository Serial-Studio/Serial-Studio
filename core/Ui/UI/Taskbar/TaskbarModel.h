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

#include <QHash>
#include <QObject>
#include <QStandardItemModel>

namespace UI {

/**
 * @brief QStandardItemModel used to represent dashboard widgets in a hierarchical UI.
 */
class TaskbarModel : public QStandardItemModel {
  Q_OBJECT

public:
  enum Roles {
    WindowIdRole = Qt::UserRole + 1,
    WidgetTypeRole,
    WidgetNameRole,
    WidgetIconRole,
    GroupIdRole,
    GroupNameRole,
    IsGroupRole,
    WindowStateRole,
    IconIdRole,
  };

  enum WindowState {
    WindowNormal    = 0,
    WindowMinimized = 1,
    WindowClosed    = 2
  };
  Q_ENUM(WindowState)

  explicit TaskbarModel(QObject* parent = nullptr);
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

}  // namespace UI
