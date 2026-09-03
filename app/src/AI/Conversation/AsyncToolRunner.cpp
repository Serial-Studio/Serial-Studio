/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation/AsyncToolRunner.h"

#include "AI/Tools/ToolFilesystemTools.h"

//--------------------------------------------------------------------------------------------------
// Construction & destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates the runner with a single worker, so two scans queue instead of competing for
 *        the disk and the sandbox never sees concurrent walks of the same roots.
 */
AI::AsyncToolRunner::AsyncToolRunner(QObject* parent) : QObject(parent)
{
  m_pool.setMaxThreadCount(1);
}

/**
 * @brief Waits for the running scan before the runner dies. The pool is a member for exactly
 *        this reason: a task still walking the workspace must not outlive the emitter it posts
 *        its result through.
 */
AI::AsyncToolRunner::~AsyncToolRunner()
{
  m_pool.waitForDone();
}

//--------------------------------------------------------------------------------------------------
// Dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether this tool runs on the worker lane. The two read-only sandbox primitives qualify;
 *        every writing tool stays inline so its effect is ordered against the rest of the turn.
 */
bool AI::AsyncToolRunner::handles(const QString& name)
{
  return name == QStringLiteral("fs.read") || name == QStringLiteral("fs.search");
}

/**
 * @brief Queues one tool call and reports its result on the caller's thread. The generation is
 *        echoed back untouched so the conversation can discard a result whose turn has moved on.
 */
void AI::AsyncToolRunner::run(const QString& callId,
                              const QString& name,
                              const QJsonObject& arguments,
                              quint64 generation)
{
  m_pool.start([this, callId, name, arguments, generation] {
    const auto reply = ToolDetail::executeFsTool(name, arguments);
    QMetaObject::invokeMethod(
      this,
      [this, callId, name, arguments, reply, generation] {
        Q_EMIT toolFinished(callId, name, arguments, reply, generation);
      },
      Qt::QueuedConnection);
  });
}
