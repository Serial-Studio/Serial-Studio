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

/**
 * @struct DriverProperty
 * @brief Describes a configurable property exposed by an I/O driver.
 *
 * Driver properties are typically used by the UI or configuration layer to
 * present editable settings such as text fields, numeric inputs, checkboxes,
 * or combo boxes.
 */
struct DriverProperty {
  /**
   * @enum Type
   * @brief Supported editor/widget types for a driver property.
   */
  enum Type {
    Text,       /**< Plain text input. */
    HexText,    /**< Hexadecimal text input. */
    IntField,   /**< Integer numeric input. */
    FloatField, /**< Floating-point numeric input. */
    CheckBox,   /**< Boolean checkbox input. */
    ComboBox,   /**< Drop-down list input. */
  };

  QString key;         /**< Internal property key used to identify the setting. */
  QString label;       /**< Human-readable label shown in the user interface. */
  QString description; /**< Optional explanatory text for the property. */
  Type type = Text;    /**< Input/editor type used to edit this property. */
  QVariant value;      /**< Current value of the property. */
  QStringList options; /**< Available options when @ref type is @ref ComboBox. */
  QVariant min;        /**< Optional minimum value for numeric properties. */
  QVariant max;        /**< Optional maximum value for numeric properties. */
};

/**
 * @typedef ByteArrayPtr
 * @brief Shared immutable byte buffer.
 *
 * This alias wraps a @c QByteArray in a shared pointer to enable efficient
 * ownership sharing without copying the payload.
 */
typedef std::shared_ptr<const QByteArray> ByteArrayPtr;

/**
 * @struct CapturedData
 * @brief Represents a block of acquired raw data and its timing metadata.
 *
 * Instances of this structure are emitted by drivers when new data is received.
 * In addition to the raw payload, it carries the acquisition timestamp and
 * optional frame timing hints for downstream processing.
 */
struct CapturedData {
  using SteadyClock = std::chrono::steady_clock;   /**< Monotonic clock type used for timestamps. */
  using SteadyTimePoint = SteadyClock::time_point; /**< Timestamp type based on @ref SteadyClock. */

  IO::ByteArrayPtr data;     /**< Shared immutable payload buffer. */
  SteadyTimePoint timestamp; /**< Time at which the payload was acquired. */
  std::chrono::nanoseconds frameStep =
    std::chrono::nanoseconds(1); /**< Time step between logical frames. */
  qsizetype logicalFramesHint =
    0; /**< Optional hint describing how many logical frames are contained in @ref data. */
};

/**
 * @typedef CapturedDataPtr
 * @brief Shared immutable pointer to a @ref CapturedData instance.
 */
typedef std::shared_ptr<const CapturedData> CapturedDataPtr;

/**
 * @brief Creates a shared immutable byte array from a copied buffer.
 * @param data Source byte array to copy into shared storage.
 * @return Shared pointer containing an immutable copy of @p data.
 */
[[nodiscard]] inline ByteArrayPtr makeByteArray(const QByteArray& data) noexcept
{
  return std::make_shared<const QByteArray>(data);
}

/**
 * @brief Creates a shared immutable byte array from a moved buffer.
 * @param data Source byte array whose contents are moved into shared storage.
 * @return Shared pointer containing the moved byte array.
 */
[[nodiscard]] inline ByteArrayPtr makeByteArray(QByteArray&& data) noexcept
{
  return std::make_shared<const QByteArray>(std::move(data));
}

/**
 * @brief Creates a shared @ref CapturedData object from a copied payload.
 * @param data Raw payload to copy.
 * @param timestamp Acquisition timestamp associated with the payload.
 * @param frameStep Time step between logical frames in the payload.
 * @param logicalFramesHint Optional number of logical frames represented by the payload.
 * @return Shared pointer to an immutable @ref CapturedData object.
 *
 * The @p frameStep value is clamped to a minimum of 1 nanosecond.
 */
[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  const QByteArray& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint             = 0) noexcept
{
  auto captured               = std::make_shared<CapturedData>();
  captured->data              = makeByteArray(data);
  captured->timestamp         = timestamp;
  captured->frameStep         = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

/**
 * @brief Creates a shared @ref CapturedData object from a moved payload.
 * @param data Raw payload to move.
 * @param timestamp Acquisition timestamp associated with the payload.
 * @param frameStep Time step between logical frames in the payload.
 * @param logicalFramesHint Optional number of logical frames represented by the payload.
 * @return Shared pointer to an immutable @ref CapturedData object.
 *
 * The @p frameStep value is clamped to a minimum of 1 nanosecond.
 */
[[nodiscard]] inline CapturedDataPtr makeCapturedData(
  QByteArray&& data,
  CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
  std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
  qsizetype logicalFramesHint             = 0) noexcept
{
  auto captured               = std::make_shared<CapturedData>();
  captured->data              = makeByteArray(std::move(data));
  captured->timestamp         = timestamp;
  captured->frameStep         = std::max(std::chrono::nanoseconds(1), frameStep);
  captured->logicalFramesHint = logicalFramesHint;
  return captured;
}

/**
 * @class HAL_Driver
 * @brief Abstract interface for all I/O drivers.
 *
 * This class defines the common contract implemented by transport backends such
 * as UART, Network, BLE, Audio, Modbus, CANBus, HID, USB, and Process drivers.
 *
 * Drivers are responsible for:
 * - opening and closing the transport,
 * - reporting capability and configuration state,
 * - exposing configurable properties,
 * - sending raw data,
 * - publishing acquired data with acquisition timestamps.
 *
 * Incoming data is emitted through @ref dataReceived as shared
 * @ref CapturedDataPtr objects.
 */
class HAL_Driver : public QObject {
  Q_OBJECT

signals:
  /**
   * @brief Emitted when the driver configuration changes.
   *
   * This signal can be used to refresh user interface state or trigger
   * validation of dependent settings.
   */
  void configurationChanged();

  /**
   * @brief Emitted after data is sent by the driver.
   * @param data Raw payload that was transmitted.
   */
  void dataSent(const QByteArray& data);

  /**
   * @brief Emitted when the driver receives data.
   * @param data Shared immutable object containing payload and timing metadata.
   */
  void dataReceived(const IO::CapturedDataPtr& data);

public:
  /**
   * @brief Constructs the driver.
   * @param parent Optional Qt parent object.
   */
  explicit HAL_Driver(QObject* parent = nullptr) : QObject(parent) {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~HAL_Driver() = default;

  /**
   * @brief Closes the driver and releases any associated resources.
   */
  virtual void close() = 0;

  /**
   * @brief Indicates whether the driver is currently open.
   * @return @c true if the underlying transport is open, otherwise @c false.
   */
  [[nodiscard]] virtual bool isOpen() const noexcept = 0;

  /**
   * @brief Indicates whether the driver can currently read data.
   * @return @c true if reading is supported and available, otherwise @c false.
   */
  [[nodiscard]] virtual bool isReadable() const noexcept = 0;

  /**
   * @brief Indicates whether the driver can currently write data.
   * @return @c true if writing is supported and available, otherwise @c false.
   */
  [[nodiscard]] virtual bool isWritable() const noexcept = 0;

  /**
   * @brief Indicates whether the current driver configuration is valid.
   * @return @c true if the driver has enough valid configuration to operate.
   */
  [[nodiscard]] virtual bool configurationOk() const noexcept = 0;

  /**
   * @brief Writes raw data to the driver.
   * @param data Payload to transmit.
   * @return Number of bytes written, or a negative value on error.
   */
  [[nodiscard]] virtual qint64 write(const QByteArray& data) = 0;

  /**
   * @brief Opens the driver using the requested I/O mode.
   * @param mode Qt open mode flags.
   * @return @c true on success, otherwise @c false.
   */
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode) = 0;

  /**
   * @brief Returns the list of configurable properties exposed by the driver.
   * @return List of driver property descriptors.
   */
  [[nodiscard]] virtual QList<IO::DriverProperty> driverProperties() const = 0;

  /**
   * @brief Returns a persistent identifier for the currently selected device.
   * @return JSON object describing the selected device.
   *
   * The default implementation returns an empty object, which indicates that
   * the driver does not expose a stable device identifier.
   */
  [[nodiscard]] virtual QJsonObject deviceIdentifier() const { return {}; }

  /**
   * @brief Selects a device using a previously stored identifier.
   * @param id JSON object describing the target device.
   * @return @c true if the device was successfully selected, otherwise @c false.
   *
   * The default implementation does not support identifier-based selection and
   * always returns @c false.
   */
  virtual bool selectByIdentifier(const QJsonObject& id)
  {
    Q_UNUSED(id);
    return false;
  }

public slots:
  /**
   * @brief Updates the value of a driver property.
   * @param key Internal property key.
   * @param value New value for the property.
   */
  virtual void setDriverProperty(const QString& key, const QVariant& value) = 0;

protected:
  /**
   * @brief Publishes received data using a copied payload.
   * @param data Raw payload to copy.
   * @param timestamp Acquisition timestamp.
   * @param frameStep Time step between logical frames.
   * @param logicalFramesHint Optional number of logical frames in the payload.
   *
   * This helper packages the arguments into a @ref CapturedData object and
   * emits @ref dataReceived.
   */
  void publishReceivedData(
    const QByteArray& data,
    CapturedData::SteadyTimePoint timestamp = CapturedData::SteadyClock::now(),
    std::chrono::nanoseconds frameStep      = std::chrono::nanoseconds(1),
    qsizetype logicalFramesHint             = 0)
  {
    Q_EMIT dataReceived(makeCapturedData(data, timestamp, frameStep, logicalFramesHint));
  }

  /**
   * @brief Publishes received data using a moved payload.
   * @param data Raw payload to move.
   * @param timestamp Acquisition timestamp.
   * @param frameStep Time step between logical frames.
   * @param logicalFramesHint Optional number of logical frames in the payload.
   *
   * This overload avoids copying the payload when ownership can be transferred.
   */
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
