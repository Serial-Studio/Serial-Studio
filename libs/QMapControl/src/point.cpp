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

#include "point.h"
namespace qmapcontrol
{
    Point::Point()
    {}
    Point::Point(const Point& point)
            :Geometry(point.name()), X(point.longitude()), Y(point.latitude())
    {
        visible = point.isVisible();
        mywidget = 0;
        mypixmap = QPixmap();
        mypen = point.mypen;
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);
    }

    Point::Point(qreal x, qreal y, QString name, enum Alignment alignment)
            : Geometry(name), X(x), Y(y), myalignment(alignment)
    {
        GeometryType = "Point";
        mywidget = 0;
        mypixmap = QPixmap();
        visible = true;
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);
    }

    Point::Point(qreal x, qreal y, QWidget* widget, QString name, enum Alignment alignment)
            : Geometry(name), X(x), Y(y), mywidget(widget), myalignment(alignment)
    {
        // Point(x, y, name, alignment);
        GeometryType = "Point";
        mypixmap = QPixmap();
        visible = true;
        size = widget->size();
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);

        if(mywidget!=0)
        {
            mywidget->show();
        }
    }
    Point::Point(qreal x, qreal y, QPixmap pixmap, QString name, enum Alignment alignment)
            : Geometry(name), X(x), Y(y), mypixmap(pixmap), myalignment(alignment)
    {
        GeometryType = "Point";
        mywidget = 0;
        visible = true;
        size = pixmap.size();
        homelevel = -1;
        minsize = QSize(-1,-1);
        maxsize = QSize(-1,-1);
    }
    /*
        Point& Point::operator=(const Point& rhs)
        {
        if (this == &rhs)
        return *this;
        else
        {
        X = rhs.X;
        Y = rhs.Y;
        size = rhs.size;

        mywidget = rhs.mywidget;
        mypixmap = rhs.mypixmap;
        alignment = rhs.alignment;
        homelevel = rhs.homelevel;
        minsize = rhs.minsize;
        maxsize = rhs.maxsize;
}
}
*/
    Point::~Point()
    {
        if(mywidget!=0)
        {
            delete mywidget;
            mywidget = 0;
        }
    }

    void Point::setPixmap( QPixmap qPixmap )
    {
        mypixmap = qPixmap;
        size = mypixmap.size();

        //forces redraw
        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }

    void Point::setVisible(bool visible)
    {
        this->visible = visible;
        if (mywidget !=0)
        {
            mywidget->setVisible(visible);
        }
    }

    QRectF Point::boundingBox()
    {
        qreal minlon=180;
        qreal maxlon=-180;
        qreal minlat=90;
        qreal maxlat=-90;

        if (longitude() < minlon) minlon = longitude();
        if (longitude() > maxlon) maxlon = longitude();
        if (latitude() < minlat) minlat = latitude();
        if (latitude() > maxlat) maxlat = latitude();

        QPointF min = QPointF(minlon, minlat);
        QPointF max = QPointF(maxlon, maxlat);
        QPointF dist = max - min;
        QSizeF si = QSizeF(dist.x(), dist.y());

        return QRectF(min, si);
    }

    qreal Point::longitude() const
    {
        return X;
    }
    qreal Point::latitude() const
    {
        return Y;
    }
    QPointF Point::coordinate() const
    {
        return QPointF(X, Y);
    }

    void Point::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)
    {
        if (!visible)
            return;

        if (homelevel > 0)
        {

            int currentzoom = mapadapter->maxZoom() < mapadapter->minZoom() ? mapadapter->minZoom() - mapadapter->currentZoom() : mapadapter->currentZoom();

            // int currentzoom = mapadapter->getZoom();
            int diffzoom = homelevel-currentzoom;
            int viewheight = size.height();
            int viewwidth = size.width();
            viewheight = int(viewheight / pow(2.0, diffzoom));
            viewwidth = int(viewwidth / pow(2.0, diffzoom));

            if (minsize.height()!= -1 && viewheight < minsize.height())
                viewheight = minsize.height();
            else if (maxsize.height() != -1 && viewheight > maxsize.height())
                viewheight = maxsize.height();


            if (minsize.width()!= -1 && viewwidth < minsize.width())
                viewwidth = minsize.width();
            else if (maxsize.width() != -1 && viewwidth > maxsize.width())
                viewwidth = maxsize.width();

            displaysize = QSize(viewwidth, viewheight);
        }
        else
        {
            displaysize = size;
        }


        if (mypixmap.size().width() > 0)
        {
            const QPointF c = QPointF(X, Y);
            QPoint point = mapadapter->coordinateToDisplay(c);

            if (viewport.contains(point))
            {
                QPoint alignedtopleft = alignedPoint(point);
                painter->drawPixmap(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height(), mypixmap);
            }

        }
        else if (mywidget!=0)
        {
            drawWidget(mapadapter, offset);
        }

    }

    void Point::drawWidget(const MapAdapter* mapadapter, const QPoint offset)
    {
        const QPointF c = QPointF(X, Y);
        QPoint point = mapadapter->coordinateToDisplay(c);
        point -= offset;

        QPoint alignedtopleft = alignedPoint(point);

        if (mywidget!=0)
        {
            mywidget->setGeometry(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height());
        }
    }

    QPoint Point::alignedPoint(const QPoint point) const
    {
        QPoint alignedtopleft;
        if (myalignment == Middle)
        {
            alignedtopleft.setX(point.x()-displaysize.width()/2);
            alignedtopleft.setY(point.y()-displaysize.height()/2);
        }
        else if (myalignment == TopLeft)
        {
            alignedtopleft.setX(point.x());
            alignedtopleft.setY(point.y());
        }
        else if (myalignment == TopRight)
        {
            alignedtopleft.setX(point.x()-displaysize.width());
            alignedtopleft.setY(point.y());
        }
        else if (myalignment == BottomLeft)
        {
            alignedtopleft.setX(point.x());
            alignedtopleft.setY(point.y()-displaysize.height());
        }
        else if (myalignment == BottomRight)
        {
            alignedtopleft.setX(point.x()-displaysize.width());
            alignedtopleft.setY(point.y()-displaysize.height());
        }
        else if (myalignment == BottomMiddle)
        {
            alignedtopleft.setX(point.x()-displaysize.width()/2);
            alignedtopleft.setY(point.y()-displaysize.height());
        }
        else if (myalignment == TopMiddle)
        {
            alignedtopleft.setX(point.x()-displaysize.width()/2);
            alignedtopleft.setY(point.y());
        }
        return alignedtopleft;
    }

    bool Point::Touches(Point* click, const MapAdapter* mapadapter)
    {
        if (this->isVisible() == false)
            return false;

        if ( !click || !mapadapter )
            return false;

        if (points().size() < 1)
        {
            return false;
        }

        QPointF clickPt = mapadapter->coordinateToDisplay(click->coordinate());
        qreal halfwidth = 2; // use 2 pixels by default
        if (mypixmap.width() > 0)
        {
            halfwidth = static_cast<qreal> (mypixmap.width()) / static_cast<qreal> (2);
        }

        QPointF pt1 = mapadapter->coordinateToDisplay(coordinate());
        qreal pt1x1 = pt1.x() - halfwidth;
        qreal pt1x2 = pt1.x() + halfwidth;
        qreal pt1y1 = pt1.y() - halfwidth;
        qreal pt1y2 = pt1.y() + halfwidth;

        QPointF pt2 = mapadapter->coordinateToDisplay(coordinate());
        qreal pt2x1 = pt2.x() - halfwidth;
        qreal pt2x2 = pt2.x() + halfwidth;
        qreal pt2y1 = pt2.y() - halfwidth;
        qreal pt2y2 = pt2.y() + halfwidth;

        // build lazy bounding box
        qreal upperLeftX = qMin(pt1x1, qMin(pt1x2, qMin(pt2x1, pt2x2)));
        qreal upperLeftY = qMin(pt1y1, qMin(pt1y2, qMin(pt2y1, pt2y2)));
        qreal lowerRightX = qMax(pt1x1, qMax(pt1x2, qMax(pt2x1, pt2x2)));
        qreal lowerRightY = qMax(pt1y1, qMax(pt1y2, qMax(pt2y1, pt2y2)));
        QRectF bounds(QPointF(upperLeftX, upperLeftY), QPointF(lowerRightX,
               lowerRightY));

        if ( bounds.contains(clickPt) )
        {
            emit(geometryClicked(this, QPoint(0, 0)));
            return true;
        }

        return false;
    }

    void Point::setCoordinate(QPointF point)
    {
        if ( X == point.x() &&
             Y == point.y() )        
        {
            //no change, prevent unessessary update/redraw
            return;
        }

        X = point.x();
        Y = point.y();
        
        emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
        emit(positionChanged(this));
    }
    QList<Point*> Point::points()
    {
        //TODO: assigning temp?!
        QList<Point*> points;
        points.append(this);
        return points;
    }

    QWidget* Point::widget()
    {
        return mywidget;
    }

    QPixmap Point::pixmap()
    {
        return mypixmap;
    }

    void Point::setBaselevel(int zoomlevel)
    {
        homelevel = zoomlevel;
    }
    void Point::setMinsize(QSize minsize)
    {
        this->minsize = minsize;
    }
    void Point::setMaxsize(QSize maxsize)
    {
        this->maxsize = maxsize;
    }
    Point::Alignment Point::alignment() const
    {
        return myalignment;
    }
}
