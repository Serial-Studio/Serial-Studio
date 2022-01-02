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

#include <QTimer>
#include <QResizeEvent>
#include <UI/Widgets/Common/ElidedLabel.h>

Widgets::ElidedLabel::ElidedLabel(QWidget *parent, Qt::WindowFlags flags)
    : QLabel(parent, flags)
    , m_eliding(false)
    , m_originalText("")
    , m_elideMode(Qt::ElideNone)
{
}

Widgets::ElidedLabel::ElidedLabel(const QString &text, QWidget *parent,
                                  Qt::WindowFlags flags)
    : QLabel(text, parent, flags)
    , m_eliding(false)
    , m_originalText("")
    , m_elideMode(Qt::ElideNone)
{
    setText(text);
}

void Widgets::ElidedLabel::setType(const Qt::TextElideMode type)
{
    m_elideMode = type;
    elide();
}

void Widgets::ElidedLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QTimer::singleShot(50, this, SLOT(elide()));
}

void Widgets::ElidedLabel::setText(const QString &text)
{
    m_originalText = text;
    QLabel::setText(text);
    elide();
}

void Widgets::ElidedLabel::elide()
{
    if (m_eliding == false)
    {
        m_eliding = true;

        QFontMetrics metrics(font());
        QLabel::setText(metrics.elidedText(m_originalText, m_elideMode, width()));

        m_eliding = false;
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_ElidedLabel.cpp"
#endif
