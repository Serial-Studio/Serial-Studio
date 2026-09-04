/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation/AutoVerifier.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

#include "AI/CommandRegistry.h"
#include "AI/Logging.h"
#include "AI/ToolDispatcher.h"
#include "DataModel/FrameKeys.h"

//--------------------------------------------------------------------------------------------------
// Construction / wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the verifier to the safety registry it asserts read-backs against; the
 *        dispatcher arrives later because the conversation is built before it exists.
 */
AI::AutoVerifier::AutoVerifier(const CommandRegistry& commands)
  : m_commands(commands), m_dispatcher(nullptr)
{}

/**
 * @brief Sets the tool dispatcher. The verifier does not take ownership.
 */
void AI::AutoVerifier::setDispatcher(ToolDispatcher* dispatcher)
{
  m_dispatcher = dispatcher;
}

//--------------------------------------------------------------------------------------------------
// Read-back policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps an apply-class mutation to its Safe-tier read-back check, mirroring the arg
 *        construction assistant.script.apply proved out; returns false when no map exists.
 */
static bool readBackCommandFor(const QString& name,
                               const QJsonObject& arguments,
                               QString& cmd,
                               QJsonObject& args)
{
  if (name == QStringLiteral("project.frameParser.setCode")) {
    cmd  = QStringLiteral("project.frameParser.dryCompile");
    args = arguments;
    return true;
  }

  if (name == QStringLiteral("project.dataset.setTransformCode")) {
    cmd  = QStringLiteral("project.dataset.transform.dryRun");
    args = arguments;
    if (!args.contains(QStringLiteral("values")))
      args[QStringLiteral("values")] = QJsonArray{0.0};

    return true;
  }

  if (name == QStringLiteral("project.painter.setCode")) {
    cmd  = QStringLiteral("project.painter.dryRun");
    args = arguments;
    return true;
  }

  if (name == QStringLiteral("project.outputWidget.update")
      && arguments.contains(QStringLiteral("transmitFunction"))) {
    cmd  = QStringLiteral("project.outputWidget.dryRun");
    args = QJsonObject{
      {QStringLiteral("code"), arguments.value(QStringLiteral("transmitFunction"))}
    };
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Verification after a successful apply-class mutation: runs the matching Safe-tier
 *        read-back (asserted, never Confirm-class) and returns a verification object for
 *        the tool result and card, or empty when not applicable.
 */
QJsonObject AI::AutoVerifier::verify(const QString& name,
                                     const QJsonObject& arguments,
                                     const QJsonObject& reply)
{
  if (!reply.value(QStringLiteral("ok")).toBool())
    return {};

  if (name == QStringLiteral("assistant.script.apply")) {
    QJsonObject v;
    v[QStringLiteral("ok")]     = true;
    v[QStringLiteral("method")] = QStringLiteral("internal dry-run");
    qCDebug(serialStudioAI) << "AutoVerify:" << name << "via internal dry-run ok= true";
    return v;
  }

  if (name == QStringLiteral("assistant.project.bulkApply")) {
    const int failures = reply.value(QStringLiteral("failureCount")).toInt();
    QJsonObject v;
    v[QStringLiteral("ok")]     = failures == 0;
    v[QStringLiteral("method")] = QStringLiteral("batch failure scan");
    if (failures > 0)
      v[QStringLiteral("detail")] = tr("%1 operation(s) failed").arg(failures);

    qCDebug(serialStudioAI) << "AutoVerify:" << name
                            << "via batch failure scan ok=" << (failures == 0);
    return v;
  }

  if (name == QStringLiteral("project.source.update"))
    return verifySourceUpdate(arguments);

  QString cmd;
  QJsonObject args;
  if (!readBackCommandFor(name, arguments, cmd, args))
    return {};

  const bool safe = m_commands.safetyOf(cmd) == Safety::Safe;
  if (!safe) {
    qCWarning(serialStudioAI) << "AutoVerify: refusing non-Safe read-back" << cmd;
    return {};
  }

  const auto check = m_dispatcher->executeCommand(cmd, args);
  bool check_ok    = check.value(QStringLiteral("ok")).toBool();
  const auto inner = check.value(QStringLiteral("result")).toObject();
  if (check_ok && inner.contains(QStringLiteral("ok")))
    check_ok = inner.value(QStringLiteral("ok")).toBool();

  QJsonObject v;
  v[QStringLiteral("ok")]     = check_ok;
  v[QStringLiteral("method")] = cmd;
  if (!check_ok)
    v[QStringLiteral("detail")] =
      QString::fromUtf8(QJsonDocument(check).toJson(QJsonDocument::Compact)).left(300);

  qCDebug(serialStudioAI) << "AutoVerify:" << name << "via" << cmd << "ok=" << check_ok;
  return v;
}

/**
 * @brief Read-back verification for project.source.update: fetches the Safe-tier source
 *        list and confirms every requested field actually round-tripped, because generic
 *        CRUD patches are where weak models misfire most (observed 2026-07-14).
 */
QJsonObject AI::AutoVerifier::verifySourceUpdate(const QJsonObject& arguments)
{
  QJsonObject v;
  v[QStringLiteral("method")] = QStringLiteral("project.source.list");

  const auto check =
    m_dispatcher->executeCommand(QStringLiteral("project.source.list"), QJsonObject());
  if (!check.value(QStringLiteral("ok")).toBool()) {
    v[QStringLiteral("ok")]     = false;
    v[QStringLiteral("detail")] = tr("Source list read-back failed");
    qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list "
                               "ok= false (list failed)";
    return v;
  }

  const int source_id = arguments.value(Keys::SourceId).toInt(-1);
  const auto rows =
    check.value(QStringLiteral("result")).toObject().value(QStringLiteral("sources")).toArray();

  QJsonObject row;
  for (const auto& r : rows) {
    const auto obj = r.toObject();
    if (obj.value(Keys::SourceId).toInt(-1) == source_id) {
      row = obj;
      break;
    }
  }

  if (row.isEmpty()) {
    v[QStringLiteral("ok")]     = false;
    v[QStringLiteral("detail")] = tr("Source %1 not found after update").arg(source_id);
    qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list "
                               "ok= false (source missing)";
    return v;
  }

  QStringList mismatched;
  for (auto it = arguments.constBegin(); it != arguments.constEnd(); ++it) {
    if (it.key() == Keys::SourceId || !row.contains(it.key()))
      continue;

    if (row.value(it.key()) != it.value())
      mismatched.append(it.key());
  }

  v[QStringLiteral("ok")] = mismatched.isEmpty();
  if (!mismatched.isEmpty())
    v[QStringLiteral("detail")] =
      tr("Fields did not round-trip: %1").arg(mismatched.join(QStringLiteral(", ")));

  qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list ok="
                          << mismatched.isEmpty();
  return v;
}
