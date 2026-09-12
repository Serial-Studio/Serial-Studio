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

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "Core/DataModel/Frame.h"
#include "DataModel/Importers/ModbusRegisterMap.h"

namespace DataModel {

/**
 * @brief Imports Modbus register maps (CSV, XML, JSON) and generates a project.
 */
class ModbusMapImporter : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int registerCount
             READ registerCount
             NOTIFY registersChanged)
  Q_PROPERTY(int groupCount
             READ groupCount
             NOTIFY registersChanged)
  Q_PROPERTY(QString fileName
             READ fileName
             NOTIFY fileNameChanged)
  // clang-format on

signals:
  void previewReady();
  void registersChanged();
  void fileNameChanged();

private:
  explicit ModbusMapImporter();

public:
  static ModbusMapImporter& instance();

  ModbusMapImporter(ModbusMapImporter&&)                 = delete;
  ModbusMapImporter(const ModbusMapImporter&)            = delete;
  ModbusMapImporter& operator=(ModbusMapImporter&&)      = delete;
  ModbusMapImporter& operator=(const ModbusMapImporter&) = delete;

  [[nodiscard]] int registerCount() const noexcept;
  [[nodiscard]] int groupCount() const noexcept;
  [[nodiscard]] QString fileName() const;
  [[nodiscard]] Q_INVOKABLE QString registerInfo(int index) const;

public slots:
  void importRegisterMap();
  void confirmImport();
  void confirmMerge();
  void cancelImport();

public:
  /**
   * @brief Contiguous run of registers of the same type, used by the project generator.
   */
  struct RegisterBlock {
    quint8 registerType;
    quint16 startAddress;
    quint16 count;
    quint8 slaveAddress;
    QVector<ModbusMap::RegisterEntry> entries;
  };

private:
  void showPreview(const QString& filePath);
  void loadRegisterGroups(const QVector<RegisterBlock>& blocks, bool append) const;

  [[nodiscard]] QVector<RegisterBlock> computeBlocks() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] Group buildOutputGroup(const QVector<RegisterBlock>& blocks,
                                       const QStringList& tableNames,
                                       const QList<QStringList>& registerNames,
                                       int groupIndex) const;
  [[nodiscard]] static OutputWidget buildControl(const RegisterBlock& block,
                                                 const ModbusMap::RegisterEntry& entry,
                                                 const QString& tableName,
                                                 const QString& regName,
                                                 int widgetId);
  [[nodiscard]] static QString controlTransmitScript(const RegisterBlock& block,
                                                     const ModbusMap::RegisterEntry& entry,
                                                     const QString& tableName,
                                                     const QString& regName);
  [[nodiscard]] Dataset buildDatasetFromEntry(const ModbusMap::RegisterEntry& entry,
                                              bool isBool,
                                              const QString& tableName,
                                              const QString& regName,
                                              int datasetIndex) const;

  [[nodiscard]] static QString blockTitle(const RegisterBlock& block,
                                          qsizetype blockCount,
                                          bool multiUnit);
  [[nodiscard]] static bool spansSeveralUnits(const QVector<RegisterBlock>& blocks);
  [[nodiscard]] static QStringList blockRegisterNames(const RegisterBlock& block);
  [[nodiscard]] static QString luaEntryType(const ModbusMap::RegisterEntry& entry, bool bitBlock);
  [[nodiscard]] static QString luaEntryLine(const RegisterBlock& block,
                                            const QString& registerName,
                                            const QString& tableName,
                                            qsizetype index);
  [[nodiscard]] static QString rawWordRegisterName(const QString& registerName);
  [[nodiscard]] QString buildLuaParser(const QVector<RegisterBlock>& blocks,
                                       const QStringList& tableNames,
                                       const QList<QStringList>& registerNames) const;

  [[nodiscard]] static QString registerTypeName(quint8 type);

  [[nodiscard]] static QString selectDatasetWidget(const ModbusMap::RegisterEntry& entry);

  QString m_filePath;
  QVector<ModbusMap::RegisterEntry> m_registers;
};
}  // namespace DataModel
