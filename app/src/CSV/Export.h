/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
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
