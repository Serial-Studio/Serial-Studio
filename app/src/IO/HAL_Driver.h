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

#include <memory>
#include <QIODevice>
#include <QObject>

namespace IO {
/**
 * @brief Type alias for shared byte array pointers used in data hotpath.
 *
 * **Design Rationale:**
 * Using shared_ptr<const T> provides:
 * - Zero-copy distribution to multiple consumers
 * - Thread-safe reference counting (atomic operations)
 * - Compile-time const-correctness (immutable after creation)
 *
 * **Performance Characteristics:**
 * - Time: O(1) reference count increment/decrement
 * - Space: sizeof(QByteArray) + 16 bytes (control block overhead)
 * - Thread-safe: Yes (atomic reference counting)
 *
 * **Memory Model:**
 * - Single allocation contains both data and control block
 * - Reference count is atomically incremented on copy
 * - Data is deallocated when last reference is destroyed
 */
typedef std::shared_ptr<const QByteArray> ByteArrayPtr;

/**
 * @brief Creates a shared byte array from a const reference.
 *
 * This overload performs a deep copy of the source data into a new
 * shared_ptr-managed QByteArray. Use this when the source data must
 * remain independent.
 *
 * **Performance:** O(n) where n = data.size() (due to copy)
 *
 * **Thread Safety:** Safe to call from any thread
 *
 * @param data Source data (will be copied into shared_ptr)
 * @return Shared pointer to immutable byte array
 *
 * @note Consider using the rvalue overload if source is temporary
 */
[[nodiscard]] inline ByteArrayPtr makeByteArray(const QByteArray& data) noexcept
{
  return std::make_shared<const QByteArray>(data);
}

/**
 * @brief Creates a shared byte array from an rvalue reference.
 *
 * Uses move semantics to transfer ownership without copying. Prefer this
 * overload when passing temporary objects or when the source data is no
 * longer needed.
 *
 * **Performance:** O(1) - no data copy, only pointer/size transfer
 *
 * **Thread Safety:** Safe to call from any thread
 *
 * @param data Source data (will be moved into shared_ptr, leaves source empty)
 * @return Shared pointer to immutable byte array
 *
 * @note After this call, `data` is in a valid but unspecified state
 *       (typically empty)
 */
[[nodiscard]] inline ByteArrayPtr makeByteArray(QByteArray&& data) noexcept
{
  return std::make_shared<const QByteArray>(std::move(data));
}

/**
 * @class HAL_Driver
 * @brief Abstract base class for hardware abstraction layer drivers.
 *
 * **Overview:**
 * This interface defines the core API for all I/O drivers in the system
 * (UART, Network, BluetoothLE, Audio, Modbus, CANBus). It provides a uniform
 * abstraction for opening, closing, and transferring data across different
 * hardware protocols.
 *
 * **Buffering Strategy:**
 * The class includes a high-efficiency buffered data processing mechanism
 * through `processData()`, which reduces signal overhead in high-frequency
 * environments by aggregating data until a configurable buffer threshold is
 * reached. Once the threshold is met, data is emitted via `dataReceived()`.
 *
 * **Performance Characteristics:**
 * - `isOpen()`, `isReadable()`, `isWritable()`: O(1) state queries
 * - `write()`: O(n) where n = data.size(), device-dependent
 * - `processData()`: O(1) buffer append, O(n) emit when threshold reached
 *
 * **Thread Safety:**
 * - ✅ Safe: Buffer operations (`processData()`, `flushBuffer()`)
 * - ⚠️  Caution: Virtual methods must be implemented thread-safely by
 *   subclasses
 * - ⚠️  Caution: Signals are emitted across thread boundaries - use queued
 *   connections
 *
 * **Lifecycle:**
 * 1. Construction: Subclass initializes device-specific state
 * 2. `open()`: Establishes connection to hardware
 * 3. `write()` / `processData()`: Data transfer operations
 * 4. `close()`: Releases hardware resources
 * 5. Destruction: Virtual destructor ensures proper cleanup
 *
 * **Implementation Requirements:**
 * Subclasses must override all pure virtual methods:
 * - `open()`: Device-specific connection logic
 * - `close()`: Device-specific disconnection logic
 * - `isOpen()`, `isReadable()`, `isWritable()`: State queries
 * - `configurationOk()`: Validation of device settings
 * - `write()`: Device-specific data transmission
 *
 * @note This class follows the Non-Virtual Interface (NVI) pattern for buffer
 *       operations while using pure virtuals for device-specific behavior.
 *
 * @see setBufferSize(), processData(), flushBuffer()
 * @see IO::Manager for the central I/O orchestration layer
 */
class HAL_Driver : public QObject {
  Q_OBJECT

signals:
  /**
   * @brief Emitted when the driver configuration changes.
   */
  void configurationChanged();

  /**
   * @brief Emitted after data is sent.
   * @param data The raw data that was sent.
   */
  void dataSent(const QByteArray& data);

  /**
   * @brief Emitted when buffered data is ready.
   * @param data The buffered data (shared pointer for zero-copy distribution).
   */
  void dataReceived(const IO::ByteArrayPtr& data);

public:
  /**
   * @brief Constructor.
   * @param parent Optional QObject parent.
   */
  explicit HAL_Driver(QObject* parent = nullptr) : QObject(parent) {}

  /**
   * @brief Virtual destructor.
   */
  virtual ~HAL_Driver() = default;

  /**
   * @brief Close the driver.
   */
  virtual void close() = 0;

  /**
   * @brief Check if the device is open.
   *
   * **Performance:** O(1) - simple state query
   *
   * **Thread Safety:** Implementation-dependent (subclass responsibility)
   *
   * @return True if the device connection is established and active
   *
   * @note This method should not perform I/O operations, only state checks
   */
  [[nodiscard]] virtual bool isOpen() const noexcept = 0;

  /**
   * @brief Check if the device is readable.
   *
   * **Performance:** O(1) - simple state query
   *
   * **Thread Safety:** Implementation-dependent (subclass responsibility)
   *
   * @return True if data can be received from the device
   *
   * @note A device can be open but not readable (e.g., write-only mode)
   */
  [[nodiscard]] virtual bool isReadable() const noexcept = 0;

  /**
   * @brief Check if the device is writable.
   *
   * **Performance:** O(1) - simple state query
   *
   * **Thread Safety:** Implementation-dependent (subclass responsibility)
   *
   * @return True if data can be sent to the device
   *
   * @note A device can be open but not writable (e.g., read-only mode)
   */
  [[nodiscard]] virtual bool isWritable() const noexcept = 0;

  /**
   * @brief Check if the current configuration is valid.
   *
   * Validates that all device parameters (baud rate, parity, address, etc.)
   * are properly set and the device can be opened successfully.
   *
   * **Performance:** O(1) - validation logic only, no I/O
   *
   * **Thread Safety:** Implementation-dependent (subclass responsibility)
   *
   * @return True if the configuration is valid and device can be opened
   *
   * @note This method should be called before `open()` to prevent errors
   */
  [[nodiscard]] virtual bool configurationOk() const noexcept = 0;

  /**
   * @brief Write data to the device.
   * @param data The data to write.
   * @return Number of bytes successfully written.
   */
  [[nodiscard]] virtual quint64 write(const QByteArray& data) = 0;

  /**
   * @brief Open the device with the given mode.
   * @param mode The open mode.
   * @return True if successfully opened.
   */
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode) = 0;
};
}  // namespace IO
