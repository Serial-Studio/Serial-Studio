/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

#include "CsvPlayer.h"
#include "JsonParser.h"

/*
 * Only instance of the class
 */
static CsvPlayer *INSTANCE = nullptr;

/**
 * Shows a macOS-like message box with the given properties
 */
static int NiceMessageBox(QString text, QString informativeText, QString windowTitle = qAppName(),
                          QMessageBox::StandardButtons buttons = QMessageBox::Ok)
{
   auto icon = QPixmap(":/images/icon.png").scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

   QMessageBox box;
   box.setIconPixmap(icon);
   box.setWindowTitle(windowTitle);
   box.setStandardButtons(buttons);
   box.setText("<h3>" + text + "</h3>");
   box.setInformativeText(informativeText);

   return box.exec();
}

CsvPlayer::CsvPlayer() { }

CsvPlayer *CsvPlayer::getInstance()
{
   if (!INSTANCE)
      INSTANCE = new CsvPlayer;

   return INSTANCE;
}

bool CsvPlayer::isOpen() { }

int CsvPlayer::progress() const { }

bool CsvPlayer::isPlaying() const { }

QString CsvPlayer::timestamp() const { }

void CsvPlayer::play() { }

void CsvPlayer::pause() { }

void CsvPlayer::toggle() { }

void CsvPlayer::openFile()
{
   // Check that manual JSON mode is activaded
   auto opMode = JsonParser::getInstance()->operationMode();
   auto jsonOpen = !JsonParser::getInstance()->jsonMapData().isEmpty();
   if (opMode != JsonParser::kManual || !jsonOpen)
   {
      NiceMessageBox(tr("Invalid configuration for CSV player"),
                     tr("You need to select a JSON map file in order to use this feature"));
      return;
   }

   // Get file name
   auto file
       = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Select CSV file"), QDir::homePath(), tr("CSV files (*.csv)"));

   // File name empty, abort
   if (file.isEmpty())
      return;

   // Close previous file
   if (isOpen())
      closeFile();

   // Try to open the current file
}

void CsvPlayer::closeFile() { }

void CsvPlayer::nextFrame() { }

void CsvPlayer::previousFrame() { }
