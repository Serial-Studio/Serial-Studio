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

#ifndef IMAGEPOINT_H
#define IMAGEPOINT_H

#include "qmapcontrol_global.h"
#include "point.h"

namespace qmapcontrol
{

    //! Draws an image into the map
    /*! This is a convenience class for Point.
     * It configures the pixmap of a Point to draw the given image.
     * The image will be loaded from the given path and written in the points pixmap.
     *
     * @author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT ImagePoint : public Point
    {
    public:
        //! Creates a point which loads and displays the given image file
        /*!
         * Use this contructor to load the given image file and let the point display it.
         * When you want multiple points to display the same image, use the other contructor and pass a pointer to that image.
         * @param x longitude
         * @param y latitude
         * @param filename the file which should be loaded and displayed
         * @param name the name of the image point
         * @param alignment alignment (Middle or TopLeft)
         */
        ImagePoint(qreal x, qreal y, QString filename, QString name = QString(), Alignment alignment = Middle);

        //! Creates a point which displays the given image
        /*!
         * Use this contructor to display the given image.
         * You have to load that image yourself, but can use it for multiple points.
         * @param x longitude
         * @param y latitude
         * @param pixmap pointer to the image pixmap
         * @param name the name of the image point
         * @param alignment alignment (Middle or TopLeft)
         */
        ImagePoint(qreal x, qreal y, QPixmap pixmap, QString name = QString(), Alignment alignment = Middle);
        virtual ~ImagePoint();
    };
}
#endif
