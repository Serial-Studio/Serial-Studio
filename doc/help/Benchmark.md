# Benchmark Dialog

The Benchmark dialog measures how fast the machine you're running on can extract, parse, and
visualize frames through Serial Studio's data pipeline. It drives the real hot path, not a
synthetic micro-benchmark, so the numbers it reports are the throughput your hardware would
sustain on live data. Use it to verify a build, compare machines, or check that a
parser or transform change hasn't regressed throughput.

This page describes the interactive, in-app dialog. For the headless form used in CI and
deployment gating, see [Command-Line Interface](Command-Line-Interface.md#hotpath-benchmark);
both run the same `HotpathBenchmark` engine. For the architecture being measured, see
[The Data Hotpath](Data-Hotpath.md) and [Threading and Timing Guarantees](Threading-and-Timing.md).

## Opening the dialog

Open the **About** window (the **About** toolbar button, authoring mode only), then click
**Benchmark**. The dialog runs entirely on the local machine; nothing is uploaded.

## What it measures

The benchmark feeds newline-delimited synthetic frames (sine-wave numeric channels, plus
rotating non-numeric status words for the mixed phases) through the same chain that live data
takes: `FrameReader` extraction, the `FrameBuilder` decode/parse/transform stage, and the
downstream consumers. Each phase reports a sustained **throughput** in frames per second and
the wall-clock **time** it took to reach the target frame count.

Two workload selectors shape the run:

- **Frames per phase** (100 K / 250 K / 500 K / 1 M): the number of frames pushed through each
  phase. More frames give a steadier average and a longer freeze.
- **Minimum duration** (1 / 2 / 5 / 10 seconds): a wall-clock floor. A phase keeps running
  until it has both reached the frame count and run at least this long, so a fast machine still
  produces a statistically meaningful sample.

Two groups of checkboxes pick which phases run:

- **Stages** (Parsers / Data export / Dashboard): which families of phases to schedule. The raw
  data pipeline always runs and is not a toggle. The default is Parsers only, which is the
  cheapest meaningful sweep.
- **Data** (Numeric only / Mixed): whether each selected stage runs over numeric channels, the
  mixed workload (numeric channels plus trailing non-numeric string tokens), or both.

The Run button stays disabled until at least one stage and at least one data type are selected.

## The phases

The data pipeline always runs first. Each selected stage then runs every parser engine
(Built-in, Lua, JavaScript) over each selected data type. The data-pipeline and parser phases
are **gated** (Pass/Fail); the data-export and dashboard phases are **informational** and
report `n/a` in the Result column.

| Phase | What it exercises | Gate (at default 256 K min-fps) |
|---|---|---|
| **Data pipeline** | Raw `FrameReader` extraction only: delimiter scan and dequeue, no parse. | 1024 K frames/s |
| **Built-in parser (numeric)** | Native C++ template over numeric channels. | 1024 K frames/s |
| **Built-in parser (mixed)** | Native template with trailing non-numeric string tokens. | 512 K frames/s |
| **Lua parser (numeric)** | `parse()` in Lua 5.4 over numeric channels. | 256 K frames/s |
| **JavaScript parser (numeric)** | `parse()` in `QJSEngine` over the same channels. | 128 K frames/s |
| **Lua parser (mixed)** | Lua `parse()` over the mixed workload. | 128 K frames/s |
| **JavaScript parser (mixed)** | JavaScript `parse()` over the mixed workload. | 64 K frames/s |
| **Built-in / Lua / JavaScript + data export** | Each engine, numeric and/or mixed, with every export/output consumer enabled (CSV, API, plus MDF4 / Sessions / gRPC where built). | informational |
| **Built-in / Lua / JavaScript + dashboard** | Each engine, numeric and/or mixed, feeding a full dashboard (multiplot, FFT, bar, LED, waterfall, GPS, 3D plot). | informational |

The gates are **tiered off the minimum frame rate**, so JavaScript is held to half its Lua
peer and the data pipeline to four times the parser baseline. This reflects the real cost
hierarchy: raw extraction is the cheapest stage, the Built-in template is faster than the
scripted engines, Lua's stack-based API is faster than `QJSEngine`'s value boxing, and the
mixed workload (with `toDouble` failures on the string tokens) is heavier than the numeric one.
The same per-language ranking is described in
[Frame Parser Scripting — Performance Tips](JavaScript-API.md#performance-tips).

The informational phases exist to show the cost of fanning out to consumers and to the
dashboard, not to gate a build. Their throughput is expected to be lower than the gated phases.

## Why the interface freezes

A warning in the dialog states that the window stops responding while the benchmark runs. This
is by design, and it's the same property that makes the hot path fast:

- **The hot path is single-threaded on the main thread.** `FrameReader` and `FrameBuilder` run
  on the main (GUI) thread; the benchmark drives them flat-out in a tight loop, so the event
  loop can't repaint until the loop returns. See
  [Threading and Timing Guarantees](Threading-and-Timing.md#framereader-and-framebuilder-run-on-the-main-thread)
  for why frame parsing lives on the main thread.
- **The dashboard phase pumps events about every 16 ms.** Only the final dashboard phase yields
  to the event loop so widgets repaint while it measures the dashboard sub-hotpath; every other
  phase blocks straight through.
- **The benchmark is for measurement, not interactive use.** Don't run it while you need the
  UI. Pick a smaller frame count and a 1-second floor for a quick check.

## What it does to your session

The runner snapshots your session, runs the benchmark against a synthetic project, and restores
everything when it finishes. Specifically, while the benchmark runs it:

- Switches to a synthetic **Project File** project (an 8-channel comma-decoded source, or a
  full multi-widget dashboard for the dashboard phase) and loads the selected parser language.
- Redirects every exporter into a **throwaway temporary workspace**, so CSV/MDF4/Session files
  generated during the run never touch your real data and are deleted afterward.
- Toggles the export/output consumers on and off per phase, then **flushes their worker queues**
  so queued frames are released rather than pinned.
- Pins a fixed 10-second plot window for the dashboard phase.

When the run ends, the runner restores your operation mode, plot window, and consumer toggles
to exactly what they were, and **reloads your original project** (or returns to the prior mode
with a clean dashboard if no project was open). If you had a project open before benchmarking,
it comes back automatically.

## Reading the results

- **Throughput** is the sustained frames/second for that phase. Higher is better.
- **Time** is the wall-clock seconds the phase ran.
- **Result** is **Pass** (green) or **Fail** (red) for the five gated phases, and `n/a` for the
  two informational ones. A gated phase fails if its throughput is below the tiered target shown
  above.

A failing gated phase usually means the machine is too slow for the default 256 kHz target, the
build is unoptimized (a debug build, or one without the shipped PGO profile), or a parser /
transform change regressed throughput. The headless
[CLI form](Command-Line-Interface.md#hotpath-benchmark) is what CI uses to enforce these gates
per pull request and on the shipped binary; the dialog is the same measurement on demand.

**Clear** empties the results table; **Close** dismisses the dialog (also clearing the table).
Neither is available while a benchmark is running.

## See also

- [The Data Hotpath](Data-Hotpath.md): the pipeline being measured, stage by stage.
- [Threading and Timing Guarantees](Threading-and-Timing.md): why the hot path is
  single-threaded and what that guarantees.
- [Frame Parser Scripting](JavaScript-API.md): the `parse()` API and the Lua-vs-JavaScript
  performance characteristics the benchmark quantifies.
- [Command-Line Interface](Command-Line-Interface.md#hotpath-benchmark): the headless benchmark
  used for CI and deployment gating.
- [Operation Modes](Operation-Modes.md): the Project File mode the benchmark runs under.
