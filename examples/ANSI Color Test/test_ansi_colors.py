#!/usr/bin/env python3
"""
Serial Studio — ANSI Color Test

Sends a systematic ANSI/VT-100 color test suite via UDP to localhost:9000.
Use this to verify color rendering in the Serial Studio terminal view.
"""

import math
import socket
import sys
import time


HOST = '127.0.0.1'
PORT = 9000


# ─── Transport ────────────────────────────────────────────────────────────────

def send(text, host=HOST, port=PORT):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(text.encode('utf-8'), (host, port))
    finally:
        sock.close()


def emit(text, delay=0.0):
    send(text)
    if delay:
        time.sleep(delay)


# ─── Color primitives ─────────────────────────────────────────────────────────

R = "\033[0m"


def fg(r, g, b):
    return f"\033[38;2;{r};{g};{b}m"


def bg(r, g, b):
    return f"\033[48;2;{r};{g};{b}m"


def lerp(a, b, t):
    return int(a + (b - a) * t)


def lerp_rgb(c1, c2, t):
    return (lerp(c1[0], c2[0], t), lerp(c1[1], c2[1], t), lerp(c1[2], c2[2], t))


def hsv_to_rgb(h, s, v):
    h = h % 360
    c_val = v * s
    x = c_val * (1 - abs((h / 60) % 2 - 1))
    m = v - c_val
    if   h < 60:  rv, gv, bv = c_val, x,     0
    elif h < 120: rv, gv, bv = x,     c_val, 0
    elif h < 180: rv, gv, bv = 0,     c_val, x
    elif h < 240: rv, gv, bv = 0,     x,     c_val
    elif h < 300: rv, gv, bv = x,     0,     c_val
    else:         rv, gv, bv = c_val, 0,     x
    return (int((rv + m) * 255), int((gv + m) * 255), int((bv + m) * 255))


# ─── Section header ───────────────────────────────────────────────────────────

_section_count = 0


def section(title):
    global _section_count
    _section_count += 1
    emit(f"\n\033[1;37m[{_section_count}] {title}\033[0m\n" + "─" * 64 + "\n\n", 0.05)


# ─── 1. Text Attributes ───────────────────────────────────────────────────────

def test_attributes():
    section("Text Attributes")

    attrs = [
        ("0",     "Normal"),
        ("1",     "Bold"),
        ("2",     "Dim"),
        ("3",     "Italic"),
        ("4",     "Underline"),
        ("4:2",   "Double Underline"),
        ("4:3",   "Curly Underline"),
        ("5",     "Blink"),
        ("7",     "Reverse"),
        ("8",     "Hidden"),
        ("9",     "Strikethrough"),
        ("1;4",   "Bold + Underline"),
        ("1;7",   "Bold + Reverse"),
        ("3;9",   "Italic + Strikethrough"),
    ]

    for code, name in attrs:
        emit(f"  \033[{code}m{name:<26}\033[0m  \\033[{code}m\n", 0.06)

    emit("\n", 0.1)


# ─── 2. 4-bit Colors ──────────────────────────────────────────────────────────

def test_4bit():
    section("4-bit Colors  (16 standard ANSI colors)")

    fg_normal = [
        (30, "Black"),   (31, "Red"),     (32, "Green"),   (33, "Yellow"),
        (34, "Blue"),    (35, "Magenta"), (36, "Cyan"),    (37, "White"),
    ]
    fg_bright = [
        (90, "Br. Black"),  (91, "Br. Red"),    (92, "Br. Green"),  (93, "Br. Yellow"),
        (94, "Br. Blue"),   (95, "Br. Magenta"), (96, "Br. Cyan"),   (97, "Br. White"),
    ]
    bg_normal = [
        (40, "Black"),   (41, "Red"),     (42, "Green"),   (43, "Yellow"),
        (44, "Blue"),    (45, "Magenta"), (46, "Cyan"),    (47, "White"),
    ]
    bg_bright = [
        (100, "Br. Black"),  (101, "Br. Red"),    (102, "Br. Green"),  (103, "Br. Yellow"),
        (104, "Br. Blue"),   (105, "Br. Magenta"), (106, "Br. Cyan"),   (107, "Br. White"),
    ]

    emit("  Foreground — normal (30–37):\n")
    for code, name in fg_normal:
        emit(f"    \033[{code}m{name:<14}\033[0m  \\033[{code}m\n", 0.04)

    emit("\n  Foreground — bright (90–97):\n")
    for code, name in fg_bright:
        emit(f"    \033[{code}m{name:<14}\033[0m  \\033[{code}m\n", 0.04)

    emit("\n  Background — normal (40–47):\n")
    for code, name in bg_normal:
        emit(f"    \033[{code}m  {name:<12}  \033[0m  \\033[{code}m\n", 0.04)

    emit("\n  Background — bright (100–107):\n")
    for code, name in bg_bright:
        emit(f"    \033[{code}m  {name:<12}  \033[0m  \\033[{code}m\n", 0.04)

    emit("\n", 0.1)


# ─── 3. 4-bit FG × BG Matrix ─────────────────────────────────────────────────

def test_color_matrix():
    section("4-bit FG × BG Matrix")

    fg_codes = [30, 31, 32, 33, 34, 35, 36, 37, 97]
    bg_codes = [40, 41, 42, 43, 44, 45, 46, 47]

    header = "         " + "".join(f" \033[2m{b}\033[0m " for b in bg_codes)
    emit("  " + header + "\n")

    for f in fg_codes:
        row = f"  \033[2m{f:3d}\033[0m  "
        for b in bg_codes:
            row += f" \033[{f};{b}m Aa \033[0m"
        emit(row + "\n", 0.07)

    emit("\n", 0.1)


# ─── 4. 256-Color Palette ─────────────────────────────────────────────────────

def test_256_colors():
    section("8-bit Colors  (256-color palette)")

    emit("  System colors (0–15):\n  ")
    for i in range(16):
        emit(f"\033[48;5;{i}m  {i:2d} \033[0m")
        if (i + 1) % 8 == 0:
            emit("\n  ")
    emit("\n")
    time.sleep(0.1)

    emit("\n  6×6×6 RGB cube (16–231):\n")
    for row in range(6):
        emit("  ")
        for col in range(36):
            idx = 16 + row * 36 + col
            emit(f"\033[48;5;{idx}m  \033[0m")
            if (col + 1) % 18 == 0:
                emit("  ")
        emit("\n")
        time.sleep(0.06)

    emit("\n  Grayscale ramp (232–255):\n  ")
    time.sleep(0.1)
    for i in range(232, 256):
        emit(f"\033[48;5;{i}m  \033[0m")
        if (i - 231) % 12 == 0:
            emit("\n  ")
    emit("\n\n")
    time.sleep(0.2)


# ─── 5. 24-bit Truecolor ──────────────────────────────────────────────────────

def test_truecolor():
    section("24-bit Truecolor  (16.7 million colors)")

    # BAR * 2 chars/cell + 2-space indent = 78 chars (fits 80-col terminal)
    BAR = 38

    def color_bar(fn):
        emit("  ")
        for i in range(BAR):
            col = fn(i / (BAR - 1))
            emit(bg(*col) + "  " + R)
            time.sleep(0.006)
        emit("\n")

    # Primary channel ramps
    channel_ramps = [
        ("Red   (R→0)",  lambda t: (int(t * 255), 0,            0           )),
        ("Green (G→0)",  lambda t: (0,            int(t * 255), 0           )),
        ("Blue  (B→0)",  lambda t: (0,            0,            int(t * 255))),
        ("White (0→W)",  lambda t: (int(t * 255), int(t * 255), int(t * 255))),
    ]

    emit("  Primary channel ramps:\n")
    for name, fn in channel_ramps:
        emit(f"  {name}\n")
        color_bar(fn)
        time.sleep(0.04)

    # Multi-color gradients
    gradients = [
        ("Red → Blue",    (255,   0,   0), (  0,   0, 255)),
        ("Green → Yellow",(  0, 180,   0), (255, 220,   0)),
        ("Cyan → Magenta",(  0, 210, 210), (210,   0, 210)),
        ("Black → White", (  0,   0,   0), (255, 255, 255)),
        ("Navy → Orange", ( 10,  10, 100), (255, 140,   0)),
    ]

    emit("\n  Color gradients:\n")
    for name, start, end in gradients:
        emit(f"  {name}\n")
        color_bar(lambda t, s=start, e=end: lerp_rgb(s, e, t))
        time.sleep(0.04)

    # HSV sweeps
    emit("\n  HSV sweeps:\n")

    emit("  Hue (S=1, V=1)\n")
    color_bar(lambda t: hsv_to_rgb(t * 360, 1.0, 1.0))
    time.sleep(0.04)

    emit("  Saturation (H=0°, V=1)\n")
    color_bar(lambda t: hsv_to_rgb(0, t, 1.0))
    time.sleep(0.04)

    emit("  Value (H=0°, S=1)\n")
    color_bar(lambda t: hsv_to_rgb(0, 1.0, t))
    emit("\n")
    time.sleep(0.2)


# ─── 6. Named Truecolor Samples ───────────────────────────────────────────────

def test_named_colors():
    section("Named Truecolor Samples  (CSS/X11 reference)")

    named = [
        (255,   0,   0, "Red"),
        (  0, 128,   0, "Green"),
        (  0,   0, 255, "Blue"),
        (255, 255,   0, "Yellow"),
        (  0, 255, 255, "Cyan"),
        (255,   0, 255, "Magenta"),
        (255, 165,   0, "Orange"),
        (128,   0, 128, "Purple"),
        (255,  20, 147, "Deep Pink"),
        ( 64, 224, 208, "Turquoise"),
        (255, 215,   0, "Gold"),
        (  0, 191, 255, "Deep Sky Blue"),
        (138,  43, 226, "Blue Violet"),
        (  0, 128, 128, "Teal"),
        (210, 105,  30, "Chocolate"),
        (128, 128, 128, "Gray"),
        (192, 192, 192, "Silver"),
        ( 75,   0, 130, "Indigo"),
    ]

    for rv, gv, bv, name in named:
        swatch = bg(rv, gv, bv) + "    " + R
        label  = fg(rv, gv, bv) + f"  {name:<18}" + R
        code   = f"\033[2m  \\033[38;2;{rv};{gv};{bv}m\033[0m"
        emit(swatch + label + code + "\n", 0.07)

    emit("\n", 0.1)


# ─── 7. VT-100 / Control Sequences ───────────────────────────────────────────

def test_vt100():
    section("VT-100 Control Sequences")

    emit("  Cursor up — \\033[1A:\n")
    emit("  Line A\n  Line B\n")
    time.sleep(0.15)
    emit("\033[1A")
    emit("  Line B  \033[32m← cursor moved up one line\033[0m\n")
    time.sleep(0.3)

    emit("\n  Cursor back / overwrite — \\033[3D:\n")
    emit("  ABCDEF")
    time.sleep(0.2)
    emit("\033[3D")
    emit("\033[31mXYZ\033[0m\n")
    time.sleep(0.3)

    emit("\n  Erase to end of line — \\033[K:\n")
    emit("  Original text here")
    time.sleep(0.2)
    emit("\033[10D\033[K")
    emit("\033[32mreplaced\033[0m\n")
    time.sleep(0.3)

    emit("\n  Combined attributes:\n")
    combos = [
        ("\033[1m",      "Bold"),
        ("\033[2m",      "Dim"),
        ("\033[3m",      "Italic"),
        ("\033[4m",      "Underline"),
        ("\033[7m",      "Reverse"),
        ("\033[9m",      "Strikethrough"),
        ("\033[1;31m",   "Bold Red"),
        ("\033[1;32m",   "Bold Green"),
        ("\033[4;34m",   "Underline Blue"),
        ("\033[1;7;33m", "Bold + Reverse + Yellow"),
    ]
    for code, name in combos:
        emit(f"    {code}{name:<28}\033[0m  {repr(code)}\n", 0.07)

    emit("\n", 0.1)


# ─── 8. 2D Truecolor Rendering ────────────────────────────────────────────────

def test_2d_render():
    section("2D Truecolor Rendering  (plasma / sine field)")

    emit("  Each cell is an independent 24-bit RGB value.\n"
         "  Banding or posterization indicates limited color support.\n\n")

    rows, cols = 14, 64
    for row in range(rows):
        line = "  "
        for col in range(cols):
            v1 = math.sin(col / 5.0)
            v2 = math.sin(row / 3.0)
            v3 = math.sin((col + row) / 8.0)
            v4 = math.sin(math.sqrt(col * col + row * row) / 4.5)
            v = (v1 + v2 + v3 + v4) / 4.0  # −1 … +1
            hue = (v + 1) / 2 * 360
            col_rgb = hsv_to_rgb(hue, 1.0, 1.0)
            line += bg(*col_rgb) + " "
        emit(line + R + "\n", 0.05)

    emit("\n", 0.2)


# ─── Summary ──────────────────────────────────────────────────────────────────

def print_summary():
    section("Summary")

    tests = [
        ("Text attributes",          "bold, dim, italic, underline, blink, reverse, strikethrough"),
        ("4-bit colors",             "16-color foreground and background, FG×BG matrix"),
        ("8-bit colors",             "256-color system palette, RGB cube, grayscale ramp"),
        ("24-bit truecolor",         "channel ramps, gradients, hue/saturation/value sweeps"),
        ("Named truecolor samples",  "18 standard CSS/X11 color references"),
        ("VT-100 sequences",         "cursor movement, overwrite, erase, combined attributes"),
        ("2D truecolor rendering",   "sine-field plasma — detects banding or palette reduction"),
    ]

    for name, detail in tests:
        emit(f"  \033[32m✓\033[0m  {name}\n     \033[2m{detail}\033[0m\n\n", 0.06)

    emit("  All tests complete.\n\n", 0.1)


# ─── Full Run ──────────────────────────────────────────────────────────────────

def run_all():
    print("\n" + "─" * 64)
    print("  Serial Studio — ANSI Color Test")
    print(f"  Sending to {HOST}:{PORT} via UDP")
    print("─" * 64 + "\n")

    test_attributes()
    time.sleep(0.2)

    test_4bit()
    time.sleep(0.2)

    test_color_matrix()
    time.sleep(0.2)

    test_256_colors()
    time.sleep(0.2)

    test_truecolor()
    time.sleep(0.2)

    test_named_colors()
    time.sleep(0.2)

    test_vt100()
    time.sleep(0.2)

    test_2d_render()
    time.sleep(0.2)

    print_summary()

    print("─" * 64)
    print("  Done. Check the Serial Studio terminal view.")
    print("─" * 64 + "\n")


# ─── Entry Point ──────────────────────────────────────────────────────────────

if __name__ == "__main__":
    try:
        if len(sys.argv) > 1:
            opt = sys.argv[1]
            dispatch = {
                "--attributes": test_attributes,
                "--4bit":       test_4bit,
                "--matrix":     test_color_matrix,
                "--256":        test_256_colors,
                "--truecolor":  test_truecolor,
                "--named":      test_named_colors,
                "--vt100":      test_vt100,
                "--plasma":     test_2d_render,
            }
            if opt in ("--help", "-h"):
                print("\nSerial Studio — ANSI Color Test")
                print("─" * 64)
                print("\nUsage: python test_ansi_colors.py [option]")
                print("\nOptions:")
                print("  (no args)      Run all tests")
                print("  --attributes   Text attributes (bold, italic, underline, …)")
                print("  --4bit         16-color foreground, background, FG×BG matrix")
                print("  --matrix       4-bit FG × BG color matrix only")
                print("  --256          256-color (8-bit) palette")
                print("  --truecolor    24-bit truecolor ramps, gradients, HSV sweeps")
                print("  --named        Named truecolor samples (CSS/X11 reference)")
                print("  --vt100        VT-100 cursor, erase, and attribute sequences")
                print("  --plasma       2D truecolor rendering test (sine-field plasma)")
                print("  --help         Show this help")
                print(f"\nAll output sent via UDP to {HOST}:{PORT}\n")
            elif opt in dispatch:
                dispatch[opt]()
            else:
                print(f"Unknown option: {opt}. Use --help.")
                sys.exit(1)
        else:
            run_all()
    except KeyboardInterrupt:
        print("\n\nInterrupted.\n")
        sys.exit(0)
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)
