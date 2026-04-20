# Lorenz attractor + Serial Studio

## Overview

This project shows how to simulate and visualize the Lorenz attractor, a chaotic system of differential equations, using an Arduino board and [Serial Studio](https://serial-studio.github.io/). The Arduino calculates the Lorenz trajectory in real time and sends the resulting ($x$, $y$, $z$) values to Serial Studio for plotting.

The Lorenz system, introduced by Edward Lorenz in 1963, is a set of three coupled differential equations commonly used to model atmospheric convection. Its iconic butterfly-shaped attractor is a symbol of chaos theory. For more, see [this article](https://marksmath.org/visualization/LorenzExperiment/).

![Lorenz attractor in Serial Studio](doc/screenshot.png)

> This project uses features available only with a paid license. See [serial-studio.com](https://serial-studio.com/) for details.

## Lorenz system basics

The system is governed by:

$$
\frac{dx}{dt} = \sigma (y - x)
$$
$$
\frac{dy}{dt} = x (\rho - z) - y
$$
$$
\frac{dz}{dt} = x y - \beta z
$$

Where:

- $\sigma$ (sigma): rate of rotation (set to $10.0$).
- $\rho$ (rho): height of the attractor (set to $28.0$).
- $\beta$ (beta): damping factor (set to $\frac{8}{3}$).

The Arduino program uses the Euler method for numerical integration to evolve the system over time.

## Project features

- **Real-time visualization.** Watch the Lorenz attractor's chaotic motion as it happens.
- **Custom X-axis.** Use Serial Studio's project editor to pick any dataset as the X-axis source.
- **Dynamic visualization.** Plot $x$, $y$, and $z$ on 2D or 3D graphs.

## Hardware setup

### Requirements

- **Arduino board.** Uno, Mega, Nano, or compatible.
- **Serial Studio.** Download the latest version from [here](https://serial-studio.github.io/).

### Connections

No extra hardware beyond the Arduino. Connect it to your computer via USB.

## Arduino sketch

The code simulates the Lorenz attractor and transmits $x$, $y$, and $z$ to Serial Studio:

```cpp
//
// Lorenz Attractor Data Generator
//
// Author: Alex Spataru
//

// Parameters for the Lorenz system
float sigma = 10.0;     // σ: rate of rotation
float rho = 28.0;       // ρ: height of attractor
float beta = 8.0 / 3.0; // β: damping factor

// Initial conditions
float x = 0.1;  // Initial X value
float y = 0.0;  // Initial Y value
float z = 0.0;  // Initial Z value

// Time step
float dt = 0.01; // Time increment for numerical integration

// Interval between data transmissions (milliseconds)
unsigned long transmissionInterval = 1;
unsigned long lastTransmissionTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
}

void loop() {
  // Calculate the derivatives
  float dx = sigma * (y - x) * dt;
  float dy = (x * (rho - z) - y) * dt;
  float dz = (x * y - beta * z) * dt;

  // Update the state
  x += dx;
  y += dy;
  z += dz;

  // Transmit data at regular intervals
  if (millis() - lastTransmissionTime >= transmissionInterval) {
    lastTransmissionTime = millis();
    Serial.print(x, 6);
    Serial.print(",");
    Serial.print(y, 6);
    Serial.print(",");
    Serial.println(z, 6);
  }
}
```

## Serial Studio configuration

### 1. Setting up the project

1. Open Serial Studio and click **Project Editor**.
2. Create a new project or import the provided `LorenzAttractor.ssproj` file.
3. Add three datasets for $x$, $y$, and $z$:
   - Dataset $x$: use $y$ as the X-axis source.
   - Dataset $y$: use $z$ as the X-axis source.
   - Dataset $z$: use $x$ as the X-axis source.

### 2. Plotting the Lorenz attractor

1. Open the project in Serial Studio.
2. Connect to the Arduino using the correct serial port, set the baud rate to 115200.
3. Add a Multi-Plot widget to visualize the attractor.

Here's how the project editor should look:

![Serial Studio project setup](doc/project-setup.png)

## Custom X-axis example

Serial Studio's custom X-axis feature lets you map any dataset to serve as the X-axis source for plots. It's particularly useful for:

- Plotting values against elapsed time or packet numbers.
- Advanced visualizations like the Lorenz attractor.

## Python UDP version

For testing without Arduino hardware, `lorenz_udp.py` sends Lorenz attractor data via UDP.

### Requirements

- Python 3.x (no extra packages needed).

### Usage

1. Run the Python script:

   ```bash
   python3 lorenz_udp.py
   ```

2. In Serial Studio:
   - Select **Network** as the data source.
   - Choose **UDP** protocol.
   - Set the port to **9000**.
   - Load the `LorenzAttractor.ssproj` project file.
   - Click **Connect**.

The script sends Lorenz attractor data to `127.0.0.1:9000` continuously.

## Troubleshooting

- **No data appears.**
  - Make sure the Arduino sketch is uploaded correctly.
  - Check the serial port and baud rate in Serial Studio.
  - For the UDP version: make sure Serial Studio is listening on port 9000 and the Python script is running.
- **Chaotic output.**
  - Make sure `transmissionInterval` in the Arduino sketch fits your system.
  - For the UDP version: adjust the `transmission_interval` parameter in the Python script if needed.
