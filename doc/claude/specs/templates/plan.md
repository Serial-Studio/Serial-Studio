---
spec: NNNN-short-slug
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: YYYY-MM-DD
---

# Plan NNNN — <Feature name>

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

<The chosen design in 3-5 sentences: what gets built, where it plugs in, and why this shape
over the alternatives. The detail lives below; this is the summary a reviewer reads first.>

## Affected subsystems & files

<Concrete paths. List every file you expect to create or edit, with a one-line role. Grep to
confirm the touch-points exist — for a new enum value or driver, every switch/list it must
appear in (see `ss-new-driver` for the driver touch-point list).>

| File | Change |
|------|--------|
| `app/src/...` | |

## Architecture & data flow

<How data and control move through the change. Name the objects, the signals/slots, and the
threads. Reference `doc/claude/architecture.md` sections rather than re-deriving them.>

## Hotpath & threading impact

<REQUIRED. Answer explicitly even if the answer is "none".>

- **Touches the hotpath?** (`FrameReader` / `CircularBuffer` / `FrameBuilder` / `Dashboard`
  draw / span fast lane) — yes/no. If yes: read the code in full, invoke `ss-hotpath`, and
  state how the SPSC / `Qt::DirectConnection` / no-alloc / slot-pool / cached-flag rules are
  preserved, plus the `--benchmark-hotpath` plan.
- **New cross-thread signal/slot?** — yes/no; connection type and why.
- **New input to a cached hotpath flag?** (`m_operationMode`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, `m_streamAvailable`, ...) — if yes, where its change signal wires
  into the cache refresh (silent breakage otherwise — see `common-mistakes.md`).
- **Timestamp ownership** — confirm the source stamps at the driver boundary; nothing
  re-stamps downstream.

## Data model & persistence

<`Frame.h` `Keys::` additions (single source of truth), schema/writer version bumps,
`widgetSettings` / project-JSON shape, Sessions DB schema, legacy-alias fallbacks. State the
migration story for anything that reads old files.>

## API / SDK surface

<New or changed API handlers (`app/src/API/Handlers/`, registered in
`CommandHandler::initializeHandlers()`), `EnumLabels.cpp` slugs/labels, generated SDK, script
`apiCall` reach. Commercial surfaces go behind `#ifdef BUILD_COMMERCIAL`.>

## QML / UI

<New components, the model that feeds them, ComboBox restore-race guards, theme/glass surface,
font auto-scale. None if headless.>

## Tradeoffs & alternatives considered

<The decisions a reviewer might make differently, surfaced up front. For each: the options,
the pick, and the one-line why. Recommend, do not enumerate — but record what was rejected so
review does not relitigate it.>

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| | | |

## Risks & mitigations

<What could regress or break, and how the plan defends against it. Include the silent-breakage
classes from `common-mistakes.md` that this change is exposed to.>

-

## Test & verification plan

<Maps each acceptance criterion to a concrete check. State which apply:>

- **Unit (you can run):** `tests/scripts/` JS-parser cases — list them.
- **Integration / security / perf (maintainer runs):** `pytest tests/...` — list the files
  and what they assert; note the app must be up with the API server enabled.
- **Hotpath:** `--benchmark-hotpath` if the hotpath is touched.
- **Static:** `python scripts/code-verify.py --check <files>`; `qt-cpp-review` before handoff;
  `python scripts/sanitize-commit.py` before commit.
