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
    : m_filePos(0) // Initialize file position
{
  // Set stream object pointer to null
  //m_stream = nullptr; commented this out becasue no longer using Qtextstream so no need mstream anymore

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
  return m_file.isOpen() && IO::Manager::instance().connected();
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
  if (!fileOpen() || m_file.size() <= 0)
    return 0;

  // Use our internal file pointer position vs. total size
  qreal sent = static_cast<qreal>(m_filePos);
  qreal total = static_cast<qreal>(m_file.size());
  qreal ratio = (total == 0) ? 0 : (sent / total);

         // Return progress as a percentage
  return static_cast<int>(ratio * 100);

  /* Return progress as percentage - REMOVED BECAUSE NO LONGER USING mstream in binary file transmission
  qreal txb = m_stream->pos();
  qreal len = m_file.size();
  return qMin(1.0, (txb / len)) * 100;
  */
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

  // Try to open the file as read-only (binary by default)
  m_file.setFileName(path);
  if (m_file.open(QFile::ReadOnly))
  {
    // Reset position
    m_filePos = 0;

    emit fileChanged();
    emit transmissionProgressChanged();
  }
  else
  {
    qWarning() << "File open error" << m_file.errorString();
  }
}


  /* Try to open the file as read-only - COMMENTED OUT BECAUSE THIS WAS FOR TEXT FILE
  m_file.setFileName(path);
  if (m_file.open(QFile::ReadOnly))
  {
    m_stream = new QTextStream(&m_file); //QTextStream only good for reading and writing file as text not for raw binary data

    emit fileChanged();
    emit transmissionProgressChanged();
  }
*/

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

  // Reset position
  m_filePos = 0;

  /* Reset text stream handler - COMMENTED OUT BECAUSE NOT USING QSTREAM ANYMORE IN BINARY FILE UPLOAD
  if (m_stream)
  {
    delete m_stream;
    m_stream = nullptr;
  }
*/
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
  if (IO::Manager::instance().connected())
  {
    // If file has already been sent, reset text stream position
    if (transmissionProgress() == 100)
    {
      m_filePos = 0;
      //m_stream->seek(0); REMOVED CAUSE NO LONGER USING MSTREAM IN BINARY FILE UPLOAD
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
  if (!IO::Manager::instance().connected())
    return;

  // We’ll send data in chunks rather than lines - FOR BINARY
  const qint64 chunkSize = 64; // adjust as needed
  if (!m_file.isOpen())
    return;

  // Check if we’re at the end
  if (m_filePos >= m_file.size())
  {
    // Reached end of file, stop transmission
    stopTransmission();
    return;
  }

  // Read from current position
  m_file.seek(m_filePos);

  QByteArray data = m_file.read(chunkSize);
  if (!data.isEmpty())
  {
    // Write raw bytes directly to the serial port
    IO::Manager::instance().writeData(data);

           // Update position
    m_filePos += data.size();

           // Update progress
    emit transmissionProgressChanged();
  }
  /* Send next line to device - REMOVED BECAUSE NO LONGER USING MSTREAM
  if (m_stream && !m_stream->atEnd())
  {
    QMetaObject::invokeMethod(
        this,
        [=] {
          if (m_stream)
          {
            auto line = m_stream->readLine(); //readLine() is only good for reading and writing file as text not for raw binary data
            if (!line.isEmpty())
            {
              //if (!line.endsWith("\n"))
                //line.append("\n");

              IO::Manager::instance().writeData(line.toUtf8()); // UTF* cannot read
              emit transmissionProgressChanged();
            }
          }
        },
        Qt::QueuedConnection);
  }*/

  // Reached end of file, stop transmission
  else
    stopTransmission();
}
