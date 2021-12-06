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

#ifndef LINESTRING_H
#define LINESTRING_H

#include "qmapcontrol_global.h"
#include "curve.h"

namespace qmapcontrol
{
    //! A collection of Point objects to describe a line
    /*!
     * A LineString is a Curve with linear interpolation between Points. Each consecutive pair of Points defines a Line segment.
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT LineString : public Curve
    {
        Q_OBJECT

    public:
        LineString();
        //! constructor
        /*!
         * The constructor of a LineString takes a list of Points to form a line.
         * @param points a list of points
         * @param name the name of the LineString
         * @param pen a QPen can be used to modify the look of the line.
         * @see http://doc.trolltech.com/4.3/qpen.html
         */
        LineString ( QList<Point*> const points, QString name = QString(), QPen* pen = 0 );
        virtual ~LineString();

        //! returns the points of the LineString
        /*!
         * @return  a list with the points of the LineString
         */
        QList<Point*>	points();

        //! adds a point at the end of the LineString
        /*!
         * @param point the point which should be added to the LineString
         */
        void addPoint ( Point* point );

        //! sets the given list as points of the LineString
        //! NOTE: these points will get reparented and cleaned up automatically
        /*!
         * @param points the points which should be set for the LineString
         */
        void setPoints ( QList<Point*> points);

        //! returns the number of Points the LineString consists of
        /*!
         * @return the number of the LineString´s Points
         */
        int numberOfPoints() const;

        //! returns the bounding box (rect) that contains all points
        /*!
         * @return the rect that contains all points
         */
        virtual QRectF boundingBox();

        //! returns true if the LineString has Childs
        /*!
         * This is equal to: numberOfPoints() > 0
         * @return true it the LineString has Childs (=Points)
         * @see clickedPoints()
         */
        virtual bool hasPoints() const;

        //! returns true if the LineString has clicked Points
        /*!
         * @return true if childs of a LineString were clicked
         * @see clickedPoints()
         */
        virtual bool hasClickedPoints() const;

        //! returns the clicked Points
        /*!
         * If a LineString was clicked it could be neccessary to figure out which of its points where clicked.
         * Do do so the methods hasPoints() and clickedPoints() can be used.
         * When a point is added to a LineString the Point becomes its child.
         * It is possible (depending on the zoomfactor) to click more than one Point of a LineString, so this method returns a list.
         * @return the clicked Points of the LineString
         */
        virtual QList<Geometry*> & clickedPoints();

    protected:
        virtual bool Touches ( Geometry* geom, const MapAdapter* mapadapter );
        virtual bool Touches ( Point* geom, const MapAdapter* mapadapter );
        virtual void draw ( QPainter* painter, const MapAdapter* mapadapter, const QRect &screensize, const QPoint offset );

    private:
        //! removes cleans up memory of child points that were reparented with setPoints()
        /*!
         * @see setPoints()
         */
        void removePoints();

        QList<Point*>	childPoints;
    };
}
#endif
