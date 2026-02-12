#!/usr/bin/env python3
"""
RC Plane Telemetry Simulator for Serial Studio

Simulates a small RC plane flying a full flight profile with realistic sensor
data. Designed to exercise Serial Studio widgets including gyroscope,
GPS, compass, gauges, and bar widgets.

Sends CSV frames via UDP to localhost:9000 in the format:
  $gx,gy,gz,ax,ay,az,lat,lon,alt,hdg,aspd,thr,batt,rssi,temp;\n

Usage:
  python3 rc_plane_simulator.py              # Run full flight (loops)
  python3 rc_plane_simulator.py --once       # Run single flight
  python3 rc_plane_simulator.py --fast       # 50 Hz update rate
  python3 rc_plane_simulator.py --port 8000  # Custom UDP port
"""

import math
import random
import socket
import sys
import time

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

UDP_HOST = "127.0.0.1"
UDP_PORT = 9000
FRAME_RATE = 20

# Base GPS position (open field)
BASE_LAT = 37.4135
BASE_LON = -122.1530
GROUND_ALT = 5.0

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def lerp(a, b, t):
    return a + (b - a) * t


def smoothstep(t):
    t = max(0.0, min(1.0, t))
    return t * t * (3.0 - 2.0 * t)


def ease_in_out(t):
    return smoothstep(t)


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def noise(amp=0.1):
    return random.gauss(0, amp)


def wrap_heading(h):
    return h % 360.0


def wrap_angle(a):
    while a > 180:
        a -= 360
    while a < -180:
        a += 360
    return a


def meters_to_lat(m):
    return m / 111320.0


def meters_to_lon(m, lat_deg):
    return m / (111320.0 * math.cos(math.radians(lat_deg)))


# ---------------------------------------------------------------------------
# Flight Simulator
# ---------------------------------------------------------------------------


class RCPlaneSimulator:
    """Simulates an RC plane's full flight profile."""

    PHASES = [
        ("preflight", 4.0),
        ("engine_start", 3.0),
        ("taxi", 5.0),
        ("takeoff_roll", 4.0),
        ("liftoff", 4.0),
        ("climb", 10.0),
        ("level_flight", 8.0),
        ("gentle_turns", 14.0),
        ("steep_turns", 10.0),
        ("figure_eight", 18.0),
        ("loop", 8.0),
        ("aileron_roll", 4.0),
        ("knife_edge", 5.0),
        ("inverted", 5.0),
        ("dive_pullup", 8.0),
        ("zero_g_push", 6.0),
        ("recovery", 6.0),
        ("cruise_home", 12.0),
        ("approach", 10.0),
        ("flare", 4.0),
        ("rollout", 5.0),
        ("shutdown", 4.0),
    ]

    def __init__(self, frame_rate=FRAME_RATE):
        self.dt = 1.0 / frame_rate
        self.t = 0.0

        # Build phase time table
        self.phase_starts = []
        acc = 0.0
        for _, dur in self.PHASES:
            self.phase_starts.append(acc)
            acc += dur
        self.total_duration = acc

        self._reset_state()

    def _reset_state(self):
        self.heading = 0.0
        self.pos_n = 0.0
        self.pos_e = 0.0
        self.alt = GROUND_ALT
        self.speed = 0.0
        self.battery = 12.60
        self.motor_temp = 22.0
        self.pitch = 0.0
        self.roll = 0.0

    def get_phase(self):
        for i, (name, dur) in enumerate(self.PHASES):
            start = self.phase_starts[i]
            if self.t < start + dur:
                p = (self.t - start) / dur
                return name, p
        return "shutdown", 1.0

    def step(self):
        phase, p = self.get_phase()
        sensors = self._compute(phase, p)
        self.t += self.dt
        return sensors

    def is_done(self):
        return self.t >= self.total_duration

    def reset(self):
        self.t = 0.0
        self._reset_state()

    # -------------------------------------------------------------------
    # Sensor computation per phase
    # -------------------------------------------------------------------

    def _compute(self, phase, p):
        gx, gy, gz = 0.0, 0.0, 0.0
        throttle = 0.0
        vib = 0.0
        load_factor = 1.0
        fwd_accel = 0.0

        # ---- Pre-flight: everything off ----
        if phase == "preflight":
            throttle = 0.0

        # ---- Engine start: vibration ramps up ----
        elif phase == "engine_start":
            throttle = lerp(0, 8, ease_in_out(p))
            vib = lerp(0.0, 0.4, p)

        # ---- Taxi: slow roll on ground ----
        elif phase == "taxi":
            throttle = 12.0
            self.speed = lerp(0, 2.5, ease_in_out(p))
            self.heading = wrap_heading(lerp(0, 0, p))
            vib = 0.3

        # ---- Takeoff roll: full throttle, accelerating ----
        elif phase == "takeoff_roll":
            throttle = lerp(50, 100, ease_in_out(p))
            self.speed = lerp(2.5, 18, ease_in_out(p))
            fwd_accel = lerp(2.0, 5.0, ease_in_out(p))
            vib = 0.4

        # ---- Liftoff: nose-up, climbing ----
        elif phase == "liftoff":
            throttle = 95.0
            self.speed = lerp(18, 22, p)
            self.pitch = lerp(0, 15, ease_in_out(p))
            self.alt = lerp(GROUND_ALT, 30, ease_in_out(p))
            gx = lerp(5, 1, p)
            fwd_accel = lerp(3.0, 1.0, p)
            vib = 0.25

        # ---- Climb to pattern altitude ----
        elif phase == "climb":
            throttle = 80.0
            self.speed = lerp(22, 20, p)
            self.pitch = lerp(15, 8, ease_in_out(p))
            self.alt = lerp(30, 120, ease_in_out(p))
            self.heading = wrap_heading(lerp(0, 270, ease_in_out(p)))
            gx = lerp(2, 0.5, p)
            gz = lerp(0, -8, ease_in_out(p))
            fwd_accel = lerp(1.5, 0.0, p)
            vib = 0.15

        # ---- Straight and level ----
        elif phase == "level_flight":
            throttle = 55.0
            self.speed = 18.0
            self.pitch = lerp(8, 0, ease_in_out(p))
            self.alt = 120.0
            self.heading = 270.0
            vib = 0.08

        # ---- Gentle banked turns (left orbit) ----
        elif phase == "gentle_turns":
            throttle = 58.0
            self.speed = 18.0
            self.alt = 120.0
            bank = 25.0
            turn_rate = -15.0
            self.heading = wrap_heading(270 + turn_rate * p * 14)
            self.roll = -bank * math.sin(2 * math.pi * p)

            gz = turn_rate * math.cos(2 * math.pi * p)
            gy = bank * 2 * math.pi / 14 * math.cos(2 * math.pi * p)
            vib = 0.10

        # ---- Steep 60-degree banked turns ----
        elif phase == "steep_turns":
            throttle = 72.0
            self.speed = 20.0
            self.alt = lerp(120, 115, p)
            bank = 55.0
            osc = math.sin(2 * math.pi * p * 1.5)
            self.roll = bank * osc
            self.heading = wrap_heading(
                self.heading + (-25 if osc > 0 else 25) * self.dt
            )

            gy = bank * 1.5 * 2 * math.pi / 10 * math.cos(
                2 * math.pi * p * 1.5
            )
            gz = -25 * osc
            load_factor = 1.0 / max(0.3, math.cos(math.radians(bank * abs(osc))))
            vib = 0.15

        # ---- Figure-eight: alternating steep banks ----
        elif phase == "figure_eight":
            throttle = 65.0
            self.speed = 19.0
            self.alt = 115.0
            bank = 45.0 * math.sin(2 * math.pi * p * 2)
            self.roll = bank
            self.heading = wrap_heading(
                self.heading + (-20 * math.sin(2 * math.pi * p * 2)) * self.dt
            )

            gx = noise(2)
            gy = 45 * 2 * 2 * math.pi / 18 * math.cos(2 * math.pi * p * 2)
            gz = -20 * math.sin(2 * math.pi * p * 2)
            load_factor = 1.0 / max(0.3, math.cos(math.radians(bank)))
            vib = 0.12

        # ---- Loop: full pitch loop ----
        elif phase == "loop":
            throttle = 90.0
            self.speed = lerp(22, 16, abs(p - 0.5) * 2)
            loop_angle = p * 360
            self.pitch = wrap_angle(loop_angle)

            gx = 360.0 / 8.0
            gz = noise(3)
            load_factor = lerp(2.5, 3.5, math.sin(math.pi * p))

            self.alt = 115 + 40 * math.sin(math.radians(loop_angle))
            vib = 0.20

        # ---- Aileron roll: 360-degree roll ----
        elif phase == "aileron_roll":
            throttle = 75.0
            self.speed = 20.0
            roll_angle = p * 360
            self.roll = wrap_angle(roll_angle)

            gy = 360.0 / 4.0
            gx = noise(5)
            gz = noise(5)

            self.alt = 115 + noise(2)
            vib = 0.18

        # ---- Knife-edge: sustained 90-degree bank ----
        elif phase == "knife_edge":
            throttle = 85.0
            self.speed = 21.0
            entry = ease_in_out(min(p * 4, 1.0))
            exit_f = ease_in_out(max((p - 0.75) * 4, 0.0))
            bank = 90 * entry * (1.0 - exit_f)
            self.roll = bank

            gy = (90 * 4 / 5) * (1 if p < 0.25 else (-1 if p > 0.75 else 0))
            gz = noise(3)

            self.alt = lerp(115, 110, p)
            vib = 0.20

        # ---- Inverted flight ----
        elif phase == "inverted":
            throttle = 70.0
            self.speed = 19.0
            entry = ease_in_out(min(p * 3, 1.0))
            if p > 0.7:
                bank = lerp(180, 0, ease_in_out((p - 0.7) / 0.3))
            else:
                bank = 180 * entry

            self.roll = wrap_angle(bank)

            gx = noise(2)
            gy = 180 / 5 * (1 if p < 0.15 else (-1 if p > 0.7 else 0))
            gz = noise(2)

            self.alt = lerp(110, 105, p)
            vib = 0.15

        # ---- Dive and aggressive pull-up ----
        elif phase == "dive_pullup":
            throttle = lerp(40, 95, p) if p < 0.5 else lerp(95, 60, p)
            if p < 0.5:
                dp = p * 2
                self.pitch = lerp(0, -40, ease_in_out(dp))
                self.speed = lerp(20, 35, dp)
                self.alt = lerp(105, 60, ease_in_out(dp))
                gx = -40 / 4
                load_factor = lerp(1.0, 0.3, ease_in_out(dp))
                fwd_accel = lerp(0, 4.0, dp)
            else:
                dp = (p - 0.5) * 2
                self.pitch = lerp(-40, 10, ease_in_out(dp))
                self.speed = lerp(35, 22, dp)
                self.alt = lerp(60, 100, ease_in_out(dp))
                gx = 50 / 4
                load_factor = lerp(3.5, 1.0, ease_in_out(dp))
                fwd_accel = lerp(-2.0, 0, dp)
            gy = noise(3)
            gz = noise(2)
            vib = 0.25

        # ---- Zero-G parabolic pushover ----
        elif phase == "zero_g_push":
            throttle = lerp(60, 20, ease_in_out(p))
            if p < 0.2:
                dp = p * 5
                self.pitch = lerp(10, 30, ease_in_out(dp))
                self.speed = lerp(22, 25, dp)
                gx = 20 / 1.2
                load_factor = lerp(1.0, 1.8, ease_in_out(dp))
            elif p < 0.75:
                dp = (p - 0.2) / 0.55
                self.pitch = lerp(30, -20, dp)
                self.speed = lerp(25, 20, dp)
                gx = -50 / 3.3
                load_factor = lerp(0.1, 0.0, dp)
            else:
                dp = (p - 0.75) * 4
                self.pitch = lerp(-20, 0, ease_in_out(dp))
                self.speed = lerp(20, 19, dp)
                gx = 20 / 1.5
                load_factor = lerp(0.0, 1.0, ease_in_out(dp))
            self.alt = 100 + 20 * math.sin(math.pi * p)
            gy = noise(1)
            gz = noise(1)
            vib = 0.05

        # ---- Recovery to normal flight ----
        elif phase == "recovery":
            throttle = lerp(50, 55, p)
            self.speed = lerp(19, 18, p)
            self.pitch = lerp(self.pitch, 0, ease_in_out(p))
            self.roll = lerp(self.roll, 0, ease_in_out(p))
            self.alt = lerp(self.alt, 110, ease_in_out(p))
            vib = 0.08

        # ---- Cruise back toward field ----
        elif phase == "cruise_home":
            throttle = 55.0
            self.speed = 18.0
            self.alt = 110.0

            target_hdg = math.degrees(
                math.atan2(-self.pos_e, -self.pos_n)
            ) % 360
            hdg_diff = wrap_angle(target_hdg - self.heading)
            turn = clamp(hdg_diff, -15, 15)
            self.heading = wrap_heading(self.heading + turn * self.dt)
            self.roll = turn * 1.5

            gz = turn
            vib = 0.08

        # ---- Approach: descending, slowing ----
        elif phase == "approach":
            throttle = lerp(55, 25, p)
            self.speed = lerp(18, 14, ease_in_out(p))
            self.alt = lerp(110, 15, ease_in_out(p))
            self.pitch = lerp(0, -5, ease_in_out(p))

            target_hdg = 0.0
            hdg_diff = wrap_angle(target_hdg - self.heading)
            turn = clamp(hdg_diff, -20, 20)
            self.heading = wrap_heading(self.heading + turn * self.dt)
            self.roll = turn * 1.0

            gx = lerp(0, -5 / 10, p)
            gz = turn * 0.5
            vib = 0.06

        # ---- Flare: nose up, touchdown ----
        elif phase == "flare":
            throttle = lerp(25, 0, ease_in_out(p))
            self.speed = lerp(14, 8, p)
            self.heading = wrap_heading(
                lerp(self.heading, 0, ease_in_out(p))
            )
            self.pitch = lerp(-5, 3, ease_in_out(p))
            self.alt = lerp(15, GROUND_ALT, ease_in_out(p))
            self.roll = lerp(self.roll, 0, ease_in_out(p))

            gx = 8 / 4
            vib = 0.10

            if p > 0.85:
                vib = 0.6

        # ---- Rollout: ground deceleration ----
        elif phase == "rollout":
            throttle = 0.0
            self.speed = lerp(8, 0, ease_in_out(p))
            self.alt = GROUND_ALT
            self.pitch = lerp(3, 0, p)
            self.roll = 0
            vib = lerp(0.4, 0.05, p)

        # ---- Shutdown ----
        elif phase == "shutdown":
            throttle = 0
            self.speed = 0
            self.alt = GROUND_ALT
            self.pitch = 0
            self.roll = 0
            vib = lerp(0.1, 0.0, p)

        # ---------------------------------------------------------------
        # Add vibration noise proportional to throttle + vib factor
        # ---------------------------------------------------------------
        vib_scale = vib + (throttle / 100.0) * 0.08
        gx += noise(vib_scale * 5)
        gy += noise(vib_scale * 5)
        gz += noise(vib_scale * 3)

        # ---------------------------------------------------------------
        # Compute body-frame accelerometer values (m/s²)
        # The accelerometer measures gravity + dynamic acceleration.
        # In level 1G flight: ax≈0, ay≈0, az≈9.81
        # ---------------------------------------------------------------
        G = 9.81
        pitch_rad = math.radians(self.pitch)
        roll_rad = math.radians(self.roll)

        ax = -G * math.sin(pitch_rad) + fwd_accel
        ay = G * math.cos(pitch_rad) * math.sin(roll_rad)
        az = G * math.cos(pitch_rad) * math.cos(roll_rad) * load_factor

        ax += noise(vib_scale * 1.5)
        ay += noise(vib_scale * 1.5)
        az += noise(vib_scale * 1.0)

        # ---------------------------------------------------------------
        # Integrate position from heading and speed
        # ---------------------------------------------------------------
        hdg_rad = math.radians(self.heading)
        self.pos_n += self.speed * math.cos(hdg_rad) * self.dt
        self.pos_e += self.speed * math.sin(hdg_rad) * self.dt

        # ---------------------------------------------------------------
        # Derived values
        # ---------------------------------------------------------------
        lat = BASE_LAT + meters_to_lat(self.pos_n)
        lon = BASE_LON + meters_to_lon(self.pos_e, BASE_LAT)
        airspeed_kmh = self.speed * 3.6

        # Battery drain (faster at high throttle)
        drain = (throttle / 100.0) * 0.0008 + 0.00005
        self.battery = max(10.8, self.battery - drain * self.dt)

        # Motor temperature (thermal model)
        target_temp = 22 + throttle * 0.55
        self.motor_temp += (target_temp - self.motor_temp) * 0.03 * self.dt

        # RSSI decreases with distance from field
        dist = math.sqrt(self.pos_n ** 2 + self.pos_e ** 2 + self.alt ** 2)
        rssi = clamp(99 - dist * 0.015, 35, 99)

        return {
            "gx": round(gx, 3),
            "gy": round(gy, 3),
            "gz": round(gz, 3),
            "lat": round(lat, 7),
            "lon": round(lon, 7),
            "alt": round(max(GROUND_ALT, self.alt), 2),
            "heading": round(wrap_heading(self.heading), 2),
            "airspeed": round(max(0, airspeed_kmh), 1),
            "battery": round(self.battery, 2),
            "rssi": round(rssi, 1),
            "throttle": round(clamp(throttle, 0, 100), 1),
            "motor_temp": round(self.motor_temp, 1),
            "ax": round(ax, 3),
            "ay": round(ay, 3),
            "az": round(az, 3),
            "phase": phase,
        }


# ---------------------------------------------------------------------------
# Frame formatting and UDP transmission
# ---------------------------------------------------------------------------


def format_frame(s):
    """Format sensor dict as a Serial Studio CSV frame."""
    fields = [
        f'{s["gx"]:.3f}',
        f'{s["gy"]:.3f}',
        f'{s["gz"]:.3f}',
        f'{s["ax"]:.3f}',
        f'{s["ay"]:.3f}',
        f'{s["az"]:.3f}',
        f'{s["lat"]:.7f}',
        f'{s["lon"]:.7f}',
        f'{s["alt"]:.2f}',
        f'{s["heading"]:.2f}',
        f'{s["airspeed"]:.1f}',
        f'{s["throttle"]:.1f}',
        f'{s["battery"]:.2f}',
        f'{s["rssi"]:.1f}',
        f'{s["motor_temp"]:.1f}',
    ]
    return "$" + ",".join(fields) + ";\n"


def send_udp(sock, addr, data):
    sock.sendto(data.encode("utf-8"), addr)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    port = UDP_PORT
    rate = FRAME_RATE
    loop = True

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--once":
            loop = False
        elif args[i] == "--fast":
            rate = 50
        elif args[i] == "--port" and i + 1 < len(args):
            i += 1
            port = int(args[i])
        elif args[i] in ("--help", "-h"):
            print(__doc__)
            sys.exit(0)
        else:
            print(f"Unknown option: {args[i]}")
            print("Use --help for usage information.")
            sys.exit(1)
        i += 1

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    addr = (UDP_HOST, port)

    sim = RCPlaneSimulator(frame_rate=rate)
    dt = 1.0 / rate
    flight_num = 1

    print(f"RC Plane Telemetry Simulator")
    print(f"Sending to {UDP_HOST}:{port} at {rate} Hz")
    print(f"Flight duration: {sim.total_duration:.0f}s")
    print(f"Mode: {'single flight' if not loop else 'continuous (Ctrl+C to stop)'}")
    print()

    try:
        while True:
            print(f"--- Flight #{flight_num} ---")
            last_phase = ""

            while not sim.is_done():
                t_start = time.monotonic()

                sensors = sim.step()
                frame = format_frame(sensors)
                send_udp(sock, addr, frame)

                phase = sensors["phase"]
                if phase != last_phase:
                    print(f"  [{sim.t:6.1f}s] {phase}")
                    last_phase = phase

                elapsed = time.monotonic() - t_start
                sleep_time = dt - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)

            print(f"  Flight #{flight_num} complete.\n")

            if not loop:
                break

            flight_num += 1
            sim.reset()

    except KeyboardInterrupt:
        print("\nSimulation stopped.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
