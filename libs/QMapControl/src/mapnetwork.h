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

#ifndef MAPNETWORK_H
#define MAPNETWORK_H

#include "qmapcontrol_global.h"
#include <QObject>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QVector>
#include <QPixmap>
#include <QMutex>
#include "imagemanager.h"

/**
        @author Kai Winter <kaiwinter@gmx.de>
 */
namespace qmapcontrol
{
    class ImageManager;
    class QMAPCONTROL_EXPORT MapNetwork : QObject
    {
        Q_OBJECT

    public:
        MapNetwork(ImageManager* parent);
        ~MapNetwork();

        void loadImage(const QString& host, const QString& url);

        /*!
         * checks if the given url is already loading
         * @param url the url of the image
         * @return boolean, if the image is already loading
         */
        bool imageIsLoading(QString url);

        /*!
         * Aborts all current loading threads.
         * This is useful when changing the zoom-factor, though newly needed images loads faster
         */
        void abortLoading();
        void setProxy(QString host, int port, const QString username = QString(), const QString password = QString());

        /*!
        *
        * @return number of elements in the load queue
        */
        int loadQueueSize() const;

        /*!
        *
        * @return next free http downloader thread
        */
        QNetworkAccessManager* nextFreeHttp();

        /*!
         * sets the disk cache for each network manager
         * @param qCache the disk cache object to set
         */
        void setDiskCache( QNetworkDiskCache* qCache );

    private:
        Q_DISABLE_COPY (MapNetwork)

        ImageManager* parent;
        QNetworkAccessManager* http;
        QList<QNetworkReply*> replyList;
        QMap<QString, QString> loadingMap;
        qreal loaded;
        mutable QMutex vectorMutex;
        bool    networkActive;
        bool    cacheEnabled;

    private slots:
        void requestFinished(QNetworkReply *reply);
    };
}
#endif
