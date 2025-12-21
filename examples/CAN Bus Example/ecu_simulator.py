#!/usr/bin/env python3
"""
VirtualCAN ECU Simulator for Serial Studio

This script simulates various ECU (Electronic Control Unit) components
communicating over a virtual CAN bus. Designed to work with Serial Studio's
CAN bus visualization module.

Requirements:
    pip install python-can

Usage:
    python ecu_simulator.py

The simulator creates multiple virtual ECU modules that broadcast realistic
automotive data over the virtual CAN bus.
"""

import can
import time
import struct
import math
import random
import socket
import threading
from dataclasses import dataclass
from typing import Callable, Dict, List, Optional
from enum import IntEnum
import argparse


# =============================================================================
# CAN ID Definitions (Standard 11-bit IDs)
# =============================================================================

class CanIds(IntEnum):
    """Standard CAN message IDs for different ECU components."""
    # Engine ECU
    ENGINE_RPM_SPEED = 0x100
    ENGINE_TEMP_LOAD = 0x101
    ENGINE_THROTTLE = 0x102
    
    # Transmission ECU
    TRANSMISSION_GEAR = 0x200
    TRANSMISSION_TEMP = 0x201
    
    # Body Control Module
    BCM_LIGHTS = 0x300
    BCM_DOORS = 0x301
    
    # Battery Management System (EV/Hybrid)
    BMS_VOLTAGE_CURRENT = 0x400
    BMS_SOC_TEMP = 0x401
    BMS_CELL_VOLTAGES = 0x402
    
    # Instrument Cluster
    CLUSTER_ODOMETER = 0x500
    CLUSTER_FUEL = 0x501
    
    # Chassis / ABS
    ABS_WHEEL_SPEEDS = 0x600
    ABS_BRAKE_PRESSURE = 0x601
    
    # Steering
    STEERING_ANGLE = 0x700
    
    # Airbag / Safety
    AIRBAG_STATUS = 0x7E0


# =============================================================================
# Data Classes for ECU State
# =============================================================================

@dataclass
class EngineState:
    """Engine ECU state variables."""
    rpm: float = 800.0          # RPM (0-8000)
    speed_kmh: float = 0.0      # Vehicle speed km/h (0-300)
    coolant_temp: float = 20.0  # Coolant temperature °C (-40 to 150)
    engine_load: float = 0.0    # Engine load % (0-100)
    throttle_pos: float = 0.0   # Throttle position % (0-100)
    intake_temp: float = 25.0   # Intake air temperature °C
    fuel_pressure: float = 350.0  # Fuel pressure kPa


@dataclass
class TransmissionState:
    """Transmission ECU state variables."""
    gear: int = 0               # Current gear (0=P, 1-6=forward, -1=R)
    trans_temp: float = 40.0    # Transmission fluid temp °C
    torque_converter_slip: float = 0.0  # Slip percentage


@dataclass
class BatteryState:
    """Battery Management System state (for EV/Hybrid)."""
    pack_voltage: float = 400.0     # Total pack voltage V
    pack_current: float = 0.0       # Pack current A (negative = charging)
    soc: float = 80.0               # State of charge %
    pack_temp: float = 25.0         # Average pack temperature °C
    cell_voltages: List[float] = None  # Individual cell voltages
    
    def __post_init__(self):
        if self.cell_voltages is None:
            # Simulate 96 cells in series (typical EV)
            self.cell_voltages = [4.1] * 8  # We'll send 8 sample cells


@dataclass 
class ChassisState:
    """Chassis and ABS state variables."""
    wheel_speed_fl: float = 0.0  # Front left wheel speed km/h
    wheel_speed_fr: float = 0.0  # Front right wheel speed km/h
    wheel_speed_rl: float = 0.0  # Rear left wheel speed km/h
    wheel_speed_rr: float = 0.0  # Rear right wheel speed km/h
    brake_pressure: float = 0.0  # Brake pressure bar
    steering_angle: float = 0.0  # Steering angle degrees (-720 to 720)


@dataclass
class BodyControlState:
    """Body Control Module state."""
    headlights: bool = False
    high_beams: bool = False
    turn_left: bool = False
    turn_right: bool = False
    hazards: bool = False
    door_fl: bool = False  # True = open
    door_fr: bool = False
    door_rl: bool = False
    door_rr: bool = False
    trunk: bool = False


@dataclass
class ClusterState:
    """Instrument cluster state."""
    odometer_km: float = 50000.0  # Total distance km
    trip_km: float = 0.0          # Trip distance km
    fuel_level: float = 75.0      # Fuel level %
    fuel_consumption: float = 8.5  # L/100km


# =============================================================================
# Qt VirtualCAN Client (TCP-based)
# =============================================================================

class QtVirtualCANClient:
    """
    Client for Qt's VirtualCAN TCP server.

    Protocol: Text-based format per Qt VirtualCAN spec:
      can<channel>:<CAN-ID>#<flags>#<hex-data>\n

    CAN-ID is sent as DECIMAL (not hex!):
      0x100 → "can0:256##data"  (256 decimal = 0x100 hex)

    Flags (optional):
      R - Remote Request
      X - Extended Frame Format
      F - Flexible Data Rate (CAN FD)
      B - Bitrate Switch
      E - Error State Indicator
      L - Local Echo

    Example: "can0:256##0102030405060708\n" (CAN ID 0x100, no flags)
    """

    def __init__(self, host: str = 'localhost', port: int = 35468,
                 channel: str = 'can0'):
        """Initialize VirtualCAN client."""
        self.host = host
        self.port = port
        self.channel = channel
        self.sock: Optional[socket.socket] = None

    def connect(self) -> bool:
        """Connect to Qt VirtualCAN TCP server."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print(f"Connected to Qt VirtualCAN: {self.host}:{self.port}/{self.channel}")
            return True
        except Exception as e:
            print(f"Failed to connect to Qt VirtualCAN")
            print(f"Error: {e}")
            print("\nTroubleshooting:")
            print("  - Start Serial Studio first")
            print("  - Select VirtualCAN driver")
            print(f"  - Connect to interface: {self.channel}")
            print("  - Then run this simulator")
            return False

    def disconnect(self):
        """Disconnect from server."""
        if self.sock:
            self.sock.close()
            self.sock = None

    def send_frame(self, can_id: int, data: bytes):
        """
        Send CAN frame to VirtualCAN server.

        Uses Qt VirtualCAN text protocol:
          can<channel>:<CAN-ID>#<flags>#<hex-data>\n

        CAN-ID must be decimal (e.g., 0x100 → 256):
          can0:256##0102030405060708\n
        """
        if not self.sock:
            return

        try:
            hex_data = data.hex().upper()
            flags = ""
            frame = f"{self.channel}:{can_id}#{flags}#{hex_data}\n"
            self.sock.sendall(frame.encode('utf-8'))
        except Exception as e:
            print(f"Error sending frame 0x{can_id:03X}: {e}")


# =============================================================================
# ECU Simulator Class
# =============================================================================

class ECUSimulator:
    """
    Main ECU Simulator class that manages virtual CAN bus communication
    and simulates realistic automotive data.
    """
    
    def __init__(self, interface: str = "socketcan", channel: str = "vcan0",
                 bitrate: int = 500000):
        """
        Initialize the ECU simulator.

        Args:
            interface: CAN interface type ('socketcan', 'pcan', 'vector', etc.)
            channel: CAN channel name (e.g., 'vcan0', 'can0', 'PCAN_USBBUS1')
            bitrate: CAN bus bitrate
        """
        self.interface = interface
        self.channel = channel
        self.bitrate = bitrate
        self.bus: Optional[can.Bus] = None
        
        # Initialize ECU states
        self.engine = EngineState()
        self.transmission = TransmissionState()
        self.battery = BatteryState()
        self.chassis = ChassisState()
        self.body = BodyControlState()
        self.cluster = ClusterState()
        
        # Simulation control
        self.running = False
        self.threads: List[threading.Thread] = []
        self.simulation_time = 0.0
        
        # Driving simulation parameters
        self.target_speed = 0.0
        self.acceleration = 0.0
        
    def connect(self) -> bool:
        """Connect to the CAN bus."""
        try:
            self.bus = can.Bus(
                interface=self.interface,
                channel=self.channel,
                bitrate=self.bitrate,
                receive_own_messages=True
            )
            print(f"Connected to {self.interface}:{self.channel} @ {self.bitrate} bps")
            return True
        except Exception as e:
            print(f"Failed to connect to {self.interface}:{self.channel}")
            print(f"Error: {e}")
            print("\nTroubleshooting:")
            if self.interface == "socketcan":
                print("  - Linux: Ensure vcan0 is set up (see README.md)")
                print("  - macOS/Windows: SocketCAN not supported, use hardware interface")
            elif self.interface == "pcan":
                print("  - Ensure PEAK PCAN drivers are installed")
                print("  - Check that PCAN hardware is connected")
            elif self.interface == "vector":
                print("  - Ensure Vector CAN drivers are installed")
                print("  - Check that Vector hardware is connected")
            return False
    
    def disconnect(self):
        """Disconnect from the CAN bus."""
        if self.bus:
            self.bus.shutdown()
            self.bus = None
            print("Disconnected from CAN bus")
    
    def send_frame(self, arbitration_id: int, data: bytes):
        """Send a CAN frame."""
        if self.bus:
            msg = can.Message(
                arbitration_id=arbitration_id,
                data=data,
                is_extended_id=False
            )
            try:
                self.bus.send(msg)
            except can.CanError as e:
                print(f"Error sending frame 0x{arbitration_id:03X}: {e}")

    # =========================================================================
    # Data Encoding Functions
    # =========================================================================
    
    def encode_engine_rpm_speed(self) -> bytes:
        """
        Encode engine RPM and vehicle speed.
        Format: [RPM_H, RPM_L, SPEED_H, SPEED_L, 0, 0, 0, 0]
        RPM: 0-8000 (0.25 RPM/bit)
        Speed: 0-300 km/h (0.01 km/h per bit)
        """
        rpm_raw = int(self.engine.rpm * 4)  # 0.25 RPM resolution
        speed_raw = int(self.engine.speed_kmh * 100)  # 0.01 km/h resolution
        
        return struct.pack('>HH4x', rpm_raw, speed_raw)
    
    def encode_engine_temp_load(self) -> bytes:
        """
        Encode coolant temp, intake temp, and engine load.
        Format: [COOLANT_TEMP, INTAKE_TEMP, LOAD, 0, 0, 0, 0, 0]
        Temps: offset by 40 (value + 40 = actual °C)
        Load: 0-100%
        """
        coolant = int(self.engine.coolant_temp + 40)
        intake = int(self.engine.intake_temp + 40)
        load = int(self.engine.engine_load)
        
        return struct.pack('BBB5x', coolant, intake, load)
    
    def encode_engine_throttle(self) -> bytes:
        """
        Encode throttle position and fuel pressure.
        Format: [THROTTLE, FUEL_PRESS_H, FUEL_PRESS_L, 0, 0, 0, 0, 0]
        """
        throttle = int(self.engine.throttle_pos * 2.55)  # Scale 0-100 to 0-255
        fuel_press = int(self.engine.fuel_pressure)
        
        return struct.pack('>BH5x', throttle, fuel_press)
    
    def encode_transmission(self) -> bytes:
        """
        Encode transmission gear and temperature.
        Format: [GEAR, TEMP, SLIP, 0, 0, 0, 0, 0]
        Gear: 0=P, 1-6=D, 255=R, 254=N
        """
        if self.transmission.gear == -1:
            gear_byte = 255
        elif self.transmission.gear == 0:
            gear_byte = 254  # Neutral/Park
        else:
            gear_byte = self.transmission.gear
            
        temp = int(self.transmission.trans_temp + 40)
        slip = int(self.transmission.torque_converter_slip)
        
        return struct.pack('BBB5x', gear_byte, temp, slip)
    
    def encode_bms_voltage_current(self) -> bytes:
        """
        Encode battery pack voltage and current.
        Format: [VOLTAGE_H, VOLTAGE_L, CURRENT_H, CURRENT_L, 0, 0, 0, 0]
        Voltage: 0.1V resolution
        Current: 0.1A resolution, signed
        """
        voltage_raw = int(self.battery.pack_voltage * 10)
        current_raw = int(self.battery.pack_current * 10)
        
        return struct.pack('>Hh4x', voltage_raw, current_raw)
    
    def encode_bms_soc_temp(self) -> bytes:
        """
        Encode state of charge and pack temperature.
        Format: [SOC, TEMP, 0, 0, 0, 0, 0, 0]
        SOC: 0-100%
        Temp: offset by 40
        """
        soc = int(self.battery.soc)
        temp = int(self.battery.pack_temp + 40)
        
        return struct.pack('BB6x', soc, temp)
    
    def encode_bms_cell_voltages(self) -> bytes:
        """
        Encode sample cell voltages (8 cells, 1 byte each).
        Format: [CELL1, CELL2, CELL3, CELL4, CELL5, CELL6, CELL7, CELL8]
        Each cell: (value * 50) - 150 = voltage in mV offset
        """
        cells = []
        for v in self.battery.cell_voltages[:8]:
            # Map 3.0V-4.2V to 0-255
            cell_byte = int((v - 3.0) * 212.5)
            cell_byte = max(0, min(255, cell_byte))
            cells.append(cell_byte)
        
        return bytes(cells)
    
    def encode_wheel_speeds(self) -> bytes:
        """
        Encode individual wheel speeds.
        Format: [FL_H, FL_L, FR_H, FR_L, RL_H, RL_L, RR_H, RR_L]
        Speed: 0.01 km/h resolution
        """
        fl = int(self.chassis.wheel_speed_fl * 100)
        fr = int(self.chassis.wheel_speed_fr * 100)
        rl = int(self.chassis.wheel_speed_rl * 100)
        rr = int(self.chassis.wheel_speed_rr * 100)
        
        return struct.pack('>HHHH', fl, fr, rl, rr)
    
    def encode_brake_pressure(self) -> bytes:
        """
        Encode brake pressure.
        Format: [PRESSURE_H, PRESSURE_L, 0, 0, 0, 0, 0, 0]
        Pressure: 0.1 bar resolution
        """
        pressure = int(self.chassis.brake_pressure * 10)
        
        return struct.pack('>H6x', pressure)
    
    def encode_steering_angle(self) -> bytes:
        """
        Encode steering wheel angle.
        Format: [ANGLE_H, ANGLE_L, RATE_H, RATE_L, 0, 0, 0, 0]
        Angle: 0.1 degree resolution, signed
        """
        angle = int(self.chassis.steering_angle * 10)
        
        return struct.pack('>h6x', angle)
    
    def encode_body_lights(self) -> bytes:
        """
        Encode lighting status.
        Format: [LIGHTS_BYTE, 0, 0, 0, 0, 0, 0, 0]
        Bit 0: Headlights
        Bit 1: High beams
        Bit 2: Left turn
        Bit 3: Right turn
        Bit 4: Hazards
        """
        lights = 0
        if self.body.headlights: lights |= 0x01
        if self.body.high_beams: lights |= 0x02
        if self.body.turn_left: lights |= 0x04
        if self.body.turn_right: lights |= 0x08
        if self.body.hazards: lights |= 0x10
        
        return struct.pack('B7x', lights)
    
    def encode_body_doors(self) -> bytes:
        """
        Encode door status.
        Format: [DOORS_BYTE, 0, 0, 0, 0, 0, 0, 0]
        Bit 0: FL door
        Bit 1: FR door
        Bit 2: RL door
        Bit 3: RR door
        Bit 4: Trunk
        """
        doors = 0
        if self.body.door_fl: doors |= 0x01
        if self.body.door_fr: doors |= 0x02
        if self.body.door_rl: doors |= 0x04
        if self.body.door_rr: doors |= 0x08
        if self.body.trunk: doors |= 0x10
        
        return struct.pack('B7x', doors)
    
    def encode_odometer(self) -> bytes:
        """
        Encode odometer and trip readings.
        Format: [ODO_3, ODO_2, ODO_1, ODO_0, TRIP_H, TRIP_L, 0, 0]
        Odometer: 0.1 km resolution
        Trip: 0.1 km resolution
        """
        odo = int(self.cluster.odometer_km * 10)
        trip = int(self.cluster.trip_km * 10)
        
        return struct.pack('>IH2x', odo, trip)
    
    def encode_fuel(self) -> bytes:
        """
        Encode fuel level and consumption.
        Format: [FUEL_LEVEL, CONSUMPTION_H, CONSUMPTION_L, 0, 0, 0, 0, 0]
        Level: 0-100%
        Consumption: 0.01 L/100km resolution
        """
        level = int(self.cluster.fuel_level)
        consumption = int(self.cluster.fuel_consumption * 100)
        
        return struct.pack('>BH5x', level, consumption)

    # =========================================================================
    # Simulation Update Functions
    # =========================================================================
    
    def update_driving_simulation(self, dt: float):
        """
        Update the driving simulation state.
        Creates realistic acceleration, cruising, and braking patterns.
        """
        self.simulation_time += dt
        
        # Create a driving pattern: accelerate, cruise, brake, repeat
        cycle_time = self.simulation_time % 60  # 60-second cycle
        
        if cycle_time < 15:
            # Accelerating phase
            self.target_speed = min(120, cycle_time * 8)
            self.acceleration = 2.0
            self.engine.throttle_pos = 60 + random.uniform(-5, 5)
            self.chassis.brake_pressure = 0
        elif cycle_time < 40:
            # Cruising phase
            self.target_speed = 100 + 20 * math.sin(cycle_time * 0.2)
            self.acceleration = 0
            self.engine.throttle_pos = 30 + random.uniform(-3, 3)
            self.chassis.brake_pressure = 0
        elif cycle_time < 50:
            # Braking phase
            self.target_speed = max(0, 120 - (cycle_time - 40) * 12)
            self.acceleration = -3.0
            self.engine.throttle_pos = 0
            self.chassis.brake_pressure = 50 + random.uniform(-5, 5)
        else:
            # Stopped/idle phase
            self.target_speed = 0
            self.acceleration = 0
            self.engine.throttle_pos = 0
            self.chassis.brake_pressure = 0
        
        # Smoothly adjust speed toward target
        speed_diff = self.target_speed - self.engine.speed_kmh
        self.engine.speed_kmh += speed_diff * 0.1
        self.engine.speed_kmh = max(0, self.engine.speed_kmh)
        
        # Update RPM based on speed and gear
        self._update_engine(dt)
        self._update_transmission(dt)
        self._update_chassis(dt)
        self._update_battery(dt)
        self._update_body(dt)
        self._update_cluster(dt)
    
    def _update_engine(self, dt: float):
        """Update engine parameters."""
        # RPM calculation based on speed and gear
        if self.transmission.gear > 0:
            gear_ratio = [0, 3.5, 2.1, 1.4, 1.0, 0.8, 0.65][self.transmission.gear]
            target_rpm = 800 + (self.engine.speed_kmh * gear_ratio * 40)
        else:
            target_rpm = 800 + self.engine.throttle_pos * 20
        
        # Smooth RPM changes
        rpm_diff = target_rpm - self.engine.rpm
        self.engine.rpm += rpm_diff * 0.3
        self.engine.rpm = max(600, min(7000, self.engine.rpm))
        self.engine.rpm += random.uniform(-20, 20)  # Add jitter
        
        # Engine load based on throttle and RPM
        self.engine.engine_load = (self.engine.throttle_pos * 0.7 + 
                                   self.engine.rpm / 7000 * 30)
        self.engine.engine_load = min(100, self.engine.engine_load)
        
        # Coolant temperature (warms up over time, stabilizes around 90°C)
        target_temp = 90 + self.engine.engine_load * 0.1
        temp_diff = target_temp - self.engine.coolant_temp
        self.engine.coolant_temp += temp_diff * 0.01
        self.engine.coolant_temp += random.uniform(-0.2, 0.2)
        
        # Intake temperature varies slightly
        self.engine.intake_temp = 25 + math.sin(self.simulation_time * 0.1) * 5
        
        # Fuel pressure stays relatively constant
        self.engine.fuel_pressure = 350 + random.uniform(-5, 5)
    
    def _update_transmission(self, dt: float):
        """Update transmission parameters."""
        # Auto gear selection based on speed
        speed = self.engine.speed_kmh
        if speed < 5:
            target_gear = 0  # Neutral/Park
        elif speed < 20:
            target_gear = 1
        elif speed < 40:
            target_gear = 2
        elif speed < 60:
            target_gear = 3
        elif speed < 80:
            target_gear = 4
        elif speed < 100:
            target_gear = 5
        else:
            target_gear = 6
        
        self.transmission.gear = target_gear
        
        # Transmission temperature
        target_trans_temp = 60 + self.engine.engine_load * 0.3
        temp_diff = target_trans_temp - self.transmission.trans_temp
        self.transmission.trans_temp += temp_diff * 0.005
        
        # Torque converter slip (higher at low speeds)
        if speed < 20:
            self.transmission.torque_converter_slip = 15 - speed * 0.5
        else:
            self.transmission.torque_converter_slip = max(0, 5 - (speed - 20) * 0.1)
    
    def _update_chassis(self, dt: float):
        """Update chassis and wheel parameters."""
        base_speed = self.engine.speed_kmh
        
        # Simulate slight wheel speed differences (realistic)
        self.chassis.wheel_speed_fl = base_speed + random.uniform(-0.5, 0.5)
        self.chassis.wheel_speed_fr = base_speed + random.uniform(-0.5, 0.5)
        self.chassis.wheel_speed_rl = base_speed + random.uniform(-0.3, 0.3)
        self.chassis.wheel_speed_rr = base_speed + random.uniform(-0.3, 0.3)
        
        # Ensure non-negative
        self.chassis.wheel_speed_fl = max(0, self.chassis.wheel_speed_fl)
        self.chassis.wheel_speed_fr = max(0, self.chassis.wheel_speed_fr)
        self.chassis.wheel_speed_rl = max(0, self.chassis.wheel_speed_rl)
        self.chassis.wheel_speed_rr = max(0, self.chassis.wheel_speed_rr)
        
        # Steering angle simulation (gentle lane changes)
        self.chassis.steering_angle = 30 * math.sin(self.simulation_time * 0.3)
    
    def _update_battery(self, dt: float):
        """Update battery/BMS parameters."""
        # Simulate power draw based on speed and load
        power_kw = self.engine.speed_kmh * 0.15 + self.engine.engine_load * 0.5
        
        # Current draw (P = V * I)
        self.battery.pack_current = power_kw * 1000 / self.battery.pack_voltage
        self.battery.pack_current += random.uniform(-2, 2)
        
        # SOC slowly decreases
        self.battery.soc -= self.battery.pack_current * dt / 3600 * 0.01
        self.battery.soc = max(0, min(100, self.battery.soc))
        
        # Pack voltage varies with SOC and current
        self.battery.pack_voltage = 350 + self.battery.soc * 0.8 - self.battery.pack_current * 0.05
        
        # Temperature rises with current
        target_temp = 25 + abs(self.battery.pack_current) * 0.1
        temp_diff = target_temp - self.battery.pack_temp
        self.battery.pack_temp += temp_diff * 0.01
        
        # Update cell voltages (slight variation between cells)
        base_cell_v = self.battery.pack_voltage / 96
        self.battery.cell_voltages = [
            base_cell_v + random.uniform(-0.02, 0.02) for _ in range(8)
        ]
    
    def _update_body(self, dt: float):
        """Update body control module parameters."""
        # Headlights on during "night" portions of simulation
        self.body.headlights = (int(self.simulation_time / 30) % 2 == 0)
        
        # Turn signals during lane changes
        steering = self.chassis.steering_angle
        self.body.turn_left = steering < -15
        self.body.turn_right = steering > 15
        
        # Doors always closed during driving
        self.body.door_fl = False
        self.body.door_fr = False
        self.body.door_rl = False
        self.body.door_rr = False
        self.body.trunk = False
    
    def _update_cluster(self, dt: float):
        """Update instrument cluster parameters."""
        # Odometer increases with speed
        distance_km = self.engine.speed_kmh * dt / 3600
        self.cluster.odometer_km += distance_km
        self.cluster.trip_km += distance_km
        
        # Fuel consumption and level
        if self.engine.speed_kmh > 0:
            # L/100km varies with speed and load
            self.cluster.fuel_consumption = 5 + self.engine.engine_load * 0.1
            # Fuel level decreases
            fuel_used = self.cluster.fuel_consumption * distance_km / 100
            self.cluster.fuel_level -= fuel_used
            self.cluster.fuel_level = max(0, self.cluster.fuel_level)
        else:
            self.cluster.fuel_consumption = 0

    # =========================================================================
    # CAN Message Transmission
    # =========================================================================
    
    def transmit_engine_messages(self):
        """Transmit engine ECU messages."""
        self.send_frame(CanIds.ENGINE_RPM_SPEED, self.encode_engine_rpm_speed())
        self.send_frame(CanIds.ENGINE_TEMP_LOAD, self.encode_engine_temp_load())
        self.send_frame(CanIds.ENGINE_THROTTLE, self.encode_engine_throttle())
    
    def transmit_transmission_messages(self):
        """Transmit transmission ECU messages."""
        self.send_frame(CanIds.TRANSMISSION_GEAR, self.encode_transmission())
    
    def transmit_bms_messages(self):
        """Transmit BMS messages."""
        self.send_frame(CanIds.BMS_VOLTAGE_CURRENT, self.encode_bms_voltage_current())
        self.send_frame(CanIds.BMS_SOC_TEMP, self.encode_bms_soc_temp())
        self.send_frame(CanIds.BMS_CELL_VOLTAGES, self.encode_bms_cell_voltages())
    
    def transmit_chassis_messages(self):
        """Transmit chassis/ABS messages."""
        self.send_frame(CanIds.ABS_WHEEL_SPEEDS, self.encode_wheel_speeds())
        self.send_frame(CanIds.ABS_BRAKE_PRESSURE, self.encode_brake_pressure())
        self.send_frame(CanIds.STEERING_ANGLE, self.encode_steering_angle())
    
    def transmit_body_messages(self):
        """Transmit body control messages."""
        self.send_frame(CanIds.BCM_LIGHTS, self.encode_body_lights())
        self.send_frame(CanIds.BCM_DOORS, self.encode_body_doors())
    
    def transmit_cluster_messages(self):
        """Transmit instrument cluster messages."""
        self.send_frame(CanIds.CLUSTER_ODOMETER, self.encode_odometer())
        self.send_frame(CanIds.CLUSTER_FUEL, self.encode_fuel())

    # =========================================================================
    # Main Simulation Loop
    # =========================================================================
    
    def run(self, update_rate: float = 50.0):
        """
        Run the main simulation loop.
        
        Args:
            update_rate: Simulation update frequency in Hz
        """
        if not self.connect():
            return
        
        self.running = True
        dt = 1.0 / update_rate
        
        print(f"Starting ECU simulation at {update_rate} Hz")
        print("Press Ctrl+C to stop")
        print("-" * 50)
        
        # Message transmission rates (in Hz)
        rates = {
            'engine': 100,      # 100 Hz (every 10ms)
            'transmission': 50, # 50 Hz (every 20ms)
            'bms': 10,          # 10 Hz (every 100ms)
            'chassis': 100,     # 100 Hz (every 10ms)
            'body': 10,         # 10 Hz (every 100ms)
            'cluster': 5,       # 5 Hz (every 200ms)
        }
        
        # Counters for rate control
        counters = {key: 0 for key in rates}
        
        try:
            last_time = time.time()
            frame_count = 0
            
            while self.running:
                current_time = time.time()
                elapsed = current_time - last_time
                
                if elapsed >= dt:
                    last_time = current_time
                    frame_count += 1
                    
                    # Update simulation state
                    self.update_driving_simulation(dt)
                    
                    # Transmit messages based on their rates
                    for msg_type, rate in rates.items():
                        counters[msg_type] += 1
                        interval = int(update_rate / rate)
                        
                        if counters[msg_type] >= interval:
                            counters[msg_type] = 0
                            
                            if msg_type == 'engine':
                                self.transmit_engine_messages()
                            elif msg_type == 'transmission':
                                self.transmit_transmission_messages()
                            elif msg_type == 'bms':
                                self.transmit_bms_messages()
                            elif msg_type == 'chassis':
                                self.transmit_chassis_messages()
                            elif msg_type == 'body':
                                self.transmit_body_messages()
                            elif msg_type == 'cluster':
                                self.transmit_cluster_messages()
                    
                    # Print status every second
                    if frame_count % int(update_rate) == 0:
                        self._print_status()
                else:
                    # Small sleep to prevent busy-waiting
                    time.sleep(0.0001)
                    
        except KeyboardInterrupt:
            print("\nStopping simulation...")
        finally:
            self.running = False
            self.disconnect()
    
    def _print_status(self):
        """Print current simulation status to console."""
        print(f"\rRPM: {self.engine.rpm:5.0f} | "
              f"Speed: {self.engine.speed_kmh:5.1f} km/h | "
              f"Gear: {self.transmission.gear} | "
              f"Throttle: {self.engine.throttle_pos:4.1f}% | "
              f"Coolant: {self.engine.coolant_temp:4.1f}°C | "
              f"SOC: {self.battery.soc:5.1f}%", end='')


# =============================================================================
# VirtualCAN Standalone Simulation
# =============================================================================

def run_virtual_simulation(client: QtVirtualCANClient, update_rate: float):
    """
    Run standalone ECU simulation using VirtualCAN (no python-can dependency).
    """
    # Initialize state
    engine = EngineState()
    transmission = TransmissionState()
    battery = BatteryState()

    simulation_time = 0.0
    dt = 1.0 / update_rate

    print(f"\nSimulating vehicle ECU at {update_rate} Hz")
    print("Press Ctrl+C to stop")
    print("-" * 60)

    try:
        last_time = time.time()
        frame_count = 0

        while True:
            current_time = time.time()
            elapsed = current_time - last_time

            if elapsed >= dt:
                last_time = current_time
                frame_count += 1
                simulation_time += dt

                # Simple driving simulation
                cycle_time = simulation_time % 60

                if cycle_time < 15:
                    target_speed = min(120, cycle_time * 8)
                    engine.throttle_pos = 60 + random.uniform(-5, 5)
                elif cycle_time < 40:
                    target_speed = 100 + 20 * math.sin(cycle_time * 0.2)
                    engine.throttle_pos = 30 + random.uniform(-3, 3)
                elif cycle_time < 50:
                    target_speed = max(0, 120 - (cycle_time - 40) * 12)
                    engine.throttle_pos = 0
                else:
                    target_speed = 0
                    engine.throttle_pos = 0

                # Update speed
                speed_diff = target_speed - engine.speed_kmh
                engine.speed_kmh += speed_diff * 0.1
                engine.speed_kmh = max(0, engine.speed_kmh)

                # Update gear
                if engine.speed_kmh < 5:
                    transmission.gear = 0
                elif engine.speed_kmh < 20:
                    transmission.gear = 1
                elif engine.speed_kmh < 40:
                    transmission.gear = 2
                elif engine.speed_kmh < 60:
                    transmission.gear = 3
                elif engine.speed_kmh < 80:
                    transmission.gear = 4
                elif engine.speed_kmh < 100:
                    transmission.gear = 5
                else:
                    transmission.gear = 6

                # Update RPM
                if transmission.gear > 0:
                    gear_ratios = [0, 3.5, 2.1, 1.4, 1.0, 0.8, 0.65]
                    target_rpm = 800 + (engine.speed_kmh * gear_ratios[transmission.gear] * 40)
                else:
                    target_rpm = 800 + engine.throttle_pos * 20

                rpm_diff = target_rpm - engine.rpm
                engine.rpm += rpm_diff * 0.3
                engine.rpm = max(600, min(7000, engine.rpm))
                engine.rpm += random.uniform(-20, 20)

                # Update engine params
                engine.engine_load = min(100, engine.throttle_pos * 0.7 + engine.rpm / 7000 * 30)
                target_temp = 90 + engine.engine_load * 0.1
                engine.coolant_temp += (target_temp - engine.coolant_temp) * 0.01
                engine.intake_temp = 25 + math.sin(simulation_time * 0.1) * 5
                engine.fuel_pressure = 350 + random.uniform(-5, 5)

                # Update battery
                power_kw = engine.speed_kmh * 0.15 + engine.engine_load * 0.5
                battery.pack_current = power_kw * 1000 / battery.pack_voltage
                battery.soc = max(0, min(100, battery.soc - battery.pack_current * dt / 3600 * 0.01))
                battery.pack_voltage = 350 + battery.soc * 0.8

                # Send CAN frames
                # 0x100: RPM and Speed
                rpm_raw = int(engine.rpm * 4)
                speed_raw = int(engine.speed_kmh * 100)
                data = struct.pack('>HH4x', rpm_raw, speed_raw)
                client.send_frame(CanIds.ENGINE_RPM_SPEED, data)

                # 0x101: Temps and load
                coolant = int(engine.coolant_temp + 40)
                intake = int(engine.intake_temp + 40)
                load = int(engine.engine_load)
                data = struct.pack('BBB5x', coolant, intake, load)
                client.send_frame(CanIds.ENGINE_TEMP_LOAD, data)

                # 0x102: Throttle and fuel
                throttle = int(engine.throttle_pos * 2.55)
                fuel_press = int(engine.fuel_pressure)
                data = struct.pack('>BH5x', throttle, fuel_press)
                client.send_frame(CanIds.ENGINE_THROTTLE, data)

                # 0x200: Transmission
                gear_byte = 254 if transmission.gear == 0 else transmission.gear
                data = struct.pack('BB6x', gear_byte, 80)
                client.send_frame(CanIds.TRANSMISSION_GEAR, data)

                # 0x400: Battery V/I
                voltage_raw = int(battery.pack_voltage * 10)
                current_raw = int(battery.pack_current * 10)
                data = struct.pack('>Hh4x', voltage_raw, current_raw)
                client.send_frame(CanIds.BMS_VOLTAGE_CURRENT, data)

                # 0x401: Battery SOC
                soc = int(battery.soc)
                data = struct.pack('BB6x', soc, 65)
                client.send_frame(CanIds.BMS_SOC_TEMP, data)

                # Print status every second
                if frame_count % int(update_rate) == 0:
                    print(f"\rRPM: {engine.rpm:5.0f} | "
                          f"Speed: {engine.speed_kmh:5.1f} km/h | "
                          f"Gear: {transmission.gear} | "
                          f"Throttle: {engine.throttle_pos:4.1f}% | "
                          f"Coolant: {engine.coolant_temp:4.1f}°C | "
                          f"SOC: {battery.soc:5.1f}%", end='')
            else:
                time.sleep(0.0001)

    except KeyboardInterrupt:
        print("\n\nStopping simulation...")
    finally:
        client.disconnect()


# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    import platform
    import sys

    parser = argparse.ArgumentParser(
        description='CAN ECU Simulator for Serial Studio',
        epilog='Examples:\n'
               '  python ecu_simulator.py -i pcan -c PCAN_USBBUS1\n'
               '  python ecu_simulator.py -i socketcan -c vcan0\n'
               '  python ecu_simulator.py --virtual',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '-i', '--interface',
        default=None,
        choices=['socketcan', 'pcan', 'vector', 'systec', 'tinycan',
                 'passthru', 'virtual'],
        help='CAN interface type (auto-detects if not specified)'
    )
    parser.add_argument(
        '-c', '--channel',
        default=None,
        help='CAN channel name (auto-detects if not specified)'
    )
    parser.add_argument(
        '-b', '--bitrate',
        type=int,
        default=500000,
        help='CAN bus bitrate (default: 500000)'
    )
    parser.add_argument(
        '-r', '--rate',
        type=float,
        default=100.0,
        help='Simulation update rate in Hz (default: 100)'
    )
    parser.add_argument(
        '--virtual',
        action='store_true',
        help='Use Qt VirtualCAN (no hardware, works on all platforms)'
    )

    args = parser.parse_args()

    # Handle --virtual flag
    if args.virtual:
        args.interface = 'virtual'
        args.channel = args.channel or 'can0'

    # Auto-detect interface and channel based on platform
    if args.interface is None or args.channel is None:
        system = platform.system()

        if system == "Linux":
            args.interface = args.interface or "socketcan"
            args.channel = args.channel or "vcan0"
        elif system == "Darwin":  # macOS
            print("macOS detected")
            print("\nOptions:")
            print("  1. VirtualCAN (no hardware): python ecu_simulator.py --virtual")
            print("  2. PCAN hardware: python ecu_simulator.py -i pcan -c PCAN_USBBUS1")
            sys.exit(1)
        elif system == "Windows":
            print("Windows detected")
            print("\nOptions:")
            print("  1. VirtualCAN (no hardware): python ecu_simulator.py --virtual")
            print("  2. PCAN hardware: python ecu_simulator.py -i pcan -c PCAN_USBBUS1")
            sys.exit(1)
        else:
            print(f"Unknown platform: {system}")
            print("Please specify interface and channel manually")
            sys.exit(1)

    # Use VirtualCAN with standalone implementation
    if args.interface == 'virtual':
        print("=" * 60)
        print("Qt VirtualCAN ECU Simulator")
        print("=" * 60)
        print(f"Server:  localhost:35468")
        print(f"Channel: {args.channel}")
        print(f"Rate:    {args.rate} Hz")
        print("=" * 60)
        print("\nIMPORTANT: Start Serial Studio and connect to VirtualCAN first!")
        print(f"  Setup → CAN Bus → Driver: VirtualCAN, Interface: {args.channel}")
        print("=" * 60)

        # Create VirtualCAN client
        client = QtVirtualCANClient(channel=args.channel)
        if not client.connect():
            sys.exit(1)

        # Run standalone virtual simulation
        run_virtual_simulation(client, args.rate)
        return

    print("=" * 60)
    print("CAN ECU Simulator for Serial Studio")
    print("=" * 60)
    print(f"Platform:  {platform.system()}")
    print(f"Interface: {args.interface}")
    print(f"Channel:   {args.channel}")
    print(f"Bitrate:   {args.bitrate} bps")
    print(f"Rate:      {args.rate} Hz")
    print("=" * 60)

    simulator = ECUSimulator(
        interface=args.interface,
        channel=args.channel,
        bitrate=args.bitrate
    )

    simulator.run(update_rate=args.rate)


if __name__ == '__main__':
    main()
