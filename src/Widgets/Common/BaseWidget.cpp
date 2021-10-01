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

#include "BaseWidget.h"
#include "UI/Dashboard.h"
#include "Misc/ThemeManager.h"

#include <QResizeEvent>

using namespace Widgets;

BaseWidget::BaseWidget()
    : m_widget(nullptr)
    , m_resizeWidget(true)
{
    // Set window palette
    QPalette palette;
    auto theme = Misc::ThemeManager::getInstance();
    palette.setColor(QPalette::Base, theme->datasetWindowBackground());
    palette.setColor(QPalette::Window, theme->datasetWindowBackground());
    setPalette(palette);

    // Configure label style
    m_label.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Set stylesheets
    // clang-format off
    auto valueQSS = QSS("background-color:%1; color:%2; border:1px solid %3;",
                        theme->base(),
                        theme->widgetForegroundPrimary(),
                        theme->widgetIndicator1());
    m_label.setStyleSheet(valueQSS);
    // clang-format on
}

void BaseWidget::setValue(const QString &label)
{
    // Change label text
    if (m_label.text() != label)
    {
        m_label.setText(label);

        // Resize label font (so it fits inside the box)
        while (QFontMetrics(m_label.font()).horizontalAdvance(label) + 24
               > m_label.width())
        {
            QFont font = m_label.font();
            font.setPixelSize(font.pixelSize() - 1);
            m_label.setFont(font);
        }
    }
}

void BaseWidget::setWidget(QWidget *widget, Qt::Alignment alignment, bool autoresize)
{
    Q_ASSERT(widget != nullptr);

    if (m_widget == nullptr)
    {
        m_widget = widget;
        m_resizeWidget = autoresize;

        m_layout.setSpacing(24);
        m_layout.addWidget(m_widget);
        m_layout.addWidget(&m_label);
        m_layout.setAlignment(m_widget, alignment);
        m_layout.setContentsMargins(24, 24, 24, 24);
        setLayout(&m_layout);
    }
}

void BaseWidget::resizeEvent(QResizeEvent *event)
{
    // Get width & height (exluding layout margins & spacing)
    auto width = event->size().width() - 72;
    auto height = event->size().height() - 48;

    // Get fonts & calculate size
    auto labelFont = UI::Dashboard::getInstance()->monoFont();
    auto gaugeFont = UI::Dashboard::getInstance()->monoFont();
    labelFont.setPixelSize(width / 18);
    gaugeFont.setPixelSize(width / 24);

    // Set label font (so it fits inside the box)
    m_label.setFont(labelFont);
    while (QFontMetrics(m_label.font()).horizontalAdvance(m_label.text()) + 12
           > m_label.width())
    {
        QFont font = m_label.font();
        font.setPixelSize(font.pixelSize() - 1);
        m_label.setFont(font);
    }

    // Set widget font
    if (m_widget)
        m_widget->setFont(gaugeFont);

    // Set widget sizes
    m_label.setMinimumWidth(width * 0.4);
    m_label.setMaximumWidth(width * 0.4);
    m_label.setMaximumHeight(height * 0.4);

    // Set widget size
    if (m_resizeWidget && m_widget)
    {
        m_widget->setMinimumWidth(width * 0.6);
        m_widget->setMaximumWidth(width * 0.6);
    }

    // Accept event
    event->accept();

    // Emit resize signal
    emit resized();
}
