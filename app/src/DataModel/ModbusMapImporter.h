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
#include <QVector>

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
   * @brief One Modbus register parsed from a CSV/XML/JSON map.
   */
  struct RegisterEntry {
    quint16 address;
    QString name;
    quint8 registerType;
    QString dataType;
    QString units;
    double min;
    double max;
    double scale;
    double offset;
  };

  /**
   * @brief Contiguous run of registers of the same type, used by the project generator.
   */
  struct RegisterBlock {
    quint8 registerType;
    quint16 startAddress;
    quint16 count;
    QVector<RegisterEntry> entries;
  };

  [[nodiscard]] static quint8 parseRegisterType(const QString& str);

private:
  void showPreview(const QString& filePath);
  void loadRegisterGroups(const QVector<RegisterBlock>& blocks) const;

  [[nodiscard]] bool parseCSV(const QString& path);
  [[nodiscard]] bool parseXML(const QString& path);
  [[nodiscard]] bool parseJSON(const QString& path);

  [[nodiscard]] QVector<RegisterBlock> computeBlocks() const;
  [[nodiscard]] QJsonObject buildProject() const;
  [[nodiscard]] QString buildFrameParser(const QVector<RegisterBlock>& blocks) const;

  [[nodiscard]] static int registersForDataType(const QString& dataType);
  [[nodiscard]] static QString registerTypeName(quint8 type);
  [[nodiscard]] static bool parseRegisterEntry(const QJsonObject& obj,
                                               RegisterEntry& entry,
                                               int defaultType = -1);

  [[nodiscard]] static QString selectDatasetWidget(const RegisterEntry& entry);
  [[nodiscard]] static QString emitParserEntry(const RegisterEntry& entry,
                                               const RegisterBlock& block,
                                               int datasetIndex);

  QString m_filePath;
  QVector<RegisterEntry> m_registers;
};
}  // namespace DataModel
