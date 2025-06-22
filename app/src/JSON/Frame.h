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
  Frame();
  ~Frame();

  void clear();
  void buildUniqueIds();

  [[nodiscard]] bool isValid() const;
  [[nodiscard]] bool equalsStructure(const JSON::Frame &other) const;

  [[nodiscard]] QJsonObject serialize() const;
  [[nodiscard]] bool read(const QJsonObject &object);

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &checksum() const;
  [[nodiscard]] const QByteArray &frameEnd() const;
  [[nodiscard]] const QByteArray &frameStart() const;

  [[nodiscard]] const QVector<Group> &groups() const;
  [[nodiscard]] const QVector<Action> &actions() const;

private:
  QString m_title;
  QString m_checksum;
  QByteArray m_frameEnd;
  QByteArray m_frameStart;

  QVector<Group> m_groups;
  QVector<Action> m_actions;

  bool m_containsCommercialFeatures;

  friend class UI::Dashboard;
  friend class JSON::FrameBuilder;
};
} // namespace JSON
