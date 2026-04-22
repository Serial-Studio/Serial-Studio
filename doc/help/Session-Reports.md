# Session Reports (Pro)

Serial Studio Pro can export any recorded session as a **self-contained HTML report** or a **PDF**. The report includes a cover page, session metadata, per-parameter summary statistics, and one interactive Chart.js plot per numeric dataset. It's a share-ready artifact for teammates, customers, or a lab notebook, generated from the same SQLite session database used by the [Session Explorer](Session-Database.md).

## Generating a report

1. Open the **Session Explorer** from the toolbar or the **File** menu.
2. Select a recorded session.
3. Click **Generate Report** in the session detail pane.
4. Configure branding, page, and section options in the dialog (see below).
5. Click **Export HTML**, **Export PDF**, or **Export Both**.

A progress dialog appears while charts render and (for PDF) the page is rasterized. When finished, the files are written to:

```
<Workspace>/Reports/<Project Title>/
```

The workspace root is the folder set under **Settings → Workspace**. Project titles are sanitized so the filename is always safe.

---

## What's in the report

The report has four sections, each independently toggleable:

| Section | Contents |
|---|---|
| **Cover page** | Logo, company name, document title, project name, session ID, duration, sample count, parameter count, start/end timestamps, author, generation date |
| **Test information** | Project title, session ID, start/end times, duration, sample count, parameter count, tags (as chips), and session notes |
| **Measurement summary** | Per-parameter table with sample count, min, max, mean, and standard deviation. Non-numeric datasets show `—` for numeric columns |
| **Parameter trends** | One Chart.js line chart per numeric dataset, laid out in a responsive grid. A screen-only overlay chart (hidden on print) plots all parameters on shared axes for shape comparison |

---

## Options

All options are persisted between runs, so you only configure them once per project.

### Branding tab

- **Company name**, **document title**, **author name**
- **Logo**: PNG, JPG, or SVG. Embedded inline so the HTML stays self-contained.

### Page tab

- **Page size**: A4, A3, Letter, Legal
- **Orientation**: Portrait or Landscape
- **Line width**: 0.5 – 3.0 pt (default 1.4)
- **Line style**: solid, dashed, or dotted

### Sections tab

Toggle each of the four sections on or off independently. Turning off **Parameter trends** produces a text-only summary report that renders instantly and prints to a few pages.

---

## HTML vs PDF

Both formats ship the same content, rendered from the same template.

| | **HTML** | **PDF** |
|---|---|---|
| Single file | Yes | Yes |
| Self-contained | Yes (Chart.js, CSS, JS, and logo are inlined) | N/A (rasterized) |
| Interactive charts | Yes (hover, tooltip, overlay toggle) | No (charts are rendered as images) |
| File size | Larger (embeds Chart.js + raw data) | Smaller, fixed per page count |
| Best for | Internal review, web sharing, debugging | Customer deliverables, regulatory submissions, printing |

Choose **Export Both** to get an interactive version for your team and a printable version for the archive in a single step.

---

## How charts are rendered

Charts use **Chart.js 4.x**, bundled inside Serial Studio. No network access is required, since the full library, stylesheet, and client runtime are embedded in the HTML at render time.

### Downsampling

Long sessions are downsampled to a maximum of **1,200 points per chart** using first/min/max/last bucket decimation across 300 equal-time buckets. This preserves the signal envelope (peaks and valleys survive) while keeping charts responsive even for sessions with millions of samples.

If you need the full-resolution data, export the session to **CSV** from the Session Explorer instead.

### Numeric formatting

Tick labels use scientific notation for values with magnitude below 10⁻³ or at or above 10⁶. Otherwise they display with three decimal places. Colors rotate through a muted HSL palette (base hue 210°, stepped by 48° per parameter) tuned for print legibility.

---

## Performance notes

- HTML export is near-instant for any session size: the template inlines the data as JSON and all rendering happens in the browser when the file is opened.
- PDF export is slower. Serial Studio loads the HTML in an off-screen browser, waits for Chart.js to finish rendering, then rasterizes the page. Expect a few seconds for sessions with many datasets.
- The report is independent of the live project: it pulls session data directly from the SQLite database, so you can regenerate reports from archived sessions long after the original project has changed.

---

## See also

- [Session Database](Session-Database.md): where sessions are recorded and browsed.
- [CSV Import & Export](CSV-Import-Export.md): for full-resolution tabular exports.
- [Pro vs Free Features](Pro-vs-Free.md): full list of Pro-only features.
