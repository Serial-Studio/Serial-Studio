/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <QCanDbcFileParser>
#include <QCanMessageDescription>
#include <QCanSignalDescription>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>

#include "DataModel/Frame.h"

class SessionContext;

namespace DataModel {
/**
 * @brief Imports CAN Database (DBC) files and generates Serial Studio projects.
 */
class DBCImporter : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int signalCount
             READ signalCount
             NOTIFY messagesChanged)
  Q_PROPERTY(int messageCount
             READ messageCount
             NOTIFY messagesChanged)
  Q_PROPERTY(QString dbcFileName
             READ dbcFileName
             NOTIFY dbcFileNameChanged)
  // clang-format on

signals:
  void previewReady();
  void messagesChanged();
  void dbcFileNameChanged();

public:
  explicit DBCImporter(SessionContext& ctx);
  DBCImporter(DBCImporter&&)                 = delete;
  DBCImporter(const DBCImporter&)            = delete;
  DBCImporter& operator=(DBCImporter&&)      = delete;
  DBCImporter& operator=(const DBCImporter&) = delete;

public:
  [[nodiscard]] static DBCImporter& instance();

  [[nodiscard]] int signalCount() const;
  [[nodiscard]] int messageCount() const;
  [[nodiscard]] QString dbcFileName() const;

  [[nodiscard]] QJsonObject projectFromMessages(const QList<QCanMessageDescription>& messages);

  [[nodiscard]] Q_INVOKABLE QString messageInfo(int index) const;

public slots:
  void importDBC();
  void cancelImport();
  void confirmImport();
  void showPreview(const QString& filePath);

private:
  /**
   * @brief Multiplexing role of a CAN signal as classified during DBC import. Selector covers
   *        both the top-level MultiplexorSwitch and an SG_MUL_VAL_ switch that is itself gated;
   *        ExtendedMuxed is the sole drop verdict, reserved for switch ranges that do not fit a
   *        qint64.
   */
  enum class MuxRole {
    Plain,
    Muxed,
    Selector,
    ExtendedMuxed
  };

  /**
   * @brief One inclusive range of raw multiplexor values; a single value has lo == hi.
   */
  struct MuxRange {
    qint64 lo;
    qint64 hi;
  };

  /**
   * @brief One gating condition: the selector signal's name plus the ranges of it, sorted by
   *        lower bound, that switch the dependent signal into the frame.
   */
  struct MuxSpec {
    QString parent;
    QList<MuxRange> ranges;
  };

  /**
   * @brief One importable signal in decode order, carrying every condition that gates it. A
   *        nested SG_MUL_VAL_ switch is a Selector with a non-empty gate list.
   */
  struct OrderedSignal {
    MuxRole role;
    QList<MuxSpec> gates;
    QCanSignalDescription signal;
  };

  std::vector<Group> generateGroups(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] Dataset buildDatasetFromSignal(
    const OrderedSignal& entry,
    const QString& groupWidget,
    const QString& tableName,
    const QCanDbcFileParser::ValueDescriptions& valueLabels,
    int datasetIndex);
  void buildTableNames(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] QString tableNameFor(const QCanMessageDescription& message) const;
  [[nodiscard]] QList<OrderedSignal> orderedSignals(const QCanMessageDescription& message) const;
  [[nodiscard]] std::vector<TableDef> generateTables(const QList<QCanMessageDescription>& messages);

  QString generateLuaParser(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] QString generateMessageSpec(const QCanMessageDescription& message) const;
  [[nodiscard]] static QString muxSpecField(const OrderedSignal& entry,
                                            const QString& rootSelector);
  [[nodiscard]] static QString signalSpecLine(const OrderedSignal& entry,
                                              const QString& rootSelector);
  [[nodiscard]] static QString enumTransformCode(
    const QString& table,
    const QString& reg,
    const QCanDbcFileParser::ValueDescriptions& valueLabels);

  QString selectGroupWidget(const QCanMessageDescription& message);
  QString selectWidgetForSignal(const QCanSignalDescription& signal);

  [[nodiscard]] static bool isPlottableSignal(const QCanSignalDescription& signal);

  [[nodiscard]] static QString datasetTitle(const OrderedSignal& entry);
  [[nodiscard]] static QString muxTitleSuffix(const QList<MuxSpec>& gates);
  [[nodiscard]] static QString rootSelectorName(const QList<OrderedSignal>& entries);
  [[nodiscard]] static bool gatesResolved(const QList<MuxSpec>& gates,
                                          const QSet<QString>& resolved);
  static void appendResolved(const OrderedSignal& entry,
                             QList<OrderedSignal>& ordered,
                             QList<OrderedSignal>& pending,
                             QSet<QString>& resolved);
  [[nodiscard]] static bool simpleMuxValue(const QList<MuxSpec>& gates,
                                           const QString& rootSelector,
                                           qint64& outValue);
  [[nodiscard]] static bool buildMuxGates(const QCanSignalDescription& signal,
                                          QList<MuxSpec>& outGates);
  [[nodiscard]] static MuxRole classifyMux(const QCanSignalDescription& signal,
                                           QList<MuxSpec>& outGates);
  [[nodiscard]] bool hasImportableSignals(const QCanMessageDescription& message) const;

  [[nodiscard]] QString detectGpsWidget(
    const QList<QCanSignalDescription>& signalDescriptions) const;
  [[nodiscard]] QString detectMotionWidget(
    const QList<QCanSignalDescription>& signalDescriptions) const;

  [[nodiscard]] int countTotalSignals(const QList<QCanMessageDescription>& messages) const;

private:
  SessionContext& m_ctx;
  QString m_dbcFilePath;
  QHash<quint32, QString> m_tableNames;
  QList<QCanMessageDescription> m_messages;
  QCanDbcFileParser::MessageValueDescriptions m_valueDescriptions;
  int m_skippedExtendedMuxSignals;
};

}  // namespace DataModel
