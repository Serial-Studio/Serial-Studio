# CAN Bus Example for Serial Studio

This example shows how to use CAN bus with Serial Studio. It includes a car simulator that sends realistic vehicle data.

## Quick Start (No Hardware Required!)

### 1. Start Serial Studio
- Go to **Setup** → **CAN Bus** tab
- Select **Driver**: `VirtualCAN`
- Select **Interface**: `can0`
- Click **Connect**

### 2. Run the Simulator
```bash
cd "examples/CAN Bus Example"
python3 ecu_simulator.py --virtual
```

### 3. See Live CAN Data
You will see CAN messages in Serial Studio's Console (View → Console).

Switch Console to **Hexadecimal** mode to see the raw data.

---

## Importing the DBC File

To create a dashboard with gauges, plots, and organized data:

### Step 1: Connect to CAN Bus
Make sure Serial Studio is connected (VirtualCAN or hardware).

### Step 2: Import DBC Database
1. In the **Setup** → **CAN Bus** panel, click **Import DBC File...**
2. Navigate to `examples/CAN Bus Example/`
3. Select `example_vehicle.dbc`
4. Click **Open**

### Step 3: Review the Preview
A window appears showing:
- All CAN messages from the DBC file
- Message IDs (0x100, 0x101, etc.)
- How many signals each message has
- Total number of signals

Look at the summary, then click **Create Project**.

### Step 4: Save the Project
1. Serial Studio generates:
   - JavaScript frame parser (decodes CAN frames automatically)
   - Groups for each CAN message
   - Datasets for each signal
   - Auto-assigned widgets (Gauges, Plots, Bars)
2. The **Project Editor** opens automatically
3. Click **File** → **Save** (or Ctrl+S)
4. Save as `vehicle_can.ssproj`

### Step 5: View the Dashboard
1. Close the Project Editor
2. The Dashboard automatically loads your project
3. You'll see:
   - **Engine RPM** gauge
   - **Vehicle Speed** plot
   - **Coolant Temperature** gauge
   - **Battery State of Charge** bar
   - And many more widgets for all signals

### Step 6: Customize (Optional)
Reopen the project editor to:
- Rearrange widget layout
- Change widget types
- Adjust min/max ranges
- Modify titles and units

---

## What's in This Example

### Simulated CAN Messages

The ECU simulator sends realistic automotive data:

**Engine ECU (100 Hz)**
- `0x100`: RPM (0-8000), Speed (0-300 km/h)
- `0x101`: Coolant Temp, Intake Temp, Engine Load
- `0x102`: Throttle Position, Fuel Pressure

**Transmission ECU (50 Hz)**
- `0x200`: Current Gear, Trans Temp, Torque Converter Slip

**Battery Management (10 Hz)**
- `0x400`: Pack Voltage, Pack Current
- `0x401`: State of Charge, Pack Temperature
- `0x402`: Individual Cell Voltages

**Chassis/ABS (100 Hz)**
- `0x600`: Individual Wheel Speeds
- `0x601`: Brake Pressure
- `0x700`: Steering Angle

**Body Control (10 Hz)**
- `0x300`: Lights (headlights, turn signals)
- `0x301`: Doors (open/closed status)

**Instrument Cluster (5 Hz)**
- `0x500`: Odometer, Trip Distance
- `0x501`: Fuel Level, Fuel Consumption

### Files

- **`ecu_simulator.py`**: All-in-one simulator (VirtualCAN, PCAN, SocketCAN, Vector)
- **`example_vehicle.dbc`**: DBC database file
- **`README.md`**: This file

---

## Usage Options

### VirtualCAN (Recommended for Testing)

**Works on macOS/Windows/Linux without hardware!**

```bash
# Simple - just run it
python3 ecu_simulator.py --virtual

# Custom update rate
python3 ecu_simulator.py --virtual -r 100

# Custom channel
python3 ecu_simulator.py --virtual -c can1
```

**In Serial Studio:**
- Driver: `VirtualCAN`
- Interface: `can0` (or your specified channel)

### Physical Hardware

Serial Studio supports all Qt CAN backends:

| Backend | Platforms | Typical Use |
|---------|-----------|-------------|
| **VirtualCAN** | All | Testing without hardware (TCP-based) |
| **SocketCAN** | Linux | Most USB-CAN adapters on Linux |
| **PEAK CAN** | All | PEAK PCAN-USB adapters |
| **Vector CAN** | Windows/Linux | Vector VN-series interfaces |
| **PassThru CAN** | Windows | SAE J2534 Pass-Thru devices |
| **SysTec CAN** | Windows | SYS TEC USB-CANmodul |
| **Tiny CAN** | Windows | MHS Tiny-CAN adapters |

#### macOS with PEAK PCAN-USB
```bash
python3 ecu_simulator.py -i pcan -c PCAN_USBBUS1
```

**In Serial Studio:**
- Driver: `PEAK CAN`
- Interface: `PCAN_USBBUS1`
- Bitrate: `500000`

#### Linux with SocketCAN
```bash
# Setup virtual CAN interface
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# Run simulator
python3 ecu_simulator.py -i socketcan -c vcan0
```

**In Serial Studio:**
- Driver: `SocketCAN`
- Interface: `vcan0`
- Bitrate: `500000`

#### Windows with PEAK PCAN
```bash
python ecu_simulator.py -i pcan -c PCAN_USBBUS1
```

**In Serial Studio:**
- Driver: `PEAK CAN`
- Interface: `PCAN_USBBUS1`
- Bitrate: `500000`

---

## Hardware Setup (If Using Physical CAN)

### macOS

1. **Buy CAN Hardware**: PEAK PCAN-USB adapter (~$150-200)
2. **Install Drivers**: [PEAK macOS Drivers](https://www.peak-system.com/Drivers.523.0.html?&L=1)
3. **Restart** your Mac
4. **Verify**: Check that Serial Studio detects the interface

### Linux

**Option 1: Virtual CAN (No Hardware)**
```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

**Option 2: Physical CAN Hardware**
```bash
# For SocketCAN-compatible devices (most USB-CAN adapters)
sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0
```

### Windows

1. **Install CAN Hardware Drivers**:
   - [PEAK PCAN](https://www.peak-system.com/Drivers.523.0.html?&L=1)
   - [Vector CAN](https://www.vector.com/us/en/products/products-a-z/libraries-drivers/)
   - [SysTec CAN](https://www.systec-electronic.com/en/company/support/driver)

2. **Connect Hardware** and verify in Device Manager

3. **Launch Serial Studio** and check driver dropdown

---

## Troubleshooting

### "No CAN Drivers Found"

**VirtualCAN**: Should always be available. If not, Qt installation may be incomplete.

**Physical Hardware**:
- **macOS/Windows**: Install CAN hardware drivers first
- **Linux**: Load kernel modules: `sudo modprobe vcan` or `sudo modprobe can`

### Python Simulator Won't Connect

**VirtualCAN Mode**:
```
Error: Failed to connect to Qt VirtualCAN

Solution: Start Serial Studio FIRST, connect to VirtualCAN, THEN run the simulator.
The first client to connect creates the TCP server.
```

**Hardware Mode**:
```bash
# Check if python-can is installed
pip install python-can

# Verify hardware is detected
python3 -c "import can; print(can.detect_available_configs())"
```

### No Data in Serial Studio

1. **Check Console**: Go to View → Console, switch to **Hexadecimal** mode
2. **Verify Connection**: Status bar should show "Connected"
3. **Check Bitrate**: Must match on both sides (500000)
4. **Verify Interface**: Both must use same interface name

### Data Shows as Dots/Garbage

The raw CAN frames are binary data. To see meaningful information:
1. **Import the DBC file** (see "Importing the DBC File" section above)
2. Or switch Console to **Hexadecimal** mode to see raw hex values

---

## Advanced Customization

### Modify Simulation Behavior

Edit `ecu_simulator.py`:

```python
# Change driving pattern (line ~425)
if cycle_time < 15:
    self.target_speed = min(200, cycle_time * 10)  # Faster acceleration

# Adjust message rates (line ~667)
rates = {
    'engine': 200,      # Increase to 200 Hz
    'transmission': 100,
    # ...
}
```

### Create Custom DBC Files

Use DBC editors to create your own CAN databases:
- [Vector CANdb++ Editor](https://www.vector.com/int/en/products/products-a-z/software/candb-editor/)
- [PEAK PCAN-Explorer](https://www.peak-system.com/PCAN-Explorer.366.0.html)
- [SavvyCAN](https://www.savvycan.com/) (Free, open-source)

Then import into Serial Studio as described above.

### Monitor CAN Traffic

**Linux Command Line**:
```bash
candump vcan0                    # View all frames
candump -t a vcan0               # With timestamps
cansend vcan0 100#1122334455    # Send test frame
```

**Serial Studio Console**:
- View → Console
- Set Display Mode to **Hexadecimal**
- All CAN frames shown in format: `[ID_H ID_L DLC DATA...]`

---

## See Also

- [Serial Studio Documentation](https://serial-studio.com/)
- [Qt VirtualCAN Documentation](https://doc.qt.io/qt-6/qtserialbus-virtualcan-overview.html)
- [python-can Documentation](https://python-can.readthedocs.io/)
- [DBC File Format Specification](https://www.csselectronics.com/pages/can-dbc-file-database-intro)
- [PEAK PCAN Products](https://www.peak-system.com/)
- [Vector CAN Products](https://www.vector.com/)
