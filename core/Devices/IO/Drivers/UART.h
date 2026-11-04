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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QByteArray>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QtSerialPort>
#include <QVariantMap>

#include "Core/Async/AsyncClock.h"
#include "Core/IO/ConnectionSequence.h"
#include "Core/IO/HAL_Driver.h"

namespace IO {
namespace Drivers {
class SequenceRunner;
class UartSequencePort;

/**
 * @brief HAL driver for serial (UART) ports via QSerialPort.
 */
class UART : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool autoReconnect
             READ autoReconnect
             WRITE setAutoReconnect
             NOTIFY autoReconnectChanged)
  Q_PROPERTY(bool dtrEnabled
             READ dtrEnabled
             WRITE setDtrEnabled
             NOTIFY dtrEnabledChanged)
  Q_PROPERTY(bool rtsEnabled
             READ rtsEnabled
             WRITE setRtsEnabled
             NOTIFY rtsEnabledChanged)
  Q_PROPERTY(qint32 liveBaudRate
             READ liveBaudRate
             NOTIFY liveBaudRateChanged)
  Q_PROPERTY(QString connectionSequenceJson
             READ connectionSequenceJson
             NOTIFY connectionSequenceChanged)
  Q_PROPERTY(quint8 portIndex
             READ portIndex
             WRITE setPortIndex
             NOTIFY portIndexChanged)
  Q_PROPERTY(quint8 parityIndex
             READ parityIndex
             WRITE setParity
             NOTIFY parityChanged)
  Q_PROPERTY(quint8 dataBitsIndex
             READ dataBitsIndex
             WRITE setDataBits
             NOTIFY dataBitsChanged)
  Q_PROPERTY(quint8 stopBitsIndex
             READ stopBitsIndex
             WRITE setStopBits
             NOTIFY stopBitsChanged)
  Q_PROPERTY(quint8 flowControlIndex
             READ flowControlIndex
             WRITE setFlowControl
             NOTIFY flowControlChanged)
  Q_PROPERTY(qint32 baudRate
             READ baudRate
             WRITE setBaudRate
             NOTIFY baudRateChanged)
  Q_PROPERTY(QStringList portList
             READ portList
             NOTIFY availablePortsChanged)
  Q_PROPERTY(QStringList parityList
             READ parityList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList baudRateList
             READ baudRateList
             NOTIFY baudRateChanged)
  Q_PROPERTY(QStringList dataBitsList
             READ dataBitsList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList stopBitsList
             READ stopBitsList
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList flowControlList
             READ flowControlList
             NOTIFY languageChanged)
  // clang-format on

signals:
  void portChanged();
  void parityChanged();
  void languageChanged();
  void baudRateChanged();
  void dataBitsChanged();
  void stopBitsChanged();
  void portIndexChanged();
  void dtrEnabledChanged();
  void rtsEnabledChanged();
  void flowControlChanged();
  void liveBaudRateChanged();
  void baudRateListChanged();
  void autoReconnectChanged();
  void baudRateIndexChanged();
  void availablePortsChanged();
  void connectionSequenceChanged();
  void connectionError(const QString& name);

public:
  explicit UART();
  ~UART();

  UART(UART&&)                 = delete;
  UART(const UART&)            = delete;
  UART& operator=(UART&&)      = delete;
  UART& operator=(const UART&) = delete;

  void close() override;
  void applyConnectionSettings(const QJsonObject& settings) override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QString exclusiveResource() const override;
  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;
  bool selectByIdentifier(const QJsonObject& id) override;

  [[nodiscard]] QSerialPort* port() const;
  [[nodiscard]] bool autoReconnect() const;

  [[nodiscard]] bool runSequence();
  [[nodiscard]] bool sendBreak(int ms);
  [[nodiscard]] bool setLiveBaudRate(qint32 rate);
  [[nodiscard]] QJsonObject lineStates() const;
  [[nodiscard]] QString connectionSequenceJson() const;
  Q_INVOKABLE [[nodiscard]] QVariantMap validateConnectionSequence(const QString& json) const;

  [[nodiscard]] bool dtrEnabled() const;
  [[nodiscard]] bool rtsEnabled() const;
  [[nodiscard]] quint8 portIndex() const;
  [[nodiscard]] quint8 parityIndex() const;
  [[nodiscard]] quint8 displayMode() const;
  [[nodiscard]] quint8 dataBitsIndex() const;
  [[nodiscard]] quint8 stopBitsIndex() const;
  [[nodiscard]] quint8 flowControlIndex() const;

  [[nodiscard]] QStringList portList() const;
  [[nodiscard]] QStringList baudRateList() const;

  [[nodiscard]] QStringList parityList() const;
  [[nodiscard]] QStringList dataBitsList() const;
  [[nodiscard]] QStringList stopBitsList() const;
  [[nodiscard]] QStringList flowControlList() const;

  [[nodiscard]] qint32 baudRate() const;
  [[nodiscard]] qint32 liveBaudRate() const;
  [[nodiscard]] QSerialPort::Parity parity() const;
  [[nodiscard]] QSerialPort::DataBits dataBits() const;
  [[nodiscard]] QSerialPort::StopBits stopBits() const;
  [[nodiscard]] QSerialPort::FlowControl flowControl() const;

public slots:
  void setupExternalConnections();
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setBaudRate(const qint32 rate);
  void setDtrEnabled(const bool enabled);
  void setRtsEnabled(const bool enabled);
  void setParity(const quint8 parityIndex);
  void setPortIndex(const quint8 portIndex);
  void registerDevice(const QString& device);
  void setDataBits(const quint8 dataBitsIndex);
  void setStopBits(const quint8 stopBitsIndex);
  void setAutoReconnect(const bool autoreconnect);
  void setFlowControl(const quint8 flowControlIndex);
  void setConnectionSequence(const QVariant& value);

private slots:
  void onReadyRead();
  void populateErrors();
  void pollAutoReconnect();
  void refreshSerialDevices();
  void handleError(QSerialPort::SerialPortError error);
  void onSequenceFinished(bool ok, const QString& reason, int stepIndex);

private:
  [[nodiscard]] bool portOpen() const noexcept;

  void releaseBreak();
  void stopSequence();
  void publishSequenceNote(const QString& text);
  void applyDeviceProperty(const QVariant& value);
  void applyBaudRateProperty(const QVariant& value);
  bool relocateOpenPortIndex(const QVector<QSerialPortInfo>& ports);
  void startSequence(const IO::ConnectionSequence& sequence, bool connectTime);

private:
  QSerialPort* m_port;

  bool m_dtrEnabled;
  bool m_autoReconnect;
  bool m_pendingReconnect;
  bool m_usingCustomSerialPort;
  bool m_rtsEnabled;
  bool m_breakHeld;
  bool m_sequenceConnecting;

  QString m_lastPortName;

  qint32 m_baudRate;
  QSerialPort::Parity m_parity;
  QSerialPort::DataBits m_dataBits;
  QSerialPort::StopBits m_stopBits;
  QSerialPort::FlowControl m_flowControl;

  quint8 m_portIndex;
  quint8 m_parityIndex;
  quint8 m_dataBitsIndex;
  quint8 m_stopBitsIndex;
  quint8 m_flowControlIndex;

  QTimer m_reconnectTimer;

  QSettings m_settings;
  QStringList m_deviceNames;
  QStringList m_customDevices;
  QStringList m_deviceLocations;

  QMap<QSerialPort::SerialPortError, QString> m_errorDescriptions;

  // Spec 0103: destroyed bottom-up, the runner before the port adapter and the clock it uses
  Async::TimerId m_breakTimer;
  IO::ConnectionSequence m_sequence;
  Async::SystemClock m_clock;
  std::unique_ptr<UartSequencePort> m_sequencePort;
  std::unique_ptr<SequenceRunner> m_runner;
};
}  // namespace Drivers
}  // namespace IO
