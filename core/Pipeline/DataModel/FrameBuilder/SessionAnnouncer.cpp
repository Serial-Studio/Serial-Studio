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

#include "DataModel/FrameBuilder/SessionAnnouncer.h"

#include <algorithm>
#include <utility>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Out-of-line key function: pins the host interface's vtable to this translation unit.
 */
DataModel::SessionAnnouncerHost::~SessionAnnouncerHost() = default;

/**
 * @brief Binds the host and the project state. Reads neither: the owner is still under
 *        construction when this runs.
 */
DataModel::SessionAnnouncer::SessionAnnouncer(SessionAnnouncerHost& host,
                                              const DataModel::Frame& project,
                                              const QSet<int>& streamSourceIds)
  : m_host(host), m_project(project), m_streamSourceIds(streamSourceIds)
{}

//--------------------------------------------------------------------------------------------------
// Announcements
//--------------------------------------------------------------------------------------------------

/**
 * @brief Announces the session's layouts as the connect edge does: one per source in a multi-source
 *        project, the whole frame for an image-only single source. Any other single-source layout
 *        goes out with its first block.
 */
void DataModel::SessionAnnouncer::announceConnect()
{
  SS_ASSERT_LOG(!m_project.title.isEmpty());

  if (isMultiSource()) {
    for (const auto& src : m_project.sources)
      announceIfGrouped(src);

    return;
  }

  if (isImageOnly())
    m_host.announceProjectFrame();
}

/**
 * @brief Announces every layout again after a live project re-sync: forgets the marks first, so a
 *        layout that already went out is sent again, then the connect set and every stream source.
 *        Does nothing, forgetting included, outside a connected project session.
 */
void DataModel::SessionAnnouncer::announceResync(bool connected, SerialStudio::OperationMode mode)
{
  SS_ASSERT(mode >= SerialStudio::ProjectFile && mode <= SerialStudio::QuickPlot, return);

  if (!connected || mode != SerialStudio::ProjectFile || !hasLayout())
    return;

  m_host.forgetPublishedStructures();
  announceConnect();
  for (const int sourceId : std::as_const(m_streamSourceIds))
    announceSource(sourceId);
}

/**
 * @brief Announces one source's layout by id; an unknown id or a group-less source announces
 *        nothing.
 */
void DataModel::SessionAnnouncer::announceSource(int sourceId)
{
  SS_ASSERT(sourceId >= 0, return);

  for (const auto& src : m_project.sources)
    if (src.sourceId == sourceId) {
      announceIfGrouped(src);
      return;
    }
}

//--------------------------------------------------------------------------------------------------
// Decisions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p sourceId publishes its own frame on a synthetic refresh. A stream source's
 *        values come from the dashboard's stream ingest, so publishing would double-push its rings.
 */
bool DataModel::SessionAnnouncer::publishesOwnFrame(int sourceId) const noexcept
{
  return !m_streamSourceIds.contains(sourceId);
}

/**
 * @brief Whether the combined frame may stand in on a synthetic refresh. Only for a single-source
 *        project: in a multi-source one it would claim every layout as source 0's.
 */
bool DataModel::SessionAnnouncer::combinedFrameStandsIn(bool anyOwnFrame) const noexcept
{
  return !anyOwnFrame && !isMultiSource() && hasLayout();
}

/**
 * @brief Whether the project has more than one source.
 */
bool DataModel::SessionAnnouncer::isMultiSource() const noexcept
{
  return m_project.sources.size() > 1;
}

/**
 * @brief Whether the project has anything to lay out: at least one group, and a title.
 */
bool DataModel::SessionAnnouncer::hasLayout() const noexcept
{
  return !m_project.groups.empty() && !m_project.title.isEmpty();
}

/**
 * @brief Whether the project has groups and every one is an image widget.
 */
bool DataModel::SessionAnnouncer::isImageOnly() const
{
  return !m_project.groups.empty()
      && std::all_of(m_project.groups.begin(),
                     m_project.groups.end(),
                     [](const DataModel::Group& g) { return g.widget == QLatin1String("image"); });
}

/**
 * @brief Hands @p src to the host unless it has no group, which would be an empty layout.
 */
void DataModel::SessionAnnouncer::announceIfGrouped(const DataModel::Source& src)
{
  const bool grouped =
    std::any_of(m_project.groups.begin(),
                m_project.groups.end(),
                [&src](const DataModel::Group& g) { return g.sourceId == src.sourceId; });

  if (grouped)
    m_host.announceSourceTemplate(src);
}
