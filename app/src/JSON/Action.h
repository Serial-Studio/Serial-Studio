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

#include <QObject>
#include <QVariant>
#include <QJsonObject>

namespace JSON
{
class ProjectModel;
}

namespace JSON
{
/**
 * @class JSON::Action
 * @brief A class that represents an action which can send data to connected
 *        devices.
 *
 * The JSON::Action class allows users to create an action with an ID, and
 * manage associated metadata such as an icon, title, transmission data
 * (txData), and an end-of-line (eol) sequence. It also provides functionality
 * to serialize and deserialize the action to and from a QJsonObject, making it
 * suitable for JSON-based communication or storage.
 */
class Action
{
public:
  Action(const int actionId = -1);

  [[nodiscard]] int actionId() const;
  [[nodiscard]] const QString &icon() const;
  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &txData() const;
  [[nodiscard]] const QString &eolSequence() const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

private:
  int m_actionId;
  QString m_icon;
  QString m_title;
  QString m_txData;
  QString m_eolSequence;

  friend class JSON::ProjectModel;
};
} // namespace JSON
