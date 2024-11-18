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

#include <mutex>
#include <vector>
#include <cstring>

#include <QByteArray>

namespace IO
{
class CircularBuffer
{
public:
  explicit CircularBuffer(qsizetype capacity = 1024 * 1024);

  void clear();
  void append(const QByteArray &data);

  [[nodiscard]] qsizetype size() const;
  [[nodiscard]] qsizetype freeSpace() const;
  [[nodiscard]] QByteArray read(qsizetype size);
  [[nodiscard]] QByteArray peek(qsizetype size) const;
  [[nodiscard]] int findPatternKMP(const QByteArray &pattern);

private:
  [[nodiscard]] std::vector<int> computeKMPTable(const QByteArray &p) const;

private:
  qsizetype m_size;
  qsizetype m_head;
  qsizetype m_tail;
  qsizetype m_capacity;
  std::vector<char> m_buffer;
  mutable std::mutex m_mutex;
};
} // namespace IO
