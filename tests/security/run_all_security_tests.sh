#!/bin/bash
#
# Master security test runner
# Executes all security test suites and generates consolidated report
#
# Copyright (C) 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

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

# Test 3: Resource Exhaustion
echo ""
echo "=========================================="
echo "Running Resource Exhaustion Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "3. Resource Exhaustion Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_resource_exhaustion.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 4: Authentication Bypass
echo ""
echo "=========================================="
echo "Running Access Control Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "4. Access Control Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_access_control.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 5: Robustness Techniques
echo ""
echo "=========================================="
echo "Running Robustness Techniques Tests"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "5. Robustness Techniques Tests"
    echo "=========================================="
} >> "$REPORT_FILE"

python3 "$SCRIPT_DIR/test_robustness_techniques.py" 2>&1 | tee -a "$REPORT_FILE"

# Test 6: Instrumented Probe Suite
echo ""
echo "=========================================="
echo "Running Instrumented Probe Suite"
echo "=========================================="
{
    echo ""
    echo "=========================================="
    echo "6. Instrumented Probe Suite"
    echo "=========================================="
} >> "$REPORT_FILE"

echo ""
echo "⚠️  WARNING: The instrumented probe suite will attempt"
echo "    to stress the target system to failure."
echo "    Press Ctrl+C to skip, or Enter to proceed..."
read

python3 "$SCRIPT_DIR/security_probe_suite.py" 2>&1 | tee -a "$REPORT_FILE"

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
echo "Review the report to see all discovered weaknesses."
