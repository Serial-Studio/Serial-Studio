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
#include <QJsonObject>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QTimer>

namespace Platform {

/**
 * @brief Client end of the kiosk control channel (spec 0125): one JSON object per line over a
 *        local socket whose server is the launcher. It knows nothing about windows; it turns
 *        received lines into the three command signals and hello/state into sent lines. One
 *        client stands for one window, named by its identity.
 */
class KioskControl : public QObject {
  Q_OBJECT

signals:
  void linkEstablished();
  void raiseRequested();
  void closeRequested();
  void screenRequested(const QString& name, int reserve_left);

public:
  explicit KioskControl(QObject* parent = nullptr);
  ~KioskControl() override;

  enum class Frame {
    None,
    Line,
    Overflow
  };

  static constexpr int kProtocolVersion = 1;
  static constexpr int kMaxLineBytes    = 64 * 1024;

  [[nodiscard]] bool isLinked() const;
  [[nodiscard]] static Frame takeLine(QByteArray& buffer, QByteArray& line);

public slots:
  void setRetryInterval(int msec);
  void setIdentity(const QString& id);
  void start(const QString& server_name);
  void sendState(const QString& screen, bool active, const QString& title = QString());
  void sendHello(const QString& title, const QString& screen);

private slots:
  void dial();
  void onConnected();
  void readPending();
  void scheduleRetry();

private:
  void dispatch(const QByteArray& line);
  void sendLine(const QJsonObject& message);

  QTimer m_drain;
  QTimer m_retry;
  QByteArray m_buffer;
  QString m_identity;
  QString m_serverName;
  QLocalSocket m_socket;
};

}  // namespace Platform
