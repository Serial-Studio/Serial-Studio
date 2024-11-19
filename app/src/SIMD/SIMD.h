/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <x86/sse2.h>

#include <QVector>
#include <QPointF>

namespace SIMD
{
/**
 * @brief Initializes an array with a specific value using SIMD for bulk
 *        operations.
 *
 * This function uses SIMD instructions to fill an array with a
 * predetermined value. For arrays larger than the SIMD width, it processes
 * elements in chunks.
 *
 * Remaining elements that do not fit in the SIMD width are processed using
 * a scalar fallback loop.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @param value The value to initialize all elements in the array.
 */
template<typename T>
inline void fill(T *data, size_t count, T value)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(T);

  // SIMD bulk initialization
  size_t i = 0;
  simde__m128d fillValue = simde_mm_set1_pd(value);
  for (; i + simdWidth <= count; i += simdWidth)
    simde_mm_storeu_pd(data + i, fillValue);

  // Handle remaining elements using a scalar loop
  for (; i < count; ++i)
    data[i] = value;
}

/**
 * @brief Shifts elements in an array to the left and appends a new value.
 *
 * This function uses SIMD instructions to efficiently shift elements in an
 * array to the left by one position. For arrays larger than the SIMD width, it
 * processes elements in chunks.
 *
 * Remaining elements that do not fit in the SIMD width are processed using a
 * scalar fallback loop.
 *
 * @param data Pointer to the array of values.
 * @param count The total number of elements in the array.
 * @param newValue The value to set at the last position after the shift.
 */
template<typename T>
inline void shift(T *data, size_t count, T value)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(T);

  // Shift elements using SIMD operations
  size_t i = 0;
  for (; i + simdWidth <= count; i += simdWidth)
  {
    simde__m128d next = simde_mm_loadu_pd(data + i + 1);
    simde_mm_storeu_pd(data + i, next);
  }

  // Handle remaining elements using a scalar loop
  for (; i < count - 1; ++i)
    data[i] = data[i + 1];

  // Set the last value of the array
  data[count - 1] = value;
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
template<typename T>
inline T findMin(const T *data, size_t count)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(T);

  // SIMD comparisons
  size_t i = 0;
  simde__m128d minVec = simde_mm_set1_pd(data[0]);
  for (; i + simdWidth <= count; i += simdWidth)
  {
    simde__m128d values = simde_mm_loadu_pd(&data[i]);
    minVec = simde_mm_min_pd(minVec, values);
  }

  // Reduce SIMD register to scalar
  T minVal = data[0];
  alignas(simdWidth) T buffer[simdWidth];
  simde_mm_storeu_pd(buffer, minVec);
  for (int j = 0; j < simdWidth; ++j)
    minVal = std::min<T>(minVal, buffer[j]);

  // Scalar fallback for remaining elements
  for (; i < count; ++i)
    minVal = std::min<T>(minVal, data[i]);

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
template<typename T>
inline T findMax(const T *data, size_t count)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(T);

  // SIMD comparisons
  size_t i = 0;
  simde__m128d maxVec = simde_mm_set1_pd(data[0]);
  for (; i + simdWidth <= count; i += simdWidth)
  {
    simde__m128d values = simde_mm_loadu_pd(&data[i]);
    maxVec = simde_mm_max_pd(maxVec, values);
  }

  // Reduce SIMD register to scalar
  T maxVal = data[0];
  alignas(simdWidth) T buffer[simdWidth];
  simde_mm_storeu_pd(buffer, maxVec);
  for (int j = 0; j < simdWidth; ++j)
    maxVal = std::max<T>(maxVal, buffer[j]);

  // Scalar fallback for remaining elements
  for (; i < count; ++i)
    maxVal = std::max<T>(maxVal, data[i]);

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
 *                  (e.g., `[](const QPointF& p) { return p.y(); }`).
 *
 * @return The minimum value in the QVector<QPointF> based on the extracted
 *         values.
 */
template<typename Extractor>
inline qreal findMin(const QVector<QPointF> &data, Extractor extractor)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(qreal);

  // SIMD comparisons
  size_t i = 0;
  size_t count = data.size();
  auto minVec = simde_mm_set1_pd(extractor(data[0]));
  for (; i + simdWidth <= count; i += simdWidth)
  {
    auto values = simde_mm_set_pd(extractor(data[i + 1]), extractor(data[i]));
    minVec = simde_mm_min_pd(minVec, values);
  }

  // Reduce SIMD register to scalar
  qreal minVal = extractor(data[0]);
  alignas(16) qreal buffer[simdWidth];
  simde_mm_storeu_pd(buffer, minVec);
  for (int j = 0; j < simdWidth; ++j)
    minVal = std::min<qreal>(minVal, buffer[j]);

  // Scalar fallback for remaining elements
  for (; i < count; ++i)
    minVal = std::min<qreal>(minVal, extractor(data[i]));

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
 *                  (e.g., `[](const QPointF& p) { return p.y(); }`).
 *
 * @return The maximum value in the QVector<QPointF> based on the extracted
 *         values.
 */
template<typename Extractor>
inline qreal findMax(const QVector<QPointF> &data, Extractor extractor)
{
  // Get the number of items that can be processed in a SIMD register
  constexpr int simdWidth = sizeof(simde__m128d) / sizeof(qreal);

  // SIMD comparisons
  size_t i = 0;
  size_t count = data.size();
  auto maxVec = simde_mm_set1_pd(extractor(data[0]));
  for (; i + simdWidth <= count; i += simdWidth)
  {
    auto values = simde_mm_set_pd(extractor(data[i + 1]), extractor(data[i]));
    maxVec = simde_mm_max_pd(maxVec, values);
  }

  // Reduce SIMD register to scalar
  qreal maxVal = extractor(data[0]);
  alignas(16) qreal buffer[simdWidth];
  simde_mm_storeu_pd(buffer, maxVec);
  for (int j = 0; j < simdWidth; ++j)
    maxVal = std::max<qreal>(maxVal, buffer[j]);

  // Scalar fallback for remaining elements
  for (; i < count; ++i)
    maxVal = std::max<qreal>(maxVal, extractor(data[i]));

  return maxVal;
}

}; // namespace SIMD
