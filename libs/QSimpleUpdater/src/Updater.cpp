/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of the QSimpleUpdater library, which is released under
 * the DBAD license, you can read a copy of it below:
 *
 * DON'T BE A DICK PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING,
 * DISTRIBUTION AND MODIFICATION:
 *
 * Do whatever you like with the original work, just don't be a dick.
 * Being a dick includes - but is not limited to - the following instances:
 *
 * 1a. Outright copyright infringement - Don't just copy this and change the
 *     name.
 * 1b. Selling the unmodified original with no work done what-so-ever, that's
 *     REALLY being a dick.
 * 1c. Modifying the original work to contain hidden harmful content.
 *     That would make you a PROPER dick.
 *
 * If you become rich through modifications, related works/services, or
 * supporting the original work, share the love.
 * Only a dick would make loads off this work and not buy the original works
 * creator(s) a pint.
 *
 * Code is provided with no warranty. Using somebody else's code and bitching
 * when it goes wrong makes you a DONKEY dick.
 * Fix the problem yourself. A non-dick would submit the fix back.
 */

#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>

#include "Updater.h"
#include "Downloader.h"

Updater::Updater()
{
    m_url = "";
    m_openUrl = "";
    m_changelog = "";
    m_downloadUrl = "";
    m_latestVersion = "";
    m_customAppcast = false;
    m_notifyOnUpdate = true;
    m_notifyOnFinish = false;
    m_updateAvailable = false;
    m_downloaderEnabled = true;
    m_moduleName = qApp->applicationName();
    m_moduleVersion = qApp->applicationVersion();
    m_mandatoryUpdate = false;

    m_downloader = new Downloader();
    m_manager = new QNetworkAccessManager();

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

    setUserAgentString (QString ("%1/%2 (Qt; QSimpleUpdater)").arg (qApp->applicationName(),
                        qApp->applicationVersion()));

    connect (m_downloader, SIGNAL (downloadFinished (QString, QString)),
             this,         SIGNAL (downloadFinished (QString, QString)));
    connect (m_manager,    SIGNAL (finished (QNetworkReply*)),
             this,           SLOT (onReply  (QNetworkReply*)));
}

Updater::~Updater()
{
    delete m_downloader;
}

/**
 * Returns the URL of the update definitions file
 */
QString Updater::url() const
{
    return m_url;
}

/**
 * Returns the URL that the update definitions file wants us to open in
 * a web browser.
 *
 * \warning You should call \c checkForUpdates() before using this functio
 */
QString Updater::openUrl() const
{
    return m_openUrl;
}

/**
 * Returns the changelog defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::changelog() const
{
    return m_changelog;
}

/**
 * Returns the name of the module (if defined)
 */
QString Updater::moduleName() const
{
    return m_moduleName;
}

/**
 * Returns the platform key (be it system-set or user-set).
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
QString Updater::platformKey() const
{
    return m_platform;
}

/**
 * Returns the download URL defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::downloadUrl() const
{
    return m_downloadUrl;
}

/**
 * Returns the latest version defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::latestVersion() const
{
    return m_latestVersion;
}

/**
 * Returns the user-agent header used by the client when communicating
 * with the server through HTTP
 */
QString Updater::userAgentString() const
{
    return m_userAgentString;
}

/**
 * Returns the "local" version of the installed module
 */
QString Updater::moduleVersion() const
{
    return m_moduleVersion;
}

/**
 * Returns \c true if the updater should NOT interpret the downloaded appcast.
 * This is useful if you need to store more variables (or information) in the
 * JSON file or use another appcast format (e.g. XML)
 */
bool Updater::customAppcast() const
{
    return m_customAppcast;
}

/**
 * Returns \c true if the updater should notify the user when an update is
 * available.
 */
bool Updater::notifyOnUpdate() const
{
    return m_notifyOnUpdate;
}

/**
 * Returns \c true if the updater should notify the user when it finishes
 * checking for updates.
 *
 * \note If set to \c true, the \c Updater will notify the user even when there
 *       are no updates available (by congratulating him/her about being smart)
 */
bool Updater::notifyOnFinish() const
{
    return m_notifyOnFinish;
}

/**
 * Returns \c true if there the current update is mandatory.
 * \warning You should call \c checkForUpdates() before using this function
*/
bool Updater::mandatoryUpdate() const
{
    return m_mandatoryUpdate;
}

/**
 * Returns \c true if there is an update available.
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::updateAvailable() const
{
    return m_updateAvailable;
}

/**
 * Returns \c true if the integrated downloader is enabled.
 * \note If set to \c true, the \c Updater will open the downloader dialog if
 *       the user agrees to download the update.
 */
bool Updater::downloaderEnabled() const
{
    return m_downloaderEnabled;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 */
bool Updater::useCustomInstallProcedures() const
{
    return m_downloader->useCustomInstallProcedures();
}

/**
 * Downloads and interpets the update definitions file referenced by the
 * \c url() function.
 */
void Updater::checkForUpdates()
{
    QNetworkRequest request (url());
    if (!userAgentString().isEmpty())
        request.setRawHeader ("User-Agent", userAgentString().toUtf8());

    m_manager->get (request);
}

/**
 * Changes the \c url in which the \c Updater can find the update definitions
 * file.
 */
void Updater::setUrl (const QString& url)
{
    m_url = url;
}

/**
 * Changes the module \a name.
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void Updater::setModuleName (const QString& name)
{
    m_moduleName = name;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when an update is available.
 */
void Updater::setNotifyOnUpdate (const bool notify)
{
    m_notifyOnUpdate = notify;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when it has finished interpreting the update definitions file.
 */
void Updater::setNotifyOnFinish (const bool notify)
{
    m_notifyOnFinish = notify;
}

/**
 * Changes the user agent string used to identify the client application
 * from the server in a HTTP session.
 *
 * By default, the user agent will co
 */
void Updater::setUserAgentString (const QString& agent)
{
    m_userAgentString = agent;
    m_downloader->setUserAgentString (agent);
}

/**
 * Changes the module \a version
 * \note The module version is used to compare the local and remote versions.
 *       If the \a version parameter is empty, then the \c Updater will use the
 *       application version (referenced by \c qApp)
 */
void Updater::setModuleVersion (const QString& version)
{
    m_moduleVersion = version;
}

/**
 * If the \a enabled parameter is set to \c true, the \c Updater will open the
 * integrated downloader if the user agrees to install the update (if any)
 */
void Updater::setDownloaderEnabled (const bool enabled)
{
    m_downloaderEnabled = enabled;
}

/**
 * Changes the platform key.
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
void Updater::setPlatformKey (const QString& platformKey)
{
    m_platform = platformKey;
}

/**
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself
 */
void Updater::setUseCustomAppcast (const bool customAppcast)
{
    m_customAppcast = customAppcast;
}

/**
 * If the \a custom parameter is set to \c true, the \c Updater will not try
 * to open the downloaded file. Use the signals fired by the \c QSimpleUpdater
 * to install the update from the downloaded file by yourself.
 */
void Updater::setUseCustomInstallProcedures (const bool custom)
{
    m_downloader->setUseCustomInstallProcedures (custom);
}

/**
 * If the \a mandatory_update is set to \c true, the \c Updater has to download and install the
 * update. If the user cancels or exits, the application will close
 */
void Updater::setMandatoryUpdate(const bool mandatory_update)
{
    m_mandatoryUpdate = mandatory_update;
}
/**
 * Called when the download of the update definitions file is finished.
 */
void Updater::onReply (QNetworkReply* reply)
{
    /* Check if we need to redirect */
    QUrl redirect = reply->attribute (
                        QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirect.isEmpty()) {
        setUrl (redirect.toString());
        checkForUpdates();
        return;
    }

    /* There was a network error */
    if (reply->error() != QNetworkReply::NoError) {
        setUpdateAvailable (false);
        emit checkingFinished (url());
        return;
    }

    /* The application wants to interpret the appcast by itself */
    if (customAppcast()) {
        emit appcastDownloaded (url(), reply->readAll());
        emit checkingFinished (url());
        return;
    }

    /* Try to create a JSON document from downloaded data */
    QJsonDocument document = QJsonDocument::fromJson (reply->readAll());

    /* JSON is invalid */
    if (document.isNull()) {
        setUpdateAvailable (false);
        emit checkingFinished (url());
        return;
    }

    /* Get the platform information */
    QJsonObject updates = document.object().value ("updates").toObject();
    QJsonObject platform = updates.value (platformKey()).toObject();

    /* Get update information */
    m_openUrl = platform.value ("open-url").toString();
    m_changelog = platform.value ("changelog").toString();
    m_downloadUrl = platform.value ("download-url").toString();
    m_latestVersion = platform.value ("latest-version").toString();
    if (platform.contains("mandatory-update"))
        m_mandatoryUpdate = platform.value ("mandatory-update").toBool();

    /* Compare latest and current version */
    setUpdateAvailable (compare (latestVersion(), moduleVersion()));
    emit checkingFinished (url());
}

/**
 * Prompts the user based on the value of the \a available parameter and the
 * settings of this instance of the \c Updater class.
 */
void Updater::setUpdateAvailable (const bool available)
{
    m_updateAvailable = available;

    QMessageBox box;
    box.setTextFormat (Qt::RichText);
    box.setIcon (QMessageBox::Information);

    if (updateAvailable() && (notifyOnUpdate() || notifyOnFinish()))
    {
        QString text = tr("Would you like to download the update now?");
        if (m_mandatoryUpdate)
        {
            text = tr ("Would you like to download the update now? This is a mandatory update, exiting now will close the application");
        }

        QString title = "<h3>"
                        + tr ("Version %1 of %2 has been released!")
                        .arg (latestVersion()).arg (moduleName())
                        + "</h3>";

        box.setText (title);
        box.setInformativeText (text);
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::Yes);

        if (box.exec() == QMessageBox::Yes) {
            if (!openUrl().isEmpty())
                QDesktopServices::openUrl (QUrl (openUrl()));

            else if (downloaderEnabled()) {
                m_downloader->setUrlId (url());
                m_downloader->setFileName (downloadUrl().split ("/").last());
                m_downloader->setMandatoryUpdate(m_mandatoryUpdate);
                m_downloader->startDownload (QUrl (downloadUrl()));
            }

            else
                QDesktopServices::openUrl (QUrl (downloadUrl()));
        }else
        {
            if(m_mandatoryUpdate)
            {
                QApplication::quit();
            }
        }
    }

    else if (notifyOnFinish()) {
        box.setStandardButtons (QMessageBox::Close);
        box.setInformativeText (tr ("No updates are available for the moment"));
        box.setText ("<h3>"
                     + tr ("Congratulations! You are running the "
                           "latest version of %1").arg (moduleName())
                     + "</h3>");

        box.exec();
    }
}

/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 */
bool Updater::compare (const QString& x, const QString& y)
{
    QStringList versionsX = x.split (".");
    QStringList versionsY = y.split (".");

    int count = qMin (versionsX.count(), versionsY.count());

    for (int i = 0; i < count; ++i) {
        int a = QString (versionsX.at (i)).toInt();
        int b = QString (versionsY.at (i)).toInt();

        if (a > b)
            return true;

        else if (b > a)
            return false;
    }

    return versionsY.count() < versionsX.count();
}
