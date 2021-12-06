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

#include "emptymapadapter.h"
namespace qmapcontrol
{
    EmptyMapAdapter::EmptyMapAdapter(int tileSize, int minZoom, int maxZoom)
            :MapAdapter("", "", 256, minZoom, maxZoom)
    {
        Q_UNUSED(tileSize)
        PI = acos(-1.0);

        mNumberOfTiles = tilesonzoomlevel(minZoom);
    }

    EmptyMapAdapter::~EmptyMapAdapter()
    {
    }

    void EmptyMapAdapter::zoom_in()
    {
        if (mCurrent_zoom < mMax_zoom)
        {
            mCurrent_zoom = mCurrent_zoom + 1;
        }
        mNumberOfTiles = tilesonzoomlevel(mCurrent_zoom);
    }

    void EmptyMapAdapter::zoom_out()
    {
        if (mCurrent_zoom > mMin_zoom)
        {
            mCurrent_zoom = mCurrent_zoom - 1;
        }
        mNumberOfTiles = tilesonzoomlevel(mCurrent_zoom);
    }

    qreal EmptyMapAdapter::deg_rad(qreal x) const
    {
        return x * (PI/180.0);
    }

    qreal EmptyMapAdapter::rad_deg(qreal x) const
    {
        return x * (180/PI);
    }

    QString EmptyMapAdapter::query(int x, int y, int z) const
    {
        Q_UNUSED(x)
        Q_UNUSED(y)
        Q_UNUSED(z)
        return QString();
    }

    QPoint EmptyMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
    {
        qreal x = (coordinate.x()+180) * (mNumberOfTiles*mTileSize)/360.; // coord to pixel!
        qreal y = (1-(log(tan(PI/4+deg_rad(coordinate.y())/2)) /PI)) /2  * (mNumberOfTiles*mTileSize);

        return QPoint(int(x), int(y));
    }

    QPointF EmptyMapAdapter::displayToCoordinate(const QPoint& point) const
    {
        qreal longitude = (point.x()*(360/(mNumberOfTiles*mTileSize)))-180;
        qreal latitude = rad_deg(atan(sinh((1-point.y()*(2/(mNumberOfTiles*mTileSize)))*PI)));

        return QPointF(longitude, latitude);

    }

    bool EmptyMapAdapter::isTileValid(int x, int y, int z) const
    {
        if (mMax_zoom < mMin_zoom)
        {
            z= mMin_zoom - z;
        }

        bool result = true;
        if (x<0 || x>pow(2.0,z)-1 ||
            y<0 || y>pow(2.0,z)-1)
        {
            result = false;
        }
        return result;
    }

    int EmptyMapAdapter::tilesonzoomlevel(int zoomlevel) const
    {
        return int(pow(2.0, zoomlevel));
    }

    int EmptyMapAdapter::xoffset(int x) const
    {
        return x;
    }

    int EmptyMapAdapter::yoffset(int y) const
    {
        return y;
    }
}
