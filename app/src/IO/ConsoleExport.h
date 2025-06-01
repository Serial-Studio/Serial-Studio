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
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  void registerData(const QString &data);

private:
  QFile m_file;
  QString m_buffer;
  bool m_exportEnabled;
  QSettings m_settings;
  QTextStream m_textStream;
};
} // namespace IO
