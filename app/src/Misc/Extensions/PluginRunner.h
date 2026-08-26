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

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace Misc {
/**
 * @brief Owns the child processes of running plugins: their lifetime, captured output, and the
 *        list the dashboard shows. ExtensionManager owns the manifest and so decides whether a
 *        plugin may launch; by the time anything reaches here only a process remains to run.
 *        That is the split: nothing here reads the manifest, nothing there touches a QProcess.
 */
class PluginRunner : public QObject {
  Q_OBJECT

signals:
  void runningChanged();
  void outputChanged(const QString& id);

public:
  explicit PluginRunner(QObject* parent = nullptr);

  [[nodiscard]] bool isRunning(const QString& id) const;
  [[nodiscard]] QString output(const QString& id) const;
  [[nodiscard]] bool userClosed(const QString& id) const;
  [[nodiscard]] const QVariantList& running() const noexcept;
  [[nodiscard]] QStringList restorableIds() const;

  void appendOutput(const QString& id, const QString& text);
  void clearUserClosed(const QString& id);

  bool start(const QString& id,
             const QString& pluginDir,
             const QString& runtime,
             const QString& entryPath,
             const QString& title,
             const bool terminal,
             const bool hasPipDeps);

public slots:
  void stop(const QString& id);
  void stopAll();

private slots:
  void onFinished(const QString& id);

private:
  void forget(const QString& id);
  void wireProcess(QProcess* process, const QString& id);
  [[nodiscard]] QProcessEnvironment buildEnvironment() const;
  void startProcess(QProcess* process,
                    const QString& runtime,
                    const QString& entryPath,
                    const bool terminal);

private:
  QSettings m_settings;
  QVariantList m_running;
  QSet<QString> m_userClosed;
  QMap<QString, QString> m_output;
  QMap<QString, QProcess*> m_processes;
};
}  // namespace Misc
