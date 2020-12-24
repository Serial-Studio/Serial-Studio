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

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <QFile>
#include <QObject>
#include <QQmlEngine>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

class JsonParser : public QObject
{
   Q_OBJECT
   Q_PROPERTY(QString jsonMapFilename READ jsonMapFilename NOTIFY jsonFileMapChanged)
   Q_PROPERTY(QString jsonMapFilepath READ jsonMapFilepath NOTIFY jsonFileMapChanged)
   Q_PROPERTY(OperationMode operationMode READ operationMode WRITE setOperationMode NOTIFY operationModeChanged)

signals:
   void packetReceived();
   void jsonFileMapChanged();
   void operationModeChanged();

public:
   enum OperationMode
   {
      kManual,
      kAutomatic,
   };
   Q_ENUMS(OperationMode)

public:
   static JsonParser *getInstance();

   QByteArray jsonMapData() const;
   QJsonDocument document() const;
   QString jsonMapFilename() const;
   QString jsonMapFilepath() const;
   OperationMode operationMode() const;

public slots:
   void loadJsonMap();
   void loadJsonMap(const QString &path);
   void setOperationMode(const OperationMode mode);

private:
   JsonParser();

private slots:
   void readData(const QByteArray &data);

private:
   QFile m_jsonMap;
   OperationMode m_opMode;
   QJsonDocument m_document;
   QByteArray m_jsonMapData;
};

#endif
