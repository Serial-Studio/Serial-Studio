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

#include "AI/Conversation/HelpFetcher.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QTextDocument>

#include "AI/Logging.h"
#include "SSAssert.h"

/**
 * @brief Creates an idle fetcher; nothing is requested until fetchPage().
 */
AI::HelpFetcher::HelpFetcher(QObject* parent) : QObject(parent), m_epoch(0) {}

//--------------------------------------------------------------------------------------------------
// URL policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when the URL is https on an exactly-anchored allowlisted host. An unanchored
 *        endsWith would let attacker domains like "evilgithub.com" through, and the model
 *        controls the URL, so this check is the exfiltration gate.
 */
bool AI::HelpFetcher::urlAllowed(const QUrl& url)
{
  if (!url.isValid() || url.scheme() != QStringLiteral("https") || !url.userInfo().isEmpty())
    return false;

  static const QStringList kHosts = {
    QStringLiteral("githubusercontent.com"),
    QStringLiteral("github.com"),
    QStringLiteral("serial-studio.com"),
  };

  const auto host = url.host().toLower();
  for (const auto& allowed : kHosts)
    if (host == allowed || host.endsWith(QLatin1Char('.') + allowed))
      return true;

  return false;
}

/**
 * @brief Resolves a model-supplied help path to a URL: a full http(s) URL is taken as-is,
 *        anything else is normalized into a doc/help markdown page name.
 */
QUrl AI::HelpFetcher::pageUrl(const QString& path)
{
  static const QString kHelpBase = QStringLiteral("https://raw.githubusercontent.com/Serial-Studio/"
                                                  "Serial-Studio/master/doc/help/");

  if (path.startsWith(QStringLiteral("http"), Qt::CaseInsensitive))
    return QUrl(path);

  QString page = path;
  if (page.startsWith('/'))
    page.remove(0, 1);

  if (page.isEmpty())
    page = QStringLiteral("Home");

  if (!page.endsWith(QStringLiteral(".md"), Qt::CaseInsensitive))
    page += QStringLiteral(".md");

  return QUrl(kHelpBase + page);
}

/**
 * @brief Applies the shared transport hardening to a help-fetch reply: re-validates every
 *        redirect target against the allowlist and aborts the transfer once the buffered
 *        body exceeds the hard cap.
 */
void AI::HelpFetcher::harden(QNetworkReply* reply) const
{
  SS_ASSERT(reply != nullptr, return);

  QObject::connect(reply, &QNetworkReply::redirected, reply, [reply](const QUrl& target) {
    if (urlAllowed(target))
      Q_EMIT reply->redirectAllowed();
    else
      reply->abort();
  });

  QObject::connect(reply, &QNetworkReply::readyRead, reply, [reply]() {
    if (reply->bytesAvailable() > kMaxTransportBytes)
      reply->abort();
  });
}

//--------------------------------------------------------------------------------------------------
// Fetch flow
//--------------------------------------------------------------------------------------------------

/**
 * @brief Invalidates every in-flight fetch: replies from before this call are drained and
 *        discarded instead of being reported into a turn that has moved on.
 */
void AI::HelpFetcher::abortPending()
{
  ++m_epoch;
}

/**
 * @brief Fetches a Serial Studio help page asynchronously; the outcome arrives on
 *        fetchFinished, which for a rejected URL is emitted synchronously.
 */
void AI::HelpFetcher::fetchPage(const QString& callId, const QString& path)
{
  SS_ASSERT_LOG(!callId.isEmpty());

  const auto url = pageUrl(path);
  if (!urlAllowed(url)) {
    QJsonObject err;
    err[QStringLiteral("ok")]    = false;
    err[QStringLiteral("error")] = QStringLiteral("Only https URLs on github.com / "
                                                  "raw.githubusercontent.com / serial-studio.com "
                                                  "are allowed");
    err[QStringLiteral("url")]   = url.toString();
    Q_EMIT fetchFinished(callId, err);
    return;
  }

  qCDebug(serialStudioAI) << "meta.fetchHelp" << url.toString();

  QNetworkRequest req(url);
  req.setRawHeader("User-Agent", "SerialStudio-AIAssistant");
  req.setRawHeader("Accept", "text/markdown,text/plain;q=0.9,text/html;q=0.5");
  req.setTransferTimeout(kFetchTimeoutMs);
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                   QNetworkRequest::UserVerifiedRedirectPolicy);
  auto* reply = m_nam.get(req);
  harden(reply);

  const quint64 epoch = m_epoch;
  connect(reply, &QNetworkReply::finished, this, [this, callId, reply, url, epoch]() {
    if (epoch != m_epoch) {
      reply->deleteLater();
      return;
    }

    completePage(callId, url, reply);
  });
}

/**
 * @brief Finalizes a page request: a 404 on a doc/help page hands off to the index instead
 *        of reporting, anything else is parsed, capped and reported.
 */
void AI::HelpFetcher::completePage(const QString& callId, const QUrl& url, QNetworkReply* reply)
{
  SS_ASSERT(reply != nullptr, return);

  const auto status    = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const auto netError  = reply->error();
  const auto host      = url.host();
  const auto path      = url.path();
  const bool isHelpDoc = host.endsWith(QStringLiteral("githubusercontent.com"))
                      && path.contains(QStringLiteral("/doc/help/"));

  QJsonObject result;
  result[QStringLiteral("url")] = url.toString();

  if (status == 404 && isHelpDoc && !path.endsWith(QStringLiteral("help.json"))) {
    reply->deleteLater();
    fetchIndex(callId, url);
    return;
  }

  if (netError != QNetworkReply::NoError) {
    result[QStringLiteral("ok")]    = false;
    result[QStringLiteral("error")] = reply->errorString();
  } else {
    const auto bytes         = reply->readAll();
    const bool isRawMarkdown = host.endsWith(QStringLiteral("githubusercontent.com"))
                            || path.endsWith(QStringLiteral(".md"), Qt::CaseInsensitive);

    QString text;
    if (isRawMarkdown) {
      text = QString::fromUtf8(bytes);
    } else {
      QTextDocument doc;
      doc.setHtml(QString::fromUtf8(bytes));
      text = doc.toPlainText();
    }

    if (text.size() > kMaxFetchBytes)
      text = text.left(kMaxFetchBytes) + QStringLiteral("\n... [truncated]");

    result[QStringLiteral("ok")]      = true;
    result[QStringLiteral("content")] = text;
  }

  reply->deleteLater();
  Q_EMIT fetchFinished(callId, result);
}

/**
 * @brief Fetches help.json so a guessed page-name 404 can be self-corrected.
 */
void AI::HelpFetcher::fetchIndex(const QString& callId, const QUrl& missedUrl)
{
  static const QUrl kIndexUrl(
    QStringLiteral("https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/"
                   "master/doc/help/help.json"));

  qCDebug(serialStudioAI) << "meta.fetchHelp redirect-to-index after 404:" << missedUrl.toString();

  QNetworkRequest req(kIndexUrl);
  req.setRawHeader("User-Agent", "SerialStudio-AIAssistant");
  req.setRawHeader("Accept", "application/json");
  req.setTransferTimeout(kFetchTimeoutMs);
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                   QNetworkRequest::UserVerifiedRedirectPolicy);
  auto* reply = m_nam.get(req);
  harden(reply);

  const quint64 epoch = m_epoch;
  connect(reply, &QNetworkReply::finished, this, [this, callId, reply, missedUrl, epoch]() {
    if (epoch != m_epoch) {
      reply->deleteLater();
      return;
    }

    completeIndex(callId, missedUrl, reply);
  });
}

/**
 * @brief Finalizes the index fallback: reports the capped help.json body plus the note that
 *        teaches the model how to pick the right file name on its next call.
 */
void AI::HelpFetcher::completeIndex(const QString& callId,
                                    const QUrl& missedUrl,
                                    QNetworkReply* reply)
{
  SS_ASSERT(reply != nullptr, return);

  QJsonObject result;
  result[QStringLiteral("url")]        = missedUrl.toString();
  result[QStringLiteral("redirected")] = true;

  if (reply->error() != QNetworkReply::NoError) {
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("404 on '%1', and the help index also failed: %2")
        .arg(missedUrl.toString(), reply->errorString());
  } else {
    auto bytes = reply->readAll();
    if (bytes.size() > kMaxIndexBytes)
      bytes.truncate(kMaxIndexBytes);

    result[QStringLiteral("ok")] = true;
    result[QStringLiteral("note")] =
      QStringLiteral("The page '%1' does not exist. Below is the full "
                     "help index (help.json). Each entry has an `id` "
                     "and a `file` -- pass the file name (without the "
                     ".md extension and with hyphens preserved) to "
                     "meta.fetchHelp on the next call. Common "
                     "mistakes: pass 'Painter-Widget' not 'Painter', "
                     "'API-Reference' not 'API'.")
        .arg(missedUrl.toString());
    result[QStringLiteral("content")] = QString::fromUtf8(bytes);
  }

  reply->deleteLater();
  Q_EMIT fetchFinished(callId, result);
}
