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
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QFile>
#include <QObject>
#include <QSettings>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QReadWriteLock>

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
  [[nodiscard]] const JSON::Frame &frame() const;
  [[nodiscard]] JSON::FrameParser *frameParser() const;
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
  void onConnectedChanged();
  void readData(const QByteArray &data);

private:
  QFile m_jsonMap;
  JSON::Frame m_frame;
  QSettings m_settings;
  JSON::FrameParser *m_frameParser;
  SerialStudio::OperationMode m_opMode;

  mutable QReadWriteLock m_dataLock;
};
} // namespace JSON
