# Lorenz attractor + Serial Studio

## Overview

This project simulates and visualizes the Lorenz attractor, a chaotic system of differential equations, using a small Python generator and [Serial Studio](https://serial-studio.github.io/). The generator integrates the Lorenz trajectory in real time and streams the resulting ($x$, $y$, $z$) values to Serial Studio over UDP for plotting.

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

The generator uses the Euler method for numerical integration to evolve the system over time.

## Project features

- **Real-time visualization.** Watch the Lorenz attractor's chaotic motion as it happens.
- **Custom X-axis.** Use Serial Studio's project editor to pick any dataset as the X-axis source.
- **Dynamic visualization.** Plot $x$, $y$, and $z$ on 2D or 3D graphs.

## Requirements

- **Python 3.x.** No extra packages are needed.
- **Serial Studio.** Download the latest version from [here](https://serial-studio.github.io/).

## Running the example

Open `LorenzAttractor.ssproj` in Serial Studio and click **Connect**. That is all.

The project includes a control loop that launches `lorenz_udp.py` automatically when the connection opens, so there is nothing to start by hand. The script resolves the interpreter cross-platform (it tries `python3` first, then falls back to `python`), runs the generator from the project folder, and Serial Studio stops it again when you disconnect, switch projects, or quit.

If you prefer to run the generator yourself, start it from a terminal before connecting:

```bash
python3 lorenz_udp.py
```

The script sends one `x,y,z` sample per millisecond to `127.0.0.1:9000` until stopped.

## Serial Studio configuration

The bundled project is already set up: its data source is a UDP socket on local port **9000**, and it contains both visualizations, a Multi-Plot group (`2D Visualization`) and a 3D plot group (`3D Visualization`). Each newline-terminated `x,y,z` line is split by the project's Lua CSV frame parser.

Here is how the project editor should look:

![Serial Studio project setup](doc/project-setup.png)

### Custom X-axis example

Serial Studio's custom X-axis feature lets you map any dataset to serve as the X-axis source for plots. It is particularly useful for:

- Plotting values against elapsed time or packet numbers.
- Advanced visualizations like the Lorenz attractor.

In the bundled project the three datasets cross-reference each other ($x$ against $y$, $y$ against $z$, $z$ against $x$) to draw the attractor.

## Troubleshooting

- **No data appears.**
  - Make sure Serial Studio is listening on UDP port 9000 (the project's preconfigured source).
  - Confirm that Python 3 is installed and on your `PATH` so the control loop can launch it.
- **Chaotic output.**
  - Adjust the `transmission_interval` variable in `lorenz_udp.py` if needed.
