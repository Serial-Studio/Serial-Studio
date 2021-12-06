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

#include "mapadapter.h"
namespace qmapcontrol
{
    MapAdapter::MapAdapter(const QString& qHost, const QString& qServerPath, int qTilesize, int qMinZoom, int qMaxZoom)
            :mTileSize(qTilesize), mMin_zoom(qMinZoom), mMax_zoom(qMaxZoom)
    {
        mCurrent_zoom = qMinZoom;
        changeHostAddress( qHost, qServerPath );
    }

    MapAdapter::~MapAdapter()
    {
    }

    void MapAdapter::changeHostAddress( const QString qHost, const QString qServerPath )
    {
        mServerHost = qHost;
        mServerPath = qServerPath;
    }

    QString MapAdapter::host() const
    {
        return mServerHost;
    }
    
    QString MapAdapter::serverPath() const
    {
        return mServerPath;
    }

    int MapAdapter::tilesize() const
    {
        return mTileSize;
    }

    int MapAdapter::minZoom() const
    {
        return mMin_zoom;
    }

    int MapAdapter::maxZoom() const
    {
        return mMax_zoom;
    }

    int MapAdapter::currentZoom() const
    {
        return mCurrent_zoom;
    }

    int MapAdapter::adaptedZoom() const
    {
        return mMax_zoom < mMin_zoom ? mMin_zoom - mCurrent_zoom : mCurrent_zoom;
    }

    void MapAdapter::setBoundingBox(qreal qMinX, qreal qMinY, qreal qMaxX, qreal qMaxY )
    {
        mBoundingBox = QRectF( QPointF( qMinX, qMinY ), QPointF(qMaxX, qMaxY ) ); 
    }

    int MapAdapter::tileSize()
    {
        return mTileSize;
    }
}
