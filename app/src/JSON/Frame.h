/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#include <QVector>
#include <QObject>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>

#include "JSON/Group.h"
#include "JSON/Action.h"

namespace JSON
{
/**
 * @brief The Frame class
 *
 * The frame class represents a complete frame, including the groups & datasets
 * that compose it. This class allows Serial Studio to build the user interface
 * dinamically from the received data.
 *
 * The process of building a frame and representing it in Serial Studio is:
 * 1) Physical device sends data
 * 2) I/O driver receives data
 * 3) I/O manager processes the data and separates the frames
 * 4) JSON generator creates a JSON file with the data contained in each frame.
 * 5) UI dashboard class receives the JSON file.
 * 6) TimerEvents class notifies the UI dashboard that the user interface should
 *    be re-generated.
 * 7) UI dashboard feeds JSON data to this class.
 * 8) This class creates a model of the JSON data with the values of the latest
 *    frame.
 * 9) UI dashboard updates the widgets with the C++ model provided by this
 * class.
 */
class FrameBuilder;
class Frame
{
public:
  ~Frame();

  void clear();
  [[nodiscard]] bool isValid() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

  [[nodiscard]] int groupCount() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &frameEnd() const;
  [[nodiscard]] const QString &frameStart() const;

  [[nodiscard]] const QVector<Group> &groups() const;
  [[nodiscard]] const QVector<Action> &actions() const;

private:
  QString m_title;
  QString m_frameEnd;
  QString m_frameStart;

  QVector<Group> m_groups;
  QVector<Action> m_actions;

  friend class JSON::FrameBuilder;
};
} // namespace JSON
