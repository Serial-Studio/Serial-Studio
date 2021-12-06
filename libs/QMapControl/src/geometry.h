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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "qmapcontrol_global.h"
#include <QObject>
#include <QPainter>
#include <QDebug>
#include "mapadapter.h"

namespace qmapcontrol
{
    class Point;
    //! Main class for objects that should be painted in maps
    /*!
     * Geometry is the root class of the hierarchy. Geometry is an abstract (non-instantiable) class.
     *
     * This class and the derived classes Point, Curve and LineString are leant on the Simple
     * Feature Specification of the Open Geospatial Consortium.
     * @see www.opengeospatial.com
     *
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT Geometry : public QObject
    {
        friend class LineString;
        Q_OBJECT
    public:
        explicit Geometry(QString name = QString());
        virtual ~Geometry();

        QString GeometryType;

        //!
        /*! returns true if the given Geometry is equal to this Geometry
         *  not implemented yet!
         * @param geom The Geometry to be tested
         * @return true if the given Geometry is equal to this
         */
        bool Equals(Geometry* geom);

        //! returns a String representation of this Geometry
        /*!
         * not implemented yet!
         * @return a String representation of this Geometry
         */
        QString toString();

        //! returns the name of this Geometry
        /*!
         * @return the name of this Geometry
         */
        QString name() const;

        //! returns the parent Geometry of this Geometry
        /*!
         * A LineString is a composition of many Points. This methods returns the parent (the LineString) of a Point
         * @return the parent Geometry of this Geometry
         */
        Geometry* parentGeometry() const;

        //! returns true if this Geometry is visible
        /*!
         * @return true if this Geometry is visible
         */
        bool isVisible() const;

        //! sets the name of the geometry
        /*!
         * @param name the new name of the geometry
         */
        void setName(QString name);

        //! returns the QPen which is used on drawing
        /*!
         * The pen is set depending on the Geometry. A CirclePoint for example takes one with the constructor.
         * @return the QPen which is used for drawing
         */
        QPen* pen() const;

        //! returns the BoundingBox
        /*!
         * The bounding box in world coordinates
         * @return the BoundingBox
         */
        virtual QRectF boundingBox()=0;
        virtual bool Touches(Point* geom, const MapAdapter* mapadapter)=0;
        virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)=0;
        virtual bool hasPoints() const;
        virtual bool hasClickedPoints() const;
        virtual void setPen(QPen* pen);
        virtual QList<Geometry*>& clickedPoints();
        virtual QList<Point*> points()=0;

    private:
        Q_DISABLE_COPY( Geometry )

        Geometry* myparentGeometry;
        QList<Geometry*> 	touchedPoints;

    protected:
        QPen* mypen;
        bool visible;
        QString myname;
        void setParentGeometry(Geometry* geom);

    signals:
        void updateRequest(Geometry* geom);
        void updateRequest(QRectF rect);
        //! This signal is emitted when a Geometry is clicked
        /*!
         * A Geometry is clickable, if the containing layer is clickable.
         * The objects emits a signal if it gets clicked
         * @param  geometry The clicked Geometry
         * @param  point -unused-
         */
        void geometryClicked(Geometry* geometry, QPoint point);

        //! A Geometry emits this signal, when its position gets changed
        /*!
         * @param geom the Geometry
         */
        void positionChanged(Geometry* geom);

    public slots:
        //! if visible is true, the layer is made visible
        /*!
         * @param  visible if the layer should be visible
         */
        virtual void setVisible(bool visible);
    };
}
#endif
