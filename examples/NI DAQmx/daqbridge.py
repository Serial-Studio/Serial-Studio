"""
DAQBridge - NI DAQmx Data Acquisition and UDP Publisher
Works with any NI DAQ device supporting analog input (USB-600x, USB-621x, PCIe-6xxx, etc.)
"""

import time
import socket
import signal
import nidaqmx
import numpy as np

from collections import deque
from nidaqmx.stream_readers import AnalogMultiChannelReader
from nidaqmx.constants import AcquisitionType, TerminalConfiguration

# =============================================================================
# GLOBAL CONFIGURATION
# =============================================================================

DEVICE_NAME = "Dev1"
SAMPLE_RATE = 3000.0
SAMPLES_PER_READ = 1

# -----------------------------------------------------------------------------
# CHANNEL CONFIGURATION
# -----------------------------------------------------------------------------
# type: "voltage" or "current"
# voltage_range: (-10, 10), (-5, 5), (-2, 2), (-1, 1)
# terminal: "RSE" or "DIFF"
# shunt_resistor: resistance in ohms (only for current type, I = V / R)

CHANNEL_CONFIG = [
    {"channel": "ai0", "type": "voltage", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Voltage_0"},
    {"channel": "ai1", "type": "voltage", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Voltage_1"},
    {"channel": "ai2", "type": "current", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Current_0", "shunt_resistor": 500},
    {"channel": "ai3", "type": "current", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Current_1", "shunt_resistor": 500},
    {"channel": "ai4", "type": "voltage", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Voltage_2"},
    {"channel": "ai5", "type": "voltage", "voltage_range": (-10.0, 10.0), "terminal": "RSE", "name": "Voltage_3"},
]

# -----------------------------------------------------------------------------
# UDP CONFIGURATION
# -----------------------------------------------------------------------------

UDP_HOST = "127.0.0.1"
UDP_PORT = 9000

# -----------------------------------------------------------------------------
# OUTPUT CONFIGURATION
# -----------------------------------------------------------------------------

CSV_PRECISION = 6
PRINT_STATS_INTERVAL = 5.0
ENABLE_STATS = True

# =============================================================================
# TERMINAL CONFIG MAPPING
# =============================================================================

TERMINAL_MAP = {
    "RSE": TerminalConfiguration.RSE,
    "DIFF": TerminalConfiguration.DIFF,
}

# =============================================================================
# CUSTOM PROCESSING FUNCTION
# =============================================================================

def process_readings(raw_data: np.ndarray, channel_config: list) -> np.ndarray:
    """
    Process raw voltage readings and return values for CSV output.

    Args:
        raw_data: numpy array of shape (num_channels, num_samples) with raw voltage readings
        channel_config: list of channel configuration dictionaries

    Returns:
        numpy array of shape (num_channels, num_samples) with processed values

    Default behavior:
        - Voltage channels: pass through as-is
        - Current channels: convert voltage to current using shunt resistor (I = V / R)

    Modify this function to add your custom processing logic.
    """
    output = raw_data.copy()

    for ch_idx, ch_cfg in enumerate(channel_config):
        if ch_cfg.get("type") == "current":
            shunt = ch_cfg.get("shunt_resistor", 1.0)
            output[ch_idx, :] = raw_data[ch_idx, :] / shunt  # I = V / R

    # -----------------------------------------------------
    # ADD YOUR CUSTOM PROCESSING HERE
    # Example:
    #   output[0, :] = raw_data[0, :] * 2.0  # Scale channel 0
    #   output[1, :] = np.clip(raw_data[1, :], -5, 5)  # Clamp channel 1
    # -----------------------------------------------------

    return output

# =============================================================================
# UDP PUBLISHER
# =============================================================================

class UDPPublisher:
    """UDP publisher - one CSV row per packet."""

    def __init__(self, host: str, port: int):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1048576)
        self._address = (host, port)
        self._fmt = f"{{:.{CSV_PRECISION}f}}"
        self.packets_sent = 0
        self.bytes_sent = 0

    def publish(self, data: np.ndarray) -> None:
        """Publish each sample as a CSV row via UDP."""
        num_channels, num_samples = data.shape
        for i in range(num_samples):
            row = ",".join(self._fmt.format(data[ch, i]) for ch in range(num_channels)) + "\n"
            payload = row.encode('utf-8')
            try:
                self.socket.sendto(payload, self._address)
                self.packets_sent += 1
                self.bytes_sent += len(payload)
            except socket.error:
                pass

    def close(self) -> None:
        self.socket.close()

# =============================================================================
# DAQ READER
# =============================================================================

class DAQReader:
    """NI DAQmx reader with continuous multi-channel acquisition."""

    def __init__(self, channel_config: list):
        self.channel_config = channel_config
        self.num_channels = len(channel_config)
        self._buffer = np.zeros((self.num_channels, SAMPLES_PER_READ), dtype=np.float64)
        self.task = None
        self.reader = None
        self.samples_read = 0

    def setup(self) -> None:
        """Initialize DAQmx task and channels."""
        self.task = nidaqmx.Task()

        for ch_cfg in self.channel_config:
            physical_channel = f"{DEVICE_NAME}/{ch_cfg['channel']}"
            v_min, v_max = ch_cfg["voltage_range"]
            term_config = TERMINAL_MAP.get(ch_cfg["terminal"].upper(), TerminalConfiguration.RSE)

            self.task.ai_channels.add_ai_voltage_chan(
                physical_channel,
                name_to_assign_to_channel=ch_cfg.get("name", ch_cfg["channel"]),
                min_val=v_min,
                max_val=v_max,
                terminal_config=term_config
            )
            ch_type = ch_cfg.get("type", "voltage")
            if ch_type == "current":
                print(f"  {ch_cfg['channel']}: {ch_type} (shunt={ch_cfg.get('shunt_resistor', 1.0)} ohm)")
            else:
                print(f"  {ch_cfg['channel']}: {ch_type} ({v_min}V to {v_max}V)")

        self.task.timing.cfg_samp_clk_timing(
            rate=SAMPLE_RATE,
            sample_mode=AcquisitionType.CONTINUOUS,
            samps_per_chan=max(SAMPLES_PER_READ * 10, 10)
        )

        self.reader = AnalogMultiChannelReader(self.task.in_stream)
        self.task.start()
        print(f"DAQmx started: {self.num_channels} channels @ {SAMPLE_RATE} Hz")

    def read(self) -> np.ndarray:
        """Read samples from all channels."""
        samples = self.reader.read_many_sample(self._buffer, number_of_samples_per_channel=SAMPLES_PER_READ)
        self.samples_read += samples
        return self._buffer[:, :samples]

    def close(self) -> None:
        if self.task:
            self.task.stop()
            self.task.close()

# =============================================================================
# MAIN APPLICATION
# =============================================================================

class DAQBridge:
    """Main application: DAQ -> Process -> UDP."""

    def __init__(self):
        self.running = False
        self.num_channels = len(CHANNEL_CONFIG)
        self.daq = DAQReader(CHANNEL_CONFIG)
        self.publisher = UDPPublisher(UDP_HOST, UDP_PORT)
        self._last_stats_time = time.perf_counter()
        self._last_samples = 0
        self._loop_times = deque(maxlen=1000)

    def run(self) -> None:
        """Main acquisition loop."""
        self.running = True
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)

        print("=" * 50)
        print("DAQBridge - NI DAQmx")
        print("=" * 50)
        print(f"Device: {DEVICE_NAME}")
        print(f"Sample Rate: {SAMPLE_RATE} Hz | Samples/Read: {SAMPLES_PER_READ}")
        print(f"UDP: {UDP_HOST}:{UDP_PORT}")
        print(f"Channels: {self.num_channels}")
        print("-" * 50)

        try:
            self.daq.setup()
            print("Press Ctrl+C to stop\n")

            while self.running:
                loop_start = time.perf_counter()

                raw_data = self.daq.read()
                processed = process_readings(raw_data, CHANNEL_CONFIG)
                self.publisher.publish(processed)

                self._loop_times.append(time.perf_counter() - loop_start)
                if ENABLE_STATS:
                    self._print_stats()

        except Exception as e:
            print(f"\nError: {e}")
            raise
        finally:
            self._cleanup()

    def _signal_handler(self, signum, frame) -> None:
        print("\nShutting down...")
        self.running = False

    def _print_stats(self) -> None:
        now = time.perf_counter()
        elapsed = now - self._last_stats_time
        if elapsed >= PRINT_STATS_INTERVAL:
            rate = (self.daq.samples_read - self._last_samples) / elapsed
            avg_loop = np.mean(self._loop_times) * 1000 if self._loop_times else 0
            print(f"[STATS] Samples: {self.daq.samples_read:,} | Rate: {rate:,.0f} S/s | "
                  f"Packets: {self.publisher.packets_sent:,} | Loop: {avg_loop:.2f}ms")
            self._last_stats_time = now
            self._last_samples = self.daq.samples_read
            self._loop_times.clear()

    def _cleanup(self) -> None:
        print("Cleaning up...")
        self.daq.close()
        self.publisher.close()
        print(f"Total: {self.daq.samples_read:,} samples, {self.publisher.packets_sent:,} packets")

# =============================================================================
# ENTRY POINT
# =============================================================================

if __name__ == "__main__":
    DAQBridge().run()
