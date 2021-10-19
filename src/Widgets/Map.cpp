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

#include "Map.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QMouseEvent>
#include <QResizeEvent>
#include <QDesktopServices>

using namespace Widgets;

/**
 * Generates the user interface elements & layout
 */
Map::Map(const int index)
    : m_index(index)
    , m_lat(0)
    , m_lon(0)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->mapCount())
        return;

    // Generate widget stylesheets
    auto labelQSS = QSS("color:%1", theme->placeholderText());
    auto unitsQSS = QSS("color:%1", theme->widgetTextSecondary());
    auto titleQSS = QSS("color:%1", theme->widgetTextPrimary());
    auto valueQSS = QSS("color:%1", theme->widgetForegroundPrimary());
    auto iconsQSS = QSS("color:%1; font-weight:600;", theme->widgetTextSecondary());

    // Set window palette
    QPalette windowPalette;
    windowPalette.setColor(QPalette::Base, theme->widgetWindowBackground());
    windowPalette.setColor(QPalette::Window, theme->widgetWindowBackground());
    setPalette(windowPalette);

    // Configure scroll area container
    m_dataContainer = new QWidget(this);

    // Make the value label larger
    auto valueFont = dash->monoFont();
    valueFont.setPixelSize(dash->monoFont().pixelSize() * 1.3);

    // Configure grid layout
    m_gridLayout = new QGridLayout(m_dataContainer);
    for (int i = 0; i < 2; ++i)
    {
        // Create labels
        m_units.append(new QLabel(m_dataContainer));
        m_icons.append(new QLabel(m_dataContainer));
        m_titles.append(new QLabel(m_dataContainer));
        m_values.append(new QLabel(m_dataContainer));

        // Get pointers to labels
        auto dicon = m_icons.last();
        auto units = m_units.last();
        auto title = m_titles.last();
        auto value = m_values.last();

        // Set label alignments
        units->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        dicon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // Set label styles & fonts
        value->setFont(valueFont);
        title->setFont(dash->monoFont());
        title->setStyleSheet(titleQSS);
        value->setStyleSheet(valueQSS);
        dicon->setStyleSheet(iconsQSS);
        units->setStyleSheet(unitsQSS);

        // Set icon & units text
        dicon->setText("⤑");
        units->setText("[°]");

        // Add labels to grid layout
        m_gridLayout->addWidget(title, i, 0);
        m_gridLayout->addWidget(dicon, i, 1);
        m_gridLayout->addWidget(value, i, 2);
        m_gridLayout->addWidget(units, i, 3);

        // Set latitude values
        if (i == 0)
        {
            title->setText(tr("Latitude"));
            value->setText(QString::number(m_lat, 'f', 4));
        }

        // Set longitude values
        else if (i == 1)
        {
            title->setText(tr("Longitude"));
            value->setText(QString::number(m_lon, 'f', 4));
        }
    }

    // Load layout into container widget
    m_gridLayout->setColumnStretch(0, 2);
    m_gridLayout->setColumnStretch(1, 1);
    m_gridLayout->setColumnStretch(2, 2);
    m_gridLayout->setColumnStretch(3, 0);
    m_dataContainer->setLayout(m_gridLayout);

    // Configure map labeL
    m_mapLabel.setStyleSheet(labelQSS);
    m_mapLabel.setText(tr("Double-click to open map"));

    // Load grid layout into main layout
    m_layout.addWidget(m_dataContainer);
    m_layout.addWidget(&m_mapLabel);
    m_layout.setStretch(0, 0);
    m_layout.setStretch(1, 1);
    m_layout.setAlignment(&m_mapLabel, Qt::AlignHCenter | Qt::AlignVCenter);
    m_layout.setContentsMargins(24, 48, 24, 24);
    setLayout(&m_layout);

    // React to Qt signals
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Frees the memory allocated for each label that represents a dataset
 */
Map::~Map()
{
    foreach (auto icon, m_icons)
        delete icon;

    foreach (auto title, m_titles)
        delete title;

    foreach (auto value, m_values)
        delete value;

    foreach (auto units, m_units)
        delete units;

    delete m_gridLayout;
}

/**
 * Opens the current latitude/longitude in Google Maps
 */
void Map::openMap()
{
    QString lat = QString::number(m_lat, 'f', 6);
    QString lon = QString::number(m_lon, 'f', 6);
    auto url
        = QString("https://www.google.com/maps/search/?api=1&query=%1,%2").arg(lat, lon);
    QDesktopServices::openUrl(QUrl(url));
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void Map::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    auto dash = UI::Dashboard::getInstance();
    auto group = dash->getMap(m_index);
    if (!group)
        return;

    // Get latitiude/longitude from datasets
    qreal lat = 0;
    qreal lon = 0;
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        auto dataset = group->getDataset(i);
        if (dataset)
        {
            if (dataset->widget() == "lat")
                lat = dataset->value().toDouble();
            else if (dataset->widget() == "lon")
                lon = dataset->value().toDouble();
        }
    }

    // Update latitude/longitude labels
    if (m_lat != lat || m_lon != lon)
    {
        m_lat = lat;
        m_lon = lon;
        auto latLabel = m_values[0];
        auto lonLabel = m_values[1];
        latLabel->setText(QString::number(lat, 'f', 4));
        lonLabel->setText(QString::number(lon, 'f', 4));
    }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void Map::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    QFont font = UI::Dashboard::getInstance()->monoFont();
    QFont icon = font;
    QFont valueFont = font;
    QFont labelFont = font;
    icon.setPixelSize(qMax(8, width / 16));
    font.setPixelSize(qMax(8, width / 24));
    valueFont.setPixelSize(font.pixelSize() * 1.3);
    labelFont.setPixelSize(font.pixelSize() * 1.1);
    m_mapLabel.setFont(labelFont);

    for (int i = 0; i < m_titles.count(); ++i)
    {
        m_icons.at(i)->setFont(icon);
        m_units.at(i)->setFont(font);
        m_titles.at(i)->setFont(font);
        m_values.at(i)->setFont(valueFont);
    }

    event->accept();
}

void Map::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();

    auto theme = Misc::ThemeManager::getInstance();
    auto qss = QSS("color:%1", theme->placeholderText());
    m_mapLabel.setStyleSheet(qss);
}

void Map::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    auto theme = Misc::ThemeManager::getInstance();
    auto qss = QSS("color:%1", theme->widgetForegroundPrimary());
    m_mapLabel.setStyleSheet(qss);
}

void Map::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    auto theme = Misc::ThemeManager::getInstance();
    auto qss = QSS("color:%1", theme->placeholderText());
    m_mapLabel.setStyleSheet(qss);
}

void Map::mouseDoubleClickEvent(QMouseEvent *event)
{
    openMap();
    event->accept();
}
