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

#include "googlemapadapter.h"
namespace qmapcontrol
{
    GoogleMapAdapter::GoogleMapAdapter( googleLayerType qLayerType )
        : TileMapAdapter("mt1.google.com", "/vt/v=ap.106&hl=en&x=%2&y=%3&zoom=%1&lyrs=" + typeToString(qLayerType), 256, 17, 0)
            //: TileMapAdapter("tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17)
    {
        QString layerType = typeToString( qLayerType );
    }

    GoogleMapAdapter::~GoogleMapAdapter()
    {
    }

    QString GoogleMapAdapter::typeToString( googleLayerType qLayerType )
    {
        switch (qLayerType)
        {
            case satellite: return "s";
            case terrain: return "t";
            case hybrid: return "h";
            case roadmap:
            default: 
                return "m";
        }
    }
}
