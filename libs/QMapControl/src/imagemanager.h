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

#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "qmapcontrol_global.h"
#include <QObject>
#include <QPixmapCache>
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QBuffer>
#include <QDir>
#include <QNetworkDiskCache>

namespace qmapcontrol
{
    class MapNetwork;
    /**
    @author Kai Winter <kaiwinter@gmx.de>
     */
    class QMAPCONTROL_EXPORT ImageManager : public QObject
    {
        Q_OBJECT

    public:
        ImageManager(QObject* parent = 0);
        virtual ~ImageManager();

        //! returns a QPixmap of the asked image
        /*!
         * If this component doesn´t have the image a network query gets started to load it.
         * @param host the host of the image
         * @param path the path to the image
         * @return the pixmap of the asked image
         */
        QPixmap getImage(const QString& host, const QString& path);

        QPixmap prefetchImage(const QString& host, const QString& path);

        void receivedImage(const QPixmap pixmap, const QString& url);
        void fetchFailed(const QString &url);

        /*!
         * This method is called by MapNetwork, after all images in its queue were loaded.
         * The ImageManager emits a signal, which is used in MapControl to remove the zoom image.
         * The zoom image should be removed on Tile Images with transparency.
         * Else the zoom image stay visible behind the newly loaded tiles.
         */
        void loadingQueueEmpty();

        /*!
         * Aborts all current loading threads.
         * This is useful when changing the zoom-factor, though newly needed images loads faster
         */
        void abortLoading();

        //! sets the proxy for HTTP connections
        /*!
         * This method sets the proxy for HTTP connections.
         * This is not provided by the current Qtopia version!
         * @param host the proxy´s hostname or ip
         * @param port the proxy´s port
         * @param username the proxy´s username
         * @param password the proxy´s password
         */
        void setProxy(QString host, int port, const QString username = QString(), const QString password = QString());

        //! sets the cache directory for persistently saving map tiles
        /*!
         *
         * @param path the path where map tiles should be stored
         * @param qDiskSizeMB the about of disk space to use for caching. Default is 250MB
         */
        void setCacheDir(const QDir& path, const int qDiskSizeMB = 250);

        /*!
         * @return Number of images pending in the load queue
         */
        int loadQueueSize() const;

    private:        
        Q_DISABLE_COPY( ImageManager )

        QPixmap emptyPixmap;
        QPixmap loadingPixmap;

        MapNetwork* net;
        QNetworkDiskCache* diskCache;
        QVector<QString> prefetch;

        QHash<QString,QDateTime> failedFetches;        

    signals:
        void imageReceived();
        void loadingFinished();
    };
}
#endif
