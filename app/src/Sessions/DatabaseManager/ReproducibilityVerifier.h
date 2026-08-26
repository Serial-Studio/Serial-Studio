/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QList>
#  include <QObject>
#  include <QString>
#  include <QVariantList>
#  include <QVariantMap>

class QProcess;

namespace Sessions {
/**
 * @brief Runs the reproducibility checks of specs 0044/0047 as child processes, owning the
 *        single child slot they share, the auto-serialized candidate snapshots and the
 *        golden-tag sweep queue. DatabaseManager owns the archive; this owns the checks run
 *        against it, and learns its path through setArchive() rather than reaching back.
 */
class ReproducibilityVerifier : public QObject {
  Q_OBJECT

signals:
  void busyChanged();
  void sweepChanged();
  void reportChanged();
  void regressionBusyChanged();
  void verificationFinished(int sessionId, bool success, const QVariantMap& verdict);
  void regressionFinished(int sessionId, bool success, const QVariantMap& report);

public:
  explicit ReproducibilityVerifier(QObject* parent = nullptr);

  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] bool regressionBusy() const noexcept;
  [[nodiscard]] QVariantMap sweepStatus() const;
  [[nodiscard]] const QVariantMap& lastReport() const noexcept;

  void shutdown();
  void setArchive(const QString& filePath);

public slots:
  void verify(int sessionId);
  bool regress(int sessionId, const QString& candidatePath);
  bool regressByTag(const QString& tag, const QString& candidatePath);

private:
  void finishSweep();
  void advanceSweep(int sessionId, const QVariantMap& report);
  void concludeRegression(int sessionId, bool success, const QVariantMap& report);
  void concludeVerification(int sessionId, bool success, const QVariantMap& verdict);
  [[nodiscard]] QList<int> taggedSessions(const QString& tag, bool& ok) const;
  [[nodiscard]] bool publishStartFailure(int sessionId,
                                         const QString& code,
                                         const QString& error,
                                         const QString& hint);

private:
  bool m_regressActive;
  bool m_sweepActive;
  bool m_sweepOwnsCandidate;
  QProcess* m_process;
  QString m_sweepTag;
  QString m_archivePath;
  QString m_sweepCandidate;
  QString m_regressCandidateTemp;
  QList<int> m_sweepQueue;
  QVariantMap m_lastReport;
  QVariantList m_sweepReports;
};
}  // namespace Sessions

#endif
