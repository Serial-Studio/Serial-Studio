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

#ifndef DOWNLOAD_DIALOG_H
#define DOWNLOAD_DIALOG_H

#include <QDir>
#include <QDialog>
#include <ui_Downloader.h>

namespace Ui
{
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
  void downloadFinished(const QString &url, const QString &filepath);

public:
  explicit Downloader(QWidget *parent = 0);
  ~Downloader();

  bool useCustomInstallProcedures() const;

  QString downloadDir() const;
  void setDownloadDir(const QString &downloadDir);

public slots:
  void setUrlId(const QString &url);
  void startDownload(const QUrl &url);
  void setFileName(const QString &file);
  void setUserAgentString(const QString &agent);
  void setUseCustomInstallProcedures(const bool custom);
  void setMandatoryUpdate(const bool mandatory_update);

private slots:
  void finished();
  void openDownload();
  void installUpdate();
  void cancelDownload();
  void saveFile(qint64 received, qint64 total);
  void calculateSizes(qint64 received, qint64 total);
  void updateProgress(qint64 received, qint64 total);
  void calculateTimeRemaining(qint64 received, qint64 total);

private:
  qreal round(const qreal &input);

private:
  QString m_url;
  uint m_startTime;
  QDir m_downloadDir;
  QString m_fileName;
  Ui::Downloader *m_ui;
  QNetworkReply *m_reply;
  QString m_userAgentString;

  bool m_useCustomProcedures;
  bool m_mandatoryUpdate;

  QNetworkAccessManager *m_manager;
};

#endif
