/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <chrono>
#include <QByteArray>
#include <QHash>
#include <QIODevice>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPair>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QVariant>
#include <utility>

#include "IO/HAL_Driver.h"

// HAL_Driver.h is a header-only Q_OBJECT class with no paired .cpp anywhere in the tree, so
// AUTOMOC's source/header basename pairing never fires for it; this explicit include is what
// tells AUTOMOC to moc the header and link its QMetaObject machinery into this suite.
#include "IO/moc_HAL_Driver.cpp"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Test doubles
//--------------------------------------------------------------------------------------------------

/**
 * @brief Minimal concrete HAL_Driver that records every call so the base-class default
 *        implementations can be asserted against, both through the base pointer and via
 *        explicit override.
 */
class FakeDriver : public IO::HAL_Driver {
  Q_OBJECT

public:
  explicit FakeDriver(QObject* parent = nullptr)
    : IO::HAL_Driver(parent), m_openResult(true), m_selectResult(false), m_closeCalls(0)
  {}

  void setOpenResult(bool ok) { m_openResult = ok; }

  void setSelectResult(bool ok) { m_selectResult = ok; }

  [[nodiscard]] int closeCalls() const { return m_closeCalls; }

  [[nodiscard]] const QList<QIODevice::OpenMode>& openCalls() const { return m_openCalls; }

  [[nodiscard]] const QList<QPair<QString, QVariant>>& propertyCalls() const
  {
    return m_propertyCalls;
  }

  [[nodiscard]] const QList<QJsonObject>& selectCalls() const { return m_selectCalls; }

  void close() override { ++m_closeCalls; }

  [[nodiscard]] bool isOpen() const noexcept override { return false; }

  [[nodiscard]] bool isReadable() const noexcept override { return true; }

  [[nodiscard]] bool isWritable() const noexcept override { return true; }

  [[nodiscard]] bool configurationOk() const noexcept override { return true; }

  [[nodiscard]] qint64 write(const QByteArray& data) override { return data.size(); }

  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override
  {
    m_openCalls.append(mode);
    return m_openResult;
  }

  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override { return {}; }

  [[nodiscard]] bool selectByIdentifier(const QJsonObject& id) override
  {
    m_selectCalls.append(id);
    return m_selectResult;
  }

  void emitLinkDropped() { Q_EMIT linkDropped(); }

  void publishCopy(const QByteArray& data, std::chrono::nanoseconds frameStep)
  {
    publishReceivedData(data, IO::CapturedData::SteadyClock::now(), frameStep);
  }

  void publishMove(QByteArray data, std::chrono::nanoseconds frameStep)
  {
    publishReceivedData(std::move(data), IO::CapturedData::SteadyClock::now(), frameStep);
  }

public slots:

  void setDriverProperty(const QString& key, const QVariant& value) override
  {
    m_propertyCalls.append(qMakePair(key, value));
  }

private:
  bool m_openResult;
  bool m_selectResult;
  int m_closeCalls;
  QList<QIODevice::OpenMode> m_openCalls;
  QList<QPair<QString, QVariant>> m_propertyCalls;
  QList<QJsonObject> m_selectCalls;
};

/**
 * @brief FakeDriver that overrides the spec-0034 async-open seam, used to prove an override
 *        surfaces through a base HAL_Driver pointer rather than being hidden behind static
 *        typing.
 */
class OverridingDriver : public FakeDriver {
  Q_OBJECT

public:
  [[nodiscard]] bool supportsAsyncOpen() const noexcept override { return true; }

  [[nodiscard]] int openTimeoutMsec() const noexcept override { return 5000; }
};

//--------------------------------------------------------------------------------------------------
// Suite
//--------------------------------------------------------------------------------------------------

/**
 * @brief Covers the default implementations HAL_Driver ships for every driver that does not
 *        override them: the async-open seam, applyConnectionSettings() routing, and the
 *        publishReceivedData()/makeCapturedData() capture path.
 */
class TstHalDriver : public QObject {
  Q_OBJECT

private slots:
  void asyncOpenSeamDefaultsThroughBasePointer();
  void asyncOpenSeamOverridesThroughBasePointer();

  void beginOpenDefaultEmitsSuccessOnOpenTrue();
  void beginOpenDefaultEmitsFailureReasonOnOpenFalse();
  void abortOpenDefaultCallsCloseOnce();
  void selectByIdentifierDefaultReturnsFalse();

  void applyConnectionSettingsEmptyMakesNoPropertyCalls();
  void applyConnectionSettingsSetsOnePropertyPerKey();
  void applyConnectionSettingsRoutesObjectDeviceIdToSelectByIdentifier();
  void applyConnectionSettingsIgnoresNonObjectDeviceId();

  void publishReceivedDataCopyOverloadCarriesBytes();
  void publishReceivedDataMoveOverloadCarriesBytes();

  void makeCapturedDataClampsFrameStepToOneNanosecond_data();
  void makeCapturedDataClampsFrameStepToOneNanosecond();

  void deviceIdentifierDefaultReturnsEmptyObject();
  void linkDroppedSignalEmitsOnDirectConnection();
};

//--------------------------------------------------------------------------------------------------
// Async-open seam (spec 0034)
//--------------------------------------------------------------------------------------------------

/**
 * @brief A driver that overrides neither hook keeps the synchronous-open defaults, seen through
 *        the base pointer every caller actually holds.
 */
void TstHalDriver::asyncOpenSeamDefaultsThroughBasePointer()
{
  FakeDriver driver;
  IO::HAL_Driver* base = &driver;

  QCOMPARE(base->supportsAsyncOpen(), false);
  QCOMPARE(base->openTimeoutMsec(), 0);
}

/**
 * @brief A driver that does override the hooks is not shadowed by HAL_Driver's own defaults.
 */
void TstHalDriver::asyncOpenSeamOverridesThroughBasePointer()
{
  OverridingDriver driver;
  IO::HAL_Driver* base = &driver;

  QCOMPARE(base->supportsAsyncOpen(), true);
  QCOMPARE(base->openTimeoutMsec(), 5000);
}

/**
 * @brief The default beginOpen() is today's synchronous open(), reported immediately.
 */
void TstHalDriver::beginOpenDefaultEmitsSuccessOnOpenTrue()
{
  FakeDriver driver;
  driver.setOpenResult(true);

  QSignalSpy spy(&driver, &IO::HAL_Driver::openFinished);
  driver.beginOpen(QIODevice::ReadWrite);

  QCOMPARE(driver.openCalls().size(), 1);
  QCOMPARE(int(driver.openCalls().at(0)), int(QIODevice::ReadWrite));
  QCOMPARE(spy.count(), 1);

  const auto args = spy.takeFirst();
  QCOMPARE(args.at(0).toBool(), true);
  QVERIFY(args.at(1).toString().isEmpty());
}

/**
 * @brief A failing open() still resolves the attempt, with a non-empty reason a caller can show.
 */
void TstHalDriver::beginOpenDefaultEmitsFailureReasonOnOpenFalse()
{
  FakeDriver driver;
  driver.setOpenResult(false);

  QSignalSpy spy(&driver, &IO::HAL_Driver::openFinished);
  driver.beginOpen(QIODevice::ReadOnly);

  QCOMPARE(spy.count(), 1);
  const auto args = spy.takeFirst();
  QCOMPARE(args.at(0).toBool(), false);
  QVERIFY(!args.at(1).toString().isEmpty());
}

/**
 * @brief abortOpen() releases a half-open link the same way every driver already does: close().
 */
void TstHalDriver::abortOpenDefaultCallsCloseOnce()
{
  FakeDriver driver;
  driver.abortOpen();

  QCOMPARE(driver.closeCalls(), 1);
}

/**
 * @brief A driver that never opts into device selection returns false and touches no state; the
 *        base implementation is invoked directly, bypassing FakeDriver's own recording override.
 */
void TstHalDriver::selectByIdentifierDefaultReturnsFalse()
{
  FakeDriver driver;

  QJsonObject id;
  id.insert(QStringLiteral("port"), QStringLiteral("COM3"));

  QVERIFY(!driver.IO::HAL_Driver::selectByIdentifier(id));
  QVERIFY(driver.selectCalls().isEmpty());
}

//--------------------------------------------------------------------------------------------------
// applyConnectionSettings() routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty settings object short-circuits before touching a single property.
 */
void TstHalDriver::applyConnectionSettingsEmptyMakesNoPropertyCalls()
{
  FakeDriver driver;
  driver.applyConnectionSettings(QJsonObject());

  QVERIFY(driver.propertyCalls().isEmpty());
  QVERIFY(driver.selectCalls().isEmpty());
}

/**
 * @brief Every key in the settings object becomes exactly one setDriverProperty() call, with its
 *        JSON value carried through as the equivalent QVariant.
 */
void TstHalDriver::applyConnectionSettingsSetsOnePropertyPerKey()
{
  FakeDriver driver;

  QJsonObject settings;
  settings.insert(QStringLiteral("baudRate"), 115200);
  settings.insert(QStringLiteral("parity"), QStringLiteral("none"));

  driver.applyConnectionSettings(settings);

  QCOMPARE(driver.propertyCalls().size(), 2);

  QHash<QString, QVariant> byKey;
  for (const auto& call : driver.propertyCalls())
    byKey.insert(call.first, call.second);

  QCOMPARE(byKey.value(QStringLiteral("baudRate")).toInt(), 115200);
  QCOMPARE(byKey.value(QStringLiteral("parity")).toString(), QStringLiteral("none"));
}

/**
 * @brief A "deviceId" key whose value is a JSON object also routes to selectByIdentifier() with
 *        that sub-object, on top of the ordinary property call every key receives.
 */
void TstHalDriver::applyConnectionSettingsRoutesObjectDeviceIdToSelectByIdentifier()
{
  FakeDriver driver;

  QJsonObject deviceId;
  deviceId.insert(QStringLiteral("vendorId"), 0x2E8A);

  QJsonObject settings;
  settings.insert(QStringLiteral("deviceId"), deviceId);

  driver.applyConnectionSettings(settings);

  QCOMPARE(driver.selectCalls().size(), 1);
  QCOMPARE(driver.selectCalls().at(0), deviceId);
  QCOMPARE(driver.propertyCalls().size(), 1);
  QCOMPARE(driver.propertyCalls().at(0).first, QStringLiteral("deviceId"));
}

/**
 * @brief A "deviceId" key whose value is not a JSON object never reaches selectByIdentifier().
 */
void TstHalDriver::applyConnectionSettingsIgnoresNonObjectDeviceId()
{
  FakeDriver driver;

  QJsonObject settings;
  settings.insert(QStringLiteral("deviceId"), QStringLiteral("COM7"));

  driver.applyConnectionSettings(settings);

  QVERIFY(driver.selectCalls().isEmpty());
  QCOMPARE(driver.propertyCalls().size(), 1);
}

//--------------------------------------------------------------------------------------------------
// publishReceivedData() / makeCapturedData()
//--------------------------------------------------------------------------------------------------

/**
 * @brief The copy overload emits a CapturedData whose bytes match the caller's buffer exactly.
 */
void TstHalDriver::publishReceivedDataCopyOverloadCarriesBytes()
{
  FakeDriver driver;
  IO::CapturedDataPtr captured;
  int emitCount = 0;

  connect(
    &driver,
    &IO::HAL_Driver::dataReceived,
    &driver,
    [&](const IO::CapturedDataPtr& data) {
      captured = data;
      ++emitCount;
    },
    Qt::DirectConnection);

  const QByteArray payload("copy-payload");
  driver.publishCopy(payload, std::chrono::nanoseconds(2000));

  QCOMPARE(emitCount, 1);
  QVERIFY(static_cast<bool>(captured));
  QCOMPARE(captured->data, payload);
  QCOMPARE(captured->frameStep, std::chrono::nanoseconds(2000));
}

/**
 * @brief The move overload carries the same bytes through without a caller-visible copy.
 */
void TstHalDriver::publishReceivedDataMoveOverloadCarriesBytes()
{
  FakeDriver driver;
  IO::CapturedDataPtr captured;

  connect(
    &driver,
    &IO::HAL_Driver::dataReceived,
    &driver,
    [&](const IO::CapturedDataPtr& data) { captured = data; },
    Qt::DirectConnection);

  QByteArray payload("move-payload");
  driver.publishMove(std::move(payload), std::chrono::nanoseconds(3000));

  QVERIFY(static_cast<bool>(captured));
  QCOMPARE(captured->data, QByteArray("move-payload"));
  QCOMPARE(captured->frameStep, std::chrono::nanoseconds(3000));
}

void TstHalDriver::makeCapturedDataClampsFrameStepToOneNanosecond_data()
{
  QTest::addColumn<qint64>("requestedNanos");
  QTest::addColumn<qint64>("expectedNanos");

  QTest::newRow("zero") << qint64(0) << qint64(1);
  QTest::newRow("negative") << qint64(-500) << qint64(1);
  QTest::newRow("positive") << qint64(2000) << qint64(2000);
}

/**
 * @brief A zero or negative frameStep would make FrameReader's timing math divide by zero or go
 *        backwards, so makeCapturedData() floors it to one nanosecond instead of passing it
 * through.
 */
void TstHalDriver::makeCapturedDataClampsFrameStepToOneNanosecond()
{
  QFETCH(qint64, requestedNanos);
  QFETCH(qint64, expectedNanos);

  const auto captured = IO::makeCapturedData(QByteArray("x"),
                                             IO::CapturedData::SteadyClock::now(),
                                             std::chrono::nanoseconds(requestedNanos));

  QCOMPARE(captured->frameStep.count(), expectedNanos);
}

//--------------------------------------------------------------------------------------------------
// Identifier and link-drop seam
//--------------------------------------------------------------------------------------------------

/**
 * @brief A driver with no stable hardware identifier reports an empty object rather than null.
 */
void TstHalDriver::deviceIdentifierDefaultReturnsEmptyObject()
{
  FakeDriver driver;
  IO::HAL_Driver* base = &driver;

  QVERIFY(base->deviceIdentifier().isEmpty());
}

/**
 * @brief linkDropped is a plain signal with no default handler; a direct connection observes it
 *        exactly once per emission, appless.
 */
void TstHalDriver::linkDroppedSignalEmitsOnDirectConnection()
{
  FakeDriver driver;
  QSignalSpy spy(&driver, &IO::HAL_Driver::linkDropped);

  driver.emitLinkDropped();

  QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(TstHalDriver)

#include "tst_hal_driver.moc"
