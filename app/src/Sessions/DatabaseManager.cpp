/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/DatabaseManager.h"

#  include <QCoreApplication>
#  include <QCryptographicHash>
#  include <QDateTime>
#  include <QDir>
#  include <QFile>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QGuiApplication>
#  include <QInputDialog>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QJsonParseError>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QThread>
#  include <QTimer>

#  include "AppState.h"
#  include "DataModel/ProjectModel.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#  include "Sessions/Export.h"
#  include "Sessions/HtmlReport.h"
#  include "Sessions/Player.h"
#  include "Sessions/ReportData.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scrubs a project title for use as a folder/file name component.
 */
static QString sanitiseTitleForPath(const QString& title)
{
  QString safe = title;
  safe.remove(QChar('/'));
  safe.remove(QChar('\\'));
  safe.remove(QChar(':'));
  safe.remove(QChar('*'));
  safe.remove(QChar('?'));
  safe.remove(QChar('"'));
  safe.remove(QChar('<'));
  safe.remove(QChar('>'));
  safe.remove(QChar('|'));
  safe.remove(QChar('\0'));
  safe.remove(QStringLiteral(".."));
  safe = safe.simplified();
  if (safe.isEmpty())
    safe = QStringLiteral("Untitled");

  return safe;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes member variables and spins up the worker thread.
 */
Sessions::DatabaseManager::DatabaseManager()
  : m_thread(nullptr)
  , m_worker(nullptr)
  , m_open(false)
  , m_selectedSessionId(-1)
  , m_locked(false)
  , m_csvExportBusy(false)
  , m_csvExportProgress(0.0)
  , m_pdfExportBusy(false)
  , m_pdfExportProgress(0.0)
  , m_pendingPdfSessionId(-1)
  , m_pendingPdfActive(false)
  , m_nextToken(1)
  , m_outstandingMutations(0)
{
  qRegisterMetaType<Sessions::ReportPayloadPtr>("Sessions::ReportPayloadPtr");
  initWorker();
}

/**
 * @brief Stops the worker thread and tears the singleton down.
 */
Sessions::DatabaseManager::~DatabaseManager()
{
  shutdown();
}

/**
 * @brief Returns the single application-wide DatabaseManager.
 */
Sessions::DatabaseManager& Sessions::DatabaseManager::instance()
{
  static DatabaseManager singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Worker lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the worker, parks it on a dedicated thread, wires signals.
 */
void Sessions::DatabaseManager::initWorker()
{
  m_thread = new QThread;
  m_thread->setObjectName(QStringLiteral("Sessions::DatabaseWorker"));

  m_worker = new DatabaseWorker;
  m_worker->moveToThread(m_thread);

  connect(m_worker, &DatabaseWorker::opened, this, &DatabaseManager::onWorkerOpened);
  connect(m_worker, &DatabaseWorker::openFailed, this, &DatabaseManager::onWorkerOpenFailed);
  connect(m_worker, &DatabaseWorker::closed, this, &DatabaseManager::onWorkerClosed);
  connect(m_worker,
          &DatabaseWorker::sessionListRefreshed,
          this,
          &DatabaseManager::onWorkerSessionListRefreshed);
  connect(
    m_worker, &DatabaseWorker::tagListRefreshed, this, &DatabaseManager::onWorkerTagListRefreshed);
  connect(
    m_worker, &DatabaseWorker::lockStateChanged, this, &DatabaseManager::onWorkerLockStateChanged);
  connect(m_worker, &DatabaseWorker::notesUpdated, this, &DatabaseManager::onWorkerNotesUpdated);
  connect(
    m_worker, &DatabaseWorker::mutationFinished, this, &DatabaseManager::onWorkerMutationFinished);
  connect(
    m_worker, &DatabaseWorker::csvExportProgress, this, &DatabaseManager::onWorkerCsvProgress);
  connect(
    m_worker, &DatabaseWorker::csvExportFinished, this, &DatabaseManager::onWorkerCsvFinished);
  connect(
    m_worker, &DatabaseWorker::reportDataReady, this, &DatabaseManager::onWorkerReportDataReady);
  connect(m_worker,
          &DatabaseWorker::globalProjectJsonReady,
          this,
          &DatabaseManager::onWorkerGlobalProjectJsonReady);

  m_thread->start();
}

/**
 * @brief Synchronously closes the worker DB and joins the worker thread.
 */
void Sessions::DatabaseManager::shutdown()
{
  if (!m_thread)
    return;

  if (m_worker)
    m_worker->requestCancel();

  if (m_thread->isRunning() && m_worker) {
    QMetaObject::invokeMethod(m_worker, "closeDatabase", Qt::BlockingQueuedConnection);
    m_thread->quit();
    m_thread->wait(5000);
  }

  // Thread is dead -- delete worker directly (deleteLater would never fire)
  delete m_worker;
  m_worker = nullptr;

  delete m_thread;
  m_thread = nullptr;
}

//--------------------------------------------------------------------------------------------------
// External connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires cross-singleton signals. Called after all singletons are alive.
 */
void Sessions::DatabaseManager::setupExternalConnections()
{
  connect(&Sessions::Export::instance(),
          &Sessions::Export::openChanged,
          this,
          &Sessions::DatabaseManager::refreshSessionList);
}

//--------------------------------------------------------------------------------------------------
// Read-only accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true when a database file is open and queryable.
 */
bool Sessions::DatabaseManager::isOpen() const
{
  return m_open;
}

/**
 * @brief Returns @c true while a worker mutation or open is in flight.
 */
bool Sessions::DatabaseManager::busy() const
{
  return m_outstandingMutations > 0;
}

/**
 * @brief Returns the absolute file path of the open .db, or empty.
 */
QString Sessions::DatabaseManager::filePath() const
{
  return m_filePath;
}

/**
 * @brief Returns the base filename of the open database.
 */
QString Sessions::DatabaseManager::fileName() const
{
  return m_filePath.isEmpty() ? QString() : QFileInfo(m_filePath).fileName();
}

/**
 * @brief Returns the number of sessions in the open database.
 */
int Sessions::DatabaseManager::sessionCount() const
{
  return m_sessionList.size();
}

/**
 * @brief Returns the currently selected session id, or -1.
 */
int Sessions::DatabaseManager::selectedSessionId() const
{
  return m_selectedSessionId;
}

/**
 * @brief Returns @c true while a CSV export worker is running.
 */
bool Sessions::DatabaseManager::csvExportBusy() const
{
  return m_csvExportBusy;
}

/**
 * @brief Returns @c true while a PDF report is being rendered.
 */
bool Sessions::DatabaseManager::pdfExportBusy() const
{
  return m_pdfExportBusy;
}

/**
 * @brief Returns the user-facing status label for the active export.
 */
QString Sessions::DatabaseManager::pdfExportStatus() const
{
  return m_pdfExportStatus;
}

/**
 * @brief Returns the active PDF export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::pdfExportProgress() const
{
  return m_pdfExportProgress;
}

/**
 * @brief Returns the active CSV export's progress as a fraction in [0, 1].
 */
double Sessions::DatabaseManager::csvExportProgress() const
{
  return m_csvExportProgress;
}

/**
 * @brief Returns @c true while the session file is password-protected against deletion.
 */
bool Sessions::DatabaseManager::locked() const
{
  return m_locked;
}

/**
 * @brief Returns the cached session list as a QVariantList.
 */
QVariantList Sessions::DatabaseManager::sessionList() const
{
  return m_sessionList;
}

/**
 * @brief Returns the cached list of all tags defined in the database.
 */
QVariantList Sessions::DatabaseManager::tagList() const
{
  return m_tagList;
}

/**
 * @brief Returns the tags assigned to the currently selected session.
 */
QVariantList Sessions::DatabaseManager::selectedSessionTags() const
{
  if (m_selectedSessionId < 0)
    return {};

  return tagsForSession(m_selectedSessionId);
}

/**
 * @brief Returns the notes string for the currently selected session.
 */
QString Sessions::DatabaseManager::selectedSessionNotes() const
{
  if (m_selectedSessionId < 0)
    return {};

  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() == m_selectedSessionId)
      return m.value("notes").toString();
  }

  return {};
}

//--------------------------------------------------------------------------------------------------
// Cache lookups (Q_INVOKABLE)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the tags assigned to a specific session from the cache.
 */
QVariantList Sessions::DatabaseManager::tagsForSession(int sessionId) const
{
  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() == sessionId)
      return m.value("tags").toList();
  }

  return {};
}

/**
 * @brief Returns full metadata for a session from the cached list.
 */
QVariantMap Sessions::DatabaseManager::sessionMetadata(int sessionId) const
{
  QVariantMap result;
  for (const auto& v : m_sessionList) {
    const auto m = v.toMap();
    if (m.value("session_id").toInt() != sessionId)
      continue;

    result["session_id"]    = sessionId;
    result["project_title"] = m.value("project_title");
    result["started_at"]    = m.value("started_at");
    result["ended_at"]      = m.value("ended_at");
    result["notes"]         = m.value("notes");
    result["frame_count"]   = m.value("frame_count");
    return result;
  }

  return result;
}

/**
 * @brief Returns the canonical .db path for a project title. Pure path arithmetic.
 */
QString Sessions::DatabaseManager::canonicalDbPath(const QString& projectTitle)
{
  const QString safeTitle = sanitiseTitleForPath(projectTitle);
  const auto subdir       = Misc::WorkspaceManager::instance().path("Session Databases");
  return QStringLiteral("%1/%2/%2.db").arg(subdir, safeTitle);
}

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts the user to pick a .db file from the SQLite workspace folder.
 */
void Sessions::DatabaseManager::openDatabase()
{
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Open Session File"),
                                 Misc::WorkspaceManager::instance().path("Session Databases"),
                                 tr("Session files (*.db)"));

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openDatabase(path);

    dialog->deleteLater();
  });
  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);
  dialog->open();
}

/**
 * @brief Dispatches an asynchronous open + initial cache fetch to the worker.
 */
void Sessions::DatabaseManager::openDatabase(const QString& filePath)
{
  if (filePath.isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "openDatabase", Qt::QueuedConnection, Q_ARG(QString, filePath));
}

/**
 * @brief Dispatches an async close to the worker; main-thread cache clears on reply.
 */
void Sessions::DatabaseManager::closeDatabase(bool clearSavedPath)
{
  if (m_worker)
    m_worker->requestCancel();

  if (clearSavedPath)
    m_settings.remove("Sessions/LastDatabase");

  if (m_worker)
    QMetaObject::invokeMethod(m_worker, "closeDatabase", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Lock / unlock
//--------------------------------------------------------------------------------------------------

/**
 * @brief Prompts for a password, hashes it, and dispatches the lock write.
 */
void Sessions::DatabaseManager::lockDatabase()
{
  if (!isOpen() || m_locked)
    return;

  bool ok          = false;
  const auto first = QInputDialog::getText(nullptr,
                                           tr("Lock Session File"),
                                           tr("Choose a password to lock the session file:"),
                                           QLineEdit::Password,
                                           QString(),
                                           &ok);
  if (!ok || first.isEmpty())
    return;

  const auto second = QInputDialog::getText(nullptr,
                                            tr("Lock Session File"),
                                            tr("Confirm the password:"),
                                            QLineEdit::Password,
                                            QString(),
                                            &ok);

  if (first != second || !ok) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Passwords do not match"),
        tr("The two passwords you entered do not match. The session file was not locked."),
        QMessageBox::Warning);
    });
    return;
  }

  const QString hash =
    QString::fromLatin1(QCryptographicHash::hash(first.toUtf8(), QCryptographicHash::Md5).toHex());

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "persistLock",
                            Qt::QueuedConnection,
                            Q_ARG(QString, hash),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Prompts for the password, verifies it locally, and dispatches the unlock write.
 */
void Sessions::DatabaseManager::unlockDatabase()
{
  if (!isOpen() || !m_locked)
    return;

  if (m_passwordHash.isEmpty()) {
    setBusy(true);
    QMetaObject::invokeMethod(m_worker,
                              "persistLock",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString()),
                              Q_ARG(quint64, nextToken()));
    return;
  }

  bool ok        = false;
  const auto pwd = QInputDialog::getText(nullptr,
                                         tr("Unlock Session File"),
                                         tr("Enter the session file password:"),
                                         QLineEdit::Password,
                                         QString(),
                                         &ok);
  if (!ok)
    return;

  const auto hashPwd =
    QString::fromLatin1(QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5).toHex());

  if (hashPwd != m_passwordHash) {
    QTimer::singleShot(0, this, [] {
      Misc::Utilities::showMessageBox(
        tr("Incorrect password"),
        tr("The password you entered does not match the one stored in the session file."),
        QMessageBox::Warning);
    });
    return;
  }

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "persistLock",
                            Qt::QueuedConnection,
                            Q_ARG(QString, QString()),
                            Q_ARG(quint64, nextToken()));
}

//--------------------------------------------------------------------------------------------------
// Session management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a session by ID for detail display.
 */
void Sessions::DatabaseManager::setSelectedSessionId(int sessionId)
{
  if (m_selectedSessionId == sessionId)
    return;

  m_selectedSessionId = sessionId;
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Updates the cached notes locally and dispatches the SQL update.
 */
void Sessions::DatabaseManager::setSelectedSessionNotes(const QString& notes)
{
  if (m_selectedSessionId < 0 || !isOpen())
    return;

  if (selectedSessionNotes() == notes)
    return;

  // Optimistic local update so QML rebinds without waiting for the worker
  for (auto& v : m_sessionList) {
    auto m = v.toMap();
    if (m.value("session_id").toInt() == m_selectedSessionId) {
      m["notes"] = notes;
      v          = m;
      break;
    }
  }

  Q_EMIT sessionsChanged();
  Q_EMIT selectedSessionChanged();

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "setSessionNotes",
                            Qt::QueuedConnection,
                            Q_ARG(int, m_selectedSessionId),
                            Q_ARG(QString, notes),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches an async cascade-delete to the worker.
 */
void Sessions::DatabaseManager::deleteSession(int sessionId)
{
  Q_ASSERT(sessionId >= 0);

  if (!isOpen() || m_locked)
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "deleteSession",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(quint64, nextToken()));

  if (m_selectedSessionId == sessionId) {
    m_selectedSessionId = -1;
    Q_EMIT selectedSessionChanged();
  }
}

/**
 * @brief Asks the user to confirm before deleting a session.
 */
void Sessions::DatabaseManager::confirmDeleteSession(int sessionId)
{
  if (m_locked) {
    Misc::Utilities::showMessageBox(
      tr("Session file locked"),
      tr("Unlock the session file before deleting recorded sessions."),
      QMessageBox::Information);
    return;
  }

  const auto meta  = sessionMetadata(sessionId);
  const auto title = meta.value("started_at").toString();

  const int choice = Misc::Utilities::showMessageBox(
    tr("Delete session from %1?").arg(title),
    tr("All readings and raw data for this session are permanently removed."),
    QMessageBox::Warning,
    tr("Delete Session"),
    QMessageBox::Yes | QMessageBox::Cancel,
    QMessageBox::Cancel);

  if (choice == QMessageBox::Yes)
    deleteSession(sessionId);
}

/**
 * @brief Hands the selected session off to the SQLite player for replay.
 */
void Sessions::DatabaseManager::replaySelectedSession()
{
  if (m_selectedSessionId < 0 || m_filePath.isEmpty())
    return;

  Sessions::Player::instance().openFile(m_filePath, m_selectedSessionId);
}

//--------------------------------------------------------------------------------------------------
// Tag CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches a tag insert to the worker.
 */
void Sessions::DatabaseManager::addTag(const QString& label)
{
  if (!isOpen() || label.trimmed().isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "addTag", Qt::QueuedConnection, Q_ARG(QString, label), Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a tag delete to the worker.
 */
void Sessions::DatabaseManager::deleteTag(int tagId)
{
  if (!isOpen() || m_locked)
    return;

  setBusy(true);
  QMetaObject::invokeMethod(
    m_worker, "deleteTag", Qt::QueuedConnection, Q_ARG(int, tagId), Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a tag rename to the worker.
 */
void Sessions::DatabaseManager::renameTag(int tagId, const QString& newLabel)
{
  if (!isOpen() || newLabel.trimmed().isEmpty())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "renameTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, tagId),
                            Q_ARG(QString, newLabel),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a session/tag association insert to the worker.
 */
void Sessions::DatabaseManager::assignTagToSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "assignTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(int, tagId),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches a session/tag association delete to the worker.
 */
void Sessions::DatabaseManager::removeTagFromSession(int sessionId, int tagId)
{
  if (!isOpen())
    return;

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "unassignTag",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(int, tagId),
                            Q_ARG(quint64, nextToken()));
}

//--------------------------------------------------------------------------------------------------
// CSV export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks an output path and dispatches a streaming CSV export to the worker.
 */
void Sessions::DatabaseManager::exportSessionToCsv(int sessionId)
{
  if (!isOpen() || m_csvExportBusy)
    return;

  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir =
    QStringLiteral("%1/%2").arg(Misc::WorkspaceManager::instance().path("CSV"), safeProj);
  QDir().mkpath(dir);

  const QString suggested = QStringLiteral("%1/session_%2.csv").arg(dir).arg(sessionId);
  const auto path         = QFileDialog::getSaveFileName(
    nullptr, tr("Export Session to CSV"), suggested, tr("CSV files (*.csv)"));
  if (path.isEmpty())
    return;

  m_csvExportBusy     = true;
  m_csvExportProgress = 0.0;
  m_pendingCsvPath    = path;
  Q_EMIT csvExportBusyChanged();
  Q_EMIT csvExportProgressChanged();

  QMetaObject::invokeMethod(
    m_worker, "runCsvExport", Qt::QueuedConnection, Q_ARG(int, sessionId), Q_ARG(QString, path));
}

//--------------------------------------------------------------------------------------------------
// PDF / HTML report export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates QML options + path picker into a worker fetch + main-thread render.
 */
void Sessions::DatabaseManager::exportSessionToPdf(int sessionId, const QVariantMap& options)
{
  if (!isOpen() || m_pdfExportBusy)
    return;

  HtmlReportOptions opts;
  opts.outputPath    = options.value("outputPath").toString();
  opts.companyName   = options.value("companyName").toString();
  opts.documentTitle = options.value("documentTitle").toString();
  opts.authorName    = options.value("authorName").toString();
  opts.logoPath      = options.value("logoPath").toString();
  opts.pageSize =
    static_cast<QPageSize::PageSizeId>(options.value("pageSize", QPageSize::A4).toInt());
  opts.includeCover        = options.value("includeCover", true).toBool();
  opts.includeMetadata     = options.value("includeMetadata", true).toBool();
  opts.includeStats        = options.value("includeStats", true).toBool();
  opts.includeCharts       = options.value("includeCharts", true).toBool();
  opts.includeStatsOverlay = options.value("includeStatsOverlay", true).toBool();
  opts.lineWidth           = options.value("lineWidth", 1.4).toDouble();
  opts.lineStyle           = options.value("lineStyle", QStringLiteral("solid")).toString();

  const QString fmtStr = options.value("outputFormat", QStringLiteral("pdf")).toString().toLower();
  if (fmtStr == QStringLiteral("html"))
    opts.format = HtmlReportOptions::Format::Html;
  else if (fmtStr == QStringLiteral("both"))
    opts.format = HtmlReportOptions::Format::Both;
  else
    opts.format = HtmlReportOptions::Format::Pdf;

  auto launch = [this, sessionId](HtmlReportOptions opts) {
    m_pendingPdfOpts      = std::move(opts);
    m_pendingPdfSessionId = sessionId;
    m_pendingPdfActive    = true;

    const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);
    if (reportBusy) {
      m_pdfExportBusy     = true;
      m_pdfExportProgress = 0.0;
      m_pdfExportStatus   = tr("Loading session data…");
      Q_EMIT pdfExportBusyChanged();
      Q_EMIT pdfExportProgressChanged();
    }

    QMetaObject::invokeMethod(m_worker,
                              "runReportDataLoad",
                              Qt::QueuedConnection,
                              Q_ARG(int, sessionId),
                              Q_ARG(bool, m_pendingPdfOpts.includeCharts),
                              Q_ARG(int, 10000));
  };

  if (!opts.outputPath.isEmpty()) {
    launch(std::move(opts));
    return;
  }

  const bool wantsPdf  = (opts.format != HtmlReportOptions::Format::Html);
  const QString ext    = wantsPdf ? QStringLiteral("pdf") : QStringLiteral("html");
  const QString title  = wantsPdf ? tr("Save PDF Report") : tr("Save HTML Report");
  const QString filter = wantsPdf ? tr("PDF files (*.pdf)") : tr("HTML files (*.html)");

  const auto meta         = sessionMetadata(sessionId);
  const QString projTitle = meta.value("project_title").toString();
  const QString safeProj  = sanitiseTitleForPath(projTitle);
  const QString dir =
    QStringLiteral("%1/%2").arg(Misc::WorkspaceManager::instance().path("Reports"), safeProj);
  QDir().mkpath(dir);

  const QString baseName  = opts.documentTitle.isEmpty()
                            ? QStringLiteral("session_%1").arg(sessionId)
                            : sanitiseTitleForPath(opts.documentTitle);
  const QString suggested = QStringLiteral("%1/%2.%3").arg(dir, baseName, ext);

  auto* dialog = new QFileDialog(nullptr, title, suggested, filter);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setFileMode(QFileDialog::AnyFile);

  connect(dialog,
          &QFileDialog::fileSelected,
          this,
          [this, dialog, opts, ext, launch](const QString& path) mutable {
            dialog->deleteLater();

            if (path.isEmpty()) {
              Q_EMIT pdfExportFinished(QString(), false);
              return;
            }

            QString finalPath = path;
            const QString dot = QStringLiteral(".") + ext;
            if (!finalPath.endsWith(dot, Qt::CaseInsensitive))
              finalPath += dot;

            opts.outputPath = finalPath;
            launch(std::move(opts));
          });

  dialog->open();
}

/**
 * @brief Continues the PDF flow on the main thread once the worker has shipped data.
 */
void Sessions::DatabaseManager::renderReportFromPayload(const ReportPayloadPtr& payload)
{
  // Stale payload from a superseded request -- drop silently
  if (!m_pendingPdfActive)
    return;

  if (!payload || payload->sessionId != m_pendingPdfSessionId)
    return;

  if (!payload->ok) {
    if (m_pdfExportBusy) {
      m_pdfExportBusy     = false;
      m_pdfExportProgress = 0.0;
      m_pdfExportStatus   = tr("Failed");
      Q_EMIT pdfExportBusyChanged();
      Q_EMIT pdfExportProgressChanged();
    }

    Misc::Utilities::showMessageBox(
      tr("Report Failed"),
      payload->error.isEmpty()
        ? tr("Could not generate the report. Check the output path and try again.")
        : payload->error,
      QMessageBox::Warning);

    Q_EMIT pdfExportFinished(QString(), false);
    m_pendingPdfActive = false;
    return;
  }

  const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);

  auto* renderer = new HtmlReport(this);

  if (reportBusy) {
    connect(renderer, &HtmlReport::progress, this, [this](const QString& status, double percent) {
      m_pdfExportStatus   = status;
      m_pdfExportProgress = percent;
      Q_EMIT pdfExportProgressChanged();
    });
  }

  connect(renderer,
          &HtmlReport::finished,
          this,
          [this, renderer, reportBusy](const QString& outputPath, bool ok) {
            if (reportBusy) {
              m_pdfExportBusy     = false;
              m_pdfExportProgress = 1.0;
              m_pdfExportStatus   = ok ? tr("Done") : tr("Failed");
              Q_EMIT pdfExportBusyChanged();
              Q_EMIT pdfExportProgressChanged();
            }
            Q_EMIT pdfExportFinished(outputPath, ok);

            if (ok) {
              Misc::Utilities::revealFile(outputPath);
            } else {
              Misc::Utilities::showMessageBox(
                tr("Report Failed"),
                tr("Could not generate the report. Check the output path and try again."),
                QMessageBox::Warning);
            }

            m_pendingPdfActive = false;
            renderer->deleteLater();
          });

  renderer->render(payload->data, payload->series, m_pendingPdfOpts);
}

/**
 * @brief Opens a native QFileDialog to pick a logo for the report.
 */
void Sessions::DatabaseManager::pickReportLogo()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select logo image"), QString(), tr("Images (*.png *.jpg *.jpeg *.svg)"));
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    dialog->deleteLater();
    if (!path.isEmpty())
      Q_EMIT reportLogoPicked(path);
  });

  dialog->open();
}

//--------------------------------------------------------------------------------------------------
// Project metadata sync
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the current project JSON in the project_metadata table.
 */
void Sessions::DatabaseManager::storeProjectMetadata()
{
  if (!isOpen())
    return;

  const auto& pm  = DataModel::ProjectModel::instance();
  const auto json = QJsonDocument(pm.serializeToJson()).toJson(QJsonDocument::Compact);

  setBusy(true);
  QMetaObject::invokeMethod(m_worker,
                            "storeProjectMetadata",
                            Qt::QueuedConnection,
                            Q_ARG(QString, QString::fromUtf8(json)),
                            Q_ARG(QString, pm.title()),
                            Q_ARG(quint64, nextToken()));
}

/**
 * @brief Dispatches the JSON fetch to the worker; reply continues in
 * onWorkerGlobalProjectJsonReady.
 */
void Sessions::DatabaseManager::restoreProjectFromDb()
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(m_worker, "fetchGlobalProjectJson", Qt::QueuedConnection);
}

/**
 * @brief Worker handed back the global project JSON -- finish the restore on the main thread.
 */
void Sessions::DatabaseManager::runRestoreProjectFromJson(const QString& json)
{
  if (json.isEmpty()) {
    Misc::Utilities::showMessageBox(tr("No project data"),
                                    tr("This session file does not contain an embedded project."),
                                    QMessageBox::Warning);
    return;
  }

  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("Invalid project data"),
      tr("The embedded project JSON is malformed and cannot be restored."),
      QMessageBox::Critical);
    return;
  }

  const auto title       = doc.object().value("title").toString("Restored Project");
  const auto projectsDir = DataModel::ProjectModel::instance().jsonProjectsPath();
  const auto suggested   = QStringLiteral("%1/%2.ssproj").arg(projectsDir, title);

  const auto path = QFileDialog::getSaveFileName(
    nullptr, tr("Restore Project"), suggested, tr("Serial Studio projects (*.ssproj *.json)"));

  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(
      tr("Cannot write file"), tr("Check file permissions and try again."), QMessageBox::Critical);
    return;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  DataModel::ProjectModel::instance().openJsonFile(path);

  Q_EMIT projectMetadataRestored();
}

/**
 * @brief Reopens the database that was open during the last application run.
 */
void Sessions::DatabaseManager::restoreLastDatabase()
{
  const auto path = m_settings.value("Sessions/LastDatabase").toString();
  if (path.isEmpty())
    return;

  if (!QFileInfo::exists(path)) {
    m_settings.remove("Sessions/LastDatabase");
    return;
  }

  openDatabase(path);
}

//--------------------------------------------------------------------------------------------------
// Refresh + helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Triggers a full cache refresh on the worker.
 */
void Sessions::DatabaseManager::refreshSessionList()
{
  if (!isOpen())
    return;

  QMetaObject::invokeMethod(m_worker, "refreshAll", Qt::QueuedConnection);
}

/**
 * @brief Returns the next mutation token used to correlate worker replies.
 */
quint64 Sessions::DatabaseManager::nextToken()
{
  return ++m_nextToken;
}

/**
 * @brief Toggles the busy counter and emits busyChanged on transitions.
 */
void Sessions::DatabaseManager::setBusy(bool busy)
{
  const bool was = m_outstandingMutations > 0;
  if (busy)
    ++m_outstandingMutations;
  else if (m_outstandingMutations > 0)
    --m_outstandingMutations;

  const bool now = m_outstandingMutations > 0;
  if (was != now)
    Q_EMIT busyChanged();
}

//--------------------------------------------------------------------------------------------------
// Worker reply slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Worker finished opening the file -- populate caches and emit signals.
 */
void Sessions::DatabaseManager::onWorkerOpened(const QString& filePath,
                                               const QVariantList& sessionList,
                                               const QVariantList& tagList,
                                               bool locked,
                                               const QString& passwordHash)
{
  m_filePath        = filePath;
  m_open            = true;
  m_sessionList     = sessionList;
  m_tagList         = tagList;
  const bool wasLck = m_locked;
  m_locked          = locked;
  m_passwordHash    = passwordHash;

  m_settings.setValue("Sessions/LastDatabase", filePath);

  setBusy(false);

  Q_EMIT openChanged();
  Q_EMIT sessionsChanged();
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
  if (m_locked != wasLck)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker reported an open failure -- surface it to the user.
 */
void Sessions::DatabaseManager::onWorkerOpenFailed(const QString& filePath, const QString& error)
{
  Q_UNUSED(filePath)
  setBusy(false);

  Misc::Utilities::showMessageBox(tr("Cannot open session file"), error, QMessageBox::Critical);
}

/**
 * @brief Worker confirmed the file is closed -- clear caches and notify QML.
 */
void Sessions::DatabaseManager::onWorkerClosed()
{
  const bool wasOpen   = m_open;
  const bool wasLocked = m_locked;

  m_open = false;
  m_filePath.clear();
  m_sessionList.clear();
  m_tagList.clear();
  m_passwordHash.clear();
  m_locked            = false;
  m_selectedSessionId = -1;

  if (wasOpen)
    Q_EMIT openChanged();

  Q_EMIT sessionsChanged();
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
  if (wasLocked)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker shipped a refreshed session list -- rebroadcast to QML.
 */
void Sessions::DatabaseManager::onWorkerSessionListRefreshed(const QVariantList& sessionList)
{
  m_sessionList = sessionList;
  Q_EMIT sessionsChanged();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker shipped a refreshed tag list -- rebroadcast to QML.
 */
void Sessions::DatabaseManager::onWorkerTagListRefreshed(const QVariantList& tagList)
{
  m_tagList = tagList;
  Q_EMIT tagsChanged();
  Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker reported a lock-state change -- update local mirror.
 */
void Sessions::DatabaseManager::onWorkerLockStateChanged(bool locked, const QString& passwordHash)
{
  const bool was = m_locked;
  m_locked       = locked;
  m_passwordHash = passwordHash;
  if (was != locked)
    Q_EMIT lockedChanged();
}

/**
 * @brief Worker confirmed the notes write -- propagate to any QML waiting on signal.
 */
void Sessions::DatabaseManager::onWorkerNotesUpdated(int sessionId, const QString& notes)
{
  Q_UNUSED(notes)
  if (sessionId == m_selectedSessionId)
    Q_EMIT selectedSessionChanged();
}

/**
 * @brief Worker finished a mutation -- clear the busy bit and report errors.
 */
void Sessions::DatabaseManager::onWorkerMutationFinished(quint64 token,
                                                         bool ok,
                                                         const QString& error)
{
  Q_UNUSED(token)
  setBusy(false);

  if (!ok && !error.isEmpty())
    qWarning() << "[Sessions] mutation failed:" << error;
}

/**
 * @brief Worker is streaming CSV -- update the percentage cache.
 */
void Sessions::DatabaseManager::onWorkerCsvProgress(double percent)
{
  m_csvExportProgress = percent;
  Q_EMIT csvExportProgressChanged();
}

/**
 * @brief Worker finished writing the CSV -- report status, reveal in the OS shell.
 */
void Sessions::DatabaseManager::onWorkerCsvFinished(const QString& outputPath,
                                                    bool ok,
                                                    const QString& error)
{
  Q_UNUSED(error)
  m_csvExportBusy     = false;
  m_csvExportProgress = ok ? 1.0 : 0.0;
  m_pendingCsvPath.clear();
  Q_EMIT csvExportBusyChanged();
  Q_EMIT csvExportProgressChanged();
  Q_EMIT csvExportFinished(outputPath, ok);

  if (ok)
    Misc::Utilities::revealFile(outputPath);
}

/**
 * @brief Worker shipped the report data bundle -- kick off rendering on the main thread.
 */
void Sessions::DatabaseManager::onWorkerReportDataReady(const ReportPayloadPtr& payload)
{
  renderReportFromPayload(payload);
}

/**
 * @brief Worker handed back the global project JSON -- finish the restore flow.
 */
void Sessions::DatabaseManager::onWorkerGlobalProjectJsonReady(const QString& json)
{
  runRestoreProjectFromJson(json);
}

//--------------------------------------------------------------------------------------------------
// Schema (shared with Sessions::Export at session creation time)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates or upgrades the session-log schema on the open database.
 */
void Sessions::DatabaseManager::createSchema(QSqlQuery& q)
{
  createSchemaSessionTables(q);
  migrateColumnsTable(q);
  createSchemaSampleTables(q);
  createSchemaTagTables(q);
  createSchemaProjectMetadata(q);
}

/**
 * @brief Creates the sessions header and columns metadata tables.
 */
void Sessions::DatabaseManager::createSchemaSessionTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS sessions ("
         "  session_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  project_title TEXT NOT NULL,"
         "  started_at    TEXT NOT NULL,"
         "  ended_at      TEXT,"
         "  project_json  TEXT,"
         "  notes         TEXT"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS columns ("
         "  column_id    INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL REFERENCES sessions,"
         "  unique_id    INTEGER NOT NULL,"
         "  source_id    INTEGER NOT NULL DEFAULT 0,"
         "  source_title TEXT NOT NULL DEFAULT '',"
         "  group_title  TEXT NOT NULL,"
         "  title        TEXT NOT NULL,"
         "  units        TEXT,"
         "  widget       TEXT,"
         "  is_virtual   INTEGER NOT NULL DEFAULT 0"
         ")");
}

/**
 * @brief Adds source_id / source_title to legacy columns tables in older databases.
 */
void Sessions::DatabaseManager::migrateColumnsTable(QSqlQuery& q)
{
  auto columnExists = [&q](const QString& column) {
    if (!q.exec(QStringLiteral("PRAGMA table_info(\"columns\")"))) {
      qWarning() << "[Sessions] PRAGMA table_info failed:" << q.lastError().text();
      return false;
    }
    while (q.next())
      if (q.value(1).toString().compare(column, Qt::CaseInsensitive) == 0)
        return true;

    return false;
  };

  if (!columnExists(QStringLiteral("source_id"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_id INTEGER NOT NULL DEFAULT 0"))
      qWarning() << "[Sessions] ALTER add source_id failed:" << q.lastError().text();
  }

  if (!columnExists(QStringLiteral("source_title"))) {
    if (!q.exec("ALTER TABLE \"columns\" ADD COLUMN source_title TEXT NOT NULL DEFAULT ''"))
      qWarning() << "[Sessions] ALTER add source_title failed:" << q.lastError().text();
  }
}

/**
 * @brief Creates the per-sample tables (readings, raw_bytes, table_snapshots) and indexes.
 */
void Sessions::DatabaseManager::createSchemaSampleTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS readings ("
         "  reading_id          INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id          INTEGER NOT NULL,"
         "  timestamp_ns        INTEGER NOT NULL,"
         "  unique_id           INTEGER NOT NULL,"
         "  raw_numeric_value   REAL,"
         "  raw_string_value    TEXT,"
         "  final_numeric_value REAL,"
         "  final_string_value  TEXT,"
         "  is_numeric          INTEGER NOT NULL DEFAULT 1"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_uid_ts "
         "ON readings (session_id, unique_id, timestamp_ns)");
  q.exec("CREATE INDEX IF NOT EXISTS idx_readings_session_ts "
         "ON readings (session_id, timestamp_ns)");

  q.exec("CREATE TABLE IF NOT EXISTS raw_bytes ("
         "  raw_id       INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id   INTEGER NOT NULL,"
         "  timestamp_ns INTEGER NOT NULL,"
         "  device_id    INTEGER NOT NULL DEFAULT 0,"
         "  data         BLOB NOT NULL"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_raw_bytes_session_ts "
         "ON raw_bytes (session_id, timestamp_ns)");

  q.exec("CREATE TABLE IF NOT EXISTS table_snapshots ("
         "  snapshot_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  session_id    INTEGER NOT NULL,"
         "  timestamp_ns  INTEGER NOT NULL,"
         "  table_name    TEXT NOT NULL,"
         "  register_name TEXT NOT NULL,"
         "  numeric_value REAL,"
         "  string_value  TEXT"
         ")");
  q.exec("CREATE INDEX IF NOT EXISTS idx_snapshots_session_ts "
         "ON table_snapshots (session_id, timestamp_ns, table_name)");
}

/**
 * @brief Creates the tags catalog and the session -> tag join table.
 */
void Sessions::DatabaseManager::createSchemaTagTables(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS tags ("
         "  tag_id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  label  TEXT NOT NULL UNIQUE COLLATE NOCASE"
         ")");

  q.exec("CREATE TABLE IF NOT EXISTS session_tags ("
         "  session_id INTEGER NOT NULL,"
         "  tag_id     INTEGER NOT NULL,"
         "  PRIMARY KEY (session_id, tag_id)"
         ") WITHOUT ROWID");
}

/**
 * @brief Creates the project_metadata key/value store.
 */
void Sessions::DatabaseManager::createSchemaProjectMetadata(QSqlQuery& q)
{
  q.exec("CREATE TABLE IF NOT EXISTS project_metadata ("
         "  key   TEXT PRIMARY KEY,"
         "  value TEXT NOT NULL"
         ") WITHOUT ROWID");
}

#endif  // BUILD_COMMERCIAL
