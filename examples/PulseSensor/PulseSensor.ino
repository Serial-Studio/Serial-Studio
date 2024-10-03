//
// Pulse Sensor Data Filter Program
//
// Author: Alex Spataru
//
// This program reads data from a pulse sensor connected to an Arduino.
// It filters the raw signal using a low-pass filter to remove high-frequency noise
// and then applies a moving average filter to smooth the final signal.
//
// The filtered signal is transmitted over the serial port for visualization.
// You can use the the "Quick Plot" feature in Serial Studio to visualize and
// create a CSV file from the generated data without needing to create any
// project file.
//
// Required Components:
// - Pulse Sensor connected to analog pin A0
//
// Connections:
// - Connect the signal output of the pulse sensor to the Arduino's analog pin A0.
//
// Baud Rate:
// - Serial Monitor: 115200 baud
//

// Pin and Buffer Definitions
#define SENSOR_PIN A0   // Pulse sensor connected to analog pin A0
#define BUFFER_SIZE 10  // Buffer size for smoothing the final output

// Global Variables
int sensorValue = 0;          // Raw sensor reading from the pulse sensor
float lowPassFiltered = 0.0;  // Signal after low-pass filtering (removes high-frequency noise)
float filteredSignal = 0.0;   // Final filtered signal after smoothing
float previousLowPass = 0.0;  // Stores previous value for low-pass filtering

// Buffer for Moving Average Filter (to smooth the final output)
float buffer[BUFFER_SIZE];  // Circular buffer to store previous filtered values
int bufferIndex = 0;        // Current index for storing in the buffer
float bufferSum = 0.0;      // Sum of all values in the buffer for averaging

// Low-pass filter coefficient
const float lowPassAlpha = 0.1;  // Low-pass filter factor (smooths out high-frequency noise)

void setup() {
  // Start the serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Initialize the buffer to zero for proper averaging at startup
  for (int i = 0; i < BUFFER_SIZE; i++)
    buffer[i] = 0.0;
}

void loop() {
  // Read the analog value from the sensor connected to pin A0
  sensorValue = analogRead(SENSOR_PIN);

  // Apply a low-pass filter to smooth out rapid changes in the sensor signal
  lowPassFiltered = (lowPassAlpha * sensorValue) + ((1.0 - lowPassAlpha) * previousLowPass);
  previousLowPass = lowPassFiltered;

  // Moving average filter to further smooth the signal
  bufferSum -= buffer[bufferIndex];       // Remove the oldest value from the sum
  buffer[bufferIndex] = lowPassFiltered;  // Store the new value in the buffer
  bufferSum += buffer[bufferIndex];       // Add the new value to the sum

  // Increment the buffer index in a circular manner
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

  // Calculate the average value of the buffer for the final smoothed signal
  filteredSignal = bufferSum / BUFFER_SIZE;

  // Transmit the filtered signal to Serial Studio for visualization
  Serial.println(filteredSignal);

  // Wait a little to adjust sampling rate
  delay(5);
}