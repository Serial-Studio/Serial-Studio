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

#include "imagemanager.h"
#include "mapnetwork.h"
#include <QCryptographicHash>
#include <QPainter>
#include <QDateTime>

static const int kDefaultTimeoutDelaySecs = 30;
static const int kDefaultPixmapCacheSizeKB = 20000;

namespace qmapcontrol
{
    ImageManager::ImageManager(QObject* parent)
            : QObject(parent),
              emptyPixmap(QPixmap(1,1)),
              loadingPixmap(QPixmap(256,256)),
              net(new MapNetwork(this)),
              diskCache( new QNetworkDiskCache(this))
    {
        emptyPixmap.fill(Qt::transparent);
        
        //initialize loading image
        loadingPixmap.fill( Qt::transparent );
        QPainter paint(&loadingPixmap);
        QBrush brush( Qt::lightGray, Qt::Dense5Pattern );
        paint.fillRect(loadingPixmap.rect(), brush );
        paint.end();

        if (QPixmapCache::cacheLimit() <= kDefaultPixmapCacheSizeKB)
        {
            QPixmapCache::setCacheLimit(kDefaultPixmapCacheSizeKB);
        }
    }

    ImageManager::~ImageManager()
    {
        delete net;
        net = 0;
    }

    QPixmap ImageManager::getImage(const QString& host, const QString& url)
    {
        //qDebug() << "ImageManager::getImage";
        QPixmap pm;

        if ( net->imageIsLoading(url) )
        {
            //currently loading an image
            return loadingPixmap;
        }
        else if ( QPixmapCache::find(url, &pm) )
        {
           //image found in cache, use this version
            return pm;
        }
        //is image cached (memory)
        else if ( QPixmapCache::find(url, &pm) &&
                  !pm.isNull() )
        {
            //we had a valid copy cached in memory (not disk) so return this
            return pm;
        }
        else if ( failedFetches.contains(url) &&
                  failedFetches[url].secsTo(QDateTime::currentDateTime()) < kDefaultTimeoutDelaySecs )
        {
            //prevents spamming public servers when requests fail to return an image or server returns error code (busy/ivalid useragent etc)
            qDebug() << "Ignored: " << url << " - last request failed less than 30 seconds ago";
        }
        else
        {
            //load from net, add empty image
            net->loadImage(host, url);
        }
        return emptyPixmap;
    }

    QPixmap ImageManager::prefetchImage(const QString& host, const QString& url)
    {
        // TODO See if this actually helps on the N900 & Symbian Phones
        #if defined Q_WS_QWS || defined Q_WS_MAEMO_5 || defined Q_WS_S60
            // on mobile devices we don´t want the display refreshing when tiles are received which are
            // prefetched... This is a performance issue, because mobile devices are very slow in
            // repainting the screen
            prefetch.append(url);
        #endif
        return getImage(host, url);
    }

    void ImageManager::receivedImage(const QPixmap pixmap, const QString& url)
    {
        //qDebug() << "ImageManager::receivedImage";
        QPixmapCache::insert(url, pixmap);

        //remove from failed list (if exists) as it has now come good
        if (failedFetches.contains(url))
        {
            failedFetches.remove(url);
        }

        if (!prefetch.contains(url))
        {
            emit imageReceived();
        }
        else
        {
            #if defined Q_WS_QWS || defined Q_WS_MAEMO_5 || defined Q_WS_S60
                prefetch.remove(prefetch.indexOf(url));
            #endif
        }
    }

    void ImageManager::loadingQueueEmpty()
    {
        emit loadingFinished();
    }

    void ImageManager::abortLoading()
    {
        net->abortLoading();
    }

    void ImageManager::setProxy(QString host, int port, const QString username, const QString password)
    {
        net->setProxy(host, port, username, password);
    }

    void ImageManager::setCacheDir(const QDir& path, const int qDiskSizeMB)
    {
        if ( !path.absolutePath().isEmpty() )
        {
            QDir cacheDir = path;
            if (!cacheDir.exists())
            {
                cacheDir.mkpath( cacheDir.absolutePath() );
            }
            diskCache->setCacheDirectory( cacheDir.absolutePath() );
            diskCache->setMaximumCacheSize( qDiskSizeMB *1024*1024 ); //Megabytes to bytes
            net->setDiskCache(diskCache);
        }
        else
        {
            net->setDiskCache(0);
        }
    }

    int ImageManager::loadQueueSize() const
    {
        return net->loadQueueSize();
    }

    void qmapcontrol::ImageManager::fetchFailed(const QString &url)
    {
        qDebug() << "ImageManager::fetchFailed" << url;

        //store current time for this failed image to prevent loading it again until
        failedFetches.insert(url, QDateTime::currentDateTime());
    }

}
