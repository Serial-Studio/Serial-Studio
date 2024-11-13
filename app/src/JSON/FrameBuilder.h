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
#include <QSettings>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include "SerialStudio.h"

#include "JSON/Frame.h"
#include "JSON/FrameParser.h"

namespace JSON
{
/**
 * @brief The FrameBuilder class
 *
 * The JSON frame builder class receives raw frame data from the I/O manager
 * class and generates a frame object that represents the project title,
 * the groups that compose the project and the datasets that compose each
 * group.
 *
 * This frame is later shared with the rest of the modules, and is updated
 * automatically with new incoming raw data.
 */
class FrameBuilder : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString jsonMapFilename
             READ jsonMapFilename
             NOTIFY jsonFileMapChanged)
  Q_PROPERTY(QString jsonMapFilepath
             READ jsonMapFilepath
             NOTIFY jsonFileMapChanged)
  Q_PROPERTY(SerialStudio::OperationMode operationMode
             READ operationMode
             WRITE setOperationMode
             NOTIFY operationModeChanged)
  // clang-format on

signals:
  void jsonFileMapChanged();
  void operationModeChanged();
  void frameChanged(const JSON::Frame &frame);

private:
  explicit FrameBuilder();
  FrameBuilder(FrameBuilder &&) = delete;
  FrameBuilder(const FrameBuilder &) = delete;
  FrameBuilder &operator=(FrameBuilder &&) = delete;
  FrameBuilder &operator=(const FrameBuilder &) = delete;

public:
  static FrameBuilder &instance();

  [[nodiscard]] QString jsonMapFilepath() const;
  [[nodiscard]] QString jsonMapFilename() const;
  [[nodiscard]] SerialStudio::OperationMode operationMode() const;

public slots:
  void loadJsonMap();
  void setupExternalConnections();
  void loadJsonMap(const QString &path);
  void setFrameParser(JSON::FrameParser *editor);
  void setOperationMode(const SerialStudio::OperationMode mode);

public slots:
  void setJsonPathSetting(const QString &path);

private slots:
  void readData(const QByteArray &data);

private:
  QFile m_jsonMap;
  JSON::Frame m_frame;
  QSettings m_settings;
  SerialStudio::OperationMode m_opMode;
  JSON::FrameParser *m_frameParser;
};
} // namespace JSON
