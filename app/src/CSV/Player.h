/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QFile>
#include <QObject>
#include <QVector>
#include <QKeyEvent>

namespace CSV
{
/**
 * @brief The Player class
 *
 * The CSV player class allows users to select a CSV file and "re-play" it
 * with Serial Studio.
 */
class Player : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(qreal progress
             READ progress
             NOTIFY timestampChanged)
  Q_PROPERTY(bool isPlaying
             READ isPlaying
             NOTIFY playerStateChanged)
  Q_PROPERTY(const QString& timestamp
             READ timestamp
             NOTIFY timestampChanged)
  // clang-format on

signals:
  void openChanged();
  void timestampChanged();
  void playerStateChanged();

private:
  explicit Player();
  Player(Player &&) = delete;
  Player(const Player &) = delete;
  Player &operator=(Player &&) = delete;
  Player &operator=(const Player &) = delete;

public:
  static Player &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] qreal progress() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;

  [[nodiscard]] QString filename() const;
  [[nodiscard]] QString csvFilesPath() const;
  [[nodiscard]] const QString &timestamp() const;

public slots:
  void play();
  void pause();
  void toggle();
  void openFile();
  void closeFile();
  void nextFrame();
  void previousFrame();
  void openFile(const QString &filePath);
  void setProgress(const qreal progress);

private slots:
  void updateData();

private:
  bool promptUserForDateTimeOrInterval();
  void generateDateTimeForRows(int interval);
  void convertColumnToDateTime(int columnIndex);

  QDateTime getDateTime(int row);
  QDateTime getDateTime(const QString &cell);

  QByteArray getFrame(const int row);

  const QString &getCellValue(const int row, const int column, bool &error);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  bool handleKeyPress(QKeyEvent *keyEvent);

private:
  int m_framePos;
  bool m_playing;
  QFile m_csvFile;
  QString m_timestamp;
  QList<QStringList> m_csvData;
};
} // namespace CSV
