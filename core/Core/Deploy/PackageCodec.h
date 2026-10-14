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

#ifdef BUILD_COMMERCIAL

#  include <QByteArray>
#  include <QJsonObject>
#  include <QList>
#  include <QString>
#  include <QStringList>
#  include <QVersionNumber>

namespace Deploy {

/**
 * @brief One file carried by a package, addressed by its path relative to the mount root.
 */
struct FileEntry {
  QString path;
  QByteArray data;
};

/**
 * @brief A credential the author chose to include, applied to a driver property at load.
 */
struct Credential {
  QString target;
  QString property;
  QString value;
};

/**
 * @brief Package identity and the runtime options the Deploy dialog collected.
 */
struct Manifest {
  QString title;
  QByteArray icon;
  QJsonObject options;
  QString projectFolder;
  QStringList externals;
};

/**
 * @brief The decoded contents of a .ssdeploy file.
 */
struct Package {
  QString minAppVersion;
  QString writerVersion;
  Manifest manifest;
  QByteArray project;
  QList<FileEntry> files;
  QList<Credential> credentials;
};

/**
 * @brief Why a decode was refused; Ok means the package is usable. A zero value is a refusal.
 */
enum class DecodeStatus {
  Corrupt,
  TooLarge,
  NotAPackage,
  NeedsNewerApp,
  Ok,
};

/**
 * @brief Outcome of decode(): status, the version a newer package asks for, and a log detail.
 */
struct DecodeResult {
  DecodeStatus status = DecodeStatus::Corrupt;
  QString requiredVersion;
  QString detail;
  Package package;
};

/**
 * @brief Outcome of encode(): the file bytes, or the reason the package cannot be written.
 */
struct EncodeResult {
  bool ok = false;
  QString error;
  QByteArray bytes;
};

[[nodiscard]] EncodeResult encode(const Package& package);
[[nodiscard]] DecodeResult decode(const QByteArray& bytes, const QVersionNumber& runningVersion);
[[nodiscard]] bool isSafeEntryPath(const QString& path);

}  // namespace Deploy

#endif  // BUILD_COMMERCIAL
