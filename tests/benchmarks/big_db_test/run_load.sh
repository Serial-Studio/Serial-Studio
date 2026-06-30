#!/usr/bin/env bash
# Headless load run for the big_db_test project (POSIX: Linux + macOS).
#
# Starts the UDP frame simulator, runs the app headless loading the big project
# against a UDP source for <seconds>, then stops the simulator. Used by CI for
# PGO training (heavy transform / dashboard coverage) and as a non-crash
# verification of the optimized binary.
#
# Usage: run_load.sh <app-binary> [seconds]
#
# Exits non-zero only if the app crashes on its own before the window elapses.
# Surviving the window (we terminate it) is the success case and exits 0.
set -u

APP="${1:?app binary path required}"
SECS="${2:-25}"
DIR="$(cd "$(dirname "$0")" && pwd)"
PORT=8080

python3 "$DIR/big_db_test.py" --host 127.0.0.1 --port "$PORT" --rate 50 >/dev/null 2>&1 &
SIM=$!

"$APP" --headless --project "$DIR/big_db_test.ssproj" --udp "$PORT" >/dev/null 2>&1 &
APP_PID=$!

rc=0
i=0
while [ "$i" -lt "$SECS" ]; do
  if ! kill -0 "$APP_PID" 2>/dev/null; then
    wait "$APP_PID"
    rc=$?
    echo "big_db_test: app exited early (rc=$rc)"
    break
  fi
  sleep 1
  i=$((i + 1))
done

kill "$APP_PID" 2>/dev/null || true
kill "$SIM" 2>/dev/null || true
wait "$APP_PID" 2>/dev/null || true
wait "$SIM" 2>/dev/null || true

if [ "$rc" -eq 0 ]; then
  echo "big_db_test: load run completed ${SECS}s without crashing"
fi
exit "$rc"
