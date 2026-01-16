#!/usr/bin/env python3
"""
ANSI Color and VT-100 Test Script for Serial Studio
Sends test sequences via UDP to localhost:9000
"""

import socket
import time
import sys


def send_udp(message, host='127.0.0.1', port=9000):
    """Send a message via UDP to the specified host and port."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(message.encode('utf-8'), (host, port))
    finally:
        sock.close()


def test_4bit_foreground_colors():
    """Test standard 4-bit foreground colors (30-37)."""
    print("Testing 4-bit foreground colors...")
    colors = [
        (30, "Black"),
        (31, "Red"),
        (32, "Green"),
        (33, "Yellow"),
        (34, "Blue"),
        (35, "Magenta"),
        (36, "Cyan"),
        (37, "White"),
    ]

    for code, name in colors:
        msg = f"\033[{code}m{name:10}\033[0m (\\033[{code}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_4bit_bright_foreground_colors():
    """Test bright 4-bit foreground colors (90-97)."""
    print("Testing bright 4-bit foreground colors...")
    colors = [
        (90, "Bright Black"),
        (91, "Bright Red"),
        (92, "Bright Green"),
        (93, "Bright Yellow"),
        (94, "Bright Blue"),
        (95, "Bright Magenta"),
        (96, "Bright Cyan"),
        (97, "Bright White"),
    ]

    for code, name in colors:
        msg = f"\033[{code}m{name:15}\033[0m (\\033[{code}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_4bit_background_colors():
    """Test standard 4-bit background colors (40-47)."""
    print("Testing 4-bit background colors...")
    colors = [
        (40, "Black BG"),
        (41, "Red BG"),
        (42, "Green BG"),
        (43, "Yellow BG"),
        (44, "Blue BG"),
        (45, "Magenta BG"),
        (46, "Cyan BG"),
        (47, "White BG"),
    ]

    for code, name in colors:
        msg = f"\033[{code}m{name:12}\033[0m (\\033[{code}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_4bit_bright_background_colors():
    """Test bright 4-bit background colors (100-107)."""
    print("Testing bright 4-bit background colors...")
    colors = [
        (100, "Bright Black BG"),
        (101, "Bright Red BG"),
        (102, "Bright Green BG"),
        (103, "Bright Yellow BG"),
        (104, "Bright Blue BG"),
        (105, "Bright Magenta BG"),
        (106, "Bright Cyan BG"),
        (107, "Bright White BG"),
    ]

    for code, name in colors:
        msg = f"\033[{code}m{name:18}\033[0m (\\033[{code}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_combined_fg_bg():
    """Test combined foreground and background colors."""
    print("Testing combined foreground and background...")
    combinations = [
        (31, 47, "Red on White"),
        (32, 40, "Green on Black"),
        (33, 44, "Yellow on Blue"),
        (36, 41, "Cyan on Red"),
        (91, 104, "Bright Red on Bright Blue"),
        (92, 105, "Bright Green on Bright Magenta"),
    ]

    for fg, bg, name in combinations:
        msg = f"\033[{fg};{bg}m{name:30}\033[0m (\\033[{fg};{bg}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_8bit_foreground_samples():
    """Test 8-bit 256-color palette foreground (38;5;N)."""
    print("Testing 8-bit foreground colors (samples)...")

    # Standard colors (0-15)
    send_udp("=== Standard Colors (0-15) ===\n")
    time.sleep(0.1)
    for i in range(16):
        msg = f"\033[38;5;{i}m Color {i:3d} \033[0m"
        if (i + 1) % 8 == 0:
            msg += "\n"
        send_udp(msg)
        time.sleep(0.05)

    # RGB cube samples (16-231)
    send_udp("\n=== RGB Cube Samples (16-231) ===\n")
    time.sleep(0.1)
    samples = [16, 21, 27, 51, 87, 123, 159, 195, 201, 207, 213, 219, 225, 231]
    for i in samples:
        msg = f"\033[38;5;{i}m Color {i:3d} \033[0m"
        send_udp(msg)
        time.sleep(0.05)
    send_udp("\n")

    # Grayscale (232-255)
    send_udp("\n=== Grayscale (232-255) ===\n")
    time.sleep(0.1)
    for i in range(232, 256):
        msg = f"\033[38;5;{i}m█\033[0m"
        if (i - 232 + 1) % 12 == 0:
            msg += "\n"
        send_udp(msg)
        time.sleep(0.05)


def test_8bit_background_samples():
    """Test 8-bit 256-color palette background (48;5;N)."""
    print("Testing 8-bit background colors (samples)...")

    send_udp("\n=== 8-bit Background Samples ===\n")
    time.sleep(0.1)

    samples = [16, 21, 27, 51, 87, 123, 159, 195, 201, 207, 213, 219, 225, 231]
    for i in samples:
        msg = f"\033[48;5;{i}m BG {i:3d} \033[0m "
        send_udp(msg)
        time.sleep(0.05)
    send_udp("\n")


def test_8bit_combined():
    """Test combined 8-bit foreground and background."""
    print("Testing combined 8-bit colors...")

    send_udp("\n=== Combined 8-bit FG/BG ===\n")
    time.sleep(0.1)

    combinations = [
        (206, 57, "Pink on Blue"),
        (226, 18, "Yellow on Dark Blue"),
        (51, 226, "Cyan on Bright Yellow"),
        (196, 22, "Red on Dark Green"),
    ]

    for fg, bg, name in combinations:
        msg = f"\033[38;5;{fg};48;5;{bg}m {name:25} \033[0m (\\033[38;5;{fg};48;5;{bg}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_24bit_rgb_foreground():
    """Test 24-bit RGB foreground colors (38;2;R;G;B)."""
    print("Testing 24-bit RGB foreground colors...")

    send_udp("\n=== 24-bit RGB Foreground ===\n")
    time.sleep(0.1)

    colors = [
        (255, 82, 197, "Hot Pink"),
        (255, 165, 0, "Orange"),
        (0, 255, 127, "Spring Green"),
        (138, 43, 226, "Blue Violet"),
        (255, 20, 147, "Deep Pink"),
        (64, 224, 208, "Turquoise"),
    ]

    for r, g, b, name in colors:
        msg = f"\033[38;2;{r};{g};{b}m{name:15}\033[0m (\\033[38;2;{r};{g};{b}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_24bit_rgb_background():
    """Test 24-bit RGB background colors (48;2;R;G;B)."""
    print("Testing 24-bit RGB background colors...")

    send_udp("\n=== 24-bit RGB Background ===\n")
    time.sleep(0.1)

    colors = [
        (155, 106, 0, "Brown BG"),
        (70, 130, 180, "Steel Blue BG"),
        (60, 60, 60, "Dark Gray BG"),
        (25, 25, 112, "Midnight Blue BG"),
    ]

    for r, g, b, name in colors:
        msg = f"\033[48;2;{r};{g};{b}m {name:20} \033[0m (\\033[48;2;{r};{g};{b}m)\n"
        send_udp(msg)
        time.sleep(0.1)


def test_24bit_rgb_combined():
    """Test combined 24-bit RGB foreground and background."""
    print("Testing combined 24-bit RGB colors...")

    send_udp("\n=== Combined 24-bit RGB FG/BG ===\n")
    time.sleep(0.1)

    msg = "\033[38;2;255;82;197;48;2;155;106;0m Pink on Brown \033[0m\n"
    send_udp(msg)
    time.sleep(0.1)

    msg = "\033[38;2;255;255;255;48;2;0;0;128m White on Navy \033[0m\n"
    send_udp(msg)
    time.sleep(0.1)

    msg = "\033[38;2;0;255;0;48;2;0;0;0m Green on Black \033[0m\n"
    send_udp(msg)
    time.sleep(0.1)


def test_vt100_cursor_movement():
    """Test VT-100 cursor movement sequences."""
    print("Testing VT-100 cursor movement...")

    send_udp("\n=== VT-100 Cursor Movement ===\n")
    time.sleep(0.2)

    # Cursor up
    send_udp("Line 1\nLine 2\n")
    time.sleep(0.1)
    send_udp("\033[1A")  # Move up 1 line
    send_udp(" <- Moved up\n")
    time.sleep(0.2)

    # Cursor forward and backward
    send_udp("ABCDEF")
    time.sleep(0.1)
    send_udp("\033[3D")  # Move back 3
    send_udp("XXX")  # Overwrite
    send_udp("\n")
    time.sleep(0.2)


def test_vt100_clear():
    """Test VT-100 clear screen sequences."""
    print("Testing VT-100 clear sequences...")

    send_udp("\n=== VT-100 Clear Screen ===\n")
    time.sleep(0.2)

    send_udp("This text will be cleared...\n")
    time.sleep(0.5)
    send_udp("\033[2J")  # Clear screen
    send_udp("Screen cleared!\n")
    time.sleep(0.2)


def test_bold_and_styles():
    """Test bold and other text styles."""
    print("Testing bold and text styles...")

    send_udp("\n=== Text Styles ===\n")
    time.sleep(0.1)

    # Bold
    msg = "\033[1mBold Text\033[0m (\\033[1m)\n"
    send_udp(msg)
    time.sleep(0.1)

    # Bold + Color
    msg = "\033[1;31mBold Red\033[0m (\\033[1;31m)\n"
    send_udp(msg)
    time.sleep(0.1)

    msg = "\033[1;32mBold Green\033[0m (\\033[1;32m)\n"
    send_udp(msg)
    time.sleep(0.1)


def test_rainbow():
    """Create a rainbow effect using various colors."""
    print("Testing rainbow effect...")

    send_udp("\n=== Rainbow Effect ===\n")
    time.sleep(0.1)

    # Using 8-bit colors
    rainbow = [196, 208, 226, 46, 51, 21, 93, 129]
    text = "Serial Studio ANSI Color Support!"

    for i, char in enumerate(text):
        color = rainbow[i % len(rainbow)]
        msg = f"\033[38;5;{color}m{char}\033[0m"
        send_udp(msg)
        time.sleep(0.02)

    send_udp("\n")
    time.sleep(0.2)

    # Using RGB colors
    send_udp("RGB Rainbow: ")
    time.sleep(0.1)

    rgb_colors = [
        (255, 0, 0),    # Red
        (255, 127, 0),  # Orange
        (255, 255, 0),  # Yellow
        (0, 255, 0),    # Green
        (0, 0, 255),    # Blue
        (75, 0, 130),   # Indigo
        (148, 0, 211),  # Violet
    ]

    text = "RAINBOW"
    for i, char in enumerate(text):
        r, g, b = rgb_colors[i % len(rgb_colors)]
        msg = f"\033[38;2;{r};{g};{b}m{char}\033[0m"
        send_udp(msg)
        time.sleep(0.05)

    send_udp("\n\n")


def test_gradient():
    """Create a gradient using 256-color palette."""
    print("Testing gradient effect...")

    send_udp("=== Color Gradients ===\n")
    time.sleep(0.1)

    # Red to yellow gradient (background)
    send_udp("Red to Yellow: ")
    for i in range(196, 227, 2):
        send_udp(f"\033[48;5;{i}m \033[0m")
        time.sleep(0.03)
    send_udp("\n")
    time.sleep(0.2)

    # Blue gradient (background)
    send_udp("Blue gradient: ")
    for i in range(17, 27):
        send_udp(f"\033[48;5;{i}m \033[0m")
        time.sleep(0.03)
    send_udp("\n")
    time.sleep(0.2)

    # Grayscale gradient
    send_udp("Grayscale:     ")
    for i in range(232, 256):
        send_udp(f"\033[48;5;{i}m \033[0m")
        time.sleep(0.03)
    send_udp("\n\n")


def draw_16_color_table():
    """Display standard 16 ANSI colors (0-15) in a table format."""
    print("Drawing 16-color table...")

    send_udp("\n")
    send_udp("\033[1;37m╔══════════════════════════════════════════════════════════════════╗\033[0m\n")
    send_udp("\033[1;37m║              STANDARD 16 ANSI COLORS (0-15)                      ║\033[0m\n")
    send_udp("\033[1;37m╚══════════════════════════════════════════════════════════════════╝\033[0m\n\n")
    time.sleep(0.2)

    send_udp("  Normal Colors (0-7):\n  ")
    for i in range(8):
        send_udp(f"\033[48;5;{i}m  {i:1d}  \033[0m ")
    send_udp("\n\n")
    time.sleep(0.2)

    send_udp("  Bright Colors (8-15):\n  ")
    for i in range(8, 16):
        send_udp(f"\033[48;5;{i}m  {i:2d}  \033[0m ")
    send_udp("\n\n")
    time.sleep(0.2)


def draw_256_color_table():
    """Display full 256-color palette in organized blocks."""
    print("Drawing 256-color table...")

    send_udp("\n")
    send_udp("\033[1;37m╔══════════════════════════════════════════════════════════════════╗\033[0m\n")
    send_udp("\033[1;37m║                 256 COLOR PALETTE (8-BIT)                        ║\033[0m\n")
    send_udp("\033[1;37m╚══════════════════════════════════════════════════════════════════╝\033[0m\n\n")
    time.sleep(0.2)

    send_udp("  6x6x6 RGB Color Cube (16-231):\n")
    time.sleep(0.1)

    for row in range(6):
        send_udp("  ")
        for col in range(36):
            color_idx = 16 + (row * 36) + col
            send_udp(f"\033[48;5;{color_idx}m  \033[0m")
            if (col + 1) % 18 == 0:
                send_udp("  ")
        send_udp("\n")
        time.sleep(0.05)

    send_udp("\n  Grayscale Ramp (232-255):\n  ")
    time.sleep(0.1)

    for i in range(232, 256):
        send_udp(f"\033[48;5;{i}m  \033[0m")
        if (i - 231) % 12 == 0:
            send_udp("\n  ")
    send_udp("\n\n")
    time.sleep(0.2)


def draw_color_boxes():
    """Draw simple colored boxes with labels."""
    print("Drawing colored boxes...")

    send_udp("\n")
    send_udp("\033[1;37m╔══════════════════════════════════════════════════════════════════╗\033[0m\n")
    send_udp("\033[1;37m║                     COLOR BOX DISPLAY                            ║\033[0m\n")
    send_udp("\033[1;37m╚══════════════════════════════════════════════════════════════════╝\033[0m\n\n")
    time.sleep(0.2)

    colors = [
        (196, "Red"),
        (208, "Orange"),
        (226, "Yellow"),
        (46, "Green"),
        (51, "Cyan"),
        (21, "Blue"),
        (201, "Magenta"),
        (231, "White"),
    ]

    for color_code, name in colors:
        send_udp(f"  {name:10} ")
        send_udp(f"\033[48;5;{color_code}m" + "          " + "\033[0m")
        send_udp(f"  (Color {color_code})\n")
        time.sleep(0.1)

    send_udp("\n")
    time.sleep(0.2)


def draw_rgb_gradients():
    """Draw RGB color gradients using 24-bit colors."""
    print("Drawing RGB gradients...")

    send_udp("\n")
    send_udp("\033[1;37m╔══════════════════════════════════════════════════════════════════╗\033[0m\n")
    send_udp("\033[1;37m║                   RGB GRADIENTS (24-BIT)                         ║\033[0m\n")
    send_udp("\033[1;37m╚══════════════════════════════════════════════════════════════════╝\033[0m\n\n")
    time.sleep(0.2)

    send_udp("  Red Gradient:\n  ")
    for i in range(32):
        r = int(255 * (i / 31))
        send_udp(f"\033[48;2;{r};0;0m  \033[0m")
    send_udp("\n\n")
    time.sleep(0.1)

    send_udp("  Green Gradient:\n  ")
    for i in range(32):
        g = int(255 * (i / 31))
        send_udp(f"\033[48;2;0;{g};0m  \033[0m")
    send_udp("\n\n")
    time.sleep(0.1)

    send_udp("  Blue Gradient:\n  ")
    for i in range(32):
        b = int(255 * (i / 31))
        send_udp(f"\033[48;2;0;0;{b}m  \033[0m")
    send_udp("\n\n")
    time.sleep(0.1)

    send_udp("  Cyan-Magenta Gradient:\n  ")
    for i in range(32):
        r = int(255 * (i / 31))
        b = 255 - r
        send_udp(f"\033[48;2;{r};0;{b}m  \033[0m")
    send_udp("\n\n")
    time.sleep(0.1)

    send_udp("  Grayscale Gradient:\n  ")
    for i in range(32):
        gray = int(255 * (i / 31))
        send_udp(f"\033[48;2;{gray};{gray};{gray}m  \033[0m")
    send_udp("\n\n")
    time.sleep(0.2)


def run_all_tests():
    """Run all color and VT-100 tests."""
    print("\n" + "="*60)
    print("Serial Studio ANSI Color & VT-100 Test Suite")
    print("Sending test sequences to localhost:9000 via UDP")
    print("="*60 + "\n")

    send_udp("\n\n" + "="*60 + "\n")
    send_udp("Serial Studio ANSI Color & VT-100 Test Suite\n")
    send_udp("="*60 + "\n\n")
    time.sleep(0.5)

    # Color palette displays
    draw_16_color_table()
    time.sleep(0.5)

    draw_256_color_table()
    time.sleep(0.5)

    draw_color_boxes()
    time.sleep(0.5)

    draw_rgb_gradients()
    time.sleep(0.5)

    # 4-bit color tests
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          4-BIT COLOR TESTS (Standard ANSI)                 ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n")
    time.sleep(0.3)

    test_4bit_foreground_colors()
    time.sleep(0.2)

    test_4bit_bright_foreground_colors()
    time.sleep(0.2)

    test_4bit_background_colors()
    time.sleep(0.2)

    test_4bit_bright_background_colors()
    time.sleep(0.2)

    test_combined_fg_bg()
    time.sleep(0.5)

    # 8-bit color tests
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          8-BIT COLOR TESTS (256 Colors)                    ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n")
    time.sleep(0.3)

    test_8bit_foreground_samples()
    time.sleep(0.2)

    test_8bit_background_samples()
    time.sleep(0.2)

    test_8bit_combined()
    time.sleep(0.5)

    # 24-bit RGB color tests
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          24-BIT RGB COLOR TESTS (True Color)               ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n")
    time.sleep(0.3)

    test_24bit_rgb_foreground()
    time.sleep(0.2)

    test_24bit_rgb_background()
    time.sleep(0.2)

    test_24bit_rgb_combined()
    time.sleep(0.5)

    # VT-100 tests
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          VT-100 CONTROL SEQUENCES                          ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n")
    time.sleep(0.3)

    test_vt100_cursor_movement()
    time.sleep(0.2)

    test_bold_and_styles()
    time.sleep(0.5)

    # Creative tests
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          CREATIVE COLOR EFFECTS                            ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n")
    time.sleep(0.3)

    test_rainbow()
    time.sleep(0.2)

    test_gradient()
    time.sleep(0.5)

    # Test complete
    send_udp("\n╔════════════════════════════════════════════════════════════╗\n")
    send_udp("║          ALL TESTS COMPLETE!                               ║\n")
    send_udp("╚════════════════════════════════════════════════════════════╝\n\n")

    print("\n" + "="*60)
    print("All tests completed successfully!")
    print("Check Serial Studio console for results.")
    print("="*60 + "\n")


if __name__ == "__main__":
    try:
        if len(sys.argv) > 1:
            option = sys.argv[1]
            if option == "--16colors":
                draw_16_color_table()
            elif option == "--256colors":
                draw_256_color_table()
            elif option == "--boxes":
                draw_color_boxes()
            elif option == "--gradients":
                draw_rgb_gradients()
            elif option == "--help" or option == "-h":
                print("\nSerial Studio ANSI Color Test Script")
                print("=" * 60)
                print("\nUsage: python test_ansi_colors.py [option]")
                print("\nOptions:")
                print("  (no args)     Run full test suite with all color tests")
                print("  --16colors    Display 16-color ANSI palette")
                print("  --256colors   Display 256-color palette")
                print("  --boxes       Display simple color boxes")
                print("  --gradients   Display RGB gradients")
                print("  --help        Show this help message")
                print("\nAll output is sent via UDP to localhost:9000")
                print("Make sure Serial Studio is running and listening on that port.\n")
            else:
                print(f"Unknown option: {option}")
                print("Use --help to see available options")
                sys.exit(1)
        else:
            run_all_tests()
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user.")
        sys.exit(0)
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)
