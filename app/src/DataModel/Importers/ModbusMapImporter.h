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

#include "DataModel/Frame.h"
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
  void cancelImport();

public:
  /**
   * @brief Contiguous run of registers of the same type, used by the project generator.
   */
  struct RegisterBlock {
    quint8 registerType;
    quint16 startAddress;
    quint16 count;
    QVector<ModbusMap::RegisterEntry> entries;
  };

private:
  void showPreview(const QString& filePath);
  void loadRegisterGroups(const QVector<RegisterBlock>& blocks) const;

  [[nodiscard]] QVector<RegisterBlock> computeBlocks() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] Dataset buildDatasetFromEntry(const ModbusMap::RegisterEntry& entry,
                                              bool isBool,
                                              const QString& tableName,
                                              const QString& regName,
                                              int datasetIndex) const;

  [[nodiscard]] static QString blockTitle(const RegisterBlock& block, qsizetype blockCount);
  [[nodiscard]] static QStringList blockRegisterNames(const RegisterBlock& block);
  [[nodiscard]] static QString luaEntryType(const ModbusMap::RegisterEntry& entry, bool bitBlock);
  [[nodiscard]] QString buildLuaParser(const QVector<RegisterBlock>& blocks,
                                       const QStringList& tableNames,
                                       const QList<QStringList>& registerNames) const;

  [[nodiscard]] static QString registerTypeName(quint8 type);

  [[nodiscard]] static QString selectDatasetWidget(const ModbusMap::RegisterEntry& entry);

  QString m_filePath;
  QVector<ModbusMap::RegisterEntry> m_registers;
};
}  // namespace DataModel
