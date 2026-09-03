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

#include <atomic>
#include <functional>
#include <QTest>
#include <QThread>

#include "Platform/AppPlatform.h"

// The real-time scheduling band is per thread and is never inherited by a QThread (spec 0075, N2;
// the MMCSS coexistence contract in startup.md). Until this change the guard was a process-wide
// static set by the first caller -- the GUI thread, from initializeQmlInterface -- so the boost
// went to QML rendering and the acquisition thread, which is the one that needs the latency, could
// never claim it: its call found the latch already taken and returned. What is pinned here is that
// guard, not the Windows API behind it: registering on one thread must leave every other thread
// unregistered, and each thread's own call must be idempotent. The AvSetMmThreadCharacteristics
// half is Windows-only and untestable headless; the latch is recorded on every platform so this
// regression cannot come back on a developer's machine either.

class MmcssRegistrationTest : public QObject {
  Q_OBJECT

private slots:
  void mainThreadStartsUnregistered();
  void registrationLandsOnTheCallingThreadOnly();
  void registrationIsIdempotentPerThread();
  void bothRolesShareThePerThreadLatch();

private:
  [[nodiscard]] static bool runOnWorker(const std::function<void()>& body);
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs @p body to completion on a throwaway thread; returns false if it never finished.
 */
bool MmcssRegistrationTest::runOnWorker(const std::function<void()>& body)
{
  class Worker : public QThread {
  public:
    explicit Worker(const std::function<void()>& fn) : m_fn(fn) {}

    void run() override { m_fn(); }

  private:
    std::function<void()> m_fn;
  };

  Worker worker(body);
  worker.start();
  return worker.wait(5000);
}

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief The test's own thread never registers, so it is the control for every case below.
 */
void MmcssRegistrationTest::mainThreadStartsUnregistered()
{
  QVERIFY(!Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
}

/**
 * @brief A worker that registers becomes registered; the thread that started it does not. This is
 *        the defect: a process-wide latch made the second thread's call a silent no-op.
 */
void MmcssRegistrationTest::registrationLandsOnTheCallingThreadOnly()
{
  std::atomic<bool> registeredInWorker{false};
  const bool finished = runOnWorker([&registeredInWorker] {
    Platform::AppPlatform::registerIngestThreadWithMmcss();
    registeredInWorker.store(Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
  });

  QVERIFY(finished);
  QVERIFY(registeredInWorker.load());
  QVERIFY(!Platform::AppPlatform::mmcssRegisteredOnCurrentThread());

  std::atomic<bool> registeredInSecondWorker{true};
  const bool secondFinished = runOnWorker([&registeredInSecondWorker] {
    registeredInSecondWorker.store(Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
  });

  QVERIFY(secondFinished);
  QVERIFY(!registeredInSecondWorker.load());
}

/**
 * @brief Repeating the call on one thread is a no-op, so a worker recreated per session cannot
 *        stack registrations.
 */
void MmcssRegistrationTest::registrationIsIdempotentPerThread()
{
  std::atomic<bool> stillRegistered{false};
  const bool finished = runOnWorker([&stillRegistered] {
    Platform::AppPlatform::registerIngestThreadWithMmcss();
    Platform::AppPlatform::registerIngestThreadWithMmcss();
    Platform::AppPlatform::registerIngestThreadWithMmcss();
    stillRegistered.store(Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
  });

  QVERIFY(finished);
  QVERIFY(stillRegistered.load());
  QVERIFY(!Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
}

QTEST_GUILESS_MAIN(MmcssRegistrationTest)

/**
 * @brief Both roles claim a band and both go through the one per-thread latch, so a thread that
 *        already registered as a renderer cannot be re-registered as an ingest thread and quietly
 *        change profile underneath itself. The GUI thread takes the render role so another process
 *        cannot cost the user frames; the pipeline takes the ingest role, whose deadline is a
 *        dropped measurement rather than a dropped repaint.
 */
void MmcssRegistrationTest::bothRolesShareThePerThreadLatch()
{
  std::atomic<bool> registeredAfterRender{false};

  QVERIFY(runOnWorker([&] {
    Platform::AppPlatform::registerRenderThreadWithMmcss();
    Platform::AppPlatform::registerIngestThreadWithMmcss();
    registeredAfterRender.store(Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
  }));

  QVERIFY(registeredAfterRender.load());
  QVERIFY(!Platform::AppPlatform::mmcssRegisteredOnCurrentThread());
}

#include "tst_mmcss_registration.moc"
