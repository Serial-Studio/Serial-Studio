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

#include <QtMath>
#include <QPainter>
#include <QPainterPath>
#include <QwtPointPolar>
#include <QwtDialNeedle>
#include <QwtRoundScaleDraw>

#include <UI/Widgets/Common/AttitudeIndicator.h>

namespace Widgets
{
namespace
{
class Needle : public QwtDialNeedle
{
public:
    Needle(const QColor &color)
    {
        QPalette palette;
        palette.setColor(QPalette::Text, color);
        setPalette(palette);
    }

protected:
    virtual void drawNeedle(QPainter *painter, double length,
                            QPalette::ColorGroup colorGroup) const QWT_OVERRIDE
    {
        double triangleSize = length * 0.1;
        double pos = length - 2.0;

        QPainterPath path;
        path.moveTo(pos, 0);
        path.lineTo(pos - 2 * triangleSize, triangleSize);
        path.lineTo(pos - 2 * triangleSize, -triangleSize);
        path.closeSubpath();

        painter->setBrush(palette().brush(colorGroup, QPalette::Text));
        painter->drawPath(path);

        double l = length - 2;
        painter->setPen(QPen(palette().color(colorGroup, QPalette::Text), 3));
        painter->drawLine(QPointF(0.0, -l), QPointF(0.0, l));
    }
};
}

AttitudeIndicator::AttitudeIndicator(QWidget *parent)
    : QwtDial(parent)
    , m_gradient(0)
{
    QwtRoundScaleDraw *scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    scaleDraw->enableComponent(QwtAbstractScaleDraw::Labels, false);
    setScaleDraw(scaleDraw);

    setMode(RotateScale);
    setWrapping(true);

    setOrigin(270.0);

    setScaleMaxMinor(0);
    setScaleStepSize(30.0);
    setScale(0.0, 360.0);

    const QColor color = palette().color(QPalette::Text);
    setNeedle(new Needle(color));
}

double AttitudeIndicator::angle() const
{
    return value();
}

double AttitudeIndicator::gradient() const
{
    return m_gradient;
}

void AttitudeIndicator::setAngle(const double &angle)
{
    setValue(angle);
}

void AttitudeIndicator::setGradient(const double &gradient)
{
    auto grad = qMin(1.0, qMax(gradient, -1.0));

    if (m_gradient != grad)
    {
        m_gradient = grad;
        update();
    }
}

void AttitudeIndicator::drawScale(QPainter *painter, const QPointF &center,
                                  double radius) const
{
    const double offset = 4.0;

    const QPointF p0 = qwtPolar2Pos(center, offset, 1.5 * M_PI);

    const double w = innerRect().width();

    QPainterPath path;
    path.moveTo(qwtPolar2Pos(p0, w, 0.0));
    path.lineTo(qwtPolar2Pos(path.currentPosition(), 2 * w, M_PI));
    path.lineTo(qwtPolar2Pos(path.currentPosition(), w, 0.5 * M_PI));
    path.lineTo(qwtPolar2Pos(path.currentPosition(), w, 0.0));

    painter->save();
    painter->setClipPath(path);

    QwtDial::drawScale(painter, center, radius);

    painter->restore();
}

void AttitudeIndicator::drawScaleContents(QPainter *painter, const QPointF &center,
                                          double radius) const
{
    (void)center;
    (void)radius;

    int dir = 360 - qRound(origin() - value());
    int arc = 90 + qRound(gradient() * 90);

    const QColor skyColor(38, 151, 221);

    painter->save();
    painter->setBrush(skyColor);
    painter->drawChord(scaleInnerRect(), (dir - arc) * 16, 2 * arc * 16);
    painter->restore();
}
}
