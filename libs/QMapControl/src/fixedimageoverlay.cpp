/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2007 - 2009 Kai Winter
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

#include "fixedimageoverlay.h"
namespace qmapcontrol
{
    FixedImageOverlay::FixedImageOverlay(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QString filename, QString name)
            : ImagePoint(x_upperleft, y_upperleft, filename, name, TopLeft),
            x_lowerright(x_lowerright), y_lowerright(y_lowerright)
    {
        //qDebug() << "loading image: " << filename;
        mypixmap = QPixmap(filename);
        size = mypixmap.size();
        //qDebug() << "image size: " << size;
    }

    FixedImageOverlay::FixedImageOverlay(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QPixmap pixmap, QString name)
            : ImagePoint(x_upperleft, y_upperleft, pixmap, name, TopLeft),
            x_lowerright(x_lowerright), y_lowerright(y_lowerright)
    {
        mypixmap = pixmap;
        size = mypixmap.size();
    }

    void FixedImageOverlay::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &, const QPoint)
    {
        if (!visible)
            return;

            const QPointF c = QPointF(X, Y);
            QPoint topleft = mapadapter->coordinateToDisplay(c);

            const QPointF c2 = QPointF(x_lowerright, y_lowerright);
            QPoint lowerright = mapadapter->coordinateToDisplay(c2);

        painter->drawPixmap(topleft.x(), topleft.y(), lowerright.x()-topleft.x(), lowerright.y()-topleft.y(), mypixmap);


    }

    FixedImageOverlay::~FixedImageOverlay()
    {
    }
}
