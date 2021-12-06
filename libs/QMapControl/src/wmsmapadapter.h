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

#ifndef WMSMAPADAPTER_H
#define WMSMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "mapadapter.h"

namespace qmapcontrol
{
    //! MapAdapter for WMS servers
    /*!
     * Use this derived MapAdapter to display maps from WMS servers
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT WMSMapAdapter : public MapAdapter
    {
    public:
        //! constructor
        /*!
         * Sample of a correct initialization of a MapAdapter:<br/>
         * MapAdapter* mapadapter = new WMSMapAdapter("www2.demis.nl", "/wms/wms.asp?wms=WorldMap[...]&BBOX=%1,%2,%3,%4&WIDTH=%5&HEIGHT=%5&TRANSPARENT=TRUE", 256);<br/>
         * The placeholders %1, %2, %3, %4 creates the bounding box, %5 is for the tilesize
         * The minZoom is 0 (means the whole world is visible). The maxZoom is 17 (means it is zoomed in to the max)
         * @param host The servers URL
         * @param serverPath The path to the tiles with placeholders
         * @param tilesize the size of the tiles
         */
        WMSMapAdapter(QString host, QString serverPath, int tilesize = 256);
        virtual ~WMSMapAdapter();

        virtual QString serverPath() const;
        virtual QPoint coordinateToDisplay(const QPointF&) const;
        virtual QPointF displayToCoordinate(const QPoint&) const;
        virtual void changeHostAddress( const QString qHost, const QString qServerPath = QString() );

    protected:
        virtual void zoom_in();
        virtual void zoom_out();
        virtual QString query(int x, int y, int z) const;
        virtual bool isTileValid(int x, int y, int z) const;

    private:
        virtual QString getQ(qreal ux, qreal uy, qreal ox, qreal oy) const;

        qreal coord_per_x_tile;
        qreal coord_per_y_tile;
        
        QHash<QString,QString> mServerOptions;
        QHash<int,qreal>    mResolutions;
    };
}
#endif
