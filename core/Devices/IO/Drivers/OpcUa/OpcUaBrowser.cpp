/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/OpcUa/OpcUaBrowser.h"

#include <QUrl>

#include "IO/Drivers/OpcUaTagModel.h"

static constexpr int kOpcUaBrowseDeadlineMs      = 15000;
static constexpr const char* kBrowserBackendName = "open62541";

/**
 * @brief Constructs the browser and arms its single-shot deadline; the session and the tree model
 *        are created on first use.
 */
IO::Drivers::OpcUaBrowser::OpcUaBrowser(OpcUaBrowseHost& host, QObject* parent)
  : QObject(parent)
  , m_host(host)
  , m_browsing(false)
  , m_timer(new QTimer(this))
  , m_session(nullptr)
  , m_tagModel(nullptr)
{
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, &IO::Drivers::OpcUaBrowser::onTimeout);
}

/**
 * @brief Retires the session and the tree model NOW rather than through deleteLater, which would
 *        run after the driver is gone and reach into the backend through a freed parent.
 */
IO::Drivers::OpcUaBrowser::~OpcUaBrowser()
{
  shutdown();
}

/**
 * @brief Drops the browse session and the tree model; idempotent, so the driver can call it at the
 *        point its own teardown expects without racing this object's destruction.
 */
void IO::Drivers::OpcUaBrowser::shutdown()
{
  teardownSession();
  delete m_tagModel;
  m_tagModel = nullptr;
}

/**
 * @brief Opens a browse-only session beside the live one so the picker can walk the address
 *        space without touching the live link; stop() ends it.
 */
void IO::Drivers::OpcUaBrowser::start()
{
  if (m_browsing)
    return;

  const auto url = m_host.endpointUrl();
  if (!QUrl(url).isValid() || QUrl(url).host().isEmpty()) {
    Q_EMIT browseFailed(tr("\"%1\" is not a valid endpoint URL.").arg(url));
    return;
  }

  teardownSession();
  m_session = m_host.makeSession();
  if (!m_session) {
    Q_EMIT browseFailed(
      tr("The %1 stack is not available in this build.").arg(kBrowserBackendName));
    return;
  }

  connect(m_session, &OpcUaSession::connected, this, &IO::Drivers::OpcUaBrowser::onConnected);
  connect(m_session, &OpcUaSession::connectFailed, this, &IO::Drivers::OpcUaBrowser::onFailed);
  connect(m_session, &OpcUaSession::disconnected, this, [this] {
    onFailed(tr("The browse session was closed by the server"));
  });

  tagModel()->preselect(m_host.tags());
  m_browsing = true;
  m_timer->start(kOpcUaBrowseDeadlineMs);
  Q_EMIT browsingChanged();
  m_host.prepareClientIdentity();

  if (m_host.hasSelectedEndpoint()) {
    if (!m_session->connectToEndpoint(m_host.dialEndpoint(), m_host.identity()))
      onFailed(tr("The browse session could not be started"));

    return;
  }

  m_host.requestBrowseDiscovery();
}

/**
 * @brief Commits the picker's selection as the tag list and ends the browse session. Tags the
 *        picker never fetched keep their place: an unexpanded folder is not an unchecked one.
 */
void IO::Drivers::OpcUaBrowser::stop()
{
  if (m_tagModel && m_session && m_session->isOpen()) {
    QJsonArray array;
    const auto selected = m_tagModel->selectedTags();
    for (const auto& tag : selected)
      array.append(tagToJson(tag));

    const auto& current = m_host.tags();
    for (const auto& tag : current)
      if (!m_tagModel->hasSeen(tag.nodeId))
        array.append(tagToJson(tag));

    m_host.commitBrowsedTags(array);
  }

  if (m_tagModel)
    m_tagModel->setSession(nullptr);

  m_timer->stop();
  teardownSession();
  if (!m_browsing)
    return;

  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Ends the browse session WITHOUT committing the picker selection (the dialog's Cancel).
 */
void IO::Drivers::OpcUaBrowser::cancel()
{
  if (m_tagModel)
    m_tagModel->setSession(nullptr);

  m_timer->stop();
  teardownSession();
  if (!m_browsing)
    return;

  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief Dials the endpoint a discovery this session asked for has just selected, or fails the
 *        browse when the server offers nothing this build can open.
 */
void IO::Drivers::OpcUaBrowser::dialAfterDiscovery(const QString& discoveryError)
{
  if (m_session && m_host.hasSelectedEndpoint()) {
    if (m_session->connectToEndpoint(m_host.dialEndpoint(), m_host.identity()))
      return;
  }

  Q_EMIT browseFailed(discoveryError.isEmpty() ? tr("Endpoint discovery failed") : discoveryError);
  cancel();
}

/**
 * @brief Returns true while the browse session is up.
 */
bool IO::Drivers::OpcUaBrowser::browsing() const noexcept
{
  return m_browsing;
}

/**
 * @brief The picker's model, created on first use and fed only while a browse session is up.
 */
IO::Drivers::OpcUaTagModel* IO::Drivers::OpcUaBrowser::tagModel()
{
  if (!m_tagModel) {
    m_tagModel = new OpcUaTagModel(this);
    connect(
      m_tagModel, &OpcUaTagModel::browseError, this, [this](const QString& id, const QString& r) {
        Q_EMIT browseFailed(tr("Browse of %1 failed: %2").arg(id, r));
      });
  }

  return m_tagModel;
}

/**
 * @brief QML-facing view of the tag model.
 */
QObject* IO::Drivers::OpcUaBrowser::tagModelObject()
{
  return tagModel();
}

/**
 * @brief Lends the connected browse session to the model and fetches the root level.
 */
void IO::Drivers::OpcUaBrowser::onConnected()
{
  if (!m_session)
    return;

  m_timer->stop();

  tagModel()->setSession(m_session);
  tagModel()->fetchMore(QModelIndex());
}

/**
 * @brief A browse session that could not be opened, or one the server closed. The reason travels
 *        as the driver's last error, which is what the pane's status line reads.
 */
void IO::Drivers::OpcUaBrowser::onFailed(const QString& reason)
{
  if (!m_browsing)
    return;

  m_timer->stop();
  m_host.reportTrustFailure(m_session);

  const auto fallback = tr("Could not open a browse session on %1");
  const auto detail   = reason.isEmpty() ? fallback.arg(m_host.selectedEndpointUrl()) : reason;
  Q_EMIT browseSessionFailed(detail);

  if (m_tagModel)
    m_tagModel->setSession(nullptr);

  teardownSession();
  m_browsing = false;
  Q_EMIT browsingChanged();
}

/**
 * @brief The picker's dial has no other bound: without this deadline a server that accepts the
 *        socket and then answers nothing leaves the dialog spinning forever.
 */
void IO::Drivers::OpcUaBrowser::onTimeout()
{
  if (!m_browsing)
    return;

  onFailed(tr("Timed out after %1 s").arg(kOpcUaBrowseDeadlineMs / 1000));
}

/**
 * @brief Disconnects every signal from the session and retires it; the pointer is nulled so a late
 *        callback can never reach a freed object.
 */
void IO::Drivers::OpcUaBrowser::teardownSession()
{
  if (!m_session)
    return;

  disconnect(m_session, nullptr, this, nullptr);
  m_session->close();
  m_session->deleteLater();
  m_session = nullptr;
}
