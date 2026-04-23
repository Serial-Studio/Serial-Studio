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

#include "UI/DashboardWidget.h"

#include "Misc/ThemeManager.h"
#include "UI/Dashboard.h"
#include "UI/WidgetRegistry.h"
#include "UI/Widgets/Accelerometer.h"
#include "UI/Widgets/Bar.h"
#include "UI/Widgets/Compass.h"
#include "UI/Widgets/DataGrid.h"
#include "UI/Widgets/FFTPlot.h"
#include "UI/Widgets/Gauge.h"
#include "UI/Widgets/GPS.h"
#include "UI/Widgets/Gyroscope.h"
#include "UI/Widgets/LEDPanel.h"
#include "UI/Widgets/MultiPlot.h"
#include "UI/Widgets/Plot.h"

#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/ImageView.h"
#  include "UI/Widgets/Output/Panel.h"
#  include "UI/Widgets/Plot3D.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

UI::DashboardWidget::DashboardWidget(QQuickItem* parent)
  : QQuickItem(parent)
  , m_index(-1)
  , m_relativeIndex(-1)
  , m_widgetType(SerialStudio::DashboardNoWidget)
  , m_qmlPath("")
  , m_dbWidget(nullptr)
{
  connect(
    this, &UI::DashboardWidget::widgetIndexChanged, this, &UI::DashboardWidget::widgetColorChanged);
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &UI::DashboardWidget::widgetColorChanged);
}

UI::DashboardWidget::~DashboardWidget()
{
  if (m_dbWidget)
    m_dbWidget->deleteLater();
}

//--------------------------------------------------------------------------------------------------
// Widget properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the global index of the widget.
 */
int UI::DashboardWidget::widgetIndex() const
{
  return m_index;
}

/**
 * @brief Returns the relative index of the widget within its type.
 */
int UI::DashboardWidget::relativeIndex() const
{
  return m_relativeIndex;
}

//--------------------------------------------------------------------------------------------------
// Color & style
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the color of the widget based on the current theme.
 */
QColor UI::DashboardWidget::widgetColor() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex)) {
    if (SerialStudio::isDatasetWidget(m_widgetType)) {
      const auto& dataset = GET_DATASET(m_widgetType, m_relativeIndex);
      return QColor(SerialStudio::getDatasetColor(dataset.index));
    }
  }

  return QColor::fromRgba(qRgba(0, 0, 0, 0));
}

/**
 * @brief Returns the window title for the given widget.
 */
QString UI::DashboardWidget::widgetTitle() const
{
  if (VALIDATE_WIDGET(m_widgetType, m_relativeIndex)) {
    if (SerialStudio::isDatasetWidget(m_widgetType)) {
      const auto& dataset = GET_DATASET(m_widgetType, m_relativeIndex);
      return dataset.title;
    }

    else if (SerialStudio::isGroupWidget(m_widgetType)) {
      const auto& group = GET_GROUP(m_widgetType, m_relativeIndex);
      return group.title;
    }
  }

  return tr("Invalid");
}

/**
 * @brief Returns the type of the current widget.
 */
SerialStudio::DashboardWidget UI::DashboardWidget::widgetType() const
{
  return m_widgetType;
}

/**
 * @brief Returns a stable string key identifying this specific widget instance.
 */
QString UI::DashboardWidget::widgetId() const
{
  auto id   = UI::WidgetRegistry::instance().widgetIdByTypeAndIndex(m_widgetType, m_relativeIndex);
  auto info = UI::WidgetRegistry::instance().widgetInfo(id);
  return QStringLiteral("%1:%2:%3")
    .arg(static_cast<int>(m_widgetType))
    .arg(info.groupId)
    .arg(info.datasetIndex);
}

/**
 * @brief Returns the source/device index of the group this widget belongs to.
 */
int UI::DashboardWidget::widgetSourceId() const
{
  if (!VALIDATE_WIDGET(m_widgetType, m_relativeIndex))
    return 0;

  if (SerialStudio::isGroupWidget(m_widgetType)) {
    const auto& group = GET_GROUP(m_widgetType, m_relativeIndex);
    return group.sourceId;
  }

  if (SerialStudio::isDatasetWidget(m_widgetType)) {
    const auto& dataset = GET_DATASET(m_widgetType, m_relativeIndex);
    return dataset.sourceId;
  }

  return 0;
}

/**
 * @brief Returns the QML path of the current widget.
 */
QString UI::DashboardWidget::widgetQmlPath() const
{
  return m_qmlPath;
}

/**
 * @brief Returns the model item of the current widget.
 */
QQuickItem* UI::DashboardWidget::widgetModel() const
{
  return m_dbWidget;
}

/**
 * @brief Selects and configures the appropriate widget for the given @a index.
 */
void UI::DashboardWidget::setWidgetIndex(const int index)
{
  if (index < UI::Dashboard::instance().totalWidgetCount() && index >= 0) {
    m_index         = index;
    m_widgetType    = UI::Dashboard::instance().widgetType(index);
    m_relativeIndex = UI::Dashboard::instance().relativeIndex(index);

    // Delete previous widget before constructing replacement
    if (m_dbWidget) {
      m_dbWidget->deleteLater();
      m_dbWidget = nullptr;
    }

    switch (widgetType()) {
      case SerialStudio::DashboardDataGrid:
        m_dbWidget = new Widgets::DataGrid(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/DataGrid.qml";
        break;
      case SerialStudio::DashboardMultiPlot:
        m_dbWidget = new Widgets::MultiPlot(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/MultiPlot.qml";
        break;
      case SerialStudio::DashboardFFT:
        m_dbWidget = new Widgets::FFTPlot(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/FFTPlot.qml";
        break;
      case SerialStudio::DashboardPlot:
        m_dbWidget = new Widgets::Plot(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot.qml";
        break;
      case SerialStudio::DashboardBar:
        m_dbWidget = new Widgets::Bar(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Bar.qml";
        break;
      case SerialStudio::DashboardGauge:
        m_dbWidget = new Widgets::Gauge(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Gauge.qml";
        break;
      case SerialStudio::DashboardCompass:
        m_dbWidget = new Widgets::Compass(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Compass.qml";
        break;
      case SerialStudio::DashboardGyroscope:
        m_dbWidget = new Widgets::Gyroscope(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Gyroscope.qml";
        break;
      case SerialStudio::DashboardAccelerometer:
        m_dbWidget = new Widgets::Accelerometer(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Accelerometer.qml";
        break;
      case SerialStudio::DashboardTerminal:
        m_dbWidget = nullptr;
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Terminal.qml";
        break;
      case SerialStudio::DashboardGPS:
        m_dbWidget = new Widgets::GPS(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/GPS.qml";
        break;
      case SerialStudio::DashboardLED:
        m_dbWidget = new Widgets::LEDPanel(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/LEDPanel.qml";
        break;
#ifdef BUILD_COMMERCIAL
      case SerialStudio::DashboardPlot3D:
        m_dbWidget = new Widgets::Plot3D(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Plot3D.qml";
        break;
      case SerialStudio::DashboardImageView:
        m_dbWidget = new Widgets::ImageView(relativeIndex(), this);
        m_qmlPath  = "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/ImageView.qml";
        break;
      case SerialStudio::DashboardOutputPanel:
        m_dbWidget = new Widgets::Output::Panel(relativeIndex(), this);
        m_qmlPath =
          "qrc:/serial-studio.com/gui/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml";
        break;
#endif

      default:
        break;
    }

    if (m_dbWidget)
      m_dbWidget->setParentItem(this);

    Q_EMIT widgetIndexChanged();
  }
}
