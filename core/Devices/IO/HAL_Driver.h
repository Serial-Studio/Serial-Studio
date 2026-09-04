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
#include <QDebug>
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

/**
 * @brief One captured block of a dense typed sample stream (spec 0051): interleaved native
 *        samples plus timing metadata. Source owns time -- @c t0 is stamped at the capture
 *        boundary and per-sample times derive as t0 + i * dt, never re-stamped downstream.
 */
struct SampleBlock {
  using SteadyTimePoint = CapturedData::SteadyTimePoint;

  std::vector<float> samples;      ///< Interleaved samples (frame-major)
  int channels     = 1;            ///< Interleaved channel count
  qsizetype frames = 0;            ///< Sample frames in this block
  SteadyTimePoint t0;              ///< Capture time of the first frame
  std::chrono::nanoseconds dt{1};  ///< Per-frame step (1 / sample rate)
};

/**
 * @typedef SampleBlockPtr
 * @brief Shared immutable pointer to a @ref SampleBlock instance.
 */
typedef std::shared_ptr<const SampleBlock> SampleBlockPtr;

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
  void configurationChanged();
  void dataSent(const QByteArray& data);
  void dataReceived(const IO::CapturedDataPtr& data);
  void consoleDataReceived(const IO::CapturedDataPtr& data);
  void sampleBlockReceived(const IO::SampleBlockPtr& block);
  void openFinished(bool ok, const QString& reason);

public:
  explicit HAL_Driver(QObject* parent = nullptr) : QObject(parent), m_openReportArmed(false) {}

  virtual ~HAL_Driver() = default;

  void armOpenReport() { m_openReportArmed = true; }

  void disarmOpenReport() { m_openReportArmed = false; }

  [[nodiscard]] bool openReportArmed() const { return m_openReportArmed; }

  virtual void close()                               = 0;
  [[nodiscard]] virtual bool isOpen() const noexcept = 0;

  [[nodiscard]] virtual bool isConnecting() const noexcept { return false; }

  [[nodiscard]] virtual bool isStreamCapable() const noexcept { return false; }

  [[nodiscard]] virtual bool isReadable() const noexcept                   = 0;
  [[nodiscard]] virtual bool isWritable() const noexcept                   = 0;
  [[nodiscard]] virtual bool configurationOk() const noexcept              = 0;
  [[nodiscard]] virtual qint64 write(const QByteArray& data)               = 0;
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode)          = 0;
  [[nodiscard]] virtual QList<IO::DriverProperty> driverProperties() const = 0;

  [[nodiscard]] virtual QJsonObject deviceIdentifier() const { return {}; }

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
  /**
   * @brief Reports the outcome of the open attempt exactly once: the latch is armed by the
   *        connection manager before open() and disarmed on the first report, so a later
   *        established-link event can never masquerade as a dial verdict.
   */
  void reportOpenFinished(bool ok, const QString& reason = QString())
  {
    if (!m_openReportArmed)
      return;

    m_openReportArmed = false;
    Q_EMIT openFinished(ok, reason);
  }

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

  /**
   * @brief Publishes a terminal-only view of the incoming data. A stream-lane driver feeds the
   *        console through here: the typed sample blocks already carry the same samples to the
   *        dashboard and the sinks, so this text must never reach the FrameReader or the export
   *        fan-out or every sample would be recorded twice.
   */
  void publishConsoleData(
    QByteArray&& data,
    CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
    std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
    qsizetype logicalFramesHint             = 0)
  {
    Q_EMIT consoleDataReceived(
      makeCapturedData(std::move(data), timestamp, frameStep, logicalFramesHint));
  }

  void publishSampleBlock(const IO::SampleBlockPtr& block) { Q_EMIT sampleBlockReceived(block); }

  /**
   * @brief Logs a driver failure to the console. Drivers never raise modal dialogs: a modal pumps
   *        the event loop, so one raised from a connect or error stack lets queued work retire the
   *        very driver still on that stack (spec 0056).
   */
  static void logDriverError(const QString& title, const QString& text)
  {
    qWarning().noquote() << QStringLiteral("[%1] %2").arg(title, text);
  }

private:
  bool m_openReportArmed;
};

}  // namespace IO
