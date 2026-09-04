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

#  include <map>
#  include <memory>
#  include <QCryptographicHash>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QSet>
#  include <QSqlDatabase>
#  include <QString>
#  include <vector>

namespace IO {
class FrameReader;
}  // namespace IO

namespace Sessions {

/**
 * @brief Offline reproducibility verifier for archived sessions (spec 0044): runs only in the
 *        --verify-session child process, re-interprets archived raw bytes through the real
 *        pipeline into a temporary re-record, and sequence-diffs it against the archive. The
 *        archive stays read-only except for one appended verifications row.
 */
class Verifier {
public:
  enum class Mode {
    Verify,
    Regress
  };

  struct Options {
    QString dbPath;
    QString candidateProjectPath;
    Mode mode            = Mode::Verify;
    int sessionId        = -1;
    bool keepRegenerated = false;
  };

  explicit Verifier(const Options& options);
  Verifier(Verifier&&)                 = delete;
  Verifier(const Verifier&)            = delete;
  Verifier& operator=(Verifier&&)      = delete;
  Verifier& operator=(const Verifier&) = delete;
  ~Verifier();

  /**
   * @brief Internal verdict codes returned by run(); the CLI collapses them to a binary
   *        process exit (0 = reproduced) and the JSON report carries the full verdict.
   */
  static constexpr int kExitReproduced    = 0;
  static constexpr int kExitError         = 1;
  static constexpr int kExitDiverged      = 2;
  static constexpr int kExitNotVerifiable = 3;

  static constexpr int kMaxArchiveDevices = 64;

  static constexpr qint64 kChunkStepNs       = 1'000'000;
  static constexpr qint64 kMaxFramesPerChunk = kChunkStepNs;

  [[nodiscard]] int run();
  [[nodiscard]] const QJsonObject& report() const noexcept;

private:
  [[nodiscard]] bool openArchive();
  [[nodiscard]] bool loadSession();
  [[nodiscard]] bool sessionIsConsoleOnly() const;
  [[nodiscard]] bool verifyIntegrity();
  [[nodiscard]] bool hashSampleStream(QCryptographicHash& hash);
  void classifySession();
  [[nodiscard]] int runRegression();
  [[nodiscard]] QString loadCandidate();
  [[nodiscard]] QString replayBothSides();
  [[nodiscard]] bool regressDiff();
  [[nodiscard]] bool reportOneSidedStructural(const QString& regenPath, bool sideIsBaseline);
  [[nodiscard]] bool diffRegenPair(QSqlDatabase& baseDb,
                                   QSqlDatabase& candDb,
                                   int baseSession,
                                   int candSession);
  [[nodiscard]] QSet<int> archivedDeviceIds(bool& ok);
  [[nodiscard]] std::vector<qint64> chunkTimestamps();
  [[nodiscard]] QString latestStoredVerdict();
  [[nodiscard]] QJsonObject replaySideStats() const;
  [[nodiscard]] int settleRegressVerdict();
  [[nodiscard]] int failRegress(const QString& code, const QString& reason, const QString& hint);
  static void reparseFailureText(const QString& code, QString& reason, QString& hint);
  [[nodiscard]] QString reparseSession(const QString& projectJson,
                                       const QString& regenPath,
                                       bool injectTimestamps);
  [[nodiscard]] bool feedArchivedBytes(bool injectTimestamps);
  [[nodiscard]] IO::FrameReader& readerForDevice(int deviceId);
  [[nodiscard]] bool diffReadings();
  [[nodiscard]] QJsonObject diffDataset(QSqlDatabase& regen,
                                        int regenSessionId,
                                        qint64 uniqueId,
                                        bool compareFinals);
  [[nodiscard]] int decideVerdict(bool diverged, bool anySkipped, bool consoleOnly);
  [[nodiscard]] int settleVerdict();
  void appendVerificationRecord();
  void cleanupRegenerated();
  [[nodiscard]] int fail(const QString& code,
                         const QString& stage,
                         const QString& reason,
                         const QString& hint);

private:
  Options m_options;

  QSqlDatabase m_db;
  QString m_archiveConnName;

  int m_sessionId;
  QString m_projectJson;
  QString m_storedRawSha256;
  QString m_storedReadingsSha256;
  QString m_captureAppVersion;
  QString m_reproClassJson;
  qint64 m_framesDropped;
  qint64 m_overflowBytes;
  bool m_legacyCapture;

  QString m_rawIntegrity;
  QString m_readingsIntegrity;

  bool m_controlScriptSeen;
  bool m_finalsVerifiable;
  QSet<qint64> m_virtualDatasets;

  QString m_regenPath;
  QString m_verdict;
  QJsonArray m_datasetReports;
  QJsonArray m_notes;
  QJsonObject m_report;

  qint64 m_firstFrameChunk;
  qint64 m_lastFeedChunks;
  qint64 m_lastFeedFrames;
  bool m_chunkBudgetExceeded;
  std::vector<QString> m_regenPaths;

  QString m_candidateJson;
  QString m_candidateSha256;
  QString m_candidateTitle;
  QJsonObject m_codeChanges;
  QJsonObject m_replayStats;
  QSet<int> m_feedExcludedDevices;
  QList<int> m_candidateRemovedDevices;

  std::map<int, std::unique_ptr<IO::FrameReader>> m_readers;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
