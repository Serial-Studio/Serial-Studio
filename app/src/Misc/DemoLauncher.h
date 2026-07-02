/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
#include <QString>

namespace Misc {
/**
 * @brief One-click launcher for the bundled zero-hardware demo project.
 */
class DemoLauncher : public QObject {
  Q_OBJECT

private:
  explicit DemoLauncher();
  DemoLauncher(DemoLauncher&&)                 = delete;
  DemoLauncher(const DemoLauncher&)            = delete;
  DemoLauncher& operator=(DemoLauncher&&)      = delete;
  DemoLauncher& operator=(const DemoLauncher&) = delete;

public:
  [[nodiscard]] static DemoLauncher& instance();

public slots:
  bool startDemo();

private:
  [[nodiscard]] QString demoDirectory() const;
  [[nodiscard]] QString demoProjectPath() const;
  [[nodiscard]] bool stageDemoProject(QString& projectPath) const;
  void applyProGating() const;
  void showStartFailure() const;
};
}  // namespace Misc
