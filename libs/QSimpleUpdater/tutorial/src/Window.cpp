/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#include "Window.h"
#include "ui_Window.h"

#include <QDebug>
#include <QSimpleUpdater.h>

//==============================================================================
// Define the URL of the Update Definitions file
//==============================================================================

static const QString DEFS_URL = "https://raw.githubusercontent.com/"
                                "alex-spataru/QSimpleUpdater/master/tutorial/"
                                "definitions/updates.json";

//==============================================================================
// Window::Window
//==============================================================================

Window::Window (QWidget* parent) : QMainWindow (parent)
{
    m_ui = new Ui::Window;
    m_ui->setupUi (this);

    setWindowTitle (qApp->applicationName());

    /* QSimpleUpdater is single-instance */
    m_updater = QSimpleUpdater::getInstance();

    /* Check for updates when the "Check For Updates" button is clicked */
    connect (m_updater, SIGNAL (checkingFinished  (QString)),
             this,        SLOT (updateChangelog   (QString)));
    connect (m_updater, SIGNAL (appcastDownloaded (QString, QByteArray)),
             this,        SLOT (displayAppcast    (QString, QByteArray)));

    /* React to button clicks */
    connect (m_ui->resetButton, SIGNAL (clicked()),
             this,                SLOT (resetFields()));
    connect (m_ui->closeButton, SIGNAL (clicked()),
             this,                SLOT (close()));
    connect (m_ui->checkButton, SIGNAL (clicked()),
             this,                SLOT (checkForUpdates()));

    /* Resize the dialog to fit */
    setMinimumSize (minimumSizeHint());
    resize (minimumSizeHint());

    /* Reset the UI state */
    resetFields();
}

//==============================================================================
// Window::~Window
//==============================================================================

Window::~Window()
{
    delete m_ui;
}

//==============================================================================
// Window::checkForUpdates
//==============================================================================

void Window::resetFields()
{
    m_ui->installedVersion->setText ("0.1");
    m_ui->customAppcast->setChecked (false);
    m_ui->enableDownloader->setChecked (true);
    m_ui->showAllNotifcations->setChecked (false);
    m_ui->showUpdateNotifications->setChecked (true);
    m_ui->mandatoryUpdate->setChecked (false);

}

//==============================================================================
// Window::checkForUpdates
//==============================================================================

void Window::checkForUpdates()
{
    /* Get settings from the UI */
    QString version = m_ui->installedVersion->text();
    bool customAppcast = m_ui->customAppcast->isChecked();
    bool downloaderEnabled = m_ui->enableDownloader->isChecked();
    bool notifyOnFinish = m_ui->showAllNotifcations->isChecked();
    bool notifyOnUpdate = m_ui->showUpdateNotifications->isChecked();
    bool mandatoryUpdate = m_ui->mandatoryUpdate->isChecked();

    /* Apply the settings */
    m_updater->setModuleVersion (DEFS_URL, version);
    m_updater->setNotifyOnFinish (DEFS_URL, notifyOnFinish);
    m_updater->setNotifyOnUpdate (DEFS_URL, notifyOnUpdate);
    m_updater->setUseCustomAppcast (DEFS_URL, customAppcast);
    m_updater->setDownloaderEnabled (DEFS_URL, downloaderEnabled);
    m_updater->setMandatoryUpdate (DEFS_URL, mandatoryUpdate);

    /* Check for updates */
    m_updater->checkForUpdates (DEFS_URL);
}

//==============================================================================
// Window::updateChangelog
//==============================================================================

void Window::updateChangelog (const QString& url)
{
    if (url == DEFS_URL)
        m_ui->changelogText->setText (m_updater->getChangelog (url));
}


//==============================================================================
// Window::displayAppcast
//==============================================================================

void Window::displayAppcast (const QString& url, const QByteArray& reply)
{
    if (url == DEFS_URL) {
        QString text = "This is the downloaded appcast: <p><pre>" +
                       QString::fromUtf8 (reply) +
                       "</pre></p><p> If you need to store more information on the "
                       "appcast (or use another format), just use the "
                       "<b>QSimpleUpdater::setCustomAppcast()</b> function. "
                       "It allows your application to interpret the appcast "
                       "using your code and not QSU's code.</p>";

        m_ui->changelogText->setText (text);
    }
}
