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
  double txb = m_stream->pos();
  double len = m_file.size();
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
  auto *dialog = new QFileDialog(nullptr, tr("Select file to transmit"),
                                 QDir::homePath());

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            dialog->deleteLater();

            if (path.isEmpty())
              return;

            if (fileOpen())
              closeFile();

            m_file.setFileName(path);
            if (m_file.open(QFile::ReadOnly))
            {
              m_stream = new QTextStream(&m_file);

              emit fileChanged();
              emit transmissionProgressChanged();
            }
            else
            {
              qWarning() << "File open error:" << m_file.errorString();
            }
          });

  dialog->open();
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
    QMetaObject::invokeMethod(this, [=, this] {
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
