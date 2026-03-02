@echo off
rem -------------------------------------------------------------------------
rem system-monitor.bat
rem
rem Launch wrapper for the System Monitor example on Windows.
rem Serial Studio (Process driver) can point to this .bat file directly so
rem users do not need to type the Python command manually.
rem
rem Usage (from Command Prompt, optional):
rem   system-monitor.bat [--interval SECONDS]
rem
rem Copyright (C) 2020-2025 Alex Spataru
rem SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
rem -------------------------------------------------------------------------

setlocal

set "SCRIPT_DIR=%~dp0"

rem Locate Python (try py launcher first, then plain python/python3).
where py >nul 2>&1
if %errorlevel% == 0 (
    set "PYTHON=py -3"
    goto :check_psutil
)

where python3 >nul 2>&1
if %errorlevel% == 0 (
    set "PYTHON=python3"
    goto :check_psutil
)

where python >nul 2>&1
if %errorlevel% == 0 (
    set "PYTHON=python"
    goto :check_psutil
)

echo ERROR: Python not found. Install Python 3 from https://python.org and try again. 1>&2
exit /b 1

:check_psutil
%PYTHON% -c "import psutil" >nul 2>&1
if %errorlevel% neq 0 (
    echo Installing psutil... 1>&2
    %PYTHON% -m pip install --quiet psutil 1>&2
)

%PYTHON% "%SCRIPT_DIR%system-monitor.py" %*
