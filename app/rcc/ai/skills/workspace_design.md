# Workspace Design: What Goes Where

`dashboard_layout` covers the API: slugs, enums, min/max pairs, the
addWidget pre-flight. **This skill covers the design judgment that
comes before you call addWidget.** When the user says "organize my
dashboard" or "build an overview", they're asking for a curated layout,
not a dump of every group as a tile.

The dashboard exists to answer one question fast: **is the system
operating normally right now?** Everything else is secondary.

## Tab composition: what belongs on workspace N

A workspace holds 5-9 tiles before the user starts tab-hopping instead
of reading. Miller's Law: short-term recall holds 5±2 items; more than
that and the operator scans rather than absorbs. Use the count as a
budget, not a target: three well-chosen tiles beat seven adequate ones.

Default workspace breakdown for a multi-system project:

| Workspace        | Purpose                                                 | Tile count |
|------------------|---------------------------------------------------------|------------|
| Overview         | Is the system OK? Top-level KPIs + alarms only.         | 4-6        |
| <Subsystem>      | One per major subsystem (Engine, Battery, Comms, ...).  | 5-8        |
| Diagnostics      | Raw flags, debug, individual sensors, status grids.     | 6-10       |
| Signal Analysis  | FFT / Waterfall / Plot3D, one signal at a time.         | 2-4        |

The Overview workspace is the most-used and the least-edited. Treat it
like the front page of a newspaper: every tile fights for the slot.

## Content priority: what makes the cut

For each candidate group, ask in order:

1. **Would the operator notice within 1 second if this went wrong?**
   If yes, it belongs on Overview. (Voltage out of range, temperature
   spike, comm fault.)
2. **Does the value drift continuously or change in discrete steps?**
   Continuous → time-series tile (Plot/Gauge/Meter/Bar). Discrete →
   readout tile (DataGrid/LED/notification).
3. **Is the value meaningful without context?** RPM alone is not
   meaningful without a redline. State-of-charge IS meaningful alone.
   Context-free values need a paired reference (paint a redline,
   set `widgetMin`/`widgetMax` to operating bounds, not sensor bounds).
4. **Is this a raw flag or a derived state?** Twenty individual brake
   pressures belong on Diagnostics; the derived "brake system OK / fault"
   LED belongs on Overview.

If a group fails (1) and (4), it does NOT belong on Overview; push it
to its subsystem workspace or Diagnostics. Resist the user's urge to
"just show everything". That request gets harder to take back
once the layout is built.

## Spatial layout: first and last tile matter most

The Peak-End rule applies to dashboards: the operator's recall of the
session is anchored by the most striking tile they saw AND the last
tile they looked at. Use this:

- **Top-left tile** (first scan position in LTR locales) is the most
  important reading. Put the primary KPI here (speed, total power,
  active alarms count). Not a decoration.
- **Bottom-right tile** is the last thing they read before walking
  away. Put a "system state" summary here (alarm count, connection
  health, current operating mode) so it's the parting impression.
- **Middle tiles** carry the supporting time-series. These get scanned,
  not absorbed; they don't need to be self-explanatory the way the
  corners do.

`relativeIndex` controls ordering. Omit it for auto-assignment when you
don't care; pass it explicitly when corner placement matters.

## Widget choice: match the cognitive task

The user isn't picking a widget; they're picking how the operator will
read the value. Map the task to the tile, not the dataset's type:

| Operator task                       | Best widget        | Why                              |
|-------------------------------------|--------------------|----------------------------------|
| "Is X in range?" (yes/no)           | LED, Gauge w/ band | Position-in-band beats a number  |
| "What is X right now?" (exact)      | DataGrid, Gauge p1 | Number is unambiguous            |
| "Is X trending up/down?"            | Plot               | Trend is the whole point         |
| "Frequency content?"                | FFT, Waterfall     | Time-series hides structure      |
| "Compare X to Y, Z, W?"             | MultiPlot, Bar     | Side-by-side beats four plots    |
| "Where is X in 3D space?"           | Plot3D, GPS        | Spatial beats coordinate triples |
| "Has X crossed a threshold?"        | NotificationLog    | Events are events, not samples   |

A Gauge for "what's the exact value" is the wrong choice: needles read
to ±5%, digital readouts to the last digit. A Plot for "is X in range
right now" is the wrong choice: the operator has to scan a line and
mentally compare to bounds. Match the tile to the question.

## Accessibility: keyboard, focus, color

Serial Studio runs on operator stations where mouse use is sometimes
blocked (gloves, panel-mount, accessibility need). Workspaces are
tab-navigable from the taskbar; widget focus follows DOM order, which
matches `relativeIndex` order. So:

- **Order workspaces left-to-right by frequency-of-use.** The first tab
  is the most-touched. Don't put Diagnostics first.
- **Order tiles within a workspace by reading order.** The Tab key walks
  through them in `relativeIndex` order; don't put the most-important
  tile last in the tab cycle.
- **Color is never the sole signal.** Same rule as the painter skill:
  an LED alarm tile that's only red-vs-green is invisible to ~8% of
  male operators. Pair with text (`"ALARM"` label), shape (filled vs
  outlined), or position (LED moving from a `normal` zone to an `alarm`
  zone). `theme.alarm` is fine for emphasis, NOT for the sole signal.
- **Don't strip units.** The dataset's `.units` field renders on every
  tile that uses it; don't override with an empty string for "cleaner
  look". An operator reading a number without units is guessing.

## Don't auto-generate and walk away

`project.workspace.autoGenerate{}` produces a "reasonable starting
layout" (one tile per group, default widget per dataset). It's a
starting point, not a finish line. After auto-generate:

1. Read `project.workspace.list` to see what landed where.
2. Identify the 4-6 tiles that belong on Overview using the priority
   rules above. Move them or rebuild the Overview workspace from
   scratch.
3. Push raw-flag and debug groups out of Overview into Diagnostics.
4. Show the curated plan to the user BEFORE pushing. List the
   workspaces, the tiles on each, and the rationale ("Overview has the
   four KPIs you mentioned; Diagnostics holds the 18 individual brake
   sensors so they're not crowding the front page").

## Common mistakes

- **Pinning everything to Overview** because `autoGenerate` did it and
  it felt rude to delete tiles. Cut ruthlessly. A noisy Overview is a
  failed Overview.
- **Picking Gauge for every numeric dataset** because it looks like a
  dashboard. Gauges are for bounded scalars where position-in-range
  matters. RPM with redline: yes. Total odometer reading: no, that's
  a DataGrid.
- **Multiple FFTs on one workspace.** FFTs eat horizontal space and
  the operator can only read one spectrum at a time. Put each FFT on
  its own Signal Analysis tab, or use Waterfall to summarize over time.
- **No min/max on Gauges and Bars.** They render but show empty needles
  / flat fills. See the `dashboard_layout` min/max section: `wgtMin` /
  `wgtMax` are mandatory write-form names.
- **Cross-workspace duplication.** Pinning the same KPI on Overview AND
  on its subsystem tab feels safe but trains the operator to ignore
  Overview ("I'll just look at the subsystem tab"). Pick one home per
  reading; cross-link via workspace switching, not duplication.
- **Skipping the curated plan.** Pushing tiles silently and asking
  "looks good?" after the fact forces the user to undo your work to
  redirect you. Show the plan first.
