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

#include <QByteArray>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QVector>

#include "DataModel/Frame.h"
#include "IO/AsyncTcpDial.h"
#include "IO/Drivers/Iec104/Apci.h"
#include "IO/Drivers/Iec104/Asdu.h"
#include "IO/Drivers/OpcUaWire.h"
#include "IO/HAL_Driver.h"

class QTimer;
class AppState;
class QTcpSocket;

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO {
namespace Drivers {

/**
 * @brief One information object the station has reported, in the order it was first seen. The
 *        position in the table IS the wire slot, which is why entries are only ever appended: a
 *        renumbering would silently repoint every dataset of an already-generated project.
 */
struct Iec104Point {
  quint32 ioa                 = 0;
  std::uint8_t typeId         = 0;
  Iec104Proto::PointKind kind = Iec104Proto::PointKind::Invalid;

  [[nodiscard]] bool operator==(const Iec104Point& other) const noexcept
  {
    return ioa == other.ioa && typeId == other.typeId;
  }
};

/**
 * @brief HAL driver for IEC 60870-5-104 telecontrol stations (spec 0073), monitor direction only.
 *        The in-house protocol stack lives in Iec104/Apci and Iec104/Asdu; this class owns the
 *        socket, the discovered point table and the delta publishing tick. The point list is
 *        DISCOVERED rather than configured: the station interrogation answers with its database.
 */
class Iec104 : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString host
             READ host
             WRITE setHost
             NOTIFY hostChanged)
  Q_PROPERTY(int port
             READ port
             WRITE setPort
             NOTIFY portChanged)
  Q_PROPERTY(int commonAddress
             READ commonAddress
             WRITE setCommonAddress
             NOTIFY commonAddressChanged)
  Q_PROPERTY(int windowK
             READ windowK
             WRITE setWindowK
             NOTIFY protocolParametersChanged)
  Q_PROPERTY(int windowW
             READ windowW
             WRITE setWindowW
             NOTIFY protocolParametersChanged)
  Q_PROPERTY(int timeoutT1
             READ timeoutT1
             WRITE setTimeoutT1
             NOTIFY protocolParametersChanged)
  Q_PROPERTY(int timeoutT2
             READ timeoutT2
             WRITE setTimeoutT2
             NOTIFY protocolParametersChanged)
  Q_PROPERTY(int timeoutT3
             READ timeoutT3
             WRITE setTimeoutT3
             NOTIFY protocolParametersChanged)
  Q_PROPERTY(int pointCount
             READ pointCount
             NOTIFY pointsChanged)
  Q_PROPERTY(QString statusText
             READ statusText
             NOTIFY statusChanged)
  // clang-format on

signals:
  void hostChanged();
  void portChanged();
  void pointsChanged();
  void statusChanged();
  void commonAddressChanged();
  void protocolParametersChanged();

public:
  explicit Iec104();
  ~Iec104();

  Iec104(Iec104&&)                 = delete;
  Iec104(const Iec104&)            = delete;
  Iec104& operator=(Iec104&&)      = delete;
  Iec104& operator=(const Iec104&) = delete;

  void close() override;
  void setPersistent(const bool persistent) noexcept;
  void setSessionPeer(Iec104* peer);

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QString host() const;
  [[nodiscard]] int port() const;
  [[nodiscard]] int commonAddress() const;
  [[nodiscard]] int windowK() const;
  [[nodiscard]] int windowW() const;
  [[nodiscard]] int timeoutT1() const;
  [[nodiscard]] int timeoutT2() const;
  [[nodiscard]] int timeoutT3() const;
  [[nodiscard]] int pointCount() const;
  [[nodiscard]] QString statusText() const;

  [[nodiscard]] const QVector<Iec104Point>& points() const noexcept;
  [[nodiscard]] QJsonArray pointsJson() const;
  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject statusJson() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] DataModel::ProjectModel* loadGeneratedProject();

  [[nodiscard]] Q_INVOKABLE QString pointInfo(const int index) const;

public slots:
  void generateProject();
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setHost(const QString& host);
  void setPort(const int port);
  void setCommonAddress(const int address);
  void setWindowK(const int k);
  void setWindowW(const int w);
  void setTimeoutT1(const int ms);
  void setTimeoutT2(const int ms);
  void setTimeoutT3(const int ms);
  void setPoints(const QJsonArray& points);
  void clearPoints();

private slots:
  void onReadyRead();
  void onSocketError();
  void onProtocolTick();
  void onDialFinished(bool ok, const QString& reason);

private:
  void doClose();
  void beginSession();
  void loadSettings();
  void savePoints();
  void reserveFrame();
  void applyProtocolParameters();
  void adoptDiscoveredPoints();
  void drainReceiveBuffer(qint64 nowMs);
  void handleUnnumbered(Iec104Proto::UFunction function, qint64 nowMs);
  void handleInformation(QByteArrayView asdu);
  void ingestPoint(const Iec104Proto::Point& point);
  void publishDeltaFrame();
  void reportLinkLost(const QString& reason);
  void sendApdu(const QByteArray& apdu);
  void dialStation();

  [[nodiscard]] int slotForPoint(const Iec104Proto::Point& point);
  [[nodiscard]] const Iec104* sessionPeer() const;
  [[nodiscard]] const QVector<Iec104Point>& pointTable() const noexcept;
  [[nodiscard]] CapturedData::SteadyTimePoint toSteady(qint64 stationMs);

  [[nodiscard]] static qint64 monotonicMs();
  [[nodiscard]] static QString pointName(const Iec104Point& point);
  [[nodiscard]] static OpcUaWire::Type wireTypeFor(Iec104Proto::PointKind kind) noexcept;
  [[nodiscard]] static DataModel::Dataset datasetFor(const Iec104Point& point, int index);

  AppState& m_appState;
  DataModel::ProjectModel& m_projectModel;

  bool m_open;
  bool m_loading;
  bool m_started;
  bool m_persistent;
  bool m_clockValid;
  int m_port;
  int m_commonAddress;
  int m_windowK;
  int m_windowW;
  int m_timeoutT1;
  int m_timeoutT2;
  int m_timeoutT3;
  qint64 m_lastStampNs;
  qint64 m_clockOffsetNs;
  qint64 m_stationBaseMs;
  quint64 m_linkDrops;
  quint64 m_badQualityPoints;
  quint64 m_skippedAsdus;
  quint64 m_testTimeouts;
  quint64 m_unstampedPoints;
  quint64 m_framesPublished;
  qsizetype m_dirtyCount;
  qsizetype m_frameReserveBytes;
  QString m_host;
  QString m_lastError;
  QTimer* m_timer;
  QTcpSocket* m_socket;
  AsyncTcpDial m_dial;
  QByteArray m_rx;
  QByteArray m_frame;
  QPointer<Iec104> m_sessionPeer;
  Iec104Proto::Connection m_link;
  QHash<quint64, int> m_slotForKey;
  QVector<Iec104Point> m_points;
  QList<QVariant> m_values;
  QList<qint64> m_stamps;
  QList<bool> m_dirty;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO
