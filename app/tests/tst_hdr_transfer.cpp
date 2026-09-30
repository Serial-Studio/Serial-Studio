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

#include <cmath>
#include <QTest>

#include "Misc/HdrTransfer.h"

// The extended sRGB transfer pair behind every HDR shader (spec 0089). The GPU copies in
// app/shaders/*.frag mirror these bodies, so the properties pinned here -- exact continuity at
// 1.0, roundtrip identity across both ranges, monotonicity, and the boost contract mapping
// encoded SDR values to intensity x linear -- are what make "boost k reads as k x SDR white on
// screen" true on both platforms. A break here is a break in every emissive element at once.

class HdrTransferTest : public QObject {
  Q_OBJECT

private slots:
  void roundtripIsIdentity();
  void exactAtAnchorPoints();
  void continuousAtEveryCut();
  void strictlyMonotonic();
  void boostScalesLinearLight();
  void boostNeverDims();
  void negativesClampToZero();
};

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief oetfExt inverts eotfExt (and vice versa) across the SDR range and the >1 extension.
 */
void HdrTransferTest::roundtripIsIdentity()
{
  for (int i = 0; i <= 400; ++i) {
    const float v = static_cast<float>(i) / 100.0f;
    QVERIFY(std::abs(Misc::HdrTransfer::oetfExt(Misc::HdrTransfer::eotfExt(v)) - v) < 1e-5f);
    QVERIFY(std::abs(Misc::HdrTransfer::eotfExt(Misc::HdrTransfer::oetfExt(v)) - v) < 1e-5f);
  }
}

/**
 * @brief The anchors the emissive contract rests on are exact: 0 maps to 0 and 1 maps to 1 in
 *        both directions.
 */
void HdrTransferTest::exactAtAnchorPoints()
{
  QCOMPARE(Misc::HdrTransfer::eotfExt(0.0f), 0.0f);
  QCOMPARE(Misc::HdrTransfer::oetfExt(0.0f), 0.0f);
  QVERIFY(std::abs(Misc::HdrTransfer::eotfExt(1.0f) - 1.0f) < 1e-6f);
  QVERIFY(std::abs(Misc::HdrTransfer::oetfExt(1.0f) - 1.0f) < 1e-6f);
}

/**
 * @brief No jump at the piecewise cuts or at the 1.0 extension seam, in either direction. The
 *        probe eps must stay well under tolerance / slope: oetfExt climbs at ~12.92 near its
 *        linear cut, so a 1e-4 probe reads ~2.6e-3 of pure slope and fails without any seam.
 */
void HdrTransferTest::continuousAtEveryCut()
{
  constexpr float eps    = 1e-5f;
  const float cuts_enc[] = {Misc::HdrTransfer::kSrgbEncodedCut, 1.0f};
  const float cuts_lin[] = {Misc::HdrTransfer::kSrgbLinearCut, 1.0f};

  for (const float cut : cuts_enc) {
    const float below = Misc::HdrTransfer::eotfExt(cut - eps);
    const float above = Misc::HdrTransfer::eotfExt(cut + eps);
    QVERIFY(std::abs(above - below) < 1e-3f);
  }

  for (const float cut : cuts_lin) {
    const float below = Misc::HdrTransfer::oetfExt(cut - eps);
    const float above = Misc::HdrTransfer::oetfExt(cut + eps);
    QVERIFY(std::abs(above - below) < 1e-3f);
  }
}

/**
 * @brief Both directions are strictly increasing over (0, 4] -- a flat or reversed span would
 *        posterize gradients instead of de-banding them.
 */
void HdrTransferTest::strictlyMonotonic()
{
  float last_e = Misc::HdrTransfer::eotfExt(0.0f);
  float last_o = Misc::HdrTransfer::oetfExt(0.0f);
  for (int i = 1; i <= 400; ++i) {
    const float v = static_cast<float>(i) / 100.0f;
    const float e = Misc::HdrTransfer::eotfExt(v);
    const float o = Misc::HdrTransfer::oetfExt(v);
    QVERIFY(e > last_e);
    QVERIFY(o > last_o);
    last_e = e;
    last_o = o;
  }
}

/**
 * @brief boostEncoded(v, k) lands at exactly k x the linear light of v -- the property the
 *        output transform turns into "k x SDR white on screen".
 */
void HdrTransferTest::boostScalesLinearLight()
{
  const float intensities[] = {1.0f, 1.5f, 2.0f, 4.0f};
  for (const float k : intensities) {
    for (int i = 0; i <= 100; ++i) {
      const float v       = static_cast<float>(i) / 100.0f;
      const float boosted = Misc::HdrTransfer::boostEncoded(v, k);
      const float linear  = Misc::HdrTransfer::eotfExt(boosted);
      QVERIFY(std::abs(linear - k * Misc::HdrTransfer::eotfExt(v)) < 1e-4f);
    }
  }
}

/**
 * @brief Intensity below 1 clamps to 1: a boost can never darken the element it wraps.
 */
void HdrTransferTest::boostNeverDims()
{
  for (int i = 0; i <= 100; ++i) {
    const float v = static_cast<float>(i) / 100.0f;
    QVERIFY(std::abs(Misc::HdrTransfer::boostEncoded(v, 0.25f) - v) < 1e-5f);
    QVERIFY(std::abs(Misc::HdrTransfer::boostEncoded(v, 1.0f) - v) < 1e-5f);
  }
}

/**
 * @brief Negative inputs clamp to zero in both directions (premultiplied channels are
 *        non-negative; a negative escaping into pow() would be NaN on the GPU too).
 */
void HdrTransferTest::negativesClampToZero()
{
  QCOMPARE(Misc::HdrTransfer::eotfExt(-0.5f), 0.0f);
  QCOMPARE(Misc::HdrTransfer::oetfExt(-0.5f), 0.0f);
  QCOMPARE(Misc::HdrTransfer::boostEncoded(-0.5f, 2.0f), 0.0f);
}

QTEST_APPLESS_MAIN(HdrTransferTest)

#include "tst_hdr_transfer.moc"
