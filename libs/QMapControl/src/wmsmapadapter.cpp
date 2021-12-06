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

#include "wmsmapadapter.h"
#include <QStringList>

namespace qmapcontrol
{
    WMSMapAdapter::WMSMapAdapter(QString host, QString serverPath, int tilesize)
            : MapAdapter(host, serverPath, tilesize, 0, 17)
    {
        mNumberOfTiles = pow(2.0, mCurrent_zoom);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;
        changeHostAddress( host, serverPath );

        loc = QLocale(QLocale::English);
        loc.setNumberOptions( QLocale::OmitGroupSeparator );
        setBoundingBox( -180, -90, 180, 90 );
        
        qreal res = 0.703125;
        for( int z = 0; z < 17; z++ )
        {
            mResolutions[z] = res;
            res = res / 2;
        }

    }

    WMSMapAdapter::~WMSMapAdapter()
    {
    }

    void WMSMapAdapter::changeHostAddress( const QString qHost, const QString qServerPath )
    {
        mServerOptions.clear();
        QString serverPathPrefix;

        if ( qServerPath.contains("?") && 
             qServerPath.split("?").size() > 1 )
        {
            serverPathPrefix = qServerPath.split("?").at( 0 );
            foreach( QString opt, qServerPath.split("?").at(1).split("&") )
            {
                if (opt.contains( "="))
                {
                    mServerOptions[ opt.split("=").at(0).toUpper() ] = opt.split("=").at(1);
                }
            }
        }
        else
        {
            serverPathPrefix = "/wms/";
        }

        MapAdapter::changeHostAddress(qHost,serverPathPrefix);

        //force expected parameters
        if (!mServerOptions.contains("VERSION"))
        {
            mServerOptions["VERSION"] = "1.1.1";
        }
        if (!mServerOptions.contains("TRANSPARENT"))
        {
            mServerOptions["TRANSPARENT"]= "TRUE";
        }
        //if (!mServerOptions.contains("LAYERS"))
        //{
        //    mServerOptions["LAYERS"] = TBD;
        //}
        if (!mServerOptions.contains("SRS") && 
            !mServerOptions.contains("CRS"))
        {
            mServerOptions["SRS"] = "EPSG:4326";
            //mServerOptions["SRS"] = "EPSG:900913"; //google mercator
            //mServerOptions["SRS"] = "EPSG:3857"; //EPSG:3857 is a Spherical Mercator projection
        }
        if (!mServerOptions.contains("STYLES"))
        {
            mServerOptions["STYLES"]= QString();
        }
        if (!mServerOptions.contains("FORMAT"))
        {
            mServerOptions["FORMAT"] = "IMAGE/PNG";
        }

        mServerOptions["SRS"] = "EPSG:4326"; //mercator
        mServerOptions["SERVICE"]= "WMS";
        mServerOptions["TILED"]= "TRUE";
        mServerOptions["REQUEST"]= "GetMap";
        mServerOptions["WIDTH"]= loc.toString(tilesize());
        mServerOptions["HEIGHT"]= loc.toString(tilesize());
        mServerOptions.remove("BBOX"); //added at time of query string
    }

    QString WMSMapAdapter::serverPath() const
    {
        QString urlPath;
        
        foreach( QString key, mServerOptions.keys() )
        {
            if (!urlPath.isEmpty())
            {
                urlPath.append("&");
            }
            urlPath.append( QString("%1=%2").arg( key ).arg( mServerOptions[key] ) );
        }

        return QString("%1?%2").arg( MapAdapter::serverPath() ).arg( urlPath );
    }

    QPoint WMSMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
    {
        qreal x = (coordinate.x()+180) * (mNumberOfTiles*mTileSize)/360.; // coord to pixel!
        qreal y = -1*(coordinate.y()-90) * (mNumberOfTiles*mTileSize)/180.; // coord to pixel!
        return QPoint(int(x), int(y));
    }
    QPointF WMSMapAdapter::displayToCoordinate(const QPoint& point) const
    {
        qreal lon = (point.x()*(360./(mNumberOfTiles*mTileSize)))-180;
        qreal lat = -(point.y()*(180./(mNumberOfTiles*mTileSize)))+90;
        return QPointF(lon, lat);
    }
    void WMSMapAdapter::zoom_in()
    {
        mCurrent_zoom+=1;
        mNumberOfTiles = pow(2.0, mCurrent_zoom);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;
    }
    void WMSMapAdapter::zoom_out()
    {
        mCurrent_zoom-=1;
        mNumberOfTiles = pow(2.0, mCurrent_zoom);
        coord_per_x_tile = 360. / mNumberOfTiles;
        coord_per_y_tile = 180. / mNumberOfTiles;
    }

    bool WMSMapAdapter::isTileValid(int /*x*/, int /*y*/, int /*z*/) const
    {
        // if (x>0 && y>0 && z>0)
        {
            return true;
        }
        // return false;
    }
    QString WMSMapAdapter::query(int i, int j, int /*z*/) const
    {
        return getQ(-180+i*coord_per_x_tile,
                    90-(j+1)*coord_per_y_tile,
                    -180+i*coord_per_x_tile+coord_per_x_tile,
                    90-(j+1)*coord_per_y_tile+coord_per_y_tile);
    }

    QString WMSMapAdapter::getQ(qreal ux, qreal uy, qreal ox, qreal oy) const
    {
        qreal x1 = ux;
        qreal y1 = uy;
        qreal x2 = ox;
        qreal y2 = oy;

        //if ( mServerOptions["SRS"].toUpper() == "EPSG:4326" )
        //{
        //    if ( x1 < 0 )
        //    {
        //        x1 += 180;
        //    }
        //    if ( x1 > 360 )
        //    {
        //        x1 -= 180;
        //    }

        //    if ( x2 < 0 )
        //    {
        //        x2 += 180;
        //    }
        //    if ( x2 > 360 )
        //    {
        //        x2 -= 180;
        //    }
        //}
                
        return QString("%1&BBOX=%2,%3,%4,%5")
                        .arg(serverPath())
                        .arg(QString::number(x1,'f',6))
                        .arg(QString::number(y1,'f',6))
                        .arg(QString::number(x2,'f',6))
                        .arg(QString::number(y2,'f',6));
    }
}
