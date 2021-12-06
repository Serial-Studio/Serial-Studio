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

#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "qmapcontrol_global.h"
#include "layer.h"

namespace qmapcontrol
{
    //! MapLayer class
    /*!
     * There are two different layer types:
     *  - MapLayer: Displays Maps, but also Geometries. The configuration for displaying maps have to be done in the MapAdapter
     *  - GeometryLayer: Only displays Geometry objects.
     *
     * MapLayers also can display Geometry objects. The difference to the GeometryLayer is the repainting. Objects that are
     * added to a MapLayer are "baken" on the map. This means, when you change it´s position for example the changes are
     * not visible until a new offscreen image has been drawn. If you have "static" Geometries which won´t change their
     * position this is fine. But if you want to change the objects position or pen you should use a GeometryLayer. Those
     * are repainted immediately on changes.
     *
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT MapLayer : public Layer
    {
        Q_OBJECT

    public:
        //! MapLayer constructor
        /*!
         * This is used to construct a map layer.
         *
         * @param layername The name of the Layer
         * @param mapadapter The MapAdapter which does coordinate translation and Query-String-Forming
         * @param takeevents Should the Layer receive MouseEvents? This is set to true by default. Setting it to false could
         * be something like a "speed up hint"
         */
        MapLayer(QString layername, MapAdapter* mapadapter, bool takeevents=true);
        virtual ~MapLayer();
    };
}
#endif
