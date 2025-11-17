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

#pragma once

#include <QThread>
#include <QObject>
#include <QSettings>
#include <QPointer>
#include <QKeyEvent>

#include "SerialStudio.h"
#include "IO/HAL_Driver.h"
#include "IO/FrameReader.h"

namespace IO
{
/**
 * @class IO::Manager
 * @brief Central manager for I/O operations across multiple protocols.
 *
 * Handles communication with devices over Serial, Network, and Bluetooth LE,
 * managing configuration, connection, and data transfer.
 *
 * Integrates with `FrameReader` for parsing data streams and ensures
 * thread-safe operation using a dedicated worker thread.
 */
class Manager : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool readOnly
             READ readOnly
             NOTIFY connectedChanged)
  Q_PROPERTY(bool readWrite
             READ readWrite
             NOTIFY connectedChanged)
  Q_PROPERTY(bool isConnected
             READ isConnected
             NOTIFY connectedChanged)
  Q_PROPERTY(bool paused
             READ paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(bool threadedFrameExtraction
             READ threadedFrameExtraction
             WRITE setThreadedFrameExtraction
             NOTIFY threadedFrameExtractionChanged)
  Q_PROPERTY(SerialStudio::BusType busType
             READ busType
             WRITE setBusType
             NOTIFY busTypeChanged)
  Q_PROPERTY(QByteArray startSequence
             READ startSequence
             WRITE setStartSequence
             NOTIFY startSequenceChanged)
  Q_PROPERTY(QByteArray finishSequence
             READ finishSequence
             WRITE setFinishSequence
             NOTIFY finishSequenceChanged)
  Q_PROPERTY(QString checksumAlgorithm
             READ checksumAlgorithm
             WRITE setChecksumAlgorithm
             NOTIFY checksumAlgorithmChanged)
  Q_PROPERTY(bool configurationOk
             READ configurationOk
             NOTIFY configurationChanged)
  Q_PROPERTY(QStringList availableBuses
             READ availableBuses
             NOTIFY busListChanged)
  // clang-format on

signals:
  void driverChanged();
  void pausedChanged();
  void busTypeChanged();
  void busListChanged();
  void connectedChanged();
  void writeEnabledChanged();
  void configurationChanged();
  void maxBufferSizeChanged();
  void startSequenceChanged();
  void finishSequenceChanged();
  void checksumAlgorithmChanged();
  void threadedFrameExtractionChanged();

private:
  explicit Manager();
  Manager(Manager &&) = delete;
  Manager(const Manager &) = delete;
  Manager &operator=(Manager &&) = delete;
  Manager &operator=(const Manager &) = delete;

  ~Manager();

public:
  static Manager &instance();

  [[nodiscard]] bool paused();
  [[nodiscard]] bool readOnly();
  [[nodiscard]] bool readWrite();
  [[nodiscard]] bool isConnected();
  [[nodiscard]] bool configurationOk();
  [[nodiscard]] bool threadedFrameExtraction();

  [[nodiscard]] HAL_Driver *driver();
  [[nodiscard]] SerialStudio::BusType busType() const;

  [[nodiscard]] const QByteArray &startSequence() const;
  [[nodiscard]] const QByteArray &finishSequence() const;
  [[nodiscard]] const QString &checksumAlgorithm() const;

  [[nodiscard]] QStringList availableBuses() const;
  Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void resetFrameReader();
  void setupExternalConnections();
  void setPaused(const bool paused);
  void setDriver(IO::HAL_Driver *driver);
  void setWriteEnabled(const bool enabled);
  void processPayload(const QByteArray &payload);
  void setStartSequence(const QByteArray &sequence);
  void setFinishSequence(const QByteArray &sequence);
  void setChecksumAlgorithm(const QString &algorithm);
  void setThreadedFrameExtraction(const bool enabled);
  void setBusType(const SerialStudio::BusType &driver);

private:
  void killFrameReader();
  void startFrameReader();

  void onReadyRead();
  void onDataReceived(const QByteArray &data);

private:
  bool m_paused;
  bool m_writeEnabled;
  bool m_thrFrameExtr;
  SerialStudio::BusType m_busType;

  HAL_Driver *m_driver;
  QThread m_workerThread;
  QPointer<FrameReader> m_frameReader;

  QByteArray m_frame;
  QByteArray m_startSequence;
  QByteArray m_finishSequence;

  QSettings m_settings;
  QString m_checksumAlgorithm;
};
} // namespace IO
