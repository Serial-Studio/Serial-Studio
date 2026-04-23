/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QObject>
#include <QSettings>
#include <QString>

#include "IO/FrameConfig.h"
#include "SerialStudio.h"

/**
 * @brief Singleton owning the application-level operation mode, project file path,
 *        and derived FrameConfig for device 0.
 */
class AppState : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString projectFileName
             READ projectFileName
             NOTIFY projectFileChanged)
  Q_PROPERTY(QString projectFilePath
             READ projectFilePath
             NOTIFY projectFileChanged)
  Q_PROPERTY(SerialStudio::OperationMode operationMode
             READ operationMode
             WRITE setOperationMode
             NOTIFY operationModeChanged)
  // clang-format on

signals:
  void operationModeChanged();
  void projectFileChanged();
  void frameConfigChanged(const IO::FrameConfig& config);

private:
  explicit AppState();
  AppState(AppState&&)                 = delete;
  AppState(const AppState&)            = delete;
  AppState& operator=(AppState&&)      = delete;
  AppState& operator=(const AppState&) = delete;

public:
  [[nodiscard]] static AppState& instance();

  [[nodiscard]] QString projectFileName() const;
  [[nodiscard]] const QString& projectFilePath() const noexcept;
  [[nodiscard]] SerialStudio::OperationMode operationMode() const noexcept;
  [[nodiscard]] IO::FrameConfig frameConfig() const noexcept;

public slots:
  void setupExternalConnections();
  void restoreLastProject();
  void setOperationMode(SerialStudio::OperationMode mode);

private slots:
  void onProjectLoaded();
  void onProjectFileCleared();

private:
  [[nodiscard]] IO::FrameConfig deriveFrameConfig() const;

private:
  QSettings m_settings;
  QString m_projectFilePath;
  SerialStudio::OperationMode m_operationMode;
  IO::FrameConfig m_frameConfig;
};
