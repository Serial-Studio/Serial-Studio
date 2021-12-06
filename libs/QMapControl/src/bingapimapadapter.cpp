/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2007 - 2008 Kai Winter
* Copyright (C)        2014 Mattes Jaehne
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
* original software by Kai Winter
* Contact e-mail: kaiwinter@gmx.de
* Program URL   : http://qmapcontrol.sourceforge.net/
*
* modified to use Google Static Maps API V2  by
*     Mattes Jaehne <mattes@dev.uavp.ch>
* for
*     NGOS - The Next Generation multicopter OS
* http://ng.uavp.ch
*
*/

#include "bingapimapadapter.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace qmapcontrol
{
    bingApiMapadapter::bingApiMapadapter(QString mapType, QString apiKey)
    : TileMapAdapter("dev.virtualearth.net", "/REST/v1/Imagery/Map/", 256, 0, 21),
      myKey(apiKey),
      myMapType(mapType)
    {
        mNumberOfTiles = pow(2., mCurrent_zoom + 0.0);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;

        if (! myKey.isEmpty())
            myKey.prepend("&key=");
        if (myMapType.isEmpty())
            myMapType.append("Road");
    }

    bingApiMapadapter::~bingApiMapadapter()
    {
    }

    QPoint bingApiMapadapter::coordinateToDisplay(const QPointF& coordinate) const
    {
        qreal x = (coordinate.x() + 180.) * (mNumberOfTiles * mTileSize) / 360.;		// coord to pixel!
        qreal y = (1. - log(tan(coordinate.y() * M_PI / 180.) + 1. / cos(coordinate.y() * M_PI / 180.)) / M_PI) / 2. * (mNumberOfTiles*mTileSize);
        x += mTileSize / 2;
        y += mTileSize / 2;

        return QPoint(int(x), int(y));
    }

    QPointF bingApiMapadapter::displayToCoordinate(const QPoint& point) const
    {
        qreal lon = (point.x() - mTileSize / 2) / (mNumberOfTiles * mTileSize) * 360. - 180.;
        qreal lat = M_PI - 2. * M_PI * (point.y() - mTileSize / 2) / (mNumberOfTiles * mTileSize);
        lat = 180. / M_PI * atan(0.5 * (exp(lat) - exp(-lat)));

        return QPointF(lon, lat);
    }

    qreal bingApiMapadapter::getMercatorLatitude(qreal YCoord) const
    {
        if (YCoord > M_PI) return 9999.;
        if (YCoord < -M_PI) return -9999.;

        qreal t = atan(exp(YCoord));
        qreal res = (2. * t) - (M_PI / 2.);

        return res;
    }

    qreal bingApiMapadapter::getMercatorYCoord(qreal lati) const
    {
        qreal phi = M_PI * lati / 180.;
        qreal res = 0.5 * log((1. + sin(phi)) / (1. - sin(phi)));

        return res;
    }

    void bingApiMapadapter::zoom_in()
    {
        if (mCurrent_zoom >= maxZoom())
            return;

        mCurrent_zoom += 1;
        mNumberOfTiles = pow(2, mCurrent_zoom + 0.0);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;
    }

    void bingApiMapadapter::zoom_out()
    {
        if (mCurrent_zoom <= minZoom())
            return;

        mCurrent_zoom -= 1;
        mNumberOfTiles = pow(2, mCurrent_zoom + 0.0);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;
    }

    QString bingApiMapadapter::getQ(qreal longitude, qreal latitude, int zoom) const
    {
        QString location = "/REST/v1/Imagery/Map/";
        if (! myMapType.isEmpty())
            location.append(myMapType + "/");
        else
            location.append("Road/");
        location.append(QVariant(latitude).toString() + ",");
        location.append(QVariant(longitude).toString() + "/");
        location.append(QString::number(zoom) + "?");
        location.append("&mapSize=" + QString::number(mTileSize) + "," + QString::number(mTileSize));

        if (! myKey.isEmpty())
            location.append(myKey);
        else
            fprintf(stderr, "You are useing Bing Maps API without a (valid) key. This is not possible...\r\n");

        return location;
    }

    void bingApiMapadapter::setKey(QString apiKey)
    {
        if (apiKey.isEmpty())
            return;

        myKey.clear();
        myKey.append("&key=" + apiKey);
    }

    void bingApiMapadapter::setMapType(QString mapType) /* Aerial, AerialWithLabels, Road  */
    {
        if (mapType.isEmpty())
            return;

        myMapType.clear();
        myMapType.append(mapType);
    }
}
