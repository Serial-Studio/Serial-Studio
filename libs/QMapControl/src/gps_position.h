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

#ifndef GPS_POSITION_H
#define GPS_POSITION_H

#include "qmapcontrol_global.h"
#include <QString>

namespace qmapcontrol
{
    //! Represents a coordinate from a GPS receiver
    /*!
     * This class is used to represent a coordinate which has been parsed from a NMEA string.
     * This is not fully integrated in the API. An example which uses this data type can be found under Samples.
     * @author Kai Winter
     */
    class QMAPCONTROL_EXPORT GPS_Position
    {
    public:
        GPS_Position(float time, float longitude, QString longitude_dir, float latitude, QString latitude_dir);
        float time; /*!< time of the string*/
        float longitude; /*!< longitude coordinate*/
        float latitude; /*!< latitude coordinate*/

    private:
        QString longitude_dir;
        QString latitude_dir;
    };
}
#endif
