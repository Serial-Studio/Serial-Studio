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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#if !defined(__SSE2__)
#  define SIMDE_ENABLE_NATIVE_ALIASES
#endif

#include <cstddef>
#include <algorithm>

#include <QVector>
#include <QPointF>

#ifdef _WIN32
#  include <cmath>
#endif

#include <x86/sse2.h>

namespace SIMD
{
/**
 * @brief Initializes an array with a specific value using SIMD for bulk
 *        operations.
 *
 * This function uses SIMD instructions to fill an array with a predetermined
 * value.
 *
 * For arrays larger than the SIMD width, it processes elements in chunks.
 *
 * Remaining elements that do not fit in the SIMD width are processed using
 * a scalar fallback loop.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @param value The value to initialize all elements in the array.
 */
inline void fill(double *data, size_t count, const double value)
{
  size_t i = 0;
  auto fillValue = simde_mm_set1_pd(value);
  constexpr size_t simdWidth = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWidth <= count; i += simdWidth)
    simde_mm_storeu_pd(data + i, fillValue);

  for (; i < count; ++i)
    data[i] = value;
}

/**
 * @brief Adds a range of values from begin to end into an array using SIMD for
 * bulk operations.
 *
 * This function uses SIMD instructions to fill an array with a sequence of
 * values starting from a specified begin value and incrementing by 1 for each
 * element.
 *
 * For arrays larger than the SIMD width, it processes elements in chunks.
 * Remaining elements that do not fit in the SIMD width are processed using
 * a scalar fallback loop.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @param begin The starting value for the range.
 */
inline void fill_range(double *data, size_t count, const double begin)
{
  size_t i = 0;
  constexpr size_t simdWidth = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWidth <= count; i += simdWidth)
  {
    auto range = simde_mm_set_pd(begin + i + 1, begin + i);
    simde_mm_storeu_pd(data + i, range);
  }

  for (; i < count; ++i)
    data[i] = begin + i;
}

/**
 * @brief Shifts elements in an array to the left and appends a new value.
 *
 * This function uses SIMD instructions to efficiently shift elements in an
 * array to the left by one position.
 *
 * For arrays larger than the SIMD width, it processes elements in chunks.
 *
 * Remaining elements that do not fit in the SIMD width are processed using a
 * scalar fallback loop.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @param newValue The value to set at the last position after the shift.
 */
inline void shift(double *data, size_t count, const double newValue)
{
  size_t i = 0;
  constexpr size_t simdWidth = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWidth + 1 <= count; i += simdWidth)
  {
    auto n = simde_mm_loadu_pd(data + i + 1);
    simde_mm_storeu_pd(data + i, n);
  }

  for (; i < count - 1; ++i)
    data[i] = data[i + 1];

  data[count - 1] = newValue;
}

/**
 * @brief Finds the minimum value in an array using SIMD for parallel
 * comparisons.
 *
 * This function uses SIMD instructions to find the minimum value in an array.
 * It processes the array in chunks of SIMD width and reduces the results to
 * find the overall minimum.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @return The minimum value in the array.
 */
inline double findMin(const double *data, size_t count)
{
  size_t i = 0;
  auto minVec = simde_mm_set1_pd(data[0]);
  constexpr auto simdWidth = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWidth <= count; i += simdWidth)
  {
    auto values = simde_mm_loadu_pd(&data[i]);
    minVec = simde_mm_min_pd(minVec, values);
  }

  double minVal = data[0];
  std::vector<double> buffer(simdWidth);
  simde_mm_storeu_pd(buffer.data(), minVec);
  for (size_t j = 0; j < simdWidth; ++j)
    minVal = std::min<double>(minVal, buffer[j]);

  for (; i < count; ++i)
    minVal = std::min<double>(minVal, data[i]);

  return minVal;
}

/**
 * @brief Finds the maximum value in an array using SIMD for parallel
 * comparisons.
 *
 * This function uses SIMD instructions to find the maximum value in an array.
 * It processes the array in chunks of SIMD width and reduces the results to
 * find the overall maximum.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @return The maximum value in the array.
 */
inline double findMax(const double *data, size_t count)
{
  size_t i = 0;
  auto maxVec = simde_mm_set1_pd(data[0]);
  constexpr auto simdWidth = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWidth <= count; i += simdWidth)
  {
    auto values = simde_mm_loadu_pd(&data[i]);
    maxVec = simde_mm_max_pd(maxVec, values);
  }

  double maxVal = data[0];
  std::vector<double> buffer(simdWidth);
  simde_mm_storeu_pd(buffer.data(), maxVec);
  for (size_t j = 0; j < simdWidth; ++j)
    maxVal = std::max<double>(maxVal, buffer[j]);

  for (; i < count; ++i)
    maxVal = std::max<double>(maxVal, data[i]);

  return maxVal;
}

/**
 * @brief Finds the minimum value in a QVector<QPointF> using SIMD operations.
 *
 * This function uses SIMD instructions to efficiently find the minimum value
 * in a QVector<QPointF> by comparing elements in chunks of SIMD width. The
 * specific value to compare (e.g., x or y coordinate) is determined by the
 * provided extractor function.
 *
 * @tparam Extractor A callable object (e.g., lambda, function pointer) that
 *                   extracts the value to compare from a QPointF.
 *
 * @param data The QVector<QPointF> containing the points to search.
 * @param extractor A callable that extracts the desired value from a QPointF
 *                  (e.g., [](const QPointF& p) { return p.y(); }).
 *
 * @return The minimum value in the QVector<QPointF> based on the extracted
 *         values.
 */
template<typename Extractor>
inline double findMin(const QVector<QPointF> &data, Extractor extractor)
{
  size_t i = 0;
  size_t count = data.size();
  if (count == 0)
    return 0;

  auto minVec = simde_mm_set1_pd(extractor(data[0]));
  constexpr auto simdWith = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWith <= count; i += simdWith)
  {
    auto values = simde_mm_set_pd(extractor(data[i + 1]), extractor(data[i]));
    minVec = simde_mm_min_pd(minVec, values);
  }

  alignas(16) double buffer[simdWith];
  simde_mm_storeu_pd(buffer, minVec);
  double minVal = buffer[0];
  for (size_t j = 1; j < simdWith; ++j)
    minVal = std::min<double>(minVal, buffer[j]);

  for (; i < count; ++i)
    minVal = std::min<double>(minVal, extractor(data[i]));

  return minVal;
}

/**
 * @brief Finds the maximum value in a QVector<QPointF> using SIMD operations.
 *
 * This function uses SIMD instructions to efficiently find the maximum value
 * in a QVector<QPointF> by comparing elements in chunks of SIMD width. The
 * specific value to compare (e.g., x or y coordinate) is determined by the
 * provided extractor function.
 *
 * @tparam Extractor A callable object (e.g., lambda, function pointer) that
 *                   extracts the value to compare from a QPointF.
 *
 * @param data The QVector<QPointF> containing the points to search.
 * @param extractor A callable that extracts the desired value from a QPointF
 *                  (e.g., [](const QPointF& p) { return p.y(); }).
 *
 * @return The maximum value in the QVector<QPointF> based on the extracted
 *         values.
 */
template<typename Extractor>
inline double findMax(const QVector<QPointF> &data, Extractor extractor)
{
  size_t i = 0;
  size_t count = data.size();
  if (count == 0)
    return 0;

  auto maxVec = simde_mm_set1_pd(extractor(data[0]));
  constexpr auto simdWith = sizeof(simde__m128d) / sizeof(double);
  for (; i + simdWith <= count; i += simdWith)
  {
    auto values = simde_mm_set_pd(extractor(data[i + 1]), extractor(data[i]));
    maxVec = simde_mm_max_pd(maxVec, values);
  }

  alignas(16) double buffer[simdWith];
  simde_mm_storeu_pd(buffer, maxVec);
  double maxVal = buffer[0];
  for (size_t j = 1; j < simdWith; ++j)
    maxVal = std::max<double>(maxVal, buffer[j]);

  for (; i < count; ++i)
    maxVal = std::max<double>(maxVal, extractor(data[i]));

  return maxVal;
}
}; // namespace SIMD
