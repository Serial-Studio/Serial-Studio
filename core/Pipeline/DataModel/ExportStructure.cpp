/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "DataModel/ExportStructure.h"

#include <QDebug>

#include "Core/SSAssert.h"
#include "Misc/WorkspaceManager.h"

/**
 * @brief Drops the adopted schema so the next session starts from an empty template.
 */
void DataModel::ExportStructure::clear()
{
  clear_frame(m_templateFrame);
  SS_ASSERT_LOG(m_templateFrame.groups.empty());
}

/**
 * @brief Stores the schema template frame; must run on the worker thread (queued invoke) so the
 *        assignment never races the ingest or the close. An empty frame is ignored: structure
 *        arrives asynchronously and QuickPlot has none until its first frame, so an empty payload
 *        landing second would wipe an adopted template and no file would be made.
 */
void DataModel::ExportStructure::setTemplateFrame(const Frame& frame)
{
  if (frame.groups.empty())
    return;

  m_templateFrame = frame;
}

/**
 * @brief Adopts the structure the pipeline publishes when the connect-time fetch came back empty.
 *        QuickPlot derives its datasets from the first frame, so at connect there is nothing to
 *        fetch; blocks carry values only, so without this the file is never created. Ignored once
 *        a file exists -- an open file's schema is fixed for its lifetime.
 */
void DataModel::ExportStructure::applyPublishedStructure(const Frame& frame, bool resourceOpen)
{
  if (resourceOpen || !m_templateFrame.groups.empty())
    return;

  m_templateFrame = frame;
}

/**
 * @brief True once a schema has been adopted; a worker with none cannot create its file yet.
 */
bool DataModel::ExportStructure::hasStructure() const noexcept
{
  return !m_templateFrame.groups.empty();
}

/**
 * @brief The adopted schema the file's columns are created from.
 */
const DataModel::Frame& DataModel::ExportStructure::templateFrame() const noexcept
{
  return m_templateFrame;
}

/**
 * @brief Scrubs a project title for use as a folder or file-name component: path separators, the
 *        Windows reserved set, parent-directory hops and trailing dots or spaces all go, because
 *        every one of them either escapes the workspace or names a path Windows cannot open.
 */
QString DataModel::ExportStructure::sanitizeTitle(const QString& title, const QString& fallback)
{
  SS_ASSERT_LOG(!fallback.isEmpty());

  static const QString kForbidden = QStringLiteral("/\\:*?\"<>|");

  QString safe;
  safe.reserve(title.size());
  for (const QChar ch : title)
    if (!kForbidden.contains(ch) && ch != QChar('\0'))
      safe.append(ch);

  safe.remove(QStringLiteral(".."));
  safe = safe.simplified();

  int keep = 0;
  for (int i = safe.size(); i > 0; --i) {
    const QChar c = safe.at(i - 1);
    if (c != QChar('.') && c != QChar(' ')) {
      keep = i;
      break;
    }
  }

  safe.truncate(keep);
  return safe.isEmpty() ? fallback : safe;
}

/**
 * @brief Returns (creating it) the session directory an export lane writes into:
 *        <workspace @p workspaceKey>/<sanitized title>. Routing every lane through one helper is
 *        what keeps a session's files side by side; a directory that cannot be created comes back
 *        non-existent, so the caller must check exists().
 */
QDir DataModel::ExportStructure::sessionDir(const QString& workspaceKey,
                                            const QString& title,
                                            const QString& fallback)
{
  SS_ASSERT(!workspaceKey.isEmpty(), return QDir());

  static auto& workspaceManager = Misc::WorkspaceManager::instance();
  const auto subdir             = workspaceManager.path(workspaceKey);
  const auto path = QStringLiteral("%1/%2").arg(subdir, sanitizeTitle(title, fallback));

  QDir dir(path);
  if (!dir.exists() && !dir.mkpath(QStringLiteral(".")))
    qWarning() << "[" << workspaceKey << "] failed to create export directory:" << path;

  return dir;
}
