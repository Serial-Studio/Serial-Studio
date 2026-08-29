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

#include "Console/TextFormat.h"

#include <array>
#include <QStringView>

#include "DSPSimd.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Incoming text
//--------------------------------------------------------------------------------------------------

/**
 * @brief Normalizes @p text's line endings to LF and prefixes @p timestamp to every line that
 *        starts here and carries something other than whitespace, advancing @p state across the
 *        chunk boundary. An empty @p timestamp stamps nothing.
 */
QString Console::TextFormat::formatIncoming(const QString& text,
                                            LineState& state,
                                            const QString& timestamp)
{
  auto data = text;
  if (state.lastCharWasCR && data.startsWith('\n'))
    data.removeFirst();

  state.lastCharWasCR = data.endsWith('\r');
  data                = data.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  data                = data.replace(QStringLiteral("\r"), QStringLiteral("\n"));

  QString processedString;
  processedString.reserve(data.length() + timestamp.length() * 4);

  int pos = 0;
  while (pos < data.length()) {
    const int nlPos = data.indexOf('\n', pos);
    const int end   = (nlPos < 0) ? data.length() : nlPos;

    if (end > pos) {
      const auto segment = QStringView(data).mid(pos, end - pos);
      if (state.isStartingLine && !segment.trimmed().isEmpty())
        processedString.append(timestamp);

      processedString.append(segment);
      state.isStartingLine = false;
    }

    if (nlPos >= 0) {
      processedString.append('\n');
      state.isStartingLine = true;
      pos                  = nlPos + 1;
    }

    else
      pos = end;
  }

  return processedString;
}

/**
 * @brief Replaces every non-printable character with '.', keeping CR, LF, TAB and ESC. Used
 *        when VT-100 emulation is off and the terminal renders the bytes literally.
 */
QString Console::TextFormat::filterControlChars(const QString& text)
{
  QString filteredData;
  filteredData.reserve(text.size());

  int i = 0;
  while (i < text.size()) {
    const int runStart = i;
    while (i < text.size()) {
      const ushort unicode = text[i].unicode();

      // clang-format off
      const bool printable = (unicode != '\0')
                             && ((unicode >= 0x20 && unicode < 0x7F)
                                 || (unicode >= 0x80)
                                 || (unicode == '\r')
                                 || (unicode == '\n')
                                 || (unicode == '\t')
                                 || (unicode == 0x1B));
      // clang-format on

      if (!printable)
        break;

      ++i;
    }

    if (i > runStart)
      filteredData.append(QStringView(text).mid(runStart, i - runStart));

    if (i < text.size()) {
      filteredData.append('.');
      ++i;
    }
  }

  return filteredData;
}

//--------------------------------------------------------------------------------------------------
// Hex dump
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes one hex-dump row's 16-char ASCII column at @p out: the SIMD kernel maps
 *        full rows, a scalar tail blank-pads and dot-maps the final partial row.
 */
static void hexDumpAsciiColumn(QByteArrayView data, int i, char16_t* out)
{
  SS_ASSERT(out != nullptr, return);
  SS_ASSERT(i >= 0 && i < data.length(), return);

  if (i + 16 <= data.length()) {
    DSP::simdAsciiDots16(reinterpret_cast<const quint8*>(data.data() + i), out);
    return;
  }

  for (int j = 0; j < 16; ++j) {
    if (i + j >= data.length()) {
      out[j] = u' ';
      continue;
    }

    const auto b = static_cast<unsigned char>(data[i + j]);
    out[j]       = (b >= 0x20 && b <= 0x7E) ? static_cast<char16_t>(b) : u'.';
  }
}

/**
 * @brief Renders @p data as a classic hex dump: each row is built in a fixed char16_t scratch
 *        buffer (scalar nibble writes + the shared DSP::simdAsciiDots16 ASCII column) and
 *        appended with one QString::append call, replacing the per-character appends that
 *        dominated GUI time at MB/s rates with the hex view enabled.
 */
QString Console::TextFormat::hexDump(QByteArrayView data)
{
  static_assert(sizeof(QChar) == sizeof(char16_t), "QChar must be UTF-16 code-unit sized");

  static constexpr char kHexDigits[] = "0123456789abcdef";
  constexpr auto rowSize             = 16;
  constexpr auto rowChars            = 80;
  constexpr auto fullRowLen          = 79;
  static_assert(fullRowLen <= rowChars, "hex-dump row must fit the scratch buffer");

  QString out;
  const auto rows = (data.length() + rowSize - 1) / rowSize;
  out.reserve(rows * rowChars + 2);

  std::array<char16_t, rowChars> scratch;

  for (int i = 0; i < data.length(); i += rowSize) {
    int rowLen = 0;

    for (int shift = 20; shift >= 0; shift -= 4)
      scratch[rowLen++] = static_cast<char16_t>(kHexDigits[(i >> shift) & 0xF]);

    scratch[rowLen++] = u' ';
    scratch[rowLen++] = u'|';
    scratch[rowLen++] = u' ';

    for (int j = 0; j < rowSize; ++j) {
      if (i + j < data.length()) {
        const auto b      = static_cast<unsigned char>(data[i + j]);
        scratch[rowLen++] = static_cast<char16_t>(kHexDigits[b >> 4]);
        scratch[rowLen++] = static_cast<char16_t>(kHexDigits[b & 0xF]);
        scratch[rowLen++] = u' ';
      }

      else {
        scratch[rowLen++] = u' ';
        scratch[rowLen++] = u' ';
        scratch[rowLen++] = u' ';
      }

      if ((j + 1) == 8)
        scratch[rowLen++] = u' ';
    }

    scratch[rowLen++] = u'|';
    scratch[rowLen++] = u' ';

    hexDumpAsciiColumn(data, i, scratch.data() + rowLen);
    rowLen += rowSize;

    scratch[rowLen++] = u' ';
    scratch[rowLen++] = u'|';
    scratch[rowLen++] = u'\n';

    SS_ASSERT_LOG(rowLen == fullRowLen);
    if (rowLen != fullRowLen)
      break;

    out.append(reinterpret_cast<const QChar*>(scratch.data()), rowLen);
  }

  out += QLatin1Char('\n');
  return out;
}
