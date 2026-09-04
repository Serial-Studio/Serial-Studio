/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "Core/SSAssert.h"
#include "IO/Drivers/Network.h"

static constexpr int kHttpMaxRedirects        = 3;
static constexpr int kHttpTransferTimeoutMs   = 30000;
static constexpr int kHttpMethodGet           = 0;
static constexpr qint64 kHttpMaxResponseBytes = 8 * 1024 * 1024;

/**
 * @brief Maps a method index onto its HTTP verb.
 */
[[nodiscard]] static QByteArray httpVerb(int index)
{
  switch (index) {
    case 1:
      return QByteArrayLiteral("POST");
    case 2:
      return QByteArrayLiteral("PUT");
    case 3:
      return QByteArrayLiteral("PATCH");
    case 4:
      return QByteArrayLiteral("DELETE");
    default:
      return QByteArrayLiteral("GET");
  }
}

/**
 * @brief Describes why a reply is not a usable response, keeping a transport failure and a
 *        refusing status apart because they have different fixes.
 */
[[nodiscard]] static QString replyFailureReason(const QNetworkReply* reply, int status)
{
  if (reply->error() != QNetworkReply::NoError)
    return reply->errorString();

  if (status > 0)
    return QObject::tr("Server answered HTTP %1").arg(status);

  return QObject::tr("The server sent no response");
}

/**
 * @brief Reads at most the response cap out of @p reply, logging the first truncation of a run.
 *        A REST endpoint answering with a multi-gigabyte body would otherwise be published whole
 *        into the frame pipeline on every poll.
 */
QByteArray IO::Drivers::Network::readCappedBody(QNetworkReply* reply)
{
  SS_ASSERT(reply != nullptr, return {});

  QByteArray body = reply->read(kHttpMaxResponseBytes);
  if (reply->bytesAvailable() <= 0)
    return body;

  if (!m_httpTruncationLogged) {
    m_httpTruncationLogged = true;
    logDriverError(tr("HTTP response truncated"),
                   tr("%1 answered with more than %2 bytes")
                     .arg(m_httpUrl, QString::number(kHttpMaxResponseBytes)));
  }

  return body;
}

/**
 * @brief Opens the HTTP source by issuing the configured request once. That request IS the connect
 *        verdict: a separate HEAD probe is answered with 405 by plenty of healthy endpoints, and a
 *        POST source would fire two side-effecting requests per session. A request that never left
 *        (or one that already settled) returns false so no pending verdict is faked.
 */
bool IO::Drivers::Network::openHttp(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);

  QUrl url;
  QString reason;
  if (!urlForCurrentMode(url, reason)) {
    logDriverError(tr("HTTP error"), reason);
    return false;
  }

  m_pollsOk              = 0;
  m_pollsFailed          = 0;
  m_pollsSkipped         = 0;
  m_consecutiveFailures  = 0;
  m_httpFailureLogged    = false;
  m_httpTruncationLogged = false;

  connect(m_httpManager,
          &QNetworkAccessManager::sslErrors,
          this,
          &IO::Drivers::Network::onHttpSslErrors,
          static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));

  m_dialPending = true;
  sendHttpRequest(m_httpBody.toUtf8());

  if (m_dialPending && m_reply.isNull()) {
    m_dialPending = false;
    logDriverError(tr("HTTP error"), tr("The request could not be sent"));
    return false;
  }

  return m_dialPending || m_httpActive;
}

/**
 * @brief Stops polling and retires an in-flight request. The finished handler is disconnected
 *        before the abort so a cancelled request cannot deliver a body or a verdict into a
 *        session the user already ended. Safe when nothing is in flight.
 */
void IO::Drivers::Network::closeHttp()
{
  m_pollTimer.stop();
  m_httpActive = false;

  if (m_reply.isNull())
    return;

  disconnect(m_reply, &QNetworkReply::finished, this, &IO::Drivers::Network::onHttpReplyFinished);
  m_reply->abort();
  m_reply->deleteLater();
  m_reply.clear();
}

/**
 * @brief Issues the configured request carrying @p body. The caller guarantees nothing is in
 *        flight: a second concurrent reply would orphan the first and lose its verdict.
 */
void IO::Drivers::Network::sendHttpRequest(const QByteArray& body)
{
  QUrl url;
  QString reason;
  if (!urlForCurrentMode(url, reason))
    return;

  m_reply = m_httpManager->sendCustomRequest(buildHttpRequest(url), httpVerb(m_httpMethod), body);

  connect(m_reply,
          &QNetworkReply::finished,
          this,
          &IO::Drivers::Network::onHttpReplyFinished,
          Qt::UniqueConnection);
}

/**
 * @brief Builds the request for @p url. Redirects are followed only for GET, because replaying a
 *        body against a redirect target is a second side effect the user did not ask for, and a
 *        transfer timeout is set so a hung endpoint cannot block polling forever.
 */
QNetworkRequest IO::Drivers::Network::buildHttpRequest(const QUrl& url) const
{
  QNetworkRequest request(url);
  request.setTransferTimeout(kHttpTransferTimeoutMs);
  request.setMaximumRedirectsAllowed(kHttpMaxRedirects);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       m_httpMethod == kHttpMethodGet ? QNetworkRequest::NoLessSafeRedirectPolicy
                                                      : QNetworkRequest::ManualRedirectPolicy);
  request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/octet-stream"));

  const auto lines = QStringView{m_httpHeaders}.split(u'\n', Qt::SkipEmptyParts);
  for (const auto& line : lines) {
    const qsizetype separator = line.indexOf(u':');
    if (separator <= 0)
      continue;

    const QByteArray name = line.left(separator).trimmed().toUtf8();
    if (!name.isEmpty())
      request.setRawHeader(name, line.mid(separator + 1).trimmed().toUtf8());
  }

  return request;
}

/**
 * @brief Issues one immediate request carrying @p data, so an API can be driven from the console,
 *        an action or an output widget. A poll still in flight is cancelled: the explicit write
 *        outranks a background refresh, and only one request may be outstanding.
 */
qint64 IO::Drivers::Network::writeHttp(const QByteArray& data)
{
  if (!m_reply.isNull()) {
    disconnect(m_reply, &QNetworkReply::finished, this, &IO::Drivers::Network::onHttpReplyFinished);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply.clear();
  }

  sendHttpRequest(data);
  return m_reply.isNull() ? 0 : data.size();
}

/**
 * @brief Returns true once the opening request succeeded. HTTP holds no link of its own, so this
 *        is what "connected" means for the source.
 */
bool IO::Drivers::Network::httpOpen() const
{
  return m_httpActive;
}

/**
 * @brief Returns true when the configured URL is one this transport can request. Validation runs
 *        through the shared resolver so the Connect button and open() can never disagree.
 */
bool IO::Drivers::Network::httpConfigured() const
{
  QUrl url;
  QString reason;
  return urlForCurrentMode(url, reason);
}

/**
 * @brief Issues the next poll, unless the previous response has not arrived yet. Skipping rather
 *        than queueing is deliberate: a backlog behind a slow endpoint grows latency without
 *        bound and eventually reports data older than the dashboard already showed.
 */
void IO::Drivers::Network::onPollTimeout()
{
  if (!m_httpActive)
    return;

  if (!m_reply.isNull()) {
    ++m_pollsSkipped;
    return;
  }

  sendHttpRequest(m_httpBody.toUtf8());
}

/**
 * @brief Consumes one response. The timestamp is taken here, at the capture boundary, and the
 *        in-flight pointer is cleared before publishing so a followed redirect cannot deliver the
 *        same request twice. During the opening request this settles the dial; afterwards a
 *        failure keeps the link up, counts, and logs only the first of a run.
 */
void IO::Drivers::Network::onHttpReplyFinished()
{
  const auto timestamp = IO::CapturedData::SteadyClock::now();

  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply == nullptr || reply != m_reply)
    return;

  m_reply.clear();
  reply->deleteLater();

  const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const bool ok    = reply->error() == QNetworkReply::NoError && status >= 200 && status < 300;
  if (ok)
    publishReceivedData(readCappedBody(reply), timestamp);

  if (m_dialPending) {
    if (!ok) {
      failDial(replyFailureReason(reply, status));
      return;
    }

    m_httpActive = true;
    succeedDial();
    if (m_httpInterval > 0)
      m_pollTimer.start(m_httpInterval);

    return;
  }

  if (ok) {
    ++m_pollsOk;
    if (m_consecutiveFailures > 0)
      logDriverError(tr("HTTP source recovered"), tr("%1 is answering again").arg(m_httpUrl));

    m_consecutiveFailures = 0;
    m_httpFailureLogged   = false;
    return;
  }

  ++m_pollsFailed;
  ++m_consecutiveFailures;
  if (!m_httpFailureLogged) {
    m_httpFailureLogged = true;
    logDriverError(tr("HTTP poll failed"), replyFailureReason(reply, status));
  }
}

/**
 * @brief Continues a request whose certificate failed verification, but only when the user asked
 *        for it, and never silently. Doing nothing here tears the session down before any data is
 *        exchanged, which is the default and the safe answer.
 */
void IO::Drivers::Network::onHttpSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
  if (!m_ignoreTlsErrors || reply == nullptr)
    return;

  QStringList descriptions;
  descriptions.reserve(errors.count());
  for (const auto& error : errors)
    descriptions.append(error.errorString());

  logDriverError(tr("TLS verification bypassed"),
                 tr("Continuing to %1 despite: %2")
                   .arg(reply->url().toString(), descriptions.join(QStringLiteral("; "))));

  reply->ignoreSslErrors();
}

/**
 * @brief Appends the HTTP rows of the driver property model. The shared TLS row is appended by
 *        the facade, which emits every transport's rows in one list.
 */
void IO::Drivers::Network::appendHttpProperties(QList<IO::DriverProperty>& props) const
{
  IO::DriverProperty url;
  url.key         = QStringLiteral("httpUrl");
  url.label       = tr("URL");
  url.description = tr("REST endpoint, for example %1").arg(defaultHttpUrl());
  url.type        = IO::DriverProperty::Text;
  url.value       = m_httpUrl;
  props.append(url);

  IO::DriverProperty method;
  method.key     = QStringLiteral("httpMethodIndex");
  method.label   = tr("Method");
  method.type    = IO::DriverProperty::ComboBox;
  method.value   = m_httpMethod;
  method.options = httpMethods();
  props.append(method);

  IO::DriverProperty interval;
  interval.key         = QStringLiteral("httpInterval");
  interval.label       = tr("Poll Interval (ms)");
  interval.description = tr("Use 0 to request only when data is sent");
  interval.type        = IO::DriverProperty::IntField;
  interval.value       = m_httpInterval;
  interval.min         = 0;
  interval.max         = 3600000;
  props.append(interval);

  IO::DriverProperty body;
  body.key   = QStringLiteral("httpBody");
  body.label = tr("Request Body");
  body.type  = IO::DriverProperty::Text;
  body.value = m_httpBody;
  props.append(body);

  IO::DriverProperty headers;
  headers.key         = QStringLiteral("httpHeaders");
  headers.label       = tr("Request Headers");
  headers.description = tr("One %1 pair per line").arg(QStringLiteral("Name: Value"));
  headers.type        = IO::DriverProperty::Text;
  headers.value       = m_httpHeaders;
  props.append(headers);
}

/**
 * @brief Applies an HTTP property by key, reporting whether it was consumed.
 */
bool IO::Drivers::Network::applyHttpProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("httpUrl")) {
    setHttpUrl(value.toString());
    return true;
  }

  if (key == QLatin1String("httpMethodIndex")) {
    setHttpMethodIndex(value.toInt());
    return true;
  }

  if (key == QLatin1String("httpInterval")) {
    setHttpInterval(value.toInt());
    return true;
  }

  if (key == QLatin1String("httpBody")) {
    setHttpBody(value.toString());
    return true;
  }

  if (key == QLatin1String("httpHeaders")) {
    setHttpHeaders(value.toString());
    return true;
  }

  return false;
}
