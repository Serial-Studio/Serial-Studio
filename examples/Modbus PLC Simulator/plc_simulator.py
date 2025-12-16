#!/usr/bin/env python3
"""
Hydraulic Test Stand Simulator - Modbus TCP Server
===================================================

Physics-based simulation for Serial Studio demonstration:
- 50HP motor with VFD soft-start (0-1800 RPM in 10s)
- 45cc/rev 9-piston axial pump
- PID pressure control (target: 1500 PSI)
- Thermodynamic oil temperature model
- ISO 10816 vibration model
- Realistic failure modes (cavitation, overpressure)

Modbus TCP Server: 0.0.0.0:5020

Register Map (Holding Registers FC03):
  HR[0]  E-Stop        0/1
  HR[1]  Start LED     0/1 (off during E-Stop)
  HR[2]  Temperature   72-180 °F
  HR[3]  Pressure      0-3000 PSI (alarm: 2800)
  HR[4]  Motor RPM     0-3600
  HR[5]  Valve Pos     0-100 %
  HR[6]  Flow Rate     ×10 → 0-50.0 GPM
  HR[7]  Motor Load    0-100 %
  HR[8]  Vibration     ×10 → 0-15.0 mm/s

Usage: python plc_simulator.py
Requires: pip install pymodbus
"""

import asyncio
import random
import math
import logging
import sys

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s %(message)s',
    datefmt='%H:%M:%S'
)
log = logging.getLogger(__name__)

# =========================================================================
# Configuration
# =========================================================================

NUM_REGISTERS = 100
UPDATE_INTERVAL = 0.05  # 50ms (20 Hz) for smooth animation
SERVER_HOST = "0.0.0.0"
SERVER_PORT = 5020

# =========================================================================
# Physical Constants & System Parameters
# =========================================================================

# Motor parameters (typical 50HP hydraulic power unit)
MOTOR_MAX_RPM = 3600          # Synchronous speed (2-pole @ 60Hz)
MOTOR_RATED_RPM = 1800        # Typical operating speed (4-pole)
MOTOR_INERTIA = 2.5           # kg·m² (rotor + pump inertia)
MOTOR_DAMPING = 0.15          # Viscous damping coefficient
MOTOR_TORQUE_CONST = 25.0     # N·m per unit drive signal
STARTUP_RAMP_TIME = 10.0      # VFD ramp time in seconds

# Hydraulic pump parameters (axial piston pump)
PUMP_DISPLACEMENT = 45.0      # cc/rev
PUMP_VOLUMETRIC_EFF = 0.92    # Typical efficiency
PUMP_MECH_EFF = 0.94          # Mechanical efficiency

# System hydraulic parameters
SYSTEM_VOLUME = 5.0           # Liters (accumulator + lines)
BULK_MODULUS = 1.5e9          # Pa (hydraulic fluid)
RELIEF_VALVE_PSI = 3000       # Relief valve setting
ALARM_PRESSURE_PSI = 2800     # High pressure alarm
TARGET_PRESSURE_PSI = 1500    # Normal operating pressure
MIN_PRESSURE_PSI = 0

# Control valve parameters (proportional valve)
VALVE_CV = 4.5                # Flow coefficient
VALVE_RESPONSE_TIME = 0.3     # Seconds (first-order lag)

# Thermal parameters
AMBIENT_TEMP_F = 72           # °F
OIL_SPECIFIC_HEAT = 1900      # J/(kg·K)
OIL_MASS = 15.0               # kg in system
HEAT_TRANSFER_COEFF = 25.0    # W/K (to ambient via reservoir)
MAX_OIL_TEMP_F = 180          # Max safe operating temp

# PID gains (tuned for this system)
KP = 0.8
KI = 0.3
KD = 0.05

# Failure parameters
FAILURE_PROBABILITY = 0.0002  # ~0.02% per update (~once per 4 minutes avg)
FAILURE_DURATION = 3.0        # Seconds of instability before E-STOP
ESTOP_DURATION = 5.0          # E-STOP active time

# Valve cycling for load simulation
VALVE_CYCLE_PERIOD = 20.0     # Seconds per cycle
VALVE_MIN_POS = 30            # Minimum valve opening %
VALVE_MAX_POS = 70            # Maximum valve opening %

# =========================================================================
# Utility Functions
# =========================================================================

def psi_to_pa(psi: float) -> float:
    """Convert PSI to Pascals."""
    return psi * 6894.76

def pa_to_psi(pa: float) -> float:
    """Convert Pascals to PSI."""
    return pa / 6894.76

def f_to_c(f: float) -> float:
    """Convert Fahrenheit to Celsius."""
    return (f - 32) * 5/9

def c_to_f(c: float) -> float:
    """Convert Celsius to Fahrenheit."""
    return c * 9/5 + 32

def clamp(value: float, min_val: float, max_val: float) -> float:
    """Clamp value between min and max."""
    return max(min_val, min(max_val, value))

# =========================================================================
# PID Controller with Anti-Windup
# =========================================================================

class PIDController:
    """Industrial PID controller with anti-windup and derivative filtering."""
    
    def __init__(self, kp: float, ki: float, kd: float, setpoint: float,
                 output_min: float = 0, output_max: float = MOTOR_MAX_RPM):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.setpoint = setpoint
        self.output_min = output_min
        self.output_max = output_max
        
        self.integral = 0.0
        self.last_error = 0.0
        self.last_derivative = 0.0
        self.derivative_filter = 0.1  # Low-pass filter coefficient
        
    def update(self, measured: float, dt: float) -> float:
        """Calculate PID output with filtering."""
        error = self.setpoint - measured
        
        # Proportional
        p_term = self.kp * error
        
        # Integral with anti-windup (only integrate when not saturated)
        self.integral += error * dt
        self.integral = clamp(self.integral, -200, 200)
        i_term = self.ki * self.integral
        
        # Filtered derivative (reduces noise sensitivity)
        if dt > 0:
            raw_derivative = (error - self.last_error) / dt
            self.last_derivative += self.derivative_filter * (raw_derivative - self.last_derivative)
        d_term = self.kd * self.last_derivative
        self.last_error = error
        
        # Output centered around rated RPM
        output = MOTOR_RATED_RPM + p_term + i_term + d_term
        return clamp(output, self.output_min, self.output_max)
    
    def reset(self):
        """Reset controller state."""
        self.integral = 0.0
        self.last_error = 0.0
        self.last_derivative = 0.0

# =========================================================================
# Hydraulic System Physics Model
# =========================================================================

class HydraulicTestStand:
    """Physics-based hydraulic test stand simulation."""
    
    def __init__(self):
        # State variables
        self.motor_rpm = 0.0              # Current motor speed
        self.motor_target_rpm = 0.0       # Target from VFD/PID
        self.pressure_pa = 101325         # System pressure (start at 1 atm)
        self.oil_temp_c = f_to_c(AMBIENT_TEMP_F)
        self.valve_position = 50.0        # Commanded position %
        self.valve_actual = 50.0          # Actual position (with lag)
        
        # New measurements
        self.flow_gpm = 0.0               # Flow rate in GPM
        self.motor_load_pct = 0.0         # Motor load as percentage
        self.vibration_mm_s = 0.0         # Vibration velocity mm/s RMS
        
        # Status
        self.emergency_stop = 0
        self.start_led = 0
        self.motor_running = False
        
        # Simulation state
        self.runtime = 0.0
        self.phase_timer = 0.0
        self.test_phase = "STARTUP"
        
        # Failure state
        self.failure_timer = 0.0
        self.pre_failure_rpm = 0.0
        self.pre_failure_pressure = 0.0
        self.in_failure = False           # Track if currently in failure
        
        # PID controller
        self.pid = PIDController(KP, KI, KD, TARGET_PRESSURE_PSI)
        
        # Noise generators (filtered for realism)
        self.rpm_noise = 0.0
        self.pressure_noise = 0.0
        self.temp_noise = 0.0
        self.flow_noise = 0.0
        self.vibration_noise = 0.0
        
    def update(self, dt: float):
        """Main physics update loop."""
        self.runtime += dt
        self.phase_timer += dt
        
        # State machine
        if self.test_phase == "STARTUP":
            self._startup_sequence(dt)
        elif self.test_phase == "RUNNING":
            self._running_sequence(dt)
        elif self.test_phase == "PRESSURE_TEST":
            self._pressure_test_sequence(dt)
        elif self.test_phase == "FAILURE":
            self._failure_sequence(dt)
        elif self.test_phase == "SHUTDOWN":
            self._shutdown_sequence(dt)
        elif self.test_phase == "RESTART_WAIT":
            self._restart_wait_sequence(dt)
            
        # Update filtered noise (Ornstein-Uhlenbeck process for realistic sensor noise)
        self._update_noise(dt)
        
        # Update start LED - ON only when running AND not in E-STOP
        self.start_led = 1 if (self.motor_rpm > 50 and self.emergency_stop == 0) else 0
        self.motor_running = self.motor_rpm > 10
        
    def _update_noise(self, dt: float):
        """Generate realistic sensor noise using Ornstein-Uhlenbeck process."""
        # Mean-reverting noise for each sensor
        theta = 5.0  # Mean reversion rate
        
        # RPM noise: ±5 RPM typical
        sigma_rpm = 3.0
        self.rpm_noise += theta * (0 - self.rpm_noise) * dt + sigma_rpm * math.sqrt(dt) * random.gauss(0, 1)
        
        # Pressure noise: ±3-5 PSI typical
        sigma_pressure = 2.0
        self.pressure_noise += theta * (0 - self.pressure_noise) * dt + sigma_pressure * math.sqrt(dt) * random.gauss(0, 1)
        self.pressure_noise = clamp(self.pressure_noise, -8, 8)  # Hard limit noise
        
        # Temperature noise: ±0.3°F typical
        sigma_temp = 0.2
        self.temp_noise += theta * (0 - self.temp_noise) * dt + sigma_temp * math.sqrt(dt) * random.gauss(0, 1)
        
        # Flow noise: ±0.2 GPM typical
        sigma_flow = 0.15
        self.flow_noise += theta * (0 - self.flow_noise) * dt + sigma_flow * math.sqrt(dt) * random.gauss(0, 1)
        
        # Vibration noise: ±0.3 mm/s typical
        sigma_vib = 0.2
        self.vibration_noise += theta * (0 - self.vibration_noise) * dt + sigma_vib * math.sqrt(dt) * random.gauss(0, 1)
        
    def _update_motor_physics(self, dt: float, target_rpm: float):
        """
        Update motor RPM using realistic dynamics.
        Models VFD-controlled induction motor with load.
        """
        # Motor torque based on error from target (simplified VFD model)
        rpm_error = target_rpm - self.motor_rpm
        
        # First-order response with time constant based on inertia
        # Typical VFD acceleration: 0-1800 RPM in ~5-10 seconds under load
        time_constant = MOTOR_INERTIA / MOTOR_DAMPING
        
        # Acceleration limited by available torque
        max_accel = 400  # RPM/s maximum
        accel = clamp(rpm_error / time_constant, -max_accel, max_accel)
        
        self.motor_rpm += accel * dt
        self.motor_rpm = clamp(self.motor_rpm, 0, MOTOR_MAX_RPM)
        
    def _update_hydraulic_physics(self, dt: float):
        """
        Update pressure using fluid dynamics with damping.
        
        Simplified model for stability:
        - Pump adds flow proportional to RPM
        - Valve removes flow proportional to opening and sqrt(pressure)
        - Pressure changes smoothly with time constant
        """
        # Update flow and load first
        self._update_flow_and_load()
        
        # Target pressure based on flow balance
        # More flow with valve closed = higher pressure
        valve_open = self.valve_actual / 100.0
        flow_restriction = 1.0 - valve_open * 0.6  # 0.4 to 1.0
        
        # Pressure proportional to RPM squared (pump affinity laws) and restriction
        rpm_ratio = self.motor_rpm / MOTOR_RATED_RPM if MOTOR_RATED_RPM > 0 else 0
        target_pressure_psi = TARGET_PRESSURE_PSI * (rpm_ratio ** 1.8) * flow_restriction
        
        # Add small oscillation from pump pulsation (9-piston pump = 9x per rev)
        if self.motor_rpm > 100:
            piston_freq = self.motor_rpm / 60.0 * 9  # Hz
            pulsation = math.sin(self.runtime * 2 * math.pi * piston_freq) * 15
            pulsation += math.sin(self.runtime * 2 * math.pi * piston_freq * 2) * 5  # Harmonic
        else:
            pulsation = 0
            
        target_pressure_psi += pulsation
        
        # Smooth pressure changes with first-order lag (hydraulic capacitance)
        # Time constant ~0.3 seconds for realistic response
        tau = 0.3
        current_psi = pa_to_psi(self.pressure_pa)
        new_psi = current_psi + (target_pressure_psi - current_psi) * (dt / tau)
        
        # Relief valve (hard limit)
        new_psi = clamp(new_psi, 0, RELIEF_VALVE_PSI)
        
        self.pressure_pa = psi_to_pa(new_psi)
        
        # Update vibration
        self._update_vibration()
        
    def _update_flow_and_load(self):
        """Calculate flow rate and motor load percentage."""
        # Pump flow (L/min) = displacement * RPM * efficiency / 1000
        pump_flow_lpm = (PUMP_DISPLACEMENT * self.motor_rpm * PUMP_VOLUMETRIC_EFF) / 1000.0
        
        # Convert to GPM (1 LPM = 0.264172 GPM)
        self.flow_gpm = pump_flow_lpm * 0.264172
        
        # Calculate motor load percentage
        # Load = (actual pressure / max pressure) * (RPM / rated RPM) * 100
        # Plus mechanical losses
        current_psi = pa_to_psi(self.pressure_pa)
        pressure_load = current_psi / RELIEF_VALVE_PSI if RELIEF_VALVE_PSI > 0 else 0
        speed_factor = self.motor_rpm / MOTOR_RATED_RPM if MOTOR_RATED_RPM > 0 else 0
        mechanical_losses = 0.15  # 15% baseline mechanical load when running
        
        if self.motor_rpm > 50:
            self.motor_load_pct = (pressure_load * speed_factor * 0.85 + mechanical_losses) * 100
        else:
            self.motor_load_pct = 0
            
        self.motor_load_pct = clamp(self.motor_load_pct, 0, 100)
        
    def _update_thermal_physics(self, dt: float):
        """
        Update oil temperature using thermodynamics.
        
        Heat sources:
          - Pump inefficiency: Q = P * (1 - η)
          - Valve throttling losses
          
        Heat removal:
          - Convection to ambient via reservoir
        """
        # Power into system (W) = pressure * flow / efficiency
        pump_flow_m3 = (PUMP_DISPLACEMENT * self.motor_rpm * PUMP_VOLUMETRIC_EFF) / 60.0 * 1e-6
        hydraulic_power = self.pressure_pa * pump_flow_m3
        
        # Heat generated by inefficiencies
        pump_heat = hydraulic_power * (1 - PUMP_MECH_EFF)
        
        # Valve throttling losses (proportional to flow * pressure drop)
        valve_heat = hydraulic_power * (1 - self.valve_actual / 100) * 0.3
        
        total_heat = pump_heat + valve_heat
        
        # Heat loss to ambient (Newton's law of cooling)
        ambient_c = f_to_c(AMBIENT_TEMP_F)
        heat_loss = HEAT_TRANSFER_COEFF * (self.oil_temp_c - ambient_c)
        
        # Temperature change: dT = Q * dt / (m * Cp)
        net_heat = total_heat - heat_loss
        dT = (net_heat * dt) / (OIL_MASS * OIL_SPECIFIC_HEAT)
        
        self.oil_temp_c += dT
        self.oil_temp_c = clamp(self.oil_temp_c, ambient_c, f_to_c(MAX_OIL_TEMP_F + 40))
        
    def _update_vibration(self):
        """
        Calculate pump/motor vibration in mm/s RMS.
        
        ISO 10816 vibration severity zones for Class II machines:
          - Good:        < 2.8 mm/s
          - Acceptable:  2.8 - 7.1 mm/s
          - Unsatisfactory: 7.1 - 11.2 mm/s
          - Unacceptable: > 11.2 mm/s
        
        Vibration sources:
          - Rotational imbalance (1x RPM)
          - Bearing defects (higher harmonics)
          - Pump pulsation (Nx RPM where N = pistons)
          - Cavitation (broadband, high amplitude)
        """
        if self.motor_rpm < 50:
            self.vibration_mm_s = 0.1  # Residual/noise floor
            return
            
        # Base vibration from rotation (proportional to RPM^1.5 for imbalance)
        rpm_factor = (self.motor_rpm / MOTOR_RATED_RPM) ** 1.5
        base_vibration = 1.5 * rpm_factor  # ~1.5 mm/s at rated speed
        
        # Load-induced vibration (hydraulic forces)
        load_factor = self.motor_load_pct / 100.0
        load_vibration = 0.8 * load_factor
        
        # Pump pulsation contribution
        pulsation_vibration = 0.5 * rpm_factor
        
        # Sum components
        self.vibration_mm_s = base_vibration + load_vibration + pulsation_vibration
        
        # During failure, vibration spikes (cavitation causes high vibration)
        if self.in_failure:
            # Cavitation adds significant broadband vibration
            cavitation_vib = 4.0 + 2.0 * math.sin(self.failure_timer * 30)
            self.vibration_mm_s += cavitation_vib
            
        # Clamp to realistic range
        self.vibration_mm_s = clamp(self.vibration_mm_s, 0, 15.0)
        
    def _update_valve_dynamics(self, dt: float, target_position: float):
        """Update valve position with first-order lag (hydraulic valve response)."""
        # First-order lag: τ * dy/dt + y = u
        error = target_position - self.valve_actual
        self.valve_actual += (error / VALVE_RESPONSE_TIME) * dt
        self.valve_actual = clamp(self.valve_actual, 0, 100)
        
    def _startup_sequence(self, dt: float):
        """VFD soft-start ramp to rated speed."""
        self.emergency_stop = 0
        
        if self.phase_timer < STARTUP_RAMP_TIME:
            # S-curve ramp for smooth acceleration
            progress = self.phase_timer / STARTUP_RAMP_TIME
            # Smoothstep function: 3x² - 2x³
            smooth = progress * progress * (3 - 2 * progress)
            self.motor_target_rpm = MOTOR_RATED_RPM * smooth
            
            self._update_motor_physics(dt, self.motor_target_rpm)
            self._update_valve_dynamics(dt, 50)  # Valve at mid-position
            self._update_hydraulic_physics(dt)
            self._update_thermal_physics(dt)
        else:
            self.test_phase = "RUNNING"
            self.phase_timer = 0.0
            self.pid.reset()
            log.info("=" * 50)
            log.info(">>> STARTUP COMPLETE - System online")
            log.info("=" * 50)
            
    def _running_sequence(self, dt: float):
        """Normal operation with PID control and valve cycling."""
        self.emergency_stop = 0
        
        # Random failure check (very rare)
        if random.random() < FAILURE_PROBABILITY:
            self._trigger_failure()
            return
            
        # Cycle valve to simulate varying load
        valve_cycle = math.sin(self.runtime * 2 * math.pi / VALVE_CYCLE_PERIOD)
        valve_range = (VALVE_MAX_POS - VALVE_MIN_POS) / 2
        valve_center = (VALVE_MAX_POS + VALVE_MIN_POS) / 2
        target_valve = valve_center + valve_range * valve_cycle
        
        # PID controls motor speed to maintain pressure
        current_psi = pa_to_psi(self.pressure_pa)
        self.motor_target_rpm = self.pid.update(current_psi, dt)
        
        # Update all physics
        self._update_motor_physics(dt, self.motor_target_rpm)
        self._update_valve_dynamics(dt, target_valve)
        self._update_hydraulic_physics(dt)
        self._update_thermal_physics(dt)
        
        # Pressure test every 60 seconds
        if self.phase_timer >= 60.0:
            self.test_phase = "PRESSURE_TEST"
            self.phase_timer = 0.0
            log.info(">>> PRESSURE TEST - Increasing system pressure")
            
    def _pressure_test_sequence(self, dt: float):
        """High pressure test - close valve partially, increase RPM."""
        self.emergency_stop = 0
        
        # Higher failure probability during stress test
        if random.random() < FAILURE_PROBABILITY * 2:
            self._trigger_failure()
            return
            
        if self.phase_timer < 8.0:
            # Gradually close valve and increase RPM
            progress = min(1.0, self.phase_timer / 3.0)
            target_valve = 50 - progress * 30  # Close to 20%
            
            # Ramp up RPM
            target_rpm = MOTOR_RATED_RPM + progress * (MOTOR_MAX_RPM - MOTOR_RATED_RPM) * 0.7
            
            self._update_motor_physics(dt, target_rpm)
            self._update_valve_dynamics(dt, target_valve)
            self._update_hydraulic_physics(dt)
            self._update_thermal_physics(dt)
        else:
            self.test_phase = "RUNNING"
            self.phase_timer = 0.0
            self.pid.reset()
            log.info(">>> PRESSURE TEST COMPLETE")
            
    def _trigger_failure(self):
        """Initiate failure sequence - simulates pump cavitation or seal leak."""
        self.test_phase = "FAILURE"
        self.phase_timer = 0.0
        self.failure_timer = 0.0
        self.pre_failure_rpm = self.motor_rpm
        self.pre_failure_pressure = self.pressure_pa
        self.in_failure = True
        log.info("!" * 50)
        log.info("!!! FAULT DETECTED - Pump instability !!!")
        log.info("!" * 50)
        
    def _failure_sequence(self, dt: float):
        """
        Realistic failure: pump cavitation causes RPM and pressure instability.
        Motor wiggles ±50 RPM, pressure rises toward relief valve.
        """
        self.failure_timer += dt
        
        if self.failure_timer < FAILURE_DURATION:
            # Cavitation causes erratic torque - small RPM oscillations
            wiggle_freq1 = 12.0
            wiggle_freq2 = 19.0
            
            # RPM oscillation: ±30-50 RPM (realistic for cavitation)
            rpm_wiggle = (
                math.sin(self.failure_timer * 2 * math.pi * wiggle_freq1) * 30 +
                math.sin(self.failure_timer * 2 * math.pi * wiggle_freq2) * 20
            )
            
            # Motor tries to maintain speed but struggles
            target_rpm = self.pre_failure_rpm + rpm_wiggle
            self._update_motor_physics(dt, target_rpm)
            
            # Pressure builds gradually toward alarm level
            progress = self.failure_timer / FAILURE_DURATION
            target_psi = pa_to_psi(self.pre_failure_pressure) + progress * 800  # Rise ~800 PSI
            
            # Small pressure oscillation from relief valve chatter
            pressure_wiggle = (
                math.sin(self.failure_timer * 2 * math.pi * 6) * 25 +
                math.sin(self.failure_timer * 2 * math.pi * 10) * 15
            )
            
            # Smooth transition
            current_psi = pa_to_psi(self.pressure_pa)
            new_psi = current_psi + (target_psi - current_psi) * dt * 2  # Smooth rise
            new_psi += pressure_wiggle
            new_psi = clamp(new_psi, 0, RELIEF_VALVE_PSI)
            self.pressure_pa = psi_to_pa(new_psi)
            
            # Valve goes slightly erratic
            valve_wiggle = math.sin(self.failure_timer * 15) * 3
            self._update_valve_dynamics(dt, 50 + valve_wiggle)
            
            # Update flow and load based on current state
            self._update_flow_and_load()
            
            # Update vibration (will spike due to in_failure flag)
            self._update_vibration()
            
            # Temperature rises faster during fault
            self._update_thermal_physics(dt)
            self.oil_temp_c += 0.3 * dt  # Extra heat from cavitation
            
            # Log alarm if pressure exceeds threshold
            current_psi = pa_to_psi(self.pressure_pa)
            if current_psi >= ALARM_PRESSURE_PSI:
                log.info(f"!!! HIGH PRESSURE ALARM: {current_psi:.0f} PSI !!!")
        else:
            # Trigger E-STOP
            self.emergency_stop = 1
            self.test_phase = "SHUTDOWN"
            self.phase_timer = 0.0
            log.info(">>> E-STOP ACTIVATED - Initiating controlled shutdown")
            
    def _shutdown_sequence(self, dt: float):
        """Controlled shutdown with E-STOP active."""
        self.emergency_stop = 1
        self.in_failure = False  # Clear failure flag during shutdown
        
        # Motor coasts down (VFD in freewheel or controlled decel)
        # Deceleration rate ~200-300 RPM/s typical
        if self.motor_rpm > 0:
            decel_rate = 250  # RPM/s
            self.motor_rpm -= decel_rate * dt
            self.motor_rpm = max(0, self.motor_rpm)
            
        # Pressure bleeds through valve (opened to tank)
        self._update_valve_dynamics(dt, 80)  # Open valve to dump pressure
        
        # Pressure decay (exponential)
        if self.pressure_pa > 101325:
            # Time constant ~2-3 seconds for pressure bleed
            tau = 2.0
            self.pressure_pa = 101325 + (self.pressure_pa - 101325) * math.exp(-dt / tau)
        
        # Update flow, load, and vibration
        self._update_flow_and_load()
        self._update_vibration()
            
        # Temperature continues to cool
        self._update_thermal_physics(dt)
        
        # Check if shutdown complete
        if self.phase_timer >= ESTOP_DURATION and self.motor_rpm < 5:
            self.motor_rpm = 0
            self.test_phase = "RESTART_WAIT"
            self.phase_timer = 0.0
            log.info(">>> SHUTDOWN COMPLETE - System safe")
            
    def _restart_wait_sequence(self, dt: float):
        """Hold in E-STOP state before auto-restart."""
        self.emergency_stop = 1
        self.motor_rpm = 0
        
        # Zero out readings when stopped
        self._update_flow_and_load()
        self._update_vibration()
        
        # Continue cooling
        self._update_thermal_physics(dt)
        
        # Valve returns to center
        self._update_valve_dynamics(dt, 50)
        
        # Pressure at atmospheric
        self.pressure_pa = 101325 + (self.pressure_pa - 101325) * 0.95
        
        # Auto restart after delay
        if self.phase_timer >= 3.0:
            self.emergency_stop = 0
            self.test_phase = "STARTUP"
            self.phase_timer = 0.0
            self.in_failure = False
            self.pid.reset()
            log.info("=" * 50)
            log.info(">>> FAULT CLEARED - Restarting system...")
            log.info("=" * 50)
            
    def get_registers(self) -> list:
        """Return current state as Modbus register values with sensor noise."""
        # Apply realistic sensor noise to outputs
        rpm_reading = self.motor_rpm + self.rpm_noise
        pressure_reading = pa_to_psi(self.pressure_pa) + self.pressure_noise
        temp_reading = c_to_f(self.oil_temp_c) + self.temp_noise
        flow_reading = self.flow_gpm + self.flow_noise
        vibration_reading = self.vibration_mm_s + self.vibration_noise
        
        return [
            int(self.emergency_stop),                                        # HR[0]: E-Stop
            int(self.start_led),                                             # HR[1]: Start LED
            int(round(clamp(temp_reading, AMBIENT_TEMP_F, MAX_OIL_TEMP_F + 20))),  # HR[2]: Temp °F
            int(round(clamp(pressure_reading, 0, RELIEF_VALVE_PSI))),        # HR[3]: Pressure PSI
            int(round(clamp(rpm_reading, 0, MOTOR_MAX_RPM))),                # HR[4]: Motor RPM
            int(round(clamp(self.valve_actual, 0, 100))),                    # HR[5]: Valve %
            int(round(clamp(flow_reading * 10, 0, 500))),                    # HR[6]: Flow GPM x10
            int(round(clamp(self.motor_load_pct, 0, 100))),                  # HR[7]: Load %
            int(round(clamp(vibration_reading * 10, 0, 150))),               # HR[8]: Vibration mm/s x10
            0                                                                 # HR[9]: Spare
        ]

# =========================================================================
# pymodbus setup (version-agnostic)
# =========================================================================

try:
    import pymodbus
    PYMODBUS_VERSION = pymodbus.__version__
    log.info(f"pymodbus version: {PYMODBUS_VERSION}")
except:
    PYMODBUS_VERSION = "unknown"

from pymodbus.datastore import ModbusSequentialDataBlock, ModbusServerContext

try:
    from pymodbus.datastore import ModbusDeviceContext as SlaveContext
    USE_DEVICES_PARAM = True
    log.info("Using pymodbus 4.x API")
except ImportError:
    from pymodbus.datastore import ModbusSlaveContext as SlaveContext
    USE_DEVICES_PARAM = False
    log.info("Using pymodbus 3.x API")

from pymodbus.server import StartAsyncTcpServer

# =========================================================================
# Background Task
# =========================================================================

async def update_simulation(store: SlaveContext, sim: HydraulicTestStand):
    """High-frequency physics update loop."""
    log_counter = 0
    while True:
        try:
            sim.update(UPDATE_INTERVAL)
            store.setValues(3, 0, sim.get_registers())
            
            # Log every 500ms (compact 80-char format)
            log_counter += 1
            if log_counter >= 10:
                log_counter = 0
                psi = pa_to_psi(sim.pressure_pa)
                temp_f = c_to_f(sim.oil_temp_c)
                
                # Status flags
                flags = ""
                if sim.emergency_stop:
                    flags += "E"
                if psi >= ALARM_PRESSURE_PSI:
                    flags += "A"
                if sim.in_failure:
                    flags += "F"
                flags = flags or "-"
                
                # Compact log: [PHASE] FLAGS RPM PSI GPM L% V T
                log.info(
                    f"[{sim.test_phase[:8]:8}] {flags:3} "
                    f"R:{sim.motor_rpm:4.0f} P:{psi:4.0f} "
                    f"F:{sim.flow_gpm:4.1f} L:{sim.motor_load_pct:2.0f} "
                    f"V:{sim.vibration_mm_s:3.1f} T:{temp_f:3.0f}"
                )
        except Exception as e:
            log.error(f"Update error: {e}")
            
        await asyncio.sleep(UPDATE_INTERVAL)

# =========================================================================
# Server
# =========================================================================

async def run_server():
    sim = HydraulicTestStand()
    
    store = SlaveContext(
        di=ModbusSequentialDataBlock(0, [0] * NUM_REGISTERS),
        co=ModbusSequentialDataBlock(0, [0] * NUM_REGISTERS),
        hr=ModbusSequentialDataBlock(0, [0] * NUM_REGISTERS),
        ir=ModbusSequentialDataBlock(0, [0] * NUM_REGISTERS),
    )
    
    if USE_DEVICES_PARAM:
        context = ModbusServerContext(devices=store, single=True)
    else:
        context = ModbusServerContext(slaves=store, single=True)
        
    asyncio.create_task(update_simulation(store, sim))
    
    print()
    print("=" * 72)
    print("  HYDRAULIC TEST STAND SIMULATOR")
    print("=" * 72)
    print(f"  Server: {SERVER_HOST}:{SERVER_PORT}  |  Update: {UPDATE_INTERVAL*1000:.0f}ms ({1/UPDATE_INTERVAL:.0f}Hz)")
    print()
    print("  Registers (FC03 Holding):")
    print("    0:E-Stop  1:Start  2:Temp(°F)  3:PSI  4:RPM  5:Valve(%)")
    print("    6:GPM(÷10)  7:Load(%)  8:Vibration(÷10 mm/s)")
    print()
    print("  Phases: STARTUP → RUNNING → PRESSURE_TEST → (FAILURE → SHUTDOWN)")
    print("  Flags:  E=E-Stop  A=Alarm(>2800PSI)  F=Failure")
    print("=" * 72)
    print()
    
    await StartAsyncTcpServer(context=context, address=(SERVER_HOST, SERVER_PORT))

if __name__ == "__main__":
    try:
        asyncio.run(run_server())
    except KeyboardInterrupt:
        print("\n>>> Shutdown complete.")
    except Exception as e:
        log.error(f"Server error: {e}")
        sys.exit(1)
