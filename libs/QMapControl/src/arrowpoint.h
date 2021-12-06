/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2010 Jeffery MacEachern
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

#ifndef ARROWPOINT_H
#define ARROWPOINT_H

#include <QBrush>

#include "qmapcontrol_global.h"
#include "math.h"
#include "point.h"

namespace qmapcontrol
{
    //! Draws a directed arrow (showing orientation) into the map
    /*! This is a convenience class for Point.
     * It configures the pixmap of a Point to draw an arrow in a specific direction.
     * A QPen could be used to change the color or line-width of the arrow
     *
     * @author Jeffery MacEachern <j.maceachern@gmail.com>
     */
    class QMAPCONTROL_EXPORT ArrowPoint : public Point
    {
    public:
        //!
        /*!
         *
         * @param x longitude
         * @param y latitude
         * @param sideLength side length of the arrow's bounding box (square)
         * @param heading compass heading determining direction that arrow faces, measured in degrees clockwise from North
         * @param name name of the arrow point
         * @param alignment alignment (Middle or TopLeft)
         * @param pen QPen for drawing
         */
        ArrowPoint(qreal x, qreal y, int sideLength, qreal heading, QString name = QString(), Alignment alignment = Middle, QPen* pen=0);
        virtual ~ArrowPoint();

        //! sets the QPen which is used for drawing the arrow
        /*!
         * A QPen can be used to modify the look of the drawn arrow
         * @param pen the QPen which should be used for drawing
         * @see http://doc.trolltech.com/4.3/qpen.html
         */
        virtual void setPen(QPen* pen);

        //! sets the heading of the arrow and redraws it in the new orientation
        /*!
         * @param heading new heading
         */
        void setHeading(qreal heading);

        //! gets the current heading of the arrow
        qreal getHeading() const;
    private:
        void drawArrow();

        // Heading
        qreal h;

        // Brush to fill the arrow with - solid colour, same as pen
        QBrush mybrush;
    };
}
#endif
