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

#ifdef BUILD_COMMERCIAL

#  include "Sessions/HtmlReport.h"

#  include <algorithm>
#  include <cmath>
#  include <map>
#  include <QByteArray>
#  include <QCoreApplication>
#  include <QDateTime>
#  include <QFile>
#  include <QFileInfo>
#  include <QImage>
#  include <QJsonArray>
#  include <QJsonDocument>
#  include <QJsonObject>
#  include <QMarginsF>
#  include <QMimeDatabase>
#  include <QPageLayout>
#  include <QPageSize>
#  include <QtDebug>
#  include <QTextStream>
#  include <QTimer>
#  include <QUrl>
#  include <QWebEnginePage>
#  include <QWebEngineProfile>

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Escapes user-supplied text for safe placement in HTML body content.
 *
 * Covers the five characters that can break HTML parsing. Placeholder values
 * come from project titles / notes / tag labels so treating them all as
 * untrusted input is the safe default.
 */
static QString escapeHtml(const QString& in)
{
  QString out;
  out.reserve(in.size());
  for (const QChar& c : in) {
    switch (c.unicode()) {
      case '&':
        out += QStringLiteral("&amp;");
        break;
      case '<':
        out += QStringLiteral("&lt;");
        break;
      case '>':
        out += QStringLiteral("&gt;");
        break;
      case '"':
        out += QStringLiteral("&quot;");
        break;
      case '\'':
        out += QStringLiteral("&#39;");
        break;
      default:
        out += c;
    }
  }
  return out;
}

/**
 * @brief Formats a double for tabular display (used in the cell text + the
 *        data-sort-value attribute that drives the sortable table comparator).
 */
static QString formatNumber(double v)
{
  if (!std::isfinite(v))
    return QStringLiteral("—");

  const double abs_v = std::fabs(v);
  if (abs_v != 0.0 && (abs_v < 1.0e-3 || abs_v >= 1.0e6))
    return QString::number(v, 'e', 2);

  return QString::number(v, 'f', 3);
}

/**
 * @brief Formats a millisecond duration as HH:MM:SS.
 */
static QString formatDuration(qint64 ms)
{
  if (ms <= 0)
    return QStringLiteral("—");

  const qint64 totalSeconds = ms / 1000;
  const qint64 hours        = totalSeconds / 3600;
  const qint64 minutes      = (totalSeconds / 60) % 60;
  const qint64 seconds      = totalSeconds % 60;

  return QString::asprintf("%02lld:%02lld:%02lld",
                           static_cast<long long>(hours),
                           static_cast<long long>(minutes),
                           static_cast<long long>(seconds));
}

/**
 * @brief Re-formats an ISO 8601 timestamp in the DB Explorer's display style.
 */
static QString formatDate(const QString& iso)
{
  if (iso.isEmpty())
    return QStringLiteral("—");

  const auto dt = QDateTime::fromString(iso, Qt::ISODate);
  if (!dt.isValid())
    return iso;

  return dt.toString(QStringLiteral("MMM d, yyyy — h:mm:ss AP"));
}

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates an idle renderer. @c render() kicks the pipeline.
 */
Sessions::HtmlReport::HtmlReport(QObject* parent)
  : QObject(parent)
  , m_htmlWritten(false)
  , m_page(nullptr)
  , m_printStarted(false)
  , m_readinessAttempts(0)
{}

/**
 * @brief Deletes the off-screen WebEngine page, if one was spun up.
 */
Sessions::HtmlReport::~HtmlReport()
{
  if (m_page) {
    m_page->deleteLater();
    m_page = nullptr;
  }
}

//--------------------------------------------------------------------------------------------------
// Resource loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a UTF-8 text resource from the Qt resource system.
 *
 * Returns an empty string when the resource is missing or unreadable. Callers
 * surface the failure via the final @c finished() signal.
 */
QString Sessions::HtmlReport::readResource(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return QString();

  return QString::fromUtf8(file.readAll());
}

//--------------------------------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------------------------------

/**
 * @brief Orchestrates the render.
 *
 * Always writes the HTML artifact when the format asks for it, then starts
 * the PDF printing pipeline when requested. Exactly one @c finished() is
 * emitted per call — regardless of which path runs.
 */
void Sessions::HtmlReport::render(const ReportData& data,
                                  std::vector<DatasetSeries> series,
                                  const HtmlReportOptions& opts)
{
  // Guard against invalid inputs up front
  if (!data.valid || opts.outputPath.isEmpty()) {
    Q_EMIT finished(QString(), false);
    return;
  }

  // Capture inputs for later (async PDF path needs them alive)
  m_data   = data;
  m_series = std::move(series);
  m_opts   = opts;

  Q_EMIT progress(tr("Assembling report…"), 0.10);

  // Build the single HTML string — used for both outputs
  m_htmlCache = buildHtml();
  if (m_htmlCache.isEmpty()) {
    Q_EMIT finished(QString(), false);
    return;
  }

  Q_EMIT progress(tr("Writing output…"), 0.30);

  // Resolve sibling output paths (e.g. foo.pdf → foo.html for "Both" mode)
  const QFileInfo outInfo(opts.outputPath);
  const QString stem = outInfo.completeBaseName();
  const QString dir  = outInfo.absolutePath();
  m_pdfPath          = opts.outputPath.endsWith(".pdf", Qt::CaseInsensitive)
                       ? opts.outputPath
                       : QStringLiteral("%1/%2.pdf").arg(dir, stem);
  m_htmlPath         = opts.outputPath.endsWith(".html", Qt::CaseInsensitive)
                       ? opts.outputPath
                       : QStringLiteral("%1/%2.html").arg(dir, stem);

  // Write the HTML artifact whenever the user asked for it
  bool htmlOk = true;
  if (opts.format == HtmlReportOptions::Format::Html
      || opts.format == HtmlReportOptions::Format::Both) {
    writeHtmlArtifact(m_htmlPath, m_htmlCache, htmlOk);
    m_htmlWritten = htmlOk;
  }

  // HTML-only path finishes synchronously
  if (opts.format == HtmlReportOptions::Format::Html) {
    Q_EMIT finished(m_htmlPath, htmlOk);
    return;
  }

  // Otherwise fire the async PDF render (fails fast if HTML write errored)
  if (!htmlOk && opts.format == HtmlReportOptions::Format::Both) {
    Q_EMIT finished(m_htmlPath, false);
    return;
  }

  startPdfRender(m_htmlCache, m_pdfPath);
}

//--------------------------------------------------------------------------------------------------
// HTML assembly
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the template + stylesheet + runtime + Chart.js and expands
 *        every ``{{PLACEHOLDER}}`` with the concrete value.
 *
 * Placeholder substitution is a simple multi-pass @c QString::replace —
 * safe because template keys are unambiguous and never user-controlled.
 */
QString Sessions::HtmlReport::buildHtml() const
{
  // Load static template assets from qrc
  QString html      = readResource(QStringLiteral(":/rcc/templates/reports/session-report.html"));
  const QString css = readResource(QStringLiteral(":/rcc/templates/reports/session-report.css"));
  const QString js  = readResource(QStringLiteral(":/rcc/templates/reports/session-report.js"));
  const QString chartJs  = readResource(QStringLiteral(":/rcc/chartjs/chart.umd.min.js"));
  const QString brandSvg = readResource(QStringLiteral(":/rcc/logo/icon.svg"));

  if (html.isEmpty() || css.isEmpty() || js.isEmpty() || chartJs.isEmpty())
    return QString();

  // Per-section HTML fragments (skipped sections leave an empty string)
  const QString coverHtml    = m_opts.includeCover ? buildCoverSection() : QString();
  const QString metadataHtml = m_opts.includeMetadata ? buildMetadataSection() : QString();
  const QString summaryHtml  = m_opts.includeStats ? buildSummarySection() : QString();
  const QString chartsHtml =
    (m_opts.includeCharts && !m_series.empty()) ? buildChartsSection() : QString();

  // Template expansion (multi-pass replace — one per placeholder)
  html.replace(QStringLiteral("{{CHART_JS}}"), chartJs);
  html.replace(QStringLiteral("{{REPORT_CSS}}"), css);
  html.replace(QStringLiteral("{{REPORT_JS}}"), js);
  html.replace(QStringLiteral("{{BRAND_ICON_SVG}}"), brandSvg);
  html.replace(
    QStringLiteral("{{DOCUMENT_TITLE}}"),
    escapeHtml(m_opts.documentTitle.isEmpty() ? tr("Session Report") : m_opts.documentTitle));
  html.replace(QStringLiteral("{{COVER_SECTION}}"), coverHtml);
  html.replace(QStringLiteral("{{METADATA_SECTION}}"), metadataHtml);
  html.replace(QStringLiteral("{{SUMMARY_SECTION}}"), summaryHtml);
  html.replace(QStringLiteral("{{CHARTS_SECTION}}"), chartsHtml);
  html.replace(QStringLiteral("{{REPORT_DATA_JSON}}"), buildReportDataJson());

  // Expand the @media print placeholders last
  html.replace(QStringLiteral("{{PAGE_SIZE_CSS}}"), pageSizeCssValue());
  html.replace(QStringLiteral("{{PRINT_FOOTER_LEFT}}"), buildPrintFooterLeft());
  html.replace(QStringLiteral("{{PRINT_FOOTER_RIGHT}}"), buildPrintFooterRight());

  // Pin the chart card to the landscape printable area
  const auto chartMm = chartPagePrintableSize();
  html.replace(QStringLiteral("{{CHART_PAGE_WIDTH_MM}}"), QString::number(chartMm.width(), 'f', 1));
  html.replace(QStringLiteral("{{CHART_PAGE_HEIGHT_MM}}"),
               QString::number(chartMm.height(), 'f', 1));

  return html;
}

//--------------------------------------------------------------------------------------------------
// Section: cover
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the first-page cover fragment: logo, company, title, subtitle.
 */
QString Sessions::HtmlReport::buildCoverSection() const
{
  // Resolve the logo markup — embed raster files as base64 data URIs so the
  // HTML stays self-contained; SVG files are inlined verbatim.
  QString logoMarkup;
  if (!m_opts.logoPath.isEmpty() && QFileInfo::exists(m_opts.logoPath)) {
    const QFileInfo fi(m_opts.logoPath);
    if (fi.suffix().compare("svg", Qt::CaseInsensitive) == 0) {
      logoMarkup = readResource(m_opts.logoPath);
      if (logoMarkup.isEmpty()) {
        QFile f(m_opts.logoPath);
        if (f.open(QIODevice::ReadOnly))
          logoMarkup = QString::fromUtf8(f.readAll());
      }
    } else {
      const QString dataUri = encodeLogoAsDataUri(m_opts.logoPath);
      if (!dataUri.isEmpty())
        logoMarkup = QStringLiteral("<img src=\"%1\" alt=\"\">").arg(dataUri);
    }
  }

  const QString title =
    escapeHtml(m_opts.documentTitle.isEmpty() ? tr("Session Report") : m_opts.documentTitle);

  const QString project =
    escapeHtml(m_data.projectTitle.isEmpty() ? tr("Untitled project") : m_data.projectTitle);

  const QString subtitleLeft = escapeHtml(formatDate(m_data.startedAt));

  // Footer strip at the bottom of the cover: author + generation timestamp
  QString authorLine;
  if (!m_opts.authorName.isEmpty())
    authorLine = QStringLiteral("<div>%1 <span class=\"author\">%2</span></div>")
                   .arg(escapeHtml(tr("Prepared by")), escapeHtml(m_opts.authorName));

  const QString generated =
    tr("Generated on %1")
      .arg(QDateTime::currentDateTime().toString(QStringLiteral("MMM d, yyyy h:mm AP")));

  const QString company =
    m_opts.companyName.isEmpty()
      ? QString()
      : QStringLiteral("<div class=\"company\">%1</div>").arg(escapeHtml(m_opts.companyName));

  const QString logoBlock = logoMarkup.isEmpty()
                            ? QString()
                            : QStringLiteral("<div class=\"logo\">%1</div>").arg(logoMarkup);

  // Cover facts strip — at-a-glance document identity
  const QString facts =
    QStringLiteral(
      "<div class=\"cover-facts\">"
      "<div><div class=\"fact-label\">%1</div><div class=\"fact-value\">#%2</div></div>"
      "<div><div class=\"fact-label\">%3</div><div class=\"fact-value\">%4</div></div>"
      "<div><div class=\"fact-label\">%5</div><div class=\"fact-value\">%6</div></div>"
      "<div><div class=\"fact-label\">%7</div><div class=\"fact-value\">%8</div></div>"
      "<div><div class=\"fact-label\">%9</div><div class=\"fact-value\">%10</div></div>"
      "<div><div class=\"fact-label\">%11</div><div class=\"fact-value\">%12</div></div>"
      "</div>")
      .arg(escapeHtml(tr("Test ID")),
           QString::number(m_data.sessionId),
           escapeHtml(tr("Duration")),
           escapeHtml(formatDuration(m_data.durationMs)),
           escapeHtml(tr("Samples")),
           QString::number(m_data.frameCount),
           escapeHtml(tr("Parameters")),
           QString::number(m_data.datasets.size()),
           escapeHtml(tr("Started")),
           escapeHtml(formatDate(m_data.startedAt)),
           escapeHtml(tr("Ended")),
           escapeHtml(formatDate(m_data.endedAt)));

  return QStringLiteral("<section class=\"cover\">"
                        "%1"
                        "<div class=\"cover-body\">"
                        "%2"
                        "<h1 class=\"title\">%3</h1>"
                        "<div class=\"subtitle\"><strong>%4</strong> &middot; %5</div>"
                        "%6"
                        "</div>"
                        "<div class=\"cover-footer\">"
                        "%7"
                        "<div>%8</div>"
                        "</div>"
                        "</section>")
    .arg(
      logoBlock, company, title, project, subtitleLeft, facts, authorLine, escapeHtml(generated));
}

//--------------------------------------------------------------------------------------------------
// Section: test information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the definition-grid block plus tags and notes.
 */
QString Sessions::HtmlReport::buildMetadataSection() const
{
  struct Row {
    QString label;
    QString value;
  };

  const std::vector<Row> rows = {
    {          tr("Project"), m_data.projectTitle.isEmpty() ? QStringLiteral("—") : m_data.projectTitle},
    {  tr("Test identifier"),                               QStringLiteral("#%1").arg(m_data.sessionId)},
    {       tr("Start time"),                                              formatDate(m_data.startedAt)},
    {         tr("End time"),                                                formatDate(m_data.endedAt)},
    {   tr("Total duration"),                                         formatDuration(m_data.durationMs)},
    { tr("Samples acquired"),                                        QString::number(m_data.frameCount)},
    {tr("Parameters logged"),                                   QString::number(m_data.datasets.size())},
  };

  // Assemble <dt>/<dd> pairs
  QString defs;
  for (const auto& row : rows) {
    defs +=
      QStringLiteral("<dt>%1</dt><dd>%2</dd>").arg(escapeHtml(row.label), escapeHtml(row.value));
  }

  // Tag chips (render nothing when the session has no tags)
  QString tagsBlock;
  if (!m_data.tags.isEmpty()) {
    QString chips;
    for (const QString& t : m_data.tags)
      chips += QStringLiteral("<span class=\"tag\">%1</span>").arg(escapeHtml(t));

    tagsBlock = QStringLiteral("<dt>%1</dt><dd><div class=\"tags\">%2</div></dd>")
                  .arg(escapeHtml(tr("Classification")), chips);
  }

  // Notes block (wrapped, styled callout)
  QString notesBlock;
  if (!m_data.notes.trimmed().isEmpty()) {
    notesBlock = QStringLiteral("<div class=\"notes-label\">%1</div>"
                                "<div class=\"notes\">%2</div>")
                   .arg(escapeHtml(tr("Notes")), escapeHtml(m_data.notes));
  }

  return QStringLiteral("<section class=\"section\">"
                        "<h2>%1</h2>"
                        "<dl class=\"defs\">%2%3</dl>"
                        "%4"
                        "</section>")
    .arg(escapeHtml(tr("Test Information")), defs, tagsBlock, notesBlock);
}

//--------------------------------------------------------------------------------------------------
// Section: measurement summary
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the sortable stats table.
 */
QString Sessions::HtmlReport::buildSummarySection() const
{
  if (m_data.datasets.empty())
    return QString();

  // Units column is dropped entirely when no parameter carries a unit,
  // so the table fits the remaining columns more naturally on the page.
  const bool anyUnits = std::any_of(m_data.datasets.begin(),
                                    m_data.datasets.end(),
                                    [](const DatasetStats& d) { return !d.units.isEmpty(); });

  // Table header — data-sort-type drives the JS comparator
  QString header = QStringLiteral("<thead><tr>")
                 + QStringLiteral("<th class=\"align-left\" data-sort-type=\"text\">%1</th>")
                     .arg(escapeHtml(tr("Parameter")));
  if (anyUnits) {
    header += QStringLiteral("<th class=\"align-left\" data-sort-type=\"text\">%1</th>")
                .arg(escapeHtml(tr("Units")));
  }
  header += QStringLiteral("<th data-sort-type=\"numeric\">%1</th>"
                           "<th data-sort-type=\"numeric\">%2</th>"
                           "<th data-sort-type=\"numeric\">%3</th>"
                           "<th data-sort-type=\"numeric\">%4</th>"
                           "<th data-sort-type=\"numeric\">%5</th>"
                           "</tr></thead>")
              .arg(escapeHtml(tr("Samples")),
                   escapeHtml(tr("Minimum")),
                   escapeHtml(tr("Maximum")),
                   escapeHtml(tr("Mean")),
                   escapeHtml(tr("Std. Deviation")));

  // One row per dataset; data-sort-value preserves raw numeric order
  QString body;
  for (const auto& ds : m_data.datasets) {
    const QString fullName =
      ds.group.isEmpty() ? ds.title : QStringLiteral("%1 / %2").arg(ds.group, ds.title);

    const bool numeric = ds.numericSamples > 0;
    const QString dash = QStringLiteral("—");

    auto numCell = [&](double v) {
      if (!numeric)
        return QStringLiteral("<td class=\"numeric dim\">%1</td>").arg(dash);

      return QStringLiteral("<td class=\"numeric\" data-sort-value=\"%1\">%2</td>")
        .arg(QString::number(v, 'g', 17), escapeHtml(formatNumber(v)));
    };

    const qint64 totalSamples = ds.numericSamples + ds.stringSamples;

    QString row = QStringLiteral("<tr>")
                + QStringLiteral("<td class=\"align-left\">%1</td>").arg(escapeHtml(fullName));
    if (anyUnits) {
      row += QStringLiteral("<td class=\"align-left %1\">%2</td>")
               .arg(ds.units.isEmpty() ? QStringLiteral("dim") : QString(),
                    escapeHtml(ds.units.isEmpty() ? dash : ds.units));
    }
    row += QStringLiteral("<td class=\"numeric\" data-sort-value=\"%1\">%2</td>")
             .arg(QString::number(totalSamples), QString::number(totalSamples));
    row += numCell(ds.minValue);
    row += numCell(ds.maxValue);
    row += numCell(ds.mean);
    row += numCell(ds.stddev);
    row += QStringLiteral("</tr>");
    body += row;
  }

  return QStringLiteral("<section class=\"section\">"
                        "<h2>%1 <span class=\"muted screen-only\">%2</span></h2>"
                        "<table class=\"summary-table\">%3<tbody>%4</tbody></table>"
                        "</section>")
    .arg(escapeHtml(tr("Measurement Summary")),
         escapeHtml(tr("click a column to sort")),
         header,
         body);
}

//--------------------------------------------------------------------------------------------------
// Section: parameter trends (charts grid)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds one Chart.js canvas per numeric series, wrapped in cards.
 */
QString Sessions::HtmlReport::buildChartsSection() const
{
  QString cards;
  for (const auto& s : m_series) {
    const QString fullName =
      s.group.isEmpty() ? s.title : QStringLiteral("%1 / %2").arg(s.group, s.title);

    const QString units =
      s.units.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(escapeHtml(s.units));

    // Subtitle with real sample count (not the decimated point count)
    const double duration = s.timesSec.empty() ? 0.0 : s.timesSec.back();
    const QString sub     = tr("%1 samples over %2 seconds")
                          .arg(QString::number(static_cast<qulonglong>(s.totalSamples)),
                               QString::number(duration, 'f', 2));

    cards += QStringLiteral("<div class=\"chart-card\">"
                            "<h3>%1%2</h3>"
                            "<div class=\"chart-sub\">%3</div>"
                            "<div class=\"chart-box\"><canvas id=\"chart-%4\"></canvas></div>"
                            "</div>")
               .arg(escapeHtml(fullName), units, escapeHtml(sub), QString::number(s.uniqueId));
  }

  // Screen-only overlay chart — every signal on one plot, toggleable via
  // Chart.js's built-in legend. Hidden in print via @media print.
  const QString overlay =
    m_series.size() >= 2
      ? QStringLiteral("<section class=\"section screen-only overlay-section\">"
                       "<h2>%1 <span class=\"muted\">%2</span></h2>"
                       "<div class=\"overlay-box\"><canvas id=\"chart-overlay\"></canvas></div>"
                       "</section>")
          .arg(escapeHtml(tr("Combined Parameter View")),
               escapeHtml(tr("click legend items to toggle signals")))
      : QString();

  return QStringLiteral("%1"
                        "<section class=\"section new-page\">"
                        "<h2 id=\"parameter-trends-heading\">%2</h2>"
                        "<div class=\"charts-grid\">%3</div>"
                        "</section>")
    .arg(overlay, escapeHtml(tr("Parameter Trends")), cards);
}

//--------------------------------------------------------------------------------------------------
// JSON payload (consumed by session-report.js)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serialises all chart series into a compact JSON blob.
 *
 * The JS side reads this from ``<script id="report-data">`` on boot and
 * builds one Chart.js instance per entry.
 */
QString Sessions::HtmlReport::buildReportDataJson() const
{
  // Index DatasetStats by uniqueId so each series can carry its
  // population-wide min/max/mean (computed in SQL, not from the decimated
  // sample window).
  std::map<int, const DatasetStats*> statsByUid;
  for (const auto& d : m_data.datasets)
    statsByUid.emplace(d.uniqueId, &d);

  QJsonArray seriesArr;
  for (const auto& s : m_series) {
    QJsonArray times;
    QJsonArray values;
    for (double t : s.timesSec)
      times.append(t);

    for (double v : s.values)
      values.append(std::isfinite(v) ? v : 0.0);

    QJsonObject entry;
    entry["uniqueId"] = s.uniqueId;
    entry["title"]  = s.group.isEmpty() ? s.title : QStringLiteral("%1 / %2").arg(s.group, s.title);
    entry["units"]  = s.units;
    entry["times"]  = times;
    entry["values"] = values;

    // Per-series stats payload — present only when the dataset has numeric
    // samples; the JS plugin is no-op for any series missing this block.
    const auto it = statsByUid.find(s.uniqueId);
    if (it != statsByUid.end() && it->second->numericSamples > 0) {
      QJsonObject stats;
      stats["min"]   = std::isfinite(it->second->minValue) ? it->second->minValue : 0.0;
      stats["max"]   = std::isfinite(it->second->maxValue) ? it->second->maxValue : 0.0;
      stats["mean"]  = std::isfinite(it->second->mean) ? it->second->mean : 0.0;
      entry["stats"] = stats;
    }

    seriesArr.append(entry);
  }

  QJsonObject style;
  style["lineWidth"] = m_opts.lineWidth > 0 ? m_opts.lineWidth : 1.4;
  style["lineStyle"] = m_opts.lineStyle.isEmpty() ? QStringLiteral("solid") : m_opts.lineStyle;

  // Per-chart annotation toggle + translated labels rendered by the JS plugin
  QJsonObject statsLabels;
  statsLabels["min"]  = tr("Min");
  statsLabels["max"]  = tr("Max");
  statsLabels["mean"] = tr("Mean");

  QJsonObject options;
  options["showStatsOverlay"] = m_opts.includeStatsOverlay;
  options["statsLabels"]      = statsLabels;

  QJsonObject root;
  root["series"]  = seriesArr;
  root["style"]   = style;
  root["options"] = options;

  // Compact form — whitespace would bloat the inline blob for big sessions
  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

//--------------------------------------------------------------------------------------------------
// Small helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the landscape CSS @page size token for the selected paper.
 */
QString Sessions::HtmlReport::pageSizeCssValue() const
{
  const QString key  = QPageSize(m_opts.pageSize).key();
  const QString name = key.isEmpty() ? QStringLiteral("A4") : key;
  return name + QStringLiteral(" landscape");
}

/**
 * @brief Printable area of the chart page in millimetres.
 */
QSizeF Sessions::HtmlReport::chartPagePrintableSize() const
{
  // Landscape paper dimensions, matching startPrinting()
  QSizeF paper = QPageSize(m_opts.pageSize).size(QPageSize::Millimeter);
  if (paper.width() < paper.height())
    paper = QSizeF(paper.height(), paper.width());

  // Subtract the 16 mm margin on each side
  const double printableW = std::max(50.0, paper.width() - 32.0);
  const double printableH = std::max(50.0, paper.height() - 32.0);
  return QSizeF(printableW, printableH);
}

/**
 * @brief Short text used in the print-only @bottom-left page box.
 */
QString Sessions::HtmlReport::buildPrintFooterLeft() const
{
  const QString docPart =
    m_opts.documentTitle.isEmpty() ? tr("Session Report") : m_opts.documentTitle;

  // CSS content:"..." strings need doubled backslashes; escape newline and quote
  QString s = QStringLiteral("Serial Studio · %1").arg(docPart);
  s.replace('\\', QStringLiteral("\\\\"));
  s.replace('"', QStringLiteral("\\\""));
  return s;
}

/**
 * @brief Builds the translated CSS @c content expression for the right-side
 *        page footer ("Page X of Y").
 *
 * Returns the full @c content: value (interleaved string literals and
 * @c counter() calls) so translators can reorder words around the page /
 * total numbers without losing the live counters. The translation is
 * @c "Page %1 of %2"; the @c %1 / @c %2 markers anchor the two counter
 * calls when the string is split.
 */
QString Sessions::HtmlReport::buildPrintFooterRight() const
{
  // Escape characters that would break the CSS string literal we emit
  auto cssEscape = [](QString s) {
    s.replace('\\', QStringLiteral("\\\\"));
    s.replace('"', QStringLiteral("\\\""));
    return s;
  };

  // English fallback as a safe default if the translation is malformed
  const QString fallback = QStringLiteral("\"Page \" counter(page) \" of \" counter(pages)");

  const QString tmpl = tr("Page %1 of %2");
  const int p1       = tmpl.indexOf(QLatin1String("%1"));
  const int p2       = tmpl.indexOf(QLatin1String("%2"));
  if (p1 < 0 || p2 < 0 || p2 <= p1)
    return fallback;

  const QString before  = cssEscape(tmpl.left(p1));
  const QString between = cssEscape(tmpl.mid(p1 + 2, p2 - p1 - 2));
  const QString after   = cssEscape(tmpl.mid(p2 + 2));

  return QStringLiteral("\"%1\" counter(page) \"%2\" counter(pages) \"%3\"")
    .arg(before, between, after);
}

/**
 * @brief Encodes a raster image as a ``data:`` URI for inlining into the HTML.
 */
QString Sessions::HtmlReport::encodeLogoAsDataUri(const QString& path) const
{
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly))
    return QString();

  const QByteArray bytes = f.readAll();
  if (bytes.isEmpty())
    return QString();

  const QString mime = QMimeDatabase().mimeTypeForFileNameAndData(path, bytes).name();

  return QStringLiteral("data:%1;base64,%2").arg(mime, QString::fromLatin1(bytes.toBase64()));
}

//--------------------------------------------------------------------------------------------------
// HTML artifact write
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes the self-contained HTML to disk.
 */
void Sessions::HtmlReport::writeHtmlArtifact(const QString& htmlPath,
                                             const QString& html,
                                             bool& success)
{
  QFile out(htmlPath);
  if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    success = false;
    return;
  }

  QTextStream ts(&out);
  ts.setEncoding(QStringConverter::Utf8);
  ts << html;
  out.close();

  success = (out.error() == QFileDevice::NoError);
}

//--------------------------------------------------------------------------------------------------
// PDF render pipeline
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the HTML into an off-screen QWebEnginePage.
 *
 * The page's runtime flips @c window.__reportReady synchronously after
 * Chart.js finishes building every canvas. We read that flag via
 * @c runJavaScript — the callback runs even on hidden (off-screen) pages,
 * unlike @c requestAnimationFrame which Chromium suspends for pages that
 * aren't attached to a visible view.
 */
void Sessions::HtmlReport::startPdfRender(const QString& html, const QString& pdfPath)
{
  Q_UNUSED(pdfPath);

  Q_EMIT progress(tr("Loading rendering engine…"), 0.45);

  // Reuse the global default profile to avoid a WebEngine cold-start per export
  m_page = new QWebEnginePage(QWebEngineProfile::defaultProfile(), this);

  connect(m_page, &QWebEnginePage::loadFinished, this, &HtmlReport::onLoadFinished);
  connect(m_page, &QWebEnginePage::pdfPrintingFinished, this, &HtmlReport::onPdfPrintingFinished);

  // baseUrl resolves any future relative references against qrc
  m_page->setHtml(html, QUrl(QStringLiteral("qrc:/rcc/templates/reports/")));
}

/**
 * @brief Page parse done — probe the readiness flag.
 */
void Sessions::HtmlReport::onLoadFinished(bool ok)
{
  if (!ok) {
    Q_EMIT finished(m_pdfPath, false);
    return;
  }

  Q_EMIT progress(tr("Rendering charts…"), 0.65);
  m_readinessAttempts = 0;
  probeReadiness();
}

/**
 * @brief Reads @c window.__reportReady. If set, prints; else re-probes.
 *
 * The probe is non-blocking: runJavaScript posts to the renderer and fires
 * its callback when the result is available. The next probe is scheduled
 * only from inside the previous callback so at most one probe is in flight
 * at any time (no queue buildup, no races).
 *
 * Upper bound: 30 attempts × 100 ms = 3 s — generous for the synchronous
 * ready flag, but we still cap it so a broken template can't hang.
 */
void Sessions::HtmlReport::probeReadiness()
{
  if (m_printStarted || !m_page)
    return;

  constexpr int kMaxAttempts = 30;
  if (++m_readinessAttempts > kMaxAttempts) {
    qWarning() << "[HtmlReport] Ready flag never set — printing anyway";
    startPrinting();
    return;
  }

  m_page->runJavaScript(QStringLiteral("window.__reportReady === true"),
                        [this](const QVariant& result) {
                          if (m_printStarted || !m_page)
                            return;

                          if (result.toBool()) {
                            startPrinting();
                            return;
                          }

                          // Chain the next probe from inside this callback so only one is in flight
                          QTimer::singleShot(100, this, &HtmlReport::probeReadiness);
                        });
}

/**
 * @brief Hands off the loaded page to Chromium's PDF printer (landscape).
 */
void Sessions::HtmlReport::startPrinting()
{
  if (m_printStarted || !m_page)
    return;

  m_printStarted = true;

  Q_EMIT progress(tr("Generating PDF…"), 0.85);

  const QPageLayout layout(QPageSize(m_opts.pageSize),
                           QPageLayout::Landscape,
                           QMarginsF(16, 16, 16, 16),
                           QPageLayout::Millimeter);
  m_page->printToPdf(m_pdfPath, layout);
}

/**
 * @brief WebEngine signal → final @c finished() emission.
 *
 * For @c Both mode the emitted path is the PDF (primary artifact); the
 * caller can separately check the HTML was also written by inspecting the
 * filesystem.
 */
void Sessions::HtmlReport::onPdfPrintingFinished(const QString& filePath, bool success)
{
  Q_UNUSED(filePath);
  Q_EMIT finished(m_pdfPath, success);
}

#endif  // BUILD_COMMERCIAL
