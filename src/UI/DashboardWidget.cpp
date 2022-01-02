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

#include <Misc/ThemeManager.h>
#include <UI/DashboardWidget.h>

#include <UI/Widgets/Bar.h>
#include <UI/Widgets/GPS.h>
#include <UI/Widgets/Plot.h>
#include <UI/Widgets/Gauge.h>
#include <UI/Widgets/Compass.h>
#include <UI/Widgets/FFTPlot.h>
#include <UI/Widgets/LEDPanel.h>
#include <UI/Widgets/DataGroup.h>
#include <UI/Widgets/Gyroscope.h>
#include <UI/Widgets/MultiPlot.h>
#include <UI/Widgets/Accelerometer.h>

/**
 * Constructor function
 */
UI::DashboardWidget::DashboardWidget(QQuickItem *parent)
    : DeclarativeWidget(parent)
    , m_index(-1)
    , m_widgetVisible(false)
    , m_isExternalWindow(false)
{
    // clang-format off
    connect(&UI::Dashboard::instance(), &UI::Dashboard::widgetVisibilityChanged,
            this, &UI::DashboardWidget::updateWidgetVisible);
    // clang-format on
}

/**
 * Delete widget on class destruction
 */
UI::DashboardWidget::~DashboardWidget()
{
    if (m_dbWidget)
        delete m_dbWidget;
}

/**
 * Returns the global index of the widget (index of the current widget in relation to all
 * registered widgets).
 */
int UI::DashboardWidget::widgetIndex() const
{
    return m_index;
}

/**
 * Returns the relative index of the widget (e.g. index of a bar widget in relation to the
 * total number of bar widgets).
 */
int UI::DashboardWidget::relativeIndex() const
{
    return UI::Dashboard::instance().relativeIndex(widgetIndex());
}

/**
 * Returns @c true if the QML interface should display this widget.
 */
bool UI::DashboardWidget::widgetVisible() const
{
    return m_widgetVisible;
}

/**
 * Returns the path of the SVG icon to use with this widget
 */
QString UI::DashboardWidget::widgetIcon() const
{
    return UI::Dashboard::instance().widgetIcon(widgetIndex());
}

/**
 * Returns the appropiate window title for the given widget
 */
QString UI::DashboardWidget::widgetTitle() const
{
    if (widgetIndex() >= 0)
    {
        auto titles = UI::Dashboard::instance().widgetTitles();
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
bool UI::DashboardWidget::isExternalWindow() const
{
    return m_isExternalWindow;
}

/**
 * Returns the type of the current widget (e.g. group, plot, bar, gauge, etc...)
 */
UI::Dashboard::WidgetType UI::DashboardWidget::widgetType() const
{
    return UI::Dashboard::instance().widgetType(widgetIndex());
}

/**
 * Changes the visibility & enabled status of the widget
 */
void UI::DashboardWidget::setVisible(const bool visible)
{
    if (m_dbWidget)
    {
        m_dbWidget->setEnabled(visible);
        update();
    }
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

        // Delete previous widget
        if (m_dbWidget)
            delete m_dbWidget;

        // Construct new widget
        switch (widgetType())
        {
            case UI::Dashboard::WidgetType::Group:
                m_dbWidget = new Widgets::DataGroup(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::MultiPlot:
                m_dbWidget = new Widgets::MultiPlot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::FFT:
                m_dbWidget = new Widgets::FFTPlot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Plot:
                m_dbWidget = new Widgets::Plot(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Bar:
                m_dbWidget = new Widgets::Bar(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Gauge:
                m_dbWidget = new Widgets::Gauge(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Compass:
                m_dbWidget = new Widgets::Compass(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Gyroscope:
                m_dbWidget = new Widgets::Gyroscope(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::Accelerometer:
                m_dbWidget = new Widgets::Accelerometer(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::GPS:
                m_dbWidget = new Widgets::GPS(relativeIndex());
                break;
            case UI::Dashboard::WidgetType::LED:
                m_dbWidget = new Widgets::LEDPanel(relativeIndex());
                break;
            default:
                break;
        }

        // Configure widget
        if (m_dbWidget)
        {
            setWidget(m_dbWidget);
            updateWidgetVisible();
            connect(m_dbWidget, &Widgets::DashboardWidgetBase::updated,
                    [=]() { update(); });

            Q_EMIT widgetIndexChanged();
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
void UI::DashboardWidget::setIsExternalWindow(const bool isWindow)
{
    m_isExternalWindow = isWindow;
    Q_EMIT isExternalWindowChanged();
}

/**
 * Updates the visibility status of the current widget (this function is called
 * automatically by the UI::Dashboard class via signals/slots).
 */
void UI::DashboardWidget::updateWidgetVisible()
{
    bool visible = UI::Dashboard::instance().widgetVisible(widgetIndex());

    if (widgetVisible() != visible && !isExternalWindow())
    {
        m_widgetVisible = visible;

        if (m_dbWidget)
        {
            m_dbWidget->setEnabled(visible);
            update();
        }

        Q_EMIT widgetVisibleChanged();
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_DashboardWidget.cpp"
#endif
