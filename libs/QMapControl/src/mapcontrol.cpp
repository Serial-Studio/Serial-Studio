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

#include "mapcontrol.h"
#include <QTimer>

namespace qmapcontrol
{
MapControl::MapControl(QWidget *parent, Qt::WindowFlags windowFlags)
    : QFrame(parent, windowFlags)
    , m_layermanager(0)
    , m_imagemanager(0)
    , size(100, 100)
    , mouse_wheel_events(true)
    , mousepressed(false)
    , mymousemode(Panning)
    , scaleVisible(false)
    , crosshairsVisible(true)
    , m_loadingFlag(false)
    , steps(0)
    , m_doubleBuffer(0)
{
    __init();
}

MapControl::MapControl(QSize size, MouseMode mousemode, bool showScale,
                       bool showCrosshairs, QWidget *parent, Qt::WindowFlags windowFlags)
    : QFrame(parent, windowFlags)
    , m_layermanager(0)
    , m_imagemanager(0)
    , size(size)
    , mouse_wheel_events(true)
    , mousepressed(false)
    , mymousemode(mousemode)
    , scaleVisible(showScale)
    , crosshairsVisible(showCrosshairs)
    , m_loadingFlag(false)
    , steps(0)
    , m_doubleBuffer(0)
{
    __init();
}

MapControl::~MapControl()
{
    if (m_layermanager)
    {
        m_layermanager->deleteLater();
        m_layermanager = 0;
    }

    if (m_imagemanager)
    {
        m_imagemanager->deleteLater();
        m_imagemanager = 0;
    }
}

void MapControl::__init()
{
    m_layermanager = new LayerManager(this, size);
    m_imagemanager = new ImageManager(this);
    screen_middle = QPoint(size.width() / 2, size.height() / 2);

    mousepressed = false;

    connect(m_imagemanager, SIGNAL(imageReceived()), this, SLOT(updateRequestNew()));

    connect(m_imagemanager, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()));

    this->setMaximumSize(size.width() + 1, size.height() + 1);
    mouse_wheel_events = true;
}

void MapControl::enableMouseWheelEvents(bool enabled)
{
    mouse_wheel_events = enabled;
}

bool MapControl::mouseWheelEventsEnabled()
{
    return mouse_wheel_events;
}

QPointF MapControl::currentCoordinate() const
{
    return m_layermanager->currentCoordinate();
}

Layer *MapControl::layer(const QString &layername) const
{
    return m_layermanager->layer(layername);
}

QList<QString> MapControl::layers() const
{
    return m_layermanager->layers();
}

int MapControl::numberOfLayers() const
{
    return m_layermanager->layers().size();
}

void MapControl::followGeometry(const Geometry *geom) const
{
    if (geom == 0)
    {
        return;
    }

    // ensures only one signal is ever connected
    stopFollowing(geom);

    connect(geom, SIGNAL(positionChanged(Geometry *)), this,
            SLOT(positionChanged(Geometry *)));
}

void MapControl::positionChanged(Geometry *geom)
{
    if (!m_layermanager->layer() || !m_layermanager->layer()->mapadapter())
    {
        qDebug() << "MapControl::positionChanged() - no layers configured";
        return;
    }

    Point *point = dynamic_cast<Point *>(geom);
    if (point != 0)
    {
        QPoint start = m_layermanager->layer()->mapadapter()->coordinateToDisplay(
            currentCoordinate());
        QPoint dest = m_layermanager->layer()->mapadapter()->coordinateToDisplay(
            point->coordinate());
        QPoint step = (dest - start);
        m_layermanager->scrollView(step);
        updateRequestNew();
    }
}

void MapControl::moveTo(QPointF coordinate)
{
    target = coordinate;
    steps = 25;
    if (moveMutex.tryLock())
    {
        QTimer::singleShot(40, this, SLOT(tick()));
    }
    else
    {
        // stopMove(coordinate);
        moveMutex.unlock();
    }
}
void MapControl::tick()
{
    if (!m_layermanager->layer() || !m_layermanager->layer()->mapadapter())
    {
        qDebug() << "MapControl::tick() - no layers configured";
        return;
    }

    QPoint start
        = m_layermanager->layer()->mapadapter()->coordinateToDisplay(currentCoordinate());
    QPoint dest = m_layermanager->layer()->mapadapter()->coordinateToDisplay(target);

    QPoint step = (dest - start) / steps;
    m_layermanager->scrollView(step);

    update();
    m_layermanager->updateRequest();
    steps--;
    if (steps > 0)
    {
        QTimer::singleShot(50, this, SLOT(tick()));
    }
    else
    {
        moveMutex.unlock();
    }
}

void MapControl::paintEvent(QPaintEvent *evnt)
{
    Q_UNUSED(evnt);

    if (m_doubleBuffer == 0)
    {
        m_doubleBuffer = new QPixmap(width(), height());
    }
    // check for resize change
    else if (m_doubleBuffer->width() != width() || m_doubleBuffer->height() != height())
    {
        delete m_doubleBuffer;
        m_doubleBuffer = new QPixmap(width(), height());
    }

    QPainter dbPainter;
    dbPainter.begin(m_doubleBuffer);

    m_layermanager->drawImage(&dbPainter);
    m_layermanager->drawGeoms(&dbPainter);

    // draw scale
    if (scaleVisible)
    {
        static QList<double> distanceList;
        if (distanceList.isEmpty())
        {
            distanceList << 5000000 << 2000000 << 1000000 << 1000000 << 1000000 << 100000
                         << 100000 << 50000 << 50000 << 10000 << 10000 << 10000 << 1000
                         << 1000 << 500 << 200 << 100 << 50 << 25;
        }

        if (currentZoom() >= m_layermanager->minZoom()
            && distanceList.size() > currentZoom())
        {
            double line;
            line = distanceList.at(currentZoom()) / pow(2.0, 18 - currentZoom())
                / 0.597164;

            // draw the scale
            dbPainter.setPen(Qt::black);
            QPoint p1(10, size.height() - 20);
            QPoint p2((int)line, size.height() - 20);
            dbPainter.drawLine(p1, p2);

            dbPainter.drawLine(10, size.height() - 15, 10, size.height() - 25);
            dbPainter.drawLine((int)line, size.height() - 15, (int)line,
                               size.height() - 25);

            QString distance;
            if (distanceList.at(currentZoom()) >= 1000)
            {
                distance
                    = QVariant(distanceList.at(currentZoom()) / 1000).toString() + " km";
            }
            else
            {
                distance = QVariant(distanceList.at(currentZoom())).toString() + " m";
            }

            dbPainter.drawText(QPoint((int)line + 10, size.height() - 15), distance);
        }
    }

    if (crosshairsVisible)
    {
        auto oldPen = dbPainter.pen();
        dbPainter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
        dbPainter.drawLine(screen_middle.x(), screen_middle.y() - 15, screen_middle.x(),
                           screen_middle.y() + 15); // |
        dbPainter.drawLine(screen_middle.x() - 15, screen_middle.y(),
                           screen_middle.x() + 15, screen_middle.y()); // -
        dbPainter.drawEllipse(screen_middle.x() - 10, screen_middle.y() - 10, 20, 20);
        dbPainter.setPen(oldPen);
    }

    dbPainter.drawRect(0, 0, size.width(), size.height());

    if (mousepressed && mymousemode == Dragging)
    {
        QRect rect = QRect(pre_click_px, current_mouse_pos);
        dbPainter.drawRect(rect);
    }
    dbPainter.end();
    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(rect(), *m_doubleBuffer, m_doubleBuffer->rect());
    painter.end();
}

// mouse events
void MapControl::mousePressEvent(QMouseEvent *evnt)
{
    m_layermanager->mouseEvent(evnt);

    if (m_layermanager->layers().size() > 0)
    {
        if (evnt->button() == 1)
        {
            mousepressed = true;
            pre_click_px = QPoint(evnt->x(), evnt->y());
        }
        else if (evnt->button() == 2 && mouseWheelEventsEnabled()
                 && mymousemode != None) // zoom in
        {
            zoomIn();
        }
        else if (evnt->button() == 4 && mouseWheelEventsEnabled()
                 && mymousemode != None) // zoom out
        {
            zoomOut();
        }
    }

    // emit(mouseEvent(evnt));
    emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));
}

void MapControl::mouseReleaseEvent(QMouseEvent *evnt)
{
    mousepressed = false;
    if (mymousemode == Dragging)
    {
        QPointF ulCoord = clickToWorldCoordinate(pre_click_px);
        QPointF lrCoord = clickToWorldCoordinate(current_mouse_pos);

        QRectF coordinateBB
            = QRectF(ulCoord, QSizeF((lrCoord - ulCoord).x(), (lrCoord - ulCoord).y()));

        emit(boxDragged(coordinateBB));
    }

    emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));
}

void MapControl::mouseMoveEvent(QMouseEvent *evnt)
{
    if (mousepressed && mymousemode == Panning)
    {
        QPoint offset = pre_click_px - QPoint(evnt->x(), evnt->y());
        m_layermanager->scrollView(offset);
        pre_click_px = QPoint(evnt->x(), evnt->y());
    }
    else if (mousepressed && mymousemode == Dragging)
    {
        current_mouse_pos = QPoint(evnt->x(), evnt->y());
    }

    update();
}

void MapControl::wheelEvent(QWheelEvent *evnt)
{
    (void) evnt;
    /*if(mouse_wheel_events && evnt->angleDelta() )
    {
        if(evnt->delta() > 0)
        {
            if( currentZoom() == m_layermanager->maxZoom() )
            {
                return;
            }

            setView(clickToWorldCoordinate(evnt->pos())); //zoom in under mouse cursor
            zoomIn();
        }
        else
        {
            if( currentZoom() == m_layermanager->minZoom() )
            {
                return;
            }
            zoomOut();
        }
        evnt->accept();
    }
    else
    {
        evnt->ignore();
    }*/
}

ImageManager *MapControl::getImageManager()
{
    return m_imagemanager;
}

QPointF MapControl::clickToWorldCoordinate(QPoint click)
{
    if (!m_layermanager->layer() || !m_layermanager->layer()->mapadapter())
    {
        qDebug() << "MapControl::clickToWorldCoordinate() - no layers configured";
        return QPointF();
    }
    // click coordinate to image coordinate
    QPoint displayToImage
        = QPoint(click.x() - screen_middle.x() + m_layermanager->getMapmiddle_px().x(),
                 click.y() - screen_middle.y() + m_layermanager->getMapmiddle_px().y());

    // image coordinate to world coordinate
    return m_layermanager->layer()->mapadapter()->displayToCoordinate(displayToImage);
}

void MapControl::updateRequest(QRect rect)
{
    update(rect);
}

void MapControl::updateRequestNew()
{
    m_layermanager->forceRedraw();
}

// slots
void MapControl::zoomIn()
{
    m_layermanager->zoomIn();
    updateView();
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::zoomOut()
{
    m_layermanager->zoomOut();
    updateView();
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setZoom(int zoomlevel)
{
    m_layermanager->setZoom(zoomlevel);
    updateView();
    emit viewChanged(currentCoordinate(), currentZoom());
}

int MapControl::currentZoom() const
{
    return m_layermanager->currentZoom();
}

void MapControl::scrollLeft(int pixel)
{
    m_layermanager->scrollView(QPoint(-pixel, 0));
    updateView();
}

void MapControl::scrollRight(int pixel)
{
    m_layermanager->scrollView(QPoint(pixel, 0));
    updateView();
}

void MapControl::scrollUp(int pixel)
{
    m_layermanager->scrollView(QPoint(0, -pixel));
    updateView();
}

void MapControl::scrollDown(int pixel)
{
    m_layermanager->scrollView(QPoint(0, pixel));
    updateView();
}

void MapControl::scroll(const QPoint scroll)
{
    m_layermanager->scrollView(scroll);
    updateView();
}

void MapControl::updateView() const
{
    m_layermanager->setView(currentCoordinate());
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setView(const QPointF &coordinate) const
{
    m_layermanager->setView(coordinate);
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setView(const QList<QPointF> coordinates) const
{
    m_layermanager->setView(coordinates);
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setViewAndZoomIn(const QList<QPointF> coordinates) const
{
    m_layermanager->setViewAndZoomIn(coordinates);
    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setView(const Point *point) const
{
    m_layermanager->setView(point->coordinate());
}

void MapControl::loadingFinished()
{
    m_layermanager->removeZoomImage();
}

void MapControl::addLayer(Layer *layer)
{
    layer->setImageManager(m_imagemanager);
    m_layermanager->addLayer(layer);
    update();
}

void MapControl::removeLayer(Layer *layer)
{
    disconnect(layer, 0, 0, 0);
    m_layermanager->removeLayer(layer);
    update();
}

void MapControl::setMouseMode(MouseMode mousemode)
{
    mymousemode = mousemode;
}

MapControl::MouseMode MapControl::mouseMode()
{
    return mymousemode;
}

void MapControl::stopFollowing(const Geometry *geom) const
{
    disconnect(geom, SIGNAL(positionChanged(Geometry *)), this,
               SLOT(positionChanged(Geometry *)));
}

void MapControl::enablePersistentCache(const QDir &path, const int qDiskSizeMB)
{
    m_imagemanager->setCacheDir(path, qDiskSizeMB);
}

void MapControl::setProxy(QString host, int port, const QString username,
                          const QString password)
{
    m_imagemanager->setProxy(host, port, username, password);
}

void MapControl::showScale(bool visible)
{
    scaleVisible = visible;
}

void MapControl::showCrosshairs(bool visible)
{
    crosshairsVisible = visible;
}

void MapControl::resize(const QSize newSize)
{
    this->size = newSize;
    screen_middle = QPoint(newSize.width() / 2, newSize.height() / 2);

    this->setMaximumSize(newSize.width() + 1, newSize.height() + 1);
    m_layermanager->resize(newSize);

    QFrame::resize(newSize);

    emit viewChanged(currentCoordinate(), currentZoom());
}

void MapControl::setUseBoundingBox(bool usebounds)
{
    if (m_layermanager)
        m_layermanager->setUseBoundingBox(usebounds);
}

bool MapControl::isBoundingBoxEnabled()
{
    if (m_layermanager)
        return m_layermanager->isBoundingBoxEnabled();
    return false;
}

void MapControl::setBoundingBox(QRectF &rect)
{
    if (m_layermanager)
        m_layermanager->setBoundingBox(rect);
}

QRectF MapControl::getBoundingBox()
{
    if (m_layermanager)
        return m_layermanager->getBoundingBox();

    // Return an empty QRectF if there is no m_layermanager
    return QRectF();
}

QRectF MapControl::getViewport()
{
    if (m_layermanager)
        return m_layermanager->getViewport();

    // Return an empty QRectF if there is no m_layermanager
    return QRectF();
}

bool MapControl::isGeometryVisible(Geometry *geometry)
{
    if (!geometry || getViewport() == QRectF())
        return false;

    return getViewport().contains(geometry->boundingBox());
}

int MapControl::loadingQueueSize()
{
    return m_imagemanager->loadQueueSize();
}

}
