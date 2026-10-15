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

#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QStringList>

namespace AI {

/**
 * @brief Action-safety classification for AI tool dispatch.
 */
enum class Safety : quint8 {
  Safe          = 0,
  Confirm       = 1,
  Blocked       = 2,
  AlwaysConfirm = 3,
};

/**
 * @brief Tag map for AI tool execution.
 */
class CommandRegistry {
public:
  [[nodiscard]] static CommandRegistry& instance();

  [[nodiscard]] Safety safetyOf(const QString& commandName) const;
  [[nodiscard]] bool deviceControlAllowed() const;
  [[nodiscard]] bool isDeviceGated(const QString& commandName) const;
  [[nodiscard]] Safety argumentAwareSafety(const QString& commandName,
                                           const QJsonObject& args) const;
  [[nodiscard]] Safety effectiveSafety(const QString& commandName, const QJsonObject& args) const;

  [[nodiscard]] static int strictness(Safety safety) noexcept;
  [[nodiscard]] static bool isDispatcher(const QString& commandName);
  [[nodiscard]] static QStringList plannedCommands(const QString& commandName,
                                                   const QJsonObject& args);

  void setDeviceControlAllowed(bool allowed);

private:
  CommandRegistry();
  CommandRegistry(CommandRegistry&&)                 = delete;
  CommandRegistry(const CommandRegistry&)            = delete;
  CommandRegistry& operator=(CommandRegistry&&)      = delete;
  CommandRegistry& operator=(const CommandRegistry&) = delete;

  void load();

  bool m_deviceControlAllowed;
  QSet<QString> m_deviceGated;
  QHash<QString, Safety> m_tags;
};

}  // namespace AI
