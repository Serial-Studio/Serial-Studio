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

#ifndef INVISIBLEPOINT_H
#define INVISIBLEPOINT_H

#include "qmapcontrol_global.h"
#include "point.h"

namespace qmapcontrol
{
    //! Draws an invisible point into the map
    /*! This is a convenience class for point.
     * It configures the pixmap of a point to draw nothing,
     * still being a clickable point.
     *
     * @author Frederic Bourgeois <bourgeoislab@gmail.com>
     */
    class QMAPCONTROL_EXPORT InvisiblePoint : public Point
    {
    public:
        //!
        /*!
         *
         * @param x longitude
         * @param y latitude
         * @param name name of the invisible point
         */
        InvisiblePoint(qreal x, qreal y, QString name = QString());

        //!
        /*!
         *
         * @param x longitude
         * @param y latitude
         * @param width width
         * @param height height
         * @param name name of the invisible point
         */
        InvisiblePoint(qreal x, qreal y, int width = 10, int height = 10, QString name = QString());

        //!
        /*!
         *
         * @param x longitude
         * @param y latitude
         * @param sideLength side length of the bounding box (square)
         * @param name name of the invisible point
         */
        InvisiblePoint(qreal x, qreal y, int sideLength = 10, QString name = QString());
        virtual ~InvisiblePoint();
    };
}
#endif
