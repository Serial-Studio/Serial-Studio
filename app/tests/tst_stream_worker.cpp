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

#include <QTest>

#include "DataModel/FrameBuilder.h"
#include "IO/StreamWorker.h"

/**
 * StreamWorker.cpp references these two FrameBuilder members when a frame builder is injected;
 * every suite construction passes the default nullptr, so the calls never execute here. Defining
 * them as stubs satisfies the linker without dragging FrameBuilder.cpp's dependency web into the
 * lean test tier (they are the only FrameBuilder symbols this link set may reference).
 */
void DataModel::FrameBuilder::injectTableApiLua(lua_State*) {}

void DataModel::FrameBuilder::injectTableApiJS(QJSEngine*) {}

using DataModel::DataBlockPtr;
using IO::SampleBlock;
using IO::SampleBlockPtr;
using IO::StreamChannelConfig;
using IO::StreamConfig;
using IO::StreamProcessor;
using IO::StreamWorker;

/**
 * @brief Minimal concrete HAL driver used to feed the worker's block signal.
 */
class StubDriver : public IO::HAL_Driver {
  Q_OBJECT

public:
  void close() override {}

  [[nodiscard]] bool isOpen() const noexcept override { return true; }

  [[nodiscard]] bool isReadable() const noexcept override { return true; }

  [[nodiscard]] bool isWritable() const noexcept override { return false; }

  [[nodiscard]] bool isStreamCapable() const noexcept override { return true; }

  [[nodiscard]] bool configurationOk() const noexcept override { return true; }

  [[nodiscard]] qint64 write(const QByteArray&) override { return -1; }

  [[nodiscard]] bool open(const QIODevice::OpenMode) override { return true; }

  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override { return {}; }

public slots:

  void setDriverProperty(const QString&, const QVariant&) override {}

public:
  void feed(const SampleBlockPtr& block) { publishSampleBlock(block); }
};

/**
 * @brief Builds a mono block of @p values at 48 kHz starting at a fixed epoch.
 */
static SampleBlockPtr makeBlock(const std::vector<float>& values)
{
  auto block      = std::make_shared<SampleBlock>();
  block->channels = 1;
  block->frames   = static_cast<qsizetype>(values.size());
  block->samples  = values;
  block->t0       = SampleBlock::SteadyTimePoint(std::chrono::seconds(1));
  block->dt       = std::chrono::nanoseconds(1'000'000'000 / 48'000);
  return block;
}

/**
 * @brief Builds a one-channel stream config with optional transform code and FFT.
 */
static StreamConfig makeConfig(const QString& transform = QString(),
                               int language             = 1,
                               int fftSamples           = 0)
{
  StreamConfig config;
  config.sourceId   = 0;
  config.channels   = 1;
  config.sampleRate = 48000.0;

  StreamChannelConfig channel;
  channel.uniqueId          = 7;
  channel.channel           = 0;
  channel.plot              = true;
  channel.fft               = fftSamples > 0;
  channel.fftSamples        = fftSamples;
  channel.transformCode     = transform;
  channel.transformLanguage = language;
  config.datasets.push_back(channel);
  return config;
}

/**
 * @brief Collects the blocks a worker publishes. Since spec 0055 D8 the worker owns no display
 *        ring: it emits blockReady on its own thread, so the connection is queued and
 *        QTest::qWait pumps the blocks onto the test thread.
 */
class BlockCollector : public QObject {
  Q_OBJECT

public:
  explicit BlockCollector(StreamWorker& worker)
  {
    connect(
      worker.processor(),
      &StreamProcessor::blockReady,
      this,
      [this](const DataBlockPtr& block) {
        if (block)
          m_blocks.push_back(block);
      },
      Qt::QueuedConnection);
  }

  [[nodiscard]] const std::vector<DataBlockPtr>& blocks() const noexcept { return m_blocks; }

  /**
   * @brief Last sample of the first column across every collected block, or 0 when none arrived.
   */
  [[nodiscard]] double latest() const
  {
    for (auto it = m_blocks.rbegin(); it != m_blocks.rend(); ++it) {
      const auto& block = **it;
      if (block.columns.empty() || block.samples <= 0)
        continue;

      return block.columns.front().values[static_cast<std::size_t>(block.samples - 1)];
    }

    return 0.0;
  }

private:
  std::vector<DataBlockPtr> m_blocks;
};

/**
 * @brief Unit coverage for the stream worker: teardown ordering (the repo's recurring crash
 *        class), impulse survival through the display path (AC9 logic tier), transforms, and the
 *        Safe-mode runaway abort (AC8/AC17 logic tier).
 */
class TestStreamWorker : public QObject {
  Q_OBJECT

private slots:
  void teardownJoinsAndIsIdempotent();
  void impulseSurvivesDisplayPath();
  void blockTransformApplies();
  void perSampleTransformApplies();
  void safeModeAbortsRunawayTransform();
  void jsRunawayBlockTransformTimesOut();
  void jsRunawayPerSampleTransformTimesOut();
  void outOfRangeChannelClearsItsColumn();
  void fftWindowPublishedWhenFilled();
};

/**
 * @brief stop() joins the thread, twice in a row, and destruction after stop is clean.
 */
void TestStreamWorker::teardownJoinsAndIsIdempotent()
{
  StubDriver driver;
  auto worker = std::make_unique<StreamWorker>(&driver, makeConfig());

  driver.feed(makeBlock({1.0f, 2.0f, 3.0f}));
  QTest::qWait(100);

  worker->stop();
  QVERIFY(!worker->abandoned());
  worker->stop();
  QVERIFY(!worker->abandoned());
  worker.reset();
}

/**
 * @brief A single-sample impulse in a long block must reach the published samples (R11).
 */
void TestStreamWorker::impulseSurvivesDisplayPath()
{
  StubDriver driver;
  StreamWorker worker(&driver, makeConfig());

  BlockCollector collector(worker);

  std::vector<float> samples(4800, 0.0f);
  samples[2400] = 100.0f;
  driver.feed(makeBlock(samples));

  bool found = false;
  for (int i = 0; i < 100 && !found; ++i) {
    QTest::qWait(20);
    for (const auto& block : collector.blocks())
      for (const auto& column : block->columns)
        for (qsizetype k = 0; k < block->samples; ++k)
          if (qFuzzyCompare(column.values[static_cast<std::size_t>(k)], 100.0))
            found = true;
  }

  QVERIFY2(found, "impulse peak never reached the published samples");
  worker.stop();
}

/**
 * @brief transform_block output replaces the samples (latest value reflects the transform).
 */
void TestStreamWorker::blockTransformApplies()
{
  const QString code = QStringLiteral("function transform_block(samples, info)\n"
                                      "  for i = 1, info.count do\n"
                                      "    samples[i] = samples[i] * 2\n"
                                      "  end\n"
                                      "  return samples\n"
                                      "end\n");

  StubDriver driver;
  StreamWorker worker(&driver, makeConfig(code));
  BlockCollector collector(worker);

  driver.feed(makeBlock({1.0f, 2.0f, 21.0f}));

  double latest = 0.0;
  for (int i = 0; i < 100 && latest == 0.0; ++i) {
    QTest::qWait(20);
    latest = collector.latest();
  }

  QCOMPARE(latest, 42.0);
  worker.stop();
}

/**
 * @brief The per-sample transform(value) fallback runs at full rate when no block form exists.
 */
void TestStreamWorker::perSampleTransformApplies()
{
  const QString code = QStringLiteral("function transform(value)\n"
                                      "  return value + 1\n"
                                      "end\n");

  StubDriver driver;
  StreamWorker worker(&driver, makeConfig(code));
  BlockCollector collector(worker);

  driver.feed(makeBlock({1.0f, 2.0f, 3.0f}));

  double latest = 0.0;
  for (int i = 0; i < 100 && latest == 0.0; ++i) {
    QTest::qWait(20);
    latest = collector.latest();
  }

  QCOMPARE(latest, 4.0);
  worker.stop();
}

/**
 * @brief Safe mode: a `while true` block transform is aborted by the watchdog, the block falls
 *        back to raw samples, and the error is counted (AC8 logic tier).
 */
void TestStreamWorker::safeModeAbortsRunawayTransform()
{
  const QString code = QStringLiteral("function transform_block(samples, info)\n"
                                      "  while true do end\n"
                                      "end\n");

  auto config        = makeConfig(code);
  config.luaFastMode = false;

  StubDriver driver;
  StreamWorker worker(&driver, config);
  BlockCollector collector(worker);

  driver.feed(makeBlock({5.0f, 6.0f, 7.0f}));

  double latest = 0.0;
  for (int i = 0; i < 200 && latest == 0.0; ++i) {
    QTest::qWait(25);
    latest = collector.latest();
  }

  QCOMPARE(latest, 7.0);
  QVERIFY(worker.processor() != nullptr);
  QVERIFY(worker.processor()->transformErrorCount() >= 1);
  worker.stop();
  QVERIFY(!worker.abandoned());
}

/**
 * @brief JS parity with Safe-mode Lua (A1): a `while(true)` transform_block is interrupted
 *        off-thread, the block falls back to the source's own samples, and the timeout is counted
 *        for the 1 Hz diagnostics pull. Before the fix this pinned the worker thread forever.
 */
void TestStreamWorker::jsRunawayBlockTransformTimesOut()
{
  const QString code = QStringLiteral("function transform_block(samples, info) {\n"
                                      "  while (true) {}\n"
                                      "}\n");

  StubDriver driver;
  StreamWorker worker(&driver, makeConfig(code, SerialStudio::JavaScript));
  BlockCollector collector(worker);

  driver.feed(makeBlock({5.0f, 6.0f, 7.0f}));

  double latest = 0.0;
  for (int i = 0; i < 200 && latest == 0.0; ++i) {
    QTest::qWait(25);
    latest = collector.latest();
  }

  QCOMPARE(latest, 7.0);
  QVERIFY(worker.processor() != nullptr);
  QVERIFY(worker.processor()->transformTimeoutCount() >= 1);
  QVERIFY(worker.processor()->transformErrorCount() >= 1);
  worker.stop();
  QVERIFY(!worker.abandoned());
}

/**
 * @brief The per-sample JS lane is armed once for the whole pass, so a runaway transform(value)
 *        aborts within one budget rather than per sample, and the block falls back to raw.
 */
void TestStreamWorker::jsRunawayPerSampleTransformTimesOut()
{
  const QString code = QStringLiteral("function transform(value) {\n"
                                      "  while (true) {}\n"
                                      "}\n");

  StubDriver driver;
  StreamWorker worker(&driver, makeConfig(code, SerialStudio::JavaScript));
  BlockCollector collector(worker);

  driver.feed(makeBlock({5.0f, 6.0f, 7.0f}));

  double latest = 0.0;
  for (int i = 0; i < 200 && latest == 0.0; ++i) {
    QTest::qWait(25);
    latest = collector.latest();
  }

  QCOMPARE(latest, 7.0);
  QVERIFY(worker.processor()->transformTimeoutCount() >= 1);
  worker.stop();
  QVERIFY(!worker.abandoned());
}

/**
 * @brief A block that does not carry the configured channel clears that column instead of
 *        republishing the previous block's samples (A12): the widget must go quiet, not freeze on
 *        a stale value that looks live.
 */
void TestStreamWorker::outOfRangeChannelClearsItsColumn()
{
  auto config                = makeConfig();
  config.channels            = 4;
  config.datasets[0].channel = 3;

  StubDriver driver;
  StreamWorker worker(&driver, config);
  BlockCollector collector(worker);

  auto wide      = std::make_shared<SampleBlock>();
  wide->channels = 4;
  wide->frames   = 2;
  wide->samples  = {0.0f, 0.0f, 0.0f, 11.0f, 0.0f, 0.0f, 0.0f, 22.0f};
  wide->t0       = SampleBlock::SteadyTimePoint(std::chrono::seconds(1));
  wide->dt       = std::chrono::nanoseconds(1'000'000'000 / 48'000);
  driver.feed(wide);

  double latest = 0.0;
  for (int i = 0; i < 100 && latest == 0.0; ++i) {
    QTest::qWait(20);
    latest = collector.latest();
  }

  QCOMPARE(latest, 22.0);

  const std::size_t before = collector.blocks().size();
  driver.feed(makeBlock({33.0f, 44.0f}));
  for (int i = 0; i < 100 && collector.blocks().size() == before; ++i)
    QTest::qWait(20);

  QVERIFY(collector.blocks().size() > before);
  QCOMPARE(collector.latest(), 0.0);
  worker.stop();
}

/**
 * @brief Once the FFT ring fills, updates carry a full linearized window of the newest samples.
 */
void TestStreamWorker::fftWindowPublishedWhenFilled()
{
  StubDriver driver;
  StreamWorker worker(&driver, makeConfig(QString(), 1, 8));
  BlockCollector collector(worker);

  driver.feed(makeBlock({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

  std::vector<double> window;
  for (int i = 0; i < 100 && window.empty(); ++i) {
    QTest::qWait(20);
    for (const auto& block : collector.blocks())
      if (!block->columns.empty() && !block->columns.front().fftWindow.empty())
        window = block->columns.front().fftWindow;
  }

  QCOMPARE(window.size(), std::size_t(8));
  QCOMPARE(window.front(), 3.0);
  QCOMPARE(window.back(), 10.0);
  worker.stop();
}

QTEST_MAIN(TestStreamWorker)
#include "tst_stream_worker.moc"
