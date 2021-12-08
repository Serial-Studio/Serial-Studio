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

#pragma once

#include <QwtDial>
#include <QwtDialNeedle>

//
// Based on https://github.com/opencor/qwt/blob/master/examples/dials/AttitudeIndicator.h
//

namespace Widgets
{
class AttitudeIndicator : public QwtDial
{
public:
    AttitudeIndicator(QWidget *parent = Q_NULLPTR);

    double angle() const;
    double gradient() const;

public Q_SLOTS:
    void setAngle(const double &angle);
    void setGradient(const double &gradient);

protected:
    void drawScale(QPainter *painter, const QPointF &center,
                   double radius) const QWT_OVERRIDE;
    void drawScaleContents(QPainter *painter, const QPointF &center,
                           double radius) const QWT_OVERRIDE;

private:
    double m_gradient;
};
}
