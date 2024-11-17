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

#include "UI/DashboardWidget.h"

#include "UI/Dashboard.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Plot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Accelerometer.h"

#include "Misc/ThemeManager.h"

/**
 * Constructor function
 */
UI::DashboardWidget::DashboardWidget(QQuickItem *parent)
  : QQuickItem(parent)
  , m_index(-1)
  , m_relativeIndex(-1)
  , m_widgetType(SerialStudio::DashboardNoWidget)
  , m_qmlPath("")
  , m_dbWidget(nullptr)
{
  connect(this, &UI::DashboardWidget::widgetIndexChanged, this,
          &UI::DashboardWidget::widgetColorChanged);
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &UI::DashboardWidget::widgetColorChanged);
}

/**
 * Delete widget on class destruction
 */
UI::DashboardWidget::~DashboardWidget()
{
  if (m_dbWidget)
    m_dbWidget->deleteLater();
}

/**
 * Returns the global index of the widget (index of the current widget in
 * relation to all registered widgets).
 */
int UI::DashboardWidget::widgetIndex() const
{
  return m_index;
}

/**
 * Returns the relative index of the widget (e.g. index of a bar widget in
 * relation to the total number of bar widgets).
 */
int UI::DashboardWidget::relativeIndex() const
{
  return m_relativeIndex;
}

/**
 * Returns the color of the widget based on the current theme & widget index.
 */
QColor UI::DashboardWidget::widgetColor() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex))
  {
    if (SerialStudio::isDatasetWidget(m_widgetType))
    {
      const auto &dataset = GET_DATASET(m_widgetType, m_relativeIndex);
      return QColor(SerialStudio::getDatasetColor(dataset.index()));
    }
  }

  return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

/**
 * Returns the path of the SVG icon to use with this widget
 */
QString UI::DashboardWidget::widgetIcon() const
{
  return SerialStudio::dashboardWidgetIcon(m_widgetType);
}

/**
 * Returns the appropiate window title for the given widget
 */
QString UI::DashboardWidget::widgetTitle() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex))
  {
    if (SerialStudio::isDatasetWidget(m_widgetType))
    {
      const auto &dataset = GET_DATASET(m_widgetType, m_relativeIndex);
      return dataset.title();
    }

    else if (SerialStudio::isGroupWidget(m_widgetType))
    {
      const auto &group = GET_GROUP(m_widgetType, m_relativeIndex);
      return group.title();
    }
  }

  return tr("Invalid");
}

/**
 * Returns the type of the current widget (e.g. group, plot, bar, gauge, etc...)
 */
SerialStudio::DashboardWidget UI::DashboardWidget::widgetType() const
{
  return m_widgetType;
}

/**
 * Returns the QML path of the current widget.
 */
QString UI::DashboardWidget::widgetQmlPath() const
{
  return m_qmlPath;
}

/**
 * Returns the model item of the current widget.
 */
QQuickItem *UI::DashboardWidget::widgetModel() const
{
  return m_dbWidget;
}

/**
 * Selects & configures the appropiate widget for the given @a index
 */
void UI::DashboardWidget::setWidgetIndex(const int index)
{
  if (index < UI::Dashboard::instance().totalWidgetCount() && index >= 0)
  {
    // Update widget index
    m_index = index;
    m_widgetType = UI::Dashboard::instance().widgetType(index);
    m_relativeIndex = UI::Dashboard::instance().relativeIndex(index);

    // Delete previous widget
    if (m_dbWidget)
    {
      m_dbWidget->deleteLater();
      m_dbWidget = nullptr;
    }

    // Construct new widget
    switch (widgetType())
    {
      case SerialStudio::DashboardDataGrid:
        m_dbWidget = new Widgets::DataGrid(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/DataGrid.qml";
        break;
      case SerialStudio::DashboardMultiPlot:
        m_dbWidget = new Widgets::MultiPlot(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/MultiPlot.qml";
        break;
      case SerialStudio::DashboardFFT:
        m_dbWidget = new Widgets::FFTPlot(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/FFTPlot.qml";
        break;
      case SerialStudio::DashboardPlot:
        m_dbWidget = new Widgets::Plot(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Plot.qml";
        break;
      case SerialStudio::DashboardBar:
        m_dbWidget = new Widgets::Bar(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Bar.qml";
        break;
      case SerialStudio::DashboardGauge:
        m_dbWidget = new Widgets::Gauge(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Gauge.qml";
        break;
      case SerialStudio::DashboardCompass:
        m_dbWidget = new Widgets::Compass(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Compass.qml";
        break;
      case SerialStudio::DashboardGyroscope:
        m_dbWidget = new Widgets::Gyroscope(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Gyroscope.qml";
        break;
      case SerialStudio::DashboardAccelerometer:
        m_dbWidget = new Widgets::Accelerometer(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/Accelerometer.qml";
        break;
      case SerialStudio::DashboardGPS:
        m_dbWidget = new Widgets::GPS(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/GPS.qml";
        break;
      case SerialStudio::DashboardLED:
        m_dbWidget = new Widgets::LEDPanel(relativeIndex(), this);
        m_qmlPath = "qrc:/qml/Widgets/Dashboard/LEDPanel.qml";
        break;
      default:
        break;
    }

    // Configure widget
    if (m_dbWidget)
    {
      m_dbWidget->setParentItem(this);
      Q_EMIT widgetIndexChanged();
    }
  }
}
