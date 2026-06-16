/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/FileSandbox.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringConverter>
#include <QTextStream>

#include "AI/Logging.h"
#include "Misc/WorkspaceManager.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the empty sandbox; roots are derived live from WorkspaceManager.
 */
AI::FileSandbox::FileSandbox() {}

/**
 * @brief Returns the process-wide filesystem sandbox.
 */
AI::FileSandbox& AI::FileSandbox::instance()
{
  static FileSandbox singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Envelope helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a failure envelope with an error code and optional hint.
 */
static QJsonObject failure(const QString& error, const QString& hint = {})
{
  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = error;
  if (!hint.isEmpty())
    out[QStringLiteral("hint")] = hint;

  return out;
}

/**
 * @brief Returns a workspace-relative display path, or the absolute path if outside.
 */
static QString displayPath(const QString& absolute, const QString& root)
{
  if (absolute == root)
    return QStringLiteral(".");

  const auto prefix = root + QLatin1Char('/');
  if (absolute.startsWith(prefix))
    return absolute.mid(prefix.size());

  return absolute;
}

//--------------------------------------------------------------------------------------------------
// Roots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the base workspace directory (read root and ancestor of the write root).
 */
QString AI::FileSandbox::workspaceRoot() const
{
  const auto base = Misc::WorkspaceManager::instance().path();
  QDir dir(base);
  if (!dir.exists())
    dir.mkpath(QStringLiteral("."));

  return QFileInfo(base).canonicalFilePath();
}

/**
 * @brief Returns the 'AI/' write root, creating it if needed.
 */
QString AI::FileSandbox::writeRoot() const
{
  return QFileInfo(Misc::WorkspaceManager::instance().path(QStringLiteral("AI")))
    .canonicalFilePath();
}

/**
 * @brief Returns the session allow-list of user-dropped paths.
 */
QStringList AI::FileSandbox::droppedPaths() const
{
  const QMutexLocker locker(&m_dropMutex);
  return m_droppedPaths;
}

/**
 * @brief Returns every directory/file that authorizes a read: workspace + dropped paths.
 */
QStringList AI::FileSandbox::readRoots() const
{
  QStringList roots;
  const auto ws = workspaceRoot();
  if (!ws.isEmpty())
    roots.append(ws);

  const QMutexLocker locker(&m_dropMutex);
  for (const auto& dropped : m_droppedPaths)
    if (!dropped.isEmpty() && !roots.contains(dropped))
      roots.append(dropped);

  return roots;
}

//--------------------------------------------------------------------------------------------------
// Path canonicalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Canonicalizes a path by resolving its deepest existing ancestor and re-appending tail.
 */
QString AI::FileSandbox::canonicalParentJoin(const QString& absolute)
{
  const QFileInfo info(absolute);
  if (info.exists())
    return info.canonicalFilePath();

  QStringList tail;
  QFileInfo cursor(info);
  for (int hops = 0; hops <= kMaxRecurseDepth; ++hops) {
    tail.prepend(cursor.fileName());
    const auto parentPath = cursor.absolutePath();
    if (parentPath == cursor.absoluteFilePath())
      return {};

    const QFileInfo parent(parentPath);
    if (parent.exists()) {
      const auto base = parent.canonicalFilePath();
      if (base.isEmpty())
        return {};

      return QDir::cleanPath(base + QDir::separator() + tail.join(QDir::separator()));
    }

    cursor = parent;
  }

  return {};
}

/**
 * @brief Returns true when a canonical path is the root itself or a descendant of it.
 */
bool AI::FileSandbox::isWithinRoot(const QString& canonical, const QString& root)
{
  if (canonical.isEmpty() || root.isEmpty())
    return false;

  if (canonical == root)
    return true;

  return canonical.startsWith(root + QLatin1Char('/'));
}

/**
 * @brief Re-canonicalizes an opened path and confirms it still resolves within a root.
 */
static bool openedPathWithinRoots(const QString& openedPath, const QStringList& roots)
{
  const QFileInfo info(openedPath);
  const auto canonical = info.canonicalFilePath();
  if (canonical.isEmpty())
    return false;

  for (const auto& root : roots) {
    if (root.isEmpty())
      continue;

    if (canonical == root || canonical.startsWith(root + QLatin1Char('/')))
      return true;
  }

  return false;
}

/**
 * @brief Resolves an input path for reading against the workspace + dropped roots.
 */
AI::FileSandbox::Resolved AI::FileSandbox::resolveRead(const QString& input) const
{
  if (input.isEmpty() || input.contains(QChar(QChar::Null)))
    return {false, {}, QStringLiteral("invalid_path"), {}};

  const auto base      = workspaceRoot();
  const QString joined = QDir::isAbsolutePath(input) ? input : QDir(base).absoluteFilePath(input);
  const auto canonical = canonicalParentJoin(QDir::cleanPath(joined));
  if (canonical.isEmpty())
    return {false, {}, QStringLiteral("not_found"), {}};

  if (isWithinRoot(canonical, base))
    return {true, canonical, {}, {}};

  for (const auto& dropped : droppedPaths())
    if (canonical == dropped)
      return {true, canonical, {}, {}};

  return {false,
          {},
          QStringLiteral("outside_sandbox"),
          QStringLiteral("Reads are limited to the Serial Studio workspace folder and files "
                         "the user dragged into the chat this session.")};
}

/**
 * @brief Returns true when a relative path's first segment is the 'AI' write folder.
 */
static bool startsWithAiSegment(const QString& relative)
{
  const auto clean = QDir::cleanPath(relative);
  if (clean.compare(QStringLiteral("AI"), Qt::CaseInsensitive) == 0)
    return true;

  const auto head = clean.section(QLatin1Char('/'), 0, 0);
  return head.compare(QStringLiteral("AI"), Qt::CaseInsensitive) == 0;
}

/**
 * @brief Resolves an input path for writing; only the 'AI/' write root is permitted.
 */
AI::FileSandbox::Resolved AI::FileSandbox::resolveWrite(const QString& input) const
{
  if (input.isEmpty() || input.contains(QChar(QChar::Null)))
    return {false, {}, QStringLiteral("invalid_path"), {}};

  const auto root = writeRoot();
  if (root.isEmpty())
    return {false, {}, QStringLiteral("write_root_unavailable"), {}};

  QString joined;
  if (QDir::isAbsolutePath(input))
    joined = input;
  else if (startsWithAiSegment(input))
    joined = QDir(workspaceRoot()).absoluteFilePath(input);
  else
    joined = QDir(root).absoluteFilePath(input);

  const auto canonical = canonicalParentJoin(QDir::cleanPath(joined));
  if (canonical.isEmpty())
    return {false, {}, QStringLiteral("not_found"), {}};

  if (!isWithinRoot(canonical, root))
    return {false,
            {},
            QStringLiteral("outside_sandbox"),
            QStringLiteral("Writes and deletes are limited to the workspace 'AI/' subfolder. "
                           "Pass a path under AI/, e.g. 'notes.md' or 'exports/data.csv'.")};

  return {true, canonical, {}, {}};
}

//--------------------------------------------------------------------------------------------------
// Binary detection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when a byte sample looks like binary (NUL byte or invalid UTF-8).
 */
static bool looksBinary(const QByteArray& sample)
{
  if (sample.contains('\0'))
    return true;

  auto decoder = QStringDecoder(QStringDecoder::Utf8);
  (void)QString(decoder.decode(sample));
  return decoder.hasError();
}

//--------------------------------------------------------------------------------------------------
// Drop allow-list
//--------------------------------------------------------------------------------------------------

/**
 * @brief Canonicalizes and records a user-dropped path for session-scoped read access.
 */
QString AI::FileSandbox::registerDroppedPath(const QString& localPath)
{
  const QFileInfo dropped(localPath);
  if (dropped.isSymLink() || !dropped.isFile())
    return {};

  const auto canonical = dropped.canonicalFilePath();
  if (canonical.isEmpty())
    return {};

  const QMutexLocker locker(&m_dropMutex);
  if (!m_droppedPaths.contains(canonical)) {
    m_droppedPaths.append(canonical);
    qCDebug(serialStudioAI) << "Sandbox: registered dropped path" << canonical;
  }

  return canonical;
}

/**
 * @brief Clears every dropped path; called when a conversation is reset.
 */
void AI::FileSandbox::clearDroppedPaths()
{
  const QMutexLocker locker(&m_dropMutex);
  m_droppedPaths.clear();
}

//--------------------------------------------------------------------------------------------------
// list
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends one directory's children to the listing, recursing within the depth cap.
 */
static void collectEntries(const QString& dir,
                           const QString& root,
                           bool recursive,
                           int depth,
                           QJsonArray& rows,
                           bool& truncated)
{
  if (depth > AI::FileSandbox::kMaxRecurseDepth || truncated)
    return;

  const auto flags = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks;
  const auto infos = QDir(dir).entryInfoList(flags, QDir::Name | QDir::DirsFirst);
  for (const auto& info : infos) {
    if (rows.size() >= AI::FileSandbox::kMaxListEntries) {
      truncated = true;
      return;
    }

    QJsonObject row;
    row[QStringLiteral("path")] = displayPath(info.absoluteFilePath(), root);
    row[QStringLiteral("type")] = info.isDir() ? QStringLiteral("dir") : QStringLiteral("file");
    if (info.isFile())
      row[QStringLiteral("sizeBytes")] = static_cast<double>(info.size());

    row[QStringLiteral("modified")] = info.lastModified().toString(Qt::ISODate);
    rows.append(row);

    if (recursive && info.isDir())
      collectEntries(info.absoluteFilePath(), root, true, depth + 1, rows, truncated);
  }
}

/**
 * @brief Lists a directory under the read roots; supports bounded recursion.
 */
QJsonObject AI::FileSandbox::list(const QJsonObject& args) const
{
  const auto input    = args.value(QStringLiteral("path")).toString();
  const auto resolved = resolveRead(input.isEmpty() ? QStringLiteral(".") : input);
  if (!resolved.ok)
    return failure(resolved.error, resolved.hint);

  const QFileInfo info(resolved.path);
  if (!info.isDir())
    return failure(QStringLiteral("not_a_directory"),
                   QStringLiteral("Use fs.read for files; fs.list expects a directory."));

  bool truncated       = false;
  const bool recursive = args.value(QStringLiteral("recursive")).toBool(false);
  QJsonArray rows;
  collectEntries(resolved.path, workspaceRoot(), recursive, 0, rows, truncated);

  QJsonObject out;
  out[QStringLiteral("ok")]        = true;
  out[QStringLiteral("path")]      = displayPath(resolved.path, workspaceRoot());
  out[QStringLiteral("count")]     = rows.size();
  out[QStringLiteral("entries")]   = rows;
  out[QStringLiteral("truncated")] = truncated;
  if (truncated)
    out[QStringLiteral("hint")] =
      QStringLiteral("Listing capped; narrow the path or list subdirectories individually.");

  return out;
}

//--------------------------------------------------------------------------------------------------
// read
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a byte slice of a file under the read roots, paged via offset/limit.
 */
QJsonObject AI::FileSandbox::read(const QJsonObject& args) const
{
  const auto resolved = resolveRead(args.value(QStringLiteral("path")).toString());
  if (!resolved.ok)
    return failure(resolved.error, resolved.hint);

  QFileInfo info(resolved.path);
  if (!info.isFile())
    return failure(QStringLiteral("not_a_file"),
                   QStringLiteral("Use fs.list for directories; fs.read expects a file."));

  QFile file(resolved.path);
  if (!file.open(QIODevice::ReadOnly))
    return failure(QStringLiteral("open_failed"), file.errorString());

  if (info.isSymLink() || !openedPathWithinRoots(file.fileName(), readRoots()))
    return failure(QStringLiteral("outside_sandbox"),
                   QStringLiteral("Refusing to read a symlinked or sandbox-escaping path."));

  const qint64 total  = file.size();
  const qint64 offset = qBound<qint64>(0, args.value(QStringLiteral("offset")).toInteger(0), total);
  const qint64 want   = args.value(QStringLiteral("limit")).toInt(kMaxReadSlice);
  const qint64 limit  = qBound<qint64>(1, want, kMaxReadSlice);

  if (!file.seek(offset))
    return failure(QStringLiteral("seek_failed"), file.errorString());

  const QByteArray sniff = file.peek(qMin<qint64>(limit, kBinarySniffBytes));
  if (looksBinary(sniff))
    return failure(QStringLiteral("binary_file"),
                   QStringLiteral("File appears binary. fs.read returns text only; request a "
                                  "different file or a known text range."));

  const QByteArray chunk = file.read(limit);
  const qint64 next      = offset + chunk.size();

  QJsonObject out;
  out[QStringLiteral("ok")]            = true;
  out[QStringLiteral("path")]          = displayPath(resolved.path, workspaceRoot());
  out[QStringLiteral("content")]       = QString::fromUtf8(chunk);
  out[QStringLiteral("offset")]        = static_cast<double>(offset);
  out[QStringLiteral("bytesReturned")] = static_cast<double>(chunk.size());
  out[QStringLiteral("totalBytes")]    = static_cast<double>(total);
  out[QStringLiteral("eof")]           = (next >= total);
  if (next < total)
    out[QStringLiteral("nextOffset")] = static_cast<double>(next);

  return out;
}

//--------------------------------------------------------------------------------------------------
// search
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scans one text file for a needle, appending bounded line-level hits.
 */
static void searchFile(const QString& path,
                       const QString& root,
                       const QRegularExpression& needle,
                       QJsonArray& hits,
                       bool& truncated)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return;

  if (looksBinary(file.peek(AI::FileSandbox::kBinarySniffBytes)))
    return;

  QTextStream stream(&file);
  int lineNo = 0;
  while (!stream.atEnd()) {
    const auto line = stream.readLine();
    ++lineNo;
    if (!needle.match(line).hasMatch())
      continue;

    if (hits.size() >= AI::FileSandbox::kMaxSearchHits) {
      truncated = true;
      return;
    }

    QJsonObject row;
    row[QStringLiteral("file")] = displayPath(path, root);
    row[QStringLiteral("line")] = lineNo;
    row[QStringLiteral("text")] = line.left(400);
    hits.append(row);
  }
}

/**
 * @brief Walks the read roots, collecting files to search within the scan cap.
 */
static QStringList gatherSearchFiles(const QStringList& roots, int cap)
{
  QStringList files;
  for (const auto& root : roots) {
    const QFileInfo rootInfo(root);
    if (rootInfo.isFile()) {
      files.append(root);
      continue;
    }

    QDirIterator it(root,
                    QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);
    while (it.hasNext() && files.size() < cap)
      files.append(it.next());

    if (files.size() >= cap)
      break;
  }

  return files;
}

/**
 * @brief Grep-like content search across the read roots; literal or regex.
 */
QJsonObject AI::FileSandbox::search(const QJsonObject& args) const
{
  const auto query = args.value(QStringLiteral("query")).toString();
  if (query.isEmpty())
    return failure(QStringLiteral("missing_query"));

  const bool isRegex = args.value(QStringLiteral("isRegex")).toBool(false);
  const auto pattern = isRegex ? query : QRegularExpression::escape(query);
  QRegularExpression needle(pattern, QRegularExpression::CaseInsensitiveOption);
  if (!needle.isValid())
    return failure(QStringLiteral("bad_regex"), needle.errorString());

  const auto files = gatherSearchFiles(readRoots(), kMaxSearchFiles);
  bool truncated   = files.size() >= kMaxSearchFiles;
  QJsonArray hits;
  for (const auto& file : files) {
    searchFile(file, workspaceRoot(), needle, hits, truncated);
    if (hits.size() >= kMaxSearchHits) {
      truncated = true;
      break;
    }
  }

  QJsonObject out;
  out[QStringLiteral("ok")]           = true;
  out[QStringLiteral("query")]        = query;
  out[QStringLiteral("filesScanned")] = files.size();
  out[QStringLiteral("count")]        = hits.size();
  out[QStringLiteral("hits")]         = hits;
  out[QStringLiteral("truncated")]    = truncated;
  return out;
}

//--------------------------------------------------------------------------------------------------
// write / append
//--------------------------------------------------------------------------------------------------

/**
 * @brief Atomically writes (or appends) UTF-8 content to a file under the 'AI/' write root.
 */
static QJsonObject writeBytes(const QString& path,
                              const QByteArray& payload,
                              bool appendMode,
                              const QString& root)
{
  const QFileInfo info(path);
  if (info.isSymLink())
    return failure(QStringLiteral("outside_sandbox"),
                   QStringLiteral("Refusing to write through a symlinked target."));

  if (!QDir().mkpath(info.absolutePath()))
    return failure(QStringLiteral("mkdir_failed"), info.absolutePath());

  if (appendMode) {
    QFile file(path);
    if (!file.open(QIODevice::Append))
      return failure(QStringLiteral("open_failed"), file.errorString());

    if (file.write(payload) != payload.size())
      return failure(QStringLiteral("write_failed"), file.errorString());

    file.close();
  } else {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
      return failure(QStringLiteral("open_failed"), file.errorString());

    if (file.write(payload) != payload.size())
      return failure(QStringLiteral("write_failed"), file.errorString());

    if (!file.commit())
      return failure(QStringLiteral("commit_failed"), file.errorString());
  }

  QJsonObject out;
  out[QStringLiteral("ok")]           = true;
  out[QStringLiteral("path")]         = displayPath(path, root);
  out[QStringLiteral("bytesWritten")] = static_cast<double>(payload.size());
  return out;
}

/**
 * @brief Writes content to an 'AI/' file, replacing it atomically.
 */
QJsonObject AI::FileSandbox::write(const QJsonObject& args) const
{
  const auto resolved = resolveWrite(args.value(QStringLiteral("path")).toString());
  if (!resolved.ok)
    return failure(resolved.error, resolved.hint);

  const auto payload = args.value(QStringLiteral("content")).toString().toUtf8();
  if (payload.size() > kMaxWriteBytes)
    return failure(QStringLiteral("too_large"),
                   QStringLiteral("Content exceeds the 4 MB write cap; split it into parts."));

  return writeBytes(resolved.path, payload, false, workspaceRoot());
}

/**
 * @brief Appends content to an 'AI/' file, creating it if needed.
 */
QJsonObject AI::FileSandbox::append(const QJsonObject& args) const
{
  const auto resolved = resolveWrite(args.value(QStringLiteral("path")).toString());
  if (!resolved.ok)
    return failure(resolved.error, resolved.hint);

  const auto payload = args.value(QStringLiteral("content")).toString().toUtf8();
  const QFileInfo info(resolved.path);
  const qint64 existing = info.isFile() ? info.size() : 0;
  if (existing + payload.size() > kMaxWriteBytes)
    return failure(QStringLiteral("too_large"),
                   QStringLiteral("Resulting file would exceed the 4 MB cap."));

  return writeBytes(resolved.path, payload, true, workspaceRoot());
}

//--------------------------------------------------------------------------------------------------
// remove
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deletes a file or empty directory under the 'AI/' write root.
 */
QJsonObject AI::FileSandbox::remove(const QJsonObject& args) const
{
  const auto resolved = resolveWrite(args.value(QStringLiteral("path")).toString());
  if (!resolved.ok)
    return failure(resolved.error, resolved.hint);

  if (resolved.path == writeRoot())
    return failure(QStringLiteral("refused"),
                   QStringLiteral("Refusing to delete the AI/ root itself."));

  const QFileInfo info(resolved.path);
  if (!info.exists())
    return failure(QStringLiteral("not_found"));

  bool removed = false;
  if (info.isDir()) {
    QDir dir(resolved.path);
    if (!dir.isEmpty())
      return failure(QStringLiteral("dir_not_empty"),
                     QStringLiteral("Delete the directory's files first; recursive delete is "
                                    "not supported."));

    removed = QDir().rmdir(resolved.path);
  } else {
    removed = QFile::remove(resolved.path);
  }

  if (!removed)
    return failure(QStringLiteral("delete_failed"));

  QJsonObject out;
  out[QStringLiteral("ok")]      = true;
  out[QStringLiteral("path")]    = displayPath(resolved.path, workspaceRoot());
  out[QStringLiteral("deleted")] = true;
  return out;
}
