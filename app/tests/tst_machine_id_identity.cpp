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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QCoreApplication>
#include <QString>
#include <QTest>

#include "Licensing/MachineID.h"

// The fingerprint is read once per process, so every case here reads the same instance. What is
// pinned is the shape of the derived values and their stability, not the platform read itself:
// the helper-spawning path is the one WP-C makes lazy, and it has no seam yet.

/**
 * @brief Pins the fingerprint contract every stored secret depends on: one stable id per process,
 *        an app-version-scoped variant that differs from it, and a non-zero cipher key derived
 *        from the same digest. A fingerprint that changed between two runs would make every
 *        stored license blob and provider key undecryptable (spec 0075, M11).
 */
class TstMachineIdIdentity : public QObject {
  Q_OBJECT

private slots:
  void machineIdIsStableWithinTheProcess();
  void machineIdIsABase64Digest();
  void appVersionIdDiffersFromTheMachineId();
  void cipherKeyIsNonZeroAndStable();
};

//--------------------------------------------------------------------------------------------------
// Identity
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two reads return the same id: the value is derived once and cached.
 */
void TstMachineIdIdentity::machineIdIsStableWithinTheProcess()
{
  const auto first  = Licensing::MachineID::instance().machineId();
  const auto second = Licensing::MachineID::instance().machineId();

  QVERIFY(!first.isEmpty());
  QCOMPARE(second, first);
}

/**
 * @brief The id is the base64 of a 128-bit digest, never a raw serial number: the fingerprint
 *        leaves the machine in activation requests.
 */
void TstMachineIdIdentity::machineIdIsABase64Digest()
{
  const auto id = Licensing::MachineID::instance().machineId();

  QCOMPARE(id.size(), 24);
  QVERIFY(id.endsWith(QStringLiteral("==")));
  QVERIFY(!id.contains(QLatin1Char(' ')));
}

/**
 * @brief The app-version-scoped id is a different value, so the trial server cannot correlate it
 *        with the license fingerprint.
 */
void TstMachineIdIdentity::appVersionIdDiffersFromTheMachineId()
{
  auto& machineId = Licensing::MachineID::instance();

  QVERIFY(!machineId.appVerMachineId().isEmpty());
  QVERIFY(machineId.appVerMachineId() != machineId.machineId());
}

//--------------------------------------------------------------------------------------------------
// Derived cipher key
//--------------------------------------------------------------------------------------------------

/**
 * @brief The SimpleCrypt key is non-zero and stable; a zero key would refuse every encrypt and
 *        silently drop the stored license and provider keys.
 */
void TstMachineIdIdentity::cipherKeyIsNonZeroAndStable()
{
  const auto key = Licensing::MachineID::instance().machineSpecificKey();

  QVERIFY(key != 0);
  QCOMPARE(Licensing::MachineID::instance().machineSpecificKey(), key);
}

QTEST_GUILESS_MAIN(TstMachineIdIdentity)

#include "tst_machine_id_identity.moc"
