/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>

namespace AI {

/**
 * @brief Bridges AI tool calls to the in-process API::CommandRegistry.
 */
class ToolDispatcher : public QObject {
  Q_OBJECT

public:
  explicit ToolDispatcher(QObject* parent = nullptr);

  [[nodiscard]] QJsonArray availableTools(const QString& category = {}) const;
  [[nodiscard]] QJsonObject listCommands(const QString& prefix = {}) const;
  [[nodiscard]] QJsonObject listCategories() const;
  [[nodiscard]] QString canonicalToolName(const QString& name) const;
  [[nodiscard]] QJsonObject describeCommand(const QString& name) const;
  [[nodiscard]] QJsonObject executeCommand(const QString& name,
                                           const QJsonObject& args,
                                           bool autoConfirmSafe);
  [[nodiscard]] QJsonObject getProjectState() const;
  [[nodiscard]] QJsonObject getSnapshot() const;
  [[nodiscard]] QJsonObject getScriptingDocs(const QString& kind) const;

signals:
  void confirmationRequested(const QString& name, const QJsonObject& args);
};

}  // namespace AI
