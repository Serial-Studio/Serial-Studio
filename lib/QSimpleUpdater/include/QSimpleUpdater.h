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

#ifndef _QSIMPLEUPDATER_MAIN_H
#define _QSIMPLEUPDATER_MAIN_H

#include <QUrl>
#include <QList>
#include <QObject>

#if defined(QSIMPLEUPDATER_BUILD_DLIB)
#  define QSU_SHARED_EXPORT Q_DECL_EXPORT
#elif defined(QSIMPLEUPDATER_USE_DLIB)
#  define QSU_SHARED_EXPORT Q_DECL_IMPORT
#else
#  define QSU_SHARED_EXPORT
#endif

class Updater;

/**
 * \brief Manages the updater instances
 *
 * The \c QSimpleUpdater class manages the updater system and allows for
 * parallel application modules to check for updates and download them.
 *
 * The behavior of each updater can be regulated by specifying the update
 * definitions URL (from where we download the individual update definitions)
 * and defining the desired options by calling the individual "setter"
 * functions (e.g. \c setNotifyOnUpdate()).
 *
 * The \c QSimpleUpdater also implements an integrated downloader.
 * If you need to use a custom install procedure/code, just create a function
 * that is called when the \c downloadFinished() signal is emitted to
 * implement your own install procedures.
 *
 * By default, the downloader will try to open the file as if you opened it
 * from a file manager or a web browser (with the "file:*" url).
 */
class QSU_SHARED_EXPORT QSimpleUpdater : public QObject
{
  Q_OBJECT

signals:
  void checkingFinished(const QString &url);
  void appcastDownloaded(const QString &url, const QByteArray &data);
  void downloadFinished(const QString &url, const QString &filepath);

public:
  static QSimpleUpdater *getInstance();

  bool usesCustomAppcast(const QString &url) const;
  bool getNotifyOnUpdate(const QString &url) const;
  bool getNotifyOnFinish(const QString &url) const;
  bool getUpdateAvailable(const QString &url) const;
  bool getDownloaderEnabled(const QString &url) const;
  bool usesCustomInstallProcedures(const QString &url) const;

  QString getOpenUrl(const QString &url) const;
  QString getChangelog(const QString &url) const;
  QString getModuleName(const QString &url) const;
  QString getDownloadUrl(const QString &url) const;
  QString getPlatformKey(const QString &url) const;
  QString getLatestVersion(const QString &url) const;
  QString getModuleVersion(const QString &url) const;
  QString getUserAgentString(const QString &url) const;

public slots:
  void checkForUpdates(const QString &url);
  void setModuleName(const QString &url, const QString &name);
  void setNotifyOnUpdate(const QString &url, const bool notify);
  void setNotifyOnFinish(const QString &url, const bool notify);
  void setPlatformKey(const QString &url, const QString &platform);
  void setModuleVersion(const QString &url, const QString &version);
  void setDownloaderEnabled(const QString &url, const bool enabled);
  void setUserAgentString(const QString &url, const QString &agent);
  void setUseCustomAppcast(const QString &url, const bool customAppcast);
  void setUseCustomInstallProcedures(const QString &url, const bool custom);
  void setMandatoryUpdate(const QString &url, const bool mandatory_update);

protected:
  QSimpleUpdater();
  ~QSimpleUpdater();

private:
  Updater *getUpdater(const QString &url) const;
};

#endif
