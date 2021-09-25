/*
 * Copyright (c) 2021 Alex Spataru <https://github.com/alex-spataru>
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

#include "WidgetLoader.h"

#include <QPushButton>
#include <QApplication>
#include <UI/Dashboard.h>

using namespace UI;
using namespace Widgets;

/**
 * Constructor function
 */
WidgetLoader::WidgetLoader(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_index(-1)
    , m_widget(nullptr)
    , m_widgetVisible(true)
{
    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Resize widget to fit QML item size
    connect(this, &QQuickPaintedItem::widthChanged, this,
            &WidgetLoader::updateWidgetSize);
    connect(this, &QQuickPaintedItem::heightChanged, this,
            &WidgetLoader::updateWidgetSize);

    // Automatically update the widget's visibility
    connect(Dashboard::getInstance(), &Dashboard::widgetVisibilityChanged, this,
            &WidgetLoader::updateWidgetVisible);
}

/**
 * Delete widget on class destruction
 */
WidgetLoader::~WidgetLoader()
{
    if (m_widget)
        m_widget->deleteLater();
}

/**
 * Handle application events manually
 */
bool WidgetLoader::event(QEvent *event)
{
    if (!m_widget)
        return false;

    switch (event->type())
    {
        case QEvent::FocusIn:
            forceActiveFocus();
            return QQuickPaintedItem::event(event);
            break;
        case QEvent::Wheel:
            processWheelEvents(static_cast<QWheelEvent *>(event));
            return true;
            break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
            processMouseEvents(static_cast<QMouseEvent *>(event));
            return true;
            break;
        default:
            break;
    }

    return QApplication::sendEvent(m_widget, event);
}

/**
 * Render the widget on the given @a painter
 */
void WidgetLoader::paint(QPainter *painter)
{
    if (m_widget && painter)
        m_widget->render(painter);
}

/**
 * Custom event filter to manage redraw requests
 */
bool WidgetLoader::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_widget)
        return false;

    if (watched == m_widget)
    {
        switch (event->type())
        {
            case QEvent::Paint:
            case QEvent::UpdateRequest:
                update();
                break;
            default:
                break;
        }
    }

    return QQuickPaintedItem::eventFilter(watched, event);
}

int WidgetLoader::widgetIndex() const
{
    return m_index;
}

int WidgetLoader::relativeIndex() const
{
    //
    // Warning: relative widget index should be calculated using the same order as defined
    // by the
    //          UI::Dashboard::widgetTitles() function.
    //

    // Get pointer to dashboard module
    auto dash = Dashboard::getInstance();

    // Check if we should return group widget
    int index = widgetIndex();
    if (index < dash->groupCount())
        return index;

    // Check if we should return plot widget
    index -= dash->groupCount();
    if (index < dash->plotCount())
        return index;

    // Check if we should return bar widget
    index -= dash->plotCount();
    if (index < dash->barCount())
        return index;

    // Check if we should return gauge widget
    index -= dash->barCount();
    if (index < dash->gaugeCount())
        return index;

    // Check if we should return thermometer widget
    index -= dash->gaugeCount();
    if (index < dash->thermometerCount())
        return index;

    // Check if we should return compass widget
    index -= dash->thermometerCount();
    if (index < dash->compassCount())
        return index;

    // Check if we should return gyro widget
    index -= dash->compassCount();
    if (index < dash->gyroscopeCount())
        return index;

    // Check if we should return accelerometer widget
    index -= dash->gyroscopeCount();
    if (index < dash->accelerometerCount())
        return index;

    // Check if we should return map widget
    index -= dash->accelerometerCount();
    if (index < dash->mapCount())
        return index;

    // Return unknown widget
    return -1;
}

bool WidgetLoader::widgetVisible() const
{
    return m_widgetVisible;
}

QString WidgetLoader::widgetIcon() const
{
    switch (widgetType())
    {
        case WidgetType::Group:
            return "qrc:/icons/group.svg";
            break;
        case WidgetType::Plot:
            return "qrc:/icons/plot.svg";
            break;
        case WidgetType::Bar:
            return "qrc:/icons/bar.svg";
            break;
        case WidgetType::Gauge:
            return "qrc:/icons/gauge.svg";
            break;
        case WidgetType::Thermometer:
            return "qrc:/icons/thermometer.svg";
            break;
        case WidgetType::Compass:
            return "qrc:/icons/compass.svg";
            break;
        case WidgetType::Gyroscope:
            return "qrc:/icons/gyroscope.svg";
            break;
        case WidgetType::Accelerometer:
            return "qrc:/icons/accelerometer.svg";
            break;
        case WidgetType::Map:
            return "qrc:/icons/map.svg";
            break;
        default:
            return "qrc:/icons/close.svg";
            break;
    }
}

/**
 * Returns the appropiate window title for the given widget
 */
QString WidgetLoader::widgetTitle() const
{
    return UI::Dashboard::getInstance()->widgetTitles().at(widgetIndex());
}

WidgetLoader::WidgetType WidgetLoader::widgetType() const
{
    //
    // Warning: relative widget index should be calculated using the same order as defined
    // by the
    //          UI::Dashboard::widgetTitles() function.
    //

    // Unitialized widget loader class
    if (widgetIndex() < 0)
        return WidgetType::Unknown;

    // Get pointer to dashboard module
    auto dash = UI::Dashboard::getInstance();

    // Check if we should return group widget
    int index = widgetIndex();
    if (index < dash->groupCount())
        return WidgetType::Group;

    // Check if we should return plot widget
    index -= dash->groupCount();
    if (index < dash->plotCount())
        return WidgetType::Plot;

    // Check if we should return bar widget
    index -= dash->plotCount();
    if (index < dash->barCount())
        return WidgetType::Bar;

    // Check if we should return gauge widget
    index -= dash->barCount();
    if (index < dash->gaugeCount())
        return WidgetType::Gauge;

    // Check if we should return thermometer widget
    index -= dash->gaugeCount();
    if (index < dash->thermometerCount())
        return WidgetType::Thermometer;

    // Check if we should return compass widget
    index -= dash->thermometerCount();
    if (index < dash->compassCount())
        return WidgetType::Compass;

    // Check if we should return gyro widget
    index -= dash->compassCount();
    if (index < dash->gyroscopeCount())
        return WidgetType::Gyroscope;

    // Check if we should return accelerometer widget
    index -= dash->gyroscopeCount();
    if (index < dash->accelerometerCount())
        return WidgetType::Accelerometer;

    // Check if we should return map widget
    index -= dash->accelerometerCount();
    if (index < dash->mapCount())
        return WidgetType::Map;

    // Return unknown widget
    return WidgetType::Unknown;
}

/**
 * Shows a window with the current widget
 */
void WidgetLoader::displayWindow()
{
    if (m_widget)
        m_widget->showNormal();
}

/**
 * Selects & configures the appropiate widget for the given @a index
 */
void WidgetLoader::setWidgetIndex(const int index)
{
    if (m_index != index && index < Dashboard::getInstance()->totalWidgetCount()
        && index >= 0)
    {
        // Update widget index
        m_index = index;

        // Delete previous widget
        if (m_widget)
        {
            m_widget->deleteLater();
            m_widget = nullptr;
        }

        // Construct new widget
        switch (widgetType())
        {
            case WidgetType::Group:
                m_widget = new QPushButton("Group");
                break;
            case WidgetType::Plot:
                m_widget = new QPushButton("Plot");
                break;
            case WidgetType::Bar:
                m_widget = new QPushButton("Bar");
                break;
            case WidgetType::Gauge:
                m_widget = new QPushButton("Gauge");
                break;
            case WidgetType::Thermometer:
                m_widget = new QPushButton("Thermometer");
                break;
            case WidgetType::Compass:
                m_widget = new QPushButton("Compass");
                break;
            case WidgetType::Gyroscope:
                m_widget = new QPushButton("Gyroscope");
                break;
            case WidgetType::Accelerometer:
                m_widget = new QPushButton("Accelerometer");
                break;
            case WidgetType::Map:
                m_widget = new QPushButton("Map");
                break;
            default:
                break;
        }

        // Allow widget to receive events from the QML interface
        if (m_widget)
        {
            m_widget->installEventFilter(this);
            updateWidgetVisible();
        }

        // Update UI
        emit widgetIndexChanged();
    }
}

/**
 * Resizes the widget to fit inside the QML item.
 */
void WidgetLoader::updateWidgetSize()
{
    if (m_widget && width() > 0 && height() > 0)
    {
        m_widget->setFixedSize(width(), height());
        update();
    }
}

void WidgetLoader::updateWidgetVisible()
{
    bool visible = false;
    auto index = relativeIndex();
    auto dash = Dashboard::getInstance();

    switch (widgetType())
    {
        case WidgetType::Group:
            visible = dash->groupVisible(index);
            break;
        case WidgetType::Plot:
            visible = dash->plotVisible(index);
            break;
        case WidgetType::Bar:
            visible = dash->barVisible(index);
            break;
        case WidgetType::Gauge:
            visible = dash->gaugeVisible(index);
            break;
        case WidgetType::Thermometer:
            visible = dash->thermometerVisible(index);
            break;
        case WidgetType::Compass:
            visible = dash->compassVisible(index);
            break;
        case WidgetType::Gyroscope:
            visible = dash->gyroscopeVisible(index);
            break;
        case WidgetType::Accelerometer:
            visible = dash->accelerometerVisible(index);
            break;
        case WidgetType::Map:
            visible = dash->mapVisible(index);
            break;
        default:
            visible = false;
            break;
    }

    if (widgetVisible() != visible)
    {
        m_widgetVisible = visible;
        emit widgetVisibleChanged();
    }
}

/**
 * Hack: calls the appropiate protected mouse event handler function of the widget item
 */
void WidgetLoader::processMouseEvents(QMouseEvent *event)
{
    if (!m_widget)
        return;

    class Hack : public QWidget
    {
    public:
        using QWidget::mouseDoubleClickEvent;
        using QWidget::mouseMoveEvent;
        using QWidget::mousePressEvent;
        using QWidget::mouseReleaseEvent;
    };

    auto hack = static_cast<Hack *>(m_widget);
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
            hack->mousePressEvent(event);
            break;
        case QEvent::MouseMove:
            hack->mouseMoveEvent(event);
            break;
        case QEvent::MouseButtonRelease:
            hack->mouseReleaseEvent(event);
            break;
        case QEvent::MouseButtonDblClick:
            hack->mouseDoubleClickEvent(event);
            break;
        default:
            break;
    }
}

/**
 * Hack: calls the appropiate protected wheel event handler function of the widget item
 */
void WidgetLoader::processWheelEvents(QWheelEvent *event)
{
    if (!m_widget)
        return;

    class Hack : public QWidget
    {
    public:
        using QWidget::wheelEvent;
    };

    static_cast<Hack *>(m_widget)->wheelEvent(event);
}
