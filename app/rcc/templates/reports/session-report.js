/* =========================================================================
 * Serial Studio — Session Report runtime
 * =========================================================================
 *
 * Responsibilities:
 *   1. Build one Chart.js line chart per parameter from the inline JSON blob
 *   2. Wire sortable column headers on the Measurement Summary table
 *   3. Signal readiness to the Qt side so printToPdf() fires after animation
 *
 * Kept dependency-free apart from Chart.js (embedded earlier in the page).
 * ------------------------------------------------------------------------- */

(function() {
  "use strict";

  // -----------------------------------------------------------------------
  // Readiness beacon — the C++ renderer polls window.__reportReady
  // -----------------------------------------------------------------------

  // Readiness flag kept for any consumer (tests, future tooling) that wants
  // to wait on a specific signal. The C++ PDF pipeline no longer checks it —
  // it just waits a fixed settle window after loadFinished, which is simpler
  // and race-free. The DOM event is still useful for browser-side consumers.
  window.__reportReady = false;

  function markReady() {
    window.__reportReady = true;
    document.dispatchEvent(new CustomEvent("report-ready"));

    // Deterministic hand-off to Qt: console.log with a well-known prefix
    // is surfaced by QWebEnginePage::javaScriptConsoleMessage, giving us
    // an event-driven "page is ready" signal without polling, bridges, or
    // race conditions. Standalone browser viewers just see a log line.
    console.log("[serial-studio-report] ready");
  }

  // -----------------------------------------------------------------------
  // Toolbar — Print to PDF button invokes the browser's print dialog. In
  // the Qt WebEngine printToPdf() path the toolbar is hidden by @media print
  // so this is purely for human viewers of the HTML output.
  // -----------------------------------------------------------------------

  function initToolbar() {
    const btn = document.getElementById("print-button");
    if (btn)
      btn.addEventListener("click", () => window.print());
  }

  // -----------------------------------------------------------------------
  // Sortable table — click a column header to toggle asc/desc. Uses a
  // data-sort-type attribute ("numeric" or "text") to pick the comparator.
  // -----------------------------------------------------------------------

  function initSortableTable() {
    const table = document.querySelector(".summary-table");
    if (!table)
      return;

    const headers = table.querySelectorAll("thead th");
    headers.forEach((th, colIndex) => {
      th.addEventListener("click", () => sortBy(table, headers, th, colIndex));
    });
  }

  function sortBy(table, headers, th, colIndex) {
    const type       = th.dataset.sortType || "text";
    const ascending  = !th.classList.contains("sort-asc");
    const tbody      = table.querySelector("tbody");
    const rows       = Array.from(tbody.querySelectorAll("tr"));

    const valueOf = (tr) => {
      const cell = tr.children[colIndex];
      if (!cell) return "";
      const raw = cell.dataset.sortValue ?? cell.textContent.trim();
      if (type === "numeric") {
        const n = parseFloat(raw);
        return Number.isFinite(n) ? n : (ascending ? Infinity : -Infinity);
      }
      return raw.toLowerCase();
    };

    rows.sort((a, b) => {
      const va = valueOf(a);
      const vb = valueOf(b);
      if (va < vb) return ascending ? -1 : 1;
      if (va > vb) return ascending ? 1 : -1;
      return 0;
    });

    rows.forEach((r) => tbody.appendChild(r));

    headers.forEach((h) => h.classList.remove("sort-asc", "sort-desc"));
    th.classList.add(ascending ? "sort-asc" : "sort-desc");
  }

  // -----------------------------------------------------------------------
  // Charts — build a line chart per parameter from the JSON blob
  // -----------------------------------------------------------------------

  function readReportData() {
    const el = document.getElementById("report-data");
    if (!el) return null;
    try {
      return JSON.parse(el.textContent);
    } catch (e) {
      console.error("Report data parse failed:", e);
      return null;
    }
  }

  function formatAxisValue(value) {
    if (!Number.isFinite(value)) return "";
    const abs = Math.abs(value);
    if (abs !== 0 && (abs < 1e-3 || abs >= 1e6)) return value.toExponential(2);
    return value.toLocaleString(undefined, {
      maximumFractionDigits: 3,
      useGrouping: false
    });
  }

  // -----------------------------------------------------------------------
  // Nice-number axis helpers — the 1-2-5 rule, ported from Serial Studio's
  // PlotWidget.qml (smartIntervalX/Y). Snaps tick intervals to human-friendly
  // round numbers so the axes read 0, 2, 4, 6… instead of 1.27, 6.27, 11.27.
  // -----------------------------------------------------------------------

  function niceStep(range, targetCount) {
    if (range <= 0 || targetCount < 2) return 1;
    const rough     = range / targetCount;
    const magnitude = Math.pow(10, Math.floor(Math.log10(rough)));
    const norm      = rough / magnitude;
    let step;
    if      (norm <= 1.0) step = 1.0;
    else if (norm <= 2.0) step = 2.0;
    else if (norm <= 5.0) step = 5.0;
    else                  step = 10.0;
    return step * magnitude;
  }

  // Expand [min, max] outward so both endpoints land on a nice step. Returns
  // the padded bounds plus the chosen step — matches how matplotlib rounds.
  function niceBounds(min, max, targetCount) {
    if (!Number.isFinite(min) || !Number.isFinite(max) || min === max) {
      const base = Number.isFinite(min) ? min : 0;
      return { min: base - 1, max: base + 1, step: 0.5 };
    }
    const step = niceStep(max - min, targetCount);
    const lo   = Math.floor(min / step) * step;
    const hi   = Math.ceil(max / step) * step;
    return { min: lo, max: hi, step: step };
  }

  // HSL palette rotation — distinct but coherent, darker saturations so the
  // signal prints clearly on white paper without neon colours.
  function paletteColor(index) {
    const hue = (210 + index * 48) % 360;  // 210° is the base blue accent
    return {
      stroke: `hsl(${hue}, 68%, 36%)`,
      fill:   `hsla(${hue}, 68%, 36%, 0.08)`
    };
  }

  function buildChart(canvas, series, colors, lineWidth, dashPattern) {
    const points = series.times.map((t, i) => ({ x: t, y: series.values[i] }));

    // Compute data bounds
    const xTimes = series.times;
    const yVals  = series.values;
    let xMin = xTimes.length ? xTimes[0] : 0;
    let xMax = xTimes.length ? xTimes[xTimes.length - 1] : 1;
    let yMin = Infinity, yMax = -Infinity;
    for (let i = 0; i < yVals.length; ++i) {
      const v = yVals[i];
      if (v < yMin) yMin = v;
      if (v > yMax) yMax = v;
    }
    if (!Number.isFinite(yMin) || !Number.isFinite(yMax)) { yMin = 0; yMax = 1; }

    // Measure the host box once — used for both the tick-target heuristic
    // and the explicit canvas sizing below.
    const host       = canvas.parentElement;
    const hostWidth  = host.clientWidth  || host.getBoundingClientRect().width  || 800;
    const hostHeight = host.clientHeight || host.getBoundingClientRect().height || 600;

    // Target tick count scales with the chart's current size: ~80 px of
    // horizontal breathing room per X label, ~50 px vertical for Y.
    const xTicksTarget = Math.max(2, Math.floor(hostWidth  / 80));
    const yTicksTarget = Math.max(2, Math.floor(hostHeight / 50));

    // X-axis: clamp to the actual data range (no padding). Still uses the
    // nice-number step so tick labels land on round values, but the axis
    // itself ends exactly where the samples end — no trailing empty space.
    const xNice  = niceBounds(xMin, xMax, xTicksTarget);
    const xBounds = { min: xMin, max: xMax, step: xNice.step };

    // Y-axis: pad outward to the nearest nice step so the trace doesn't
    // clip against the frame edge and labels align with the data range.
    const yBounds = niceBounds(yMin, yMax, yTicksTarget);

    // Force explicit canvas dimensions before Chart.js inspects the element.
    // In a headless QWebEnginePage, cards past the initial viewport measure
    // as 0×0 at layout time, which leaves Chart.js with nothing to draw.
    // Keep the backing store policy simple here — data shaping happens in
    // C++, the report runtime should only render the provided series.
    const targetDpr = Math.max(2, window.devicePixelRatio || 1);
    canvas.style.width  = hostWidth  + "px";
    canvas.style.height = hostHeight + "px";
    canvas.width  = Math.floor(hostWidth  * targetDpr);
    canvas.height = Math.floor(hostHeight * targetDpr);

    // Palette + sizes tuned for paper legibility in grayscale prints
    const axisTextColor   = "#1f1f1f";
    const axisBorderColor = "#1f1f1f";
    const majorGridColor  = "#9aa0a8";
    const minorGridColor  = "#d0d4db";
    const plotBgColor     = "#fafbfc";
    const tickFontSize    = 13;
    const axisTitleSize   = 14;

    const config = {
      type: "line",
      data: {
        datasets: [{
          label: series.title,
          data: points,
          borderColor: colors.stroke,
          backgroundColor: colors.fill,
          borderWidth: lineWidth,
          borderDash: dashPattern,
          borderJoinStyle: "round",
          borderCapStyle: "round",
          pointRadius: 0,
          pointHoverRadius: 3,
          tension: 0,
          spanGaps: true,
          fill: false
        }]
      },
      options: {
        // Fixed canvas + DPR is what printToPdf captures cleanly
        responsive: false,
        maintainAspectRatio: false,
        devicePixelRatio: targetDpr,
        animation: { duration: 0 },
        interaction: { intersect: false, mode: "nearest", axis: "x" },
        layout: { padding: { top: 10, right: 18, bottom: 6, left: 6 } },
        plugins: {
          decimation: { enabled: false },
          legend: { display: false },
          tooltip: {
            enabled: true,
            backgroundColor: "rgba(17, 24, 39, 0.92)",
            titleFont: { weight: "600" },
            padding: 8,
            cornerRadius: 4,
            callbacks: {
              title: (items) => items.length
                ? `t = ${items[0].parsed.x.toFixed(3)} s`
                : "",
              label: (item) => {
                const u = series.units ? ` ${series.units}` : "";
                return `${formatAxisValue(item.parsed.y)}${u}`;
              }
            }
          }
        },
        scales: {
          x: {
            type: "linear",
            min: xBounds.min,
            max: xBounds.max,
            title: {
              display: true,
              text: "Time (s)",
              color: axisTextColor,
              font: { size: axisTitleSize, weight: "500" }
            },
            ticks: {
              color: axisTextColor,
              font: { size: tickFontSize },
              stepSize: xBounds.step,
              callback: formatAxisValue
            },
            grid: {
              color: majorGridColor,
              lineWidth: 0.6,
              tickLength: 6,
              tickWidth: 1,
              tickColor: axisBorderColor
            },
            border: { color: axisBorderColor, width: 1 }
          },
          y: {
            min: yBounds.min,
            max: yBounds.max,
            title: { display: false },   // heading + card title already identify the series
            ticks: {
              color: axisTextColor,
              font: { size: tickFontSize },
              stepSize: yBounds.step,
              callback: formatAxisValue
            },
            grid: {
              color: majorGridColor,
              lineWidth: 0.6,
              tickLength: 6,
              tickWidth: 1,
              tickColor: axisBorderColor
            },
            border: { color: axisBorderColor, width: 1 }
          }
        }
      },

      plugins: [
        // Plot-area tint painted in beforeDraw so Chart.js's own gridlines
        // render ON TOP — using beforeDatasetsDraw covered the major grid.
        // The closed frame is drawn in afterDatasetsDraw, above the series.
        {
          id: "plot-area-frame",
          beforeDraw(chart) {
            const { ctx, chartArea } = chart;
            if (!chartArea) return;
            const { left, right, top, bottom } = chartArea;
            ctx.save();
            ctx.fillStyle = plotBgColor;
            ctx.fillRect(left, top, right - left, bottom - top);
            ctx.restore();
          },
          afterDatasetsDraw(chart) {
            const { ctx, chartArea } = chart;
            const { left, right, top, bottom } = chartArea;
            ctx.save();
            ctx.strokeStyle = axisBorderColor;
            ctx.lineWidth = 1;
            ctx.strokeRect(left + 0.5, top + 0.5,
                           (right - left) - 1, (bottom - top) - 1);
            ctx.restore();
          }
        },

        // Minor gridlines between each major tick (skipped if dense)
        {
          id: "minor-gridlines",
          beforeDatasetsDraw(chart) {
            const { ctx, chartArea, scales } = chart;
            ctx.save();
            ctx.strokeStyle = minorGridColor;
            ctx.lineWidth = 0.3;

            const drawMinor = (scale, isVertical) => {
              const ticks = scale.ticks;
              if (ticks.length < 2) return;
              const a0 = scale.getPixelForValue(ticks[0].value);
              const a1 = scale.getPixelForValue(ticks[1].value);
              const spacing = Math.abs(a1 - a0);
              if (spacing < 40) return;   // too dense — skip minors
              const subdivs = spacing >= 80 ? 5 : 2;
              for (let i = 0; i < ticks.length - 1; ++i) {
                const a = scale.getPixelForValue(ticks[i].value);
                const b = scale.getPixelForValue(ticks[i + 1].value);
                for (let k = 1; k < subdivs; ++k) {
                  const p = a + (b - a) * (k / subdivs);
                  ctx.beginPath();
                  if (isVertical) {
                    ctx.moveTo(p, chartArea.top);
                    ctx.lineTo(p, chartArea.bottom);
                  } else {
                    ctx.moveTo(chartArea.left, p);
                    ctx.lineTo(chartArea.right, p);
                  }
                  ctx.stroke();
                }
              }
            };

            drawMinor(scales.x, true);
            drawMinor(scales.y, false);
            ctx.restore();
          }
        }
      ]
    };

    return new Chart(canvas.getContext("2d"), config);
  }

  // Keep references to every Chart.js instance so beforeprint can resize
  // them against the freshly-laid-out print box — matching the bitmap to the
  // actual paper dimensions. Without this, charts rendered at screen size
  // get upscaled/stretched when Chromium rasterises the PDF.
  const _charts = [];

  // -----------------------------------------------------------------------
  // Combined overlay chart (screen-only). Every parameter is drawn on the
  // same axes with its palette colour and appears in the legend; clicking a
  // legend entry toggles that signal. Because parameters usually share no
  // common unit, the Y axis is dimensionless — the plot is meant for shape
  // comparison, not absolute-value reading.
  // -----------------------------------------------------------------------
  function buildOverlayChart(canvas, seriesList, lineWidth, dashPattern) {
    // Overlay is screen-only (hidden by @media print) so it runs in
    // responsive mode — lets Chart.js size the backing store and, crucially,
    // compute legend-click coordinates correctly. Manually setting
    // canvas.width / style.width together breaks hit-testing on the legend.
    const host       = canvas.parentElement;
    const hostWidth  = host.clientWidth  || host.getBoundingClientRect().width  || 1000;

    const datasets = seriesList.map((s, i) => {
      const c = paletteColor(i);
      return {
        label: s.units ? `${s.title} (${s.units})` : s.title,
        borderColor: c.stroke,
        backgroundColor: c.fill,
        borderWidth: lineWidth,
        borderDash: dashPattern,
        borderJoinStyle: "round",
        borderCapStyle: "round",
        pointRadius: 0,
        pointHoverRadius: 3,
          tension: 0,
        spanGaps: true,
        fill: false,
        data: s.times.map((t, j) => ({ x: t, y: s.values[j] }))
      };
    });

    // X-axis spans the union of all series time ranges
    let xMin = Infinity, xMax = -Infinity;
    seriesList.forEach((s) => {
      if (s.times.length) {
        xMin = Math.min(xMin, s.times[0]);
        xMax = Math.max(xMax, s.times[s.times.length - 1]);
      }
    });
    if (!Number.isFinite(xMin) || !Number.isFinite(xMax)) { xMin = 0; xMax = 1; }

    const xTicksTarget = Math.max(2, Math.floor(hostWidth / 80));
    const xNice        = niceBounds(xMin, xMax, xTicksTarget);

    const axisTextColor   = "#1f1f1f";
    const axisBorderColor = "#1f1f1f";
    const plotBgColor     = "#fafbfc";
    const tickFontSize    = 13;
    const axisTitleSize   = 14;

    return new Chart(canvas.getContext("2d"), {
      type: "line",
      data: { datasets: datasets },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 0 },
        interaction: { intersect: false, mode: "nearest", axis: "x" },
        plugins: {
          decimation: { enabled: false },
          legend: {
            display: true,
            position: "top",
            labels: {
              color: axisTextColor,
              font: { size: 13 },
              boxWidth: 16,
              usePointStyle: true
            }
          },
          tooltip: {
            enabled: true,
            backgroundColor: "rgba(17, 24, 39, 0.92)",
            titleFont: { weight: "600" },
            padding: 8,
            cornerRadius: 4,
            callbacks: {
              title: (items) => items.length
                ? `t = ${items[0].parsed.x.toFixed(3)} s`
                : ""
            }
          }
        },
        scales: {
          x: {
            type: "linear",
            min: xMin,
            max: xMax,
            title: {
              display: true,
              text: "Time (s)",
              color: axisTextColor,
              font: { size: axisTitleSize, weight: "500" }
            },
            ticks: {
              color: axisTextColor,
              font: { size: tickFontSize },
              stepSize: xNice.step,
              callback: (v) => xNice.step >= 1 ? Number(v).toFixed(0) : Number(v).toFixed(1)
            },
            grid:   { color: "#9aa0a8", lineWidth: 0.6, tickLength: 6, tickColor: axisBorderColor },
            border: { color: axisBorderColor, width: 1 }
          },
          y: {
            title: { display: false },
            ticks: {
              color: axisTextColor,
              font: { size: tickFontSize },
              callback: formatAxisValue
            },
            grid:   { color: "#9aa0a8", lineWidth: 0.6, tickLength: 6, tickColor: axisBorderColor },
            border: { color: axisBorderColor, width: 1 }
          }
        }
      },
      plugins: [{
        id: "plot-area-frame",
        beforeDraw(chart) {
          const { ctx, chartArea } = chart;
          if (!chartArea) return;
          const { left, right, top, bottom } = chartArea;
          ctx.save();
          ctx.fillStyle = plotBgColor;
          ctx.fillRect(left, top, right - left, bottom - top);
          ctx.restore();
        },
        afterDatasetsDraw(chart) {
          const { ctx, chartArea } = chart;
          const { left, right, top, bottom } = chartArea;
          ctx.save();
          ctx.strokeStyle = axisBorderColor;
          ctx.lineWidth = 1;
          ctx.strokeRect(left + 0.5, top + 0.5,
                         (right - left) - 1, (bottom - top) - 1);
          ctx.restore();
        }
      }]
    });
  }

  function initCharts() {
    const data = readReportData();
    if (!data || !Array.isArray(data.series) || data.series.length === 0)
      return 0;

    // Shared line appearance (width + dash pattern) — set by the export
    // dialog and inlined into the data blob.
    const style       = data.style || {};
    const lineWidth   = Number.isFinite(+style.lineWidth) ? +style.lineWidth : 1.4;
    const lineStyle   = String(style.lineStyle || "solid").toLowerCase();
    const dashPattern = lineStyle === "dashed" ? [6, 4]
                      : lineStyle === "dotted" ? [1, 3]
                      : [];

    let built = 0;
    data.series.forEach((series, index) => {
      const canvas = document.getElementById("chart-" + series.uniqueId);
      if (!canvas) return;
      const chart = buildChart(canvas, series, paletteColor(index),
                               lineWidth, dashPattern);
      // Stash enough to rebuild on resize (tick count depends on canvas size)
      _charts.push({
        canvas: canvas, chart: chart, series: series,
        colors: paletteColor(index),
        lineWidth: lineWidth, dashPattern: dashPattern
      });
      ++built;
    });

    // Overlay chart — all series on one plot, with the Chart.js legend
    // acting as toggle buttons. Only rendered when there is more than one
    // series to overlay and the overlay canvas exists (screen mode only).
    const overlayCanvas = document.getElementById("chart-overlay");
    if (overlayCanvas && data.series.length >= 2) {
      buildOverlayChart(overlayCanvas, data.series, lineWidth, dashPattern);
    }

    // Hook the browser's print lifecycle — runs in both File→Print and
    // QWebEnginePage::printToPdf. We destroy and rebuild each chart so the
    // tick-count heuristic re-evaluates for the print-time dimensions;
    // otherwise charts keep the screen-size tick density, which looks too
    // sparse (or dense) when the canvas changes shape dramatically.
    window.addEventListener("beforeprint", () => {
      // Flush layout so clientWidth/Height reports print-time dimensions
      void document.body.offsetHeight;

      _charts.forEach((entry) => {
        entry.chart.destroy();
        entry.chart = buildChart(entry.canvas, entry.series, entry.colors,
                                 entry.lineWidth, entry.dashPattern);
      });
    });

    return built;
  }

  // -----------------------------------------------------------------------
  // Bootstrap
  // -----------------------------------------------------------------------

  function bootstrap() {
    // Every step is guarded — if anything throws (Chart.js version skew,
    // malformed data, missing DOM), we still flip the readiness beacon so
    // the PDF render pipeline can't hang on this page.
    try { initToolbar(); }       catch (e) { console.error("toolbar:",  e); }
    try { initSortableTable(); } catch (e) { console.error("sort:",     e); }

    // Force a synchronous layout before sizing charts so parent boxes have
    // non-zero dimensions even in a headless QWebEnginePage where nothing
    // ever "scrolls into view". Reading offsetHeight flushes pending layout.
    void document.body.offsetHeight;

    try { initCharts(); }        catch (e) { console.error("charts:",   e); }

    // Flip the readiness beacon synchronously. Originally this was gated on
    // a double requestAnimationFrame, but off-screen QWebEnginePages are
    // marked "hidden" by Chromium → rAF is suspended → the fence never
    // fired. Chart.js runs synchronously (animation duration 0) so there is
    // nothing async to wait for here anyway; printToPdf() forces its own
    // paint pass when it runs.
    markReady();
  }

  if (document.readyState === "loading")
    document.addEventListener("DOMContentLoaded", bootstrap);
  else
    bootstrap();
})();
