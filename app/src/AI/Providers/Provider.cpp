/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Providers/Provider.h"

#include <QHostAddress>
#include <QNetworkRequest>
#include <QUrl>

#include "AI/SseEventReader.h"

//--------------------------------------------------------------------------------------------------
// Transport policy (shared by every streaming backend)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a host is the local machine, the only place an API key may travel in the clear.
 */
static bool isLoopbackHost(const QString& host)
{
  if (host.compare(QStringLiteral("localhost"), Qt::CaseInsensitive) == 0)
    return true;

  const QHostAddress address(host);
  return !address.isNull() && address.isLoopback();
}

/**
 * @brief Whether a request may carry a provider key: https anywhere, plain http to loopback only.
 *        A local-model URL is user-supplied, so this is the one gate between a typo and a key
 *        posted across the network in cleartext (J8).
 */
bool AI::Reply::isTransportAllowed(const QUrl& url)
{
  if (!url.isValid() || url.host().isEmpty())
    return false;

  const auto scheme = url.scheme().toLower();
  if (scheme == QStringLiteral("https"))
    return true;

  return scheme == QStringLiteral("http") && isLoopbackHost(url.host());
}

/**
 * @brief Applies the streaming request policy: redirects are surfaced rather than followed, so a
 *        3xx cannot silently move an authenticated POST to another host.
 */
void AI::Reply::applyStreamPolicy(QNetworkRequest& request)
{
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QVariant::fromValue(QNetworkRequest::ManualRedirectPolicy));
}

/**
 * @brief The one stream parse-error policy: a recoverable frame error is skipped, an
 *        unrecoverable buffer state ends the turn, so no backend ships a truncated reply as
 *        success and none of them throws a whole turn away over one malformed frame (J6).
 */
bool AI::Reply::endsTurnOnParseError(const QString& reason)
{
  return SseEventReader::fatalReason(reason);
}

//--------------------------------------------------------------------------------------------------
// Finalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the stream finished and emits @ref finished exactly once.
 */
void AI::Reply::finishOk()
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT finished();
}

/**
 * @brief Marks the stream finished with an error message; the error precedes the completion so a
 *        listener sees the cause before the turn closes.
 */
void AI::Reply::finishWithError(const QString& message)
{
  if (m_finished)
    return;

  m_finished = true;
  Q_EMIT errorOccurred(message);
  Q_EMIT finished();
}

/**
 * @brief Charges bytes against the per-reply budget; on breach, ends the turn with a visible
 *        error and aborts the transport so Qt stops buffering the runaway stream.
 */
bool AI::Reply::streamBudgetBreached(qsizetype bytes)
{
  if (!chargeStreamBudget(bytes))
    return false;

  finishWithError(
    tr("Reply exceeded the %1 MB stream limit").arg(kMaxStreamedReplyBytes / (1024 * 1024)));
  abort();
  return true;
}
