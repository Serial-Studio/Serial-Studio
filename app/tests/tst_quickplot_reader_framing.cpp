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

#include "DataModel/FrameBuilder/QuickPlotReaderFraming.h"

/**
 * @brief The two decisions that keep a declared binary Quick Plot session in step with the
 *        reader that frames it (spec 0111 R14): the builder's binary flag flips on the pipeline
 *        thread at once, while the reader is only recreated after a round trip through AppState.
 */
class TstQuickPlotReaderFraming : public QObject {
  Q_OBJECT

private slots:
  void lineFramedBatchesNeverTakeThePackedDecode();
  void aDeclaredReaderOpensThePackedLane();
  void aReaderBackOnLinesWhileBinaryIsHeldIsAMissedReconnect();
  void theSwitchWindowAndADroppedDeclarationAreNotLosses();
};

/**
 * @brief The window between declare() and the reader's recreation: a newline-framed line of
 *        exactly the packed length used to decode as nonsense and be staged once.
 */
void TstQuickPlotReaderFraming::lineFramedBatchesNeverTakeThePackedDecode()
{
  QVERIFY(!DataModel::packedLaneOpen(true, false));
  QVERIFY(!DataModel::packedLaneOpen(false, false));
}

/**
 * @brief Both halves agree: the declaration holds binary and the reader frames with its
 *        delimiters. A reader still declared after the builder dropped binary stays shut.
 */
void TstQuickPlotReaderFraming::aDeclaredReaderOpensThePackedLane()
{
  QVERIFY(DataModel::packedLaneOpen(true, true));
  QVERIFY(!DataModel::packedLaneOpen(false, true));
}

/**
 * @brief A reconnect the builder never saw leaves it binary while AppState handed the reader
 *        line framing again; every line was dropped and the device's re-declaration deduped.
 */
void TstQuickPlotReaderFraming::aReaderBackOnLinesWhileBinaryIsHeldIsAMissedReconnect()
{
  QVERIFY(DataModel::declaredFramingLost(true, false, true));
}

/**
 * @brief Not a loss: the reader has not switched yet (was line, still line), it is still
 *        declared, or the builder itself dropped binary and the reader followed.
 */
void TstQuickPlotReaderFraming::theSwitchWindowAndADroppedDeclarationAreNotLosses()
{
  QVERIFY(!DataModel::declaredFramingLost(false, false, true));
  QVERIFY(!DataModel::declaredFramingLost(true, true, true));
  QVERIFY(!DataModel::declaredFramingLost(true, false, false));
  QVERIFY(!DataModel::declaredFramingLost(false, true, true));
}

QTEST_APPLESS_MAIN(TstQuickPlotReaderFraming)

#include "tst_quickplot_reader_framing.moc"
