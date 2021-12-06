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

#include "DataGroup.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QRegularExpression>

static QRegularExpression REGEXP("^[+-]?(\\d*\\.)?\\d+$");

namespace Widgets
{

/**
 * Generates the user interface elements & layout
 */
DataGroup::DataGroup(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    const auto dash = UI::Dashboard::getInstance();
    const auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->groupCount())
        return;

    // Get group pointer
    const auto group = dash->getGroups(m_index);
    if (!group)
        return;

    // Generate widget stylesheets
    const auto titleQSS = QSS("color:%1", theme->widgetTextPrimary());
    const auto unitsQSS = QSS("color:%1", theme->widgetTextSecondary());
    const auto valueQSS = QSS("color:%1", theme->widgetForegroundPrimary());
    const auto iconsQSS = QSS("color:%1; font-weight:600;", theme->widgetTextSecondary());

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
    for (int dataset = 0; dataset < group->datasetCount(); ++dataset)
    {
        // Create labels
        m_units.append(new QLabel(m_dataContainer));
        m_icons.append(new QLabel(m_dataContainer));
        m_titles.append(new ElidedLabel(m_dataContainer));
        m_values.append(new ElidedLabel(m_dataContainer));

        // Get pointers to labels
        auto dicon = m_icons.last();
        auto units = m_units.last();
        auto title = m_titles.last();
        auto value = m_values.last();

        // Set elide modes for title & value fields
        title->setType(Qt::ElideRight);
        value->setType(Qt::ElideRight);

        // Set label alignments
        units->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        title->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dicon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // Set label styles & fonts
        value->setFont(valueFont);
        title->setFont(dash->monoFont());
        units->setFont(dash->monoFont());
        title->setStyleSheet(titleQSS);
        value->setStyleSheet(valueQSS);
        units->setStyleSheet(unitsQSS);
        dicon->setStyleSheet(iconsQSS);

        // Configure elide modes

        // Lazy widgets, set label initial data
#ifdef LAZY_WIDGETS
        auto set = group->getDataset(dataset);
        if (set)
        {
            title->setText(set->title());
            if (!set->units().isEmpty())
                units->setText(QString("[%1]").arg(set->units()));
        }
#endif

        // Set icon text
        dicon->setText("⤑");

        // Add labels to grid layout
        m_gridLayout->addWidget(title, dataset, 0);
        m_gridLayout->addWidget(dicon, dataset, 1);
        m_gridLayout->addWidget(value, dataset, 2);
        m_gridLayout->addWidget(units, dataset, 3);
    }

    // Load layout into container widget
    m_gridLayout->setColumnStretch(0, 2);
    m_gridLayout->setColumnStretch(1, 1);
    m_gridLayout->setColumnStretch(2, 2);
    m_gridLayout->setColumnStretch(3, 0);
    m_dataContainer->setLayout(m_gridLayout);

    // Configure scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_dataContainer);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Configure main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_scrollArea);
    m_mainLayout->setContentsMargins(12, 0, 0, 12);
    setLayout(m_mainLayout);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Frees the memory allocated for each label that represents a dataset
 */
DataGroup::~DataGroup()
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
    delete m_scrollArea;
    delete m_mainLayout;
}

/**
 * Checks if the widget is enabled, if so, the widget shall be updated
 * to display the latest data frame.
 *
 * If the widget is disabled (e.g. the user hides it, or the external
 * window is hidden), then the widget shall ignore the update request.
 */
void DataGroup::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    const auto dash = UI::Dashboard::getInstance();
    const auto group = dash->getGroups(m_index);
    if (!group)
        return;

    // Update labels
    JSON::Dataset *dataset;
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        dataset = group->getDataset(i);
        if (dataset)
        {
#ifndef LAZY_WIDGETS
            m_titles.at(i)->setText(set->title());
            if (!set->units().isEmpty())
                m_units.at(i)->setText(QString("[%1]").arg(set->units()));
#endif
            // Get dataset value
            auto value = dataset->value();

            // Check if value is a number, if so make sure that
            // we always show a fixed number of decimal places
            if (REGEXP.match(value).hasMatch())
                value = QString::number(value.toDouble(), 'f', dash->precision());

            // Update label
            m_values.at(i)->setText(value + " ");
        }
    }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void DataGroup::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    QFont font = UI::Dashboard::getInstance()->monoFont();
    QFont icon = font;
    QFont valueFont = font;
    icon.setPixelSize(qMax(8, width / 16));
    font.setPixelSize(qMax(8, width / 24));
    valueFont.setPixelSize(font.pixelSize() * 1.3);

    for (int i = 0; i < m_titles.count(); ++i)
    {
        m_units.at(i)->setFont(font);
        m_icons.at(i)->setFont(icon);
        m_titles.at(i)->setFont(font);
        m_values.at(i)->setFont(valueFont);
    }

    event->accept();
}

void DataGroup::wheelEvent(QWheelEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::wheelEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->wheelEvent(event);
}

void DataGroup::mouseMoveEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mouseMoveEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mouseMoveEvent(event);
}

void DataGroup::mousePressEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mousePressEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mousePressEvent(event);
}

void DataGroup::mouseReleaseEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mouseReleaseEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mouseReleaseEvent(event);
}

void DataGroup::mouseDoubleClickEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mouseDoubleClickEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mouseDoubleClickEvent(event);
}
}
