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

#ifndef JSON_FRAME_INFO_H
#define JSON_FRAME_INFO_H

#include <QList>
#include <QDateTime>
#include <QJsonDocument>

/**
 * Defines a JSON frame information structure. We need to use this in order to be able
 * to correctly process all received frames in a thread-worker manner.
 *
 * Every time we receive a frame from the serial/network device, we generate the JSON
 * in another thread in order to avoid putting excessive pressure in the GUI thread.
 *
 * This results in a perceived increase of application performance. However, since each
 * JSON frame is generated in a different worker thread, the main thread may not receive
 * JSON data in the correct order.
 *
 * To mitigate this, we create this structure, which contains the following information:
 *    - Frame number (assigned by the JSON::Generator class when raw frame data is
 *      received, and before the JSON object is genereated/parsed).
 *    - RX date/time (assigned in the same manner as the frame number).
 *    - JSON document/object (which contains all the frame information + what we should
 *      do with it).
 *
 * We need to register the frame number because (in some cases), the RX date/time will be
 * the same between two or more frames (e.g. if you have a very high baud rate, there is
 * a chance that frames will be registered with the same rx date/time down to the
 * millisecond, this was the root cause of bug #35).
 *
 * To mitigate this, we simply increment the frame number each time that we receive a raw
 * frame. Frame numbers are registered as a uint64_t for this very reason (it would take
 * about 585 years to overflow this variable, if you had a computer that was able to
 * process a frame every nanosecond).
 *
 * Frame number is reset when the device connection state is changed.
 */
typedef struct
{
    quint64 frameNumber;
    QDateTime rxDateTime;
    QJsonDocument jsonDocument;
} JFI_Object;

//----------------------------------------------------------------------------------------
// Convenience functions
//----------------------------------------------------------------------------------------

extern bool JFI_Valid(const JFI_Object &info);
extern void JFI_SortList(QList<JFI_Object> *list);

extern JFI_Object JFI_Empty(const quint64 n = 0);
extern JFI_Object JFI_CreateNew(const quint64 n, const QDateTime &t,
                                const QJsonDocument &d);

/*
 * Important magic to be able to use JFI structures in Qt signals/slots
 */
Q_DECLARE_METATYPE(JFI_Object)

#endif
