/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>
#include <QStringList>

namespace SelfTest {

/**
 * @brief Tally produced by one suite run: the suite name, checks executed, checks failed.
 */
struct SuiteResult {
  QString name;
  int checks;
  int failures;
};

/**
 * @brief Runs the in-process self-test suites reached through --selftest. The suites execute
 *        inside CLI::process(), before ModuleManager::instantiateCoreModules() has run, so a
 *        suite must never touch an application singleton.
 */
class Runner {
public:
  [[nodiscard]] static QStringList suiteNames();
  [[nodiscard]] static int runAndReport(const QString& suiteFilter);
};

}  // namespace SelfTest
