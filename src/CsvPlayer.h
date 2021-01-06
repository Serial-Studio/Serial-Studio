/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef DATA_REPLAY_H
#define DATA_REPLAY_H

#include <QFile>
#include <QTimer>
#include <QObject>
#include <QJsonDocument>

class CsvPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ isOpen NOTIFY openChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY timestampChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playerStateChanged)
    Q_PROPERTY(QString timestamp READ timestamp NOTIFY timestampChanged)

signals:
    void openChanged();
    void timestampChanged();
    void playerStateChanged();

public:
    static CsvPlayer *getInstance();

    bool isOpen() const;
    qreal progress() const;
    bool isPlaying() const;
    int frameCount() const;
    QString filename() const;
    int framePosition() const;
    QString timestamp() const;

private:
    CsvPlayer();

public slots:
    void play();
    void pause();
    void toggle();
    void openFile();
    void closeFile();
    void nextFrame();
    void previousFrame();
    void setProgress(const qreal progress);

private slots:
    void updateData();

private:
    bool validateRow(const int row);
    QJsonDocument getJsonFrame(const int row);
    QString getCellValue(int row, int cell, bool *error = nullptr);

private:
    int m_framePos;
    bool m_playing;
    QFile m_csvFile;
    QTimer m_frameTimer;
    QString m_timestamp;
    QList<QStringList> m_csvData;
    QMap<QString, int> m_datasetIndexes;
    QMap<QString, QSet<QString>> m_model;
};

#endif
