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

#include <QThread>
#include <QObject>

#include "HAL_Driver.h"
#include "FrameReader.h"
#include "SerialStudio.h"

namespace IO
{
/**
 * @class IO::Manager
 * @brief Centralized manager for I/O operations across multiple communication
 * protocols.
 *
 * The `IO::Manager` class orchestrates the communication between various
 * devices and protocols, such as serial ports, network connections, and
 * Bluetooth Low Energy (BLE). It acts as the primary interface for configuring,
 * connecting, and interacting with hardware devices, and it manages data
 * transmission and reception efficiently.
 *
 * Key Features:
 * - **Multiple Bus Types**: Supports Serial, Network, and Bluetooth LE
 *   communication.
 * - **Dynamic Driver Management**: Automatically handles driver initialization,
 *   configuration, and cleanup when switching between bus types.
 * - **Frame Parsing**: Integrates with `FrameReader` to process and parse
 *   incoming data streams based on configurable start and end sequences or
 *   delimiters.
 * - **Thread Safety**: Executes frame processing in a dedicated worker thread,
 *   ensuring responsiveness and isolation from the main application thread.
 * - **Signals**: Emits various signals to notify changes in configuration,
 *   connection state, and data transmission/reception.
 *
 * @signals
 * - `connectedChanged()`: Emitted when the connection state changes.
 * - `driverChanged()`: Emitted when the driver instance is updated.
 * - `configurationChanged()`: Emitted when the configuration settings are
 *   modified.
 * - `dataSent(const QByteArray &data)`: Emitted when data is successfully sent
 *    to the device.
 * - `dataReceived(const QByteArray &data)`: Emitted when raw data is received.
 * - `frameReceived(const QByteArray &frame)`: Emitted when a valid frame is
 *   parsed.
 * - `busTypeChanged()`: Emitted when the bus type is changed.
 * - `startSequenceChanged()`: Emitted when the start sequence for frame
 *   detection is updated.
 * - `finishSequenceChanged()`: Emitted when the finish sequence for frame
 *   detection is updated.
 * - `separatorSequenceChanged()`: Emitted when the separator sequence is
 *   updated.
 * - `writeEnabledChanged()`: Emitted when the write-enabled flag is modified.
 * - `busListChanged()`: Emitted when the list of available buses changes.
 *
 * This class is designed to simplify I/O operations and promote modularity by
 * abstracting low-level driver interactions, making it easy to extend support
 * for additional protocols and devices.
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
  Q_PROPERTY(bool connected
             READ connected
             NOTIFY connectedChanged)
  Q_PROPERTY(SerialStudio::BusType busType
             READ busType
             WRITE setBusType
             NOTIFY busTypeChanged)
  Q_PROPERTY(QString startSequence
             READ startSequence
             WRITE setStartSequence
             NOTIFY startSequenceChanged)
  Q_PROPERTY(QString finishSequence
             READ finishSequence
             WRITE setFinishSequence
             NOTIFY finishSequenceChanged)
  Q_PROPERTY(QString separatorSequence
             READ separatorSequence
             WRITE setSeparatorSequence
             NOTIFY separatorSequenceChanged)
  Q_PROPERTY(bool configurationOk
             READ configurationOk
             NOTIFY configurationChanged)
  Q_PROPERTY(QStringList availableBuses
             READ availableBuses
             NOTIFY busListChanged)
  // clang-format on

signals:
  void driverChanged();
  void busTypeChanged();
  void busListChanged();
  void connectedChanged();
  void writeEnabledChanged();
  void configurationChanged();
  void maxBufferSizeChanged();
  void startSequenceChanged();
  void finishSequenceChanged();
  void separatorSequenceChanged();
  void dataSent(const QByteArray &data);
  void dataReceived(const QByteArray &data);
  void frameReceived(const QByteArray &frame);

private:
  explicit Manager();
  Manager(Manager &&) = delete;
  Manager(const Manager &) = delete;
  Manager &operator=(Manager &&) = delete;
  Manager &operator=(const Manager &) = delete;

public:
  static Manager &instance();

  [[nodiscard]] bool readOnly();
  [[nodiscard]] bool readWrite();
  [[nodiscard]] bool connected();
  [[nodiscard]] bool configurationOk();

  [[nodiscard]] HAL_Driver *driver();
  [[nodiscard]] SerialStudio::BusType busType() const;

  [[nodiscard]] const QString &startSequence() const;
  [[nodiscard]] const QString &finishSequence() const;
  [[nodiscard]] const QString &separatorSequence() const;

  [[nodiscard]] QStringList availableBuses() const;
  Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void setupExternalConnections();
  void setWriteEnabled(const bool enabled);
  void processPayload(const QByteArray &payload);
  void setStartSequence(const QString &sequence);
  void setFinishSequence(const QString &sequence);
  void setSeparatorSequence(const QString &sequence);
  void setBusType(const SerialStudio::BusType &driver);

private slots:
  void setDriver(HAL_Driver *driver);

private:
  bool m_writeEnabled;
  SerialStudio::BusType m_busType;

  HAL_Driver *m_driver;
  QThread m_workerThread;
  FrameReader m_frameReader;

  QString m_startSequence;
  QString m_finishSequence;
  QString m_separatorSequence;
};
} // namespace IO
