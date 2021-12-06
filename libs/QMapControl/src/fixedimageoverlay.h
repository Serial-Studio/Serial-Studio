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

#ifndef FIXEDIMAGEOVERLAY_H
#define FIXEDIMAGEOVERLAY_H

#include "qmapcontrol_global.h"
#include "imagepoint.h"

namespace qmapcontrol
{

    //! Draws a fixed image into the map.
    /*!
     * This class draws a image overlay onto a map, whose upper left and lower
     * right corners lay always on the given coordinates. The methods
     * setBaselevel, setMaxsize and setMinsize have no effect for this class.
     *
     * @author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT FixedImageOverlay : public ImagePoint
    {
    public:
        //! Creates an image overlay which loads and displays the given image file
        /*!
        * Use this contructor to load the given image file and let the point
        * display it.
        * When you want multiple points to display the same image, use the
        * other contructor and pass a pointer to that image.
        * @param x_upperleft the coordinate of the upper left corner where the image should be aligned
        * @param y_upperleft the coordinate of the upper left corner where the image should be aligned
        * @param x_lowerright the coordinate of the lower right corner where the image should be aligned
        * @param y_lowerright the coordinate of the lower right corner where the image should be aligned
        * @param filename the file which should be loaded and displayed
        * @param name the name of the image point
        */
        FixedImageOverlay(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QString filename, QString name = QString());

        //! Creates an image overlay which displays the given image
        /*!
        * Use this contructor to display the given image.
        * @param x_upperleft the coordinate of the upper left corner where the image should be aligned
        * @param y_upperleft the coordinate of the upper left corner where the image should be aligned
        * @param x_lowerright the coordinate of the lower right corner where the image should be aligned
        * @param y_lowerright the coordinate of the lower right corner where the image should be aligned
        * @param pixmap pointer to the image pixmap
        * @param name the name of the image point
        */
        FixedImageOverlay(qreal x_upperleft, qreal y_upperleft, qreal x_lowerright, qreal y_lowerright, QPixmap pixmap, QString name = QString());

        virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset);
        virtual ~FixedImageOverlay();

    private:
        qreal x_lowerright;
        qreal y_lowerright;
    };
}
#endif
