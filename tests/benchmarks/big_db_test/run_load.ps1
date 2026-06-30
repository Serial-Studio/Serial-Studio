# Headless load run for the big_db_test project (Windows / PowerShell).
#
# Starts the UDP frame simulator, runs the app headless loading the big project
# against a UDP source for -Seconds, then stops the simulator. Used by CI for
# PGO training (heavy transform / dashboard coverage) and as a non-crash
# verification of the optimized binary.
#
# Usage: run_load.ps1 -App <app.exe> [-Seconds 25]
#
# Exits non-zero only if the app crashes on its own before the window elapses.
# Surviving the window (we terminate it) is the success case and exits 0.
param(
    [Parameter(Mandatory = $true)][string]$App,
    [int]$Seconds = 25
)

$dir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$port = 8080

$sim = Start-Process -FilePath 'python' `
    -ArgumentList @("$dir/big_db_test.py", '--host', '127.0.0.1', '--port', "$port", '--rate', '50') `
    -PassThru -NoNewWindow
$appProc = Start-Process -FilePath $App `
    -ArgumentList @('--headless', '--project', "$dir/big_db_test.ssproj", '--udp', "$port") `
    -PassThru -NoNewWindow

$rc = 0
if ($appProc.WaitForExit($Seconds * 1000)) {
    $rc = $appProc.ExitCode
    Write-Host "big_db_test: app exited early (rc=$rc)"
}
else {
    $appProc.Kill()
    Write-Host "big_db_test: load run completed $Seconds s without crashing"
}

try { $sim.Kill() } catch { }
exit $rc
