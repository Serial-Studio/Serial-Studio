/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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
#include <QList>
#include <QString>

#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

/**
 * @brief The installation's OPC UA identity and its trust store (spec 0067 stage 2). Owns the
 *        client certificate and key used for Sign and SignAndEncrypt channels, the list of
 *        server certificates the user has accepted, and the inspection that turns a DER blob
 *        into something a trust prompt can show. Qt Core only in the header, so the driver and
 *        the API handler can use it without seeing the protocol stack.
 *
 *        Everything lives per-INSTALLATION, in the writable config location. A key in a project
 *        file would travel with the project; a trust decision in a project file would have to be
 *        re-made on every machine that opens it.
 */
namespace OpcUaSecurity {

[[nodiscard]] QString storageDirectory();
[[nodiscard]] QString applicationUri();

[[nodiscard]] bool ensureClientIdentity(QByteArray& certificate, QByteArray& key);
[[nodiscard]] QByteArray clientCertificate();
[[nodiscard]] QByteArray clientPrivateKey();
[[nodiscard]] bool regenerateClientIdentity();
[[nodiscard]] bool exportClientCertificate(const QString& path);

[[nodiscard]] QList<QByteArray> trustedCertificates();
[[nodiscard]] bool isTrusted(const QByteArray& certificate);
[[nodiscard]] bool trustCertificate(const QByteArray& certificate);
[[nodiscard]] bool revokeTrust(const QString& fingerprint);

[[nodiscard]] bool plaintextPasswordAllowed();
void setPlaintextPasswordAllowed(bool allowed);

[[nodiscard]] OpcUaTypes::CertInfo inspect(const QByteArray& certificate, const QString& host);
[[nodiscard]] QByteArray readCertificateFile(const QString& path);

}  // namespace OpcUaSecurity
}  // namespace Drivers
}  // namespace IO
