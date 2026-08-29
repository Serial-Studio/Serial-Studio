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

#include <atomic>
#include <memory>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QVariant>
#include <QVector>

#include "DataModel/Frame.h"
#include "IO/Drivers/OpcUaWire.h"
#include "IO/HAL_Driver.h"

class QTimer;

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace IO {
namespace Drivers {

/**
 * @brief One polled CIP tag: the name the dashboard shows, the controller's symbolic tag name, the
 *        declared wire type and, for array tags, the element to read (-1 for a scalar).
 */
struct EipTag {
  QString name;
  QString tag;
  OpcUaWire::Type type = OpcUaWire::Type::Invalid;
  int element          = -1;

  [[nodiscard]] bool operator==(const EipTag& other) const noexcept
  {
    return name == other.name && tag == other.tag && type == other.type && element == other.element;
  }
};

/**
 * @brief Poll worker for the EtherNet/IP client (spec 0073). libplctag's create and read calls
 *        BLOCK until the controller answers or the timeout expires, so they run here on the
 *        driver's own QThread. Every tag handle is created, read and destroyed on this thread.
 */
class EipPollWorker : public QObject {
  Q_OBJECT

signals:
  void frameReady(const QByteArray& frame, qint64 stampNs);
  void linkLost(const QString& reason);

public:
  explicit EipPollWorker();
  ~EipPollWorker();

  EipPollWorker(EipPollWorker&&)                 = delete;
  EipPollWorker(const EipPollWorker&)            = delete;
  EipPollWorker& operator=(EipPollWorker&&)      = delete;
  EipPollWorker& operator=(const EipPollWorker&) = delete;

  void configure(const QString& host,
                 const QString& path,
                 const QString& plcType,
                 int interval,
                 QVector<EipTag> tags);

  void requestAbort() noexcept;

  [[nodiscard]] quint64 readsOk() const noexcept;
  [[nodiscard]] quint64 readsFailed() const noexcept;
  [[nodiscard]] quint64 framesPublished() const noexcept;
  [[nodiscard]] const QString& dialError() const noexcept;

public slots:
  [[nodiscard]] bool connectToPlc();
  void shutdown();

private slots:
  void onPollTick();

private:
  [[nodiscard]] QByteArray attributes(const EipTag& tag) const;
  [[nodiscard]] bool readTag(int index, QVariant& value);
  void publishDirtySlots(qint64 stampNs);
  void reportFailure(const QString& reason);

  bool m_open;
  bool m_reported;
  int m_interval;
  int m_deadTicks;
  int m_frameSlot;
  QString m_host;
  QString m_path;
  QString m_plcType;
  QString m_dialError;
  QTimer* m_timer;
  QByteArray m_frames[2];
  QVector<EipTag> m_tags;
  QList<int> m_handles;
  QList<QVariant> m_values;
  QList<bool> m_dirty;
  std::atomic<bool> m_abort;
  // code-verify off
  // Counters are pulled from the GUI thread while the poll thread increments them, so they are
  // atomic rather than the plain quint64 of spec 0033. They are DELIBERATELY packed: one poll
  // thread writes all three at poll rate and one reader samples them at 1 Hz, so there is no
  // cross-core write contention to pad against and three cache lines of padding would cost more
  // than the sharing.
  std::atomic<quint64> m_readsOk;
  std::atomic<quint64> m_readsFailed;
  std::atomic<quint64> m_framesPublished;
  // code-verify on
};

/**
 * @brief HAL driver for EtherNet/IP controllers reached by CIP symbolic tag name (spec 0073):
 *        Allen-Bradley ControlLogix/CompactLogix, MicroLogix, SLC and PLC-5 families, plus the
 *        Omron NJ/NX line. The driver keeps the configuration and the wire layout; a driver-owned
 *        worker thread does the blocking protocol work.
 */
class EthernetIp : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString host
             READ host
             WRITE setHost
             NOTIFY hostChanged)
  Q_PROPERTY(QString cipPath
             READ cipPath
             WRITE setCipPath
             NOTIFY cipPathChanged)
  Q_PROPERTY(int plcTypeIndex
             READ plcTypeIndex
             WRITE setPlcTypeIndex
             NOTIFY plcTypeChanged)
  Q_PROPERTY(QStringList plcTypeList
             READ plcTypeList
             CONSTANT)
  Q_PROPERTY(QStringList plcTypeLabels
             READ plcTypeLabels
             CONSTANT)
  Q_PROPERTY(QStringList tagTypeList
             READ tagTypeList
             CONSTANT)
  Q_PROPERTY(int pollInterval
             READ pollInterval
             WRITE setPollInterval
             NOTIFY pollIntervalChanged)
  Q_PROPERTY(int tagCount
             READ tagCount
             NOTIFY tagsChanged)
  Q_PROPERTY(QString statusText
             READ statusText
             NOTIFY statusChanged)
  Q_PROPERTY(bool tagsLocked
             READ tagsLocked
             NOTIFY statusChanged)
  // clang-format on

signals:
  void hostChanged();
  void tagsChanged();
  void statusChanged();
  void cipPathChanged();
  void plcTypeChanged();
  void pollIntervalChanged();

public:
  explicit EthernetIp();
  ~EthernetIp();

  EthernetIp(EthernetIp&&)                 = delete;
  EthernetIp(const EthernetIp&)            = delete;
  EthernetIp& operator=(EthernetIp&&)      = delete;
  EthernetIp& operator=(const EthernetIp&) = delete;

  void close() override;
  void setPersistent(const bool persistent) noexcept;
  void setSessionPeer(EthernetIp* peer);

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QString host() const;
  [[nodiscard]] QString cipPath() const;
  [[nodiscard]] QString plcType() const;
  [[nodiscard]] int plcTypeIndex() const;
  [[nodiscard]] int pollInterval() const;
  [[nodiscard]] int tagCount() const;
  [[nodiscard]] bool tagsLocked() const noexcept;
  [[nodiscard]] QString statusText() const;

  [[nodiscard]] static QStringList plcTypeList();
  [[nodiscard]] static QStringList tagTypeList();
  [[nodiscard]] static QStringList plcTypeLabels();

  [[nodiscard]] const QVector<EipTag>& tags() const noexcept;
  [[nodiscard]] QJsonArray tagsJson() const;
  [[nodiscard]] QJsonArray wireSchema() const;
  [[nodiscard]] QJsonObject statusJson() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] DataModel::ProjectModel* loadGeneratedProject();

  [[nodiscard]] Q_INVOKABLE QString tagInfo(const int index) const;

public slots:
  void generateProject();
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setHost(const QString& host);
  void setCipPath(const QString& path);
  void setPlcTypeIndex(const int index);
  void setPollInterval(const int interval);
  void setTags(const QJsonArray& tags);
  void addTag(const QString& name, const QString& tag, const QString& type, const int element);
  void removeTag(const int index);
  void clearTags();

private slots:
  void onFrameReady(const QByteArray& frame, qint64 stampNs);
  void onLinkLost(const QString& reason);

private:
  void doClose();
  void loadSettings();
  void saveTags();
  [[nodiscard]] bool tagsFrozen() const noexcept;
  [[nodiscard]] const EthernetIp* sessionPeer() const;
  [[nodiscard]] static DataModel::Dataset datasetFor(const EipTag& tag, int index);

  bool m_open;
  bool m_persistent;
  int m_plcTypeIndex;
  int m_pollInterval;
  qint64 m_lastStampNs;
  quint64 m_linkDrops;
  QString m_host;
  QString m_cipPath;
  QString m_lastError;
  std::unique_ptr<QThread> m_thread;
  EipPollWorker* m_worker;
  QPointer<EthernetIp> m_sessionPeer;
  QVector<EipTag> m_tags;
  QSettings m_settings;
};

}  // namespace Drivers
}  // namespace IO
