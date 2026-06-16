/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

namespace DataModel {

/**
 * @brief Wire-format-aware classification of a Protocol Buffers scalar field type.
 */
enum class ProtoScalar {
  Double,
  Float,
  Int32,
  Int64,
  UInt32,
  UInt64,
  SInt32,
  SInt64,
  Fixed32,
  Fixed64,
  SFixed32,
  SFixed64,
  Bool,
  String,
  Bytes,
  MessageRef,
  EnumRef,
};

/**
 * @brief A single field declared inside a parsed `.proto` message.
 */
struct ProtoField {
  int tag            = 0;
  bool repeated      = false;
  ProtoScalar scalar = ProtoScalar::Int32;
  QString name;
  QString typeRef;
};

/**
 * @brief A message definition parsed from a `.proto` schema, possibly with nested messages.
 */
struct ProtoMessage {
  QString name;
  QString qualifiedName;
  QVector<ProtoField> fields;
  QVector<ProtoMessage> nested;
};

/**
 * @brief Imports `.proto` schema files and generates Serial Studio projects.
 */
class ProtoImporter : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int fieldCount
             READ fieldCount
             NOTIFY messagesChanged)
  Q_PROPERTY(int messageCount
             READ messageCount
             NOTIFY messagesChanged)
  Q_PROPERTY(QString protoFileName
             READ protoFileName
             NOTIFY fileNameChanged)
  // clang-format on

signals:
  void previewReady();
  void messagesChanged();
  void fileNameChanged();

private:
  explicit ProtoImporter();
  ProtoImporter(ProtoImporter&&)                 = delete;
  ProtoImporter(const ProtoImporter&)            = delete;
  ProtoImporter& operator=(ProtoImporter&&)      = delete;
  ProtoImporter& operator=(const ProtoImporter&) = delete;

public:
  [[nodiscard]] static ProtoImporter& instance();

  [[nodiscard]] int fieldCount() const noexcept;
  [[nodiscard]] int messageCount() const noexcept;
  [[nodiscard]] QString protoFileName() const;

  [[nodiscard]] Q_INVOKABLE QString messageInfo(int index) const;
  [[nodiscard]] Q_INVOKABLE QString fieldInfo(int messageIndex, int fieldIndex) const;

public slots:
  void importProto();
  void cancelImport();
  void confirmImport();
  void showPreview(const QString& filePath);

private:
  /**
   * @brief Per-occurrence dispatch metadata produced alongside groups during buildGroups.
   */
  struct DispatchRecord {
    QString varName;
    const ProtoMessage* msg;
    int baseDatasetIndex;
    bool isTopLevel;
    QVector<int> childRecordIndex;
  };

  [[nodiscard]] QJsonObject generateProject() const;
  [[nodiscard]] QString generateFrameParser(int totalDatasets,
                                            const QVector<DispatchRecord>& records) const;
  [[nodiscard]] QString frameParserDecoder() const;

  static void emitParserBanner(QString& code, const QString& sourceFile, int totalDatasets);
  static void emitDispatchTables(QString& code, const QVector<DispatchRecord>& records);
  static QString formatDispatchEntry(const ProtoField& field,
                                     const DispatchRecord& rec,
                                     const QVector<DispatchRecord>& records,
                                     int& datasetIdx,
                                     int& childCursor);
  static void emitTopLevelDispatchers(QString& code, const QVector<DispatchRecord>& records);
  static void emitScoreDispatcher(QString& code);
  static void emitParseEntry(QString& code);
  static void emitDecoderReaders(QString& code);
  static void emitVarintAndSignedReaders(QString& code);
  static void emitFixedAndFloatReaders(QString& code);
  static void emitStringAndSkipWireReaders(QString& code);
  static void emitDecoderParseMsg(QString& code);

  [[nodiscard]] const ProtoMessage* findMessage(const QString& typeRef) const;
  [[nodiscard]] const ProtoMessage* messageAt(int index) const;
  [[nodiscard]] int countScalarFields(const ProtoMessage& message) const;
  [[nodiscard]] int countFieldsRecursive(const ProtoMessage& message) const;
  [[nodiscard]] int countFieldsRecursive(const ProtoMessage& message,
                                         QSet<const ProtoMessage*>& visited) const;

  void buildGroups(const ProtoMessage& message,
                   const QString& parentFieldName,
                   int sourceId,
                   int& groupIdCounter,
                   int& datasetIndexCounter,
                   QJsonArray& groupsOut,
                   QVector<DispatchRecord>& dispatchOut,
                   QSet<const ProtoMessage*>& visited) const;

  [[nodiscard]] QString selectGroupWidget(const ProtoMessage& message) const;

private:
  QString m_protoFilePath;
  QVector<ProtoMessage> m_messages;
};

}  // namespace DataModel
