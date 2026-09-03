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

#include "FakeTransport.h"

#include <algorithm>
#include <cstring>
#include <QTimer>
#include <QVariant>
#include <utility>

namespace Test {

FakeNetworkReply::FakeNetworkReply(const QNetworkRequest& request,
                                   QNetworkAccessManager::Operation operation,
                                   int httpStatus,
                                   QByteArray body,
                                   QNetworkReply::NetworkError error,
                                   QObject* parent)
  : QNetworkReply(parent)
  , m_body(std::move(body))
  , m_offset(0)
  , m_httpStatus(httpStatus)
  , m_error(error)
{
  setRequest(request);
  setUrl(request.url());
  setOperation(operation);
  open(QIODevice::ReadOnly | QIODevice::Unbuffered);
  QTimer::singleShot(0, this, &FakeNetworkReply::deliver);
}

FakeNetworkReply::~FakeNetworkReply() = default;

/**
 * @brief Marks the reply as cancelled by the caller and finishes it.
 */
void FakeNetworkReply::abort()
{
  m_error = QNetworkReply::OperationCanceledError;
  m_body.clear();
  m_offset = 0;
  setError(m_error, QStringLiteral("aborted"));
  Q_EMIT errorOccurred(m_error);
  setFinished(true);
  Q_EMIT finished();
}

qint64 FakeNetworkReply::bytesAvailable() const
{
  return QNetworkReply::bytesAvailable() + (m_body.size() - m_offset);
}

bool FakeNetworkReply::isSequential() const
{
  return true;
}

qint64 FakeNetworkReply::readData(char* data, qint64 maxSize)
{
  if (m_offset >= m_body.size())
    return -1;

  const qint64 count = std::min<qint64>(maxSize, m_body.size() - m_offset);
  std::memcpy(data, m_body.constData() + m_offset, static_cast<std::size_t>(count));
  m_offset += count;
  return count;
}

/**
 * @brief Publishes the canned status, body and error on the first event-loop turn.
 */
void FakeNetworkReply::deliver()
{
  setAttribute(QNetworkRequest::HttpStatusCodeAttribute, m_httpStatus);
  setHeader(QNetworkRequest::ContentLengthHeader, QVariant::fromValue(m_body.size()));
  Q_EMIT metaDataChanged();

  if (m_error != QNetworkReply::NoError) {
    setError(m_error, QStringLiteral("scripted transport error"));
    Q_EMIT errorOccurred(m_error);
  }

  if (!m_body.isEmpty()) {
    Q_EMIT downloadProgress(m_body.size(), m_body.size());
    Q_EMIT readyRead();
  }

  setFinished(true);
  Q_EMIT finished();
}

FakeTransport::FakeTransport(QObject* parent) : QNetworkAccessManager(parent) {}

FakeTransport::~FakeTransport() = default;

/**
 * @brief Queues one answer, matched against a URL substring in insertion order.
 */
void FakeTransport::enqueue(const QString& urlSubstring,
                            int httpStatus,
                            const QByteArray& body,
                            QNetworkReply::NetworkError err)
{
  m_queue.append(Canned{urlSubstring, httpStatus, body, err});
}

QStringList FakeTransport::requestedUrls() const
{
  return m_requested;
}

int FakeTransport::pendingResponses() const
{
  return static_cast<int>(m_queue.size());
}

/**
 * @brief Answers from the queue, or with a 404 when nothing matches; never reaches the network.
 */
QNetworkReply* FakeTransport::createRequest(Operation operation,
                                            const QNetworkRequest& request,
                                            QIODevice* outgoingData)
{
  Q_UNUSED(outgoingData);

  const QString url = request.url().toString();
  m_requested.append(url);

  for (qsizetype i = 0; i < m_queue.size(); ++i) {
    if (!url.contains(m_queue.at(i).urlSubstring))
      continue;

    const Canned canned = m_queue.takeAt(i);
    return new FakeNetworkReply(
      request, operation, canned.httpStatus, canned.body, canned.error, this);
  }

  return new FakeNetworkReply(
    request, operation, 404, QByteArray(), QNetworkReply::ContentNotFoundError, this);
}

}  // namespace Test
