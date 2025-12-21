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

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QCanDbcFileParser>
#include <QCanMessageDescription>
#include <QCanSignalDescription>

#include "Frame.h"
#include "SerialStudio.h"

namespace JSON
{
/**
 * @brief Imports CAN Database (DBC) files and converts them to Serial Studio
 *        projects.
 *
 * The DBCImporter class provides functionality to parse DBC files using Qt's
 * QCanDbcFileParser and generate Serial Studio project files (.ssproj)
 * from the CAN message and signal definitions.
 *
 * The import process follows a two-phase workflow:
 * 1. Preview Phase: The DBC file is parsed and a preview dialog shows the
 *    user a summary of messages and signals that will be imported.
 * 2. Confirmation Phase: Upon user confirmation, a project file is
 *    generated with appropriate groups, datasets, and a JavaScript
 *    frame parser.
 *
 * Key features:
 * - Uses Qt's QCanDbcFileParser for robust DBC parsing
 * - Automatically generates JavaScript frame parsers for CAN signal extraction
 * - Smart widget assignment based on signal units and value ranges
 * - Supports both Intel (little-endian) and Motorola (big-endian) byte order
 * - Handles signal scaling and offset transformations
 * - Creates structured projects with groups for each CAN message
 *
 * This class follows the singleton pattern and is registered with QML for UI
 * interaction.
 *
 * @note This is a Pro-only feature (commercial license required).
 */
class DBCImporter : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int signalCount READ signalCount NOTIFY messagesChanged)
  Q_PROPERTY(int messageCount READ messageCount NOTIFY messagesChanged)
  Q_PROPERTY(QString dbcFileName READ dbcFileName NOTIFY dbcFileNameChanged)

signals:
  void previewReady();
  void messagesChanged();
  void dbcFileNameChanged();

private:
  explicit DBCImporter();
  DBCImporter(DBCImporter &&) = delete;
  DBCImporter(const DBCImporter &) = delete;
  DBCImporter &operator=(DBCImporter &&) = delete;
  DBCImporter &operator=(const DBCImporter &) = delete;

public:
  static DBCImporter &instance();

  [[nodiscard]] int signalCount() const;
  [[nodiscard]] int messageCount() const;
  [[nodiscard]] QString dbcFileName() const;

  Q_INVOKABLE QString messageInfo(int index) const;

public slots:
  void importDBC();
  void cancelImport();
  void confirmImport();
  void showPreview(const QString &filePath);

private:
  std::vector<Group>
  generateGroups(const QList<QCanMessageDescription> &messages);
  QJsonObject generateProject(const QList<QCanMessageDescription> &messages);
  QString generateFrameParser(const QList<QCanMessageDescription> &messages);

  QString sanitizeJavaScriptString(const QString &str);
  QString selectGroupWidget(const QCanMessageDescription &message);
  QString selectWidgetForSignal(const QCanSignalDescription &signal);
  QString generateSignalExtraction(const QCanSignalDescription &signal);
  QString generateMessageDecoder(const QCanMessageDescription &message,
                                 int &datasetIndex);

  int countTotalSignals(const QList<QCanMessageDescription> &messages) const;

private:
  QString m_dbcFilePath;
  QList<QCanMessageDescription> m_messages;
};

} // namespace JSON
