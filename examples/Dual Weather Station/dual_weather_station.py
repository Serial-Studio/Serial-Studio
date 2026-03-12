#!/usr/bin/env python3
"""
Dual Weather Station Simulator for Serial Studio

Simulates two weather stations transmitting sensor data over UDP:

  Station A — Outdoor (port 9001):
    Temperature, Humidity, Wind Speed, Wind Direction,
    Barometric Pressure, UV Index
    Frame format:  /*val1,val2,...,valN*/

  Station B — Indoor (port 9002):
    Temperature, Humidity, CO2, PM2.5, VOC Index, Dew Point
    Frame format:  <%val1,val2,...,valN%>

Open "Dual Weather Station.ssproj" in Serial Studio and connect both
UDP sources before running this script.

Usage:
  python3 dual_weather_station.py
  python3 dual_weather_station.py --rate 30
  python3 dual_weather_station.py --host 192.168.1.100 --rate 60
"""

import argparse
import math
import random
import socket
import sys
import threading
import time

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

DEFAULT_HOST = "127.0.0.1"
DEFAULT_RATE = 60

PORT_OUTDOOR = 9001
PORT_INDOOR = 9002

# Simulated "time of day" advances independently of wall-clock time so that
# the 24-hour cycle completes in roughly 4 minutes of simulation time.
SIM_DAY_SCALE = 360.0  # 1 real second = 360 simulated seconds (1/360 of a day)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def lerp(a, b, t):
    return a + (b - a) * t


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def noise(amp=0.1):
    return random.gauss(0, amp)


def hour_of_day(t):
    """Return a simulated hour in [0, 24) from elapsed real seconds."""
    simulated_seconds = t * SIM_DAY_SCALE
    return (simulated_seconds / 3600.0) % 24.0


def solar_elevation(t):
    """
    Return a [0, 1] value representing solar elevation above the horizon.
    Peaks at solar noon (12:00), zero at night.
    """
    h = hour_of_day(t)
    angle = math.pi * (h - 6.0) / 12.0
    return clamp(math.sin(angle), 0.0, 1.0)


def dew_point(temp_c, rh_pct):
    """Magnus formula approximation for dew point."""
    a = 17.625
    b = 243.04
    alpha = math.log(rh_pct / 100.0 + 1e-9) + a * temp_c / (b + temp_c)
    return b * alpha / (a - alpha)


# ---------------------------------------------------------------------------
# Outdoor Station Simulator
# ---------------------------------------------------------------------------


class OutdoorStation:
    """Simulates an outdoor weather station with realistic meteorology."""

    def __init__(self):
        self._t = 0.0
        self._wind_dir = random.uniform(0, 360)
        self._pressure = random.uniform(1005, 1015)
        self._wind_speed = random.uniform(5, 15)

    def step(self, dt):
        self._t += dt
        t = self._t

        # Temperature: sinusoidal 24-hour cycle + slow random walk + noise
        # Peaks near 14:00, troughs near 04:00.
        h = hour_of_day(t)
        temp_cycle = math.sin(math.pi * (h - 4.0) / 12.0)
        temp_base = 25.0 + 10.0 * temp_cycle
        temperature = clamp(temp_base + noise(0.3), 15.0, 35.0)

        # Humidity: inversely correlated with temperature
        humidity_base = lerp(85.0, 35.0, (temperature - 15.0) / 20.0)
        humidity = clamp(humidity_base + noise(1.5), 10.0, 100.0)

        # Wind speed: Rayleigh-like distribution with gustiness
        mean_speed = 15.0 + 10.0 * solar_elevation(t)
        gust = abs(noise(6.0))
        self._wind_speed = clamp(
            self._wind_speed * 0.92 + mean_speed * 0.08 + gust, 0.0, 80.0
        )

        # Wind direction: slow random walk
        self._wind_dir += noise(1.2)
        self._wind_dir %= 360.0

        # Pressure: slow drift between 990 and 1025 hPa
        pressure_target = 1010.0 + 15.0 * math.sin(t / 600.0)
        self._pressure += (pressure_target - self._pressure) * 0.001 + noise(0.05)
        self._pressure = clamp(self._pressure, 990.0, 1025.0)

        # UV index: sinusoidal with time of day, 0 at night
        uv = clamp(11.0 * solar_elevation(t) + noise(0.2), 0.0, 11.0)

        return (
            round(temperature, 2),
            round(humidity, 2),
            round(self._wind_speed, 2),
            round(self._wind_dir, 2),
            round(self._pressure, 2),
            round(uv, 2),
        )


# ---------------------------------------------------------------------------
# Indoor Station Simulator
# ---------------------------------------------------------------------------


class IndoorStation:
    """Simulates an indoor air-quality station."""

    def __init__(self):
        self._t = 0.0
        self._co2 = 900.0
        self._pm25 = 8.0
        self._voc = 80.0
        self._indoor_temp = 22.0

    def step(self, dt, outdoor_temp):
        self._t += dt
        t = self._t

        # Indoor temperature: more stable, follows outdoor with thermal lag
        target_temp = 22.0 + (outdoor_temp - 22.0) * 0.15
        self._indoor_temp += (target_temp - self._indoor_temp) * 0.002 + noise(0.02)
        temperature = clamp(self._indoor_temp, 20.0, 26.0)

        # Humidity: moderately controlled
        h = hour_of_day(t)
        rh_base = 45.0 + 8.0 * math.sin(math.pi * h / 12.0)
        humidity = clamp(rh_base + noise(1.0), 30.0, 60.0)

        # CO2: rises during "occupied hours" (08:00–22:00)
        h_cur = hour_of_day(t)
        occupied = 1.0 if 8.0 <= h_cur <= 22.0 else 0.0
        co2_target = lerp(600.0, 2000.0, occupied * solar_elevation(t))
        self._co2 += (co2_target - self._co2) * 0.003 + noise(5.0)
        co2 = clamp(self._co2, 400.0, 2000.0)

        # PM2.5: generally low with occasional spikes
        spike = abs(noise(1.0)) * 50.0 if random.random() < 0.005 else 0.0
        self._pm25 = clamp(self._pm25 * 0.98 + noise(0.3) + spike, 0.0, 50.0)

        # VOC index: correlated loosely with CO2 / occupancy
        voc_target = lerp(50.0, 400.0, (co2 - 400.0) / 1600.0)
        self._voc += (voc_target - self._voc) * 0.005 + noise(3.0)
        voc = clamp(self._voc, 0.0, 500.0)

        # Dew point derived from indoor conditions
        dp = dew_point(temperature, humidity)

        return (
            round(temperature, 2),
            round(humidity, 2),
            round(co2, 2),
            round(self._pm25, 2),
            round(voc, 2),
            round(dp, 2),
        )


# ---------------------------------------------------------------------------
# Frame formatting and UDP transmission
# ---------------------------------------------------------------------------


def format_outdoor_frame(values):
    """Build an outdoor station CSV frame: /*val1,...,valN*/"""
    fields = ",".join(f"{v:.2f}" for v in values)
    return f"/*{fields}*/"


def format_indoor_frame(values):
    """Build an indoor station CSV frame: <%val1,...,valN%>"""
    fields = ",".join(f"{v:.2f}" for v in values)
    return f"<%{fields}%>"


def send_udp(sock, host, port, data):
    """Send a UTF-8 encoded frame over UDP."""
    sock.sendto(data.encode("utf-8") + b"\n", (host, port))


# ---------------------------------------------------------------------------
# Station threads
# ---------------------------------------------------------------------------


# Shared state for the live display line (written by both threads)
_display_lock = threading.Lock()
_display = {"outdoor": "", "indoor": ""}


def _refresh_display():
    """Overwrite the current terminal line with the latest frame data."""
    with _display_lock:
        line = f"  OUT: {_display['outdoor']}   IN: {_display['indoor']}"

    # Pad to 120 chars so stale characters from a longer previous line are erased
    sys.stdout.write(f"\r{line:<120}")
    sys.stdout.flush()


def run_outdoor(host, rate, stop_event):
    """Thread target: continuously sends outdoor station frames."""
    dt = 1.0 / rate
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    station = OutdoorStation()

    print(f"[Outdoor] Sending to {host}:{PORT_OUTDOOR} at {rate} Hz")

    try:
        while not stop_event.is_set():
            t_start = time.monotonic()

            values = station.step(dt)
            frame = format_outdoor_frame(values)
            send_udp(sock, host, PORT_OUTDOOR, frame)

            with _display_lock:
                _display["outdoor"] = frame

            _refresh_display()

            elapsed = time.monotonic() - t_start
            sleep_time = dt - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)

    finally:
        sock.close()


def run_indoor(host, rate, stop_event):
    """Thread target: continuously sends indoor station frames."""
    dt = 1.0 / rate
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    outdoor = OutdoorStation()
    station = IndoorStation()

    print(f"[Indoor]  Sending to {host}:{PORT_INDOOR} at {rate} Hz")

    try:
        while not stop_event.is_set():
            t_start = time.monotonic()

            outdoor_values = outdoor.step(dt)
            outdoor_temp = outdoor_values[0]

            values = station.step(dt, outdoor_temp)
            frame = format_indoor_frame(values)
            send_udp(sock, host, PORT_INDOOR, frame)

            with _display_lock:
                _display["indoor"] = frame

            _refresh_display()

            elapsed = time.monotonic() - t_start
            sleep_time = dt - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)

    finally:
        sock.close()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(
        description="Dual Weather Station Simulator for Serial Studio",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--rate",
        type=float,
        default=DEFAULT_RATE,
        metavar="HZ",
        help=f"Update rate in Hz (default: {DEFAULT_RATE} Hz)",
    )
    parser.add_argument(
        "--host",
        default=DEFAULT_HOST,
        metavar="HOST",
        help=f"UDP destination host (default: {DEFAULT_HOST})",
    )
    args = parser.parse_args()

    if args.rate <= 0:
        print("Error: --rate must be a positive number.", file=sys.stderr)
        sys.exit(1)

    print("Dual Weather Station Simulator")
    print(f"Rate: {args.rate} Hz  |  Host: {args.host}")
    print("Press Ctrl+C to stop.\n")

    stop_event = threading.Event()

    t_outdoor = threading.Thread(
        target=run_outdoor,
        args=(args.host, args.rate, stop_event),
        daemon=True,
        name="outdoor",
    )
    t_indoor = threading.Thread(
        target=run_indoor,
        args=(args.host, args.rate, stop_event),
        daemon=True,
        name="indoor",
    )

    t_outdoor.start()
    t_indoor.start()

    try:
        while t_outdoor.is_alive() and t_indoor.is_alive():
            time.sleep(0.1)

    except KeyboardInterrupt:
        stop_event.set()
        print("\nStopping simulation...")

    t_outdoor.join(timeout=2.0)
    t_indoor.join(timeout=2.0)
    print("Done.")


if __name__ == "__main__":
    main()
