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

#ifndef POINT_H
#define POINT_H

//#include "qglobal.h"
#if QT_VERSION >= 0x050000
    // Qt5 code
    //#include <QtWidgets>
#else
    // Qt4 code
    #include <QWidget>
#endif

#include "qmapcontrol_global.h"
#include "geometry.h"

namespace qmapcontrol
{
    //! A geometric point to draw objects into maps
    /*!
     * This class can be used to draw your custom QPixmap or other QWidgets into maps.
     * You can instantiate a Point with any Pixmap you want. The objects cares about collision detection (for clickable objects)
     *
     * When drawing a pixmap, take care you are adding the point to a GeometryLayer.
     * You can also add a point to a MapLayer, but this should only be done, if the point is not changing its position or color etc.
     * (GeometryLayers are assured to be repainted on any changes at the point. MapLayers only gets repainted, if a new
     * offscreenImage is painter. This is a performance issue.)
     *
     * Points emit click events, if the containing layer receives clickevents (the default)
     *
     * You can also add a widget into maps. But keep in mind, that widgets always are drawn on top of all layers.
     * You also have to handle click events yourself.
     *
     * To create "zoomable objects" (objects that increases size on zooming), a base level have to be set.
     * The base level is the zoom level on which the point´s pixmap gets displayed on full size.
     * On lower zoom levels it gets displayed smaller and on higher zoom levels larger.
     * A minimal size can be set as well as a maximum size.
     * @see setBaselevel, setMinsize, setMaxsize
     *
     * @author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT Point : public Geometry
    {
        Q_OBJECT

    public:
        friend class Layer;
        friend class LineString;

        //! sets where the point should be aligned
        enum Alignment
        {
            TopLeft, /*!< Align on TopLeft*/
            TopRight, /*!< Align on TopRight*/
            TopMiddle, /*!< Align on TopLeft*/
            BottomLeft, /*!< Align on BottomLeft*/
            BottomRight,/*!< Align on BottomRight*/
            BottomMiddle, /*!< Align on BottomMiddle*/
            Middle /*!< Align on Middle*/
        };

        Point();
        explicit Point(const Point&);
        //! Copy Constructor
        /*!
         * This constructor creates a Point with no image or widget.
         * @param x longitude
         * @param y latitude
         * @param name name of the point
         * @param alignment alignment of the point (Middle or TopLeft)
         */
        Point(qreal x, qreal y, QString name = QString(), enum Alignment alignment=Middle);

        //! Constructor
        /*!
         * This constructor creates a point which will display the given widget.
         * You can set an alignment on which corner the widget should be aligned to the coordinate.
         * You have to set the size of the widget, before adding it to
         * IMPORTANT: You have to set the QMapControl as parent for the widget!
         * @param x longitude
         * @param y latitude
         * @param widget the widget which should be displayed by this point
         * @param name name of the point
         * @param alignment allignment of the point (Middle or TopLeft)
         */
        Point(qreal x, qreal y, QWidget* widget, QString name = QString(), enum Alignment alignment = Middle);

        //! Constructor
        /*!
         * This constructor creates a point which will display the give pixmap.
         * You can set an alignment on which corner the pixmap should be aligned to the coordinate.
         * @param x longitude
         * @param y latitude
         * @param pixmap the pixmap which should be displayed by this point
         * @param name name of the point
         * @param alignment allignment of the point (Middle or TopLeft)
         */
        Point(qreal x, qreal y, QPixmap pixmap, QString name = QString(), enum Alignment alignment = Middle);
        virtual ~Point();

        //! returns the bounding box of the point
        /*!
         * The Bounding contains the coordinate of the point and its size.
         * The size is set, if the point contains a pixmap or a widget
         * @return the bounding box of the point
         */
        virtual QRectF boundingBox();

        //! returns the longitude of the point
        /*!
         * @return the longitude of the point
         */
        qreal longitude() const;

        //! returns the latitude of the point
        /*!
         * @return the latitude of the point
         */
        qreal latitude() const;

        //! returns the coordinate of the point
        /*!
         * The x component of the returned QPointF is the longitude value,
         * the y component the latitude
         * @return the coordinate of a point
         */
        QPointF coordinate() const;

        virtual QList<Point*> points();

        /*! \brief returns the widget of the point
        @return the widget of the point
         */
        QWidget* widget();

        //! returns the pixmap of the point
        /*!
         * @return the pixmap of the point
         */
        QPixmap pixmap();

        //! Sets the zoom level on which the points pixmap gets displayed on full size
        /*!
         * Use this method to set a zoom level on which the pixmap gets displayed with its real size.
         * On zoomlevels below it will be displayed smaller, and on zoom levels thereover it will be displayed larger
         * @see setMinsize, setMaxsize
         * @param zoomlevel the zoomlevel on which the point will be displayed on full size
         */
        void setBaselevel(int zoomlevel);

        //! sets a minimal size for the pixmap
        /*!
         * When the point's pixmap should change its size on zooming, this method sets the minimal size.
         * @see setBaselevel
         * @param minsize the minimal size which the pixmap should have
         */
        void setMinsize(QSize minsize);

        //! sets a maximal size for the pixmap
        /*!
         * When the point´s pixmap should change its size on zooming, this method sets the maximal size.
         * @see setBaselevel
         * @param maxsize the maximal size which the pixmap should have
         */
        void setMaxsize(QSize maxsize);

        Point::Alignment alignment() const;

        virtual void setPixmap( QPixmap qPixmap );

    protected:
        qreal X;
        qreal Y;
        QSize size;

        QWidget* mywidget;
        QPixmap mypixmap;
        Alignment myalignment;
        int homelevel;
        QSize displaysize;
        QSize minsize;
        QSize maxsize;


        void drawWidget(const MapAdapter* mapadapter, const QPoint offset);
        // void drawPixmap(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint versch);
        virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset);
        QPoint alignedPoint(const QPoint point) const;

        //! returns true if the given Point touches this Point
        /*!
         * The collision detection checks for the bounding rectangulars.
         * @param geom the other point which should be tested on collision
         * @param mapadapter the mapadapter which is used for calculations
         * @return
         */
        virtual bool Touches(Point* p, const MapAdapter* mapadapter);

    public slots:
        void setCoordinate(QPointF point);
        virtual void setVisible(bool visible);
    };
}
#endif
