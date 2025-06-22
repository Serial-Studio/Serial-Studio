/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);

private slots:
  void writeValues();
  void registerFrame(const JSON::Frame &frame);

private:
  QVector<QPair<int, QString>> createCsvFile(const CSV::TimestampFrame &frame);

private:
  QFile m_csvFile;
  bool m_exportEnabled;
  QTextStream m_textStream;
  QVector<TimestampFrame> m_frames;
};
} // namespace CSV
