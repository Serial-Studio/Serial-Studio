#
# hydrogen.py
# 
# Simulates a hydrogen atom's electron cloud using a probabilistic model of the 
# 1s orbital and streams the data via UDP for real-time visualization in 
# Serial Studio.
# 
# --- Purpose ---
# This script is designed for educational and visualization purposes, showing
# how an electron in a hydrogen atom is not in a fixed orbit but distributed 
# according to quantum probability density. 
#
# It leverages a simplified Monte Carlo simulation to generate random 
# positions based on the 1s orbital probability distribution.
# 
# --- What It Streams (UDP, CSV Format) ---
#   x       : Electron X-coordinate in 3D space
#   y       : Electron Y-coordinate in 3D space
#   z       : Electron Z-coordinate in 3D space
#   psi²    : Probability density at radius r (ψ² = |ψ(r)|²)
#   r       : Radial distance from nucleus (for spatial context)
#
# Example frame:
#  -0.283291,0.453772,0.125448,0.038142,0.621987
#
# --- Use In Serial Studio ---
# - Set input source to UDP on port 9000
# - Load the hydrogen.json project from the examples.
# - Plot combinations like:
#     x vs y        → 2D orbital projection
#     x vs psi²     → Electron probability across space
#     x vs z        → Orbital cross-sections
# 
# --- Notes ---
# - Not a true quantum mechanical solver. This is a visual/statistical 
#   approximation.
# - It's recommended to enable the scatter plot features instead of the
#   interpolated/line plots for better visualization.
# 

import socket
import time
import math
import random

#------------------------------------------------------------------------------
# UDP socket configuration
#------------------------------------------------------------------------------

UDP_IP = "localhost"     
UDP_PORT = 9000         

#------------------------------------------------------------------------------
# Bohr radius (normalized units)
#------------------------------------------------------------------------------

a0 = 1.0 

#------------------------------------------------------------------------------
# Functions
#------------------------------------------------------------------------------

def sample_radius():
    """Rejection sampling from 1s orbital: P(r) ∝ r^2 * e^(-2r/a0)"""
    while True:
        r = random.uniform(0, 5 * a0)
        p = (r ** 2) * math.exp(-2 * r / a0)
        if random.uniform(0, 1) < p:
            return r

def sample_angles():
    """Sample spherical angles θ ∈ [0, π], φ ∈ [0, 2π]"""
    theta = math.acos(2 * random.uniform(0, 1) - 1)
    phi = random.uniform(0, 2 * math.pi)
    return theta, phi

def compute_wavefunction_density(r):
    """1s orbital ψ² (normalized)"""
    return (1 / math.pi) * math.exp(-2 * r / a0)

#------------------------------------------------------------------------------
# Data streaming loop
#------------------------------------------------------------------------------

if __name__ == "__main__":
    t_start = time.time()
    try:
        # Initialize the UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.bind(('', 0))

        # Execute the simulation
        while True:
            # Time since start (for noise modulation)
            t = time.time() - t_start

            # Obtain radius
            r = sample_radius()

            # Obtain angles
            theta, phi = sample_angles()

            # Spherical to Cartesian
            x = r * math.sin(theta) * math.cos(phi)
            y = r * math.sin(theta) * math.sin(phi)
            z = r * math.cos(theta)
            psi2 = compute_wavefunction_density(r)

            # Transmit and UDP datagram with x,y,z,psi²,r
            message = f"{x:.6f},{y:.6f},{z:.6f},{psi2:.6f},{r:.6f}\n"
            sock.sendto(message.encode('utf-8'), (UDP_IP, UDP_PORT))

            # 1 KHz stream
            time.sleep(0.001)

    except KeyboardInterrupt:
        print("\nUDP stream stopped.")
