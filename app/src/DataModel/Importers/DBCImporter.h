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
#include "DataModel/Importers/DBCMultiplexing.h"

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
  std::vector<Group> generateGroups(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] Dataset buildDatasetFromSignal(
    const DBCMux::OrderedSignal& entry,
    const QString& groupWidget,
    const QString& tableName,
    const QCanDbcFileParser::ValueDescriptions& valueLabels,
    int datasetIndex);
  void buildTableNames(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] QString tableNameFor(const QCanMessageDescription& message) const;
  [[nodiscard]] std::vector<TableDef> generateTables(const QList<QCanMessageDescription>& messages);

  QString generateLuaParser(const QList<QCanMessageDescription>& messages);
  [[nodiscard]] QString generateMessageSpec(const QCanMessageDescription& message) const;
  [[nodiscard]] static QString muxSpecField(const DBCMux::OrderedSignal& entry,
                                            const QString& rootSelector);
  [[nodiscard]] static QString signalSpecLine(const DBCMux::OrderedSignal& entry,
                                              const QString& rootSelector);
  [[nodiscard]] static QString enumTransformCode(
    const QString& table,
    const QString& reg,
    const QCanDbcFileParser::ValueDescriptions& valueLabels);

  QString selectGroupWidget(const QCanMessageDescription& message);
  QString selectWidgetForSignal(const QCanSignalDescription& signal);

  [[nodiscard]] static bool isPlottableSignal(const QCanSignalDescription& signal);

  [[nodiscard]] static QString datasetTitle(const DBCMux::OrderedSignal& entry);
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
