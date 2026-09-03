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

#include <QSettings>
#include <QStandardPaths>
#include <QTest>

#include "Licensing/MachineID.h"
#include "Licensing/SimpleCrypt.h"

// The suite runs in Qt's test mode, so the seeded fingerprint lands in a throwaway config
// location and never touches the developer's real licensing store.

/**
 * @brief The OS label MachineID stores its fingerprint under.
 */
static QString platformLabel()
{
#if defined(Q_OS_LINUX)
  return QStringLiteral("Linux");
#elif defined(Q_OS_MAC)
  return QStringLiteral("macOS");
#elif defined(Q_OS_WIN)
  return QStringLiteral("Windows");
#elif defined(Q_OS_BSD)
  return QStringLiteral("BSD");
#else
  return QStringLiteral("Unknown");
#endif
}

/**
 * @brief Seeds the last-good fingerprint store the way a previous run would have left it.
 */
static void seedStoredFingerprint(const QString& rawId)
{
  static constexpr quint64 kStoreKey = 0x5331'4D49'4452'4157ULL;

  Licensing::SimpleCrypt crypt(kStoreKey);
  crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);

  QSettings settings;
  settings.beginGroup("licensing");
  settings.setValue("lastGoodRawId", crypt.encryptToString(rawId));
  settings.setValue("lastGoodOs", platformLabel());
  settings.endGroup();
  settings.sync();
}

/**
 * @brief The startup contract of Licensing::MachineID: a machine that already has a fingerprint
 *        answers from it instead of spawning ioreg / reg / powershell on the GUI thread.
 */
class TstMachineId : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void storedFingerprintSkipsTheToolSpawn();
  void identifiersAreDerivedFromIt();
};

/**
 * @brief Redirects QSettings into the test location and seeds the store BEFORE the singleton is
 *        first reached: its constructor is what reads the fingerprint.
 */
void TstMachineId::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("serial-studio-tests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_machine_id"));

  seedStoredFingerprint(QStringLiteral("00000000-1111-2222-3333-444444444444"));
}

/**
 * @brief With a fingerprint on disk, the platform tools are not run at all.
 */
void TstMachineId::storedFingerprintSkipsTheToolSpawn()
{
  QVERIFY(Licensing::MachineID::instance().usedStoredFingerprint());
}

/**
 * @brief The identifiers and the crypt key are still derived, and stay stable within the run.
 */
void TstMachineId::identifiersAreDerivedFromIt()
{
  auto& machine = Licensing::MachineID::instance();

  QVERIFY(!machine.machineId().isEmpty());
  QVERIFY(!machine.appVerMachineId().isEmpty());
  QVERIFY(machine.machineSpecificKey() != 0);
  QVERIFY(machine.machineId() != machine.appVerMachineId());
  QCOMPARE(machine.machineId(), Licensing::MachineID::instance().machineId());
}

QTEST_MAIN(TstMachineId)

#include "tst_machine_id.moc"
