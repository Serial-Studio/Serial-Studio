/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#include <googlemapadapter.h>

#include <UI/Dashboard.h>
#include <UI/Widgets/GPS.h>
#include <Misc/ThemeManager.h>

/**
 * Generates the user interface elements & layout
 */
Widgets::GPS::GPS(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = &UI::Dashboard::instance();
    auto theme = &Misc::ThemeManager::instance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->gpsCount())
        return;

    // Set window palette
    QPalette windowPalette;
    windowPalette.setColor(QPalette::Base, theme->widgetWindowBackground());
    windowPalette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(windowPalette);

    // Initialize title widget children
    m_label = new QLabel(&m_titleWidget);
    m_zoomIn = new QPushButton(&m_titleWidget);
    m_zoomOut = new QPushButton(&m_titleWidget);

    // Configure fonts
    QFont font("Roboto Mono", 12);
    m_label->setFont(font);
    m_zoomIn->setFont(font);
    m_zoomOut->setFont(font);

    // Configure zoom buttons
    m_zoomIn->setText("+");
    m_zoomOut->setText("-");
    m_zoomIn->setMinimumWidth(m_zoomIn->height());
    m_zoomIn->setMaximumWidth(m_zoomIn->height());
    m_zoomOut->setMinimumWidth(m_zoomOut->height());
    m_zoomOut->setMaximumWidth(m_zoomOut->height());
    connect(m_zoomIn, SIGNAL(clicked()), &m_mapControl, SLOT(zoomIn()));
    connect(m_zoomOut, SIGNAL(clicked()), &m_mapControl, SLOT(zoomOut()));

    // Configure title widget
    m_titleWidget.setMinimumHeight(32);
    m_titleWidget.setMaximumHeight(32);
    m_titleLayout.setContentsMargins(0, 0, 0, 0);
    m_titleLayout.addWidget(m_label, 1, Qt::AlignVCenter);
    m_titleLayout.addWidget(m_zoomOut, 0, Qt::AlignVCenter);
    m_titleLayout.addWidget(m_zoomIn, 0, Qt::AlignVCenter);
    m_titleWidget.setLayout(&m_titleLayout);

    // Configure map control
    m_mapControl.showScale(true);
    m_mapControl.setFrameShadow(QFrame::Plain);
    m_mapControl.setFrameShape(QFrame::NoFrame);
    m_mapControl.setMouseMode(qmapcontrol::MapControl::None);
    m_mapControl.resize(QSize(width() - 28, height() - 36 - m_titleWidget.height()));

    // Load google maps adapter
    auto *mapadapter = new qmapcontrol::GoogleMapAdapter();
    auto *l = new qmapcontrol::Layer("Custom Layer", mapadapter,
                                     qmapcontrol::Layer::MapLayer);
    m_mapControl.addLayer(l);
    m_mapControl.setZoom(14);

    // Configure layout
    m_layout.setSpacing(8);
    m_layout.addWidget(&m_titleWidget, 0);
    m_layout.addWidget(&m_mapControl, 1);
    m_layout.setContentsMargins(12, 12, 12, 12);
    setLayout(&m_layout);

    // React to Qt signals
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()), Qt::QueuedConnection);
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Widgets::GPS::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Invalid index, abort update
    auto dash = &UI::Dashboard::instance();
    if (m_index < 0 || m_index >= dash->gpsCount())
        return;

    // Get group reference
    auto group = dash->getGPS(m_index);

    // Get latitiude/longitude from datasets
    qreal lat = -1;
    qreal lon = -1;
    qreal alt = -1;
    for (int i = 0; i < group.datasetCount(); ++i)
    {
        auto dataset = group.getDataset(i);
        if (dataset.widget() == "lat")
            lat = dataset.value().toDouble();
        else if (dataset.widget() == "lon")
            lon = dataset.value().toDouble();
        else if (dataset.widget() == "alt")
            alt = dataset.value().toDouble();
    }

    // Update map position
    m_mapControl.setView(QPointF(lon, lat));

    // Update map title
    auto latstr = QString::number(lat, 'f', dash->precision());
    auto lonstr = QString::number(lon, 'f', dash->precision());
    auto altstr = QString::number(alt, 'f', dash->precision());

    // clang-format off
    m_label->setText(QString("<u>POS:</u><i> %1,%2</i>&nbsp;<u>ALT:</u><i> %3 m</i>")
                     .arg(latstr, lonstr, altstr));
    // clang-format on

    // Repaint widget
    requestRepaint();
}

/**
 * Changes the size of the map when the widget is resized
 */
void Widgets::GPS::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    auto height = event->size().height();
    m_mapControl.resize(QSize(width - 28, height - 26 - m_titleWidget.height()));
    event->accept();
}

/**
 * Manually calls the zoom in / zoom out button clicks when the user presses on the widget
 */
void Widgets::GPS::mousePressEvent(QMouseEvent *event)
{
    // Get x and y coordinates relative to title widget
    auto x = event->x() - m_titleWidget.x();
    auto y = event->y() - m_titleWidget.y();

    // Press zoom in button
    if (x >= m_zoomIn->x() && x <= m_zoomIn->x() + m_zoomIn->width())
    {
        if (y >= m_zoomIn->y() && y <= m_zoomIn->y() + m_zoomIn->height())
            Q_EMIT m_zoomIn->clicked();
    }

    // Press zoom out button
    else if (x >= m_zoomOut->x() && x <= m_zoomOut->x() + m_zoomOut->width())
    {
        if (y >= m_zoomOut->y() && y <= m_zoomOut->y() + m_zoomOut->height())
            Q_EMIT m_zoomOut->clicked();
    }

    // Accept event
    event->accept();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_GPS.cpp"
#endif
