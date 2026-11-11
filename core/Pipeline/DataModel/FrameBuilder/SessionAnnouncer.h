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

#pragma once

#include <QSet>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"

namespace DataModel {

/**
 * @brief The three facade hooks a session announcement needs: publish one source's layout, publish
 *        the whole project frame, and forget which layouts already went out. FrameBuilder
 *        implements it; the unit tier substitutes a recording stub.
 */
class SessionAnnouncerHost {
public:
  SessionAnnouncerHost()                                       = default;
  SessionAnnouncerHost(SessionAnnouncerHost&&)                 = delete;
  SessionAnnouncerHost(const SessionAnnouncerHost&)            = delete;
  SessionAnnouncerHost& operator=(SessionAnnouncerHost&&)      = delete;
  SessionAnnouncerHost& operator=(const SessionAnnouncerHost&) = delete;
  virtual ~SessionAnnouncerHost();

  virtual void announceSourceTemplate(const DataModel::Source& src) = 0;
  virtual void announceProjectFrame()                               = 0;
  virtual void forgetPublishedStructures()                          = 0;
};

/**
 * @brief Decides which layouts a session announces and which frame may stand for it (spec 0119).
 *        Reads only the bound project and stream-source set, so a test drives the real decisions
 *        without a pipeline. Builder thread only; the facade owns the thread marshal.
 */
class SessionAnnouncer {
public:
  SessionAnnouncer(SessionAnnouncerHost& host,
                   const DataModel::Frame& project,
                   const QSet<int>& streamSourceIds);

  SessionAnnouncer(SessionAnnouncer&&)                 = delete;
  SessionAnnouncer(const SessionAnnouncer&)            = delete;
  SessionAnnouncer& operator=(SessionAnnouncer&&)      = delete;
  SessionAnnouncer& operator=(const SessionAnnouncer&) = delete;

  void announceConnect();
  void announceResync(bool connected, SerialStudio::OperationMode mode);
  void announceSource(int sourceId);

  [[nodiscard]] bool publishesOwnFrame(int sourceId) const noexcept;
  [[nodiscard]] bool combinedFrameStandsIn(bool anyOwnFrame) const noexcept;

private:
  [[nodiscard]] bool isMultiSource() const noexcept;
  [[nodiscard]] bool hasLayout() const noexcept;
  [[nodiscard]] bool isImageOnly() const;
  void announceIfGrouped(const DataModel::Source& src);

private:
  SessionAnnouncerHost& m_host;
  const DataModel::Frame& m_project;
  const QSet<int>& m_streamSourceIds;
};

}  // namespace DataModel
