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

#include "mapnetwork.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QMapIterator>
#include <QWaitCondition>

#include <QMutexLocker>

namespace qmapcontrol
{
    MapNetwork::MapNetwork(ImageManager* parent)
        :   parent(parent), 
            http(new QNetworkAccessManager(this)),
            loaded(0),
            networkActive( false ),
            cacheEnabled(false)
    {
        connect(http, SIGNAL(finished(QNetworkReply *)), this, SLOT(requestFinished(QNetworkReply *)));
    }

    MapNetwork::~MapNetwork()
    {
        foreach(QNetworkReply *reply, replyList)
        {
            if(reply->isRunning())
            {
                reply->abort();
            }
            reply->deleteLater();
            reply = 0;
        }

        http->deleteLater();
        http = 0;
    }

    void MapNetwork::loadImage(const QString& host, const QString& url)
    {
        QString hostName = host;
        QString portNumber = QString("80");

        QRegExp r(".:.");

        if(r.indexIn(host) >= 0)
        {
            QStringList s = host.split(":");

            hostName = s.at(0);
            portNumber = s.at(1);
        }

        QString finalUrl = QString("http://%1:%2%3").arg(hostName).arg(portNumber).arg(url);
        QNetworkRequest request = QNetworkRequest(QUrl(finalUrl));

        if( cacheEnabled )
        {
            // prefer our cached version (if enabled) over fresh network query
            request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
            // Data obtained should be saved to cache for future uses
            request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
        }

        request.setRawHeader("User-Agent", "Mozilla/5.0 (PC; U; Intel; Linux; en) AppleWebKit/420+ (KHTML, like Gecko)");

        QMutexLocker lock(&vectorMutex);
        replyList.append( http->get(request) );
        loadingMap.insert( finalUrl, url );
}

    void MapNetwork::requestFinished(QNetworkReply *reply)
    {
        if (!reply)
        {
            //qDebug() << "MapNetwork::requestFinished - reply no longer valid";
            return;
        }

        //qDebug() << "MapNetwork::requestFinished" << reply->url().toString();
        if (reply->error() == QNetworkReply::NoError)
        {
            QString id = reply->url().toString();
            // check if id is in map?
            bool idInMap = false;
            QString url;
            {
                QMutexLocker lock(&vectorMutex);
                idInMap = loadingMap.contains(id);
                if(idInMap)
                {
                    url = loadingMap[id];
                    loadingMap.remove(id);
                }
            }
          
            if (idInMap)
            {
                //qDebug() << "request finished for reply: " << reply << ", belongs to: " << url << endl;
                QByteArray ax;

                if (reply->bytesAvailable()>0)
                {
                    QPixmap pm;
                    ax = reply->readAll();

                    if (pm.loadFromData(ax) && pm.size().width() > 1 && pm.size().height() > 1)
                    {
                        loaded += pm.size().width()*pm.size().height()*pm.depth()/8/1024;
                        //qDebug() << "Network loaded: " << loaded << " width:" << pm.size().width() << " height:" <<pm.size().height();
                        parent->receivedImage(pm, url);
                    }
                    else
                    {
                        parent->fetchFailed(url);
                    }
                }
            }
        }

        if (loadQueueSize() == 0)
        {
            //qDebug () << "all loaded";
            parent->loadingQueueEmpty();
        }

        QMutexLocker lock(&vectorMutex);
        replyList.removeAll(reply);

        reply->deleteLater();
        reply = 0;
    }

    int MapNetwork::loadQueueSize() const
    {
        QMutexLocker lock(&vectorMutex);
        return loadingMap.size();
    }

    void MapNetwork::setDiskCache(QNetworkDiskCache *qCache)
    {
        cacheEnabled = (qCache != 0);
        if (http)
        {
            http->setCache(qCache);
        }
    }

    void MapNetwork::abortLoading()
    {
        //qDebug() << "MapNetwork::abortLoading";
        // be sure that replyList is copied in case it's modified in another thread
        QListIterator<QNetworkReply *> iter(replyList);
        while(iter.hasNext())
        {
            QNetworkReply *reply = iter.next();
            if (reply)
            {
                if(reply->isRunning())
                {
                    reply->abort();
                }
                reply->deleteLater();
                reply = 0;
            }
        }
        QMutexLocker lock(&vectorMutex);
        replyList.clear();
        loadingMap.clear();
    }

    bool MapNetwork::imageIsLoading(QString url)
    {
        QMutexLocker lock(&vectorMutex);
        return loadingMap.values().contains(url);
    }

    void MapNetwork::setProxy(const QString host, const int port, const QString username, const QString password)
    {
        // do not set proxy on qt/extended
#ifndef Q_WS_QWS
        if (http)
        {
            QNetworkProxy proxy = QNetworkProxy( QNetworkProxy::HttpProxy, host, port );
            proxy.setUser( username );
            proxy.setPassword( password );
            http->setProxy( proxy );
        }
#endif
    }
}
