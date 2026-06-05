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
#include <QJsonObject>
#include <QObject>
#include <QString>

#include "DataModel/Frame.h"

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

private:
  explicit DBCImporter();
  DBCImporter(DBCImporter&&)                 = delete;
  DBCImporter(const DBCImporter&)            = delete;
  DBCImporter& operator=(DBCImporter&&)      = delete;
  DBCImporter& operator=(const DBCImporter&) = delete;

public:
  [[nodiscard]] static DBCImporter& instance();

  [[nodiscard]] int signalCount() const;
  [[nodiscard]] int messageCount() const;
  [[nodiscard]] QString dbcFileName() const;

  [[nodiscard]] Q_INVOKABLE QString messageInfo(int index) const;

public slots:
  void importDBC();
  void cancelImport();
  void confirmImport();
  void showPreview(const QString& filePath);

private:
  enum SignalFamily {
    WheelSpeeds,
    TirePressures,
    Temperatures,
    Voltages,
    BatteryCluster,
    StatusFlags,
    GenericRelated,
    None
  };

  /**
   * @brief Multiplexing role of a CAN signal as classified during DBC import.
   */
  enum class MuxRole {
    Plain,
    Selector,
    SimpleMuxed,
    ExtendedMuxed
  };

  std::vector<Group> generateGroups(const QList<QCanMessageDescription>& messages);
  Dataset buildDatasetFromSignal(const QCanSignalDescription& signal,
                                 const QString& groupWidget,
                                 int datasetIndex,
                                 MuxRole role,
                                 qint64 muxValue);
  QJsonObject generateProject(const QList<QCanMessageDescription>& messages);
  QJsonObject generateNativeParserParams(const QList<QCanMessageDescription>& messages);

  [[nodiscard]] static QJsonObject signalToJson(const QCanSignalDescription& signal,
                                                MuxRole role,
                                                qint64 muxValue);

  QString selectGroupWidget(const QCanMessageDescription& message);
  QString selectWidgetForSignal(const QCanSignalDescription& signal);

  [[nodiscard]] static bool isIntegerSignal(const QCanSignalDescription& signal);

  [[nodiscard]] MuxRole classifyMux(const QCanSignalDescription& signal,
                                    const QCanMessageDescription& message,
                                    qint64& outMuxValue) const;
  [[nodiscard]] int findSelectorIndex(const QCanMessageDescription& message) const;
  [[nodiscard]] bool hasImportableSignals(const QCanMessageDescription& message) const;

  [[nodiscard]] QString detectGpsWidget(
    const QList<QCanSignalDescription>& signalDescriptions) const;
  [[nodiscard]] QString detectMotionWidget(
    const QList<QCanSignalDescription>& signalDescriptions) const;
  [[nodiscard]] QString resolveSignalFamilyWidget(
    SignalFamily family,
    int signalCount,
    const QList<QCanSignalDescription>& signalDescriptions) const;

  [[nodiscard]] SignalFamily detectSignalFamily(
    const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] bool hasPositionalPattern(const QList<QCanSignalDescription>& signalList,
                                          const QStringList& positions) const;
  [[nodiscard]] bool hasNumberedPattern(const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] bool allSimilarUnits(const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] bool hasBatterySignals(const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] bool allStatusSignals(const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] int countPlottable(const QList<QCanSignalDescription>& signalList) const;
  [[nodiscard]] bool isCriticalSignal(const QCanSignalDescription& signal) const;
  [[nodiscard]] bool shouldAssignIndividualWidget(const QString& groupWidget,
                                                  const QCanSignalDescription& signal,
                                                  bool isSingleBit) const;

  [[nodiscard]] int countTotalSignals(const QList<QCanMessageDescription>& messages) const;

private:
  QString m_dbcFilePath;
  QList<QCanMessageDescription> m_messages;
  int m_skippedExtendedMuxSignals;
};

}  // namespace DataModel
