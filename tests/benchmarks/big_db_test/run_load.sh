#!/usr/bin/env bash
# Load run for the big_db_test project (POSIX: Linux + macOS).
#
# Starts the UDP frame simulator (both sources), runs the app loading the big
# project against it, and lets the app quit itself via --exit-after. Used by
# CI for PGO training (transform, dashboard and widget-paint coverage) and as
# a non-crash verification of the optimized binary.
#
# Two modes:
#   windowed (default)  The full QML interface on the offscreen platform with
#                       the software scene graph: no display is needed, and the
#                       widgets are built, laid out and painted. The project's
#                       control script walks every workspace (--define
#                       BIGDB_TOUR=1) and logs each one it shows.
#   headless            No QML at all: parse, transform and export paths only.
#
# The app must exit through its own event loop: PGO-instrumented builds flush
# profile data (.profraw/.gcda/.pgc) only on a normal exit, so killing it here
# used to silently discard the whole training run. A kill is now the failure
# path, taken only when the app misses its exit window.
#
# A windowed run that never shows a workspace is a failure too: it exits 0
# having painted nothing, and a training run would then profile no widget code
# without anyone noticing.
#
# Usage: run_load.sh <app-binary> [seconds] [windowed|headless]
#
# Exits non-zero if the app crashes before the window elapses, has to be
# killed because it failed to quit gracefully within the grace period, or (in
# windowed mode) showed no workspace.
set -u

APP="${1:?app binary path required}"
SECS="${2:-25}"
MODE="${3:-windowed}"
DIR="$(cd "$(dirname "$0")" && pwd)"
PORT=8080
VIB_PORT=8081
GRACE=30

case "$MODE" in
  windowed)
    MODE_ARGS=(-platform offscreen --define BIGDB_TOUR=1)
    export QT_QUICK_BACKEND=software
    ;;
  headless)
    MODE_ARGS=(--headless)
    ;;
  *)
    echo "big_db_test: unknown mode '$MODE' (want windowed or headless)"
    exit 2
    ;;
esac

LOG="$(mktemp "${TMPDIR:-/tmp}/big_db_test.XXXXXX")"

# One faulted channel per board type keeps the diagnostics decode path under
# load: healthy boards suppress their diagnostics frame entirely.
python3 "$DIR/big_db_test.py" --host 127.0.0.1 --port "$PORT" --vib-port "$VIB_PORT" \
  --rate 50 --faults 'TA:2:5,TB:1:3,TC:3:7' >/dev/null 2>&1 &
SIM=$!

"$APP" "${MODE_ARGS[@]}" --project "$DIR/big_db_test.ssproj" --udp "$PORT" \
  --exit-after "$SECS" >"$LOG" 2>&1 &
APP_PID=$!

i=0
limit=$((SECS + GRACE))
while kill -0 "$APP_PID" 2>/dev/null && [ "$i" -lt "$limit" ]; do
  sleep 1
  i=$((i + 1))
done

rc=0
if kill -0 "$APP_PID" 2>/dev/null; then
  kill -9 "$APP_PID" 2>/dev/null || true
  wait "$APP_PID" 2>/dev/null || true
  echo "big_db_test: app failed to exit gracefully (killed after ${SECS}s + ${GRACE}s grace)"
  rc=1
else
  wait "$APP_PID"
  rc=$?
fi

kill "$SIM" 2>/dev/null || true
wait "$SIM" 2>/dev/null || true

shown=$(grep -c '\[BigDB\] workspace ' "$LOG")
if [ "$rc" -eq 0 ] && [ "$MODE" = windowed ] && [ "$shown" -eq 0 ]; then
  echo "big_db_test: the dashboard never showed a workspace (nothing was painted)"
  rc=1
fi

if [ "$rc" -eq 0 ]; then
  echo "big_db_test: $MODE load run completed ${SECS}s and exited cleanly ($shown workspaces shown)"
else
  if [ "$i" -lt "$SECS" ]; then
    echo "big_db_test: app exited early (rc=$rc)"
  fi
  echo "big_db_test: last lines of the app log:"
  tail -n 40 "$LOG"
fi

rm -f "$LOG"
exit "$rc"
