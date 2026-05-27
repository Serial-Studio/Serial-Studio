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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <optional>
#  include <QString>

namespace Platform {

/**
 * @brief Synchronous wrapper over the OS secret store (DPAPI / Keychain / libsecret).
 */
class SecretStore {
public:
  [[nodiscard]] static bool available();
  [[nodiscard]] static bool remove(const QString& service, const QString& account);
  [[nodiscard]] static bool store(const QString& service,
                                  const QString& account,
                                  const QString& secret);
  [[nodiscard]] static std::optional<QString> retrieve(const QString& service,
                                                       const QString& account);
};

}  // namespace Platform

#endif  // BUILD_COMMERCIAL
