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
#  include <QList>
#  include <QString>

#  include "Core/Deploy/PackageCodec.h"

namespace Deploy {

/**
 * @brief Serves a package's carried files as an in-memory Qt resource tree (spec 0096), built
 *        from codec-validated entries and never from file bytes; the owner outlives every reader.
 */
class ResourceTree {
public:
  ResourceTree();
  ~ResourceTree();

  ResourceTree(ResourceTree&&)                 = delete;
  ResourceTree(const ResourceTree&)            = delete;
  ResourceTree& operator=(ResourceTree&&)      = delete;
  ResourceTree& operator=(const ResourceTree&) = delete;

  [[nodiscard]] static QByteArray build(const QList<FileEntry>& files);

  [[nodiscard]] bool isMounted() const noexcept;
  [[nodiscard]] const QString& root() const noexcept;
  [[nodiscard]] bool mount(const QList<FileEntry>& files, const QString& id);

private:
  QByteArray m_data;
  QString m_mapRoot;
  QString m_root;
};

}  // namespace Deploy

#endif  // BUILD_COMMERCIAL
