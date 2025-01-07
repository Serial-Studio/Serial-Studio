/*
 * Copyright (c) 2014, Zac Bergquist
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <cctype>
#include <iomanip>
#include <ostream>

template<unsigned RowSize, bool ShowAscii>
struct CustomHexdump
{
  CustomHexdump(const void *data, unsigned length)
    : mData(static_cast<const unsigned char *>(data))
    , mLength(length)
  {
  }
  const unsigned char *mData;
  const unsigned mLength;
};

template<unsigned RowSize, bool ShowAscii>
std::ostream &operator<<(std::ostream &out,
                         const CustomHexdump<RowSize, ShowAscii> &dump)
{
  out.fill('0');
  for (int i = 0; i < dump.mLength; i += RowSize)
  {
    out << "0x" << std::setw(6) << std::hex << i << ": ";
    for (int j = 0; j < RowSize; ++j)
    {
      if (i + j < dump.mLength)
      {
        out << std::hex << std::setw(2) << static_cast<int>(dump.mData[i + j])
            << " ";
      }

      else
        out << "   ";
    }

    out << " ";
    if (ShowAscii)
    {
      for (int j = 0; j < RowSize; ++j)
      {
        if (i + j < dump.mLength)
        {
          if (std::isprint(dump.mData[i + j]))
            out << static_cast<char>(dump.mData[i + j]);
          else
            out << ".";
        }
      }
    }

    out << std::endl;
  }

  return out;
}

typedef CustomHexdump<16, true> Hexdump;
