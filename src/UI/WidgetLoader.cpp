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

#include "Dashboard.h"
#include "WidgetLoader.h"

#include <QApplication>

#include <Widgets/Bar.h>
#include <Widgets/GPS.h>
#include <Widgets/Plot.h>
#include <Widgets/Gauge.h>
#include <Widgets/Compass.h>
#include <Widgets/FFTPlot.h>
#include <Widgets/LEDPanel.h>
#include <Widgets/DataGroup.h>
#include <Widgets/Gyroscope.h>
#include <Widgets/MultiPlot.h>
#include <Widgets/Accelerometer.h>

#include <Misc/ThemeManager.h>

namespace UI
{
/**
 * Constructor function
 */
WidgetLoader::WidgetLoader(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_index(-1)
    , m_widgetVisible(false)
    , m_isExternalWindow(false)
{
    // Set render flags
    setAntialiasing(true);
    setOpaquePainting(true);
    setRenderTarget(FramebufferObject);
    setPerformanceHints(FastFBOResizing);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemIsFocusScope, true);
    setFlag(ItemAcceptsInputMethod, true);
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
        delete m_widget;
}

/**
 * Handle application events manually
 */
bool WidgetLoader::event(QEvent *event)
{
    // Check that widget exists
    if (!m_widget)
        return false;

    // Process focus, wheel & mouse click/release events
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

    //
    // Note: mouse enter/leave events must be processed directly with
    //       the help of a QML MouseArea
    //
    return QApplication::sendEvent(m_widget, event);
}

/**
 * Render the widget on the given @a painter
 */
void WidgetLoader::paint(QPainter *painter)
{
    if (m_widget && painter && isVisible())
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

/**
 * Returns the global index of the widget (index of the current widget in relation to all
 * registered widgets).
 */
int WidgetLoader::widgetIndex() const
{
    return m_index;
}

/**
 * Returns the relative index of the widget (e.g. index of a bar widget in relation to the
 * total number of bar widgets).
 */
int WidgetLoader::relativeIndex() const
{
    return UI::Dashboard::getInstance()->relativeIndex(widgetIndex());
}

/**
 * Returns @c true if the QML interface should display this widget.
 */
bool WidgetLoader::widgetVisible() const
{
    return m_widgetVisible;
}

/**
 * Returns the path of the SVG icon to use with this widget
 */
QString WidgetLoader::widgetIcon() const
{
    return UI::Dashboard::getInstance()->widgetIcon(widgetIndex());
}

/**
 * Returns the appropiate window title for the given widget
 */
QString WidgetLoader::widgetTitle() const
{
    if (widgetIndex() >= 0)
    {
        auto titles = UI::Dashboard::getInstance()->widgetTitles();
        if (widgetIndex() < titles.count())
            return titles.at(widgetIndex());
    }

    return tr("Invalid");
}

/**
 * If set to @c true, then the widget visibility shall be controlled
 * directly by the QML interface.
 *
 * If set to @c false, then the widget visbility shall be controlled
 * by the UI::Dashboard class via the SIGNAL/SLOT system.
 */
bool WidgetLoader::isExternalWindow() const
{
    return m_isExternalWindow;
}

/**
 * Returns the type of the current widget (e.g. group, plot, bar, gauge, etc...)
 */
UI::Dashboard::WidgetType WidgetLoader::widgetType() const
{
    return UI::Dashboard::getInstance()->widgetType(widgetIndex());
}

/**
 * Returns a pointer to the current widget
 */
QWidget* WidgetLoader::widget() const
{
    return m_widget;
}

/**
 * Changes the visibility & enabled status of the widget
 */
void WidgetLoader::setVisible(const bool visible)
{
    if (m_widget)
        m_widget->setEnabled(visible);
}

/**
 * Selects & configures the appropiate widget for the given @a index
 */
void WidgetLoader::setWidgetIndex(const int index)
{
    if (index < UI::Dashboard::getInstance()->totalWidgetCount() && index >= 0)
    {
        // Update widget index
        m_index = index;

        // Delete previous widget
        if (m_widget)
        {
            delete m_widget;
            m_widget = nullptr;
        }

        // Construct new widget
        switch (widgetType())
        {
            case UI::Dashboard::WidgetType::Group:
                m_widget = new Widgets::DataGroup(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::MultiPlot:
                m_widget = new Widgets::MultiPlot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::FFT:
                m_widget = new Widgets::FFTPlot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Plot:
                m_widget = new Widgets::Plot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Bar:
                m_widget = new Widgets::Bar(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Gauge:
                m_widget = new Widgets::Gauge(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Compass:
                m_widget = new Widgets::Compass(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Gyroscope:
                m_widget = new Widgets::Gyroscope(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Accelerometer:
                m_widget = new Widgets::Accelerometer(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::GPS:
                m_widget = new Widgets::GPS(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::LED:
                m_widget = new Widgets::LEDPanel(relativeIndex());
                break;
            default:
                break;
        }

        // Allow widget to receive events from the QML interface
        if (m_widget)
        {
            m_widget->setEnabled(true);
            m_widget->installEventFilter(this);
            Q_EMIT widgetIndexChanged();
            updateWidgetVisible();
        }
    }
}

/**
 * Changes the widget visibility controller source.
 *
 * If set to @c true, then the widget visibility shall be controlled
 * directly by the QML interface.
 *
 * If set to @c false, then the widget visbility shall be controlled
 * by the UI::Dashboard class via the SIGNAL/SLOT system.
 */
void WidgetLoader::setIsExternalWindow(const bool isWindow)
{
    m_isExternalWindow = isWindow;
    Q_EMIT isExternalWindowChanged();
}

/**
 * This function must be called directly by a QML MouseArea item.
 * Unfortunatelly, enter/leave events cannot be processed
 * directly by the @c WidgetLoader::event(QEvent *event) function.
 */
void WidgetLoader::processMouseHover(const bool containsMouse)
{
    if (containsMouse)
    {
        QEnterEvent event(QPoint(0, 0), QPoint(0, 0), QPoint(0, 0));
        processEnterEvent(&event);
    }

    else
    {
        QEvent event(QEvent::Leave);
        processLeaveEvent(&event);
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

/**
 * Updates the visibility status of the current widget (this function is called
 * automatically by the UI::Dashboard class via signals/slots).
 */
void WidgetLoader::updateWidgetVisible()
{
    bool visible = UI::Dashboard::getInstance()->widgetVisible(widgetIndex());

    if (widgetVisible() != visible && !isExternalWindow())
    {
        m_widgetVisible = visible;

        if (m_widget)
            m_widget->setEnabled(visible);

        Q_EMIT widgetVisibleChanged();
    }
}

/**
 * Lets the widget handle the mouse leave events
 */
void WidgetLoader::processLeaveEvent(QEvent *event)
{
    if (!m_widget)
        return;

    class Hack : public QWidget
    {
    public:
        using QWidget::leaveEvent;
    };

    auto hack = static_cast<Hack *>(widget());
    hack->leaveEvent(event);
    update();
}

/**
 * Lets the widget handle the mouse enter events
 */
void WidgetLoader::processEnterEvent(QEnterEvent *event)
{
    if (!m_widget)
        return;

    class Hack : public QWidget
    {
    public:
        using QWidget::enterEvent;
    };

    auto hack = static_cast<Hack *>(widget());
    hack->enterEvent(event);
    update();
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

    auto hack = static_cast<Hack *>(widget());
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

    update();
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

    static_cast<Hack *>(widget())->wheelEvent(event);
    update();
}
}
