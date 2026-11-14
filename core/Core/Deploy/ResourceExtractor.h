/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#  include <memory>
#  include <QList>
#  include <QString>
#  include <QStringList>

#  include "Core/Deploy/PackageCodec.h"

class QLockFile;

namespace Deploy {

/**
 * @brief Writes a package's resources to a private folder for one session (spec 0123). Only
 *        entries under resources/ are written, each at its path relative to the project folder,
 *        and no entry can land outside the folder. The folder holds a lock file for as long as
 *        its session lives, which is how a later session tells a leftover from a live one.
 */
class ResourceExtractor {
public:
  ResourceExtractor();
  ~ResourceExtractor();

  ResourceExtractor(ResourceExtractor&&)                 = delete;
  ResourceExtractor(const ResourceExtractor&)            = delete;
  ResourceExtractor& operator=(ResourceExtractor&&)      = delete;
  ResourceExtractor& operator=(const ResourceExtractor&) = delete;

  [[nodiscard]] static bool isResourceEntry(const QString& entryPath);
  [[nodiscard]] static QStringList filesUnder(const QString& path);
  [[nodiscard]] static QString entryPathFor(const QString& projectFolder, const QString& file);
  static void sweepStale(const QString& baseDir, int minimumAgeSeconds);

  [[nodiscard]] const QString& folder() const noexcept;
  [[nodiscard]] bool extract(const QList<FileEntry>& files, const QString& baseDir, QString& error);

  void remove();

private:
  [[nodiscard]] bool writeEntry(const FileEntry& entry, QString& error) const;

private:
  QString m_folder;
  std::unique_ptr<QLockFile> m_lock;
};

}  // namespace Deploy

#endif  // BUILD_COMMERCIAL
