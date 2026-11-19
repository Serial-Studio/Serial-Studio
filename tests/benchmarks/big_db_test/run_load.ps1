# Load run for the big_db_test project (Windows / PowerShell).
#
# Starts the UDP frame simulator (both sources), runs the app loading the big
# project against it, and lets the app quit itself via --exit-after. Used by
# CI for PGO training (transform, dashboard and widget-paint coverage) and as
# a non-crash verification of the optimized binary.
#
# Two modes:
#   windowed (default)  The full QML interface on the offscreen platform with
#                       the software scene graph: no desktop session is needed,
#                       and the widgets are built, laid out and painted. The
#                       project's control script walks every workspace
#                       (--define BIGDB_TOUR=1) and logs each one it shows.
#   headless            No QML at all: parse, transform and export paths only.
#
# The app must exit through its own event loop: PGO-instrumented builds flush
# profile data (.profraw/.pgc) only on a normal exit, so killing it here used
# to silently discard the whole training run. A kill is now the failure path,
# taken only when the app misses its exit window.
#
# A windowed run that never shows a workspace is a failure too: it exits 0
# having painted nothing, and a training run would then profile no widget code
# without anyone noticing. The proof is the app's own log, so its stdout goes
# to a file.
#
# Usage: run_load.ps1 -App <app.exe> [-Seconds 25] [-Mode windowed|headless]
#
# Exits non-zero if the app crashes before the window elapses, has to be
# killed because it failed to quit gracefully within the grace period, or (in
# windowed mode) showed no workspace.
param(
    [Parameter(Mandatory = $true)][string]$App,
    [int]$Seconds = 25,
    [ValidateSet('windowed', 'headless')][string]$Mode = 'windowed'
)

$dir     = Split-Path -Parent $MyInvocation.MyCommand.Path
$port    = 8080
$vibPort = 8081
$log     = Join-Path ([System.IO.Path]::GetTempPath()) "big_db_test_$PID.log"

if ($Mode -eq 'windowed') {
    $modeArgs = @('-platform', 'offscreen', '--define', 'BIGDB_TOUR=1')
    $env:QT_QUICK_BACKEND = 'software'
}
else {
    $modeArgs = @('--headless')
}

# Ask WER for a minidump on any crash: the exit code alone cannot say whether the app died
# mid-run or during teardown, and CI uploads the dump for offline triage. CI runner images ship
# with WER reporting disabled, so re-enable it and register LocalDumps under both hives.
$dumpDir = Join-Path $dir 'dumps'
$exeName = Split-Path -Leaf $App
try {
    New-Item -ItemType Directory -Force -Path $dumpDir | Out-Null
    $werRoot = 'SOFTWARE\Microsoft\Windows\Windows Error Reporting'
    foreach ($hive in @('HKLM:', 'HKCU:')) {
        New-Item -Path "$hive\$werRoot" -Force | Out-Null
        Set-ItemProperty -Path "$hive\$werRoot" -Name Disabled -Value 0 -Type DWord
        Set-ItemProperty -Path "$hive\$werRoot" -Name DontShowUI -Value 1 -Type DWord
        $werKey = "$hive\$werRoot\LocalDumps\$exeName"
        New-Item -Path $werKey -Force | Out-Null
        Set-ItemProperty -Path $werKey -Name DumpFolder -Value $dumpDir -Type ExpandString
        Set-ItemProperty -Path $werKey -Name DumpType -Value 2 -Type DWord
        Set-ItemProperty -Path $werKey -Name DumpCount -Value 4 -Type DWord
    }
}
catch {
    Write-Host "big_db_test: WER LocalDumps setup skipped ($_)"
}

# One faulted channel per board type keeps the diagnostics decode path under
# load: healthy boards suppress their diagnostics frame entirely.
$sim = Start-Process -FilePath 'python' `
    -ArgumentList @("$dir/big_db_test.py", '--host', '127.0.0.1', '--port', "$port", `
        '--vib-port', "$vibPort", '--rate', '50', '--faults', 'TA:2:5,TB:1:3,TC:3:7') `
    -PassThru -NoNewWindow
$env:SS_TEARDOWN_TRACE = '1'
$env:SS_CRASH_DUMP_DIR = $dumpDir
$clock = [System.Diagnostics.Stopwatch]::StartNew()
$appProc = Start-Process -FilePath $App `
    -ArgumentList ($modeArgs + @('--project', "$dir/big_db_test.ssproj", '--udp', "$port", `
            '--exit-after', "$Seconds")) `
    -RedirectStandardOutput $log -PassThru -NoNewWindow

$grace = 30
$rc = 0
if ($appProc.WaitForExit(($Seconds + $grace) * 1000)) {
    $rc = $appProc.ExitCode
    $elapsed = [math]::Round($clock.Elapsed.TotalSeconds, 1)
    $shown = @(Select-String -Path $log -Pattern '\[BigDB\] workspace ' -ErrorAction SilentlyContinue).Count
    if ($rc -eq 0 -and $Mode -eq 'windowed' -and $shown -eq 0) {
        Write-Host 'big_db_test: the dashboard never showed a workspace (nothing was painted)'
        $rc = 1
    }
    elseif ($rc -eq 0) {
        Write-Host "big_db_test: $Mode load run completed $Seconds s and exited cleanly ($shown workspaces shown)"
    }
    else {
        $phase = if ($elapsed -lt $Seconds) { 'mid-run' } else { 'during teardown' }
        Write-Host "big_db_test: app exited with rc=$rc after $elapsed s ($phase)"
    }
}
else {
    $appProc.Kill()
    Write-Host "big_db_test: app failed to exit gracefully (killed after $Seconds s + $grace s grace)"
    $rc = 1
}

try { $sim.Kill() } catch { }

if ($rc -ne 0 -and (Test-Path $log)) {
    Write-Host 'big_db_test: last lines of the app log:'
    Get-Content -Path $log -Tail 40
}

Remove-Item -Path $log -ErrorAction SilentlyContinue

# Surface any crash dump: name it in the log and print a stack if a debugger is on the image.
if ($rc -ne 0) {
    Start-Sleep -Seconds 5
    $dumps = Get-ChildItem -Path $dumpDir -Filter '*.dmp' -ErrorAction SilentlyContinue
    foreach ($dump in $dumps) {
        Write-Host "big_db_test: crash dump written: $($dump.FullName)"
        $cdb = Get-ChildItem -Path "${env:ProgramFiles(x86)}\Windows Kits\10\Debuggers\x64\cdb.exe" `
            -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($cdb) {
            & $cdb.FullName -z $dump.FullName -c '.symfix; .reload; !analyze -v; k 40; q' 2>$null
        }
    }
    if (-not $dumps) {
        Write-Host "big_db_test: no crash dump found in $dumpDir"
    }
}

exit $rc
