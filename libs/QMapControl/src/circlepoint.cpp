/*
 *
 * This file is part of QMapControl,
 * an open-source cross-platform map widget
 *
 * Copyright (C) 2007 - 2008 Kai Winter
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QMapControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact e-mail: kaiwinter@gmx.de
 * Program URL   : http://qmapcontrol.sourceforge.net/
 *
 */

#include "circlepoint.h"
namespace qmapcontrol
{
CirclePoint::CirclePoint(qreal x, qreal y, int radius, QString name, Alignment alignment,
                         QPen *pen)
    : Point(x, y, name, alignment)
{
    size = QSize(radius, radius);
    mypen = pen;
    mypixmap = QPixmap(radius + 1, radius + 1);
    drawCircle();
}

CirclePoint::CirclePoint(qreal x, qreal y, QString name, Alignment alignment, QPen *pen)
    : Point(x, y, name, alignment)
{
    int radius = 10;
    size = QSize(radius, radius);
    mypen = pen;
    mypixmap = QPixmap(radius + 1, radius + 1);
    drawCircle();
}

CirclePoint::~CirclePoint() { }

void CirclePoint::setPen(QPen *pen)
{
    mypen = pen;
    drawCircle();
}

void CirclePoint::drawCircle()
{
    mypixmap.fill(Qt::transparent);
    QPainter painter(&mypixmap);
    //#if !defined Q_WS_MAEMO_5 //FIXME Maemo has a bug - it will antialias our point out
    //of existence
    painter.setRenderHints(QPainter::Antialiasing);
    //#endif
    if (mypen != 0)
    {
        painter.setPen(*mypen);
    }
    painter.drawEllipse(0, 0, size.width(), size.height());
}
}
