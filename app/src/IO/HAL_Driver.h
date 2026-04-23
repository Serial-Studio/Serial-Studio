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

#include <algorithm>
#include <chrono>
#include <memory>
#include <QIODevice>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>

namespace IO {

struct DriverProperty {
  enum Type {
    Text,
    HexText,
    IntField,
    FloatField,
    CheckBox,
    ComboBox,
  };

  QString key;
  QString label;
  QString description;
  Type type = Text;
  QVariant value;
  QStringList options;
  QVariant min;
  QVariant max;
};

typedef std::shared_ptr<const QByteArray> ByteArrayPtr;

struct CapturedData {
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  IO::ByteArrayPtr data;
  SteadyTimePoint timestamp;
  std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1);
  qsizetype logicalFramesHint        = 0;
};

typedef std::shared_ptr<const CapturedData> CapturedDataPtr;

[[nodiscard]] inline ByteArrayPtr makeByteArray(const QByteArray& data) noexcept
{
  return std::make_shared<const QByteArray>(data);
}

[[nodiscard]] inline ByteArrayPtr makeByteArray(QByteArray&& data) noexcept
{
  return std::make_shared<const QByteArray>(std::move(data));
}

[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  const QByteArray& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint = 0) noexcept
{
  auto captured             = std::make_shared<CapturedData>();
  captured->data            = makeByteArray(data);
  captured->timestamp       = timestamp;
  captured->frameStep       = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  QByteArray&& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint = 0) noexcept
{
  auto captured             = std::make_shared<CapturedData>();
  captured->data            = makeByteArray(std::move(data));
  captured->timestamp       = timestamp;
  captured->frameStep       = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

/**
 * @class HAL_Driver
 * @brief Abstract interface for every I/O driver (UART, Network, BLE, Audio,
 *        Modbus, CANBus, HID, USB, Process). Drivers own the timestamp stamped
 *        at acquisition and publish shared `CapturedDataPtr` via
 *        `publishReceivedData()`.
 */
class HAL_Driver : public QObject {
  Q_OBJECT

signals:
  void configurationChanged();
  void dataSent(const QByteArray& data);
  void dataReceived(const IO::CapturedDataPtr& data);

public:
  explicit HAL_Driver(QObject* parent = nullptr) : QObject(parent) {}
  virtual ~HAL_Driver() = default;

  virtual void close() = 0;

  [[nodiscard]] virtual bool isOpen() const noexcept          = 0;
  [[nodiscard]] virtual bool isReadable() const noexcept      = 0;
  [[nodiscard]] virtual bool isWritable() const noexcept      = 0;
  [[nodiscard]] virtual bool configurationOk() const noexcept = 0;

  [[nodiscard]] virtual qint64 write(const QByteArray& data)       = 0;
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode)  = 0;

  [[nodiscard]] virtual QList<IO::DriverProperty> driverProperties() const = 0;

  [[nodiscard]] virtual QJsonObject deviceIdentifier() const { return {}; }

  virtual bool selectByIdentifier(const QJsonObject& id)
  {
    Q_UNUSED(id);
    return false;
  }

public slots:
  virtual void setDriverProperty(const QString& key, const QVariant& value) = 0;

protected:
  void publishReceivedData(const QByteArray& data,
                           CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
                           std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1),
                           qsizetype logicalFramesHint = 0)
  {
    Q_EMIT dataReceived(makeCapturedData(data, timestamp, frameStep, logicalFramesHint));
  }

  void publishReceivedData(QByteArray&& data,
                           CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
                           std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1),
                           qsizetype logicalFramesHint = 0)
  {
    Q_EMIT dataReceived(makeCapturedData(
      std::move(data), timestamp, frameStep, logicalFramesHint));
  }
};
}  // namespace IO
