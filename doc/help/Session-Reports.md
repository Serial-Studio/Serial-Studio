# Session Reports (Pro)

You recorded a bench run last Tuesday and a customer wants it as a document, not a database. Session Reports (Pro) turns a session from the [Historian](Historian.md) into a file with a cover page, the min, max and mean of every value, and one chart per value.

Official builds with the embedded browser engine export a PDF. Builds without it export a single self-contained HTML file. There is no format switch: the build decides. If the dialog in step 4 is titled **Generate PDF Report**, your build exports PDF; if it is titled **Generate Report**, it exports HTML.

Each comma-separated value your device prints becomes a dataset in Serial Studio, and the report calls it a parameter. A session with temperature and humidity gives a cover page, a table with one row per value, and one chart page each.

Session Reports are a Pro feature, like the Historian. Without a license, the first use offers the Pro trial with a Yes/No prompt (it needs an internet connection); see [Pro vs Free Features](Pro-vs-Free.md). The export stays unavailable until a license or trial is active.

## Generating a report

A session is one recording of a connection, from connect to disconnect. Reports read the session from the Historian database, not the live project, so you can regenerate one from an archived session after the project has changed. The Historian list stays empty until you record one: switch on **Historian Database** under **Data Export** in the project setup, then connect your device (see [Historian](Historian.md)).

1. Open the Historian with the **Historian** button on the toolbar.
2. Select a recorded session.
3. Click **Export PDF** in the session detail toolbar (tooltip: "Generate a PDF report for the selected session"). The button keeps this name on every build.
4. The **Generate PDF Report** dialog opens (titled **Generate Report** on builds without the embedded browser engine). The defaults work, so you can skip straight to the next step. To brand or trim the report, see [Options](#options).
5. Click **Export PDF** in the dialog (**Export HTML** on builds without the embedded browser engine).
6. Confirm the destination in the **Save PDF Report** (or **Save HTML Report**) dialog.

The Save dialog is pre-filled with:

```text
<Workspace>/Reports/<Project Title>/<Document title or session_ID>.pdf
```

The workspace root is the **Workspace Folder** set in Settings → General. The file name is your document title if you set one, otherwise `session_` followed by the session ID.

## What's in the report

The report has four sections, switched on and off in the **Sections** row of [Options](#options). The **Datasets** tab of the dialog selects which datasets appear.

| Section | Contents |
|---|---|
| **Cover page** | Logo, company name, document title, project name, author, generation date, plus the session ID, duration, sample count, parameter count and start/end timestamps |
| **Test information** | Project title, the same session figures, tags, and session notes |
| **Measurement summary** | Per-parameter table with sample count, min, max, mean, and standard deviation. Non-numeric datasets show `—` for numeric columns |
| **Parameter trends** | One Chart.js line chart per numeric dataset, each starting its own page when printed. With two or more datasets, a screen-only overlay chart (hidden on print, so absent from a PDF) draws them on one set of axes |

The statistics are computed over every recorded sample, not the downsampled series the charts show.

## Options

The dialog remembers its settings between runs. They are stored per user, not per project.

| Tab | Field | Values and default |
|---|---|---|
| **Branding** | **Company**, **Document title**, **Author** | Free text, empty by default |
| **Branding** | **Logo** | PNG, JPG, or SVG, embedded inline so an HTML report stays self-contained |
| **Page** | Page size | A0–A6, B4, B5, Letter, Legal, Executive, Tabloid or Ledger; default A4. Reports always print in landscape |
| **Page** | Line width | 0.5 to 3.0, default 1.4 |
| **Page** | Line style | Solid (default), dashed, or dotted |
| **Sections** | Cover page, Test information, Measurement summary, Parameter trends | All on by default |
| **Sections** | **Annotate min, max, and mean values on plots** | Off by default; overlays those statistics on each chart |
| **Datasets** | Dataset checkboxes | Search box, **Expand All** and **Collapse All**, and a checkbox per group, folder or dataset |

At least one dataset must stay selected. Otherwise the export button is disabled and the dialog shows "Select at least one dataset to include." Turning off **Parameter trends** leaves a report without charts.

## How charts are rendered

Charts use **Chart.js 4.x**, bundled inside Serial Studio, so no network access is needed. For a PDF, Serial Studio loads the report in an off-screen browser page, waits for Chart.js to finish, and prints that page to PDF. Charts are static in a PDF. An HTML report keeps hover tooltips and the overlay chart. Series colors cycle through HSL hues starting at 210°, 48° apart, at 68% saturation and 36% lightness.

### Downsampling

Each chart holds at most 10,000 points. Longer series are split into 2,500 equal-count buckets that keep the first, minimum, maximum and last sample of each, then topped up with evenly spaced samples to 10,000. Peaks and valleys survive. Shorter series are plotted whole. The limit is not configurable.

For the full-resolution data, export the session to CSV from the Historian instead (see [CSV Export & Playback](CSV-Export-Playback.md)).

### Numeric formatting

Axis tick labels show up to three decimals, with trailing zeros dropped. Magnitudes below 0.001 or at or above 1,000,000 switch to scientific notation with two decimals (`1.23e-4`); zero stays plain. The time axis shows whole seconds for steps of 1 s or more, and one decimal below that.

## Performance

A PDF export is slower than HTML, because the off-screen browser has to render every chart before printing, and sessions with many datasets take longest. An HTML export still runs the database read and the 10,000-point downsampling first.

## See also

- [Historian](Historian.md): where sessions are recorded and browsed.
- [CSV Export & Playback](CSV-Export-Playback.md): for full-resolution tabular exports.
- [Pro vs Free Features](Pro-vs-Free.md): full list of Pro-only features.
