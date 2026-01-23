# Running Security Tests with Pytest

## Quick Start

```bash
# Run all security tests
pytest tests/security/ -v

# Run only non-destructive tests
pytest tests/security/ -v -m "not destructive"

# Run specific test file
pytest tests/security/test_api_security.py -v

# Run with detailed output
pytest tests/security/ -v -s

# Run tests in parallel (faster)
pytest tests/security/ -v -n auto
```

## Test Organization

### Pytest-Compatible Tests (Recommended)

- `test_api_security.py` - JSON exploits, injection attacks, buffer overflow ✅
- More tests coming soon (being refactored)

### Legacy Tests (Standalone Scripts)

These can still be run directly:
- `test_api_security_legacy.py`
- `test_protocol_fuzzing_legacy.py`
- `test_dos_attacks_legacy.py`
- `test_auth_bypass_legacy.py`
- `test_advanced_exploits_legacy.py`
- `weaponized_exploits.py`

Run legacy tests:
```bash
python3 tests/security/test_api_security_legacy.py
python3 tests/security/weaponized_exploits.py
```

## Pytest Markers

Filter tests by severity or type:

```bash
# Run only critical vulnerability tests
pytest tests/security/ -v -m critical

# Run only high severity tests
pytest tests/security/ -v -m high

# Run fuzzing tests
pytest tests/security/ -v -m fuzzing

# Run DoS tests
pytest tests/security/ -v -m dos

# Run exploit tests
pytest tests/security/ -v -m exploit

# Combine markers
pytest tests/security/ -v -m "critical and not destructive"
pytest tests/security/ -v -m "security and not slow"
```

## Test Output

### Successful Defense (PASS)

```
tests/security/test_api_security.py::TestJSONExploits::test_deeply_nested_json PASSED
```

Means: Server successfully defended against deeply nested JSON attack.

### Failed Defense (FAIL)

```
tests/security/test_api_security.py::TestInjectionAttacks::test_path_traversal FAILED
```

Means: **VULNERABILITY FOUND** - Server is vulnerable to this attack.

## Example Sessions

### Basic Security Scan

```bash
cd tests/security
pytest test_api_security.py -v
```

### Safe Testing (Skip Destructive Tests)

```bash
pytest tests/security/ -v -m "not destructive"
```

This skips tests that might crash the server.

### Comprehensive Penetration Test

```bash
# Run all security tests including destructive ones
pytest tests/security/ -v -s --tb=short

# If server crashes, check which test caused it
```

### Quick Vulnerability Check

```bash
# Only test for critical and high severity issues
pytest tests/security/ -v -m "critical or high"
```

## Integration with CI/CD

### GitHub Actions Example

```yaml
- name: Security Tests (Non-Destructive)
  run: |
    pytest tests/security/ -v -m "not destructive" --tb=short
```

### Local Pre-Commit

```bash
# Add to .git/hooks/pre-push
pytest tests/security/test_api_security.py -v -m "not destructive"
```

## Interpreting Results

### All Tests Pass ✅

```
======================== 15 passed in 10.23s ========================

✅ All security tests passed - No exploitable vulnerabilities found
```

**Excellent!** Your API successfully defended against all attacks.

### Some Tests Fail ❌

```
======================== 12 passed, 3 failed in 10.23s ========================

⚠️ VULNERABILITIES FOUND - Review failed tests for details
```

**Action Required:** Review failed tests to identify vulnerabilities.

### Server Crashes During Tests 💥

```
ERROR: Server not responding!
```

**Critical:** A test crashed the server. Identify which test by running with `-v` and checking the last test before crash.

## Advanced Usage

### Run with Coverage

```bash
pytest tests/security/ --cov=app/src/API --cov-report=html
```

### Run with Timeout

```bash
pytest tests/security/ -v --timeout=60
```

### Run Specific Test Class

```bash
pytest tests/security/test_api_security.py::TestJSONExploits -v
```

### Run Specific Test Method

```bash
pytest tests/security/test_api_security.py::TestJSONExploits::test_deeply_nested_json -v
```

### Run with Parameterized Values

```bash
# Run all parametrized path traversal tests
pytest tests/security/test_api_security.py::TestInjectionAttacks::test_path_traversal -v
```

## Debugging Failed Tests

### Verbose Output

```bash
pytest tests/security/ -vv -s
```

### Show Full Error Traceback

```bash
pytest tests/security/ -v --tb=long
```

### Stop on First Failure

```bash
pytest tests/security/ -v -x
```

### Enter Debugger on Failure

```bash
pytest tests/security/ -v --pdb
```

## Writing New Security Tests

Follow the pytest-compatible pattern:

```python
import pytest

@pytest.mark.security
@pytest.mark.critical
class TestMyExploit:
    def test_something_dangerous(self, security_client, vuln_tracker):
        """Test description"""
        # Try to exploit
        try:
            result = security_client.command("dangerous.command")
            # If it succeeds, log vulnerability
            vuln_tracker.log_vulnerability(
                "CRITICAL",
                "Category",
                "Description",
                payload
            )
            pytest.fail("Vulnerability: exploit succeeded")
        except Exception:
            # Expected - server defended
            pass
```

## Migrating Legacy Tests

To convert a legacy test to pytest:

1. Import pytest
2. Organize into test classes
3. Use fixtures instead of manual setup
4. Use assert instead of print
5. Use pytest.fail() for vulnerabilities
6. Add appropriate markers

See `test_api_security.py` for examples.

## Tips

1. **Start with safe tests**: Use `-m "not destructive"` until you're confident
2. **Run sequentially first**: Parallel execution (`-n auto`) can make debugging harder
3. **Watch resource usage**: Security tests can be resource-intensive
4. **Have a backup**: Tests may crash the server
5. **Review failures**: Each failed test represents a real vulnerability

## Getting Help

```bash
# List all available markers
pytest --markers

# Show available fixtures
pytest --fixtures

# Show test collection
pytest --collect-only tests/security/
```

## Report Generation

### JUnit XML (for CI)

```bash
pytest tests/security/ -v --junit-xml=security-report.xml
```

### HTML Report

```bash
pytest tests/security/ -v --html=security-report.html --self-contained-html
```

### JSON Report

```bash
pip install pytest-json-report
pytest tests/security/ -v --json-report --json-report-file=security-report.json
```

---

## Red Team vs Blue Team with Pytest

### Red Team (Find Vulnerabilities)

```bash
# Run all tests - see what fails
pytest tests/security/ -v
```

Failed tests = Vulnerabilities found!

### Blue Team (Verify Fixes)

```bash
# After implementing fixes, re-run tests
pytest tests/security/ -v
```

Goal: All tests pass (green)!

### Continuous Verification

```bash
# Watch mode (re-run on code changes)
pip install pytest-watch
ptw tests/security/ -v
```

---

Good luck! 🎯
