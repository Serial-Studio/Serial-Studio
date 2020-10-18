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

#ifndef _QSIMPLEUPDATER_MAIN_H
#define _QSIMPLEUPDATER_MAIN_H

#include <QUrl>
#include <QList>
#include <QObject>

#if defined (QSU_SHARED)
    #define QSU_DECL Q_DECL_EXPORT
#elif defined (QSU_IMPORT)
    #define QSU_DECL Q_DECL_IMPORT
#else
    #define QSU_DECL
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
class QSU_DECL QSimpleUpdater : public QObject
{
    Q_OBJECT

signals:
    void checkingFinished (const QString& url);
    void appcastDownloaded (const QString& url, const QByteArray& data);
    void downloadFinished (const QString& url, const QString& filepath);

public:
    static QSimpleUpdater* getInstance();

    bool usesCustomAppcast (const QString& url) const;
    bool getNotifyOnUpdate (const QString& url) const;
    bool getNotifyOnFinish (const QString& url) const;
    bool getUpdateAvailable (const QString& url) const;
    bool getDownloaderEnabled (const QString& url) const;
    bool usesCustomInstallProcedures (const QString& url) const;

    QString getOpenUrl (const QString& url) const;
    QString getChangelog (const QString& url) const;
    QString getModuleName (const QString& url) const;
    QString getDownloadUrl (const QString& url) const;
    QString getPlatformKey (const QString& url) const;
    QString getLatestVersion (const QString& url) const;
    QString getModuleVersion (const QString& url) const;
    QString getUserAgentString (const QString& url) const;

public slots:
    void checkForUpdates (const QString& url);
    void setModuleName (const QString& url, const QString& name);
    void setNotifyOnUpdate (const QString& url, const bool notify);
    void setNotifyOnFinish (const QString& url, const bool notify);
    void setPlatformKey (const QString& url, const QString& platform);
    void setModuleVersion (const QString& url, const QString& version);
    void setDownloaderEnabled (const QString& url, const bool enabled);
    void setUserAgentString (const QString& url, const QString& agent);
    void setUseCustomAppcast (const QString& url, const bool customAppcast);
    void setUseCustomInstallProcedures (const QString& url, const bool custom);
    void setMandatoryUpdate (const QString& url, const bool mandatory_update);

protected:
    ~QSimpleUpdater();

private:
    Updater* getUpdater (const QString& url) const;
};

#endif
