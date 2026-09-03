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
 * @brief Runs the in-process self-test suites reached through --selftest. Two registries: the
 *        pre-root suites execute inside CLI::process() before instantiateCoreModules() and must
 *        never touch an application singleton, while the post-root suites run after the
 *        composition root and are opt-in by name, so a bare --selftest never reaches them.
 */
class Runner {
public:
  [[nodiscard]] static QStringList suiteNames();
  [[nodiscard]] static QStringList postRootSuiteNames();
  [[nodiscard]] static int runAndReport(const QString& suiteFilter);
  [[nodiscard]] static int runPostRootAndReport(const QString& suiteFilter);
};

/**
 * @brief Instantiates every compiled QML file against stubbed `Cpp_*` globals (post-root).
 */
void runQmlInstantiationSuite(SuiteResult& result);

}  // namespace SelfTest
