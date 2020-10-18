/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 * Copyright (c) 2017 Gilmanov Ildar <https://github.com/gilmanov-ildar>
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

#ifndef DOWNLOAD_DIALOG_H
#define DOWNLOAD_DIALOG_H

#include <QDir>
#include <QDialog>
#include <ui_Downloader.h>

namespace Ui {
class Downloader;
}

class QNetworkReply;
class QNetworkAccessManager;

/**
 * \brief Implements an integrated file downloader with a nice UI
 */
class Downloader : public QWidget
{
    Q_OBJECT

signals:
    void downloadFinished (const QString& url, const QString& filepath);

public:
    explicit Downloader (QWidget* parent = 0);
    ~Downloader();

    bool useCustomInstallProcedures() const;

    QString downloadDir() const;
    void setDownloadDir (const QString& downloadDir);

public slots:
    void setUrlId (const QString& url);
    void startDownload (const QUrl& url);
    void setFileName (const QString& file);
    void setUserAgentString (const QString& agent);
    void setUseCustomInstallProcedures (const bool custom);
    void setMandatoryUpdate (const bool mandatory_update);

private slots:
    void finished();
    void openDownload();
    void installUpdate();
    void cancelDownload();
    void saveFile (qint64 received, qint64 total);
    void calculateSizes (qint64 received, qint64 total);
    void updateProgress (qint64 received, qint64 total);
    void calculateTimeRemaining (qint64 received, qint64 total);

private:
    qreal round (const qreal& input);

private:
    QString m_url;
    uint m_startTime;
    QDir m_downloadDir;
    QString m_fileName;
    Ui::Downloader* m_ui;
    QNetworkReply* m_reply;
    QString m_userAgentString;

    bool m_useCustomProcedures;
    bool m_mandatoryUpdate;

    QNetworkAccessManager* m_manager;
};

#endif
