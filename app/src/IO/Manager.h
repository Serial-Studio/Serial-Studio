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

#include <QObject>
#include <QElapsedTimer>
#include <IO/HAL_Driver.h>

namespace IO
{
/**
 * @brief The Manager class
 *
 * The I/O Manager class provides an abstraction layer between the physical
 * device (e.g. a serial or network port) and the rest of the application.
 *
 * Additionaly, all frame parsing, checksum verification and data processing
 * is done directly by this class. In this way, programmers can focus on
 * implementing the code to read and write data to a device, while the manager
 * class deals with the processing of the received data and generates frames.
 *
 * A "frame" is equivalent to a packet of data. To identify frames, the manager
 * class needs to know three things:
 * - The start sequence or header of the frame
 * - The end sequence or footer of the frame
 * - The data separator sequence, which allows Serial Studio to differentiate
 *   between a dataset value and another dataset value.
 *
 * Example of a frame:
 *
 *   $A,B,C,D,E,F,G;
 *
 * In this case, the start sequence of the frame is "$", while the end sequence
 * is ";". The data separator sequence is ",". Knowing this, Serial Studio
 * can process the frame and deduce that A,B,C,D,E,F and G are individual
 * dataset values.
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
  Q_PROPERTY(bool deviceAvailable
             READ deviceAvailable
             NOTIFY driverChanged)
  Q_PROPERTY(IO::Manager::SelectedDriver selectedDriver
             READ selectedDriver
             WRITE setSelectedDriver
             NOTIFY selectedDriverChanged)
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
  Q_PROPERTY(QStringList availableDrivers
             READ availableDrivers
             NOTIFY languageChanged)
  // clang-format on

signals:
  void driverChanged();
  void languageChanged();
  void connectedChanged();
  void writeEnabledChanged();
  void configurationChanged();
  void receivedBytesChanged();
  void maxBufferSizeChanged();
  void startSequenceChanged();
  void finishSequenceChanged();
  void selectedDriverChanged();
  void separatorSequenceChanged();
  void frameValidationRegexChanged();
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
  enum class SelectedDriver
  {
    Serial,
    Network,
    BluetoothLE
  };
  Q_ENUM(SelectedDriver)

  enum class ValidationStatus
  {
    FrameOk,
    ChecksumError,
    ChecksumIncomplete
  };
  Q_ENUM(ValidationStatus)

  static Manager &instance();

  [[nodiscard]] bool readOnly();
  [[nodiscard]] bool readWrite();
  [[nodiscard]] bool connected();
  [[nodiscard]] bool deviceAvailable();
  [[nodiscard]] bool configurationOk();

  [[nodiscard]] int maxBufferSize() const;

  [[nodiscard]] HAL_Driver *driver();
  [[nodiscard]] SelectedDriver selectedDriver() const;

  [[nodiscard]] const QString &startSequence() const;
  [[nodiscard]] const QString &finishSequence() const;
  [[nodiscard]] const QString &separatorSequence() const;

  [[nodiscard]] QStringList availableDrivers() const;
  Q_INVOKABLE qint64 writeData(const QByteArray &data);

public slots:
  void connectDevice();
  void toggleConnection();
  void disconnectDevice();
  void setWriteEnabled(const bool enabled);
  void processPayload(const QByteArray &payload);
  void setMaxBufferSize(const int maxBufferSize);
  void setStartSequence(const QString &sequence);
  void setFinishSequence(const QString &sequence);
  void setSeparatorSequence(const QString &sequence);
  void setSelectedDriver(const IO::Manager::SelectedDriver &driver);

private slots:
  void readFrames();
  void clearTempBuffer();
  void setDriver(HAL_Driver *driver);
  void onDataReceived(const QByteArray &data);

private:
  void readStartEndDelimetedFrames();
  void readEndDelimetedFrames(const QList<QByteArray> &delimeters);

private:
  ValidationStatus integrityChecks(const QByteArray &frame,
                                   const QByteArray &masterBuffer,
                                   int *bytesToChop);

private:
  bool m_enableCrc;
  bool m_writeEnabled;
  int m_maxBufferSize;
  HAL_Driver *m_driver;
  QByteArray m_dataBuffer;
  quint64 m_receivedBytes;
  QString m_startSequence;
  QString m_finishSequence;
  QString m_separatorSequence;
  SelectedDriver m_selectedDriver;
};
} // namespace IO
