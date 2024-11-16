/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QFile>
#include <QVector>
#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QTextStream>
#include <QJsonObject>

#include "JSON/Frame.h"

namespace CSV
{
/**
 * @brief The Export class
 *
 * The CSV export class receives data from the @c IO::Manager class and
 * exports the received frames into a CSV file selected by the user.
 *
 * CSV-data is generated periodically each time the @c Misc::TimerEvents
 * low-frequency timer expires (e.g. every 1 second). The idea behind this
 * is to allow exporting data, but avoid freezing the application when serial
 * data is received continuously.
 */
typedef struct
{
  JSON::Frame data;
  QDateTime rxDateTime;
} TimestampFrame;

class Export : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  // clang-format on

signals:
  void openChanged();
  void enabledChanged();

private:
  explicit Export();
  Export(Export &&) = delete;
  Export(const Export &) = delete;
  Export &operator=(Export &&) = delete;
  Export &operator=(const Export &) = delete;

  ~Export();

public:
  static Export &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void openCurrentCsv();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);

private slots:
  void writeValues();
  void registerFrame(const JSON::Frame &frame);

private:
  QVector<QPair<int, QString>> createCsvFile(const CSV::TimestampFrame &frame);

private:
  QFile m_csvFile;
  QString m_csvPath;
  bool m_exportEnabled;
  QTextStream m_textStream;
  QVector<TimestampFrame> m_frames;
};
} // namespace CSV
