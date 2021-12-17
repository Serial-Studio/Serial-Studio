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

#ifndef MAPCONTROL_H
#define MAPCONTROL_H

#include "qmapcontrol_global.h"
#include "layermanager.h"
#include "layer.h"
#include "mapadapter.h"
#include "geometry.h"
#include "imagemanager.h"

#include <QWidget>
#include <QFrame>
#include <QDir>

//! QMapControl namespace
namespace qmapcontrol
{
class LayerManager;
class MapAdapter;
class Layer;

//! The control element of the widget and also the widget itself
/*!
 * This is the main widget.
 * To this control layers can be added.
 * A MapControl has to be instantiated with a QSize which sets the size the widget takes
 * in a layout. The given size is also the size which is assured to be filled with map
 * images.
 *
 * @author Kai Winter <kaiwinter@gmx.de>
 */
class QMAPCONTROL_EXPORT MapControl : public QFrame
{
    Q_OBJECT

public:
    //! Declares what actions mouse movements have on the map
    enum MouseMode
    {
        Panning, /*!< The map is moved */
        Dragging, /*!< A rectangular can be drawn */
        None /*!< Mouse move events have no efect to the map */
    };

    //! The QWidget constructor of MapControl for use with QtDesigner
    /*!
     * @param qParent QWidget parent
     * @param windowFlags QWidget Window flags
     */
    MapControl(QWidget *parent = 0, Qt::WindowFlags windowFlags = Qt::WindowFlags());

    //! The constructor of MapControl
    /*!
     * The MapControl is the widget which displays the maps.
     * The size describes the area which gets filled with map data
     * When you give no MouseMode, the mouse moves the map.
     * You can change the MouseMode at runtime, e.g. to Dragging, which lets the user drag
     * a rectangular box. After the dragging a signal with the size of the box is emitted.
     * The MouseMode ´None´ can be used, to completely define the control of the map
     * yourself.
     * @param size the size which the widget should fill with map data
     * @param mousemode the way mouse events are handled
     * @param showScale true if the scale should be displayed
     * @param showCrossHairs true if crosshairs should be shown at the centre of the map
     * @param parent QWidget parent
     * @param windowFlags QWidget Window flags
     */
    MapControl(QSize size, MouseMode mousemode = Panning, bool showScale = false,
               bool showCrosshairs = true, QWidget *parent = 0,
               Qt::WindowFlags windowFlags = Qt::WindowFlags());

    ~MapControl();

    //! adds a layer
    /*!
     * If multiple layers are added, they are painted in the added order.
     * @param layer the layer which should be added
     */
    void addLayer(Layer *layer);

    //! removes a layer
    /*!
     * Removes a layer and redraws existing layers
     * @param layer the layer which should be removed
     */
    void removeLayer(Layer *layer);

    //! returns the layer with the given name
    /*!
     * @param  layername name of the wanted layer
     * @return the layer with the given name
     */
    Layer *layer(const QString &layername) const;

    //! returns the names of all layers
    /*!
     * @return returns a QList with the names of all layers
     */
    QList<QString> layers() const;

    //! returns the number of existing layers
    /*!
     * @return returns the number of existing layers
     */
    int numberOfLayers() const;

    //! returns the coordinate of the center of the map
    /*!
     * @return returns the coordinate of the middle of the screen
     */
    QPointF currentCoordinate() const;

    //! returns the current zoom level
    /*!
     * @return returns the current zoom level
     */
    int currentZoom() const;

    //! update screen
    void updateView() const;

    //! enable mouse wheel events
    /*!
     * @parm enable mouse event
     */
    void enableMouseWheelEvents(bool enabled = true);

    //! returns mouse wheel allowed
    /*!
     * @return mouse wheel events enabled
     */
    bool mouseWheelEventsEnabled();

    //! sets the middle of the map to the given coordinate
    /*!
     * @param  coordinate the coordinate which the view´s middle should be set to
     */
    void setView(const QPointF &coordinate) const;

    //! sets the view, so all coordinates are visible
    /*!
     * @param  coordinates the Coorinates which should be visible
     */
    void setView(const QList<QPointF> coordinates) const;

    //! sets the view and zooms in, so all coordinates are visible
    /*!
     * The code of setting the view to multiple coordinates is "brute force" and pretty
     * slow. Have to be reworked.
     * @param  coordinates the Coorinates which should be visible
     */
    void setViewAndZoomIn(const QList<QPointF> coordinates) const;

    //! sets the view to the given Point
    /*!
     *
     * @param point the geometric point the view should be set to
     */
    void setView(const Point *point) const;

    //! Keeps the center of the map on the Geometry, even when it moves
    /*!
     * To stop the following the method stopFollowing() have to be called
     * @param  geometry the Geometry which should stay centered.
     */
    void followGeometry(const Geometry *geometry) const;

    // TODO:
    // void followGeometry(const QList<Geometry*>) const;

    //! Stops the following of a Geometry
    /*!
     * if the view is set to follow a Geometry this method stops the trace.
     * See followGeometry().
     * @param geometry the Geometry which should not followed anymore
     */
    void stopFollowing(const Geometry *geometry) const;

    //! Smoothly moves the center of the view to the given Coordinate
    /*!
     * @param  coordinate the Coordinate which the center of the view should moved to
     */
    void moveTo(QPointF coordinate);

    //! sets the Mouse Mode of the MapControl
    /*!
     * There are three MouseModes declard by an enum.
     * The MouesMode Dragging draws an rectangular in the map while the MouseButton is
     * pressed. When the Button is released a boxDragged() signal is emitted.
     *
     * The second MouseMode (the default) is Panning, which allows to drag the map around.
     * @param mousemode the MouseMode
     */
    void setMouseMode(MouseMode mousemode);

    //! returns the current MouseMode
    /*!
     * For a explanation for the MouseModes see setMouseMode()
     * @return the current MouseMode
     */
    MapControl::MouseMode mouseMode();

    //! Enable persistent caching of map tiles
    /*!
     * Call this method to allow the QMapControl widget to save map tiles
     * persistent (also over application restarts).
     * Tiles are stored in the subdirectory "QMapControl.cache" within the
     * user's home directory. This can be changed by giving a path.
     * @param path the path to the cache directory
     * @param tileExpiry how long to keep in cache before requesting a new image. 0 or -1
     * to disable and keep forever
     */
    void enablePersistentCache(const QDir &path = QDir::homePath() + "/QMapControl.cache",
                               const int qDiskSizeMB = 250);

    //! Sets the proxy for HTTP connections
    /*!
     * This method sets the proxy for HTTP connections.
     * This is not provided by the current Qtopia version!
     * @param host the proxy´s hostname or ip
     * @param port the proxy´s port
     * @param username the proxy´s username
     * @param password the proxy´s password
     */
    void setProxy(QString host, int port, const QString username = QString(),
                  const QString password = QString());

    //! Displays the scale within the widget
    /*!
     *
     * @param show true if the scale should be displayed
     */
    void showScale(bool visible);

    void showCrosshairs(bool visible);

    //! Set whether to enable a view bounding box
    /*!
     *
     * @param usebounds enable/disable use of bounding box
     */
    void setUseBoundingBox(bool usebounds);

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
    void setBoundingBox(QRectF &rect);

    //! Get current bounding box
    /*!
     *
     * @return bounding box
     */
    QRectF getBoundingBox();

    //! Get viewport rect
    /*!
     *
     * @return view port rect
     */
    QRectF getViewport();

    //! returns if a geometry is visible on viewport
    /*!
     *
     * @param geometry the Geometry object to check
     * @return true if geometry is visible
     */
    bool isGeometryVisible(Geometry *geometry);

    //! returns loading images queue size
    /*!
     *
     * @return int pending loading images (queue size)
     */
    int loadingQueueSize();

    //! returns the imagemanager pointer
    /*!
     *
     * @return imagemanager pointer
     */
    ImageManager *getImageManager();

private:
    void __init();
    LayerManager *m_layermanager;
    ImageManager *m_imagemanager;

    QPoint screen_middle; // middle of the widget (half size)

    QPoint pre_click_px; // used for scrolling (MouseMode Panning)
    QPoint
        current_mouse_pos; // used for scrolling and dragging (MouseMode Panning/Dragging)

    QSize size; // size of the widget

    bool mouse_wheel_events;
    bool mousepressed;
    MouseMode mymousemode;
    bool scaleVisible;
    bool crosshairsVisible;

    bool m_loadingFlag;

    QMutex moveMutex; // used for method moveTo()
    QPointF target; // used for method moveTo()
    int steps; // used for method moveTo()

    QPixmap *m_doubleBuffer;

    QPointF clickToWorldCoordinate(QPoint click);

    Q_DISABLE_COPY(MapControl)

protected:
    void paintEvent(QPaintEvent *evnt);
    void mousePressEvent(QMouseEvent *evnt);
    void mouseReleaseEvent(QMouseEvent *evnt);
    void mouseMoveEvent(QMouseEvent *evnt);
    void wheelEvent(QWheelEvent *evnt);

signals:
    // void mouseEvent(const QMouseEvent* evnt);

    //! Emitted AFTER a MouseEvent occured
    /*!
     * This signals allows to receive click events within the MapWidget together with the
     * world coordinate. It is emitted on MousePressEvents and MouseReleaseEvents. The
     * kind of the event can be obtained by checking the events type.
     * @param  evnt the QMouseEvent that occured
     * @param  coordinate the corresponding world coordinate
     */
    void mouseEventCoordinate(const QMouseEvent *evnt, const QPointF coordinate);

    //! Emitted, after a Rectangular is dragged.
    /*!
     * It is possible to select a rectangular area in the map, if the MouseMode is set to
     * Dragging. The coordinates are in world coordinates
     * @param  QRectF the dragged Rect
     */
    void boxDragged(const QRectF);

    //! This signal is emitted, when a Geometry is clicked
    /*!
     * @param geometry The clicked Geometry object
     * @param coord_px  The coordinate in pixel coordinates
     */
    void geometryClicked(qmapcontrol::Geometry *geometry, QPoint coord_px);

    //! This signal is emitted, after the view have changed
    /*!
     * @param coordinate The current coordinate
     * @param zoom The current zoom
     */
    void viewChanged(const QPointF &coordinate, int zoom) const;

public slots:
    //! zooms in one step
    void zoomIn();

    //! zooms out one step
    void zoomOut();

    //! sets the given zoomlevel
    /*!
     * @param zoomlevel the zoomlevel
     */
    void setZoom(int zoomlevel);

    //! scrolls the view to the left
    void scrollLeft(int pixel = 10);

    //! scrolls the view to the right
    void scrollRight(int pixel = 10);

    //! scrolls the view up
    void scrollUp(int pixel = 10);

    //! scrolls the view down
    void scrollDown(int pixel = 10);

    //! scrolls the view by the given point
    void scroll(const QPoint scroll);

    //! updates the map for the given rect
    /*!
     * @param rect the area which should be repainted
     */
    void updateRequest(QRect rect);

    //! updates the hole map by creating a new offscreen image
    /*!
     *
     */
    void updateRequestNew();

    //! Resizes the map to the given size
    /*!
     * @param newSize The new size
     */
    void resize(const QSize newSize);

private slots:
    void tick();
    void loadingFinished();
    void positionChanged(qmapcontrol::Geometry *geom);
};
}
#endif
