# Headless load run for the big_db_test project (Windows / PowerShell).
#
# Starts the UDP frame simulator, runs the app headless loading the big project
# against a UDP source, and lets the app quit itself via --exit-after. Used by
# CI for PGO training (heavy transform / dashboard coverage) and as a non-crash
# verification of the optimized binary.
#
# The app must exit through its own event loop: PGO-instrumented builds flush
# profile data (.profraw/.pgc) only on a normal exit, so killing it here used
# to silently discard the whole training run. A kill is now the failure path,
# taken only when the app misses its exit window.
#
# Usage: run_load.ps1 -App <app.exe> [-Seconds 25]
#
# Exits non-zero if the app crashes before the window elapses or has to be
# killed because it failed to quit gracefully within the grace period.
param(
    [Parameter(Mandatory = $true)][string]$App,
    [int]$Seconds = 25
)

$dir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$port = 8080

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
    -ArgumentList @("$dir/big_db_test.py", '--host', '127.0.0.1', '--port', "$port", '--rate', '50', `
        '--faults', 'TA:2:5,TB:1:3,TC:3:7') `
    -PassThru -NoNewWindow
$clock = [System.Diagnostics.Stopwatch]::StartNew()
$appProc = Start-Process -FilePath $App `
    -ArgumentList @('--headless', '--project', "$dir/big_db_test.ssproj", '--udp', "$port", `
        '--exit-after', "$Seconds") `
    -PassThru -NoNewWindow

$grace = 30
$rc = 0
if ($appProc.WaitForExit(($Seconds + $grace) * 1000)) {
    $rc = $appProc.ExitCode
    $elapsed = [math]::Round($clock.Elapsed.TotalSeconds, 1)
    if ($rc -eq 0) {
        Write-Host "big_db_test: load run completed $Seconds s and exited cleanly"
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
