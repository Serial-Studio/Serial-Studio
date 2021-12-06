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


#ifndef GOOGLEAPIMAPADAPTER_H
#define GOOGLEAPIMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "tilemapadapter.h"

namespace qmapcontrol
{
    class QMAPCONTROL_EXPORT googleApiMapadapter : public TileMapAdapter
    {
        Q_OBJECT
    public:

        enum layerType
        {
            layerType_ROADMAP  = 0, //displays the default road map view. This is the default map type.
            layerType_SATELLITE ,   //displays Google Earth satellite images
            layerType_HYBRID ,      //displays a mixture of normal and satellite views
            layerType_TERRAIN       //displays a physical map based on terrain information
        };

        enum apiType
        {
            GoogleMapsAPI = 0,
            GoogleMapsForBusinessesAPI
        };

        googleApiMapadapter(layerType qMapType = layerType_ROADMAP, apiType qApiType = GoogleMapsAPI, QString qApiKey = "", QString qApiClientID = "", QString qServerAddress = "maps.googleapis.com");
        virtual ~googleApiMapadapter();

        virtual QPoint coordinateToDisplay(const QPointF&) const;
        virtual QPointF displayToCoordinate(const QPoint&) const;

        QString	getHost() const;
        void setKey(QString apiKey);
        void setMapLayerType(layerType qMapType = layerType_ROADMAP);

    protected:
        virtual void zoom_in();
        virtual void zoom_out();
        virtual QString query(int x, int y, int z) const;
        virtual bool isValid(int x, int y, int z) const;

        virtual QString signURL( const QString& qURL, const QString& qCryptoKey ) const;

    private:
        QString typeToString( layerType qLayerType );
        virtual QString getQ(qreal longitude, qreal latitude, int zoom) const;
        qreal getMercatorLatitude(qreal YCoord) const;
        qreal getMercatorYCoord(qreal lati) const;

        qreal mCoord_per_x_tile;
        qreal mCoord_per_y_tile;

        QString mApiKey;
        QString mApiClientID;
        apiType mApiType;
        QString mMapLayerType;
    };
}

#endif // GOOGLEAPIMAPADAPTER_H
