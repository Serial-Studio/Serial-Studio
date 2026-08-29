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

#include <QObject>
#include <QString>
#include <QVariantList>

namespace UI {
class TaskbarModel;

/**
 * @brief Owns the taskbar's widget-search state: the live filter string and the flat result lists
 *        the search popup and the widget browser bind to, both projected from the taskbar's full
 *        hierarchical model.
 */
class TaskbarSearch : public QObject {
  Q_OBJECT

signals:
  void dismissed();
  void filterChanged();
  void resultsChanged();

public:
  explicit TaskbarSearch(TaskbarModel* model, QObject* parent = nullptr);
  TaskbarSearch(TaskbarSearch&&)                 = delete;
  TaskbarSearch(const TaskbarSearch&)            = delete;
  TaskbarSearch& operator=(TaskbarSearch&&)      = delete;
  TaskbarSearch& operator=(const TaskbarSearch&) = delete;

  [[nodiscard]] QString filter() const;
  [[nodiscard]] QVariantList results() const;
  [[nodiscard]] QVariantList allWidgets() const;

public slots:
  void dismiss();
  void setFilter(const QString& filter);

private:
  TaskbarModel* m_model;
  QString m_filter;
};

}  // namespace UI
