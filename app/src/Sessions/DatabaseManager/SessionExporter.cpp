/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/DatabaseManager/SessionExporter.h"

#  include <QApplication>
#  include <QDir>
#  include <QFileDialog>
#  include <QVariantList>

#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"
#  include "Sessions/DatabaseWorker.h"
#  include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Path helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scrubs a project title for use as a folder/file name component.
 */
QString Sessions::sanitiseTitleForPath(const QString& title)
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

  int keep = 0;
  for (int i = safe.size(); i > 0; --i) {
    const QChar c = safe.at(i - 1);
    if (c != QChar('.') && c != QChar(' ')) {
      keep = i;
      break;
    }
  }
  safe.truncate(keep);

  if (safe.isEmpty())
    safe = QStringLiteral("Untitled");

  return safe;
}

//--------------------------------------------------------------------------------------------------
// Construction & wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle exporter; the worker and workspace arrive through their setters.
 */
Sessions::SessionExporter::SessionExporter(QObject* parent)
  : QObject(parent)
  , m_csvBusy(false)
  , m_pdfBusy(false)
  , m_pendingPdfActive(false)
  , m_pendingPdfSessionId(-1)
  , m_csvProgress(0.0)
  , m_pdfProgress(0.0)
  , m_worker(nullptr)
  , m_workspaceManager(nullptr)
{}

/**
 * @brief Adopts the database worker every export dispatches through.
 */
void Sessions::SessionExporter::setWorker(DatabaseWorker* worker)
{
  m_worker = worker;
}

/**
 * @brief Adopts the workspace the suggested output folders are derived from.
 */
void Sessions::SessionExporter::setWorkspace(Misc::WorkspaceManager* workspace)
{
  m_workspaceManager = workspace;
}

//--------------------------------------------------------------------------------------------------
// Export state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true while a CSV export worker is running.
 */
bool Sessions::SessionExporter::csvBusy() const noexcept
{
  return m_csvBusy;
}

/**
 * @brief Returns @c true while a PDF report is being rendered.
 */
bool Sessions::SessionExporter::pdfBusy() const noexcept
{
  return m_pdfBusy;
}

/**
 * @brief Returns the active CSV export's progress as a fraction in [0, 1].
 */
double Sessions::SessionExporter::csvProgress() const noexcept
{
  return m_csvProgress;
}

/**
 * @brief Returns the active PDF export's progress as a fraction in [0, 1].
 */
double Sessions::SessionExporter::pdfProgress() const noexcept
{
  return m_pdfProgress;
}

/**
 * @brief Returns the user-facing status label for the active report export.
 */
const QString& Sessions::SessionExporter::pdfStatus() const noexcept
{
  return m_pdfStatus;
}

//--------------------------------------------------------------------------------------------------
// CSV export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks an output path and dispatches a streaming CSV export to the worker.
 */
void Sessions::SessionExporter::exportToCsv(int sessionId, const QString& projectTitle)
{
  if (m_csvBusy)
    return;

  SS_ASSERT(m_worker != nullptr, return);
  SS_ASSERT(m_workspaceManager != nullptr, return);

  const QString safeProj = sanitiseTitleForPath(projectTitle);
  const QString dir      = QStringLiteral("%1/%2").arg(m_workspaceManager->path("CSV"), safeProj);
  QDir().mkpath(dir);

  const QString suggested =
    QStringLiteral("%1/session_%2.csv").arg(dir, QString::number(sessionId));
  const auto path = QFileDialog::getSaveFileName(
    nullptr, tr("Export Session to CSV"), suggested, tr("CSV files (*.csv)"));
  if (path.isEmpty())
    return;

  m_csvBusy        = true;
  m_csvProgress    = 0.0;
  m_pendingCsvPath = path;
  Q_EMIT csvBusyChanged();
  Q_EMIT csvProgressChanged();

  QMetaObject::invokeMethod(
    m_worker, "runCsvExport", Qt::QueuedConnection, Q_ARG(int, sessionId), Q_ARG(QString, path));
}

/**
 * @brief Worker is streaming CSV -- update the percentage cache.
 */
void Sessions::SessionExporter::onCsvProgress(double percent)
{
  m_csvProgress = percent;
  Q_EMIT csvProgressChanged();
}

/**
 * @brief Worker finished writing the CSV -- report status, reveal in the OS shell.
 */
void Sessions::SessionExporter::onCsvFinished(const QString& outputPath,
                                              bool ok,
                                              const QString& error)
{
  Q_UNUSED(error)
  m_csvBusy     = false;
  m_csvProgress = ok ? 1.0 : 0.0;
  m_pendingCsvPath.clear();
  Q_EMIT csvBusyChanged();
  Q_EMIT csvProgressChanged();
  Q_EMIT csvFinished(outputPath, ok);

  if (ok)
    Misc::Utilities::revealFile(outputPath);
}

//--------------------------------------------------------------------------------------------------
// PDF / HTML report export
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates QML options + path picker into a worker fetch + main-thread render.
 */
void Sessions::SessionExporter::exportToPdf(int sessionId,
                                            const QVariantMap& options,
                                            const QString& projectTitle)
{
  if (m_pdfBusy)
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
  opts.lineWidth           = SerialStudio::toDouble(options.value("lineWidth", 1.4));
  opts.lineStyle           = options.value("lineStyle", QStringLiteral("solid")).toString();

  const auto selList = options.value("selectedUniqueIds").toList();
  opts.selectedUniqueIds.reserve(selList.size());
  for (const auto& v : selList)
    opts.selectedUniqueIds.push_back(v.toInt());

  const QString fmtStr = options.value("outputFormat", QStringLiteral("pdf")).toString().toLower();
  if (fmtStr == QStringLiteral("html"))
    opts.format = HtmlReportOptions::Format::Html;
  else if (fmtStr == QStringLiteral("both"))
    opts.format = HtmlReportOptions::Format::Both;
  else
    opts.format = HtmlReportOptions::Format::Pdf;

#  ifndef SERIAL_STUDIO_WITH_WEBENGINE
  opts.format = HtmlReportOptions::Format::Html;
#  endif

  if (!opts.outputPath.isEmpty()) {
    launchPdfExport(sessionId, std::move(opts));
    return;
  }

  requestPdfOutputPath(sessionId, std::move(opts), projectTitle);
}

/**
 * @brief Stages PDF render context and asks the worker for the session payload.
 */
void Sessions::SessionExporter::launchPdfExport(int sessionId, HtmlReportOptions opts)
{
  SS_ASSERT(m_worker != nullptr, return);

  m_pendingPdfOpts      = std::move(opts);
  m_pendingPdfSessionId = sessionId;
  m_pendingPdfActive    = true;

  const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);
  if (reportBusy) {
    m_pdfBusy     = true;
    m_pdfProgress = 0.0;
    m_pdfStatus   = tr("Loading session data…");
    Q_EMIT pdfBusyChanged();
    Q_EMIT pdfProgressChanged();
  }

  QVariantList selectedUniqueIds;
  selectedUniqueIds.reserve(static_cast<int>(m_pendingPdfOpts.selectedUniqueIds.size()));
  for (const int uid : m_pendingPdfOpts.selectedUniqueIds)
    selectedUniqueIds.append(uid);

  QMetaObject::invokeMethod(m_worker,
                            "runReportDataLoad",
                            Qt::QueuedConnection,
                            Q_ARG(int, sessionId),
                            Q_ARG(bool, m_pendingPdfOpts.includeCharts),
                            Q_ARG(int, 10000),
                            Q_ARG(QVariantList, selectedUniqueIds));
}

/**
 * @brief Opens a Save dialog for the report path and launches the export on accept.
 */
void Sessions::SessionExporter::requestPdfOutputPath(int sessionId,
                                                     HtmlReportOptions opts,
                                                     const QString& projectTitle)
{
  const bool wantsPdf  = (opts.format != HtmlReportOptions::Format::Html);
  const QString ext    = wantsPdf ? QStringLiteral("pdf") : QStringLiteral("html");
  const QString title  = wantsPdf ? tr("Save PDF Report") : tr("Save HTML Report");
  const QString filter = wantsPdf ? tr("PDF files (*.pdf)") : tr("HTML files (*.html)");

  SS_ASSERT(m_workspaceManager != nullptr, return);

  const QString safeProj = sanitiseTitleForPath(projectTitle);
  const QString dir = QStringLiteral("%1/%2").arg(m_workspaceManager->path("Reports"), safeProj);
  QDir().mkpath(dir);

  const QString baseName  = opts.documentTitle.isEmpty()
                            ? QStringLiteral("session_%1").arg(sessionId)
                            : sanitiseTitleForPath(opts.documentTitle);
  const QString suggested = QStringLiteral("%1/%2.%3").arg(dir, baseName, ext);

  auto* dialog = new QFileDialog(qApp->activeWindow(), title, suggested, filter);
  dialog->setAcceptMode(QFileDialog::AcceptSave);
  dialog->setFileMode(QFileDialog::AnyFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog,
          &QFileDialog::fileSelected,
          this,
          [this, opts, ext, sessionId](const QString& path) mutable {
            if (path.isEmpty()) {
              Q_EMIT pdfFinished(QString(), false);
              return;
            }

            QMetaObject::invokeMethod(
              this,
              [this, opts = std::move(opts), ext, sessionId, path]() mutable {
                QString finalPath = path;
                const QString dot = QStringLiteral(".") + ext;
                if (!finalPath.endsWith(dot, Qt::CaseInsensitive))
                  finalPath += dot;

                opts.outputPath = finalPath;
                launchPdfExport(sessionId, std::move(opts));
              },
              Qt::QueuedConnection);
          });

  connect(dialog, &QFileDialog::rejected, this, [this] { Q_EMIT pdfFinished(QString(), false); });

  dialog->open();
}

/**
 * @brief Worker shipped the report data bundle -- kick off rendering on the main thread.
 */
void Sessions::SessionExporter::onReportDataReady(const ReportPayloadPtr& payload)
{
  renderReportFromPayload(payload);
}

/**
 * @brief Continues the PDF flow on the main thread once the worker has shipped data.
 */
void Sessions::SessionExporter::renderReportFromPayload(const ReportPayloadPtr& payload)
{
  if (!m_pendingPdfActive)
    return;

  if (!payload || payload->sessionId != m_pendingPdfSessionId)
    return;

  if (!payload->ok) {
    if (m_pdfBusy) {
      m_pdfBusy     = false;
      m_pdfProgress = 0.0;
      m_pdfStatus   = tr("Failed");
      Q_EMIT pdfBusyChanged();
      Q_EMIT pdfProgressChanged();
    }

    Misc::Utilities::showMessageBox(tr("Report Failed"),
                                    payload->error.isEmpty() ? tr("Could not generate the report.")
                                                             : payload->error,
                                    QMessageBox::Warning);

    Q_EMIT pdfFinished(QString(), false);
    m_pendingPdfActive = false;
    return;
  }

  const bool reportBusy = (m_pendingPdfOpts.format != HtmlReportOptions::Format::Html);

  auto* renderer = new HtmlReport(this);

  if (reportBusy) {
    connect(renderer, &HtmlReport::progress, this, [this](const QString& status, double percent) {
      m_pdfStatus   = status;
      m_pdfProgress = percent;
      Q_EMIT pdfProgressChanged();
    });
  }

  connect(renderer,
          &HtmlReport::finished,
          this,
          [this, renderer, reportBusy](const QString& outputPath, bool ok, const QString& error) {
            if (reportBusy) {
              m_pdfBusy     = false;
              m_pdfProgress = 1.0;
              m_pdfStatus   = ok ? tr("Done") : tr("Failed");
              Q_EMIT pdfBusyChanged();
              Q_EMIT pdfProgressChanged();
            }
            Q_EMIT pdfFinished(outputPath, ok);

            if (ok) {
              Misc::Utilities::revealFile(outputPath);
            } else {
              Misc::Utilities::showMessageBox(tr("Report Failed"),
                                              error.isEmpty() ? tr("Could not generate the report.")
                                                              : error,
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
void Sessions::SessionExporter::pickReportLogo()
{
  auto* dialog = new QFileDialog(qApp->activeWindow(),
                                 tr("Select logo image"),
                                 QString(),
                                 tr("Images (*.png *.jpg *.jpeg *.svg)"));
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this, [this, path]() { Q_EMIT logoPicked(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

//--------------------------------------------------------------------------------------------------
// Export-dialog queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Asks the worker thread to enumerate a session's datasets for the selection UI.
 */
void Sessions::SessionExporter::requestDatasets(int sessionId)
{
  SS_ASSERT(m_worker != nullptr, return);

  QMetaObject::invokeMethod(
    m_worker, "runDatasetListLoad", Qt::QueuedConnection, Q_ARG(int, sessionId));
}

/**
 * @brief Asks the worker thread to summarise a session's recorded stream data (spec 0054).
 */
void Sessions::SessionExporter::requestStreamStats(int sessionId)
{
  SS_ASSERT(m_worker != nullptr, return);

  QMetaObject::invokeMethod(
    m_worker, "runStreamStatsLoad", Qt::QueuedConnection, Q_ARG(int, sessionId));
}

#endif
