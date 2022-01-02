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

#include "FrameInfo.h"

/**
 * Creates an empty JFI structure with the current system date/time and frame number @c n
 */
JFI_Object JFI_Empty(const quint64 n)
{
    JFI_Object info;
    info.frameNumber = n;
    info.rxDateTime = QDateTime::currentDateTime();
    return info;
}

/**
 * Creates a new JFI structure with the given information
 *
 * @param n frame number
 * @param t date/time
 * @param d JSON document
 */
JFI_Object JFI_CreateNew(const quint64 n, const QDateTime &t, const QJsonDocument &d)
{
    JFI_Object info;
    info.rxDateTime = t;
    info.frameNumber = n;
    info.jsonDocument = d;
    return info;
}
