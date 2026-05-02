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
#  include <QSizeF>
#  include <QString>
#  include <vector>

#  include "Sessions/ReportData.h"

class QWebEnginePage;

namespace Sessions {

/**
 * @brief Caller-supplied HTML / PDF session-report rendering options.
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
  bool includeCover;
  bool includeMetadata;
  bool includeStats;
  bool includeCharts;
  bool includeStatsOverlay;
  double lineWidth;
  QString lineStyle;
  Format format;
};

/**
 * @brief Renders a session report as HTML and optionally prints it to PDF.
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

  void render(const ReportData& data,
              std::vector<DatasetSeries> series,
              const HtmlReportOptions& opts);

private:
  [[nodiscard]] QString buildHtml() const;
  [[nodiscard]] QString buildCoverSection() const;
  [[nodiscard]] QString buildCoverLogoMarkup() const;
  [[nodiscard]] QString buildMetadataSection() const;
  [[nodiscard]] QString buildSummarySection() const;
  [[nodiscard]] QString buildChartsSection() const;
  [[nodiscard]] QString buildReportDataJson() const;
  [[nodiscard]] QString buildPrintFooterLeft() const;
  [[nodiscard]] QString buildPrintFooterRight() const;
  [[nodiscard]] QString pageSizeCssValue() const;
  [[nodiscard]] QSizeF chartPagePrintableSize() const;
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
