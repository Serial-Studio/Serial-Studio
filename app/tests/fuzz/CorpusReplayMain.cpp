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

#include <cstddef>
#include <cstdint>
#include <QDir>
#include <QFile>
#include <QTest>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size);

//--------------------------------------------------------------------------------------------------
// Corpus replay harness
//--------------------------------------------------------------------------------------------------

/**
 * @brief QTest driver that replays a checked-in corpus through a libFuzzer entry point.
 *
 * ENABLE_FUZZERS is OFF in every default configure, so without this the entry points would
 * compile only for whoever remembered to turn fuzzing on, and the regression seeds would never
 * run at all. ss_add_fuzz_target() links this file into the target and defines
 * SS_FUZZ_CORPUS_DIR to the target's own corpus directory.
 */
class CorpusReplay : public QObject {
  Q_OBJECT

private slots:
  void replaysEverySeed();
};

/**
 * @brief Feeds every file in the corpus directory through the entry point.
 */
void CorpusReplay::replaysEverySeed()
{
  const QDir dir(QStringLiteral(SS_FUZZ_CORPUS_DIR));
  QVERIFY2(dir.exists(), qPrintable(dir.absolutePath()));

  const auto seeds = dir.entryInfoList(QDir::Files, QDir::Name);
  QVERIFY2(!seeds.isEmpty(), qPrintable(dir.absolutePath()));

  for (const auto& seed : seeds) {
    QFile file(seed.absoluteFilePath());
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(seed.absoluteFilePath()));

    const QByteArray bytes = file.readAll();
    const auto* data =
      static_cast<const std::uint8_t*>(static_cast<const void*>(bytes.constData()));
    QCOMPARE(LLVMFuzzerTestOneInput(data, static_cast<std::size_t>(bytes.size())), 0);
  }
}

QTEST_GUILESS_MAIN(CorpusReplay)

#include "CorpusReplayMain.moc"
