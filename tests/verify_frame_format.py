#!/usr/bin/env python3
"""
Verification script for DeviceSendsJSON frame format.
Verifies all requirements are met.
"""

from utils.data_generator import DataGenerator, ChecksumType
import json
import sys


def verify_frame_format():
    """Verify frame format is correct for DeviceSendsJSON mode."""
    print("=" * 70)
    print("FRAME FORMAT VERIFICATION")
    print("=" * 70)
    print()

    # Generate test frame
    frame_data = DataGenerator.generate_json_frame()
    payload = json.dumps(frame_data)
    frame = DataGenerator.wrap_frame(payload)  # Default mode="json"

    # Run checks
    checks_passed = 0
    checks_failed = 0

    # Check 1: Has /* start delimiter
    if frame.startswith(b'/*'):
        print("✓ Has /* start delimiter")
        checks_passed += 1
    else:
        print("✗ MISSING /* start delimiter")
        checks_failed += 1

    # Check 2: Has */ end delimiter
    if b'*/' in frame:
        print("✓ Has */ end delimiter")
        checks_passed += 1
    else:
        print("✗ MISSING */ end delimiter")
        checks_failed += 1

    # Check 3: No checksum after */
    end_pos = frame.rindex(b'*/')
    checksum_section = frame[end_pos+2:-1]
    if len(checksum_section) == 0:
        print("✓ No checksum after */")
        checks_passed += 1
    else:
        print(f"✗ UNEXPECTED checksum after */: {repr(checksum_section)}")
        checks_failed += 1

    # Check 4: Ends with newline
    if frame.endswith(b'\n'):
        print("✓ Ends with newline")
        checks_passed += 1
    else:
        print("✗ Does NOT end with newline")
        checks_failed += 1

    # Check 5: Valid JSON payload
    try:
        payload_bytes = frame[2:end_pos]  # Between /* and */
        json.loads(payload_bytes.decode('utf-8'))
        print("✓ Valid JSON payload")
        checks_passed += 1
    except Exception as e:
        print(f"✗ INVALID JSON payload: {e}")
        checks_failed += 1

    # Check 6: Has widget definitions
    if '"widget"' in payload:
        print("✓ Widget definitions present")
        checks_passed += 1
    else:
        print("✗ MISSING widget definitions")
        checks_failed += 1

    # Check 7: GPS datasets in correct order (lat, alt, lon)
    gps_widgets = ['lat', 'alt', 'lon']
    has_gps = all(f'"{w}"' in payload for w in gps_widgets)
    if has_gps:
        # Check order
        lat_pos = payload.index('"lat"')
        alt_pos = payload.index('"alt"')
        lon_pos = payload.index('"lon"')
        if lat_pos < alt_pos < lon_pos:
            print("✓ GPS datasets in correct order (lat, alt, lon)")
            checks_passed += 1
        else:
            print("✗ GPS datasets in WRONG order")
            checks_failed += 1
    else:
        print("⚠ No GPS group found (optional)")

    print()
    print("=" * 70)
    print(f"RESULTS: {checks_passed} passed, {checks_failed} failed")
    print("=" * 70)
    print()

    if checks_failed > 0:
        print("❌ Frame format verification FAILED!")
        print()
        print("Frame preview:")
        print(f"  Start: {repr(frame[:50])}")
        print(f"  End:   {repr(frame[-30:])}")
        return False
    else:
        print("✅ Frame format verification PASSED!")
        print()
        print("Frame details:")
        print(f"  Length: {len(frame)} bytes")
        print(f"  Format: /*<JSON payload>*/\\n")
        print(f"  Preview: {repr(frame[:80])}...")
        return True


if __name__ == "__main__":
    success = verify_frame_format()
    sys.exit(0 if success else 1)
