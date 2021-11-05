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

#include "LEDPanel.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>

namespace Widgets
{

/**
 * Generates the user interface elements & layout
 */
LEDPanel::LEDPanel(const int index)
    : m_index(index)
{
    // Get pointers to serial studio modules
    auto dash = UI::Dashboard::getInstance();
    auto theme = Misc::ThemeManager::getInstance();

    // Invalid index, abort initialization
    if (m_index < 0 || m_index >= dash->ledCount())
        return;

    // Get group pointer
    auto group = dash->getLED(m_index);
    if (!group)
        return;

    // Generate widget stylesheets
    auto titleQSS = QSS("color:%1", theme->widgetTextPrimary());

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
    m_gridLayout->setSpacing(16);
    for (int dataset = 0; dataset < group->datasetCount(); ++dataset)
    {
        // Create labels
        m_leds.append(new KLed(m_dataContainer));
        m_titles.append(new QLabel(m_dataContainer));

        // Get pointers to labels
        auto led = m_leds.last();
        auto title = m_titles.last();

        // Set label styles & fonts
        title->setStyleSheet(titleQSS);
        title->setFont(dash->monoFont());
        title->setText(group->getDataset(dataset)->title());

        // Set LED color & style
        led->setLook(KLed::Sunken);
        led->setShape(KLed::Circular);
        led->setColor(theme->ledEnabled());

        // Calculate column and row
        int column = 0;
        int row = dataset;
        int count = dataset + 1;
        while (count > 3)
        {
            count -= 3;
            row -= 3;
            column += 2;
        }

        // Add label and LED to grid layout
        m_gridLayout->addWidget(led, row, column);
        m_gridLayout->addWidget(title, row, column + 1);
        m_gridLayout->setAlignment(led, Qt::AlignRight | Qt::AlignVCenter);
        m_gridLayout->setAlignment(title, Qt::AlignLeft | Qt::AlignVCenter);
    }

    // Load layout into container widget
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

    // React to dashboard events
    connect(dash, SIGNAL(updated()), this, SLOT(updateData()));
}

/**
 * Frees the memory allocated for each label and LED that represents a dataset
 */
LEDPanel::~LEDPanel()
{
    foreach (auto led, m_leds)
        delete led;

    foreach (auto title, m_titles)
        delete title;

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
void LEDPanel::updateData()
{
    // Widget not enabled, do nothing
    if (!isEnabled())
        return;

    // Get group pointer
    auto dash = UI::Dashboard::getInstance();
    auto group = dash->getLED(m_index);
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
#endif
            // Get dataset value (we compare with 0.1 for low voltages)
            auto value = dataset->value().toDouble();
            if (qAbs(value) < 0.10)
                m_leds.at(i)->off();
            else
                m_leds.at(i)->on();
        }
    }
}

/**
 * Changes the size of the labels when the widget is resized
 */
void LEDPanel::resizeEvent(QResizeEvent *event)
{
    auto width = event->size().width();
    QFont font = UI::Dashboard::getInstance()->monoFont();
    font.setPixelSize(qMax(8, width / 24));
    auto fHeight = QFontMetrics(font).height() * 1.5;

    for (int i = 0; i < m_titles.count(); ++i)
    {
        m_titles.at(i)->setFont(font);
        m_leds.at(i)->setMinimumSize(fHeight, fHeight);
    }

    event->accept();
}

void LEDPanel::wheelEvent(QWheelEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::wheelEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->wheelEvent(event);
}

void LEDPanel::mouseMoveEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mouseMoveEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mouseMoveEvent(event);
}

void LEDPanel::mousePressEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mousePressEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mousePressEvent(event);
}

void LEDPanel::mouseReleaseEvent(QMouseEvent *event)
{
    class Hack : public QScrollArea
    {
    public:
        using QWidget::mouseReleaseEvent;
    };

    auto hack = static_cast<Hack *>(m_scrollArea);
    hack->mouseReleaseEvent(event);
}

void LEDPanel::mouseDoubleClickEvent(QMouseEvent *event)
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

#if SERIAL_STUDIO_MOC_INCLUDE
#    include "moc_LEDPanel.cpp"
#endif
