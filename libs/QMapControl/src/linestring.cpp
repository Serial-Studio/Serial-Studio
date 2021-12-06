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

#include "linestring.h"
namespace qmapcontrol
{
    LineString::LineString()
            : Curve()
    {
        GeometryType = "LineString";
    }

    LineString::LineString(QList<Point*> const points, QString name, QPen* pen)
            :Curve(name)
    {
        mypen = pen;
        LineString();
        setPoints(points);
    }

    LineString::~LineString()
    {
        removePoints();
    }

    void LineString::removePoints()
    {
        QListIterator<Point*> iter(childPoints);
        while(iter.hasNext())
        {
            Point *pt = iter.next();
            if (pt && pt->parentGeometry() == this)
            {
                delete pt;
                pt = 0;
            }
        }
        childPoints.clear();
    }

    void LineString::addPoint(Point* point)
    {
        point->setParentGeometry(this);
        childPoints.append(point);
    }

    QList<Point*> LineString::points()
    {
        return childPoints;
    }

    void LineString::setPoints(QList<Point*> points)
    {
        removePoints();

        for (int i=0; i<points.size(); i++)
        {
            points.at(i)->setParentGeometry(this);
        }
        childPoints = points;
    }

    void LineString::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &screensize, const QPoint offset)
    {
        if (!visible)
            return;

        QPolygon p = QPolygon();

        QPointF c;
        for (int i=0; i<childPoints.size(); i++)
        {
            c = childPoints[i]->coordinate();
            p.append(mapadapter->coordinateToDisplay(c));
        }
        if (mypen != 0)
        {
            painter->save();
            painter->setPen(*mypen);
        }
        painter->drawPolyline(p);
        if (mypen != 0)
        {
            painter->restore();
        }
        for (int i=0; i<childPoints.size(); i++)
        {
            childPoints[i]->draw(painter, mapadapter, screensize, offset);
        }
    }

    int LineString::numberOfPoints() const
    {
        return childPoints.count();
    }

    bool LineString::Touches(Point* geom, const MapAdapter* mapadapter)
    {
        touchedPoints.clear();

        if (points().size() < 2)
        {
            // really shouldn't end up here since we always add atleast points to create a line
            return false;
        }

        bool touches = false;

        QPointF clickPt = mapadapter->coordinateToDisplay(geom->coordinate());

        qreal halfwidth = 2; // use 2 pixels by default
        if (mypen && mypen->width() > 0)
        {
            halfwidth = static_cast<qreal> (mypen->width())/ static_cast<qreal> (2);
        }

        QPointF pt1 = mapadapter->coordinateToDisplay(points().at(0)->coordinate());
        qreal pt1x1 = pt1.x() - halfwidth;
        qreal pt1x2 = pt1.x() + halfwidth;
        qreal pt1y1 = pt1.y() - halfwidth;
        qreal pt1y2 = pt1.y() + halfwidth;
        for (int i = 1; i < childPoints.size(); ++i)
        {
            QPointF pt2 = mapadapter->coordinateToDisplay(childPoints.at(i)->coordinate());
            qreal pt2x1 = pt2.x() - halfwidth;
            qreal pt2x2 = pt2.x() + halfwidth;
            qreal pt2y1 = pt2.y() - halfwidth;
            qreal pt2y2 = pt2.y() + halfwidth;

            // build lazy bounding box
            qreal upperLeftX = qMin(pt1x1, qMin(pt1x2, qMin(pt2x1, pt2x2)));
            qreal upperLeftY = qMin(pt1y1, qMin(pt1y2, qMin(pt2y1, pt2y2)));
            qreal lowerRightX = qMax(pt1x1, qMax(pt1x2, qMax(pt2x1, pt2x2)));
            qreal lowerRightY = qMax(pt1y1, qMax(pt1y2, qMax(pt2y1, pt2y2)));

            QRectF bounds(QPointF(upperLeftX, upperLeftY),
                          QPointF(lowerRightX,lowerRightY));

            if (bounds.contains(clickPt))
            {
                touchedPoints.append(childPoints.at(i));
                touches = true;
            }

            pt1x1 = pt2x1;
            pt1x2 = pt2x2;
            pt1y1 = pt2y1;
            pt1y2 = pt2y2;
        }

        if (touches)
        {
            emit(geometryClicked(this, QPoint(0,0)));
        }

        return touches;
    }

    bool LineString::Touches(Geometry* /*geom*/, const MapAdapter* /*mapadapter*/)
    {
        touchedPoints.clear();

        return false;
    }

    QList<Geometry*>& LineString::clickedPoints()
    {
        return Geometry::clickedPoints();
    }

    bool LineString::hasPoints() const
    {
        return childPoints.size() > 0 ? true : false;
    }

    bool LineString::hasClickedPoints() const
    {
        return touchedPoints.size() > 0 ? true : false;
    }

    QRectF LineString::boundingBox()
    {
        qreal minlon=180;
        qreal maxlon=-180;
        qreal minlat=90;
        qreal maxlat=-90;
        for (int i=0; i<childPoints.size(); ++i)
        {
            Point* tmp = childPoints.at(i);
            if (tmp->longitude() < minlon) minlon = tmp->longitude();
            if (tmp->longitude() > maxlon) maxlon = tmp->longitude();
            if (tmp->latitude() < minlat) minlat = tmp->latitude();
            if (tmp->latitude() > maxlat) maxlat = tmp->latitude();
        }
        QPointF min = QPointF(minlon, minlat);
        QPointF max = QPointF(maxlon, maxlat);
        QPointF dist = max - min;
        QSizeF si = QSizeF(dist.x(), dist.y());

        return QRectF(min, si);
    }
}
