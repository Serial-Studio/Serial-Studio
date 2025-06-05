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

#include <QFileInfo>
#include <QFileDialog>
#include <QApplication>

#include "IO/Manager.h"
#include "Misc/Translator.h"
#include "IO/FileTransmission.h"

/**
 * Constructor function
 */
IO::FileTransmission::FileTransmission()
{
  // Set stream object pointer to null
  m_stream = nullptr;

  // Send a line to the serial device periodically
  m_timer.setInterval(100);
  m_timer.setTimerType(Qt::PreciseTimer);
  connect(&m_timer, &QTimer::timeout, this, &FileTransmission::sendLine);
}

/**
 * Destructor function
 */
IO::FileTransmission::~FileTransmission()
{
  closeFile();
}

/**
 * Returns the only instance of the class
 */
IO::FileTransmission &IO::FileTransmission::instance()
{
  static FileTransmission instance;
  return instance;
}

/**
 * Returns @c true if the application is currently transmitting a file through
 * the serial port.
 */
bool IO::FileTransmission::active() const
{
  return m_timer.isActive();
}

/**
 * Returns @c true if a file is currently selected for transmission and if the
 * serial port device is available.
 */
bool IO::FileTransmission::fileOpen() const
{
  return m_file.isOpen() && IO::Manager::instance().isConnected();
}

/**
 * Returns the name & extension of the currently selected file
 */
QString IO::FileTransmission::fileName() const
{
  if (!m_file.isOpen())
    return tr("No file selected...");

  return QFileInfo(m_file).fileName();
}

/**
 * Returns the file transmission progress in a range from 0 to 100
 */
int IO::FileTransmission::transmissionProgress() const
{
  // No file open or invalid size -> progress set to 0%
  if (!fileOpen() || m_file.size() <= 0 || !m_stream)
    return 0;

  // Return progress as percentage
  qreal txb = m_stream->pos();
  qreal len = m_file.size();
  return qMin(1.0, (txb / len)) * 100;
}

/**
 * Returns the number of milliseconds that the application should wait before
 * sending another line to the serial port device.
 */
int IO::FileTransmission::lineTransmissionInterval() const
{
  return m_timer.interval();
}

/**
 * Allows the user to select a file to send to the serial port.
 */
void IO::FileTransmission::openFile()
{
  // Let user select a file to open
  auto path = QFileDialog::getOpenFileName(
      Q_NULLPTR, tr("Select file to transmit"), QDir::homePath());

  // Filename is empty, abort
  if (path.isEmpty())
    return;

  // Close current file (if any)
  if (fileOpen())
    closeFile();

  // Try to open the file as read-only
  m_file.setFileName(path);
  if (m_file.open(QFile::ReadOnly))
  {
    m_stream = new QTextStream(&m_file);

    emit fileChanged();
    emit transmissionProgressChanged();
  }

  // Log open errors
  else
    qWarning() << "File open error" << m_file.errorString();
}

/**
 * Closes the currently selected file
 */
void IO::FileTransmission::closeFile()
{
  // Stop transmitting the file to the serial device
  stopTransmission();

  // Close current file
  if (m_file.isOpen())
    m_file.close();

  // Reset text stream handler
  if (m_stream)
  {
    delete m_stream;
    m_stream = nullptr;
  }

  // Emit signals to update the UI
  emit fileChanged();
  emit transmissionProgressChanged();
}

/**
 * Pauses the file transmission process
 */
void IO::FileTransmission::stopTransmission()
{
  m_timer.stop();
  emit activeChanged();
}

/**
 * Starts/resumes the file transmission process.ç
 *
 * @note If the file was already transmitted to the serial device, calling
 *       this function shall restart the file transmission process.
 */
void IO::FileTransmission::beginTransmission()
{
  // Only allow transmission if serial device is open
  if (IO::Manager::instance().isConnected())
  {
    // If file has already been sent, reset text stream position
    if (transmissionProgress() == 100)
    {
      m_stream->seek(0);
      emit transmissionProgressChanged();
    }

    // Start timer
    m_timer.start();
    emit activeChanged();
  }

  // Stop transmission if serial device is closed
  else
    stopTransmission();
}

/**
 * @brief Sets up external module connections for FileTransmission.
 */
void IO::FileTransmission::setupExternalConnections()
{
  // Stop transmission if device is disconnected
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &FileTransmission::stopTransmission);

  // Refresh UI when serial device connection status changes
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &FileTransmission::fileChanged);

  // Close file before application quits
  connect(qApp, &QApplication::aboutToQuit, this, &FileTransmission::closeFile);

  // Retranslate text automatically
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::FileTransmission::fileChanged);
}

/**
 * Changes the time interval to wait after sending one line from the
 * currently selected file.
 */
void IO::FileTransmission::setLineTransmissionInterval(const int msec)
{
  m_timer.setInterval(qMax(0, msec));
}

/**
 * Transmits a new line from the selected file to the serial port device.
 *
 * @note If EOF is reached, then the transmission process is automatically
 *       stopped
 */
void IO::FileTransmission::sendLine()
{
  // Transmission disabled, abort
  if (!active())
    return;

  // Device not open, abort
  if (!IO::Manager::instance().isConnected())
    return;

  // Send next line to device
  if (m_stream && !m_stream->atEnd())
  {
    QMetaObject::invokeMethod(this, [=] {
      if (m_stream)
      {
        auto line = m_stream->readLine();
        if (!line.isEmpty())
        {
          if (!line.endsWith("\n"))
            line.append("\n");

          IO::Manager::instance().writeData(line.toUtf8());
          emit transmissionProgressChanged();
        }
      }
    });
  }

  // Reached end of file, stop transmission
  else
    stopTransmission();
}
