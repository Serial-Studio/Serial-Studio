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

#include "IO/ConnectionManager/EntitlementGate.h"

#include <QDebug>
#include <QObject>

#include "Core/DataModel/FrameSupport.h"
#include "Core/License.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager/ConnectFanOut.h"

using Refusal = IO::DeviceTableQuery::ConnectRefusal;

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the device-table query, the connect fan-out and the facade's bus type and mode; all
 *        four outlive this object and are read at the moment of each question, never cached.
 */
IO::EntitlementGate::EntitlementGate(const DeviceTableQuery& query,
                                     const ConnectFanOut& fanOut,
                                     const SerialStudio::BusType& busType,
                                     const SerialStudio::OperationMode& mode)
  : m_context(nullptr)
  , m_fanOut(fanOut)
  , m_query(query)
  , m_busType(busType)
  , m_mode(mode)
  , m_builtEntitled(Core::License::activated())
  , m_rebuildDeferred(false)
  , m_admittedTables(0)
  , m_admittedTransforms(0)
{}

/**
 * @brief Binds the facade actions the gate drives and the object its queued checks run on; the
 *        facade's constructor calls it once, before any connect can reach the gate.
 */
void IO::EntitlementGate::bind(QObject* context, Actions actions)
{
  SS_ASSERT(context != nullptr, return);
  SS_ASSERT(actions.connect != nullptr, return);

  m_context = context;
  m_actions = std::move(actions);
}

//--------------------------------------------------------------------------------------------------
// Session state
//--------------------------------------------------------------------------------------------------

/**
 * @brief The facade's connected verdict: any open device in ProjectFile mode, else device 0.
 */
bool IO::EntitlementGate::connected() const
{
  if (m_mode == SerialStudio::ProjectFile)
    return m_query.anyOpen();

  return m_query.primaryOpen();
}

/**
 * @brief True while a session is open, a dial is in flight, or a connect request is pending: every
 *        state a device rebuild or a refusal would cut short.
 */
bool IO::EntitlementGate::sessionLive() const
{
  return connected() || m_query.anyDeviceConnecting() || m_fanOut.requestPending();
}

/**
 * @brief Whether a content count runs past what the live session was admitted with.
 */
bool IO::EntitlementGate::exceedsAdmitted(const int transforms, const int tables) const noexcept
{
  return transforms > m_admittedTransforms || tables > m_admittedTables;
}

//--------------------------------------------------------------------------------------------------
// Connect verdict
//--------------------------------------------------------------------------------------------------

/**
 * @brief Names what the current licensing state refuses about the pending connect: a gated bus or
 *        a multi-source project (spec 0092), or a project whose transforms or user tables need an
 *        entitlement to run (spec 0094). A GPL build has no gated bus to refuse, so only the
 *        content reason survives there.
 */
IO::DeviceTableQuery::ConnectRefusal IO::EntitlementGate::pendingRefusal() const
{
  if (Core::License::activated())
    return Refusal::None;

  const auto refusal = m_query.connectRefusal(m_busType, m_mode);
#ifndef BUILD_COMMERCIAL
  if (refusal != Refusal::ProContent)
    return Refusal::None;
#endif

  return refusal;
}

/**
 * @brief The text an API error or a log line gives for @p refusal; empty for None. One wording for
 *        every reason, through Core::License::requiresProMessage().
 */
QString IO::EntitlementGate::refusalReason(const Refusal refusal)
{
  switch (refusal) {
    case Refusal::None:
      return QString();
    case Refusal::ProBus:
      return Core::License::requiresProMessage(QStringLiteral("the selected bus"));
    case Refusal::MultiSource:
      return Core::License::requiresProMessage(QStringLiteral("projects with several sources"));
    case Refusal::ProContent:
      return Core::License::requiresProMessage(
        QStringLiteral("projects that use dataset transforms or Variables"));
  }

  return QString();
}

/**
 * @brief Stops a connect the licensing state does not cover; with @p raiseIntent it raises the Pro
 *        intent with the connect as retry, so accepting a trial connects without a second gesture.
 *        Runs before beginRequest(), and an admitted connect records the content it may run.
 */
bool IO::EntitlementGate::refuseConnect(const bool raiseIntent)
{
  const auto refusal = pendingRefusal();
  if (refusal == Refusal::None) {
    const auto content   = m_query.projectContent(m_mode);
    m_admittedTables     = content.tables;
    m_admittedTransforms = content.transforms;
    return false;
  }

  qWarning().noquote() << "[ConnectionManager] Connect refused:" << refusalReason(refusal);
  if (!raiseIntent)
    return true;

  const bool content = (refusal == Refusal::ProContent);
  Core::License::requestProFeature(content ? QString::fromLatin1(Core::License::kProContentFeature)
                                           : QStringLiteral("driver.connect"),
                                   m_actions.connect);
  return true;
}

/**
 * @brief Whether a driver's own reopen must stay closed: without an entitlement it may resume only
 *        the content its session was admitted with, so a project loaded while the link was down
 *        cannot start through the recovery path (spec 0094).
 */
bool IO::EntitlementGate::refuseRecovery() const
{
  const auto content = m_query.projectContent(m_mode);
  if (Core::License::activated() || !exceedsAdmitted(content.transforms, content.tables))
    return false;

  qWarning().noquote() << "[ConnectionManager] Reconnect refused:"
                       << refusalReason(Refusal::ProContent);
  return true;
}

/**
 * @brief Queues the reconnect a rebuild owes the session it closed. A rebuild run under a remote
 *        request refuses that reconnect silently: the request's dispatch scope has unwound by the
 *        time the reconnect runs, so a prompt would otherwise land on the host.
 */
void IO::EntitlementGate::queueReconnect()
{
  SS_ASSERT(m_context != nullptr, return);

  const bool remote = Core::License::remoteDispatchActive();
  QMetaObject::invokeMethod(
    m_context,
    [this, remote] {
      if (connected() || (remote && refuseConnect(false)))
        return;

      m_actions.connect();
    },
    Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Live session
//--------------------------------------------------------------------------------------------------

/**
 * @brief Watches the content a session may run. Entitled, that is whatever the project holds;
 *        without one, a live session whose project gains transforms or tables past its admission
 *        ends one event-loop turn later, after the load that brought them finishes. A licence
 *        ending changes no content, so a session running into R8 is left alone.
 */
void IO::EntitlementGate::observeContent()
{
  const auto content = m_query.projectContent(m_mode);
  if (Core::License::activated()) {
    m_admittedTables     = content.tables;
    m_admittedTransforms = content.transforms;
    return;
  }

  const bool grew      = exceedsAdmitted(content.transforms, content.tables);
  m_admittedTables     = qMin(m_admittedTables, content.tables);
  m_admittedTransforms = qMin(m_admittedTransforms, content.transforms);
  if (!grew || !sessionLive() || m_context == nullptr)
    return;

  const bool remote = Core::License::remoteDispatchActive();
  QMetaObject::invokeMethod(
    m_context, [this, remote] { endGrownSession(!remote); }, Qt::QueuedConnection);
}

/**
 * @brief Ends the session observeContent() found running Pro content past its admission, unless an
 *        entitlement or a disconnect settled it in the meantime; the intent stays silent for a
 *        remote request, whose caller reads the reason from the log and the link state.
 */
void IO::EntitlementGate::endGrownSession(const bool raiseIntent)
{
  const auto content = m_query.projectContent(m_mode);
  if (Core::License::activated() || !sessionLive())
    return;

  if (!exceedsAdmitted(content.transforms, content.tables))
    return;

  qWarning().noquote() << "[ConnectionManager] Session ended:"
                       << refusalReason(Refusal::ProContent);
  m_actions.disconnect();
  if (raiseIntent)
    Core::License::requestProFeature(QString::fromLatin1(Core::License::kProContentFeature),
                                     m_actions.connect);
}

//--------------------------------------------------------------------------------------------------
// Licence transitions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decides what a licensing transition does to the devices; true means do not rebuild now.
 *        A transition the devices were already rebuilt for (a trial start publishes twice) changes
 *        nothing. A live session or dial on one free bus holds the rebuild until it ends, because
 *        the rebuild would close or cancel it (spec 0094 R8).
 */
bool IO::EntitlementGate::deferRebuild()
{
  if (Core::License::activated() == m_builtEntitled)
    return true;

  m_rebuildDeferred = sessionLive() && m_query.busRefusal(m_busType, m_mode) == Refusal::None;
  return m_rebuildDeferred;
}

/**
 * @brief Records that the devices are being rebuilt under the current entitlement; any rebuild
 *        replaces a held one, so the hold is dropped.
 */
void IO::EntitlementGate::noteRebuild()
{
  m_builtEntitled   = Core::License::activated();
  m_rebuildDeferred = false;
}

/**
 * @brief Runs a held rebuild once the session that held it is over. Queued and checked again when
 *        it runs, so a connect landing in the same event-loop turn keeps the hold instead of being
 *        closed under its feet.
 */
void IO::EntitlementGate::releaseDeferredRebuild()
{
  if (!m_rebuildDeferred || m_context == nullptr)
    return;

  QMetaObject::invokeMethod(
    m_context,
    [this] {
      if (!m_rebuildDeferred || sessionLive())
        return;

      m_actions.rebuild();
    },
    Qt::QueuedConnection);
}
