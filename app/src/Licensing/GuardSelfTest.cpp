/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "Licensing/GuardSelfTest.h"

#include <QDebug>
#include <QString>

#include "Core/Licensing/CommercialToken.h"

//--------------------------------------------------------------------------------------------------
// Assertion helper
//--------------------------------------------------------------------------------------------------

/**
 * @brief The gate the old SerialStudio::activated() evaluated: a valid token plus the guard chain.
 */
[[nodiscard]] static bool gatesOpen()
{
  return Licensing::CommercialToken::current().isValid() && SS_LICENSE_GUARD();
}

/**
 * @brief Reports a single check and returns 1 on failure, 0 on success.
 */
[[nodiscard]] static int expect(const QString& name, bool condition)
{
  if (condition) {
    qInfo().noquote() << "  PASS" << name;
    return 0;
  }

  qCritical().noquote() << "  FAIL" << name;
  return 1;
}

//--------------------------------------------------------------------------------------------------
// Guard table checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs every generated guard function as linked into this binary.
 */
[[nodiscard]] static int runGuardTableChecks()
{
  int fails         = 0;
  const auto& table = Licensing::Guards::guardTable();
  for (std::size_t i = 0; i < table.size(); ++i)
    fails += expect(QStringLiteral("guard %1 passes").arg(i), table[i]());

  return fails;
}

/**
 * @brief Exercises the call-site dispatch across two full wraps of the guard table.
 */
[[nodiscard]] static int runDispatchChecks()
{
  bool ok           = true;
  const auto& table = Licensing::Guards::guardTable();
  const auto sites  = static_cast<unsigned int>(table.size() * 2);
  for (unsigned int site = 0; site < sites; ++site)
    ok = Licensing::Guards::runGuard(site) && ok;

  return expect(QStringLiteral("runGuard() dispatch passes for %1 sites").arg(sites), ok);
}

//--------------------------------------------------------------------------------------------------
// Feature-gate chain checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates the token + guard + tier chain through the real feature gates, using a
 *        synthetic in-memory token so the result is independent of activation state.
 */
[[nodiscard]] static int runGateChainChecks()
{
  const auto previous = Licensing::CommercialToken::current();

  Licensing::CommercialToken token;
  token.setFeatureTier(Licensing::FeatureTier::Trial);
  token.setVariantName(QStringLiteral("Guard Self-Test"));
  token.setInstanceName(QStringLiteral("validate-guards"));
  token.setGraceDaysRemaining(0);
  token.seal();
  Licensing::CommercialToken::setCurrent(token);

  int fails  = 0;
  fails     += expect(QStringLiteral("synthetic trial token validates"),
                  Licensing::CommercialToken::current().isValid());
  fails     += expect(QStringLiteral("activated() opens with trial token"), gatesOpen());

  auto tampered = token;
  tampered.setFeatureTier(Licensing::FeatureTier::Enterprise);
  Licensing::CommercialToken::setCurrent(tampered);
  fails += expect(QStringLiteral("tier mutated after seal() is rejected"), !gatesOpen());

  Licensing::CommercialToken::clearCurrent();
  fails += expect(QStringLiteral("cleared token closes all gates"), !gatesOpen());

  Licensing::CommercialToken::setCurrent(previous);
  return fails;
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the headless guard validation suite; returns 0 when every check passes.
 */
int Licensing::runGuardSelfTest()
{
  qInfo().noquote() << "[validate-guards] verifying license guards in this binary";
  const int fails = runGuardTableChecks() + runDispatchChecks() + runGateChainChecks();

  if (fails == 0)
    qInfo().noquote() << "[validate-guards] all guard checks passed";
  else
    qCritical().noquote() << "[validate-guards]" << fails << "check(s) failed";

  return fails == 0 ? 0 : 1;
}
