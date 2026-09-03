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

#include <cstddef>
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
 * @brief The pre-root registry. Suites run in declaration order; each one must be self-contained
 *        and must never touch an application singleton.
 */
static constexpr detail::SuiteEntry kSuites[] = {
  {"smoke", &runSmokeSuite},
};

/**
 * @brief The post-root registry: suites that need the composition root and the QML engine, run
 *        from the hook in CLI::process() after ModuleManager has built the modules. Named
 *        explicitly with --selftest-suite; a bare --selftest never reaches them.
 */
static constexpr detail::SuiteEntry kPostRootSuites[] = {
  {"qml", &runQmlInstantiationSuite},
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
 * @brief Returns the names in one registry, in registration order.
 */
static QStringList namesIn(const detail::SuiteEntry* suites, std::size_t count)
{
  SS_ASSERT(suites != nullptr, return {});
  SS_ASSERT_LOG(count > 0);

  QStringList names;
  names.reserve(static_cast<qsizetype>(count));
  for (std::size_t i = 0; i < count; ++i) {
    SS_ASSERT_LOG(suites[i].name != nullptr);
    if (suites[i].name == nullptr)
      continue;

    names.append(QString::fromLatin1(suites[i].name));
  }

  return names;
}

/**
 * @brief Runs one registry, honoring @p suiteFilter, and returns EXIT_SUCCESS / EXIT_FAILURE.
 */
static int runRegistry(const detail::SuiteEntry* suites,
                       std::size_t count,
                       const QString& suiteFilter,
                       const QStringList& available)
{
  SS_ASSERT(suites != nullptr, return EXIT_FAILURE);
  SS_ASSERT_LOG(count > 0);

  int suitesRun = 0;
  int checks    = 0;
  int failures  = 0;

  for (std::size_t i = 0; i < count; ++i) {
    const auto& entry = suites[i];
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
                          << "| available:" << available.join(QStringLiteral(", "));
    return EXIT_FAILURE;
  }

  qInfo().noquote() << QStringLiteral("[selftest] %1 suite(s), %2 checks, %3 failures")
                         .arg(suitesRun)
                         .arg(checks)
                         .arg(failures);

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * @brief Returns every registered pre-root suite name, in registration order.
 */
QStringList Runner::suiteNames()
{
  return namesIn(kSuites, std::size(kSuites));
}

/**
 * @brief Returns every registered post-root suite name, in registration order.
 */
QStringList Runner::postRootSuiteNames()
{
  return namesIn(kPostRootSuites, std::size(kPostRootSuites));
}

/**
 * @brief Runs every pre-root suite, or only the one named by @p suiteFilter, and returns the
 *        aggregate exit status: EXIT_SUCCESS when no check failed, EXIT_FAILURE otherwise.
 */
int Runner::runAndReport(const QString& suiteFilter)
{
  return runRegistry(kSuites, std::size(kSuites), suiteFilter, suiteNames());
}

/**
 * @brief Runs the post-root suite named by @p suiteFilter, after the composition root exists.
 */
int Runner::runPostRootAndReport(const QString& suiteFilter)
{
  return runRegistry(
    kPostRootSuites, std::size(kPostRootSuites), suiteFilter, postRootSuiteNames());
}

}  // namespace SelfTest
