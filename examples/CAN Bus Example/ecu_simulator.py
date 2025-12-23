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
# DBC-Defined Value Ranges (from ecu_simulator.dbc)
# =============================================================================

class DBCRanges:
    """Physical value ranges as defined in the DBC file."""
    ENGINE_RPM_MIN = 0
    ENGINE_RPM_MAX = 7000

    VEHICLE_SPEED_MIN = 0
    VEHICLE_SPEED_MAX = 200

    COOLANT_TEMP_MIN = -40
    COOLANT_TEMP_MAX = 215

    INTAKE_TEMP_MIN = -40
    INTAKE_TEMP_MAX = 215

    ENGINE_LOAD_MIN = 0
    ENGINE_LOAD_MAX = 100

    THROTTLE_POS_MIN = 0
    THROTTLE_POS_MAX = 100

    FUEL_PRESSURE_MIN = 0
    FUEL_PRESSURE_MAX = 1000

    TRANSMISSION_TEMP_MIN = -40
    TRANSMISSION_TEMP_MAX = 215

    PACK_VOLTAGE_MIN = 0
    PACK_VOLTAGE_MAX = 850

    PACK_CURRENT_MIN = -500
    PACK_CURRENT_MAX = 500

    CELL_VOLTAGE_MIN = 3.0
    CELL_VOLTAGE_MAX = 4.2

    WHEEL_SPEED_MIN = 0
    WHEEL_SPEED_MAX = 200

    BRAKE_PRESSURE_MIN = 0
    BRAKE_PRESSURE_MAX = 200

    STEERING_ANGLE_MIN = -720
    STEERING_ANGLE_MAX = 720

    FUEL_LEVEL_MIN = 0
    FUEL_LEVEL_MAX = 100

    FUEL_CONSUMPTION_MIN = 0
    FUEL_CONSUMPTION_MAX = 50


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
    
    def __init__(self, debug: bool = False):
        """Initialize the ECU simulator with physics state only."""
        self.can_sender: Optional[Callable[[int, bytes], None]] = None
        self.debug = debug
        
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

        # Noise generators (Ornstein-Uhlenbeck process for realistic sensor noise)
        self.rpm_noise = 0.0
        self.coolant_temp_noise = 0.0
        self.intake_temp_noise = 0.0
        self.fuel_pressure_noise = 0.0
        self.wheel_speed_noise = [0.0, 0.0, 0.0, 0.0]
        self.brake_pressure_noise = 0.0
        self.steering_noise = 0.0
        self.battery_voltage_noise = 0.0
        self.battery_current_noise = 0.0

        # Smooth brake pressure state
        self.brake_pressure_actual = 0.0
        self.brake_pressure_target = 0.0

    def set_can_sender(self, sender: Callable[[int, bytes], None]):
        """
        Set the CAN frame sender callback.

        Args:
            sender: Function that takes (can_id: int, data: bytes) and sends the frame
        """
        self.can_sender = sender

    def send_frame(self, arbitration_id: int, data: bytes):
        """Send a CAN frame using the configured sender."""
        if self.debug and arbitration_id == 0x100:
            print(f"\n[DEBUG] CAN ID 0x{arbitration_id:03X}: {data.hex().upper()}")
            print(f"        Byte 0-1 (RPM raw): 0x{data[0]:02X}{data[1]:02X} = {(data[0]<<8)|data[1]} → {((data[0]<<8)|data[1])*0.25:.1f} RPM")
            print(f"        Byte 2-3 (Speed raw): 0x{data[2]:02X}{data[3]:02X} = {(data[2]<<8)|data[3]} → {((data[2]<<8)|data[3])*0.01:.1f} km/h")

        if self.can_sender:
            self.can_sender(arbitration_id, data)

    # =========================================================================
    # Data Encoding Functions
    # =========================================================================
    
    def encode_engine_rpm_speed(self) -> bytes:
        """
        Encode engine RPM and vehicle speed with sensor noise.
        Format: [RPM_H, RPM_L, SPEED_H, SPEED_L, 0, 0, 0, 0]
        RPM: 0-8000 (0.25 RPM/bit)
        Speed: 0-300 km/h (0.01 km/h per bit)
        """
        rpm_with_noise = self.engine.rpm + self.rpm_noise
        rpm_clamped = max(DBCRanges.ENGINE_RPM_MIN, min(DBCRanges.ENGINE_RPM_MAX, rpm_with_noise))
        rpm_raw = int(rpm_clamped * 4)

        speed_clamped = max(DBCRanges.VEHICLE_SPEED_MIN, min(DBCRanges.VEHICLE_SPEED_MAX, self.engine.speed_kmh))
        speed_raw = int(speed_clamped * 100)

        return struct.pack('<HH4x', rpm_raw, speed_raw)
    
    def encode_engine_temp_load(self) -> bytes:
        """
        Encode coolant temp, intake temp, and engine load with sensor noise.
        Format: [COOLANT_TEMP, INTAKE_TEMP, LOAD, 0, 0, 0, 0, 0]
        Temps: offset by 40 (value + 40 = actual °C)
        Load: 0-100%
        """
        coolant_with_noise = self.engine.coolant_temp + self.coolant_temp_noise
        coolant_clamped = max(DBCRanges.COOLANT_TEMP_MIN, min(DBCRanges.COOLANT_TEMP_MAX, coolant_with_noise))
        coolant = int(coolant_clamped + 40)

        intake_with_noise = self.engine.intake_temp + self.intake_temp_noise
        intake_clamped = max(DBCRanges.INTAKE_TEMP_MIN, min(DBCRanges.INTAKE_TEMP_MAX, intake_with_noise))
        intake = int(intake_clamped + 40)

        load_clamped = max(DBCRanges.ENGINE_LOAD_MIN, min(DBCRanges.ENGINE_LOAD_MAX, self.engine.engine_load))
        load = int(load_clamped)

        return struct.pack('BBB5x', coolant, intake, load)
    
    def encode_engine_throttle(self) -> bytes:
        """
        Encode throttle position and fuel pressure with sensor noise.
        Format: [THROTTLE, FUEL_PRESS_H, FUEL_PRESS_L, 0, 0, 0, 0, 0]
        """
        throttle_clamped = max(DBCRanges.THROTTLE_POS_MIN, min(DBCRanges.THROTTLE_POS_MAX, self.engine.throttle_pos))
        throttle = int(throttle_clamped * 2.55)

        fuel_with_noise = self.engine.fuel_pressure + self.fuel_pressure_noise
        fuel_clamped = max(DBCRanges.FUEL_PRESSURE_MIN, min(DBCRanges.FUEL_PRESSURE_MAX, fuel_with_noise))
        fuel_press = int(fuel_clamped)

        return struct.pack('<BH5x', throttle, fuel_press)
    
    def encode_transmission(self) -> bytes:
        """
        Encode transmission gear and temperature.
        Format: [GEAR, TEMP, SLIP, 0, 0, 0, 0, 0]
        Gear: 0=P, 1-6=D, 255=R, 254=N
        """
        if self.transmission.gear == -1:
            gear_byte = 255
        elif self.transmission.gear == 0:
            gear_byte = 254
        else:
            gear_byte = self.transmission.gear

        temp_clamped = max(DBCRanges.TRANSMISSION_TEMP_MIN, min(DBCRanges.TRANSMISSION_TEMP_MAX, self.transmission.trans_temp))
        temp = int(temp_clamped + 40)

        slip_clamped = max(0, min(100, self.transmission.torque_converter_slip))
        slip = int(slip_clamped)

        return struct.pack('BBB5x', gear_byte, temp, slip)
    
    def encode_bms_voltage_current(self) -> bytes:
        """
        Encode battery pack voltage and current with sensor noise.
        Format: [VOLTAGE_H, VOLTAGE_L, CURRENT_H, CURRENT_L, 0, 0, 0, 0]
        Voltage: 0.1V resolution
        Current: 0.1A resolution, signed
        """
        voltage_with_noise = self.battery.pack_voltage + self.battery_voltage_noise
        voltage_clamped = max(DBCRanges.PACK_VOLTAGE_MIN, min(DBCRanges.PACK_VOLTAGE_MAX, voltage_with_noise))
        voltage_raw = int(voltage_clamped * 10)

        current_with_noise = self.battery.pack_current + self.battery_current_noise
        current_clamped = max(DBCRanges.PACK_CURRENT_MIN, min(DBCRanges.PACK_CURRENT_MAX, current_with_noise))
        current_raw = int(current_clamped * 10)

        return struct.pack('<Hh4x', voltage_raw, current_raw)
    
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
        Encode individual wheel speeds with sensor noise.
        Format: [FL_H, FL_L, FR_H, FR_L, RL_H, RL_L, RR_H, RR_L]
        Speed: 0.01 km/h resolution
        """
        fl_clamped = max(DBCRanges.WHEEL_SPEED_MIN, min(DBCRanges.WHEEL_SPEED_MAX, self.chassis.wheel_speed_fl + self.wheel_speed_noise[0]))
        fr_clamped = max(DBCRanges.WHEEL_SPEED_MIN, min(DBCRanges.WHEEL_SPEED_MAX, self.chassis.wheel_speed_fr + self.wheel_speed_noise[1]))
        rl_clamped = max(DBCRanges.WHEEL_SPEED_MIN, min(DBCRanges.WHEEL_SPEED_MAX, self.chassis.wheel_speed_rl + self.wheel_speed_noise[2]))
        rr_clamped = max(DBCRanges.WHEEL_SPEED_MIN, min(DBCRanges.WHEEL_SPEED_MAX, self.chassis.wheel_speed_rr + self.wheel_speed_noise[3]))

        fl = int(fl_clamped * 100)
        fr = int(fr_clamped * 100)
        rl = int(rl_clamped * 100)
        rr = int(rr_clamped * 100)

        return struct.pack('<HHHH', fl, fr, rl, rr)
    
    def encode_brake_pressure(self) -> bytes:
        """
        Encode brake pressure with sensor noise.
        Format: [PRESSURE_H, PRESSURE_L, 0, 0, 0, 0, 0, 0]
        Pressure: 0.1 bar resolution
        """
        pressure_with_noise = self.chassis.brake_pressure + self.brake_pressure_noise
        pressure_clamped = max(DBCRanges.BRAKE_PRESSURE_MIN, min(DBCRanges.BRAKE_PRESSURE_MAX, pressure_with_noise))
        pressure = int(pressure_clamped * 10)

        return struct.pack('<H6x', pressure)

    def encode_steering_angle(self) -> bytes:
        """
        Encode steering wheel angle with sensor noise.
        Format: [ANGLE_H, ANGLE_L, RATE_H, RATE_L, 0, 0, 0, 0]
        Angle: 0.1 degree resolution, signed
        """
        angle_with_noise = self.chassis.steering_angle + self.steering_noise
        angle_clamped = max(DBCRanges.STEERING_ANGLE_MIN, min(DBCRanges.STEERING_ANGLE_MAX, angle_with_noise))
        angle = int(angle_clamped * 10)

        return struct.pack('<h6x', angle)
    
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
        
        return struct.pack('<IH2x', odo, trip)
    
    def encode_fuel(self) -> bytes:
        """
        Encode fuel level and consumption.
        Format: [FUEL_LEVEL, CONSUMPTION_H, CONSUMPTION_L, 0, 0, 0, 0, 0]
        Level: 0-100%
        Consumption: 0.01 L/100km resolution
        """
        level_clamped = max(DBCRanges.FUEL_LEVEL_MIN, min(DBCRanges.FUEL_LEVEL_MAX, self.cluster.fuel_level))
        level = int(level_clamped)

        consumption_clamped = max(DBCRanges.FUEL_CONSUMPTION_MIN, min(DBCRanges.FUEL_CONSUMPTION_MAX, self.cluster.fuel_consumption))
        consumption = int(consumption_clamped * 100)

        return struct.pack('<BH5x', level, consumption)

    # =========================================================================
    # Simulation Update Functions
    # =========================================================================
    
    def update_driving_simulation(self, dt: float):
        """
        Update the driving simulation state.
        Creates realistic acceleration, cruising, and braking patterns.
        """
        self.simulation_time += dt

        # Update sensor noise first
        self._update_noise(dt)

        # Create a realistic driving pattern: accelerate, cruise, brake, repeat
        cycle_time = self.simulation_time % 60

        if cycle_time < 15:
            # Accelerating phase: smooth acceleration from 0 to ~120 km/h
            progress = cycle_time / 15.0
            self.target_speed = 120 * (progress ** 0.7)
            self.acceleration = 2.0
            # Throttle decreases as we approach target speed (realistic driver behavior)
            self.engine.throttle_pos = 70 - progress * 20
            self.brake_pressure_target = 0
        elif cycle_time < 40:
            # Cruising phase: maintain speed with small variations
            variation = 5 * math.sin(cycle_time * 0.5)
            self.target_speed = 110 + variation
            self.acceleration = 0
            self.engine.throttle_pos = 25 + variation * 0.5
            self.brake_pressure_target = 0
        elif cycle_time < 50:
            # Braking phase: smooth deceleration to stop
            progress = (cycle_time - 40) / 10.0
            self.target_speed = 110 * (1 - progress)
            self.acceleration = -3.0
            self.engine.throttle_pos = 0
            # Progressive braking: light at first, harder near the end
            self.brake_pressure_target = 30 + progress * 50
        else:
            # Stopped/idle phase
            self.target_speed = 0
            self.acceleration = 0
            self.engine.throttle_pos = 0
            self.brake_pressure_target = 80 if self.engine.speed_kmh < 1 else 0

        # Smoothly adjust speed toward target (realistic vehicle dynamics)
        speed_diff = self.target_speed - self.engine.speed_kmh
        speed_change_rate = 0.15 if speed_diff > 0 else 0.2
        self.engine.speed_kmh += speed_diff * speed_change_rate
        self.engine.speed_kmh = max(0, self.engine.speed_kmh)

        # Update all vehicle systems
        self._update_engine(dt)
        self._update_transmission(dt)
        self._update_chassis(dt)
        self._update_battery(dt)
        self._update_body(dt)
        self._update_cluster(dt)
    
    def _update_noise(self, dt: float):
        """
        Generate realistic sensor noise using Ornstein-Uhlenbeck process.
        This creates mean-reverting noise that simulates realistic sensor behavior.
        """
        theta = 5.0

        sigma_rpm = 15.0
        self.rpm_noise += theta * (0 - self.rpm_noise) * dt + sigma_rpm * math.sqrt(dt) * random.gauss(0, 1)

        sigma_coolant = 0.3
        self.coolant_temp_noise += theta * (0 - self.coolant_temp_noise) * dt + sigma_coolant * math.sqrt(dt) * random.gauss(0, 1)

        sigma_intake = 0.5
        self.intake_temp_noise += theta * (0 - self.intake_temp_noise) * dt + sigma_intake * math.sqrt(dt) * random.gauss(0, 1)

        sigma_fuel = 3.0
        self.fuel_pressure_noise += theta * (0 - self.fuel_pressure_noise) * dt + sigma_fuel * math.sqrt(dt) * random.gauss(0, 1)

        for i in range(4):
            sigma_wheel = 0.2
            self.wheel_speed_noise[i] += theta * (0 - self.wheel_speed_noise[i]) * dt + sigma_wheel * math.sqrt(dt) * random.gauss(0, 1)

        sigma_brake = 1.5
        self.brake_pressure_noise += theta * (0 - self.brake_pressure_noise) * dt + sigma_brake * math.sqrt(dt) * random.gauss(0, 1)

        sigma_steering = 0.5
        self.steering_noise += theta * (0 - self.steering_noise) * dt + sigma_steering * math.sqrt(dt) * random.gauss(0, 1)

        sigma_voltage = 0.3
        self.battery_voltage_noise += theta * (0 - self.battery_voltage_noise) * dt + sigma_voltage * math.sqrt(dt) * random.gauss(0, 1)

        sigma_current = 0.5
        self.battery_current_noise += theta * (0 - self.battery_current_noise) * dt + sigma_current * math.sqrt(dt) * random.gauss(0, 1)

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
        self.engine.rpm = max(600, min(6500, self.engine.rpm))
        
        # Engine load based on throttle and RPM
        self.engine.engine_load = (self.engine.throttle_pos * 0.7 +
                                   self.engine.rpm / 6500 * 30)
        self.engine.engine_load = min(100, self.engine.engine_load)
        
        # Coolant temperature (warms up over time, stabilizes around 90°C)
        target_temp = 90 + self.engine.engine_load * 0.1
        temp_diff = target_temp - self.engine.coolant_temp
        self.engine.coolant_temp += temp_diff * 0.01

        # Intake temperature varies with ambient and airflow
        self.engine.intake_temp = 25 + math.sin(self.simulation_time * 0.1) * 5

        # Fuel pressure stays relatively constant
        self.engine.fuel_pressure = 350
    
    def _update_transmission(self, dt: float):
        """Update transmission parameters with realistic shift logic."""
        speed = self.engine.speed_kmh

        # Realistic automatic transmission with hysteresis to prevent gear hunting
        # Different shift points for upshifts vs downshifts
        current_gear = self.transmission.gear

        if speed < 3:
            target_gear = 0
        elif speed < 18:
            target_gear = 1 if current_gear <= 1 else (1 if speed < 15 else current_gear)
        elif speed < 38:
            target_gear = 2 if current_gear <= 2 else (2 if speed < 32 else current_gear)
        elif speed < 58:
            target_gear = 3 if current_gear <= 3 else (3 if speed < 50 else current_gear)
        elif speed < 78:
            target_gear = 4 if current_gear <= 4 else (4 if speed < 68 else current_gear)
        elif speed < 98:
            target_gear = 5 if current_gear <= 5 else (5 if speed < 88 else current_gear)
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
        """Update chassis and wheel parameters with realistic physics."""
        base_speed = self.engine.speed_kmh

        # Calculate steering angle first (gentle lane changes)
        base_steering_angle = 30 * math.sin(self.simulation_time * 0.3)

        # All wheels must be close to base speed for realistic driving
        # Front wheels: slightly faster when turning (outside wheel), slower when turning (inside wheel)
        # Rear wheels: nearly identical to base speed
        # The differences are VERY small in normal driving (±0.1-0.3 km/h)

        # Turn-dependent wheel speed differential (Ackermann steering)
        # Outer wheels go slightly faster in turns
        steering_factor = abs(base_steering_angle) / 30.0

        # Front axle differential (steering causes speed difference)
        if base_steering_angle > 0:
            fl_diff = -0.15 * steering_factor
            fr_diff = 0.15 * steering_factor
        else:
            fl_diff = 0.15 * steering_factor
            fr_diff = -0.15 * steering_factor

        # Rear axle stays very close to base speed
        rl_diff = 0.05 * (1 if base_steering_angle > 0 else -1) * steering_factor
        rr_diff = -rl_diff

        # Apply differences to get realistic wheel speeds
        self.chassis.wheel_speed_fl = max(0, base_speed + fl_diff)
        self.chassis.wheel_speed_fr = max(0, base_speed + fr_diff)
        self.chassis.wheel_speed_rl = max(0, base_speed + rl_diff)
        self.chassis.wheel_speed_rr = max(0, base_speed + rr_diff)

        # Store steering angle
        self.chassis.steering_angle = base_steering_angle

        # Update brake pressure with realistic dynamics (smooth transitions)
        pressure_diff = self.brake_pressure_target - self.brake_pressure_actual
        self.brake_pressure_actual += pressure_diff * 5.0 * dt
        self.chassis.brake_pressure = max(0, self.brake_pressure_actual)
    
    def _update_battery(self, dt: float):
        """Update battery/BMS parameters."""
        # Simulate power draw based on speed and load
        power_kw = self.engine.speed_kmh * 0.15 + self.engine.engine_load * 0.5

        # Current draw (P = V * I)
        self.battery.pack_current = power_kw * 1000 / self.battery.pack_voltage

        # SOC slowly decreases
        self.battery.soc -= self.battery.pack_current * dt / 3600 * 0.01
        self.battery.soc = max(0, min(100, self.battery.soc))

        # Pack voltage varies with SOC and current
        self.battery.pack_voltage = 350 + self.battery.soc * 0.8 - self.battery.pack_current * 0.05

        # Temperature rises with current
        target_temp = 25 + abs(self.battery.pack_current) * 0.1
        temp_diff = target_temp - self.battery.pack_temp
        self.battery.pack_temp += temp_diff * 0.01

        # Update cell voltages (small variation between cells for realism)
        base_cell_v = self.battery.pack_voltage / 96
        for i in range(min(8, len(self.battery.cell_voltages))):
            # Each cell varies slightly based on internal resistance
            cell_variance = 0.01 * math.sin(self.simulation_time * 0.1 + i)
            self.battery.cell_voltages[i] = base_cell_v + cell_variance
    
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

    def run(self, update_rate: float = 100.0, print_status: bool = True):
        """
        Run the main simulation loop.

        Args:
            update_rate: Simulation update frequency in Hz
            print_status: Whether to print status to console
        """
        if not self.can_sender:
            raise RuntimeError("CAN sender not configured. Call set_can_sender() first.")

        self.running = True
        dt = 1.0 / update_rate

        # Message transmission rates (in Hz)
        rates = {
            'engine': 100,
            'transmission': 50,
            'bms': 10,
            'chassis': 100,
            'body': 10,
            'cluster': 5,
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
                    if print_status and frame_count % int(update_rate) == 0:
                        self._print_status()
                else:
                    time.sleep(0.0001)

        except KeyboardInterrupt:
            print("\nStopping simulation...")
        finally:
            self.running = False

    def _print_status(self):
        """Print current simulation status to console."""
        # Calculate raw values being sent
        rpm_raw = int((self.engine.rpm + self.rpm_noise) * 4)
        speed_raw = int(self.engine.speed_kmh * 100)

        print(f"\rRPM: {self.engine.rpm:5.0f} (raw: {rpm_raw:5d}, 0x{rpm_raw:04X}) | "
              f"Speed: {self.engine.speed_kmh:5.1f} km/h (raw: {speed_raw:5d}, 0x{speed_raw:04X}) | "
              f"Gear: {self.transmission.gear} | "
              f"CAN ID 0x100", end='')


# =============================================================================
# CAN Interface Adapters
# =============================================================================

def create_python_can_sender(interface: str, channel: str, bitrate: int):
    """
    Create a CAN sender function using python-can library.

    Args:
        interface: CAN interface type ('socketcan', 'pcan', 'vector', etc.)
        channel: CAN channel name
        bitrate: CAN bus bitrate

    Returns:
        Tuple of (sender_function, bus_object)
    """
    try:
        bus = can.Bus(
            interface=interface,
            channel=channel,
            bitrate=bitrate,
            receive_own_messages=True
        )
        print(f"Connected to {interface}:{channel} @ {bitrate} bps")

        def sender(can_id: int, data: bytes):
            msg = can.Message(
                arbitration_id=can_id,
                data=data,
                is_extended_id=False
            )
            try:
                bus.send(msg)
            except can.CanError as e:
                print(f"Error sending frame 0x{can_id:03X}: {e}")

        return sender, bus

    except Exception as e:
        print(f"Failed to connect to {interface}:{channel}")
        print(f"Error: {e}")
        print("\nTroubleshooting:")
        if interface == "socketcan":
            print("  - Linux: Ensure vcan0 is set up (see README.md)")
            print("  - macOS/Windows: SocketCAN not supported, use hardware interface")
        elif interface == "pcan":
            print("  - Ensure PEAK PCAN drivers are installed")
            print("  - Check that PCAN hardware is connected")
        elif interface == "vector":
            print("  - Ensure Vector CAN drivers are installed")
            print("  - Check that Vector hardware is connected")
        return None, None


def create_virtual_can_sender(host: str, port: int, channel: str):
    """
    Create a CAN sender function using Qt VirtualCAN.

    Args:
        host: VirtualCAN server host
        port: VirtualCAN server port
        channel: Virtual CAN channel name

    Returns:
        Tuple of (sender_function, client_object)
    """
    client = QtVirtualCANClient(host=host, port=port, channel=channel)
    if not client.connect():
        return None, None

    print(f"Connected to Qt VirtualCAN: {host}:{port}/{channel}")
    return client.send_frame, client


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
    parser.add_argument(
        '--debug',
        action='store_true',
        help='Enable debug output showing raw CAN frame data'
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

    # Create simulator instance
    simulator = ECUSimulator(debug=args.debug)

    # Create CAN sender based on interface type
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
        print()

        sender, client = create_virtual_can_sender('localhost', 35468, args.channel)
        if not sender:
            sys.exit(1)

        simulator.set_can_sender(sender)

        print(f"Starting ECU simulation at {args.rate} Hz")
        print("Press Ctrl+C to stop")
        print("-" * 60)
        print()

        try:
            simulator.run(update_rate=args.rate)
        finally:
            client.disconnect()

    else:
        print("=" * 60)
        print("CAN ECU Simulator for Serial Studio")
        print("=" * 60)
        print(f"Platform:  {platform.system()}")
        print(f"Interface: {args.interface}")
        print(f"Channel:   {args.channel}")
        print(f"Bitrate:   {args.bitrate} bps")
        print(f"Rate:      {args.rate} Hz")
        print("=" * 60)
        print()

        sender, bus = create_python_can_sender(args.interface, args.channel, args.bitrate)
        if not sender:
            sys.exit(1)

        simulator.set_can_sender(sender)

        print(f"Starting ECU simulation at {args.rate} Hz")
        print("Press Ctrl+C to stop")
        print("-" * 60)
        print()

        try:
            simulator.run(update_rate=args.rate)
        finally:
            if bus:
                bus.shutdown()
                print("Disconnected from CAN bus")


if __name__ == '__main__':
    main()
