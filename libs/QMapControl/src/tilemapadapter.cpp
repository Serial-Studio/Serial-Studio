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

#include "tilemapadapter.h"
namespace qmapcontrol
{
    TileMapAdapter::TileMapAdapter(const QString& host, const QString& serverPath, int tilesize, int minZoom, int maxZoom)
            :MapAdapter(host, serverPath, tilesize, minZoom, maxZoom)
    {
        PI = acos(-1.0);

        /*
            Initialize the "substring replace engine". First the string replacement
            in getQuery was made by QString().arg() but this was very slow. So this
            splits the servers path into substrings and when calling getQuery the
            substrings get merged with the parameters of the URL.
            Pretty complicated, but fast.
        */
        param1 = serverPath.indexOf("%1");
        param2 = serverPath.indexOf("%2");
        param3 = serverPath.indexOf("%3");

        int min = param1 < param2 ? param1 : param2;
        min = param3 < min ? param3 : min;

        int max = param1 > param2 ? param1 : param2;
        max = param3 > max ? param3 : max;

        int middle = param1+param2+param3-min-max;

        order[0][0] = min;
        if (min == param1)
            order[0][1] = 0;
        else if (min == param2)
            order[0][1] = 1;
        else
            order[0][1] = 2;

        order[1][0] = middle;
        if (middle == param1)
            order[1][1] = 0;
        else if (middle == param2)
            order[1][1] = 1;
        else
            order[1][1] = 2;

        order[2][0] = max;
        if (max == param1)
            order[2][1] = 0;
        else if(max == param2)
            order[2][1] = 1;
        else
            order[2][1] = 2;

        int zoom = mMax_zoom < mMin_zoom ? mMin_zoom - mCurrent_zoom : mCurrent_zoom;
        mNumberOfTiles = tilesonzoomlevel(zoom);
        loc.setNumberOptions(QLocale::OmitGroupSeparator);
    }

    TileMapAdapter::~TileMapAdapter()
    {
    }
    //TODO: pull out
    void TileMapAdapter::zoom_in()
    {
        if (mMin_zoom > mMax_zoom)
        {
            //mCurrent_zoom = mCurrent_zoom-1;
            mCurrent_zoom = mCurrent_zoom > mMax_zoom ? mCurrent_zoom-1 : mMax_zoom;
        }
        else if (mMin_zoom < mMax_zoom)
        {
            //mCurrent_zoom = mCurrent_zoom+1;
            mCurrent_zoom = mCurrent_zoom < mMax_zoom ? mCurrent_zoom+1 : mMax_zoom;
        }

        int zoom = mMax_zoom < mMin_zoom ? mMin_zoom - mCurrent_zoom : mCurrent_zoom;
        mNumberOfTiles = tilesonzoomlevel(zoom);

    }
    void TileMapAdapter::zoom_out()
    {
        if (mMin_zoom > mMax_zoom)
        {
            //mCurrent_zoom = mCurrent_zoom+1;
            mCurrent_zoom = mCurrent_zoom < mMin_zoom ? mCurrent_zoom+1 : mMin_zoom;
        }
        else if (mMin_zoom < mMax_zoom)
        {
            //mCurrent_zoom = mCurrent_zoom-1;
            mCurrent_zoom = mCurrent_zoom > mMin_zoom ? mCurrent_zoom-1 : mMin_zoom;
        }

        int zoom = mMax_zoom < mMin_zoom ? mMin_zoom - mCurrent_zoom : mCurrent_zoom;
        mNumberOfTiles = tilesonzoomlevel(zoom);
    }

    qreal TileMapAdapter::deg_rad(qreal x) const
    {
        return x * (PI/180.0);
    }
    qreal TileMapAdapter::rad_deg(qreal x) const
    {
        return x * (180/PI);
    }

    QString TileMapAdapter::query(int x, int y, int z) const
    {
        x = xoffset(x);
        y = yoffset(y);

        int a[3] = {z, x, y};
        return QString(serverPath().replace(order[2][0],2, loc.toString(a[order[2][1]]))
                .replace(order[1][0],2, loc.toString(a[order[1][1]]))
                .replace(order[0][0],2, loc.toString(a[order[0][1]])));

    }

    QPoint TileMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
    {
        qreal x = (coordinate.x()+180) * (mNumberOfTiles*mTileSize)/360.; // coord to pixel!
        qreal y = (1-(log(tan(PI/4+deg_rad(coordinate.y())/2)) /PI)) /2  * (mNumberOfTiles*mTileSize);

        return QPoint(int(x), int(y));
    }

    QPointF TileMapAdapter::displayToCoordinate(const QPoint& point) const
    {
        qreal longitude = (point.x()*(360/(mNumberOfTiles*mTileSize)))-180;
        qreal latitude = rad_deg(atan(sinh((1-point.y()*(2/(mNumberOfTiles*mTileSize)))*PI)));

        return QPointF(longitude, latitude);

    }

    bool TileMapAdapter::isTileValid(int x, int y, int z) const
    {
        if (mMax_zoom < mMin_zoom)
        {
            z= mMin_zoom - z;
        }

        if (x<0 || x > (1 << z)-1 ||
            y<0 || y > (1 << z)-1)
        {
            return false;
        }

        return true;

    }

    int TileMapAdapter::tilesonzoomlevel(int zoomlevel) const
    {
        return int(pow(2.0, zoomlevel));
    }

    int TileMapAdapter::xoffset(int x) const
    {
        return x;
    }

    int TileMapAdapter::yoffset(int y) const
    {
        return y;
    }
}
