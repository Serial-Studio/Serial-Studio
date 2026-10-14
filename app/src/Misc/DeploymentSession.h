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

#  include <QString>

#  include "Core/Deploy/PackageCodec.h"
#  include "Core/Deploy/ResourceTree.h"
#  include "Misc/CLI.h"

namespace Misc {

/**
 * @brief Opens a deployment package for the composition root (spec 0096): open() runs before the
 *        root and seals the process, loadProject() runs at the CLI project step. The object lives
 *        in runApplication's frame, so its mounted files outlive every reader.
 */
class DeploymentSession {
public:
  DeploymentSession();
  ~DeploymentSession();

  DeploymentSession(DeploymentSession&&)                 = delete;
  DeploymentSession(const DeploymentSession&)            = delete;
  DeploymentSession& operator=(DeploymentSession&&)      = delete;
  DeploymentSession& operator=(const DeploymentSession&) = delete;

  [[nodiscard]] bool isOpen() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] RuntimeOptions runtimeOptions() const;

  [[nodiscard]] bool loadProject();
  [[nodiscard]] bool open(const QString& path);

private:
  void applyIdentity() const;
  void applyCredentials();

  static void configureQmlCache();
  static void showRefusal(const QString& message);
  [[nodiscard]] static QString refusalMessage(const Deploy::DecodeResult& result);

private:
  bool m_open;
  Deploy::Package m_package;
  Deploy::ResourceTree m_tree;
};

}  // namespace Misc

#endif  // BUILD_COMMERCIAL
