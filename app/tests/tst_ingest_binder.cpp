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

#include <memory>
#include <QByteArray>
#include <QIODevice>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <vector>

#include "Core/IO/FrameConfig.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/IO/IIngestBinder.h"
#include "IO/DeviceManager.h"

// The seam between the device layer and the acquisition pipeline (spec 0077): a DeviceManager
// hands its driver to an IIngestBinder and never names a reader, so the contract under test is
// which binder calls each lifecycle step makes, against a recording binder and a stub driver.

/**
 * @brief Minimal concrete HAL driver whose open state the test controls.
 */
class StubDriver : public IO::HAL_Driver {
  Q_OBJECT

public:
  void close() override { m_open = false; }

  [[nodiscard]] bool isOpen() const noexcept override { return m_open; }

  [[nodiscard]] bool isReadable() const noexcept override { return true; }

  [[nodiscard]] bool isWritable() const noexcept override { return true; }

  [[nodiscard]] bool configurationOk() const noexcept override { return true; }

  [[nodiscard]] qint64 write(const QByteArray& data) override { return data.size(); }

  [[nodiscard]] bool open(const QIODevice::OpenMode) override
  {
    m_open = true;
    return true;
  }

  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override { return {}; }

public slots:

  void setDriverProperty(const QString&, const QVariant&) override {}

public:
  void feed(const QByteArray& bytes) { publishReceivedData(bytes); }

private:
  bool m_open = false;
};

/**
 * @brief One recorded binder call: the operation and the identity it carried.
 */
struct BinderCall {
  QString op;
  int deviceId           = -1;
  IO::HAL_Driver* driver = nullptr;
  QString checksum;
};

/**
 * @brief Records every call so a test can assert the exact sequence the device made.
 */
class RecordingBinder : public IO::IIngestBinder {
public:
  void attach(int deviceId, IO::HAL_Driver* driver, const IO::FrameConfig& config) override
  {
    calls.push_back({.op       = QStringLiteral("attach"),
                     .deviceId = deviceId,
                     .driver   = driver,
                     .checksum = config.checksumAlgorithm});
    ++live;
  }

  void reconfigure(int deviceId, const IO::FrameConfig& config) override
  {
    calls.push_back({.op       = QStringLiteral("reconfigure"),
                     .deviceId = deviceId,
                     .checksum = config.checksumAlgorithm});
  }

  void detach(int deviceId) override
  {
    calls.push_back({.op = QStringLiteral("detach"), .deviceId = deviceId});
    --live;
  }

  void rebuildStreams(const std::vector<IO::StreamAttachment>&, bool, bool) override
  {
    calls.push_back({.op = QStringLiteral("rebuildStreams")});
  }

  void setStreamPaused(bool) override
  {
    calls.push_back({.op = QStringLiteral("setStreamPaused")});
  }

  void publishStreamTemplates() override
  {
    calls.push_back({.op = QStringLiteral("publishStreamTemplates")});
  }

  void detachStreams() override { calls.push_back({.op = QStringLiteral("detachStreams")}); }

  void injectPayload(int sourceId, const IO::CapturedDataPtr&) override
  {
    calls.push_back({.op = QStringLiteral("inject"), .deviceId = sourceId});
  }

  void resetQuickPlotHeaders() override
  {
    calls.push_back({.op = QStringLiteral("resetQuickPlotHeaders")});
  }

  [[nodiscard]] IO::LinkStats linkStats() const override { return {}; }

  std::vector<BinderCall> calls;
  int live = 0;
};

/**
 * @brief Builds a frame configuration distinguishable by its checksum name.
 */
static IO::FrameConfig makeConfig(const QString& checksum)
{
  IO::FrameConfig config;
  config.checksumAlgorithm = checksum;
  return config;
}

class TstIngestBinder : public QObject {
  Q_OBJECT

private slots:
  void constructionAttachesTheDriverOnce();
  void closeDetachesAndOpenReattaches();
  void reconfigureFollowsTheAttachment();
  void destructionDetaches();
  void writeReachesTheDriverOnlyWhileOpen();
  void rawBytesCarryTheDeviceId();
};

/**
 * @brief The constructor hands the driver to the binder exactly once, under the device id and the
 *        configuration it was given.
 */
void TstIngestBinder::constructionAttachesTheDriverOnce()
{
  RecordingBinder binder;
  auto driver     = std::make_unique<StubDriver>();
  auto* rawDriver = driver.get();

  IO::DeviceManager device(3, std::move(driver), makeConfig(QStringLiteral("CRC-8")), binder);

  QCOMPARE(binder.calls.size(), static_cast<size_t>(1));
  QCOMPARE(binder.calls[0].op, QStringLiteral("attach"));
  QCOMPARE(binder.calls[0].deviceId, 3);
  QCOMPARE(binder.calls[0].driver, rawDriver);
  QCOMPARE(binder.calls[0].checksum, QStringLiteral("CRC-8"));
  QVERIFY(device.isAttached());
  QCOMPARE(binder.live, 1);
}

/**
 * @brief close() detaches once and stays idempotent; open() after it re-attaches before dialing.
 */
void TstIngestBinder::closeDetachesAndOpenReattaches()
{
  RecordingBinder binder;
  IO::DeviceManager device(
    0, std::make_unique<StubDriver>(), makeConfig(QStringLiteral("CRC-8")), binder);

  device.close();
  QVERIFY(!device.isAttached());
  QCOMPARE(binder.calls.back().op, QStringLiteral("detach"));
  QCOMPARE(binder.live, 0);

  const auto callsAfterClose = binder.calls.size();
  device.close();
  QCOMPARE(binder.calls.size(), callsAfterClose);

  QVERIFY(device.open(QIODevice::ReadWrite));
  QVERIFY(device.isOpen());
  QVERIFY(device.isAttached());
  QCOMPARE(binder.calls.back().op, QStringLiteral("attach"));
  QCOMPARE(binder.live, 1);
}

/**
 * @brief An attached device reconfigures in place; a detached one attaches with the new framing.
 */
void TstIngestBinder::reconfigureFollowsTheAttachment()
{
  RecordingBinder binder;
  IO::DeviceManager device(
    1, std::make_unique<StubDriver>(), makeConfig(QStringLiteral("CRC-8")), binder);

  device.reconfigure(makeConfig(QStringLiteral("CRC-16")));
  QCOMPARE(binder.calls.back().op, QStringLiteral("reconfigure"));
  QCOMPARE(binder.calls.back().deviceId, 1);
  QCOMPARE(binder.calls.back().checksum, QStringLiteral("CRC-16"));

  device.close();
  device.reconfigure(makeConfig(QStringLiteral("CRC-32")));
  QCOMPARE(binder.calls.back().op, QStringLiteral("attach"));
  QCOMPARE(binder.calls.back().checksum, QStringLiteral("CRC-32"));
  QVERIFY(device.isAttached());
  QCOMPARE(binder.live, 1);
}

/**
 * @brief Destroying the device retires its reader: no attachment outlives its owner.
 */
void TstIngestBinder::destructionDetaches()
{
  RecordingBinder binder;
  {
    IO::DeviceManager device(
      2, std::make_unique<StubDriver>(), makeConfig(QStringLiteral("CRC-8")), binder);
    QCOMPARE(binder.live, 1);
  }

  QCOMPARE(binder.live, 0);
  QCOMPARE(binder.calls.back().op, QStringLiteral("detach"));
  QCOMPARE(binder.calls.back().deviceId, 2);
}

/**
 * @brief A closed driver rejects writes; an open one reports the bytes it accepted.
 */
void TstIngestBinder::writeReachesTheDriverOnlyWhileOpen()
{
  RecordingBinder binder;
  IO::DeviceManager device(
    0, std::make_unique<StubDriver>(), makeConfig(QStringLiteral("CRC-8")), binder);

  QCOMPARE(device.write(QByteArrayLiteral("abc")), static_cast<qint64>(-1));

  QVERIFY(device.open(QIODevice::ReadWrite));
  QCOMPARE(device.write(QByteArrayLiteral("abc")), static_cast<qint64>(3));
}

/**
 * @brief The driver's chunks are re-emitted tagged with the device id, which is what the router
 *        fans out to the raw taps.
 */
void TstIngestBinder::rawBytesCarryTheDeviceId()
{
  RecordingBinder binder;
  auto driver     = std::make_unique<StubDriver>();
  auto* rawDriver = driver.get();
  IO::DeviceManager device(5, std::move(driver), makeConfig(QStringLiteral("CRC-8")), binder);

  QSignalSpy spy(&device, &IO::DeviceManager::rawDataReceived);
  rawDriver->feed(QByteArrayLiteral("hello"));

  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toInt(), 5);
  const auto captured = spy.at(0).at(1).value<IO::CapturedDataPtr>();
  QVERIFY(captured != nullptr);
  QCOMPARE(captured->data, QByteArrayLiteral("hello"));
}

QTEST_GUILESS_MAIN(TstIngestBinder)
#include "tst_ingest_binder.moc"
