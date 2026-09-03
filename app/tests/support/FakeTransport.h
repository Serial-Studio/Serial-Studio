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

#pragma once

#include <QByteArray>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QStringList>

namespace Test {

/**
 * @brief A QNetworkReply whose body, status and error are handed to it up front.
 *
 * Qt has no public canned reply, so the buffer and the readData()/bytesAvailable() pair are
 * implemented here. Everything is posted through the event loop rather than emitted from the
 * constructor: a caller that connects after createRequest() returns must still see the signals.
 */
class FakeNetworkReply final : public QNetworkReply {
  Q_OBJECT

public:
  FakeNetworkReply(const QNetworkRequest& request,
                   QNetworkAccessManager::Operation operation,
                   int httpStatus,
                   QByteArray body,
                   QNetworkReply::NetworkError error,
                   QObject* parent = nullptr);
  ~FakeNetworkReply() override;

  void abort() override;
  [[nodiscard]] qint64 bytesAvailable() const override;
  [[nodiscard]] bool isSequential() const override;

protected:
  [[nodiscard]] qint64 readData(char* data, qint64 maxSize) override;

private slots:
  void deliver();

private:
  QByteArray m_body;
  qint64 m_offset;
  int m_httpStatus;
  QNetworkReply::NetworkError m_error;
};

/**
 * @brief QNetworkAccessManager replacement that answers from a queue of canned responses.
 *
 * Responses are matched by URL substring, in insertion order, and consumed on match, so a test
 * scripts a whole multi-file download (catalog, then each file, then a 404) as one queue. An
 * unmatched request answers 404 rather than reaching the network: a test double that falls
 * through to the internet is worse than one that fails.
 */
class FakeTransport final : public QNetworkAccessManager {
  Q_OBJECT

public:
  explicit FakeTransport(QObject* parent = nullptr);
  ~FakeTransport() override;

  void enqueue(const QString& urlSubstring,
               int httpStatus,
               const QByteArray& body,
               QNetworkReply::NetworkError err = QNetworkReply::NoError);

  [[nodiscard]] QStringList requestedUrls() const;
  [[nodiscard]] int pendingResponses() const;

protected:
  [[nodiscard]] QNetworkReply* createRequest(Operation operation,
                                             const QNetworkRequest& request,
                                             QIODevice* outgoingData = nullptr) override;

private:
  /**
   * @brief One queued answer: the URL fragment it matches and what it replies with.
   */
  struct Canned {
    QString urlSubstring;
    int httpStatus;
    QByteArray body;
    QNetworkReply::NetworkError error;
  };

  QList<Canned> m_queue;
  QStringList m_requested;
};

}  // namespace Test
