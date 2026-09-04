/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QThreadPool>

namespace AI {

/**
 * @brief Runs the read-only filesystem tool calls off the GUI thread: fs.search scans up to 5000
 *        files and 64 MB of text, which inline execution pays for with the display tick. Results
 *        return as a queued signal carrying the turn generation they were issued for, so a
 *        cancelled or superseded turn drops them instead of answering a tool_use it never made.
 */
class AsyncToolRunner : public QObject {
  Q_OBJECT

signals:
  void toolFinished(const QString& callId,
                    const QString& name,
                    const QJsonObject& arguments,
                    const QJsonObject& reply,
                    quint64 generation);

public:
  explicit AsyncToolRunner(QObject* parent = nullptr);
  AsyncToolRunner(AsyncToolRunner&&)                 = delete;
  AsyncToolRunner(const AsyncToolRunner&)            = delete;
  AsyncToolRunner& operator=(AsyncToolRunner&&)      = delete;
  AsyncToolRunner& operator=(const AsyncToolRunner&) = delete;
  ~AsyncToolRunner() override;

  [[nodiscard]] static bool handles(const QString& name);

  void run(const QString& callId,
           const QString& name,
           const QJsonObject& arguments,
           quint64 generation);

private:
  QThreadPool m_pool;
};

}  // namespace AI
