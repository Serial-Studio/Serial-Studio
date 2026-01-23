#!/bin/bash
#
# Master security test runner
# Executes all security test suites and generates consolidated report
#
# Copyright (C) 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPORT_FILE="$SCRIPT_DIR/security_report_$(date +%Y%m%d_%H%M%S).txt"

echo "=========================================="
echo "Serial Studio Security Test Suite"
echo "=========================================="
echo ""
echo "WARNING: These tests will stress the Serial Studio API."
echo "Only run against a test instance!"
echo ""
echo "Press Ctrl+C to cancel, or Enter to continue..."
read

# Check if Serial Studio is running
echo ""
echo "[*] Checking if Serial Studio API is accessible..."
if ! nc -z 127.0.0.1 7777 2>/dev/null; then
    echo "[ERROR] Cannot connect to Serial Studio API on port 7777"
    echo "Please start Serial Studio with API Server enabled"
    exit 1
fi
echo "[OK] API is accessible"

# Start report
{
    echo "=========================================="
    echo "Serial Studio Security Test Report"
    echo "=========================================="
    echo "Date: $(date)"
    echo "Host: $(hostname)"
    echo ""
} > "$REPORT_FILE"

# Test 1: API Security
echo ""
echo "=========================================="
echo "Running API Security Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "1. API Security Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_api_security.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 2: Protocol Fuzzing
echo ""
echo "=========================================="
echo "Running Protocol Fuzzing Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "2. Protocol Fuzzing Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_protocol_fuzzing.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 3: DoS Attacks
echo ""
echo "=========================================="
echo "Running DoS Attack Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "3. DoS Attack Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_dos_attacks.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 4: Authentication Bypass
echo ""
echo "=========================================="
echo "Running Authentication Bypass Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "4. Authentication Bypass Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_auth_bypass.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 5: Advanced Exploits
echo ""
echo "=========================================="
echo "Running Advanced Exploitation Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "5. Advanced Exploitation Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_advanced_exploits.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 6: Weaponized Exploit Chain
echo ""
echo "=========================================="
echo "Running Weaponized Exploit Chain"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "6. Weaponized Exploit Chain"
    echo "=========================================="
} >> "$REPORT_FILE"

echo ""
echo "⚠️  WARNING: The weaponized exploit chain will attempt"
echo "    to fully compromise the target system."
echo "    Press Ctrl+C to skip, or Enter to proceed..."
read

python3 "$SCRIPT_DIR/weaponized_exploits.py" 2>&1 | tee -a "$REPORT_FILE"

# Final check
echo ""
echo "=========================================="
echo "Final Server Health Check"
echo "=========================================="

if nc -z 127.0.0.1 7777 2>/dev/null; then
    echo "[OK] Server is still responsive"
    echo "[OK] Server survived all tests" >> "$REPORT_FILE"
else
    echo "[CRITICAL] Server is not responding!"
    echo "[CRITICAL] Server crashed or became unresponsive" >> "$REPORT_FILE"
fi

# Summary
{
    echo ""
    echo "=========================================="
    echo "Test Summary"
    echo "=========================================="
    echo "Report saved to: $REPORT_FILE"
} >> "$REPORT_FILE"

echo ""
echo "=========================================="
echo "All Tests Complete"
echo "=========================================="
echo "Report saved to: $REPORT_FILE"
echo ""
echo "Review the report to see all discovered vulnerabilities."
