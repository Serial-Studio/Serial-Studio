/*
 * Copyright (c) 2021-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QQuickItem>

#include "SerialStudio.h"

namespace UI
{
/**
 * @brief The DashboardWidget class
 *
 * The DashboardWidget class serves as a container and manager for various
 * dashboard widgets in a QML interface. It dynamically creates and manages
 * QQuickItem-based widgets (including QQuickPaintedItems) based on the widget
 * type specified by the UI::Dashboard class.
 *
 * This class handles the creation, sizing, and visibility of widgets, as well
 * as providing a consistent interface for different widget types to the QML
 * layer. It supports both QQuickItem and QQuickPaintedItem derived widgets,
 * allowing for flexible and efficient rendering strategies.
 *
 * The class also manages special cases like GPS widgets and external windows,
 * providing properties and methods to handle these scenarios.
 */
class DashboardWidget : public QQuickItem
{
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
  Q_PROPERTY(QString widgetIcon
             READ widgetIcon
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QString widgetTitle
             READ widgetTitle
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QString widgetQmlPath
             READ widgetQmlPath
             NOTIFY widgetIndexChanged)
  Q_PROPERTY(QQuickItem* widgetModel
             READ widgetModel
             NOTIFY widgetIndexChanged)
  // clang-format on

signals:
  void widgetIndexChanged();
  void widgetColorChanged();

public:
  DashboardWidget(QQuickItem *parent = 0);
  ~DashboardWidget();

  [[nodiscard]] int widgetIndex() const;
  [[nodiscard]] int relativeIndex() const;
  [[nodiscard]] QColor widgetColor() const;
  [[nodiscard]] QString widgetIcon() const;
  [[nodiscard]] QString widgetTitle() const;
  [[nodiscard]] SerialStudio::DashboardWidget widgetType() const;

  [[nodiscard]] QString widgetQmlPath() const;
  [[nodiscard]] QQuickItem *widgetModel() const;

public slots:
  void setWidgetIndex(const int index);

private:
  int m_index;
  int m_relativeIndex;
  SerialStudio::DashboardWidget m_widgetType;

  QString m_qmlPath;
  QQuickItem *m_dbWidget;
};
} // namespace UI
