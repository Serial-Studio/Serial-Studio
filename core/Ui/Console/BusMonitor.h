/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <array>
#include <chrono>
#include <deque>
#include <QAbstractTableModel>
#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QVariantList>
#include <QVariantMap>
#include <vector>

#include "Console/ModbusRtuAnalyzer.h"

namespace Console {

/**
 * @brief Bounds of the bus monitor's message store and its Messages mode (spec 0127 R11/R17).
 */
inline constexpr int kBusMonitorMaxMessages     = 5000;
inline constexpr int kBusMonitorMaxMessageBytes = 4096;
inline constexpr int kBusMonitorGapMinMs        = 1;
inline constexpr int kBusMonitorGapMaxMs        = 1000;
inline constexpr int kBusMonitorGapDefaultMs    = 10;
inline constexpr int kBusMonitorMaxBars         = 2000;

/**
 * @brief The console's bus monitor (spec 0127 R10-R17): a bounded log of the messages on the
 *        link with their time, role and findings, shown as a table and as a timeline. In
 *        Messages mode a send is one message and received bytes are split by an idle gap; in
 *        Modbus RTU mode everything on the line goes through ModbusRtuAnalyzer. It ingests
 *        nothing without a view on screen, stages messages per chunk and publishes them once per
 *        UI tick. GUI thread only.
 */
class BusMonitor : public QAbstractTableModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int mode
             READ mode
             WRITE setMode
             NOTIFY settingsChanged)
  Q_PROPERTY(bool paused
             READ paused
             WRITE setPaused
             NOTIFY settingsChanged)
  Q_PROPERTY(int timeoutMs
             READ timeoutMs
             WRITE setTimeoutMs
             NOTIFY settingsChanged)
  Q_PROPERTY(int gapMs
             READ gapMs
             WRITE setGapMs
             NOTIFY settingsChanged)
  Q_PROPERTY(int formats
             READ formats
             WRITE setFormats
             NOTIFY settingsChanged)
  Q_PROPERTY(bool issuesOnly
             READ issuesOnly
             WRITE setIssuesOnly
             NOTIFY settingsChanged)
  Q_PROPERTY(int count
             READ count
             NOTIFY countChanged)
  Q_PROPERTY(qint64 lastTimeMs
             READ lastTimeMs
             NOTIFY countChanged)
  Q_PROPERTY(qreal averageIntervalMs
             READ averageIntervalMs
             NOTIFY countChanged)
  Q_PROPERTY(int frameTotal
             READ frameTotal
             NOTIFY countChanged)
  Q_PROPERTY(int errorTotal
             READ errorTotal
             NOTIFY countChanged)
  Q_PROPERTY(int warningTotal
             READ warningTotal
             NOTIFY countChanged)
  // clang-format on

signals:
  void countChanged();
  void settingsChanged();

public:
  enum Mode {
    Messages = 0,
    ModbusRtu,
  };
  Q_ENUM(Mode)

  enum Column {
    TimeColumn = 0,
    RoleColumn,
    SlaveColumn,
    FunctionColumn,
    LengthColumn,
    SummaryColumn,
    CrcColumn,
    DiagnosisColumn,
    RawColumn,
    ColumnCount
  };
  Q_ENUM(Column)

  enum Roles {
    SeverityRole = Qt::UserRole + 1,
    BusRoleRole,
  };

  using TimePoint = std::chrono::steady_clock::time_point;

  explicit BusMonitor(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QVariant headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] int mode() const noexcept;
  [[nodiscard]] int gapMs() const noexcept;
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int formats() const noexcept;
  [[nodiscard]] bool paused() const noexcept;
  [[nodiscard]] int timeoutMs() const noexcept;
  [[nodiscard]] bool issuesOnly() const noexcept;
  [[nodiscard]] int frameTotal() const noexcept;
  [[nodiscard]] int errorTotal() const noexcept;
  [[nodiscard]] int warningTotal() const noexcept;
  [[nodiscard]] bool watched() const noexcept;
  [[nodiscard]] qint64 lastTimeMs() const noexcept;
  [[nodiscard]] qreal averageIntervalMs() const noexcept;
  [[nodiscard]] const BusFrame& at(int row) const;
  [[nodiscard]] qint64 toMs(TimePoint time) const noexcept;

  [[nodiscard]] Q_INVOKABLE QVariantList timelineBars(qint64 fromMs,
                                                      qint64 toMs,
                                                      qreal pixels) const;
  [[nodiscard]] Q_INVOKABLE QVariantMap messageInfo(int row) const;

  void commitPending(qint64 nowMs);
  void feedSent(const QByteArray& bytes, qint64 nowMs);
  void feedReceived(const QByteArray& bytes, qint64 nowMs);

public slots:
  void clear();
  void setMode(int mode);
  void setGapMs(int gapMs);
  void setFormats(int formats);
  void setPaused(bool paused);
  void setTimeoutMs(int timeoutMs);
  void setIssuesOnly(bool issuesOnly);
  void setViewerActive(QObject* viewer, bool active);

private:
  /**
   * @brief Which row a request reported by the analyzer landed on. A finding can arrive for a
   *        request several frames back (a burst of unanswered requests in one chunk), so the
   *        last few are remembered, not just the newest.
   */
  struct RequestRow {
    quint64 analyzerSequence = 0;
    quint64 rowSequence      = 0;
  };

  static constexpr int kRequestMemory = 16;

  void resetIngest();
  void flushPacket(qint64 nowMs);
  void stage(const BusFrame& frame);
  void applyUpdate(const BusUpdate& update);
  void tally(BusSeverity before, BusSeverity after);
  void stageAnalyzed(qint64 nowMs);
  [[nodiscard]] int rowOfSequence(quint64 sequence) const noexcept;
  [[nodiscard]] QString cellText(const BusFrame& frame, int column) const;

private:
  QSettings m_settings;
  ModbusRtuAnalyzer m_analyzer;
  std::deque<BusFrame> m_rows;
  std::vector<BusFrame> m_pending;
  std::vector<BusFrame> m_frameScratch;
  std::vector<BusUpdate> m_updateScratch;
  QHash<QObject*, QMetaObject::Connection> m_viewers;
  QByteArray m_packet;
  TimePoint m_epoch;
  qint64 m_dayOffsetMs;
  quint64 m_nextSequence;
  std::array<RequestRow, kRequestMemory> m_requests;
  int m_requestCursor;
  qint64 m_packetStartMs;
  qint64 m_packetLastMs;
  int m_mode;
  int m_gapMs;
  int m_formats;
  int m_frameTotal;
  int m_errorTotal;
  int m_warningTotal;
  bool m_paused;
  bool m_issuesOnly;
  bool m_countDirty;
};

/**
 * @brief The table view's filter over a BusMonitor: with "issues only" on it keeps the rows that
 *        carry a warning or an error.
 */
class BusMonitorFilter : public QSortFilterProxyModel {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit BusMonitorFilter(BusMonitor* monitor, QObject* parent = nullptr);

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex& parent) const override;

private slots:
  void syncIssuesOnly();

private:
  BusMonitor* m_monitor;
  bool m_issuesOnly;
};

}  // namespace Console
