//
// Lorenz Attractor Data Generator
//
// Author: Alex Spataru
//
// This program simulates the Lorenz system, a set of three chaotic differential
// equations, and transmits the generated data (x, y, z) over the serial port.
// The output is ideal for visualizing the Lorenz attractor in real-time using
// tools like Serial Studio.
//
// Lorenz System Parameters:
// - σ (sigma): 10.0      (controls the rate of rotation in the attractor)
// - ρ (rho):   28.0      (sets the "height" of the attractor)
// - β (beta):  8.0 / 3.0 (controls the damping)
//
// How It Works:
// - The program calculates the Lorenz attractor using the Euler method, updating
//   the x, y, and z values at each step based on the Lorenz equations.
// - The data is transmitted at regular intervals for plotting.
//
// Required Tools:
// - Arduino board (e.g., Uno, Mega, Nano, etc.)
// - Serial Studio or any serial plotting tool to visualize the data.
//
// Baud Rates:
// - Serial Studio: 115200 baud
//

// Parameters for the Lorenz system
// clang-format off
float sigma = 10.0;      // σ: rate of rotation
float rho = 28.0;        // ρ: height of attractor
float beta = 8.0 / 3.0;  // β: damping factor
// clang-format on

// Initial conditions
float x = 0.1;  // Initial X value
float y = 0.0;  // Initial Y value
float z = 0.0;  // Initial Z value

// Time step
float dt = 0.01;  // Time increment for numerical integration (Euler's method)

// Interval between data transmissions (milliseconds)
unsigned long transmissionInterval = 1;
unsigned long lastTransmissionTime = 0;

///
/// Configures the primary serial port at a baud rate of 115200
///
void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
}

///
/// Updates the state of the Lorenz system & transmits current
/// x, y and z values theough the serial port.
///
void loop() {
  // Calculate the derivatives (rate of change) for x, y, and z
  float dx = sigma * (y - x) * dt;
  float dy = (x * (rho - z) - y) * dt;
  float dz = (x * y - beta * z) * dt;

  // Integrate the derivatives to update the system's state
  x += dx;
  y += dy;
  z += dz;

  // Transmit data at regular intervals
  if (millis() - lastTransmissionTime >= transmissionInterval) {
    // Update last transmission time
    lastTransmissionTime = millis();

    // Transmit current x, y and z values
    Serial.print(x, 6);
    Serial.print(",");
    Serial.print(y, 6);
    Serial.print(",");
    Serial.println(z, 6);
  }
}