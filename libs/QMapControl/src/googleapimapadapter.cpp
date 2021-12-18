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

#include "googleapimapadapter.h"
#include <QCryptographicHash>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

namespace qmapcontrol
{
googleApiMapadapter::googleApiMapadapter(layerType qMapType, apiType qApiType,
                                         QString qApiKey, QString qApiClientID,
                                         QString qServerAddress)
    : TileMapAdapter(qServerAddress, "/maps/api/staticmap?", 256, 1, 22)
    , mApiKey(qApiKey)
    , mApiClientID(qApiClientID)
    , mApiType(qApiType)
{
    mMapLayerType = typeToString(qMapType);
    mNumberOfTiles = pow(2., mCurrent_zoom + 0.0);
    mCoord_per_x_tile = 360. / mNumberOfTiles;
    mCoord_per_y_tile = 180. / mNumberOfTiles;

    bool usingBusinessAPI = (mApiType == GoogleMapsForBusinessesAPI);

    if (!mApiKey.isEmpty() && !usingBusinessAPI)
    {
        mApiKey.prepend("&key=");
    }

    if (!mApiClientID.isEmpty() && usingBusinessAPI)
    {
        mApiClientID.prepend("&client=");
    }

    if (!mMapLayerType.isEmpty())
    {
        mMapLayerType.prepend("&maptype=");
    }
}

googleApiMapadapter::~googleApiMapadapter() { }

QString googleApiMapadapter::getHost() const
{
    return QString("maps.googleapis.com");
}

QPoint googleApiMapadapter::coordinateToDisplay(const QPointF &coordinate) const
{
    qreal x = (coordinate.x() + 180.) * (mNumberOfTiles * mTileSize)
        / 360.; // coord to pixel!
    qreal y
        = (1.
           - log(tan(coordinate.y() * PI / 180.) + 1. / cos(coordinate.y() * PI / 180.))
               / PI)
        / 2. * (mNumberOfTiles * mTileSize);
    x += mTileSize / 2;
    y += mTileSize / 2;

    return QPoint(int(x), int(y));
}

QPointF googleApiMapadapter::displayToCoordinate(const QPoint &point) const
{
    qreal lon = (point.x() - mTileSize / 2) / (mNumberOfTiles * mTileSize) * 360. - 180.;
    qreal lat = PI - 2. * PI * (point.y() - mTileSize / 2) / (mNumberOfTiles * mTileSize);
    lat = 180. / PI * atan(0.5 * (exp(lat) - exp(-lat)));

    return QPointF(lon, lat);
}

qreal googleApiMapadapter::getMercatorLatitude(qreal YCoord) const
{
    if (YCoord > PI)
        return 9999.;
    if (YCoord < -PI)
        return -9999.;

    qreal t = atan(exp(YCoord));
    qreal res = (2. * t) - (PI / 2.);

    return res;
}

qreal googleApiMapadapter::getMercatorYCoord(qreal lati) const
{
    qreal phi = PI * lati / 180.;
    qreal res = 0.5 * log((1. + sin(phi)) / (1. - sin(phi)));

    return res;
}

void googleApiMapadapter::zoom_in()
{
    if (mCurrent_zoom >= maxZoom())
        return;

    mCurrent_zoom += 1;
    mNumberOfTiles = pow(2, mCurrent_zoom + 0.0);
    mCoord_per_x_tile = 360. / mNumberOfTiles;
    mCoord_per_y_tile = 180. / mNumberOfTiles;
}

void googleApiMapadapter::zoom_out()
{
    if (mCurrent_zoom <= minZoom())
        return;

    mCurrent_zoom -= 1;
    mNumberOfTiles = pow(2, mCurrent_zoom + 0.0);
    mCoord_per_x_tile = 360. / mNumberOfTiles;
    mCoord_per_y_tile = 180. / mNumberOfTiles;
}

bool googleApiMapadapter::isValid(int x, int y, int z) const
{
    if ((x >= 0 && x < mNumberOfTiles) && (y >= 0 && y < mNumberOfTiles) && z >= 0)
        return true;

    return false;
}

QString googleApiMapadapter::typeToString(layerType qLayerType)
{
    switch (qLayerType)
    {
        case layerType_SATELLITE:
            return "satellite";
        case layerType_HYBRID:
            return "hybrid";
        case layerType_TERRAIN:
            return "terrain";
        case layerType_ROADMAP:
        default:
            return "roadmap";
    }
}

QString googleApiMapadapter::query(int i, int j, int z) const
{
    qreal longi = ((i * mTileSize) - (mTileSize * pow(2.0, z - 1)))
        / ((mTileSize * pow(2.0, z)) / 360.);
    qreal latit = PI - 2. * PI * j / pow(2., z);
    latit = 180. / PI * atan(0.5 * (exp(latit) - exp(-latit)));

    return getQ(longi, latit, z);
}

QString googleApiMapadapter::getQ(qreal longitude, qreal latitude, int zoom) const
{
    QString location = "/maps/api/staticmap?&sensor=false&center=";
    location.append(QVariant(latitude).toString());
    location.append(",");
    location.append(QVariant(longitude).toString());
    location.append("&zoom=");
    location.append(QString::number(zoom));
    location.append("&size=" + QString::number(mTileSize) + "x"
                    + QString::number(mTileSize) + "&scale=1");

    if (!mMapLayerType.isEmpty())
    {
        location.append(mMapLayerType);
    }

    if (mApiType == GoogleMapsAPI)
    {
        if (mApiClientID.isEmpty())
        {
            fprintf(stderr,
                    "You are using Google Maps API without a (valid) key. This is "
                    "against \"Terms of use\" of Google Maps\r\n");
        }
        else
        {
            location.append(mApiClientID);
        }
    }
    else if (mApiType == GoogleMapsForBusinessesAPI)
    {
        if (mApiClientID.isEmpty())
        {
            fprintf(stderr,
                    "You are using Google Maps API without a (valid) key. This is "
                    "against \"Terms of use\" of Google Maps\r\n");
        }
        else
        {
            // Google maps for business requires we sign every URL request against our
            // apiKey

            // Example as taken from
            // https://developers.google.com/maps/documentation/business/webservices/auth#digital_signatures
            //
            // URL:
            // https://maps.googleapis.com/maps/api/geocode/json?address=New+York&client=clientID
            // Private Key: vNIXE0xscrmjlyV-12Nj_BvUPaw=
            // URL Portion to Sign:
            // /maps/api/geocode/json?address=New+York&client=clientID Signature:
            // chaRF2hTJKOScPr-RQCEhZbSzIE=
            QString urlSignature = "&signature=";
            QString computedHash = signURL(location, mApiKey);
            urlSignature.append(computedHash);
            location.append(urlSignature);
        }
    }

    return location;
}

QString googleApiMapadapter::signURL(const QString &qURL, const QString &qCryptoKey) const
{
    // Convert the key from 'web safe' base 64 to binary
    QString usablePrivateKey = qCryptoKey;
    usablePrivateKey.replace("-", "+");
    usablePrivateKey.replace("_", "/");
    QByteArray privateKeyBytes;
    privateKeyBytes.append(qPrintable(usablePrivateKey));

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(qPrintable(qURL));

    QString rawSignature = hash.hash(privateKeyBytes, QCryptographicHash::Sha1);

    // convert the bytes to string and make url-safe by replacing '+' and '/' characters
    QByteArray base64signature;
    base64signature.append(rawSignature.toUtf8());

    // base 64 encode the binary signature
    QString result = base64signature.toBase64();

    // convert the signature to 'web safe' base 64
    result.replace("+", "-");
    result.replace("/", "_");

    return result;
}

void googleApiMapadapter::setKey(QString qApiKey)
{
    if (qApiKey.isEmpty())
        return;

    mApiKey.clear();
    mApiKey.append("&key=");
    mApiKey.append(qApiKey);
}

void googleApiMapadapter::setMapLayerType(layerType qMapType)
{
    mMapLayerType.clear();
    mMapLayerType.append("&maptype=");
    mMapLayerType.append(typeToString(qMapType));
}
}
