/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/Modbus/ModbusSerialParams.h"

/**
 * @brief Maps the UI parity index to the corresponding QSerialPort::Parity enum.
 */
QSerialPort::Parity IO::Drivers::ModbusSerialParams::parityFromIndex(quint8 index) noexcept
{
  if (index == 1)
    return QSerialPort::EvenParity;

  if (index == 2)
    return QSerialPort::OddParity;

  if (index == 3)
    return QSerialPort::SpaceParity;

  if (index == 4)
    return QSerialPort::MarkParity;

  return QSerialPort::NoParity;
}

/**
 * @brief Maps the UI data-bits index to the corresponding QSerialPort::DataBits enum.
 */
QSerialPort::DataBits IO::Drivers::ModbusSerialParams::dataBitsFromIndex(quint8 index) noexcept
{
  if (index == 0)
    return QSerialPort::Data5;

  if (index == 1)
    return QSerialPort::Data6;

  if (index == 2)
    return QSerialPort::Data7;

  return QSerialPort::Data8;
}

/**
 * @brief Maps the UI stop-bits index to the corresponding QSerialPort::StopBits enum.
 */
QSerialPort::StopBits IO::Drivers::ModbusSerialParams::stopBitsFromIndex(quint8 index) noexcept
{
  if (index == 1)
    return QSerialPort::OneAndHalfStop;

  if (index == 2)
    return QSerialPort::TwoStop;

  return QSerialPort::OneStop;
}
