/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QElapsedTimer>
#  include <QKeyEvent>
#  include <QMap>
#  include <QObject>
#  include <QSqlDatabase>
#  include <QString>
#  include <vector>

#  include "SerialStudio.h"

namespace Sessions {

/**
 * @brief Replays Serial Studio SQLite export files as if they were live data.
 *
 * Opens a .db produced by SQLite::Export and walks the latest session's
 * readings table in timestamp order, reconstructing each sampled frame from
 * its per-dataset rows. Frames are injected into the standard pipeline via
 * @c IO::ConnectionManager::processPayload so widgets, exporters, and the
 * API see the same byte stream they would see from a live device.
 *
 * Each .db may contain multiple recording sessions — the player opens the
 * most recent one; older sessions can be selected explicitly via
 * @c openFile(path, sessionId).
 */
class Player : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(double progress
             READ progress
             NOTIFY timestampChanged)
  Q_PROPERTY(int frameCount
             READ frameCount
             NOTIFY playerStateChanged)
  Q_PROPERTY(int framePosition
             READ framePosition
             NOTIFY timestampChanged)
  Q_PROPERTY(bool isPlaying
             READ isPlaying
             NOTIFY playerStateChanged)
  Q_PROPERTY(const QString& timestamp
             READ timestamp
             NOTIFY timestampChanged)
  // clang-format on

signals:
  void openChanged();
  void timestampChanged();
  void playerStateChanged();

private:
  explicit Player();
  Player(Player&&)                 = delete;
  Player(const Player&)            = delete;
  Player& operator=(Player&&)      = delete;
  Player& operator=(const Player&) = delete;
  ~Player();

public:
  [[nodiscard]] static Player& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] int frameCount() const;
  [[nodiscard]] int framePosition() const;
  [[nodiscard]] double progress() const;
  [[nodiscard]] QString filename() const;
  [[nodiscard]] const QString& timestamp() const;

public slots:
  void play();
  void pause();
  void toggle();
  void openFile();
  void closeFile();
  void nextFrame();
  void previousFrame();
  void openFile(const QString& filePath);
  void openFile(const QString& filePath, int sessionId);
  void setProgress(const double progress);

private slots:
  void updateData();

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;
  bool handleKeyPress(QKeyEvent* keyEvent);

private:
  // Session loading
  void openSessionInternal(int sessionId);
  [[nodiscard]] int latestSessionId() const;

  // Restores the project embedded in the session row and switches the app
  // to ProjectFile mode so the dashboard has groups to build widgets from.
  // No-op when the session row has no snapshot (falls through to QuickPlot
  // headers in openSessionInternal).
  [[nodiscard]] bool restoreProjectFromSession(int sessionId);

  // Capture / restore the project + operation-mode state that existed before
  // the user opened a session. Populated on first openFile call while no
  // session is active; consumed by closeFile.
  void capturePreSessionState();
  void restorePreSessionState();

  // Schema/column prep
  void loadColumnOrder();
  void buildMultiSourceMapping();

  // Timestamp index
  void buildTimestampIndex();

  // Frame synthesis
  [[nodiscard]] QByteArray buildFrameAt(qint64 timestampNs);
  void injectFrame(const QByteArray& frame);
  void processFrameBatch(int startFrame, int endFrame);

  // Display helpers
  void updateTimestampDisplay();
  [[nodiscard]] QString formatTimestamp(double seconds) const;

private:
  QSqlDatabase m_db;
  QString m_filePath;
  QString m_connectionName;
  int m_sessionId;

  int m_framePos;
  bool m_playing;
  bool m_multiSource;
  QString m_timestamp;
  double m_startTimestampSeconds;

  QElapsedTimer m_elapsedTimer;

  // uniqueId list in the authoritative column order expected by
  // FrameBuilder (matches DataModel::ExportSchema ordering).
  std::vector<int> m_columnUniqueIds;

  // Timestamp index (ns) for every frame in the session; sorted ascending.
  std::vector<qint64> m_timestampsNs;

  // Multi-source playback mapping: column index → sourceId
  QMap<int, int> m_columnToSource;
  QMap<int, int> m_sourceColumnCount;

  // Snapshot of app state at the moment the user opened a session.
  // m_preSessionCaptured is set on entry to the first openFile() and
  // cleared by closeFile() after the restore runs — protects against
  // re-capturing the (already-replaced) session state on a second openFile
  // call while another session is still loaded.
  bool m_preSessionCaptured;
  SerialStudio::OperationMode m_preSessionOperationMode;
  QString m_preSessionProjectPath;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
