/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#include "Checksum.h"

quint8 IO::crc8(const QByteArray &data)
{
    quint8 crc = 0xff;
    for (auto i = 0; i < data.length(); i++)
    {
        crc ^= data.at(i);
        for (auto j = 0; j < 8; j++)
        {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }

    return crc;
}

quint16 IO::crc16(const QByteArray &data)
{
    quint8 x;
    quint16 crc = 0xFFFF;

    for (auto i = 0; i < data.length(); ++i)
    {
        x = crc >> 8 ^ data.at(i);
        x ^= x >> 4;
        crc = (crc << 8) ^ ((quint16)(x << 12)) ^ ((quint16)(x << 5)) ^ ((quint16)x);
    }

    return crc;
}

quint32 IO::crc32(const QByteArray &data)
{
    quint32 mask;
    quint32 crc = 0xFFFFFFFF;
    for (auto i = 0; i < data.length(); ++i)
    {
        crc = crc ^ data.at(i);
        for (auto j = 8; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~crc;
}
