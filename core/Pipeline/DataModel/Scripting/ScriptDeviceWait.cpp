/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/ScriptDeviceWait.h"

#include <QThread>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/ControlScriptWorker.h"
#include "DataModel/Scripting/JsWatchdog.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kReplyPollSliceMs = 5;
static constexpr int kMaxReplyWaitMs   = 30000;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Coerces a JS string or byte array into raw bytes; anything else yields empty bytes.
 */
QByteArray DataModel::ScriptDeviceWait::payloadFromJsValue(const QJSValue& data)
{
  QByteArray payload;
  if (data.isString()) {
    payload = data.toString().toUtf8();
    return payload;
  }

  if (!data.isArray())
    return payload;

  const int len = data.property(QStringLiteral("length")).toInt();
  payload.reserve(len);
  for (int i = 0; i < len; ++i)
    payload.append(static_cast<char>(data.property(static_cast<quint32>(i)).toInt() & 0xff));

  return payload;
}

/**
 * @brief Builds the reply-acceptance test from the JS `until` argument: a terminator string, a
 *        byte count, or (undefined) the first non-empty reply.
 */
[[nodiscard]] static std::function<bool(const QByteArray&)> replyAcceptor(const QJSValue& until)
{
  if (until.isString()) {
    const QByteArray terminator = until.toString().toUtf8();
    if (!terminator.isEmpty())
      return [terminator](const QByteArray& reply) {
        return reply.contains(terminator);
      };
  }

  if (until.isNumber()) {
    const int expectedLen = until.toInt();
    if (expectedLen > 0)
      return [expectedLen](const QByteArray& reply) {
        return reply.size() >= expectedLen;
      };
  }

  return [](const QByteArray& reply) {
    return !reply.isEmpty();
  };
}

//--------------------------------------------------------------------------------------------------
// Write + wait
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p data to @p sourceId, then polls the reply capture buffer through the GUI-thread
 *        marshaller until @p until is satisfied, @p timeoutMs elapses or @p aborted fires.
 */
QVariantMap DataModel::ScriptDeviceWait::writeAndWait(ControlApiMarshaller* marshaller,
                                                      JsWatchdog* watchdog,
                                                      const AbortPredicate& aborted,
                                                      const QString& abortMessage,
                                                      const QJSValue& data,
                                                      int timeoutMs,
                                                      const QJSValue& until,
                                                      int sourceId)
{
  SS_ASSERT_LOG(static_cast<bool>(aborted));

  QVariantMap out;
  out.insert(QStringLiteral("ok"), false);
  out.insert(QStringLiteral("data"), QString());
  out.insert(QStringLiteral("bytesRead"), 0);
  out.insert(QStringLiteral("timedOut"), false);

  SS_ASSERT(marshaller != nullptr, {
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: no marshaller"));
    return out;
  });

  const auto isAborted = [&aborted] {
    return aborted && aborted();
  };
  const auto rearm = [&] {
    if (watchdog && !isAborted())
      watchdog->arm();
  };

  const QByteArray payload = payloadFromJsValue(data);
  if (sourceId < 0 || payload.isEmpty()) {
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: bad source or data"));
    return out;
  }

  if (isAborted()) {
    out.insert(QStringLiteral("error"), abortMessage);
    return out;
  }

  qint64 written = -1;
  QMetaObject::invokeMethod(marshaller,
                            "writeAndArm",
                            Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(qint64, written),
                            Q_ARG(int, sourceId),
                            Q_ARG(QByteArray, payload));

  if (written <= 0) {
    QMetaObject::invokeMethod(
      marshaller, "disarmReply", Qt::BlockingQueuedConnection, Q_ARG(int, sourceId));
    out.insert(QStringLiteral("error"), QStringLiteral("deviceWriteAndWait: write failed"));
    rearm();
    return out;
  }

  const auto satisfies = replyAcceptor(until);
  const int budget     = qBound(0, timeoutMs, kMaxReplyWaitMs);

  QByteArray reply;
  bool satisfied = false;
  for (int waited = 0; waited <= budget && !satisfied; waited += kReplyPollSliceMs) {
    if (isAborted())
      break;

    QThread::msleep(static_cast<unsigned long>(kReplyPollSliceMs));
    QMetaObject::invokeMethod(marshaller,
                              "pollReply",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QByteArray, reply),
                              Q_ARG(int, sourceId));
    satisfied = satisfies(reply);
  }

  QMetaObject::invokeMethod(
    marshaller, "disarmReply", Qt::BlockingQueuedConnection, Q_ARG(int, sourceId));
  rearm();

  out.insert(QStringLiteral("ok"), satisfied);
  out.insert(QStringLiteral("data"), QString::fromUtf8(reply));
  out.insert(QStringLiteral("bytesRead"), reply.size());
  out.insert(QStringLiteral("timedOut"), !satisfied);
  return out;
}
