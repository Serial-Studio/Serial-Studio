# Benchmark Dialog

You do not need this dialog to use Serial Studio. It answers one question: how many frames per
second can this computer process? A frame is one line of your `Serial.println()` output. A board
printing at 115200 baud sends at most a few hundred to about a thousand lines per second. This
dialog checks a machine that can swallow hundreds of thousands, so a Pass leaves you plenty of
headroom.

The dialog matters for sources in the hundreds of thousands of frames per second, for comparing
machines, and after you change a parser or transform.

The Benchmark dialog is in the free and Pro builds. It pushes synthetic frames through the real
pipeline with no device attached, and reports frames per second for each phase. "256 kHz" in
Serial Studio means 256,000 frames per second.

## Quick check

1. Disconnect your board. **Run Benchmark** is disabled while a device is connected or a
   recording is open.
2. Open the **About** window and click **Benchmark** (see [Opening the dialog](#opening-the-dialog)).
3. Choose **100 K frames** and **1 second**. Leave **Parsers** checked and uncheck **Mixed
   (numeric + text)**, so only **Numeric only** stays checked.
4. Click **Run Benchmark** and wait for the table.

The window stops responding while the run is going. That is the benchmark working, not the app
crashing.

| Pipeline | Throughput | Time | Result |
|---|---|---|---|
| Data pipeline | 3,100,000 frames/s | 1.00 s | Pass |
| Built-In parser (numeric) | 2,400,000 frames/s | 1.00 s | Pass |
| Lua parser (numeric) | 1,300,000 frames/s | 1.00 s | Pass |
| JavaScript parser (numeric) | 410,000 frames/s | 1.00 s | Pass |

The numbers above are illustrative. Yours depend on the machine.

A parser is the code that turns one line of text into numbers. Built-In is the default; Lua and
JavaScript are scripts you write yourself (see [Frame Parser Scripting](Frame-Parser.md)).

## Opening the dialog

Open the **About** window (the **About** toolbar button), then click **Benchmark**. The button is
hidden in runtime (deployment) mode and appears only in authoring mode; see
[Operation Modes](Operation-Modes.md). The dialog runs entirely on the local machine; nothing is
uploaded.

## What it measures

The benchmark feeds newline-delimited synthetic frames (sine-wave numeric channels, plus
rotating non-numeric status words for the mixed phases) through the same chain that live data
takes: `FrameReader` extraction, the `FrameBuilder` decode/parse/transform stage, and the
downstream consumers. Each phase reports a sustained **throughput** in frames per second and
the wall-clock **time** it took to reach the target frame count.

| Control | Options | Default | Effect |
|---|---|---|---|
| **Frames per phase** | 100 K / 250 K / 500 K / 1 M frames | 250 K frames | Frames pushed through each phase. More frames give a steadier average and a longer freeze. |
| **Minimum duration** | 1 / 2 / 5 / 10 seconds | 2 seconds | A wall-clock floor. A phase runs until it has both reached the frame count and run at least this long. |
| **Stages** | Parsers, Data export, Dashboard | Parsers only (Data export and Dashboard unchecked) | Which families of phases to schedule. The raw data pipeline always runs and is not a toggle. |
| **Data** | Numeric only, Mixed (numeric + text) | Both checked | Numeric channels, or numeric channels plus trailing text fields such as status words, or both. |

**Run Benchmark** stays disabled until at least one stage and one data type are checked. It is
also disabled while a device is connected or a recording is open; the dialog says "Disconnect the
active device before running a benchmark." or "Close the open recording before running a
benchmark." Do that first.

## The phases

The data pipeline always runs first. Each selected stage then runs every parser engine
(Built-In, Lua, JavaScript) over each selected data type. A **gate** is the minimum speed a phase
must reach to show Pass. The data-pipeline and parser phases are gated; the data-export and
dashboard phases are informational and show `n/a` in the Result column.

| Phase | What it exercises | Gate (frames/s) |
|---|---|---|
| **Data pipeline** | Raw `FrameReader` extraction only: delimiter scan and dequeue, no parse. | 1,024,000 |
| **Built-In parser (numeric)** | Native C++ template over numeric channels. | 1,024,000 |
| **Built-In parser (mixed)** | Built-In parser with trailing non-numeric string tokens. | 512,000 |
| **Lua parser (numeric)** | `parse()` in Lua over numeric channels. | 256,000 |
| **JavaScript parser (numeric)** | `parse()` in `QJSEngine` over the same channels. | 128,000 |
| **Lua parser (mixed)** | Lua `parse()` over the mixed workload. | 128,000 |
| **JavaScript parser (mixed)** | JavaScript `parse()` over the mixed workload. | 64,000 |
| **Built-In / Lua / JavaScript + data export** | Each engine with every export/output consumer enabled (CSV, API, plus MDF4 / Sessions / gRPC where built). | informational |
| **Built-In / Lua / JavaScript + dashboard** | Each engine feeding a full dashboard of widgets. | informational |

The dialog uses fixed gates. They equal what `--benchmark-hotpath` uses at its default `--min-fps`
of 256000, scaled per phase. Built-In is fastest, then Lua, then JavaScript, and mixed data is
slower than numeric. The per-language ranking is also in
[Frame Parser Scripting: Performance Tips](Frame-Parser.md#performance-tips).

The dialog gates seven phases. The headless CI form gates two more, for nine: **Lua + data
export** and **Lua + dashboard**, each floored at 128,000 frames/s. Those two rows print `n/a` and
never fail in the dialog. In builds with allocation statistics enabled (`SS_ALLOC_STATS`), the CI
form also fails if the Built-In parser allocates per frame; other builds report `n/a`. See the
[CLI section](Command-Line-Interface.md#acquisition-pipeline-benchmark).

## Why the interface freezes

The Benchmark dialog stops responding while it runs, and a warning in the dialog says so. For a
quick check, pick 100 K frames and a 1-second floor; the defaults (250 K frames, 2 seconds) take
longer. Every phase runs at least the minimum duration, so the default run (seven phases of 2
seconds or more) takes 14 seconds or longer. Wait for the table to appear.

Normally `FrameReader` and `FrameBuilder` live on a dedicated pipeline thread, away from the GUI.
To drive them with plain synchronous calls against your live session, the dialog relocates them
onto the GUI thread for the run and hands them back when it ends. The tight benchmark loop then
blocks that thread's event loop, so it cannot repaint until the loop returns. See
[Threading and Timing Guarantees](Threading-and-Timing.md#framereader-and-framebuilder-run-on-a-dedicated-pipeline-thread)
for where frame parsing normally lives and why.

Each phase yields to the event loop about every 16 ms so the spinner and progress bar repaint.
The time spent on each of those pumps is discounted from the measurement, so a repaint never
deflates throughput.

## What it does to your session

Your project reloads automatically when the run ends, and exported files never touch your real
data folders. While the benchmark runs, the runner snapshots your session, switches to a built-in
test project, and loads the selected parser language. It redirects every exporter into a
throwaway temporary workspace, so CSV, MDF4, and Session files generated during the run are
deleted afterward. When the run ends, it restores your settings and reloads your original
project. If no project was open, it returns to the prior mode
with a clean dashboard.

## Reading the results

In the Benchmark dialog (**About** > **Benchmark**), the results table has four columns:

- **Pipeline** names the phase.
- **Throughput** is the sustained frames per second for that phase. Higher is better.
- **Time** is the wall-clock seconds the phase ran.
- **Result** is **Pass** or **Fail** for the gated phases (up to seven), and `n/a` for the
  informational export and dashboard phases. A gated phase fails if its throughput is below its gate in
  [The phases](#the-phases).

Under the table, **Peak memory** shows the highest process memory observed during the run, in
MiB. It is included when you copy the results.

A Fail means the machine is too slow for that phase, other heavy programs are competing for the
CPU, or the build is unoptimized (a debug build, or one without the shipped PGO profile). Close
other heavy programs and run again. If you built from source, use a Release build. A Fail on a
slow laptop does not affect normal Arduino-rate use. After you edit a parser, a Fail on its
phase means the change made it slower.

**Copy** places a Markdown report on the clipboard: a header with the version, OS, CPU, workload
and build, then the table with an extra **Target** column holding each gate (`n/a` for
informational rows), then **Peak memory**. **Clear** empties the
table. Both are enabled only when results exist and no run is active. **Close** dismisses the
dialog (also clearing the table) and is disabled during a run. For CI and deployment gating, use
the headless [CLI form](Command-Line-Interface.md#acquisition-pipeline-benchmark), which
defaults to 1 M frames and 10 seconds.

## See also

- [The Acquisition Pipeline](Data-Hotpath.md): the pipeline being measured, stage by stage.
- [Threading and Timing Guarantees](Threading-and-Timing.md): where the pipeline normally runs,
  why it's single-threaded internally, and what that guarantees.
- [Frame Parser Scripting](Frame-Parser.md): the `parse()` API and the Lua-vs-JavaScript
  performance characteristics the benchmark quantifies.
- [Command-Line Interface](Command-Line-Interface.md#acquisition-pipeline-benchmark): the headless benchmark
  used for CI and deployment gating.
- [Operation Modes](Operation-Modes.md): the modes the benchmark runs under and why its button
  hides in runtime mode.
