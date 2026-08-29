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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QObject>
#  include <QString>
#  include <QVariantMap>

#  include "Sessions/DatabaseWorker.h"
#  include "Sessions/HtmlReport.h"
#  include "Sessions/ReportData.h"

namespace Misc {
class WorkspaceManager;
}  // namespace Misc

namespace Sessions {

/**
 * @brief Scrubs a project title for use as a folder/file name component. Shared by every path the
 *        historian derives from a project title, so an archive and its exports never disagree on
 *        what a title spells.
 */
[[nodiscard]] QString sanitiseTitleForPath(const QString& title);

/**
 * @brief Everything the historian ships out of an archive: the streaming CSV export, the PDF/HTML
 *        report, and the dataset/stream enumerations the export dialog is built from. Each one is
 *        a two-step round trip (worker thread reads, main thread writes), so the pending render
 *        context and both busy/progress pairs live here and one class starts and finishes a job.
 */
class SessionExporter : public QObject {
  Q_OBJECT

signals:
  void logoPicked(const QString& path);
  void csvBusyChanged();
  void csvProgressChanged();
  void csvFinished(const QString& outputPath, bool success);
  void pdfBusyChanged();
  void pdfProgressChanged();
  void pdfFinished(const QString& outputPath, bool success);

public:
  explicit SessionExporter(QObject* parent = nullptr);

  [[nodiscard]] bool csvBusy() const noexcept;
  [[nodiscard]] bool pdfBusy() const noexcept;
  [[nodiscard]] double csvProgress() const noexcept;
  [[nodiscard]] double pdfProgress() const noexcept;
  [[nodiscard]] const QString& pdfStatus() const noexcept;

  void setWorker(DatabaseWorker* worker);
  void setWorkspace(Misc::WorkspaceManager* workspace);

public slots:
  void pickReportLogo();
  void requestDatasets(int sessionId);
  void requestStreamStats(int sessionId);
  void exportToCsv(int sessionId, const QString& projectTitle);
  void exportToPdf(int sessionId, const QVariantMap& options, const QString& projectTitle);

  void onCsvProgress(double percent);
  void onCsvFinished(const QString& outputPath, bool ok, const QString& error);
  void onReportDataReady(const Sessions::ReportPayloadPtr& payload);

private:
  void launchPdfExport(int sessionId, HtmlReportOptions opts);
  void requestPdfOutputPath(int sessionId, HtmlReportOptions opts, const QString& projectTitle);
  void renderReportFromPayload(const ReportPayloadPtr& payload);

private:
  bool m_csvBusy;
  bool m_pdfBusy;
  bool m_pendingPdfActive;
  int m_pendingPdfSessionId;
  double m_csvProgress;
  double m_pdfProgress;
  QString m_pdfStatus;
  QString m_pendingCsvPath;
  HtmlReportOptions m_pendingPdfOpts;
  DatabaseWorker* m_worker;
  Misc::WorkspaceManager* m_workspaceManager;
};

}  // namespace Sessions

#endif
