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

#ifndef MAPADAPTER_H
#define MAPADAPTER_H

#include "qmapcontrol_global.h"
#include <QObject>
#include <QSize>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QLocale>
#include <QDebug>
#include <cmath>

namespace qmapcontrol
{
    //! Used to fit map servers into QMapControl
    /*!
     * MapAdapters are needed to convert between world- and display coordinates.
     * This calculations depend on the used map projection.
     * There are two ready-made MapAdapters:
     *  - TileMapAdapter, which is ready to use for OpenStreetMap or Google (Mercator projection)
     *  - WMSMapAdapter, which could be used for the most WMS-Server (some servers show errors, because of image ratio)
     *
     * MapAdapters are also needed to form the HTTP-Queries to load the map tiles.
     * The maps from WMS Servers are also divided into tiles, because those can be better cached.
     *
     * @see TileMapAdapter, @see WMSMapAdapter
     *
     *  @author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT MapAdapter : public QObject
    {
        friend class Layer;

        Q_OBJECT

    public:
        virtual ~MapAdapter();

        //! returns the host of this MapAdapter
        /*!
         * @return  the host of this MapAdapter
         */
        QString host() const;
        
        //! returns the server path part of this MapAdapter
        /*!
         * @return  the serverpath of this MapAdapter
         */
        virtual QString serverPath() const;

        //! change or update server host address post init
        /*!
         * @param host the host address
         * @param serverPath the server path
         */
        virtual void changeHostAddress( const QString qHost, const QString qServerPath = QString() );

        //! returns the size of the tiles
        /*!
         * @return the size of the tiles
         */
        int tilesize() const;

        //! returns the min zoom value
        /*!
         * @return the min zoom value
         */
        int minZoom() const;

        //! returns the max zoom value
        /*!
         * @return the max zoom value
         */
        int maxZoom() const;

        //! returns the current zoom
        /*!
         * @return the current zoom
         */
        int currentZoom() const;

        virtual int adaptedZoom()const;
        
        //! translates a world coordinate to display coordinate
        /*!
         * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
         * To divide model from view the current zoom should be moved to the layers.
         * @param  coordinate the world coordinate
         * @return the display coordinate (in widget coordinates)
         */
        virtual QPoint coordinateToDisplay(const QPointF& coordinate) const = 0;

        //! translates display coordinate to world coordinate
        /*!
         * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
         * To divide model from view the current zoom should be moved to the layers.
         * @param  point the display coordinate
         * @return the world coordinate
         */
        virtual QPointF displayToCoordinate(const QPoint& point) const = 0;

        QRectF getBoundingbox() const { return mBoundingBox; }
        void setBoundingBox(qreal qMinX, qreal qMinY, qreal qMaxX, qreal qMaxY );

        //! returns the tilesize for the mapadapter
        /*!
         * returns the tilesize for the mapadapter
         * @return the tile size
         */
        int tileSize();

    protected:
        MapAdapter(const QString& qHost, const QString& qServerPath, int qTilesize, int qMinZoom = 0, int qMaxZoom = 0);
        virtual void zoom_in() = 0;
        virtual void zoom_out() = 0;
        virtual bool isTileValid(int x, int y, int z) const = 0;
        virtual QString query(int x, int y, int z) const = 0;

        QSize       mSize;
        QString     mServerHost;
        QString     mServerPath;

        int         mTileSize;
        int         mMin_zoom;
        int         mMax_zoom;
        int         mCurrent_zoom;

        int param1;
        int param2;
        int param3;
        int param4;
        int param5;
        int param6;

        QString sub1;
        QString sub2;
        QString sub3;
        QString sub4;
        QString sub5;
        QString sub6;

        int order[3][2];

        int mMiddle_x;
        int mMiddle_y;

        qreal mNumberOfTiles;
        QLocale loc;
        QRectF mBoundingBox;
    };
}
#endif
