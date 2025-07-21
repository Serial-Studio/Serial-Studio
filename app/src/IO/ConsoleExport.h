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
#include <QObject>
#include <QSettings>
#include <QTextStream>

namespace IO
{
class ConsoleExport : public QObject
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
  explicit ConsoleExport();
  ConsoleExport(ConsoleExport &&) = delete;
  ConsoleExport(const ConsoleExport &) = delete;
  ConsoleExport &operator=(ConsoleExport &&) = delete;
  ConsoleExport &operator=(const ConsoleExport &) = delete;

  ~ConsoleExport();

public:
  static ConsoleExport &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);

private slots:
  void writeData();
  void createFile();
  void registerData(QStringView data);

private:
  QFile m_file;
  QString m_buffer;
  bool m_exportEnabled;
  QSettings m_settings;
  QTextStream m_textStream;
};
} // namespace IO
