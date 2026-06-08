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

#include "Misc/Utilities.h"

#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QSettings>
#include <QSpacerItem>
#include <QUrl>

#include "AppInfo.h"

//--------------------------------------------------------------------------------------------------
// Platform-specific hooks
//--------------------------------------------------------------------------------------------------

#ifdef Q_OS_MACOS
extern int Misc_Utilities_showNativeMessageBox(const QString& text,
                                               const QString& informativeText,
                                               QMessageBox::Icon icon,
                                               const QString& windowTitle,
                                               QMessageBox::StandardButtons bt,
                                               QMessageBox::StandardButton defaultButton,
                                               const ButtonTextMap& buttonTexts);
#endif

//--------------------------------------------------------------------------------------------------
// Singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the singleton Utilities instance.
 */
Misc::Utilities& Misc::Utilities::instance()
{
  static Utilities singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Application control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restarts the application by relaunching the executable.
 */
void Misc::Utilities::rebootApplication()
{
  qApp->processEvents();
  QSettings().sync();

  QString exe      = QCoreApplication::applicationFilePath();
  QStringList args = QCoreApplication::arguments().mid(1);
  QProcess::startDetached(exe, args);

  qApp->exit();
}

//--------------------------------------------------------------------------------------------------
// Image handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a @c QPixmap object with the appropiate resolution for the screen.
 */
QPixmap Misc::Utilities::getHiDpiPixmap(const QString& path)
{
  return QPixmap(hdpiImagePath(path));
}

//--------------------------------------------------------------------------------------------------
// User dialogs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a @c file path for the image with the appropiate resolution for the screen.
 */
QString Misc::Utilities::hdpiImagePath(const QString& path)
{
  const auto dpr  = qApp->devicePixelRatio();
  const int ratio = qMin<int>(2, static_cast<int>(ceil(dpr)));

  QString filename;
  auto list            = path.split(".");
  const auto extension = list.last();
  for (int i = 0; i < list.count() - 1; ++i)
    filename.append(list.at(i));

  filename.append(QStringLiteral("@"));
  filename.append(QString::number(ratio));
  filename.append(QStringLiteral("x."));
  filename.append(extension);

  return filename;
}

/**
 * @brief Shows a macOS-like message box with the given properties.
 */
int Misc::Utilities::showMessageBox(const QString& text,
                                    const QString& informativeText,
                                    QMessageBox::Icon icon,
                                    const QString& windowTitle,
                                    QMessageBox::StandardButtons bt,
                                    QMessageBox::StandardButton defaultButton,
                                    const ButtonTextMap& buttonTexts)
{
  if (qApp->platformName() == QLatin1String("offscreen"))
    return QMessageBox::Ok;

#ifdef Q_OS_MACOS
  return Misc_Utilities_showNativeMessageBox(
    text, informativeText, icon, windowTitle, bt, defaultButton, buttonTexts);
#endif

  QMessageBox box;
  box.setStandardButtons(bt);
  box.setWindowTitle(windowTitle);
  box.setWindowFlag(Qt::WindowStaysOnTopHint, true);
  if (defaultButton != QMessageBox::NoButton)
    box.setDefaultButton(defaultButton);

  box.setText(QStringLiteral("<h3>") + text + QStringLiteral("</h3>"));
  box.setInformativeText(informativeText);

  if (icon == QMessageBox::NoIcon)
    box.setIconPixmap(getHiDpiPixmap(":/logo/small-icon.png"));
  else
    box.setIcon(icon);

  // code-verify off
  if (bt & QMessageBox::Ok)
    box.button(QMessageBox::Ok)->setText(tr("Ok"));
  if (bt & QMessageBox::Save)
    box.button(QMessageBox::Save)->setText(tr("Save"));
  if (bt & QMessageBox::SaveAll)
    box.button(QMessageBox::SaveAll)->setText(tr("Save all"));
  if (bt & QMessageBox::Open)
    box.button(QMessageBox::Open)->setText(tr("Open"));
  if (bt & QMessageBox::Yes)
    box.button(QMessageBox::Yes)->setText(tr("Yes"));
  if (bt & QMessageBox::YesToAll)
    box.button(QMessageBox::YesToAll)->setText(tr("Yes to all"));
  if (bt & QMessageBox::No)
    box.button(QMessageBox::No)->setText(tr("No"));
  if (bt & QMessageBox::NoToAll)
    box.button(QMessageBox::NoToAll)->setText(tr("No to all"));
  if (bt & QMessageBox::Abort)
    box.button(QMessageBox::Abort)->setText(tr("Abort"));
  if (bt & QMessageBox::Retry)
    box.button(QMessageBox::Retry)->setText(tr("Retry"));
  if (bt & QMessageBox::Ignore)
    box.button(QMessageBox::Ignore)->setText(tr("Ignore"));
  if (bt & QMessageBox::Close)
    box.button(QMessageBox::Close)->setText(tr("Close"));
  if (bt & QMessageBox::Cancel)
    box.button(QMessageBox::Cancel)->setText(tr("Cancel"));
  if (bt & QMessageBox::Discard)
    box.button(QMessageBox::Discard)->setText(tr("Discard"));
  if (bt & QMessageBox::Help)
    box.button(QMessageBox::Help)->setText(tr("Help"));
  if (bt & QMessageBox::Apply)
    box.button(QMessageBox::Apply)->setText(tr("Apply"));
  if (bt & QMessageBox::Reset)
    box.button(QMessageBox::Reset)->setText(tr("Reset"));
  if (bt & QMessageBox::RestoreDefaults)
    box.button(QMessageBox::RestoreDefaults)->setText(tr("Restore defaults"));
  // code-verify on

  for (auto it = buttonTexts.constBegin(); it != buttonTexts.constEnd(); ++it)
    box.button(it.key())->setText(" " + it.value() + " ");

  auto* spacer = new QSpacerItem(320, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  auto* layout = qobject_cast<QGridLayout*>(box.layout());
  layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());

  return box.exec();
}

/**
 * @brief Displays the standard "About Qt" dialog.
 */
void Misc::Utilities::aboutQt()
{
  qApp->aboutQt();
}

/**
 * @brief Copies the given @a text to the system clipboard.
 */
void Misc::Utilities::copyText(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reveals the given file in the host file manager (Explorer/Finder/desktop).
 */
void Misc::Utilities::revealFile(const QString& pathToReveal)
{
#if defined(Q_OS_WIN)
  QStringList param;
  const QFileInfo fileInfo(pathToReveal);
  if (!fileInfo.isDir())
    param += QLatin1String("/select,");

  param += QDir::toNativeSeparators(fileInfo.absoluteFilePath());
  QProcess::startDetached("explorer.exe", param);
#elif defined(Q_OS_MAC)
  QString escaped = pathToReveal;
  escaped.replace(QLatin1String("\\"), QLatin1String("\\\\"));
  escaped.replace(QLatin1String("\""), QLatin1String("\\\""));
  QStringList scriptArgs;
  scriptArgs
    << QLatin1String("-e")
    << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(escaped);
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
  scriptArgs.clear();
  scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
  QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
  QDesktopServices::openUrl(QUrl::fromLocalFile(pathToReveal));
#endif
}
