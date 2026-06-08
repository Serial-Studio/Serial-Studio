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

#include "WorkspaceManager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the WorkspaceManager and initializes the workspace path.
 */
Misc::WorkspaceManager::WorkspaceManager()
{
  m_temporaryActive = false;

  auto def =
    QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                         QStringLiteral("Serial Studio"));
  m_path = m_settings.value(QStringLiteral("Workspace"), def).toString();

  QDir dir(m_path);
  if (!dir.exists() && !dir.mkpath(".")) {
    qWarning() << "Failed to create workspace directory:" << m_path;
    m_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/Serial Studio";
    qWarning() << "Using fallback workspace:" << m_path;
  }

  migrateLegacyProjectsFolder();
}

/**
 * @brief Returns the singleton instance of WorkspaceManager.
 */
Misc::WorkspaceManager& Misc::WorkspaceManager::instance()
{
  static WorkspaceManager instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Path queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the base workspace path.
 */
QString Misc::WorkspaceManager::path() const noexcept
{
  return m_path;
}

/**
 * @brief Returns the user friendly workspace path.
 */
QString Misc::WorkspaceManager::shortPath() const
{
  return path().replace(QDir::homePath(), QStringLiteral("~"));
}

/**
 * @brief Returns the full path to a subdirectory within the workspace.
 */
QString Misc::WorkspaceManager::path(const QString& subdirectory) const
{
  QString path = QStringLiteral("%1/%2").arg(m_path, subdirectory);

  QDir dir(path);
  if (!dir.exists()) {
    if (!dir.mkpath("."))
      qCritical() << "Failed to create workspace subdirectory:" << path;
  }

  return path;
}

//--------------------------------------------------------------------------------------------------
// Path management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Redirects the workspace to a transient path without persisting it (benchmark use).
 */
void Misc::WorkspaceManager::setTemporaryPath(const QString& path)
{
  Q_ASSERT(!path.isEmpty());
  if (m_temporaryActive)
    return;

  m_savedPath       = m_path;
  m_path            = path;
  m_temporaryActive = true;
  Q_EMIT pathChanged();
}

/**
 * @brief Restores the workspace path saved by setTemporaryPath().
 */
void Misc::WorkspaceManager::clearTemporaryPath()
{
  if (!m_temporaryActive)
    return;

  m_path            = m_savedPath;
  m_temporaryActive = false;
  m_savedPath.clear();
  Q_EMIT pathChanged();
}

/**
 * @brief Resets the workspace path to the default location.
 */
void Misc::WorkspaceManager::reset()
{
  m_path = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                QStringLiteral("Serial Studio"));
  m_settings.setValue(QStringLiteral("Workspace"), m_path);

  Q_EMIT pathChanged();
}

/**
 * @brief Rewrites a saved "JSON Projects" path so it resolves under the new "Projects" folder.
 */
QString Misc::WorkspaceManager::remapLegacyPath(const QString& path) const
{
  if (path.isEmpty())
    return path;

  QString p = QDir::fromNativeSeparators(path);
  if (!p.contains(QStringLiteral("/JSON Projects/")))
    return path;

  p.replace(QStringLiteral("/JSON Projects/"), QStringLiteral("/Projects/"));
  return p;
}

/**
 * @brief Copy-with-marker migration from {workspace}/JSON Projects to {workspace}/Projects.
 */
void Misc::WorkspaceManager::migrateLegacyProjectsFolder()
{
  static const QLatin1StringView kMigratedKey("Workspace/LegacyProjectsFolderMigrated");
  static const QLatin1StringView kMarkerFileName(".migrated-from-json-projects");

  if (m_settings.value(kMigratedKey, false).toBool())
    return;

  const QString legacyDir = QStringLiteral("%1/JSON Projects").arg(m_path);
  const QString newDir    = QStringLiteral("%1/Projects").arg(m_path);
  const QString marker    = QStringLiteral("%1/%2").arg(newDir, kMarkerFileName);

  QDir legacy(legacyDir);
  if (!legacy.exists()) {
    m_settings.setValue(kMigratedKey, true);
    return;
  }

  if (QFileInfo::exists(marker)) {
    m_settings.setValue(kMigratedKey, true);
    return;
  }

  QDir target(newDir);
  if (!target.exists() && !target.mkpath(QStringLiteral("."))) {
    qWarning() << "Failed to create" << newDir << "for legacy projects migration";
    return;
  }

  const auto entries =
    legacy.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);

  bool allCopied = true;
  for (const auto& entry : entries) {
    const QString src = entry.absoluteFilePath();
    QString dst       = QStringLiteral("%1/%2").arg(newDir, entry.fileName());

    if (entry.isFile() && QFileInfo::exists(dst)) {
      QFile a(src);
      QFile b(dst);
      if (a.size() == b.size() && a.open(QIODevice::ReadOnly) && b.open(QIODevice::ReadOnly)
          && a.readAll() == b.readAll())
        continue;
    }

    if (QFileInfo::exists(dst)) {
      const QString base = entry.completeBaseName();
      const QString ext  = entry.suffix();
      int n              = 1;
      do {
        const QString suffix =
          n == 1 ? QStringLiteral(".legacy") : QStringLiteral(".legacy.%1").arg(n);
        dst = ext.isEmpty() ? QStringLiteral("%1/%2%3").arg(newDir, base, suffix)
                            : QStringLiteral("%1/%2%3.%4").arg(newDir, base, suffix, ext);
        ++n;
      } while (QFileInfo::exists(dst) && n < 1000);
    }

    if (entry.isFile()) {
      if (!QFile::copy(src, dst)) {
        qWarning() << "Failed to copy" << src << "to" << dst;
        allCopied = false;
      }
    } else {
      qWarning() << "Skipping non-file entry in legacy projects folder:" << src;
    }
  }

  if (!allCopied)
    return;

  QFile m(marker);
  if (!m.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to write migration marker" << marker;
    return;
  }

  const QByteArray stamp = QStringLiteral("migrated\n").toUtf8();
  if (m.write(stamp) != stamp.size()) {
    m.close();
    QFile::remove(marker);
    qWarning() << "Short write on migration marker" << marker;
    return;
  }
  m.flush();
  m.close();

  m_settings.setValue(kMigratedKey, true);
}

/**
 * @brief Opens a dialog for the user to select a new workspace path.
 */
void Misc::WorkspaceManager::selectPath()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(), tr("Select Workspace Location"), m_path);

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        m_path = path;
        m_settings.setValue(QStringLiteral("Workspace"), path);
        Q_EMIT pathChanged();
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}
