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
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTest>

#include "Core/IO/HAL_Driver.h"

/**
 * @brief A Setup-pane style driver whose one property is a saved default, written through the
 *        base class's gated helper exactly as the real drivers write theirs.
 */
class PersistingDriver final : public IO::HAL_Driver {
  Q_OBJECT

public:
  explicit PersistingDriver(QSettings& settings) : m_settings(settings), m_persistenceChanges(0) {}

  void close() override {}

  [[nodiscard]] bool isOpen() const noexcept override { return false; }

  [[nodiscard]] bool isReadable() const noexcept override { return false; }

  [[nodiscard]] bool isWritable() const noexcept override { return false; }

  [[nodiscard]] bool configurationOk() const noexcept override { return true; }

  [[nodiscard]] qint64 write(const QByteArray& data) override { return data.size(); }

  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override
  {
    Q_UNUSED(mode);
    return false;
  }

  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override { return {}; }

  [[nodiscard]] int persistenceChanges() const noexcept { return m_persistenceChanges; }

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override
  {
    persistSetting(m_settings, key, value);
  }

protected:
  void onPersistenceChanged() noexcept override { ++m_persistenceChanges; }

private:
  QSettings& m_settings;
  int m_persistenceChanges;
};

/**
 * @brief A project's connection settings are mirrored onto the Setup pane's driver instance so
 *        the pane shows them; held persistence keeps that mirror out of the pane's saved defaults
 *        without demoting the instance from the one that owns them.
 */
class TstDriverPersistenceHold : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void aHeldApplyLeavesTheSavedDefaultAlone();
  void releasingTheHoldRestoresWrites();
  void theHoldNeverChangesTheInstanceRole();
  void aSubObjectHearsEveryChange();

private:
  [[nodiscard]] QSettings& settings();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects QSettings into the test sandbox so the suite never touches a real installation.
 */
void TstDriverPersistenceHold::initTestCase()
{
  QStandardPaths::setTestModeEnabled(true);
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_driver_persistence_hold"));
}

/**
 * @brief Starts every case from the user's own saved default.
 */
void TstDriverPersistenceHold::init()
{
  settings().clear();
  settings().setValue(QStringLiteral("host"), QStringLiteral("setup-pane"));
}

/**
 * @brief The one settings store the suite shares.
 */
QSettings& TstDriverPersistenceHold::settings()
{
  static QSettings store;
  return store;
}

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief A project's settings applied under the hold reach the driver but not the saved default.
 */
void TstDriverPersistenceHold::aHeldApplyLeavesTheSavedDefaultAlone()
{
  PersistingDriver driver(settings());
  QJsonObject project;
  project.insert(QStringLiteral("host"), QStringLiteral("project"));

  driver.setPersistenceHeld(true);
  QVERIFY(!driver.persistent());
  driver.applyConnectionSettings(project);
  driver.setPersistenceHeld(false);

  QCOMPARE(settings().value(QStringLiteral("host")).toString(), QStringLiteral("setup-pane"));
}

/**
 * @brief A user's edit in the Setup pane after the mirror is remembered as before.
 */
void TstDriverPersistenceHold::releasingTheHoldRestoresWrites()
{
  PersistingDriver driver(settings());
  driver.setPersistenceHeld(true);
  driver.setPersistenceHeld(false);
  QVERIFY(driver.persistent());

  driver.setDriverProperty(QStringLiteral("host"), QStringLiteral("user-edit"));
  QCOMPARE(settings().value(QStringLiteral("host")).toString(), QStringLiteral("user-edit"));
}

/**
 * @brief The industrial drivers route status through their live session only from the instance
 *        that owns the saved defaults; a hold must not reroute it mid-apply.
 */
void TstDriverPersistenceHold::theHoldNeverChangesTheInstanceRole()
{
  PersistingDriver setup(settings());
  setup.setPersistenceHeld(true);
  QVERIFY(setup.ownsSavedDefaults());
  setup.setPersistenceHeld(false);

  PersistingDriver live(settings());
  live.setPersistent(false);
  QVERIFY(!live.ownsSavedDefaults());
  QVERIFY(!live.persistent());
  live.setPersistenceHeld(false);
  QVERIFY(!live.persistent());
}

/**
 * @brief A driver with a sub-object that persists on its own (Modbus register groups) is told of
 *        both the role switch and the hold.
 */
void TstDriverPersistenceHold::aSubObjectHearsEveryChange()
{
  PersistingDriver driver(settings());
  driver.setPersistent(false);
  driver.setPersistenceHeld(true);
  driver.setPersistenceHeld(false);
  QCOMPARE(driver.persistenceChanges(), 3);
}

QTEST_MAIN(TstDriverPersistenceHold)

#include "tst_driver_persistence_hold.moc"
