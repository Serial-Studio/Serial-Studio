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

#include "GPS.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QUrlQuery>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QNetworkReply>
#include <QDesktopServices>

using namespace Widgets;

#define APP_ID QStringLiteral("36496bad1955bf3365448965a42b9eac")
#define G_MAPS QStringLiteral("https://www.google.com/maps/search/?api=1&query=%1,%2")

/**
 * Converts the given latitude/longitude to degrees-minutes-seconds format
 */
static const QString DD_TO_DMS(const qreal dd, const bool horizontal)
{
    // Calculate degrees
    auto val = qAbs(dd);
    auto deg = static_cast<int>(val);

    // Calculate minutes
    val = (val - deg) * 60;
    auto min = static_cast<int>(val);

    // Calculate seconds
    auto sec = (val - min) * 60;

    // clang-format off
    auto dms = QString("%1°%2'%3\"").arg(QString::number(deg),
                                         QString::number(min),
                                         QString::number(sec, 'f', 2));
    // clang-format on

    // Add heading
    if (horizontal)
    {
        if (deg < 0)
            dms.append("W ");
        else
            dms.append("E ");
    }
    else
    {
        if (deg < 0)
            dms.append("S ");
        else
            dms.append("N ");
    }

    // Return constructed string
    return dms;
}

/**
 * Generates the user interface elements & layout
 */
GPS::GPS(const int index)
    : m_index(index)
    , m_lat(0)
    , m_lon(0)
    , m_alt(0)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->gpsCount())
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
    for (int i = 0; i < 3; ++i)
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
        units->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
        units->setText("[DMS]");
        value->setText("--.--");

        // Add labels to grid layout
        m_gridLayout->addWidget(title, i, 0);
        m_gridLayout->addWidget(dicon, i, 1);
        m_gridLayout->addWidget(value, i, 2);
        m_gridLayout->addWidget(units, i, 3);

        // Set latitude values
        if (i == 0)
            title->setText(tr("Latitude"));

        // Set longitude values
        else if (i == 1)
            title->setText(tr("Longitude"));

        // Set altitude values
        else if (i == 2)
        {
            units->setText("[m]");
            title->setText(tr("Altitude"));
        }
    }

    // Load layout into container widget
    m_gridLayout->setColumnStretch(0, 2);
    m_gridLayout->setColumnStretch(1, 1);
    m_gridLayout->setColumnStretch(2, 2);
    m_gridLayout->setColumnStretch(3, 0);
    m_dataContainer->setLayout(m_gridLayout);

    // Configure position label
    m_posLabel.setStyleSheet(titleQSS);
    m_posLabel.setText(tr("Loading..."));

    // Configure map labeL
    m_mapLabel.setStyleSheet(labelQSS);
    m_mapLabel.setText(tr("Double-click to open map"));

    // Load grid layout into main layout
    m_layout.addWidget(m_dataContainer);
    m_layout.addWidget(new QWidget(this));
    m_layout.addWidget(&m_posLabel);
    m_layout.addWidget(&m_mapLabel);
    m_layout.addWidget(new QWidget(this));
    m_layout.setStretch(0, 0);
    m_layout.setStretch(1, 1);
    m_layout.setStretch(2, 0);
    m_layout.setStretch(3, 0);
    m_layout.setStretch(4, 1);
    m_layout.setAlignment(&m_posLabel, Qt::AlignHCenter | Qt::AlignVCenter);
    m_layout.setAlignment(&m_mapLabel, Qt::AlignHCenter | Qt::AlignVCenter);
    m_layout.setContentsMargins(12, 0, 0, 12);
    setLayout(&m_layout);

    // Invalidate network throttle timer
    m_networkThrottle.invalidate();

    // React to Qt signals
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Frees the memory allocated for each label that represents a dataset
 */
GPS::~GPS()
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
void GPS::openMap()
{
    QString lat = QString::number(m_lat, 'f', 6);
    QString lon = QString::number(m_lon, 'f', 6);
    QDesktopServices::openUrl(QUrl(QString(G_MAPS).arg(lat, lon)));
}

/**
 * Gets GEO information about the current latitude/longitude
 */
void GPS::queryCity()
{
    // Keep server load low
    if (m_networkThrottle.isValid() && m_networkThrottle.elapsed() < 5 * 1000)
        return;

    // Restart throttle timer
    m_networkThrottle.start();

    // Construct GEO information request
    QUrl url("http://api.openweathermap.org/data/2.5/weather");
    QUrlQuery query;
    query.addQueryItem("lat", QString::number(m_lat, 'f', 6));
    query.addQueryItem("lon", QString::number(m_lon, 'f', 6));
    query.addQueryItem("mode", "json");
    query.addQueryItem("APPID", APP_ID);
    url.setQuery(query);

    // Ask the server for current GEO information
    QNetworkReply *reply = m_manager.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { processGeoResponse(reply); });
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void GPS::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    auto dash = UI::Dashboard::getInstance();
    auto group = dash->getGPS(m_index);
    if (!group)
        return;

    // Get latitiude/longitude from datasets
    qreal lat = -1;
    qreal lon = -1;
    qreal alt = -1;
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        auto dataset = group->getDataset(i);
        if (dataset)
        {
            if (dataset->widget() == "lat")
                lat = dataset->value().toDouble();
            else if (dataset->widget() == "lon")
                lon = dataset->value().toDouble();
            else if (dataset->widget() == "alt")
                alt = dataset->value().toDouble();
        }
    }

    // Update latitude/longitude labels
    if (m_lat != lat || m_lon != lon || m_alt != alt)
    {
        // Update latitude/longitude labels
        m_lat = lat;
        m_lon = lon;
        m_alt = alt;
        auto latLabel = m_values[0];
        auto lonLabel = m_values[1];
        auto altLabel = m_values[2];
        lonLabel->setText(DD_TO_DMS(lon, true));
        latLabel->setText(DD_TO_DMS(lat, false));
        altLabel->setText(QString::number(alt, 'f', 4) + " ");

        // Ensure that non-received data is displayed as "--.--"
        if (lat == -1)
            latLabel->setText("--.--");
        if (lon == -1)
            lonLabel->setText("--.--");
        if (alt == -1)
            altLabel->setText("--.--");

        // Get city
        queryCity();
    }
}

/**
 * Reads JSON frame with information about the current latitude/longitude
 */
void GPS::processGeoResponse(QNetworkReply *reply)
{
    // Invalid reply handle
    if (!reply)
        return;

    // Start throttle timer
    if (!m_networkThrottle.isValid())
        m_networkThrottle.start();

    // No error, read JSON document
    if (!reply->error())
    {
        QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        QJsonObject object = document.object();
        auto name = object.value("name").toString();
        auto country = object.value("sys").toObject().value("country").toString();
        m_posLabel.setText(name + ", " + country);
    }

    // Delete the reply object
    reply->deleteLater();
}

/**
 * Removes the link effect from the label when the mouse leaves the widget
 */
void GPS::leaveEvent(QEvent *event)
{
    event->accept();

    auto theme = Misc::ThemeManager::getInstance();
    auto qss = QSS("color:%1", theme->placeholderText());
    m_mapLabel.setStyleSheet(qss);
}

/**
 * Adds the link effect from the label when the mouse enters the widget
 */
void GPS::enterEvent(QEnterEvent *event)
{
    event->accept();

    // clang-format off
    auto theme = Misc::ThemeManager::getInstance();
    auto qss = QSS("color:%1; "
                   "text-decoration:underline;",
                   theme->widgetForegroundPrimary());
    m_mapLabel.setStyleSheet(qss);
    // clang-format on
}

/**
 * Changes the size of the labels when the widget is resized
 */
void GPS::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    QFont font = UI::Dashboard::getInstance()->monoFont();
    QFont icon = font;
    QFont labelFont = font;
    icon.setPixelSize(qMax(8, width / 16));
    font.setPixelSize(qMax(8, width / 24));
    labelFont.setPixelSize(font.pixelSize() * 1.5);
    m_mapLabel.setFont(font);
    m_posLabel.setFont(labelFont);

    for (int i = 0; i < m_titles.count(); ++i)
    {
        m_icons.at(i)->setFont(icon);
        m_units.at(i)->setFont(font);
        m_titles.at(i)->setFont(font);
        m_values.at(i)->setFont(font);
    }

    event->accept();
}

void GPS::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void GPS::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void GPS::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void GPS::mouseDoubleClickEvent(QMouseEvent *event)
{
    openMap();
    event->accept();
}
