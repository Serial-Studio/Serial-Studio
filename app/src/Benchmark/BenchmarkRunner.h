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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVariantList>

namespace Benchmark {

/**
 * @brief Drives the in-app hotpath benchmark and exposes its progress/results to QML.
 */
class BenchmarkRunner : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool running
             READ running
             NOTIFY runningChanged)
  Q_PROPERTY(double progress
             READ progress
             NOTIFY progressChanged)
  Q_PROPERTY(QString currentPhase
             READ currentPhase
             NOTIFY currentPhaseChanged)
  Q_PROPERTY(QVariantList results
             READ results
             NOTIFY resultsChanged)
  Q_PROPERTY(QStringList frameOptions
             READ frameOptions
             NOTIFY optionsChanged)
  Q_PROPERTY(QStringList secondsOptions
             READ secondsOptions
             NOTIFY optionsChanged)
  // clang-format on

signals:
  void runningChanged();
  void progressChanged();
  void currentPhaseChanged();
  void resultsChanged();
  void optionsChanged();
  void finished();
  void dashboardPreviewActive(bool active);

private:
  explicit BenchmarkRunner();
  BenchmarkRunner(BenchmarkRunner&&)                 = delete;
  BenchmarkRunner(const BenchmarkRunner&)            = delete;
  BenchmarkRunner& operator=(BenchmarkRunner&&)      = delete;
  BenchmarkRunner& operator=(const BenchmarkRunner&) = delete;

public:
  [[nodiscard]] static BenchmarkRunner& instance();

  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] double progress() const noexcept;
  [[nodiscard]] QString currentPhase() const;
  [[nodiscard]] QVariantList results() const;
  [[nodiscard]] QStringList frameOptions() const;
  [[nodiscard]] QStringList secondsOptions() const;

public slots:
  void copyResults();
  void clearResults();
  void start(int framesIndex, int secondsIndex);

private slots:
  void retranslate();

private:
  void beginSession();
  void endSession();
  void finishSession();
  void announcePhase(int index);
  void executePhase(int index);

private:
  bool m_running;
  double m_progress;
  int m_phaseIndex;
  quint64 m_frames;
  double m_seconds;
  QString m_currentPhase;
  QVariantList m_results;
  QStringList m_phaseLabels;
  QStringList m_frameOptions;
  QStringList m_secondsOptions;

  int m_savedMode;
  double m_savedPlotTimeRange;
  QString m_savedProjectPath;
  bool m_savedCsvExport;
  bool m_savedApiServer;
#ifdef BUILD_COMMERCIAL
  bool m_savedMdfExport;
  bool m_savedSessionExport;
#endif
#ifdef ENABLE_GRPC
  bool m_savedGrpcServer;
#endif
  std::unique_ptr<QTemporaryDir> m_tempWorkspace;
};

}  // namespace Benchmark
