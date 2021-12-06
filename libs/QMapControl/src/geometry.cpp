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

#include "geometry.h"
namespace qmapcontrol
{
    Geometry::Geometry(QString name)
            : GeometryType("Geometry"), myparentGeometry(0), mypen(0), visible(true), myname(name)
    {
    }

    Geometry::~Geometry()
    {
    }

    QString Geometry::name() const
    {
        return myname;
    }
    Geometry* Geometry::parentGeometry() const
    {
        return myparentGeometry;
    }
    void Geometry::setParentGeometry(Geometry* geom)
    {
        myparentGeometry = geom;
    }
    bool Geometry::hasPoints() const
    {
        return false;
    }
    bool Geometry::hasClickedPoints() const
    {
        return false;
    }

    QList<Geometry*>& Geometry::clickedPoints()
    {
        return touchedPoints;
    }

    bool Geometry::isVisible() const
    {
        return visible;
    }
    void Geometry::setVisible(bool visible)
    {
        this->visible = visible;
        emit(updateRequest(boundingBox()));
    }

    void Geometry::setName(QString name)
    {
        myname = name;
    }

    void Geometry::setPen(QPen* pen)
    {
        mypen = pen;
    }
    QPen* Geometry::pen() const
    {
        return mypen;
    }
}
