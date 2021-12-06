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

#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include "qmapcontrol_global.h"
#include <QObject>
#include <QMap>
#include <QListIterator>
#include <QRectF>
#include "layer.h"
#include "mapadapter.h"
#include "mapcontrol.h"

namespace qmapcontrol
{
    class Layer;
    class MapAdapter;
    class MapControl;

    class LayerManager;

    //! Handles Layers and viewport related settings
    /*!
     * This class handles internally all layers which were added to the MapControl.
     * It also stores values for scrolling.
     * It initiates the creation of a new offscreen image and on zooming the zoom images gets here created.
     *	@author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT LayerManager : public QObject
    {
        Q_OBJECT

    public:
        LayerManager(MapControl*, QSize);
        ~LayerManager();

        //! returns the coordinate of the center of the map
        /*!
         * @return returns the coordinate of the middle of the screen
         */
        QPointF currentCoordinate() const;

        //! returns the current offscreen image
        /*!
         * @return the current offscreen image
         */
        QPixmap getImage() const;

        //! returns the layer with the given name
        /*!
         * @param  layername name of the wanted layer
         * @return the layer with the given name
         */
        Layer* layer(const QString&) const;

        //! returns the base layer
        /*!
         * This will return the base layer of the LayerManager.
         * The base layer is the one which is used to do internal coordinate calculations.
         * @return the base layer
         */
        Layer* layer() const;

        //! returns the names of all layers
        /*!
         * @return returns a QList with the names of all layers
         */
        QList<QString> layers() const;

        //! sets the middle of the map to the given coordinate
        /*!
         * @param  coordinate the coordinate which the view´s middle should be set to
         */
        void setView(const QPointF& coordinate);

        //! sets the view, so all coordinates are visible
        /*!
         * @param  coordinates the Coorinates which should be visible
         */
        void setView(const QList<QPointF> coordinates);

        //! sets the view and zooms in, so all coordinates are visible
        /*!
         * The code of setting the view to multiple coordinates is "brute force" and pretty slow.
         * Have to be reworked.
         * @param  coordinates the Coorinates which should be visible
         */
        void setViewAndZoomIn (const QList<QPointF> coordinates);

        //! zooms in one step
        void zoomIn();

        //! zooms out one step
        void zoomOut();

        //! return min Zoom Level
        int minZoom();

        //! return max Zoom Level
        int maxZoom();

        //! sets the given zoomlevel
        /*!
         * @param zoomlevel the zoomlevel
         */
        void setZoom(int zoomlevel);

        //! The Viewport of the display
        /*!
         * Returns the visible viewport in world coordinates
         * @return the visible viewport in world coordinates
         */
        QRectF getViewport() const;

        //! scrolls the view
        /*!
         * Scrolls the view by the given value in pixels and in display coordinates
         * @param  offset the distance which the view should be scrolled
         */
        void scrollView(const QPoint& offset);

        //! forwards mouseevents to the layers
        /*!
         * This method is invoked by the MapControl which receives Mouse Events.
         * These events are forwarded to the layers, so they can check for clicked geometries.
         * @param  evnt the mouse event
         */
        void mouseEvent(const QMouseEvent* evnt);

        //! returns the middle of the map in projection coordinates
        /*!
         *
         * @return the middle of the map in projection coordinates
         */
        QPoint getMapmiddle_px() const;

        void forceRedraw();
        void removeZoomImage();

        //! adds a layer
        /*!
         * If multiple layers are added, they are painted in the added order.
         * @param layer the layer which should be added
         */
        void addLayer(Layer* layer);

        //! removes a layer
        /*!
         * Removes a layer and redraws existing layers
         * @param layer the layer which should be removed
         */
        void removeLayer( Layer* layer );

        //! returns the current zoom level
        /*!
         * @return returns the current zoom level
         */
        int currentZoom() const;

        void drawGeoms(QPainter* painter);
        void drawImage(QPainter* painter);


        //! Set whether to enable a view bounding box
        /*!
         *
         * @param usebounds enable/disable use of bounding box
         */
        void setUseBoundingBox( bool usebounds );

        //! Check if bounding box is being used
        /*!
         *
         * @return if bounding box is being used
         */
        bool isBoundingBoxEnabled();

        //! Set constraints for bounding box
        /*!
         *
         * @param rect specified bounds for view to stay within
         */
        void setBoundingBox( QRectF &rect );

        //! Get current bounding box
        /*!
         *
         * @return bounding box
         */
        QRectF getBoundingBox();

    private:
        LayerManager& operator=(const LayerManager& rhs);
        LayerManager(const LayerManager& old);
        //! This method have to be invoked to draw a new offscreen image
        /*!
         * @param clearImage if the current offscreeen image should be cleared
         * @param showZoomImage if a zoom image should be painted
         */
        void newOffscreenImage(bool clearImage=true, bool showZoomImage=true);
        inline bool checkOffscreen() const;
        inline bool containsAll(QList<QPointF> coordinates) const;
        inline void moveWidgets();
        inline void setMiddle(QList<QPointF> coordinates);

        MapControl* mapcontrol;
        QPoint screenmiddle; // middle of the screen
        QPoint scroll; // scrollvalue of the offscreen image
        QPoint zoomImageScroll; // scrollvalue of the zoom image

        QSize size; // widget size
        QSize offSize; // size of the offscreen image

        QPixmap composedOffscreenImage;
        QPixmap zoomImage;

        QList<Layer*>	mylayers;

        QPoint mapmiddle_px; // projection-display coordinates
        QPointF mapmiddle; // world coordinate

        QPoint whilenewscroll;

        QRectF boundingBox; // limit viewing area if desired
        bool useBoundingBox;

    public slots:
        void updateRequest(QRectF rect);
        void updateRequest();
        void resize(QSize newSize);
    };
}
#endif
