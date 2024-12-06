//
// Lorenz Attractor Data Generator (Fixed-Point Arithmetic)
//
// Author: Alex Spataru
//
// This program is specifically designed for the ESP32 microcontroller and simulates
// the Lorenz system using fixed-point arithmetic. The Lorenz system is a set of
// three chaotic differential equations, and the program transmits the generated
// data (x, y, z) over the serial port. The output is ideal for visualizing the
// Lorenz attractor in real-time using tools like Serial Studio.
//
// Lorenz System Parameters:
// - σ (sigma): 10.0      (controls the rate of rotation in the attractor)
// - ρ (rho):   28.0      (sets the "height" of the attractor)
// - β (beta):  8.0 / 3.0 (controls the damping)
//
// How It Works:
// - The program calculates the Lorenz attractor using the Euler method, updating
//   the x, y, and z values at each step based on the Lorenz equations.
// - It uses fixed-point arithmetic for precise calculations, which is necessary
//   due to the ESP32's 32-bit floating-point limitations.
// - The ESP32's FreeRTOS capabilities are leveraged to separate simulation and
//   serial transmission tasks for optimal performance.
// - The data is transmitted asynchronously at regular intervals for visualization.
//
// Required Tools:
// - ESP32 microcontroller (e.g., ESP32 DevKit, NodeMCU ESP32, etc.)
// - Serial Studio or any serial plotting tool to visualize the data.
//
// Baud Rates:
// - Serial Studio: 115200 baud
//
// Notes:
// - This program is optimized for the ESP32 and may require modifications to work
//   on other platforms due to its use of FreeRTOS and ESP32-specific features.
// - Ensure your ESP32 board is correctly connected and configured for the Arduino IDE.
//
// How to Run:
// - Load this program onto an ESP32 using the Arduino IDE or similar tool.
// - Connect to the ESP32 via Serial Studio or another serial plotting tool.
// - Observe the real-time Lorenz attractor visualization.
//

#include <Arduino.h>

// Queue for asynchronous data transmission
QueueHandle_t DATA_QUEUE;

///
/// Task to calculate the Lorenz system state
///
void simulationTask(void *param) {
  // Parameters for the Lorenz system (scaled by 1000 for fixed-point arithmetic)
  const int32_t sigma = 10000;  // σ: 10.0 scaled by 1000
  const int32_t rho = 28000;    // ρ: 28.0 scaled by 1000
  const int32_t beta = 2666;    // β: 8/3 scaled by 1000

  // Initial conditions (scaled by 1000)
  int32_t x = 100;
  int32_t y = 0;  
  int32_t z = 0;

  // Initialize derivatives
  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dz = 0;

  // Time step (scaled by 1000)
  const int32_t dt = 10; 

  // Task loop
  while (true) {
    // Calculate the derivatives (scaled by 1000)
    dx = ((sigma * (y - x)) / 1000) * dt / 1000;
    dy = (((x * (rho - z)) / 1000 - y) * dt) / 1000;
    dz = (((x * y) / 1000 - (beta * z) / 1000) * dt) / 1000;

    // Integrate the derivatives to update the system's state
    x += dx;
    y += dy;
    z += dz;

    // Send data to the queue
    int32_t data[3] = { x, y, z };
    xQueueSend(DATA_QUEUE, &data, portMAX_DELAY);

    // Maintain consistent time step
    vTaskDelay(pdMS_TO_TICKS(dt));
  }
}

///
/// Task to transmit Lorenz system data over Serial
///
void serialTask(void *param) {
  while (true) {
    int32_t data[3];
    if (xQueueReceive(DATA_QUEUE, &data, portMAX_DELAY)) {
      Serial.print(data[0] / 1000.0, 6);
      Serial.print(",");
      Serial.print(data[1] / 1000.0, 6);
      Serial.print(",");
      Serial.println(data[2] / 1000.0, 6);
    }
  }
}

///
/// Configures the ESP32, initializes the serial port, and creates tasks
///
void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  while (!Serial)
    ;

  // Create a queue to hold Lorenz system data
  DATA_QUEUE = xQueueCreate(10, sizeof(int32_t[3]));

  // Create tasks for simulation and data transmission
  xTaskCreate(simulationTask, "SimulationTask", 4096, NULL, 1, NULL);
  xTaskCreate(serialTask, "SerialTask", 2048, NULL, 1, NULL);
}

///
/// The main loop is empty; tasks handle simulation and data transmission
///
void loop() {
  // Do nothing
}
