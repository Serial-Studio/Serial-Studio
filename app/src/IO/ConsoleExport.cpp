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

#include "ConsoleExport.h"

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>

#include "IO/Console.h"
#include "IO/Manager.h"
#include "Misc/Utilities.h"
#include "Misc/TimerEvents.h"

/**
 * Constructor function, configures the path in which Serial Studio shall
 * automatically write generated console log files.
 */
IO::ConsoleExport::ConsoleExport()
  : m_exportEnabled(true)
{
  m_filePath = QStringLiteral("%1/%2/Console")
                   .arg(QStandardPaths::writableLocation(
                            QStandardPaths::DocumentsLocation),
                        qApp->applicationDisplayName());
}

/**
 * Close file & finnish write-operations before destroying the class.
 */
IO::ConsoleExport::~ConsoleExport()
{
  closeFile();
}

/**
 * Returns a pointer to the only instance of this class.
 */
IO::ConsoleExport &IO::ConsoleExport::instance()
{
  static ConsoleExport instance;
  return instance;
}

/**
 * Returns @c true if the console output file is open.
 */
bool IO::ConsoleExport::isOpen() const
{
  return m_file.isOpen();
}

/**
 * Returns @c true if console log export is enabled.
 */
bool IO::ConsoleExport::exportEnabled() const
{
  return m_exportEnabled;
}

/**
 * Write all remaining console data & close the output file.
 */
void IO::ConsoleExport::closeFile()
{
  if (isOpen())
  {
    if (m_buffer.size() > 0)
      writeData();

    m_file.close();
    m_textStream.setDevice(nullptr);

    Q_EMIT openChanged();
  }
}

/**
 * Configures the signal/slot connections with the modules of the application
 * that this module depends upon.
 */
void IO::ConsoleExport::setupExternalConnections()
{
  connect(&IO::Console::instance(), &IO::Console::displayString, this,
          &IO::ConsoleExport::registerData);
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &IO::ConsoleExport::closeFile);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &IO::ConsoleExport::writeData);
}

/**
 * Enables or disables data export.
 */
void IO::ConsoleExport::setExportEnabled(const bool enabled)
{
  m_exportEnabled = enabled;
  Q_EMIT enabledChanged();

  if (!exportEnabled() && isOpen())
  {
    m_buffer.clear();
    closeFile();
  }
}

/**
 * Writes current buffer data to the output file, and creates a new file
 * if needed.
 */
void IO::ConsoleExport::writeData()
{
  // Device not connected, abort
  if (!IO::Manager::instance().connected())
    return;

  // Export is disabled, abort
  if (!exportEnabled())
    return;

  // Write data to the file
  if (m_buffer.size() > 0)
  {
    // Create a new file if required
    if (!isOpen())
      createFile();

    // Write data to hard disk
    if (m_textStream.device())
    {
      m_textStream << m_buffer;
      m_textStream.flush();
      m_buffer.clear();
    }
  }
}

/**
 * Creates a new console log output file based on the current date/time.
 */
void IO::ConsoleExport::createFile()
{
  // Close current file (if any)
  if (isOpen())
    closeFile();

  // Get filename
  const auto dateTime = QDateTime::currentDateTime();
  const auto fileName
      = dateTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"))
        + QStringLiteral(".txt");

  // Generate file path if required
  QDir dir(m_filePath);
  if (!dir.exists())
    dir.mkpath(QStringLiteral("."));

  // Open file
  m_file.setFileName(dir.filePath(fileName));
  if (!m_file.open(QIODeviceBase::WriteOnly | QIODevice::Text))
  {
    Misc::Utilities::showMessageBox(tr("Console Output File Error"),
                                    tr("Cannot open file for writing!"));
    closeFile();
    return;
  }

  // Configure text stream
  m_textStream.setDevice(&m_file);
  m_textStream.setGenerateByteOrderMark(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  m_textStream.setCodec("UTF-8");
#else
  m_textStream.setEncoding(QStringConverter::Utf8);
#endif

  // Emit signals
  Q_EMIT openChanged();
}

/**
 * Appends the given console data to the output buffer.
 */
void IO::ConsoleExport::registerData(const QString &data)
{
  if (!data.isEmpty() && exportEnabled())
    m_buffer.append(data);
}
