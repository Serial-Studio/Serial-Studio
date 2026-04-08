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

#include "Updater.h"

#include <QApplication>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>

#include "Downloader.h"

/**
 * @brief Constructs an Updater instance with default settings.
 *
 * The platform key is auto-detected based on the build target. The user-agent
 * string is initialized from the application name and version.
 */
Updater::Updater()
  : m_url("")
  , m_customAppcast(false)
  , m_notifyOnUpdate(true)
  , m_notifyOnFinish(false)
  , m_updateAvailable(false)
  , m_downloaderEnabled(true)
  , m_mandatoryUpdate(false)
  , m_openUrl("")
  , m_changelog("")
  , m_moduleName(qApp->applicationName())
  , m_downloadUrl("")
  , m_moduleVersion(qApp->applicationVersion())
  , m_latestVersion("")
  , m_downloader(new Downloader())
  , m_manager(new QNetworkAccessManager())
{
#if defined Q_OS_WIN
  m_platform = "windows";
#elif defined Q_OS_MAC
  m_platform = "osx";
#elif defined Q_OS_LINUX
  m_platform = "linux";
#elif defined Q_OS_ANDROID
  m_platform = "android";
#elif defined Q_OS_IOS
  m_platform = "ios";
#endif

  setUserAgentString(
    QString("%1/%2 (Qt; QSimpleUpdater)").arg(qApp->applicationName(), qApp->applicationVersion()));

  connect(m_downloader, &Downloader::downloadFinished, this, &Updater::downloadFinished);
  connect(m_manager, &QNetworkAccessManager::finished, this, &Updater::onReply);
}

/**
 * @brief Destroys the Updater and its owned Downloader and network manager.
 */
Updater::~Updater()
{
  delete m_downloader;
  delete m_manager;
}

/**
 * @brief Returns the URL of the update definitions file.
 */
QString Updater::url() const
{
  return m_url;
}

/**
 * @brief Returns the URL that the update definitions file wants us to open in
 *        a web browser.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
QString Updater::openUrl() const
{
  return m_openUrl;
}

/**
 * @brief Returns the changelog defined by the update definitions file.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
QString Updater::changelog() const
{
  return m_changelog;
}

/**
 * @brief Returns the name of the module (if defined).
 */
QString Updater::moduleName() const
{
  return m_moduleName;
}

/**
 * @brief Returns the platform key (auto-detected or user-set).
 *
 * Default platform keys:
 *   - On iOS: @c "ios"
 *   - On macOS: @c "osx"
 *   - On Android: @c "android"
 *   - On GNU/Linux: @c "linux"
 *   - On Microsoft Windows: @c "windows"
 */
QString Updater::platformKey() const
{
  return m_platform;
}

/**
 * @brief Returns the download URL defined by the update definitions file.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
QString Updater::downloadUrl() const
{
  return m_downloadUrl;
}

/**
 * @brief Returns the latest version defined by the update definitions file.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
QString Updater::latestVersion() const
{
  return m_latestVersion;
}

/**
 * @brief Returns the user-agent header used by the client when communicating
 *        with the server through HTTP.
 */
QString Updater::userAgentString() const
{
  return m_userAgentString;
}

/**
 * @brief Returns the local version of the installed module.
 */
QString Updater::moduleVersion() const
{
  return m_moduleVersion;
}

/**
 * @brief Returns @c true if the updater should NOT interpret the downloaded
 *        appcast.
 *
 * This is useful if you need to store additional variables in the JSON file or
 * use another appcast format (e.g. XML).
 */
bool Updater::customAppcast() const
{
  return m_customAppcast;
}

/**
 * @brief Returns @c true if the updater should notify the user when an update
 *        is available.
 */
bool Updater::notifyOnUpdate() const
{
  return m_notifyOnUpdate;
}

/**
 * @brief Returns @c true if the updater should notify the user when it finishes
 *        checking for updates.
 *
 * @note If set to @c true, the Updater will notify the user even when there are
 *       no updates available.
 */
bool Updater::notifyOnFinish() const
{
  return m_notifyOnFinish;
}

/**
 * @brief Returns @c true if the current update is mandatory.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
bool Updater::mandatoryUpdate() const
{
  return m_mandatoryUpdate;
}

/**
 * @brief Returns @c true if there is an update available.
 *
 * @warning You should call @c checkForUpdates() before using this function.
 */
bool Updater::updateAvailable() const
{
  return m_updateAvailable;
}

/**
 * @brief Returns @c true if the integrated downloader is enabled.
 *
 * @note If enabled, the Updater will open the downloader dialog when the user
 *       agrees to download the update.
 */
bool Updater::downloaderEnabled() const
{
  return m_downloaderEnabled;
}

/**
 * @brief Returns @c true if the updater will not intervene when the download
 *        has finished.
 *
 * Use the QSimpleUpdater signals to know when the download is completed.
 */
bool Updater::useCustomInstallProcedures() const
{
  return m_downloader->useCustomInstallProcedures();
}

/**
 * @brief Downloads and interprets the update definitions file referenced by
 *        the @c url() function.
 */
void Updater::checkForUpdates()
{
  QNetworkRequest request(url());
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);

  if (!userAgentString().isEmpty())
    request.setRawHeader("User-Agent", userAgentString().toUtf8());

  m_manager->get(request);
}

/**
 * @brief Changes the @a url of the update definitions file.
 */
void Updater::setUrl(const QString& url)
{
  m_url = url;
}

/**
 * @brief Changes the module @a name.
 *
 * @note The module name is used on user prompts. If empty, the application name
 *       will be used.
 */
void Updater::setModuleName(const QString& name)
{
  m_moduleName = name;
}

/**
 * @brief Sets whether to notify the user when an update is available.
 */
void Updater::setNotifyOnUpdate(const bool notify)
{
  m_notifyOnUpdate = notify;
}

/**
 * @brief Sets whether to notify the user when checking for updates finishes.
 */
void Updater::setNotifyOnFinish(const bool notify)
{
  m_notifyOnFinish = notify;
}

/**
 * @brief Changes the user-agent string used to identify the client application
 *        in HTTP sessions.
 */
void Updater::setUserAgentString(const QString& agent)
{
  m_userAgentString = agent;
  m_downloader->setUserAgentString(agent);
}

/**
 * @brief Changes the module @a version.
 *
 * @note The module version is used to compare the local and remote versions.
 *       If empty, the application version will be used.
 */
void Updater::setModuleVersion(const QString& version)
{
  m_moduleVersion = version;
}

/**
 * @brief Enables or disables the integrated downloader.
 *
 * If enabled, the Updater will open the integrated downloader when the user
 * agrees to install the update (if any).
 */
void Updater::setDownloaderEnabled(const bool enabled)
{
  m_downloaderEnabled = enabled;
}

/**
 * @brief Sets the directory where downloaded files will be saved.
 */
void Updater::setDownloadDir(const QString& dir)
{
  m_downloader->setDownloadDir(dir);
}

/**
 * @brief Changes the platform key.
 *
 * Default platform keys:
 *   - On iOS: @c "ios"
 *   - On macOS: @c "osx"
 *   - On Android: @c "android"
 *   - On GNU/Linux: @c "linux"
 *   - On Microsoft Windows: @c "windows"
 */
void Updater::setPlatformKey(const QString& platformKey)
{
  m_platform = platformKey;
}

/**
 * @brief Enables or disables custom appcast interpretation.
 *
 * When enabled, the Updater will not parse the server response. Instead, it
 * emits the @c appcastDownloaded() signal so the application can interpret
 * the data itself.
 */
void Updater::setUseCustomAppcast(const bool customAppcast)
{
  m_customAppcast = customAppcast;
}

/**
 * @brief Enables or disables custom install procedures.
 *
 * When enabled, the Updater will not try to open the downloaded file. Use the
 * QSimpleUpdater signals to implement your own install logic.
 */
void Updater::setUseCustomInstallProcedures(const bool custom)
{
  m_downloader->setUseCustomInstallProcedures(custom);
}

/**
 * @brief Sets whether the update is mandatory.
 *
 * When mandatory, the application will quit if the user declines the update.
 */
void Updater::setMandatoryUpdate(const bool mandatory_update)
{
  m_mandatoryUpdate = mandatory_update;
}

/**
 * @brief Sets the HTTP basic authentication username for downloads.
 */
void Updater::setDownloadUserName(const QString& user_name)
{
  m_downloadUserName = user_name;
}

/**
 * @brief Sets the HTTP basic authentication password for downloads.
 */
void Updater::setDownloadPassword(const QString& password)
{
  m_downloadPassword = password;
}

/**
 * @brief Called when the download of the update definitions file is finished.
 *
 * Parses the JSON response, extracts platform-specific update information,
 * and determines whether an update is available.
 */
void Updater::onReply(QNetworkReply* reply)
{
  // Ensure the reply is cleaned up when we're done
  reply->deleteLater();

  // Check if we need to redirect
  QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (!redirect.isEmpty()) {
    setUrl(redirect.toString());
    checkForUpdates();
    return;
  }

  // There was a network error
  if (reply->error() != QNetworkReply::NoError) {
    setUpdateAvailable(false);
    emit checkingFinished(url());
    return;
  }

  // The application wants to interpret the appcast by itself
  if (customAppcast()) {
    emit appcastDownloaded(url(), reply->readAll());
    emit checkingFinished(url());
    return;
  }

  // Try to create a JSON document from downloaded data
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

  // JSON is invalid
  if (document.isNull()) {
    setUpdateAvailable(false);
    emit checkingFinished(url());
    return;
  }

  // Get the platform information
  QJsonObject updates  = document.object().value("updates").toObject();
  QJsonObject platform = updates.value(platformKey()).toObject();

  // Get update information
  m_openUrl       = platform.value("open-url").toString();
  m_changelog     = platform.value("changelog").toString();
  m_downloadUrl   = platform.value("download-url").toString();
  m_latestVersion = platform.value("latest-version").toString();
  if (platform.contains("mandatory-update"))
    m_mandatoryUpdate = platform.value("mandatory-update").toBool();

  // Compare latest and current version
  setUpdateAvailable(compare(latestVersion(), moduleVersion()));
  emit checkingFinished(url());
}

/**
 * @brief Prompts the user based on the value of the @a available parameter and
 *        the notification settings of this Updater instance.
 */
void Updater::setUpdateAvailable(const bool available)
{
  m_updateAvailable = available;

  QMessageBox box;
  box.setTextFormat(Qt::RichText);
  box.setIcon(QMessageBox::Information);

  if (updateAvailable() && (notifyOnUpdate() || notifyOnFinish())) {
    QString text = tr("Would you like to download the update now?");
    if (m_mandatoryUpdate) {
      text = tr("Would you like to download the update now?<br />This is a "
                "mandatory update, exiting now will close the application.");
    }

    text += "<br/><br/>";
    if (!m_changelog.isEmpty())
      text += tr("<strong>Change log:</strong><br/>%1").arg(m_changelog);

    QString title = "<h3>"
                  + tr("Version %1 of %2 has been released!").arg(latestVersion()).arg(moduleName())
                  + "</h3>";

    box.setText(title);
    box.setInformativeText(text);
    box.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton(QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes) {
      if (!openUrl().isEmpty())
        QDesktopServices::openUrl(QUrl(openUrl()));

      else if (downloaderEnabled()) {
        m_downloader->setUrlId(url());
        m_downloader->setFileName(downloadUrl().split("/").last());
        m_downloader->setMandatoryUpdate(m_mandatoryUpdate);
        auto download_url = QUrl(downloadUrl());
        download_url.setUserName(m_downloadUserName);
        download_url.setPassword(m_downloadPassword);
        m_downloader->startDownload(download_url);
      }

      else
        QDesktopServices::openUrl(QUrl(downloadUrl()));
    } else {
      if (m_mandatoryUpdate)
        QApplication::quit();
    }
  }

  else if (notifyOnFinish()) {
    box.setStandardButtons(QMessageBox::Close);
    box.setInformativeText(tr("No updates are available for the moment"));
    box.setText("<h3>"
                + tr("Congratulations! You are running the latest version "
                     "of %1")
                    .arg(moduleName())
                + "</h3>");

    box.exec();
  }
}

/**
 * @brief Compares two version strings using QSimpleUpdater::compareVersions().
 *
 * @param x The remote version.
 * @param y The local version.
 * @return @c true if @a x is greater than @a y.
 */
bool Updater::compare(const QString& x, const QString& y)
{
  return QSimpleUpdater::compareVersions(x, y);
}

#if QSU_INCLUDE_MOC
#  include "moc_Updater.cpp"
#endif
