/*
 * Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
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

#include "QSimpleUpdater.h"

#include <QRegularExpression>

#include "Updater.h"

static QList<QString> URLS;
static QList<Updater*> UPDATERS;

/**
 * @brief Destroys the QSimpleUpdater instance and cleans up all managed
 *        Updater objects.
 */
QSimpleUpdater::~QSimpleUpdater()
{
  URLS.clear();

  for (Updater* updater : UPDATERS)
    updater->deleteLater();

  UPDATERS.clear();
}

/**
 * @brief Returns the singleton instance of QSimpleUpdater.
 * @return Pointer to the global QSimpleUpdater instance.
 */
QSimpleUpdater* QSimpleUpdater::getInstance()
{
  static QSimpleUpdater updater;
  return &updater;
}

/**
 * @brief Compares two semantic version strings.
 *
 * Supports versions with optional 'v' prefix and pre-release suffixes
 * (e.g. "1.2.3-alpha1", "v2.0.0-rc2"). A stable release is always
 * considered newer than a pre-release of the same version.
 *
 * @param remote The remote (available) version string.
 * @param local  The local (installed) version string.
 * @return @c true if @a remote is newer than @a local.
 */
bool QSimpleUpdater::compareVersions(const QString& remote, const QString& local)
{
  // clang-format off
  static QRegularExpression re(
    "v?(\\d+)(?:\\.(\\d+))?(?:\\.(\\d+))?(?:-(\\w+)(?:(\\d+))?)?"
  );
  // clang-format on

  QRegularExpressionMatch remoteMatch = re.match(remote);
  QRegularExpressionMatch localMatch  = re.match(local);

  if (!remoteMatch.hasMatch() || !localMatch.hasMatch())
    return false;

  // Compare major, minor, and patch components
  for (int i = 1; i <= 3; ++i) {
    int remoteNum = remoteMatch.captured(i).toInt();
    int localNum  = localMatch.captured(i).toInt();

    if (remoteNum > localNum)
      return true;
    else if (localNum > remoteNum)
      return false;
  }

  // Compare pre-release suffixes
  QString remoteSuffix = remoteMatch.captured(4);
  QString localSuffix  = localMatch.captured(4);

  // Remote is stable, local is pre-release
  if (remoteSuffix.isEmpty() && !localSuffix.isEmpty())
    return true;

  // Remote is pre-release, local is stable
  if (!remoteSuffix.isEmpty() && localSuffix.isEmpty())
    return false;

  // Compare suffixes lexicographically (alpha < beta < rc)
  if (remoteSuffix != localSuffix)
    return remoteSuffix > localSuffix;

  // Same suffix type — compare numeric part
  int remoteSuffixNum = remoteMatch.captured(5).toInt();
  int localSuffixNum  = localMatch.captured(5).toInt();
  return remoteSuffixNum > localSuffixNum;
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url uses
 *        a custom appcast format.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::usesCustomAppcast(const QString& url) const
{
  return getUpdater(url)->customAppcast();
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url will
 *        notify the user when an update is available.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::getNotifyOnUpdate(const QString& url) const
{
  return getUpdater(url)->notifyOnUpdate();
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url will
 *        notify the user when it finishes checking for updates.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::getNotifyOnFinish(const QString& url) const
{
  return getUpdater(url)->notifyOnFinish();
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url has
 *        an update available.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::getUpdateAvailable(const QString& url) const
{
  return getUpdater(url)->updateAvailable();
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url has
 *        the integrated downloader enabled.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::getDownloaderEnabled(const QString& url) const
{
  return getUpdater(url)->downloaderEnabled();
}

/**
 * @brief Returns @c true if the Updater registered with the given @a url uses
 *        custom install procedures.
 *
 * If you want to implement your own way to handle the downloaded file, bind to
 * the @c downloadFinished() signal and call @c setUseCustomInstallProcedures().
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
bool QSimpleUpdater::usesCustomInstallProcedures(const QString& url) const
{
  return getUpdater(url)->useCustomInstallProcedures();
}

/**
 * @brief Returns the URL to open in a web browser for the Updater registered
 *        with the given @a url.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getOpenUrl(const QString& url) const
{
  return getUpdater(url)->openUrl();
}

/**
 * @brief Returns the changelog of the Updater registered with the given @a url.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getChangelog(const QString& url) const
{
  return getUpdater(url)->changelog();
}

/**
 * @brief Returns the module name of the Updater registered with the given
 *        @a url.
 *
 * @note If the module name is empty, the application name will be used.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getModuleName(const QString& url) const
{
  return getUpdater(url)->moduleName();
}

/**
 * @brief Returns the download URL of the Updater registered with the given
 *        @a url.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getDownloadUrl(const QString& url) const
{
  return getUpdater(url)->downloadUrl();
}

/**
 * @brief Returns the platform key of the Updater registered with the given
 *        @a url.
 *
 * Default platform keys:
 *   - On iOS: @c "ios"
 *   - On macOS: @c "osx"
 *   - On Android: @c "android"
 *   - On GNU/Linux: @c "linux"
 *   - On Microsoft Windows: @c "windows"
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getPlatformKey(const QString& url) const
{
  return getUpdater(url)->platformKey();
}

/**
 * @brief Returns the latest remote version of the Updater registered with
 *        the given @a url.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getLatestVersion(const QString& url) const
{
  return getUpdater(url)->latestVersion();
}

/**
 * @brief Returns the local module version of the Updater registered with the
 *        given @a url.
 *
 * @note If the module version is empty, the application version will be used.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getModuleVersion(const QString& url) const
{
  return getUpdater(url)->moduleVersion();
}

/**
 * @brief Returns the user-agent string used by the Updater registered with
 *        the given @a url.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
QString QSimpleUpdater::getUserAgentString(const QString& url) const
{
  return getUpdater(url)->userAgentString();
}

/**
 * @brief Instructs the Updater registered with the given @a url to download
 *        and interpret the update definitions file.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::checkForUpdates(const QString& url)
{
  getUpdater(url)->checkForUpdates();
}

/**
 * @brief Sets the download directory for the Updater registered with the given
 *        @a url.
 */
void QSimpleUpdater::setDownloadDir(const QString& url, const QString& dir)
{
  getUpdater(url)->setDownloadDir(dir);
}

/**
 * @brief Changes the module @a name of the Updater registered with the given
 *        @a url.
 *
 * @note The module name is used on user prompts. If empty, the application name
 *       will be used instead.
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setModuleName(const QString& url, const QString& name)
{
  getUpdater(url)->setModuleName(name);
}

/**
 * @brief Sets whether to notify the user when an update is available.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setNotifyOnUpdate(const QString& url, const bool notify)
{
  getUpdater(url)->setNotifyOnUpdate(notify);
}

/**
 * @brief Sets whether to notify the user when checking for updates finishes.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setNotifyOnFinish(const QString& url, const bool notify)
{
  getUpdater(url)->setNotifyOnFinish(notify);
}

/**
 * @brief Changes the platform key of the Updater registered with the given
 *        @a url.
 *
 * Default platform keys:
 *   - On iOS: @c "ios"
 *   - On macOS: @c "osx"
 *   - On Android: @c "android"
 *   - On GNU/Linux: @c "linux"
 *   - On Microsoft Windows: @c "windows"
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setPlatformKey(const QString& url, const QString& platform)
{
  getUpdater(url)->setPlatformKey(platform);
}

/**
 * @brief Changes the module @a version of the Updater registered with the
 *        given @a url.
 *
 * @note The module version is compared against the remote version. If empty,
 *       the application version will be used.
 */
void QSimpleUpdater::setModuleVersion(const QString& url, const QString& version)
{
  getUpdater(url)->setModuleVersion(version);
}

/**
 * @brief Enables or disables the integrated downloader for the Updater
 *        registered with the given @a url.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setDownloaderEnabled(const QString& url, const bool enabled)
{
  getUpdater(url)->setDownloaderEnabled(enabled);
}

/**
 * @brief Changes the user-agent string used by the Updater registered with
 *        the given @a url.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setUserAgentString(const QString& url, const QString& agent)
{
  getUpdater(url)->setUserAgentString(agent);
}

/**
 * @brief Enables or disables custom appcast interpretation for the Updater
 *        registered with the given @a url.
 *
 * When enabled, the Updater will not parse the downloaded appcast. Instead, it
 * emits the @c appcastDownloaded() signal so the application can interpret the
 * data itself.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setUseCustomAppcast(const QString& url, const bool customAppcast)
{
  getUpdater(url)->setUseCustomAppcast(customAppcast);
}

/**
 * @brief Enables or disables custom install procedures for the Updater
 *        registered with the given @a url.
 *
 * When enabled, the downloader will not attempt to open the downloaded file.
 * Use the @c downloadFinished() signal to implement your own install logic.
 *
 * @note If no Updater is registered with the given @a url, one will be created
 *       automatically.
 */
void QSimpleUpdater::setUseCustomInstallProcedures(const QString& url, const bool custom)
{
  getUpdater(url)->setUseCustomInstallProcedures(custom);
}

/**
 * @brief Sets whether the update is mandatory for the Updater registered with
 *        the given @a url.
 *
 * When mandatory, the application will quit if the user declines the update.
 */
void QSimpleUpdater::setMandatoryUpdate(const QString& url, const bool mandatory_update)
{
  getUpdater(url)->setMandatoryUpdate(mandatory_update);
}

/**
 * @brief Sets the HTTP basic authentication username for downloads.
 */
void QSimpleUpdater::setDownloadUserName(const QString& url, const QString& userName)
{
  getUpdater(url)->setDownloadUserName(userName);
}

/**
 * @brief Sets the HTTP basic authentication password for downloads.
 */
void QSimpleUpdater::setDownloadPassword(const QString& url, const QString& password)
{
  getUpdater(url)->setDownloadPassword(password);
}

/**
 * @brief Returns the Updater instance registered with the given @a url.
 *
 * If no Updater is registered with the given @a url, a new one is created and
 * configured automatically.
 */
Updater* QSimpleUpdater::getUpdater(const QString& url) const
{
  if (!URLS.contains(url)) {
    Updater* updater = new Updater;
    updater->setUrl(url);

    URLS.append(url);
    UPDATERS.append(updater);

    connect(updater, &Updater::checkingFinished, this, &QSimpleUpdater::checkingFinished);
    connect(updater, &Updater::downloadFinished, this, &QSimpleUpdater::downloadFinished);
    connect(updater, &Updater::appcastDownloaded, this, &QSimpleUpdater::appcastDownloaded);
  }

  return UPDATERS.at(URLS.indexOf(url));
}

#if QSU_INCLUDE_MOC
#  include "moc_QSimpleUpdater.cpp"
#endif
