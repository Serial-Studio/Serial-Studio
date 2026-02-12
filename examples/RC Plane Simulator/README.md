# RC Plane Telemetry Simulator

A Python script that simulates a small RC plane flying a complete flight profile, generating realistic telemetry data for Serial Studio widgets.

## Purpose

This example is designed to validate and demonstrate Serial Studio widgets using a realistic RC plane flight profile with multiple maneuver phases.

## Widgets Exercised

| Widget Type    | Data Source                          |
|----------------|--------------------------------------|
| Gyroscope      | Pitch, Roll, Yaw rates (°/s)        |
| Accelerometer  | X, Y, Z body-frame acceleration (m/s²) |
| GPS Map        | Latitude, Longitude, Altitude        |
| Compass        | Heading (0°–360°)                    |
| Gauge          | Airspeed (km/h), Battery (V)         |
| Bar            | Throttle (%), RSSI (%), Motor Temp   |
| Time Plots     | Gyro rates, Accel axes, Altitude, Battery, RSSI |

## Flight Profile (~170 seconds)

```
Preflight → Engine Start → Taxi → Takeoff → Climb →
Level Flight → Gentle Turns → Steep Turns → Figure-8 →
Loop → Aileron Roll → Knife Edge → Inverted →
Dive & Pull-up → Zero-G Push → Recovery →
Cruise Home → Approach → Flare → Rollout → Shutdown
```

## Usage

1. Open Serial Studio and load `RC Plane Simulator.ssproj` as the project file
2. Set the I/O source to **UDP** on port **9000**
3. Run the simulator:

```bash
python3 rc_plane_simulator.py
```

### Options

```
python3 rc_plane_simulator.py              # Continuous loop
python3 rc_plane_simulator.py --once       # Single flight
python3 rc_plane_simulator.py --fast       # 50 Hz update rate
python3 rc_plane_simulator.py --port 8000  # Custom UDP port
```

## Frame Format

CSV over UDP with `$` start delimiter and `;` end delimiter:

```
$gx,gy,gz,ax,ay,az,lat,lon,alt,heading,airspeed,throttle,battery,rssi,motor_temp;\n
```

## Requirements

- Python 3.6+ (no external dependencies)
