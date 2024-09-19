/*
 * Copyright (c) 2014-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _QSIMPLEUPDATER_UPDATER_H
#define _QSIMPLEUPDATER_UPDATER_H

#include <QUrl>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <QSimpleUpdater.h>

class Downloader;

/**
 * \brief Downloads and interprests the update definition file
 */
class QSU_SHARED_EXPORT Updater : public QObject
{
  Q_OBJECT

signals:
  void checkingFinished(const QString &url);
  void downloadFinished(const QString &url, const QString &filepath);
  void appcastDownloaded(const QString &url, const QByteArray &data);

public:
  Updater();
  ~Updater();

  QString url() const;
  QString openUrl() const;
  QString changelog() const;
  QString moduleName() const;
  QString downloadUrl() const;
  QString platformKey() const;
  QString moduleVersion() const;
  QString latestVersion() const;
  QString userAgentString() const;
  bool mandatoryUpdate() const;

  bool customAppcast() const;
  bool notifyOnUpdate() const;
  bool notifyOnFinish() const;
  bool updateAvailable() const;
  bool downloaderEnabled() const;
  bool useCustomInstallProcedures() const;

public slots:
  void checkForUpdates();
  void setUrl(const QString &url);
  void setModuleName(const QString &name);
  void setNotifyOnUpdate(const bool notify);
  void setNotifyOnFinish(const bool notify);
  void setUserAgentString(const QString &agent);
  void setModuleVersion(const QString &version);
  void setDownloaderEnabled(const bool enabled);
  void setPlatformKey(const QString &platformKey);
  void setUseCustomAppcast(const bool customAppcast);
  void setUseCustomInstallProcedures(const bool custom);
  void setMandatoryUpdate(const bool mandatory_update);

private slots:
  void onReply(QNetworkReply *reply);
  void setUpdateAvailable(const bool available);

private:
  bool compare(const QString &x, const QString &y);

private:
  QString m_url;
  QString m_userAgentString;

  bool m_customAppcast;
  bool m_notifyOnUpdate;
  bool m_notifyOnFinish;
  bool m_updateAvailable;
  bool m_downloaderEnabled;
  bool m_mandatoryUpdate;

  QString m_openUrl;
  QString m_platform;
  QString m_changelog;
  QString m_moduleName;
  QString m_downloadUrl;
  QString m_moduleVersion;
  QString m_latestVersion;

  Downloader *m_downloader;
  QNetworkAccessManager *m_manager;
};

#endif
