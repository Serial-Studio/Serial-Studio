/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#pragma once

#include <QByteArray>
#include <QChar>
#include <QString>

namespace IO {
namespace Drivers {

/**
 * @brief Pure text helpers for the USB Advanced Control composer: the setup-packet fields and
 *        the data payload reach the driver as user-typed hex, and a completed transfer reaches
 *        it as a libusb status code. Nothing here touches a device, so the whole namespace is
 *        free functions over Qt Core alone and is pinned by tst_usb_hex_parsing.
 */
namespace UsbHex {

/**
 * @brief Mirror of @c libusb_transfer_status. Duplicated so this translation unit (and its unit
 *        suite) build without the libusb headers; @c USB.cpp static_asserts every value against
 *        the real enum, so a libusb renumbering fails the build instead of mislabelling a
 *        failure in the composer.
 */
enum TransferStatus : int {
  kTransferCompleted = 0,
  kTransferError     = 1,
  kTransferTimedOut  = 2,
  kTransferCancelled = 3,
  kTransferStall     = 4,
  kTransferNoDevice  = 5,
  kTransferOverflow  = 6,
};

[[nodiscard]] bool isHexChar(const QChar c);

[[nodiscard]] QString controlStatusText(const int status);

[[nodiscard]] QByteArray parseHexBytes(const QString& text, bool& ok);

[[nodiscard]] unsigned int parseHexUInt(const QString& text, const unsigned int max, bool& ok);

}  // namespace UsbHex
}  // namespace Drivers
}  // namespace IO
