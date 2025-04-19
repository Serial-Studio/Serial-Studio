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

#include <QDir>
#include <QUrl>
#include <QPalette>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QAbstractButton>
#include <QDesktopServices>

#include <QSpacerItem>
#include <QGridLayout>

#include "AppInfo.h"
#include "Misc/Utilities.h"

/**
 * Returns a pointer to the only instance of the class
 */
Misc::Utilities &Misc::Utilities::instance()
{
  static Utilities singleton;
  return singleton;
}

/**
 * Restarts the application - with macOS specific code to make it work
 */
void Misc::Utilities::rebootApplication()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  qApp->exit();
  QProcess::startDetached(qApp->arguments().first(), qApp->arguments());
#else
#  ifdef Q_OS_MAC
  auto bundle = qApp->applicationDirPath() + "/../../";
  QProcess::startDetached("open", {"-n", "-a", bundle});
#  else
  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
#  endif
  qApp->exit();
#endif
}

/**
 * @brief Returns a @c QPixmap object with the appropiate resolution for the
 * screen.
 *
 * Returns a @c QPixmap object that loads the image at the given @a path. In the
 * case that the application is being executed on a HiDPI screen, the the scaled
 * version of the image will be automatically loaded.
 *
 * @param path location of the image to load
 */
QPixmap Misc::Utilities::getHiDpiPixmap(const QString &path)
{
  const auto dpr = qApp->devicePixelRatio();
  const int ratio = qMin<int>(2, static_cast<int>(ceil(dpr)));

  QString filename;
  auto list = path.split(".");
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
 * Asks the user if he/she wants the application to check for updates
 * automatically
 */
bool Misc::Utilities::askAutomaticUpdates()
{
  const int result = showMessageBox(
      tr("Check for updates automatically?"),
      tr("Should %1 automatically check for updates? "
         "You can always check for updates manually from "
         "the \"About\" dialog")
          .arg(APP_NAME),
      QMessageBox::NoIcon, APP_NAME, QMessageBox::Yes | QMessageBox::No);
  return result == QMessageBox::Yes;
}

/**
 * Shows a macOS-like message box with the given properties
 */
int Misc::Utilities::showMessageBox(
    const QString &text, const QString &informativeText, QMessageBox::Icon icon,
    const QString &windowTitle, QMessageBox::StandardButtons bt,
    QMessageBox::StandardButton defaultButton, const ButtonTextMap &buttonTexts)
{

  // Create message box & set options
  QMessageBox box;
  box.setStandardButtons(bt);
  box.setWindowTitle(windowTitle);
  box.setWindowFlag(Qt::WindowStaysOnTopHint, true);
  if (defaultButton != QMessageBox::NoButton)
    box.setDefaultButton(defaultButton);

  // Set title & informative text
  box.setText(QStringLiteral("<h3>") + text + QStringLiteral("</h3>"));
  box.setInformativeText(informativeText);

  // Set icon
  if (icon == QMessageBox::NoIcon)
    box.setIconPixmap(getHiDpiPixmap(":/rcc/images/icon-small.png"));
  else
    box.setIcon(icon);

  // Add button translations
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

  // Change text in buttons
  for (auto it = buttonTexts.constBegin(); it != buttonTexts.constEnd(); ++it)
    box.button(it.key())->setText(" " + it.value() + " ");

  // Resize message box
  // clang-format off
  auto *spacer = new QSpacerItem(320, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  auto *layout = qobject_cast<QGridLayout *>(box.layout());
  layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());
  // clang-format on

  // Show message box & return user decision to caller
  return box.exec();
}

/**
 * Displays the about Qt dialog
 */
void Misc::Utilities::aboutQt()
{
  qApp->aboutQt();
}

/**
 * Reveals the file contained in @a pathToReveal in Explorer/Finder.
 * On GNU/Linux, this function shall open the file directly with the desktop
 * services.
 *
 * Hacking details:
 * http://stackoverflow.com/questions/3490336/how-to-reveal-in-finder-or-show-in-explorer-with-qt
 */
void Misc::Utilities::revealFile(const QString &pathToReveal)
{
#if defined(Q_OS_WIN)
  QStringList param;
  const QFileInfo fileInfo(pathToReveal);
  if (!fileInfo.isDir())
    param += QLatin1String("/select,");
  param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
  QProcess::startDetached("explorer.exe", param);
#elif defined(Q_OS_MAC)
  QStringList scriptArgs;
  scriptArgs << QLatin1String("-e")
             << QString::fromLatin1(
                    "tell application \"Finder\" to reveal POSIX file \"%1\"")
                    .arg(pathToReveal);
  QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
  scriptArgs.clear();
  scriptArgs << QLatin1String("-e")
             << QLatin1String("tell application \"Finder\" to activate");
  QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
  QDesktopServices::openUrl(QUrl::fromLocalFile(pathToReveal));
#endif
}
