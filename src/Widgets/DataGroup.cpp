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

#include <QResizeEvent>

using namespace Widgets;

/**
 * Generates the user interface elements & layout
 */
DataGroup::DataGroup(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->groupCount())
        return;

    // Get group pointer
    auto group = dash->getGroup(m_index);
    if (!group)
        return;

    // Generate widget stylesheets
    auto font = dash->monoFont();
    font.setBold(true);
    auto valueQSS = QSS("color:%1", theme->datasetValue());
    auto titleQSS = QSS("color:%1", theme->datasetTextPrimary());
    auto unitsQSS = QSS("color:%1", theme->datasetTextSecondary());
    auto iconsQSS = QSS("color:%1; font-weight:600;", theme->datasetTextSecondary());
    auto windwQSS = QSS("background-color:%1", theme->datasetWindowBackground());

    // Configure scroll area container
    m_dataContainer = new QWidget(this);

    // Configure grid layout
    m_gridLayout = new QGridLayout(m_dataContainer);
    for (int dataset = 0; dataset < group->datasetCount(); ++dataset)
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
        value->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        dicon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        // Set label styles
        title->setFont(dash->monoFont());
        value->setFont(dash->monoFont());
        units->setFont(dash->monoFont());
        title->setStyleSheet(titleQSS);
        value->setStyleSheet(valueQSS);
        units->setStyleSheet(unitsQSS);
        dicon->setStyleSheet(iconsQSS);

        // Configure elide modes

        // Set label data
        auto set = group->getDataset(dataset);
        title->setText(set->title());
        value->setText(set->value());
        if (!set->units().isEmpty())
            units->setText(QString("[%1]").arg(set->units()));

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
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_mainLayout);

    // Set background color
    m_dataContainer->setStyleSheet(windwQSS);

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(update()));
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

    delete m_gridLayout;
    delete m_scrollArea;
    delete m_mainLayout;
}

/**
 * Updates the widget's data
 */
void DataGroup::update()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    auto dash = UI::Dashboard::getInstance();
    auto group = dash->getGroup(m_index);
    if (!group)
        return;

    // Update labels
    JSON::Dataset *set;
    for (int i = 0; i < group->datasetCount(); ++i)
    {
        set = group->getDataset(i);
        m_titles.at(i)->setText(set->title());
        m_values.at(i)->setText(set->value());
        if (!set->units().isEmpty())
            m_units.at(i)->setText(QString("[%1]").arg(set->units()));
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
    icon.setPixelSize(width / 16);
    font.setPixelSize(width / 24);

    for (int i = 0; i < m_titles.count(); ++i)
    {
        m_units.at(i)->setFont(font);
        m_icons.at(i)->setFont(icon);
        m_titles.at(i)->setFont(font);
        m_values.at(i)->setFont(font);
    }

    event->accept();
}
