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

#include <QByteArray>
#include <QByteArrayView>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QTimer>

#include "Core/Bus/Subscription.h"
#include "Core/IO/IRawByteTap.h"
#include "DataModel/Project/DeviceDeclaration.h"

namespace Core::Bus {
class MessageBus;
struct ConnectionStateChanged;
}  // namespace Core::Bus

namespace DataModel {

class ProjectModel;

/**
 * @brief What the Project File watch last made of a device declaration (spec 0111 R3).
 */
enum class DeclarationStatus : quint8 {
  None,
  Matches,
  Differs,
  Kept,
  Rejected,
};

/**
 * @brief The GUI-thread half of device declarations (spec 0111 R3/R6): asks a Quick Plot device
 *        for its declaration on connect, and in Project File mode reads one from the raw bytes
 *        and offers it as a new unsaved project when it differs, never touching the loaded one.
 *        A ProjectModel sub-object bound in setupExternalConnections(); inert when sealed.
 */
class DeclarationWatch
  : public QObject
  , public IO::IRawByteTap {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool requestOnConnect
             READ requestOnConnect
             WRITE setRequestOnConnect
             NOTIFY requestOnConnectChanged)
  Q_PROPERTY(bool pendingDifference
             READ pendingDifference
             NOTIFY pendingDifferenceChanged)
  // clang-format on

signals:
  void requestOnConnectChanged();
  void pendingDifferenceChanged();

public:
  DeclarationWatch(ProjectModel& model, Core::Bus::MessageBus& bus);
  DeclarationWatch(DeclarationWatch&&)                 = delete;
  DeclarationWatch(const DeclarationWatch&)            = delete;
  DeclarationWatch& operator=(DeclarationWatch&&)      = delete;
  DeclarationWatch& operator=(const DeclarationWatch&) = delete;

  [[nodiscard]] bool requestOnConnect() const noexcept { return m_requestOnConnect; }

  [[nodiscard]] bool pendingDifference() const noexcept
  {
    return m_status == DeclarationStatus::Differs;
  }

  [[nodiscard]] QJsonObject status() const;

  Q_INVOKABLE [[nodiscard]] bool adopt();

  void setupExternalConnections();
  void onDeviceBytes(int deviceId, const IO::CapturedDataPtr& data) override;

public slots:
  void keep();
  void setRequestOnConnect(bool enabled);

private:
  [[nodiscard]] qsizetype resumeCarry(QByteArrayView chunk, QByteArrayView prefix);
  [[nodiscard]] qsizetype collectLine(QByteArrayView chunk, qsizetype from);
  [[nodiscard]] QByteArray noticeKey() const;

  void onLinkState(const Core::Bus::ConnectionStateChanged& link);
  void onOperationMode(int mode);
  void scanChunk(QByteArrayView chunk);
  void offerLine(QByteArrayView raw);
  void acceptLine(QByteArrayView line);
  void forwardLine(QByteArrayView line);
  void notifyDifference();
  void setStatus(DeclarationStatus status);
  void scheduleReevaluate();
  void reevaluate();
  void forget();

private:
  int m_mode;
  bool m_connected;
  bool m_binarySession;
  bool m_requestOnConnect;
  DeclarationStatus m_status;
  QString m_reason;
  QByteArray m_carry;
  QByteArray m_lastLine;
  QByteArray m_declaredContent;
  QJsonObject m_declaredProjection;
  QTimer m_reevaluateTimer;
  QSettings m_settings;
  ProjectModel& m_model;
  QSet<QByteArray> m_kept;
  QSet<QByteArray> m_noticed;
  Core::Bus::MessageBus& m_bus;
  Core::Bus::Subscription m_modeState;
  Core::Bus::Subscription m_linkState;
  Core::Bus::Subscription m_framingState;
  DeclarationState m_state;
  DeviceDeclaration m_declaration;
  IO::CapturedData::SteadyTimePoint m_chunkTime;
  IO::CapturedData::SteadyTimePoint m_lastForward;
};

}  // namespace DataModel
