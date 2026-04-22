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

#  include <QObject>
#  include <QPageLayout>
#  include <QPageSize>
#  include <QString>
#  include <vector>

#  include "Sessions/ReportData.h"

class QWebEnginePage;

namespace Sessions {

/**
 * @brief Caller-supplied rendering options (branding, paper, output format).
 *
 * Fields mirror the QVariantMap sent from the QML options dialog — keep POD-
 * friendly so the adapter in @c DatabaseManager stays a straight
 * field-by-field copy.
 */
struct HtmlReportOptions {
  enum class Format {
    Pdf,
    Html,
    Both
  };

  QString outputPath;
  QString companyName;
  QString documentTitle;
  QString authorName;
  QString logoPath;
  QPageSize::PageSizeId pageSize;
  QPageLayout::Orientation orientation;
  bool includeCover;
  bool includeMetadata;
  bool includeStats;
  bool includeCharts;
  double lineWidth;   // stroke width in CSS pixels (1.4 default)
  QString lineStyle;  // "solid" | "dashed" | "dotted"
  Format format;
};

/**
 * @brief Renders a session report as a self-contained HTML document and,
 *        optionally, as a PDF produced by printing that HTML.
 *
 * The HTML is built from a resource-embedded template with
 * ``{{PLACEHOLDER}}`` expansion. Chart.js, the runtime JS, the stylesheet
 * and the Serial Studio brand icon are all inlined at render time so the
 * output works offline and travels as a single file.
 *
 * For PDF output the renderer drives an off-screen @c QWebEnginePage,
 * waits for the client-side runtime to flip ``window.__reportReady``, then
 * calls ``printToPdf()``. Because that call is asynchronous the renderer is
 * a @c QObject that emits @c finished() when the whole pipeline settles.
 *
 * @c HtmlReport instances are single-shot: call @c render() once and listen
 * for @c finished(). The caller owns the instance and should @c deleteLater
 * it from the @c finished slot.
 */
class HtmlReport : public QObject {
  Q_OBJECT

signals:
  void finished(const QString& outputPath, bool success);
  void progress(const QString& status, double percent);

public:
  explicit HtmlReport(QObject* parent = nullptr);
  ~HtmlReport() override;

  HtmlReport(HtmlReport&&)                 = delete;
  HtmlReport(const HtmlReport&)            = delete;
  HtmlReport& operator=(HtmlReport&&)      = delete;
  HtmlReport& operator=(const HtmlReport&) = delete;

  /**
   * @brief Start the render. Emits @c finished() exactly once.
   *
   * Returns immediately if the inputs are invalid (no path, invalid data).
   */
  void render(const ReportData& data,
              std::vector<DatasetSeries> series,
              const HtmlReportOptions& opts);

private:
  [[nodiscard]] QString buildHtml() const;
  [[nodiscard]] QString buildCoverSection() const;
  [[nodiscard]] QString buildMetadataSection() const;
  [[nodiscard]] QString buildSummarySection() const;
  [[nodiscard]] QString buildChartsSection() const;
  [[nodiscard]] QString buildReportDataJson() const;
  [[nodiscard]] QString buildPrintFooterLeft() const;
  [[nodiscard]] QString pageSizeCssValue() const;
  [[nodiscard]] QString pageSizeCssLandscape() const;
  [[nodiscard]] QString encodeLogoAsDataUri(const QString& path) const;

  [[nodiscard]] static QString readResource(const QString& path);

  void writeHtmlArtifact(const QString& htmlPath, const QString& html, bool& success);
  void startPdfRender(const QString& html, const QString& pdfPath);
  void startPrinting();
  void probeReadiness();

private slots:
  void onLoadFinished(bool ok);
  void onPdfPrintingFinished(const QString& filePath, bool success);

private:
  ReportData m_data;
  std::vector<DatasetSeries> m_series;
  HtmlReportOptions m_opts;

  QString m_htmlCache;
  QString m_pdfPath;
  QString m_htmlPath;
  bool m_htmlWritten;

  QWebEnginePage* m_page;
  bool m_printStarted;
  int m_readinessAttempts;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
