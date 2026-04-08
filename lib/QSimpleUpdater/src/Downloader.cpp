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

#include "Downloader.h"

#include <math.h>

#include <QApplication>
#include <QAuthenticator>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "AuthenticateDialog.h"

static const QString PARTIAL_DOWN(".part");

/**
 * @brief Constructs the Downloader widget.
 *
 * Initializes the UI, network manager, and default settings. The download
 * directory defaults to the user's home Downloads folder.
 *
 * @param parent Optional parent widget.
 */
Downloader::Downloader(QWidget* parent)
  : QWidget(parent)
  , m_startTime(0)
  , m_ui(new Ui::Downloader)
  , m_reply(nullptr)
  , m_useCustomProcedures(false)
  , m_mandatoryUpdate(false)
  , m_manager(new QNetworkAccessManager())
{
  m_ui->setupUi(this);

  // Set download directory
  m_downloadDir.setPath(QDir::homePath() + "/Downloads/");

  // Make the window look like a modal dialog
  setWindowIcon(QIcon());
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

  // Configure the appearance and behavior of the buttons
  m_ui->openButton->setEnabled(false);
  m_ui->openButton->setVisible(false);
  connect(m_ui->stopButton, &QPushButton::clicked, this, &Downloader::cancelDownload);
  connect(m_ui->openButton, &QPushButton::clicked, this, &Downloader::installUpdate);
  connect(
    m_manager, &QNetworkAccessManager::authenticationRequired, this, &Downloader::authenticate);

  // Resize to fit
  setFixedSize(minimumSizeHint());
}

/**
 * @brief Destroys the Downloader and frees owned resources.
 */
Downloader::~Downloader()
{
  delete m_ui;

  if (m_reply) {
    m_reply->abort();
    m_reply->deleteLater();
  }

  delete m_manager;
}

/**
 * @brief Returns @c true if the updater shall not intervene when the download
 *        has finished.
 *
 * Use the QSimpleUpdater signals to know when the download is completed.
 */
bool Downloader::useCustomInstallProcedures() const
{
  return m_useCustomProcedures;
}

/**
 * @brief Changes the URL identifier used to associate the downloader dialog
 *        with an Updater instance.
 *
 * @note The @a url parameter is not the download URL; it is the URL of the
 *       appcast file.
 */
void Downloader::setUrlId(const QString& url)
{
  m_url = url;
}

/**
 * @brief Begins downloading the file at the given @a url.
 */
void Downloader::startDownload(const QUrl& url)
{
  // Reset UI
  m_ui->progressBar->setValue(0);
  m_ui->stopButton->setText(tr("Stop"));
  m_ui->downloadLabel->setText(tr("Downloading updates"));
  m_ui->timeLabel->setText(tr("Time remaining") + ": " + tr("unknown"));

  // Configure the network request
  QNetworkRequest request(url);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  request.setTransferTimeout(10000);
#endif

  if (!m_userAgentString.isEmpty())
    request.setRawHeader("User-Agent", m_userAgentString.toUtf8());

  // Clean up previous reply if any
  if (m_reply) {
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }

  // Start download
  m_reply     = m_manager->get(request);
  m_startTime = QDateTime::currentDateTime().toSecsSinceEpoch();

  // Ensure that downloads directory exists
  if (!m_downloadDir.exists())
    m_downloadDir.mkpath(".");

  // Remove old downloads
  QFile::remove(m_downloadDir.filePath(m_fileName));
  QFile::remove(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));

  // Update UI when download progress changes or download finishes
  connect(m_reply, &QNetworkReply::metaDataChanged, this, &Downloader::metaDataChanged);
  connect(m_reply, &QNetworkReply::downloadProgress, this, &Downloader::updateProgress);
  connect(m_reply, &QNetworkReply::finished, this, &Downloader::finished);

  showNormal();
}

/**
 * @brief Changes the name of the downloaded file.
 */
void Downloader::setFileName(const QString& file)
{
  m_fileName = file;

  if (m_fileName.isEmpty())
    m_fileName = "QSU_Update.bin";
}

/**
 * @brief Changes the user-agent string used to communicate with the remote
 *        HTTP server.
 */
void Downloader::setUserAgentString(const QString& agent)
{
  m_userAgentString = agent;
}

/**
 * @brief Called when the download finishes.
 *
 * Renames the partial file, emits @c downloadFinished(), and triggers the
 * install flow.
 */
void Downloader::finished()
{
  if (!m_reply || m_reply->error() != QNetworkReply::NoError) {
    QFile::remove(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));
    return;
  }

  // Rename file
  QFile::rename(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN),
                m_downloadDir.filePath(m_fileName));

  // Notify application
  emit downloadFinished(m_url, m_downloadDir.filePath(m_fileName));

  // Install the update
  m_reply->close();
  m_reply = nullptr;
  installUpdate();
  setVisible(false);
}

/**
 * @brief Opens the downloaded file using the system's default handler.
 *
 * @note If the downloaded file is not found, an error dialog is shown.
 */
void Downloader::openDownload()
{
  if (!m_fileName.isEmpty())
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_downloadDir.filePath(m_fileName)));
  else
    QMessageBox::critical(
      this, tr("Error"), tr("Cannot find downloaded update!"), QMessageBox::Close);
}

/**
 * @brief Instructs the OS to open the downloaded file as an installer.
 *
 * @note If @c useCustomInstallProcedures() returns @c true, this function does
 *       nothing. Use the QSimpleUpdater signals to implement your own install
 *       logic.
 */
void Downloader::installUpdate()
{
  if (useCustomInstallProcedures())
    return;

  // Update labels
  m_ui->stopButton->setText(tr("Close"));
  m_ui->downloadLabel->setText(tr("Download complete!"));
  m_ui->timeLabel->setText(tr("The installer will open separately") + "...");

  // Ask the user to install the download
  QMessageBox box;
  box.setIcon(QMessageBox::Question);
  box.setDefaultButton(QMessageBox::Ok);
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  box.setInformativeText(tr("Click \"OK\" to begin installing the update"));

  QString text = tr("In order to install the update, you may need to "
                    "quit the application.");

  if (m_mandatoryUpdate)
    text = tr("In order to install the update, you may need to "
              "quit the application. This is a mandatory update, exiting now "
              "will close the application.");

  box.setText("<h3>" + text + "</h3>");

  // User wants to install the download
  if (box.exec() == QMessageBox::Ok) {
    if (!useCustomInstallProcedures())
      openDownload();
  } else {
    if (m_mandatoryUpdate)
      QApplication::quit();

    m_ui->openButton->setEnabled(true);
    m_ui->openButton->setVisible(true);
    m_ui->timeLabel->setText(tr("Click the \"Open\" button to apply the update"));
  }
}

/**
 * @brief Prompts the user to cancel the download, and cancels it if confirmed.
 *
 * If the update is mandatory and the user cancels, the application will quit.
 */
void Downloader::cancelDownload()
{
  if (m_reply && !m_reply->isFinished()) {
    QMessageBox box;
    box.setWindowTitle(tr("Updater"));
    box.setIcon(QMessageBox::Question);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    QString text = tr("Are you sure you want to cancel the download?");
    if (m_mandatoryUpdate) {
      text = tr("Are you sure you want to cancel the download? This is a "
                "mandatory update, exiting now will close the application");
    }
    box.setText(text);

    if (box.exec() == QMessageBox::Yes) {
      hide();
      m_reply->abort();
      if (m_mandatoryUpdate)
        QApplication::quit();
    }
  } else {
    if (m_mandatoryUpdate)
      QApplication::quit();

    hide();
  }
}

/**
 * @brief Writes the downloaded data to the disk.
 */
void Downloader::saveFile(qint64 received, qint64 total)
{
  Q_UNUSED(received);
  Q_UNUSED(total);

  if (!m_reply)
    return;

  // Check if we need to redirect
  QUrl url = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (!url.isEmpty()) {
    startDownload(url);
    return;
  }

  // Save downloaded data to disk
  QFile file(m_downloadDir.filePath(m_fileName + PARTIAL_DOWN));
  if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
    file.write(m_reply->readAll());
    file.close();
  }
}

/**
 * @brief Calculates the appropriate size units (bytes, KB or MB) for the
 *        received data and total download size, then updates the UI labels.
 */
void Downloader::calculateSizes(qint64 received, qint64 total)
{
  QString totalSize;
  QString receivedSize;

  if (total < 1024)
    totalSize = tr("%1 bytes").arg(total);
  else if (total < 1048576)
    totalSize = tr("%1 KB").arg(round(total / 1024));
  else
    totalSize = tr("%1 MB").arg(round(total / 1048576));

  if (received < 1024)
    receivedSize = tr("%1 bytes").arg(received);
  else if (received < 1048576)
    receivedSize = tr("%1 KB").arg(received / 1024);
  else
    receivedSize = tr("%1 MB").arg(received / 1048576);

  m_ui->downloadLabel->setText(tr("Downloading updates") + " (" + receivedSize + " " + tr("of")
                               + " " + totalSize + ")");
}

/**
 * @brief Extracts the filename from the Content-Disposition HTTP header.
 */
void Downloader::metaDataChanged()
{
  if (!m_reply)
    return;

  QString filename = "";
  QVariant variant = m_reply->header(QNetworkRequest::ContentDispositionHeader);
  if (variant.isValid()) {
    QString contentDisposition = QByteArray::fromPercentEncoding(variant.toByteArray()).constData();
    QRegularExpression regExp(R"(filename=(\S+))");
    QRegularExpressionMatch match = regExp.match(contentDisposition);
    if (match.hasMatch())
      filename = match.captured(1);

    setFileName(filename);
  }
}

/**
 * @brief Updates the progress bar and triggers size/time calculations.
 */
void Downloader::updateProgress(qint64 received, qint64 total)
{
  if (total > 0) {
    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setMaximum(100);
    m_ui->progressBar->setValue((received * 100) / total);

    calculateSizes(received, total);
    calculateTimeRemaining(received, total);
    saveFile(received, total);
  }

  else {
    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setMaximum(0);
    m_ui->progressBar->setValue(-1);
    m_ui->downloadLabel->setText(tr("Downloading Updates") + "...");
    m_ui->timeLabel->setText(QString("%1: %2").arg(tr("Time Remaining")).arg(tr("Unknown")));
  }
}

/**
 * @brief Calculates and displays the estimated time remaining for the
 *        download.
 *
 * Uses the elapsed time and bytes received to estimate the remaining download
 * time, displaying it in hours, minutes, or seconds as appropriate.
 */
void Downloader::calculateTimeRemaining(qint64 received, qint64 total)
{
  uint difference = QDateTime::currentDateTime().toSecsSinceEpoch() - m_startTime;

  if (difference > 0) {
    QString timeString;
    qreal timeRemaining = (total - received) / (received / difference);

    if (timeRemaining > 7200) {
      timeRemaining /= 3600;
      int hours = int(timeRemaining + 0.5);

      if (hours > 1)
        timeString = tr("about %1 hours").arg(hours);
      else
        timeString = tr("about one hour");
    }

    else if (timeRemaining > 60) {
      timeRemaining /= 60;
      int minutes = int(timeRemaining + 0.5);

      if (minutes > 1)
        timeString = tr("%1 minutes").arg(minutes);
      else
        timeString = tr("1 minute");
    }

    else if (timeRemaining <= 60) {
      int seconds = int(timeRemaining + 0.5);

      if (seconds > 1)
        timeString = tr("%1 seconds").arg(seconds);
      else
        timeString = tr("1 second");
    }

    m_ui->timeLabel->setText(tr("Time remaining") + ": " + timeString);
  }
}

/**
 * @brief Handles HTTP authentication requests by showing a credentials dialog.
 */
void Downloader::authenticate(QNetworkReply* reply, QAuthenticator* authenticator)
{
  Q_UNUSED(reply);
  AuthenticateDialog dlg(this);
  dlg.setUserName(authenticator->user());
  dlg.setPassword(authenticator->password());
  if (dlg.exec()) {
    authenticator->setUser(dlg.userName());
    authenticator->setPassword(dlg.password());
  }
}

/**
 * @brief Rounds the given @a input to two decimal places.
 */
qreal Downloader::round(const qreal& input)
{
  return static_cast<qreal>(roundf(static_cast<float>(input) * 100) / 100);
}

/**
 * @brief Returns the current download directory path.
 */
QString Downloader::downloadDir() const
{
  return m_downloadDir.absolutePath();
}

/**
 * @brief Sets the download directory to the given @a downloadDir path.
 */
void Downloader::setDownloadDir(const QString& downloadDir)
{
  if (m_downloadDir.absolutePath() != downloadDir)
    m_downloadDir.setPath(downloadDir);
}

/**
 * @brief Sets whether the current update is mandatory.
 *
 * When mandatory, the application will quit if the user cancels the download
 * or declines installation.
 */
void Downloader::setMandatoryUpdate(const bool mandatory_update)
{
  m_mandatoryUpdate = mandatory_update;
}

/**
 * @brief Enables or disables custom install procedures.
 *
 * When enabled, the Downloader will not attempt to open the downloaded file.
 * Use the QSimpleUpdater signals to implement your own install logic.
 */
void Downloader::setUseCustomInstallProcedures(const bool custom)
{
  m_useCustomProcedures = custom;
}

#if QSU_INCLUDE_MOC
#  include "moc_Downloader.cpp"
#endif
