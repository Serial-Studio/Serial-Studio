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

#pragma once

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

#include "IO/Drivers/OpcUa/OpcUaTag.h"
#include "IO/Drivers/OpcUaSession.h"
#include "IO/Drivers/OpcUaTypes.h"

namespace IO {
namespace Drivers {

class OpcUaTagModel;

/**
 * @brief What a browse session needs from the driver that owns it: a session factory, the endpoint
 *        and identity a dial would use, the current tag list, and somewhere to hand a committed
 *        selection. Implemented by the OpcUa facade, which is why the browser itself resolves no
 *        singleton and holds no driver state.
 */
class OpcUaBrowseHost {
public:
  OpcUaBrowseHost()          = default;
  virtual ~OpcUaBrowseHost() = default;

  OpcUaBrowseHost(OpcUaBrowseHost&&)                 = delete;
  OpcUaBrowseHost(const OpcUaBrowseHost&)            = delete;
  OpcUaBrowseHost& operator=(OpcUaBrowseHost&&)      = delete;
  OpcUaBrowseHost& operator=(const OpcUaBrowseHost&) = delete;

  virtual void prepareClientIdentity()                         = 0;
  virtual void requestBrowseDiscovery()                        = 0;
  virtual void commitBrowsedTags(const QJsonArray& tags)       = 0;
  virtual void reportTrustFailure(const OpcUaSession* session) = 0;

  [[nodiscard]] virtual OpcUaSession* makeSession()                  = 0;
  [[nodiscard]] virtual bool hasSelectedEndpoint() const noexcept    = 0;
  [[nodiscard]] virtual QString endpointUrl() const                  = 0;
  [[nodiscard]] virtual QString selectedEndpointUrl() const          = 0;
  [[nodiscard]] virtual OpcUaTypes::Endpoint dialEndpoint() const    = 0;
  [[nodiscard]] virtual OpcUaSession::Identity identity() const      = 0;
  [[nodiscard]] virtual const QList<OpcUaTag>& tags() const noexcept = 0;
};

/**
 * @brief The tag picker's browse-only session (spec 0066 R6): a second session opened beside the
 *        live one so the address space can be walked without touching the live link, plus the
 *        lazily browsed tree model it feeds. A browse never reports an open verdict: the driver's
 *        single `openFinished` contract belongs to the live session alone.
 */
class OpcUaBrowser : public QObject {
  Q_OBJECT

signals:
  void browsingChanged();
  void browseFailed(const QString& reason);
  void browseSessionFailed(const QString& reason);

public:
  explicit OpcUaBrowser(OpcUaBrowseHost& host, QObject* parent = nullptr);
  ~OpcUaBrowser();

  OpcUaBrowser(OpcUaBrowser&&)                 = delete;
  OpcUaBrowser(const OpcUaBrowser&)            = delete;
  OpcUaBrowser& operator=(OpcUaBrowser&&)      = delete;
  OpcUaBrowser& operator=(const OpcUaBrowser&) = delete;

  void shutdown();
  void dialAfterDiscovery(const QString& discoveryError);

  [[nodiscard]] bool browsing() const noexcept;
  [[nodiscard]] OpcUaTagModel* tagModel();
  [[nodiscard]] QObject* tagModelObject();

public slots:
  void start();
  void stop();
  void cancel();

private slots:
  void onConnected();
  void onFailed(const QString& reason);
  void onTimeout();

private:
  void teardownSession();

  OpcUaBrowseHost& m_host;
  bool m_browsing;
  QTimer* m_timer;
  OpcUaSession* m_session;
  OpcUaTagModel* m_tagModel;
};

}  // namespace Drivers
}  // namespace IO
