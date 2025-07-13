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

#include <QObject>
#include <QIODevice>

namespace IO
{
/**
 * @class HAL_Driver
 * @brief Base class for hardware abstraction layer (HAL) drivers.
 *
 * Provides a common interface for device I/O operations and data access.
 * Subclasses must implement all pure virtual methods to handle
 * protocol-specific functionality.
 *
 * Signals are available for configuration changes, data transmission, and data
 * reception.
 */
class HAL_Driver : public QObject
{
  Q_OBJECT

signals:
  void configurationChanged();
  void dataSent(const QByteArray &data);
  void dataReceived(const QByteArray &data);

public:
  virtual void close() = 0;
  [[nodiscard]] virtual bool isOpen() const = 0;
  [[nodiscard]] virtual bool isReadable() const = 0;
  [[nodiscard]] virtual bool isWritable() const = 0;
  [[nodiscard]] virtual bool configurationOk() const = 0;
  [[nodiscard]] virtual quint64 write(const QByteArray &data) = 0;
  [[nodiscard]] virtual bool open(const QIODevice::OpenMode mode) = 0;
};
} // namespace IO
