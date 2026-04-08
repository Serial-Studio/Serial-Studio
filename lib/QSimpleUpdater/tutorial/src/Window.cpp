/*
 * Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#include "Window.h"

#include <QSimpleUpdater.h>

#include <QDebug>

#include "ui_Window.h"

// Update definitions URL pointing to the tutorial's JSON file
static const QString DEFS_URL = "https://raw.githubusercontent.com/"
                                "alex-spataru/QSimpleUpdater/master/tutorial/"
                                "definitions/updates.json";

/**
 * @brief Constructs the tutorial window and connects signals.
 */
Window::Window(QWidget* parent)
  : QMainWindow(parent), m_ui(new Ui::Window), m_updater(QSimpleUpdater::getInstance())
{
  m_ui->setupUi(this);
  setWindowTitle(qApp->applicationName());

  // Connect updater signals
  connect(m_updater, &QSimpleUpdater::checkingFinished, this, &Window::updateChangelog);
  connect(m_updater, &QSimpleUpdater::appcastDownloaded, this, &Window::displayAppcast);

  // Connect button signals
  connect(m_ui->resetButton, &QPushButton::clicked, this, &Window::resetFields);
  connect(m_ui->closeButton, &QPushButton::clicked, this, &Window::close);
  connect(m_ui->checkButton, &QPushButton::clicked, this, &Window::checkForUpdates);

  // Size and reset
  setMinimumSize(minimumSizeHint());
  resize(minimumSizeHint());
  resetFields();
}

/**
 * @brief Destroys the window and frees the UI.
 */
Window::~Window()
{
  delete m_ui;
}

/**
 * @brief Resets all UI fields to their default values.
 */
void Window::resetFields()
{
  m_ui->installedVersion->setText("0.1");
  m_ui->customAppcast->setChecked(false);
  m_ui->enableDownloader->setChecked(true);
  m_ui->showAllNotifcations->setChecked(false);
  m_ui->showUpdateNotifications->setChecked(true);
  m_ui->mandatoryUpdate->setChecked(false);
}

/**
 * @brief Reads the UI settings and triggers an update check.
 */
void Window::checkForUpdates()
{
  QString version        = m_ui->installedVersion->text();
  bool customAppcast     = m_ui->customAppcast->isChecked();
  bool downloaderEnabled = m_ui->enableDownloader->isChecked();
  bool notifyOnFinish    = m_ui->showAllNotifcations->isChecked();
  bool notifyOnUpdate    = m_ui->showUpdateNotifications->isChecked();
  bool mandatoryUpdate   = m_ui->mandatoryUpdate->isChecked();

  m_updater->setModuleVersion(DEFS_URL, version);
  m_updater->setNotifyOnFinish(DEFS_URL, notifyOnFinish);
  m_updater->setNotifyOnUpdate(DEFS_URL, notifyOnUpdate);
  m_updater->setUseCustomAppcast(DEFS_URL, customAppcast);
  m_updater->setDownloaderEnabled(DEFS_URL, downloaderEnabled);
  m_updater->setMandatoryUpdate(DEFS_URL, mandatoryUpdate);

  m_updater->checkForUpdates(DEFS_URL);
}

/**
 * @brief Displays the changelog in the text browser when checking finishes.
 */
void Window::updateChangelog(const QString& url)
{
  if (url == DEFS_URL)
    m_ui->changelogText->setText(m_updater->getChangelog(url));
}

/**
 * @brief Displays the raw appcast data when custom appcast mode is enabled.
 */
void Window::displayAppcast(const QString& url, const QByteArray& reply)
{
  if (url == DEFS_URL) {
    QString text = "This is the downloaded appcast: <p><pre>" + QString::fromUtf8(reply)
                 + "</pre></p><p> If you need to store more information on the "
                   "appcast (or use another format), just use the "
                   "<b>QSimpleUpdater::setCustomAppcast()</b> function. "
                   "It allows your application to interpret the appcast "
                   "using your code and not QSU's code.</p>";

    m_ui->changelogText->setText(text);
  }
}
