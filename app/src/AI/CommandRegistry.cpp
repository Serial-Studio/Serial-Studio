/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/CommandRegistry.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "AI/Logging.h"
#include "Misc/JsonValidator.h"

//--------------------------------------------------------------------------------------------------
// Singleton accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide AI command registry.
 */
AI::CommandRegistry& AI::CommandRegistry::instance()
{
  static CommandRegistry singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Construction and load
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the safety map at first use.
 */
AI::CommandRegistry::CommandRegistry()
{
  load();
}

/**
 * @brief Reads the safety JSON resource into the tag map.
 */
void AI::CommandRegistry::load()
{
  QFile file(QStringLiteral(":/ai/command_safety.json"));
  if (!file.open(QIODevice::ReadOnly)) {
    qCWarning(serialStudioAI) << "command_safety.json could not be opened; "
                                 "all commands fall through to Confirm.";
    return;
  }

  const auto data = file.readAll();
  file.close();

  Misc::JsonValidator::Limits limits;
  limits.maxFileSize  = 1 * 1024 * 1024;
  limits.maxDepth     = 8;
  limits.maxArraySize = 1000;

  const auto result = Misc::JsonValidator::parseAndValidate(data, limits);
  if (!result.valid || !result.document.isObject()) {
    qCWarning(serialStudioAI) << "command_safety.json invalid:" << result.errorMessage;
    return;
  }

  const auto root          = result.document.object();
  const auto safe          = root.value(QStringLiteral("safe")).toArray();
  const auto blocked       = root.value(QStringLiteral("blocked")).toArray();
  const auto alwaysConfirm = root.value(QStringLiteral("alwaysConfirm")).toArray();

  for (const auto& entry : safe)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::Safe);

  for (const auto& entry : blocked)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::Blocked);

  for (const auto& entry : alwaysConfirm)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::AlwaysConfirm);

  qCDebug(serialStudioAI) << "Loaded AI command safety tags:" << safe.size() << "safe,"
                          << blocked.size() << "blocked," << alwaysConfirm.size()
                          << "alwaysConfirm.";
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the safety classification for a command name.
 */
AI::Safety AI::CommandRegistry::safetyOf(const QString& commandName) const
{
  return m_tags.value(commandName, Safety::Confirm);
}

/**
 * @brief Returns the names of all explicitly Safe commands.
 */
QStringList AI::CommandRegistry::safeNames() const
{
  QStringList result;
  for (auto it = m_tags.constBegin(); it != m_tags.constEnd(); ++it)
    if (it.value() == Safety::Safe)
      result.append(it.key());

  return result;
}

/**
 * @brief Returns the names of all explicitly Blocked commands.
 */
QStringList AI::CommandRegistry::blockedNames() const
{
  QStringList result;
  for (auto it = m_tags.constBegin(); it != m_tags.constEnd(); ++it)
    if (it.value() == Safety::Blocked)
      result.append(it.key());

  return result;
}
