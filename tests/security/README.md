# Serial Studio Security Testing Suite 🎯

**RED TEAM vs BLUE TEAM Challenge**

Comprehensive offensive security testing suite for the Serial Studio API Server. This is a full-scale penetration testing framework designed to find and exploit every possible vulnerability.

---

## ⚔️ Red Team Victory: <200ms to Compromise

**Status:** 🔴 **RED TEAM WINS** (Round 1)

The red team successfully identified **6 critical vulnerabilities** in under 200 milliseconds, achieving:
- ✅ Path traversal exploit (arbitrary file access)
- ✅ Buffer overflow via unbounded messages
- ✅ Server crash via connection exhaustion

**Blue Team Challenge:** Patch all 6 vulnerabilities and survive the next round. See [Current Vulnerability Status](#-current-vulnerability-status) below for details.

---

## ⚠️ EXTREME WARNING

**These are WEAPONIZED EXPLOITS.** This test suite contains:

- ✅ Denial of Service (DoS) attacks
- ✅ Buffer overflow exploits
- ✅ Memory corruption techniques
- ✅ Race condition exploitation (TOCTOU)
- ✅ Protocol fuzzing & parser confusion
- ✅ Command & SQL injection attempts
- ✅ Authentication bypass techniques
- ✅ Resource exhaustion attacks
- ✅ Integer overflow exploitation
- ✅ Timing side-channel attacks
- ✅ **Multi-stage exploit chains**

**⚔️ This suite WILL try to fully compromise your system!**

**Only run against a LOCAL TEST INSTANCE in an ISOLATED ENVIRONMENT!**

---

## 🔴 Current Vulnerability Status

**Red Team Status: COMPROMISED** ⚔️

Last assessment completed in **<200ms** with the following critical findings:

### Actively Exploited Vulnerabilities (6 Critical Findings)

#### 1. Path Traversal (4 variants) - CRITICAL 🔴
**Status:** ✅ EXPLOITED
**Test:** `test_api_security.py::TestInjectionAttacks::test_path_traversal`
**Payload Examples:**
- Unix: `../../../etc/passwd`
- Windows: `..\..\..\windows\system32\config\sam`
- Absolute Unix: `/etc/passwd`
- Absolute Windows: `C:\Windows\System32\config\SAM`

**Impact:** Arbitrary file access outside intended directories
**Blue Team Task:** Implement path sanitization and validation in PathPolicy

#### 2. Buffer Without Newline - HIGH 🟠
**Status:** ✅ EXPLOITED
**Test:** `test_api_security.py::TestBufferExploits::test_buffer_without_newline`
**Payload:** 10MB message without newline terminator
**Impact:** Server accepts unbounded data, potential memory exhaustion
**Blue Team Task:** Enforce newline requirement within buffer size limits

#### 3. Connection Exhaustion → Server Crash - CRITICAL 🔴
**Status:** ✅ EXPLOITED (Server Crash Confirmed)
**Test:** `test_api_security.py::TestConnectionExhaustion::test_connection_limit`
**Payload:** Simultaneous connection flood
**Impact:** Complete denial of service, server unresponsive
**Blue Team Task:** Fix connection handling to prevent crash, enforce connection limits gracefully

### Defensive Wins (10 Tests Passed) 🛡️

The following attacks were successfully defended:
- ✅ Deeply nested JSON (10,000 levels)
- ✅ Oversized JSON payloads (100MB)
- ✅ JSON with control characters
- ✅ Command injection attempts (shell, pipe, SQL)
- ✅ Raw data injection with base64

**Exploitation Speed:** <200ms from test initiation to successful compromise
**Next Steps:** Blue team must patch these 6 vulnerabilities before production deployment.

---

## Test Arsenal

### 🎯 Level 1: Basic Security Testing

#### `test_api_security.py` - General API Security
Comprehensive API attack vectors:

**JSON Exploitation:**
- Deeply nested JSON (10,000+ levels) → Stack overflow
- Oversized payloads (100MB+) → Memory exhaustion
- Malformed JSON with control characters
- Unicode normalization bombs

**Injection Attacks:**
- Path traversal: `../../../etc/passwd`
- Shell injection: `; rm -rf /`, `$(whoami)`, `` `id` ``
- SQL injection: `'; DROP TABLE datasets; --`
- Format string: `%s%s%s%s`, `%n%n%n`
- Code injection: `${7*7}`, `{{7*7}}`

**Buffer Attacks:**
- Line buffer overflow (100MB without newline)
- Rapid message flooding (1000 msg/sec)

**Batch Exploitation:**
- Oversized batches (10,000 commands)
- Recursive batch structures

**Connection Abuse:**
- Simultaneous connections (100+)
- Slowloris attack (slow data transmission, 10 bytes/sec)

**Data Injection:**
- Malicious base64 payloads (NULL bytes, binary data)
- Delimiter injection (`;`, `\r\n`, `\x00`)

#### `test_protocol_fuzzing.py` - Protocol Fuzzing
Low-level protocol attacks to trigger crashes:

**JSON Structure Fuzzing:**
- Truncated JSON, missing fields, duplicate fields
- Invalid UTF-8 sequences, Unicode BOMs
- Float edge cases (Infinity, NaN, 1e308)
- Comments in JSON (not valid), single quotes

**Boundary Testing:**
- Integer limits: INT32_MAX, INT64_MAX, negative wrap
- Special strings: null, undefined, true, false
- Empty variations, NULL bytes

**Protocol Violations:**
- Messages without newlines (hang server)
- Mixed line endings (CRLF, LF, CR)
- Pipelined requests (100 msgs at once)
- Interleaved JSON and raw data

**Random Fuzzing:**
- 100 iterations of random binary data
- Looking for unhandled edge cases

#### `test_denial_of_service.py` - Denial of Service
Multi-vector resource exhaustion:

**Memory Exhaustion:**
- Huge JSON payloads (100MB per message)
- Memory leak detection via 1000 connection churn
- Track memory growth: >500MB = vulnerability

**CPU Exhaustion:**
- Command flooding (10 threads × 1000 commands)
- Complex JSON parsing (100 nested batches)
- Measure CPU usage: >90% = vulnerability

**Connection Flooding:**
- Rapid connection establishment (500 connections)
- Slowloris attack (slow transmission)

**Bandwidth Saturation:**
- Continuous data stream (10MB/sec for 10 seconds)

**Queue Overflow:**
- Flood FrameConsumer queue (capacity: 2048)
- 10,000 rapid messages from 10 threads

**Amplification:**
- Small request → large response
- Measure amplification factor: >100x = abuse potential

#### `test_access_control.py` - Authentication & Authorization
Privilege escalation and access control bypass:

**No Authentication:**
- Direct unauthenticated API access
- Enumerate sensitive commands
- Execute privileged operations

**Network Security:**
- localhost vs 0.0.0.0 binding check
- Origin/referrer validation
- Cross-origin access

**Authorization Bypass:**
- Device connect/disconnect without auth
- File operations (load, save) without auth
- Configuration tampering
- Export control bypass

**Data Injection:**
- Raw data injection to device without auth
- Malicious project injection

**Information Disclosure:**
- System information extraction
- Error message leak analysis
- Path disclosure in errors

---

### ⚔️ Level 2: Advanced Exploitation

#### `test_exploit_techniques.py` - Advanced Techniques
Sophisticated exploitation for bypass testing:

**Race Condition Exploitation (TOCTOU):**
- 20 threads × 500 iterations each
- Connect/disconnect races
- Configuration change races
- Frame parser reset races
- **Goal:** Corrupt state, crash server

**Integer Overflow:**
- Test all limits: INT32_MAX, UINT32_MAX, INT64_MAX
- FPS overflow, points overflow
- Negative array indices
- **Goal:** Trigger undefined behavior, crashes

**Memory Corruption:**
- 10MB frame delimiters
- 100,000 level JSON nesting
- Unicode normalization bombs (`\u0061\u0301` × 100,000)
- **Goal:** Stack/heap overflow, segfault

**Parser Confusion:**
- Delimiter injection in data stream
- Newline injection in JSON strings
- NULL byte injection (`\x00`)
- **Goal:** Confuse state machine, execute arbitrary frames

**Timing Side-Channels:**
- Measure response times (50 samples, statistical analysis)
- Command enumeration via timing differences
- **Goal:** Leak command existence via timing >1ms difference

**Resource Starvation:**
- File descriptor exhaustion (1000 open sockets)
- Thread pool saturation (100 blocking requests)
- Queue overflow (10 threads × 10,000 messages)
- **Goal:** Starve resources, DoS

**Deserialization Attacks:**
- Circular reference structures (1000 levels deep)
- Type confusion (arrays as strings, objects as commands)
- **Goal:** Crash JSON parser, trigger exceptions

**Logic Bombs:**
- Invalid state transitions
- Export before connect, disconnect before connect
- Rapid mode switching (500 iterations)
- **Goal:** Trigger assertion failures, undefined states

**Compression Bombs:**
- Highly repetitive data (1MB of same character)
- **Goal:** Exploit compression, exhaust memory

**Batch Exploits:**
- Exactly 256 commands (at limit)
- Exactly 257 commands (over limit)
- Nested batch within batch
- **Goal:** Bypass batch limit enforcement

**Binary Injection:**
- Control characters (`\x00-\x1F`)
- ANSI escape codes (`\x1b[31m`)
- High bytes (`\xFF\xFE`)
- **Goal:** Confuse terminal, inject UI commands

---

### 🔥 Level 3: Weaponized Exploit Chain

#### `weaponized_exploits.py` - Multi-Stage Attack
**Full penetration test with chained vulnerabilities:**

**Phase 1: Reconnaissance 🔍**
- Enumerate all API commands
- Identify high-value targets (connect, write, open, set, load)
- Test rate limiting (measure req/sec)
- Measure max message size
- Map complete attack surface

**Phase 2: Buffer Overflow 💥**
- Socket buffer overflow (100 × 10MB chunks)
- JSON depth bomb (50,000 nesting levels)
- Check for server crash

**Phase 3: Race Condition 🏁**
- Launch 50 parallel threads
- 1000 iterations per thread
- Rapidly toggle: connect/disconnect, export on/off
- Corrupt state machine

**Phase 4: Resource Starvation 🔥**
- Open 1000 connections with partial messages
- Launch 20 threads sending 100 × 1MB messages
- Exhaust connection pool
- Exhaust memory

**Phase 5: Command Injection 💉**
- Shell injection attempts
- Path traversal attacks
- SQL injection payloads
- Format string exploits
- Check for RCE indicators (uid=, root)

**Phase 6: Logic Bypass 🚧**
- Invalid state transitions
- Send data without device connection
- Export without configuring device
- **Goal:** Bypass state validation

**Phase 7: Persistence 🎯**
- Establish 10 backdoor connections
- Maintain access via keep-alive
- Demonstrate persistence

**Success Metrics:**
- Server crash = FULL COMPROMISE
- Server unresponsive = DENIAL OF SERVICE
- Data injection without auth = CRITICAL
- Any phase success = VULNERABILITY CONFIRMED

---

## Running the Tests

### ✅ Pytest Integration (Recommended)

**The security tests are now integrated with pytest!**

```bash
# Run all security tests with pytest
pytest tests/security/ -v

# Run only non-destructive tests (safe)
pytest tests/security/ -v -m "not destructive"

# Run specific test categories
pytest tests/security/ -v -m critical
pytest tests/security/ -v -m "high or critical"
pytest tests/security/ -v -m fuzzing
```

**See [PYTEST_GUIDE.md](PYTEST_GUIDE.md) for comprehensive pytest usage.**

### Individual Test Suites

```bash
cd tests/security

# Run individual test modules with pytest (recommended)
pytest test_api_security.py -v
pytest test_protocol_fuzzing.py -v
pytest test_denial_of_service.py -v
pytest test_access_control.py -v
pytest test_exploit_techniques.py -v
pytest test_api_vulnerabilities.py -v

# ⚠️ FULL WEAPONIZED ATTACK
python3 weaponized_exploits.py
```

### Complete Red Team Assessment

```bash
cd tests/security
chmod +x run_all_security_tests.sh
./run_all_security_tests.sh
```

**This will:**
1. ✅ Verify API connectivity
2. 🔍 Run basic security tests (4 suites)
3. ⚔️ Execute advanced exploitation techniques
4. 🎯 Run weaponized exploit chain (with confirmation)
5. 📊 Generate timestamped report
6. ❤️ Perform final health check

---

## Prerequisites

1. **Serial Studio running with API enabled**
   - Launch Serial Studio
   - Enable API Server (Settings → API → Enable)
   - Verify listening on port 7777

2. **Python 3.7+**
   ```bash
   python3 --version
   ```

3. **Dependencies**
   ```bash
   pip install psutil
   ```

4. **System Requirements**
   - **RAM:** 4GB+ free (tests will consume several GB)
   - **Network:** localhost access (127.0.0.1:7777)
   - **Tools:** `nc` (netcat) for connectivity checks
   - **Permissions:** Ability to open 1000+ sockets

---

## Understanding Results

### Attack Outcomes

| Status | Meaning | Severity |
|--------|---------|----------|
| **[PWNED]** | ✅ Exploit successful | Vulnerability confirmed |
| **[BLOCKED]** | 🛡️ Defense successful | Attack mitigated |
| **[HANG]** | ⏱️ Server unresponsive | Potential DoS |
| **[CRASH]** | 💥 Server crashed | Critical failure |
| **[UNEXPECTED]** | ⚠️ Anomalous behavior | Needs investigation |

### Vulnerability Severity

- **CRITICAL** 🔴 - RCE, auth bypass, full compromise
- **HIGH** 🟠 - DoS, memory corruption, sensitive access
- **MEDIUM** 🟡 - Resource exhaustion, info disclosure
- **LOW** 🟢 - Minor leaks, timing attacks
- **INFO** ⚪ - Informational, design notes

### Expected Defenses

Your API should successfully defend against:

- ✅ JSON depth > 64 levels
- ✅ Messages > 1MB
- ✅ Batch size > 256 commands
- ✅ Connections > 8 simultaneous
- ✅ Buffer > 4MB per socket
- ✅ Raw data > 1MB
- ✅ Deeply nested structures
- ✅ Control characters in strings
- ✅ Path traversal attempts
- ✅ Invalid state transitions
- ✅ Race conditions
- ✅ Integer overflows
- ✅ Slowloris attacks
- ✅ Connection floods

---

## Report Files

Reports saved to: `security_report_YYYYMMDD_HHMMSS.txt`

**Contents:**
- Timestamp and system info
- All test suite results
- Vulnerabilities by severity
- Attack success/failure counts
- Final server health status
- **Compromise status** (weaponized chain)

---

## Red Team vs Blue Team Challenge

### 🔴 Red Team (Attacker)

**Objective:** Find vulnerabilities and achieve compromise

1. Run all tests: `./run_all_security_tests.sh`
2. Analyze report for [PWNED] results
3. Document successful exploits
4. Chain vulnerabilities for maximum impact
5. **Goal:** Crash server or achieve unauthorized access

**Success Criteria:**
- 1+ CRITICAL vulnerability
- 3+ HIGH vulnerabilities
- Server crash or hang
- Unauthorized data injection

### 🔵 Blue Team (Defender)

**Objective:** Harden system to zero vulnerabilities

1. Review red team report
2. Implement security controls
3. Re-run tests to verify fixes
4. **Goal:** All attacks show [BLOCKED]

**Success Criteria:**
- Zero [PWNED] results
- All depth/size/rate limits enforced
- Server survives all tests
- Graceful error handling

---

## Security Controls Reference

### Implemented Defenses (Current)

The codebase currently has these security controls:

1. **JSON Depth Limit:** 64 levels (`kMaxApiJsonDepth`)
2. **Message Size Limit:** 1MB (`kMaxApiMessageBytes`)
3. **Buffer Size Limit:** 4MB per socket (`kMaxApiBufferBytes`)
4. **Connection Limit:** 8 max (`kMaxApiClients`)
5. **Batch Size Limit:** 256 commands (`kMaxBatchCommands`)
6. **Raw Data Limit:** 1MB (`kMaxApiRawBytes`)
7. **Localhost Binding:** Only 127.0.0.1 (not 0.0.0.0)
8. **Buffer Cleanup:** On disconnect
9. **Error Responses:** Proper error codes

### Attack Vectors to Test

Use these tests to verify defenses:

| Attack | Test File | Expected Defense |
|--------|-----------|------------------|
| JSON Bomb | test_exploit_techniques.py | Depth limit enforced |
| Buffer Overflow | weaponized_exploits.py | 4MB limit enforced |
| Connection Flood | test_denial_of_service.py | 8 client limit |
| Batch Abuse | test_exploit_techniques.py | 256 command limit |
| Race Condition | test_exploit_techniques.py | Thread-safe operations |
| Slowloris | test_denial_of_service.py | Connection timeout |
| Memory Bomb | weaponized_exploits.py | Memory limit |
| Path Traversal | test_api_security.py | ❌ VULNERABLE - needs fix |
| Connection Exhaustion | test_api_security.py | ❌ VULNERABLE - server crash |

---

## Contributing

To add new attack vectors:

1. Choose appropriate test file
2. Create test function following pattern:
   ```python
   def test_new_attack(tester):
       print("\n[*] Testing new attack...")
       # Attack implementation
       if vulnerable:
           tester.log_exploit("Attack Name", True, "Details")
       else:
           tester.log_exploit("Attack Name", False, "Defended")
   ```
3. Add to `main()` function
4. Document in this README
5. Consider adding to weaponized chain if critical

---

## License

Copyright (C) 2020-2025 Alex Spataru

SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial

---

## Let the Games Begin! 🎮

**Red Team:** Try to pwn the system
**Blue Team:** Defend against all attacks
**Goal:** Build the most secure Serial Studio possible

Good luck! 🎯
