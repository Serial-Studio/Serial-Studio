/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>

#include "SerialStudio.h"

namespace UI {
/**
 * @brief QML-exposed widget container that dynamically instantiates and
 *        manages dashboard visualization widgets.
 */
class DashboardWidget : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int widgetIndex
             READ widgetIndex
             WRITE setWidgetIndex
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(int relativeIndex
             READ relativeIndex
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QColor widgetColor
             READ widgetColor
             NOTIFY widgetColorChanged)
  Q_PROPERTY(QString widgetTitle
             READ widgetTitle
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QString widgetQmlPath
             READ widgetQmlPath
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QQuickItem* widgetModel
             READ widgetModel
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(SerialStudio::DashboardWidget widgetType
             READ widgetType
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QString widgetId
             READ widgetId
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(int widgetSourceId
             READ widgetSourceId
             NOTIFY widgetIndexChanged)
  // clang-format on

signals:
  void widgetIndexChanged();
  void widgetColorChanged();

public:
  DashboardWidget(QQuickItem* parent = 0);
  ~DashboardWidget();

  [[nodiscard]] int widgetIndex() const;
  [[nodiscard]] int relativeIndex() const;
  [[nodiscard]] QColor widgetColor() const;
  [[nodiscard]] QString widgetTitle() const;
  [[nodiscard]] SerialStudio::DashboardWidget widgetType() const;

  [[nodiscard]] int widgetSourceId() const;
  [[nodiscard]] QString widgetId() const;
  [[nodiscard]] QString widgetQmlPath() const;
  [[nodiscard]] QQuickItem* widgetModel() const;

public slots:
  void setWidgetIndex(const int index);

private:
  int m_index;
  int m_relativeIndex;
  SerialStudio::DashboardWidget m_widgetType;

  QString m_qmlPath;
  QQuickItem* m_dbWidget;
};
}  // namespace UI
