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

#ifndef OSMMAPADAPTER_H
#define OSMMAPADAPTER_H

#include "qmapcontrol_global.h"
#include "tilemapadapter.h"
namespace qmapcontrol
{
    //! MapAdapter for OpenStreetMap
    /*!
     * This is a conveniece class, which extends and configures a TileMapAdapter
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT OSMMapAdapter : public TileMapAdapter
    {
        Q_OBJECT
                public:
        //! constructor
        /*!
         * This construct a OpenStreetmap Adapter
         */
        OSMMapAdapter();
        virtual ~OSMMapAdapter();
    };
}
#endif
