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
* modified to use Bing Maps by
*     Mattes Jaehne <mattes@dev.uavp.ch>
* for
*     NGOS - The Next Generation multicopter OS
* http://ng.uavp.ch
*
*/


#ifndef BINGAPIMAPADAPTER_H
#define BINGAPIMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "tilemapadapter.h"

namespace qmapcontrol
{
    class QMAPCONTROL_EXPORT bingApiMapadapter : public TileMapAdapter
    {
        Q_OBJECT
    public:
        bingApiMapadapter(QString mapType = "Road", QString apiKey = "");
        virtual ~bingApiMapadapter();

        virtual QPoint coordinateToDisplay(const QPointF&) const;
        virtual QPointF displayToCoordinate(const QPoint&) const;

        void setKey(QString apiKey);
        void setMapType(QString mapType); /* Aerial, AerialWithLabels, Road */

    protected:
        virtual void zoom_in();
        virtual void zoom_out();

    private:
        virtual QString getQ(qreal longitude, qreal latitude, int zoom) const;
        qreal getMercatorLatitude(qreal YCoord) const;
        qreal getMercatorYCoord(qreal lati) const;

        qreal coord_per_x_tile;
        qreal coord_per_y_tile;
        int srvNum;
        QString myKey;
        QString myMapType;
    };
}

#endif // BINGAPIMAPADAPTER_H
