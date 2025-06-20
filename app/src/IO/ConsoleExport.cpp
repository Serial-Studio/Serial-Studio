/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file contains both open-source and proprietary sections.
 * Portions enabled via preprocessor macros (e.g., BUILD_COMMERCIAL)
 * are part of Serial Studio's commercial feature set and may only be
 * used under the terms of a valid Serial Studio Commercial License.
 *
 * You may use, modify, or distribute this file under the terms of
 * the GNU General Public License (GPLv3) **only if** you:
 *   - Do NOT compile or enable any gated commercial features
 *   - Comply fully with the license terms defined in LICENSE.md
 *
 * Attempting to use Pro features without activation or a valid license
 * is a breach of this file’s licensing terms and may result in legal action.
 *
 * For full details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Mixed
 */

#include "ConsoleExport.h"

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopServices>

#include "Misc/Utilities.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Console.h"
#  include "IO/Manager.h"
#  include "Misc/TimerEvents.h"
#  include "Misc/WorkspaceManager.h"
#  include "Licensing/LemonSqueezy.h"
#endif

/**
 * Constructor function, configures the path in which Serial Studio shall
 * automatically write generated console log files.
 */
IO::ConsoleExport::ConsoleExport()
  : m_exportEnabled(false)
{
#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this, [=] {
            if (exportEnabled()
                && !Licensing::LemonSqueezy::instance().isActivated())
              setExportEnabled(false);
          });
#endif

  setExportEnabled(m_settings.value("ConsoleExport", false).toBool());
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
#ifdef BUILD_COMMERCIAL
  return m_file.isOpen();
#else
  return false;
#endif
}

/**
 * Returns @c true if console log export is enabled.
 */
bool IO::ConsoleExport::exportEnabled() const
{
#ifdef BUILD_COMMERCIAL
  return m_exportEnabled;
#else
  return false;
#endif
}

/**
 * Write all remaining console data & close the output file.
 */
void IO::ConsoleExport::closeFile()
{
#ifdef BUILD_COMMERCIAL
  if (isOpen())
  {
    if (m_buffer.size() > 0)
      writeData();

    m_file.close();
    m_textStream.setDevice(nullptr);

    Q_EMIT openChanged();
  }
#endif
}

/**
 * Configures the signal/slot connections with the modules of the application
 * that this module depends upon.
 */
void IO::ConsoleExport::setupExternalConnections()
{
#ifdef BUILD_COMMERCIAL
  connect(&IO::Console::instance(), &IO::Console::displayString, this,
          &IO::ConsoleExport::registerData);
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &IO::ConsoleExport::closeFile);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &IO::ConsoleExport::writeData);
#endif
}

/**
 * Enables or disables data export.
 */
void IO::ConsoleExport::setExportEnabled(const bool enabled)
{
#ifdef BUILD_COMMERCIAL
  if (Licensing::LemonSqueezy::instance().isActivated())
  {
    m_exportEnabled = enabled;
    Q_EMIT enabledChanged();

    if (!exportEnabled() && isOpen())
    {
      m_buffer.clear();
      closeFile();
    }

    m_settings.setValue("ConsoleExport", m_exportEnabled);
    return;
  }
#endif

  closeFile();
  m_buffer.clear();
  m_exportEnabled = false;
  m_settings.setValue("ConsoleExport", false);
  Q_EMIT enabledChanged();

  if (enabled)
    Misc::Utilities::showMessageBox(
        tr("Console Export is a Pro feature."),
        tr("This feature requires a license. Please "
           "purchase one to enable console export."));
}

/**
 * Writes current buffer data to the output file, and creates a new file
 * if needed.
 */
void IO::ConsoleExport::writeData()
{
#ifdef BUILD_COMMERCIAL
  // Device not connected, abort
  if (!IO::Manager::instance().isConnected())
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
#endif
}

/**
 * Creates a new console log output file based on the current date/time.
 */
void IO::ConsoleExport::createFile()
{
#ifdef BUILD_COMMERCIAL
  // Only enable this feature is program is activated
  if (Licensing::LemonSqueezy::instance().isActivated())
  {
    // Close current file (if any)
    if (isOpen())
      closeFile();

    // Get filename
    const auto dateTime = QDateTime::currentDateTime();
    const auto fileName
        = dateTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"))
          + QStringLiteral(".txt");

    // Get console export path
    QDir dir(Misc::WorkspaceManager::instance().path("Console"));

    // Open file
    m_file.setFileName(dir.filePath(fileName));
    if (!m_file.open(QIODeviceBase::WriteOnly | QIODevice::Text))
    {
      Misc::Utilities::showMessageBox(tr("Console Output File Error"),
                                      tr("Cannot open file for writing!"),
                                      QMessageBox::Critical);
      closeFile();
      return;
    }

    // Configure text stream
    m_textStream.setDevice(&m_file);
    m_textStream.setGenerateByteOrderMark(true);
#  if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_textStream.setCodec("UTF-8");
#  else
    m_textStream.setEncoding(QStringConverter::Utf8);
#  endif

    // Emit signals
    Q_EMIT openChanged();
  }
#endif
}

/**
 * Appends the given console data to the output buffer.
 */
void IO::ConsoleExport::registerData(const QString &data)
{
#ifdef BUILD_COMMERCIAL
  if (!data.isEmpty() && exportEnabled())
    m_buffer.append(data);
#else
  (void)data;
#endif
}
