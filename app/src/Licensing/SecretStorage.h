/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QString>

namespace Licensing {

/**
 * @brief Vault-preferred persistence for individual licensing secrets.
 */
class SecretStorage {
public:
  static void remove(const QString& group, const QString& key);
  [[nodiscard]] static QString load(const QString& group, const QString& key);
  static void save(const QString& group, const QString& key, const QString& value);
};

}  // namespace Licensing

#endif  // BUILD_COMMERCIAL
