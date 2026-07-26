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

#include "SelfTest/SelfTest.h"

#include <cstdlib>
#include <iterator>
#include <QCoreApplication>
#include <QDebug>
#include <QtGlobal>
#include <QVersionNumber>

#include "SSAssert.h"

namespace SelfTest {

namespace detail {

/**
 * @brief One registered suite: the name --selftest-suite matches, and the function that runs it.
 */
struct SuiteEntry {
  const char* name;
  void (*run)(SuiteResult&);
};

}  // namespace detail

static void runSmokeSuite(SuiteResult& result);

//---------------------------------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------------------------------

/**
 * @brief The v1 registry. Suites run in declaration order; each one must be self-contained.
 */
static constexpr detail::SuiteEntry kSuites[] = {
  {"smoke", &runSmokeSuite},
};

//---------------------------------------------------------------------------------------------------
// Suite implementations
//---------------------------------------------------------------------------------------------------

/**
 * @brief Records one boolean check into @p result, naming the expectation when it fails.
 */
static void check(SuiteResult& result, bool ok, const char* what)
{
  SS_ASSERT(what != nullptr, return);
  SS_ASSERT_LOG(result.checks >= 0);

  ++result.checks;
  if (ok)
    return;

  ++result.failures;
  qCritical().noquote() << "[selftest]" << result.name << "FAILED:" << QString::fromLatin1(what);
}

/**
 * @brief Smoke suite: invariants that hold before any application module exists, so the seam can
 *        be proven without reaching a singleton the composition root has not built yet.
 */
static void runSmokeSuite(SuiteResult& result)
{
  const auto compiled = QVersionNumber::fromString(QStringLiteral(QT_VERSION_STR));
  const auto running  = QVersionNumber::fromString(QString::fromLatin1(qVersion()));
  SS_ASSERT_LOG(!compiled.isNull());

  check(result, !running.isNull(), "runtime Qt version is readable");
  check(result, compiled.majorVersion() == running.majorVersion(), "Qt major version matches");
  check(result, compiled.minorVersion() == running.minorVersion(), "Qt minor version matches");
  check(result, !QCoreApplication::applicationName().isEmpty(), "application metadata is set");
}

//---------------------------------------------------------------------------------------------------
// Runner
//---------------------------------------------------------------------------------------------------

/**
 * @brief Returns every registered suite name, in registration order.
 */
QStringList Runner::suiteNames()
{
  SS_ASSERT_LOG(std::size(kSuites) > 0);

  QStringList names;
  names.reserve(static_cast<qsizetype>(std::size(kSuites)));
  for (const auto& entry : kSuites) {
    SS_ASSERT_LOG(entry.name != nullptr);
    if (entry.name == nullptr)
      continue;

    names.append(QString::fromLatin1(entry.name));
  }

  return names;
}

/**
 * @brief Runs every suite, or only the one named by @p suiteFilter, and returns the aggregate exit
 *        status: EXIT_SUCCESS when no check failed, EXIT_FAILURE otherwise.
 */
int Runner::runAndReport(const QString& suiteFilter)
{
  SS_ASSERT_LOG(std::size(kSuites) > 0);

  int suitesRun = 0;
  int checks    = 0;
  int failures  = 0;

  for (const auto& entry : kSuites) {
    SS_ASSERT_LOG(entry.run != nullptr);
    if (entry.run == nullptr || entry.name == nullptr)
      continue;

    SuiteResult result{QString::fromLatin1(entry.name), 0, 0};
    if (!suiteFilter.isEmpty() && suiteFilter != result.name)
      continue;

    entry.run(result);
    ++suitesRun;
    checks   += result.checks;
    failures += result.failures;

    qInfo().noquote() << QStringLiteral("[selftest] %1: %2 (%3 checks, %4 failures)")
                           .arg(result.name,
                                result.failures == 0 ? QStringLiteral("PASS")
                                                     : QStringLiteral("FAIL"))
                           .arg(result.checks)
                           .arg(result.failures);
  }

  if (suitesRun == 0) {
    qCritical().noquote() << "[selftest] unknown suite:" << suiteFilter
                          << "| available:" << suiteNames().join(QStringLiteral(", "));
    return EXIT_FAILURE;
  }

  qInfo().noquote() << QStringLiteral("[selftest] %1 suite(s), %2 checks, %3 failures")
                         .arg(suitesRun)
                         .arg(checks)
                         .arg(failures);

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

}  // namespace SelfTest
