/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2014 Frederic Bourgeois
* Based on CirclePoint code by Kai Winter
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

#include "invisiblepoint.h"
namespace qmapcontrol
{
    InvisiblePoint::InvisiblePoint(qreal x, qreal y, int sideLength, QString name)
            : Point(x, y, name, Point::Middle)
    {
        size = QSize(sideLength, sideLength);
    }

    InvisiblePoint::InvisiblePoint(qreal x, qreal y, int width, int height, QString name)
            : Point(x, y, name, Point::Middle)
    {
        size = QSize(width, height);
    }

    InvisiblePoint::InvisiblePoint(qreal x, qreal y, QString name)
            : Point(x, y, name, Point::Middle)
    {
        int sideLength = 10;
        size = QSize(sideLength, sideLength);
    }

    InvisiblePoint::~InvisiblePoint()
    {
    }
}
