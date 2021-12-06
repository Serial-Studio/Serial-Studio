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

#ifndef TILEMAPADAPTER_H
#define TILEMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "mapadapter.h"

namespace qmapcontrol
{
    //! MapAdapter for servers with image tiles
    /*!
     * Use this derived MapAdapter to display maps from OpenStreetMap
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT TileMapAdapter : public MapAdapter
    {
        Q_OBJECT
                public:
        //! constructor
        /*!
         * Sample of a correct initialization of a MapAdapter:<br/>
         * TileMapAdapter* ta = new TileMapAdapter("192.168.8.1", "/img/img_cache.php/%1/%2/%3.png", 256, 0,17);<br/>
         * The placeholders %1, %2, %3 stands for x, y, z<br/>
         * The minZoom is 0 (means the whole world is visible). The maxZoom is 17 (means it is zoomed in to the max)
         * @param host The servers URL
         * @param serverPath The path to the tiles with placeholders
         * @param tilesize the size of the tiles
         * @param minZoom the minimum zoom level
         * @param maxZoom the maximum zoom level
         */
        TileMapAdapter(const QString& host, const QString& serverPath, int tilesize, int minZoom = 0, int maxZoom = 17);

        virtual ~TileMapAdapter();

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
