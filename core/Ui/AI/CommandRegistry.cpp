/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/CommandRegistry.h"

#include <algorithm>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "AI/Logging.h"
#include "Core/JsonValidator.h"

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
AI::CommandRegistry::CommandRegistry() : m_deviceControlAllowed(false)
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
  const auto confirm       = root.value(QStringLiteral("confirm")).toArray();
  const auto blocked       = root.value(QStringLiteral("blocked")).toArray();
  const auto deviceGated   = root.value(QStringLiteral("deviceGated")).toArray();
  const auto alwaysConfirm = root.value(QStringLiteral("alwaysConfirm")).toArray();

  for (const auto& entry : safe)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::Safe);

  for (const auto& entry : confirm)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::Confirm);

  for (const auto& entry : blocked)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::Blocked);

  for (const auto& entry : deviceGated)
    if (entry.isString())
      m_deviceGated.insert(entry.toString());

  for (const auto& entry : alwaysConfirm)
    if (entry.isString())
      m_tags.insert(entry.toString(), Safety::AlwaysConfirm);

  qCDebug(serialStudioAI) << "Loaded AI command safety tags:" << safe.size() << "safe,"
                          << confirm.size() << "confirm," << blocked.size() << "blocked,"
                          << m_deviceGated.size() << "deviceGated," << alwaysConfirm.size()
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
  if (m_deviceGated.contains(commandName))
    return m_deviceControlAllowed ? Safety::AlwaysConfirm : Safety::Blocked;

  return m_tags.value(commandName, Safety::Confirm);
}

/**
 * @brief Returns whether device-gated commands are currently unblocked.
 */
bool AI::CommandRegistry::deviceControlAllowed() const
{
  return m_deviceControlAllowed;
}

/**
 * @brief Returns whether a command belongs to the device-gated set.
 */
bool AI::CommandRegistry::isDeviceGated(const QString& commandName) const
{
  return m_deviceGated.contains(commandName);
}

/**
 * @brief The tier a call actually needs (spec 0098 R9): the strictest of its own and of every
 *        command it will run. A planned command that itself dispatches others counts as Blocked,
 *        so nothing nests past one level of batching.
 */
AI::Safety AI::CommandRegistry::effectiveSafety(const QString& commandName,
                                                const QJsonObject& args) const
{
  auto tier = safetyOf(commandName);
  for (const auto& planned : plannedCommands(commandName, args)) {
    const auto inner = isDispatcher(planned) ? Safety::Blocked : safetyOf(planned);
    if (strictness(inner) > strictness(tier))
      tier = inner;
  }

  return tier;
}

/**
 * @brief Orders tiers by how much they demand of the user; the enum's values are wire ids and
 *        do not sort that way (Blocked is 2, AlwaysConfirm 3).
 */
int AI::CommandRegistry::strictness(const Safety safety) noexcept
{
  switch (safety) {
    case Safety::Safe:
      return 0;
    case Safety::Confirm:
      return 1;
    case Safety::AlwaysConfirm:
      return 2;
    case Safety::Blocked:
      return 3;
  }

  return 3;
}

/**
 * @brief Whether a command runs other commands on the caller's behalf.
 */
bool AI::CommandRegistry::isDispatcher(const QString& commandName)
{
  return commandName == QStringLiteral("project.batch")
      || commandName == QStringLiteral("assistant.project.bulkApply");
}

/**
 * @brief The commands a call will run besides itself, read from its arguments: a batch's ops, the
 *        setter a script apply forwards to, and the fixed fan-out of a tile add. Bounded by the
 *        batch limit the API enforces.
 */
QStringList AI::CommandRegistry::plannedCommands(const QString& commandName,
                                                 const QJsonObject& args)
{
  constexpr int kMaxPlannedOps = 1024;

  QStringList planned;
  if (isDispatcher(commandName)) {
    const auto ops   = args.value(QStringLiteral("ops")).toArray();
    const auto count = std::min<qsizetype>(ops.size(), kMaxPlannedOps);
    for (qsizetype i = 0; i < count; ++i)
      planned.append(ops.at(i).toObject().value(QStringLiteral("command")).toString());

    return planned;
  }

  if (commandName == QStringLiteral("assistant.script.apply")) {
    static const QHash<QString, QString> kSetterByKind = {
      {QStringLiteral("frame_parser"),      QStringLiteral("project.frameParser.setCode")},
      {   QStringLiteral("transform"), QStringLiteral("project.dataset.setTransformCode")},
      {     QStringLiteral("painter"),          QStringLiteral("project.painter.setCode")},
    };
    const auto setter = kSetterByKind.value(args.value(QStringLiteral("kind")).toString());
    if (!setter.isEmpty())
      planned.append(setter);

    return planned;
  }

  if (commandName == QStringLiteral("assistant.workspace.addTile"))
    planned = {
      QStringLiteral("project.workspace.add"),
      QStringLiteral("project.dataset.setOptions"),
      QStringLiteral("project.dataset.update"),
      QStringLiteral("project.workspace.setCustomizeMode"),
      QStringLiteral("project.workspace.addWidget"),
    };

  return planned;
}

/**
 * @brief Toggles the device-control gate driven by AI::Assistant.
 */
void AI::CommandRegistry::setDeviceControlAllowed(bool allowed)
{
  m_deviceControlAllowed = allowed;
}
