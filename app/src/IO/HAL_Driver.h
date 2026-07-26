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

/**
 * @brief Describes a configurable property exposed by an I/O driver.
 */
struct DriverProperty {
  /**
   * @enum Type
   * @brief Supported editor/widget types for a driver property.
   */
  enum Type {
    Text,
    HexText,
    IntField,
    FloatField,
    CheckBox,
    ComboBox,
    Password,
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

/**
 * @brief Represents a block of acquired raw data and its timing metadata.
 */
struct CapturedData {
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  QByteArray data;
  SteadyTimePoint timestamp;
  std::chrono::nanoseconds frameStep = std::chrono::nanoseconds(1);
  qsizetype logicalFramesHint        = 0;
};

/**
 * @typedef CapturedDataPtr
 * @brief Shared immutable pointer to a @ref CapturedData instance.
 */
typedef std::shared_ptr<const CapturedData> CapturedDataPtr;

[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  const QByteArray& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint             = 0) noexcept
{
  auto captured               = std::make_shared<CapturedData>();
  captured->data              = data;
  captured->timestamp         = timestamp;
  captured->frameStep         = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  QByteArray&& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint             = 0) noexcept
{
  auto captured               = std::make_shared<CapturedData>();
  captured->data              = std::move(data);
  captured->timestamp         = timestamp;
  captured->frameStep         = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

/**
 * @brief Abstract interface for all I/O drivers.
 */
class HAL_Driver : public QObject {
  Q_OBJECT

signals:
  void linkDropped();
  void configurationChanged();
  void dataSent(const QByteArray& data);
  void openFinished(bool ok, const QString& reason);
  void dataReceived(const IO::CapturedDataPtr& data);

public:
  explicit HAL_Driver(QObject* parent = nullptr) : QObject(parent) {}

  virtual ~HAL_Driver() = default;

  virtual void close()                                                     = 0;
  [[nodiscard]] virtual bool isOpen() const noexcept                       = 0;
  [[nodiscard]] virtual bool isReadable() const noexcept                   = 0;
  [[nodiscard]] virtual bool isWritable() const noexcept                   = 0;
  [[nodiscard]] virtual bool configurationOk() const noexcept              = 0;
  [[nodiscard]] virtual qint64 write(const QByteArray& data)               = 0;
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode)          = 0;
  [[nodiscard]] virtual QList<IO::DriverProperty> driverProperties() const = 0;

  [[nodiscard]] virtual QJsonObject deviceIdentifier() const { return {}; }

  /**
   * @brief Returns whether this driver opens through an orchestrated flow. False keeps the
   *        driver on the synchronous open() path it has always used.
   */
  [[nodiscard]] virtual bool supportsAsyncOpen() const noexcept { return false; }

  /**
   * @brief Returns the ceiling on one open attempt, or zero to take the shared default. A driver
   *        whose handshake legitimately outlasts a socket dial raises it here instead of being
   *        cut off mid-attempt.
   */
  [[nodiscard]] virtual int openTimeoutMsec() const noexcept { return 0; }

  /**
   * @brief Starts an open attempt. The base implementation is today's synchronous open,
   *        reported immediately, so a driver that opts in without overriding this behaves
   *        exactly as it does now.
   */
  virtual void beginOpen(const QIODevice::OpenMode mode)
  {
    const bool ok = open(mode);
    Q_EMIT openFinished(ok, ok ? QString() : QStringLiteral("driver reported open failure"));
  }

  /**
   * @brief Stops an open attempt that is still in flight; closing is what every driver
   *        already does to release a half-open link.
   */
  virtual void abortOpen() { close(); }

  virtual bool selectByIdentifier(const QJsonObject& id)
  {
    Q_UNUSED(id);
    return false;
  }

  virtual void applyConnectionSettings(const QJsonObject& settings)
  {
    if (settings.isEmpty())
      return;

    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
      setDriverProperty(it.key(), it.value().toVariant());

    const auto deviceIdVal = settings.value(QStringLiteral("deviceId"));
    if (deviceIdVal.isObject())
      (void)selectByIdentifier(deviceIdVal.toObject());
  }

public slots:
  virtual void setDriverProperty(const QString& key, const QVariant& value) = 0;

protected:
  void publishReceivedData(
    const QByteArray& data,
    CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
    std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
    qsizetype logicalFramesHint             = 0)
  {
    Q_EMIT dataReceived(makeCapturedData(data, timestamp, frameStep, logicalFramesHint));
  }

  void publishReceivedData(
    QByteArray&& data,
    CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
    std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
    qsizetype logicalFramesHint             = 0)
  {
    Q_EMIT dataReceived(makeCapturedData(std::move(data), timestamp, frameStep, logicalFramesHint));
  }
};

}  // namespace IO
