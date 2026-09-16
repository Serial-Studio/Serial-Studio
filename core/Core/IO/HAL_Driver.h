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
#include <utility>

#include "Core/SSAssert.h"

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

namespace IO {

/**
 * @brief One visibility rule of a driver property: the row shows while the sibling property named
 *        @c key holds one of @c values. A property's rules are ANDed.
 */
struct DriverPropertyRule {
  QString key;
  QVariantList values;
};

/**
 * @brief Describes a configurable property exposed by an I/O driver. The list a driver returns is
 *        also what a project persists, so a row a mode does not use is HIDDEN by visibleWhen,
 *        never left out.
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
  QList<DriverPropertyRule> visibleWhen;

  /**
   * @brief Adds a rule: the row shows while the sibling @p siblingKey holds one of @p shownFor.
   */
  void showWhen(const QString& siblingKey, QVariantList shownFor)
  {
    visibleWhen.append({siblingKey, std::move(shownFor)});
  }
};

/**
 * @brief Whether @p current denotes the setting @p expected names: a checkbox stores a bool and a
 *        combo an index, so the comparison follows the rule value's kind, not the QVariant type.
 */
[[nodiscard]] inline bool driverValuesMatch(const QVariant& current, const QVariant& expected)
{
  SS_ASSERT_LOG(expected.isValid());
  if (expected.typeId() == QMetaType::Bool)
    return current.toBool() == expected.toBool();

  if (expected.canConvert<qlonglong>() && expected.typeId() != QMetaType::QString)
    return current.toLongLong() == expected.toLongLong();

  return current.toString() == expected.toString();
}

/**
 * @brief Whether @p prop's row shows given its siblings in @p props: every rule must name a sibling
 *        holding one of the rule's values; a rule naming no sibling hides nothing.
 */
[[nodiscard]] inline bool driverPropertyVisible(const DriverProperty& prop,
                                                const QList<DriverProperty>& props)
{
  SS_ASSERT_LOG(!prop.key.isEmpty());
  for (const auto& rule : prop.visibleWhen) {
    const auto sibling = std::find_if(
      props.cbegin(), props.cend(), [&rule](const auto& other) { return other.key == rule.key; });
    if (sibling == props.cend())
      continue;

    const bool held = std::any_of(rule.values.cbegin(), rule.values.cend(), [&](const auto& v) {
      return driverValuesMatch(sibling->value, v);
    });
    if (!held)
      return false;
  }

  return true;
}

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
  explicit HAL_Driver(QObject* parent = nullptr)
    : QObject(parent), m_openReportArmed(false), m_bus(nullptr)
  {}

  virtual ~HAL_Driver() = default;

  /**
   * @brief Adopts the root-owned message bus a driver raises notifications on (spec 0077 T30).
   *        Attached once by the factory or registry that built the driver; a driver never
   *        reaches for the bus singleton.
   */
  void attachMessageBus(Core::Bus::MessageBus& bus)
  {
    SS_ASSERT(m_bus == nullptr, return);
    m_bus = &bus;
  }

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

  /**
   * @brief Names the exclusive device this driver claims, empty when it claims none or is not
   *        configured. A project pointing two sources at one such device is refused before any
   *        of them opens it: the second open would otherwise fail with a driver-level busy error
   *        that names neither source. Serial ports answer with their canonical port name.
   */
  [[nodiscard]] virtual QString exclusiveResource() const { return {}; }

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
  [[nodiscard]] Core::Bus::MessageBus* messageBus() const noexcept { return m_bus; }

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
  Core::Bus::MessageBus* m_bus;
};

}  // namespace IO
