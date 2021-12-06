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

#ifndef EMPTYMAPADAPTER_H
#define EMPTYMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "mapadapter.h"

namespace qmapcontrol
{
    //! MapAdapter which do not load map tiles.
    /*!
     * The EmptyMapAdapter can be used if QMapControl should not load any map tiles. This is useful if you
     * only want to display an image through a FixedImageOverlay e.g.
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT EmptyMapAdapter : public MapAdapter
    {
        Q_OBJECT
    public:
        //! Constructor.
        /*!
         * @param tileSize This parameter seems unnecessary for this type of MapAdaper on first sight. But since
         * this parameter defines the size of the offscreen image it could be used for a little performance
         * tuning (larger offscreen-images have to be redrawed less times).
         * @param minZoom the minimum zoom level
         * @param maxZoom the maximum zoom level
         */
        EmptyMapAdapter(int tileSize = 256, int minZoom = 0, int maxZoom = 17);

        virtual ~EmptyMapAdapter();

        virtual QPoint coordinateToDisplay(const QPointF&) const;
        virtual QPointF displayToCoordinate(const QPoint&) const;

        qreal PI;

    protected:
        qreal rad_deg(qreal) const;
        qreal deg_rad(qreal) const;

        virtual bool isTileValid(int x, int y, int z) const;
        virtual void zoom_in();
        virtual void zoom_out();
        virtual QString query(int x, int y, int z) const;
        virtual int tilesonzoomlevel(int zoomlevel) const;
        virtual int xoffset(int x) const;
        virtual int yoffset(int y) const;
    };
}
#endif
