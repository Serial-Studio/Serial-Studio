/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QList>
#include <QStringList>
#include <QTest>

#include "DataModel/Scripting/ScriptFrameShaping.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief KATs for the mixed scalar/vector unzip both parser engines share. A JS array and a Lua
 *        table of the same shape must produce the same frames, which is only true while one
 *        implementation answers for both.
 */
class TstScriptFrameShaping : public QObject {
  Q_OBJECT

private slots:
  void noVectorsIsOneFrame();
  void oneVectorFansOutPerIndex();
  void shortVectorPadsWithItsLastValue();
  void emptyScalarsStillFanOut();
  void lengthIsCappedAtTheHardLimit();
};

/**
 * @brief A pure-scalar return value is a single frame, scalars in order.
 */
void TstScriptFrameShaping::noVectorsIsOneFrame()
{
  const QStringList scalars{QStringLiteral("1"), QStringLiteral("2")};
  QList<QStringList> vectors;

  const auto frames = DataModel::ScriptFrames::unzipMixedFrames(scalars, vectors, 0);

  QCOMPARE(frames.size(), qsizetype(1));
  QCOMPARE(frames.at(0), scalars);
}

/**
 * @brief Every scalar repeats in each frame; the vector contributes one element per frame.
 */
void TstScriptFrameShaping::oneVectorFansOutPerIndex()
{
  const QStringList scalars{QStringLiteral("t")};
  QList<QStringList> vectors{
    QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}
  };

  const auto frames = DataModel::ScriptFrames::unzipMixedFrames(scalars, vectors, 3);

  QCOMPARE(frames.size(), qsizetype(3));
  QCOMPARE(frames.at(0), (QStringList{QStringLiteral("t"), QStringLiteral("a")}));
  QCOMPARE(frames.at(1), (QStringList{QStringLiteral("t"), QStringLiteral("b")}));
  QCOMPARE(frames.at(2), (QStringList{QStringLiteral("t"), QStringLiteral("c")}));
}

/**
 * @brief A vector shorter than the longest is held at its own last value, never dropped: a frame
 *        with a missing channel would shift every later column.
 */
void TstScriptFrameShaping::shortVectorPadsWithItsLastValue()
{
  const QStringList scalars;
  QList<QStringList> vectors{
    QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")},
    QStringList{QStringLiteral("x")}
  };

  const auto frames = DataModel::ScriptFrames::unzipMixedFrames(scalars, vectors, 3);

  QCOMPARE(frames.size(), qsizetype(3));
  QCOMPARE(frames.at(0), (QStringList{QStringLiteral("a"), QStringLiteral("x")}));
  QCOMPARE(frames.at(1), (QStringList{QStringLiteral("b"), QStringLiteral("x")}));
  QCOMPARE(frames.at(2), (QStringList{QStringLiteral("c"), QStringLiteral("x")}));
}

/**
 * @brief A return value with no scalars still fans out over the vector indices.
 */
void TstScriptFrameShaping::emptyScalarsStillFanOut()
{
  const QStringList scalars;
  QList<QStringList> vectors{
    QStringList{QStringLiteral("a"), QStringLiteral("b")}
  };

  const auto frames = DataModel::ScriptFrames::unzipMixedFrames(scalars, vectors, 2);

  QCOMPARE(frames.size(), qsizetype(2));
  QCOMPARE(frames.at(0), (QStringList{QStringLiteral("a")}));
  QCOMPARE(frames.at(1), (QStringList{QStringLiteral("b")}));
}

/**
 * @brief A malformed script result claiming a huge vector length cannot allocate past the cap.
 */
void TstScriptFrameShaping::lengthIsCappedAtTheHardLimit()
{
  const QStringList scalars;
  QList<QStringList> vectors{QStringList{QStringLiteral("a")}};

  const auto frames = DataModel::ScriptFrames::unzipMixedFrames(
    scalars, vectors, DataModel::ScriptFrames::kMaxVectorLength * 10);

  QCOMPARE(frames.size(), DataModel::ScriptFrames::kMaxVectorLength);
}

QTEST_APPLESS_MAIN(TstScriptFrameShaping)

#include "tst_script_frame_shaping.moc"
