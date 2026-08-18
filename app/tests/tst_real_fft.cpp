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

#include <kiss_fft.h>
#include <kiss_fftr.h>

#include <algorithm>
#include <cmath>
#include <QTest>
#include <vector>

/**
 * @file tst_real_fft.cpp
 * @brief Pins the one assumption FFTPlot and Waterfall inherited when they moved from kiss_fft
 *        to kiss_fftr: that the real transform returns the same bins 0..N/2 as a complex
 *        transform of the same signal with a zero imaginary column.
 *
 * The two take different arithmetic routes -- kiss_fftr runs an N/2 complex pass plus a twiddle
 * recombination -- so the results are close, never bit-identical, and this suite is the one place
 * in the tree that compares against a tolerance instead of a bit pattern. The bound is relative to
 * the spectrum's own peak magnitude, which is what makes it meaningful for a display in dB: an
 * error far below the peak cannot move a rendered bin.
 */

namespace {

constexpr int kSizes[]        = {8, 16, 64, 256, 1024, 4096};
constexpr double kTwoPi       = 6.283185307179586;
constexpr float kRelTolerance = 1e-5f;

/**
 * @brief Deterministic real test signal: a few incommensurate tones plus a DC term and a
 *        reproducible pseudo-random dither, so no bin is trivially zero and leakage is present.
 */
[[nodiscard]] std::vector<float> makeSignal(int n)
{
  std::vector<float> out(static_cast<std::size_t>(n), 0.0f);

  quint32 state = 0x12345678u;
  for (int i = 0; i < n; ++i) {
    state          = state * 1664525u + 1013904223u;
    const double t = static_cast<double>(i);
    const double v = 1.75 + 3.0 * std::sin(kTwoPi * t * 5.0 / n)
                   + 0.5 * std::cos(kTwoPi * t * 17.0 / n) + 0.25 * std::sin(kTwoPi * t * 0.37)
                   + 1e-3 * static_cast<double>(state >> 16) / 65536.0;
    out[static_cast<std::size_t>(i)] = static_cast<float>(v);
  }

  return out;
}

}  // namespace

/**
 * @brief Equivalence of the real and complex KissFFT transforms over the bins both widgets read.
 */
class TstRealFft : public QObject {
  Q_OBJECT

private slots:
  void realMatchesComplex_data();
  void realMatchesComplex();
  void oddSizeIsRejected();
};

void TstRealFft::realMatchesComplex_data()
{
  QTest::addColumn<int>("size");

  for (const int size : kSizes)
    QTest::addRow("n=%d", size) << size;
}

void TstRealFft::realMatchesComplex()
{
  QFETCH(int, size);

  const auto signal = makeSignal(size);
  const auto n      = static_cast<std::size_t>(size);
  const int bins    = size / 2 + 1;

  std::vector<kiss_fft_cpx> complexIn(n);
  for (std::size_t i = 0; i < n; ++i) {
    complexIn[i].r = signal[i];
    complexIn[i].i = 0.0f;
  }

  std::vector<kiss_fft_cpx> complexOut(n);
  kiss_fft_cfg complexPlan = kiss_fft_alloc(size, 0, nullptr, nullptr);
  QVERIFY(complexPlan != nullptr);
  kiss_fft(complexPlan, complexIn.data(), complexOut.data());
  kiss_fft_free(complexPlan);

  std::vector<kiss_fft_cpx> realOut(static_cast<std::size_t>(bins));
  kiss_fftr_cfg realPlan = kiss_fftr_alloc(size, 0, nullptr, nullptr);
  QVERIFY(realPlan != nullptr);
  kiss_fftr(realPlan, signal.data(), realOut.data());
  kiss_fftr_free(realPlan);

  float peak = 0.0f;
  for (int k = 0; k < bins; ++k) {
    const float re = complexOut[static_cast<std::size_t>(k)].r;
    const float im = complexOut[static_cast<std::size_t>(k)].i;
    peak           = std::max(peak, std::sqrt(re * re + im * im));
  }

  QVERIFY(peak > 0.0f);

  for (int k = 0; k < bins; ++k) {
    const auto idx  = static_cast<std::size_t>(k);
    const float dr  = realOut[idx].r - complexOut[idx].r;
    const float di  = realOut[idx].i - complexOut[idx].i;
    const float err = std::sqrt(dr * dr + di * di) / peak;
    if (err > kRelTolerance)
      QFAIL(QByteArray("bin ") + QByteArray::number(k) + " of " + QByteArray::number(size)
            + " diverged by " + QByteArray::number(static_cast<double>(err)) + " of peak");
  }
}

/**
 * @brief kiss_fftr refuses an odd transform size, which is what both widgets rely on to fall
 *        back to a null plan instead of transforming garbage.
 */
void TstRealFft::oddSizeIsRejected()
{
  QVERIFY(kiss_fftr_alloc(15, 0, nullptr, nullptr) == nullptr);
}

QTEST_APPLESS_MAIN(TstRealFft)

#include "tst_real_fft.moc"
