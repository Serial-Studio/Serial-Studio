//
// MPU6050 Data Capture Program
//
// Author: Alex Spataru
//
// This program reads acceleration, gyroscope, and temperature data from an MPU6050
// sensor connected to an Arduino. It outputs the processed sensor readings over the serial
// port for visualization and data logging.
//
// The data can be visualized in Serial Studio with widgets specific to accelerometers
// and gyroscopes (such as a g-meter or an attitude indicator) by creating a project file.
// A JSON project file is available with this example for easy experimentation.
//
// Required Components:
// - MPU6050 Sensor connected to the Arduino (I2C communication)
//
// Connections:
// - Connect SDA pin of the MPU6050 to the Arduino's A4 (SDA) pin.
// - Connect SCL pin of the MPU6050 to the Arduino's A5 (SCL) pin.
//
// Baud Rate:
// - Serial Monitor: 115200 baud
//

// Include necessary libraries for MPU6050 sensor communication
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Define MPU6050 object
Adafruit_MPU6050 mpu;

void setup(void) {
  // Start serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Wait for the serial port to connect (for Serial Monitor stability)
  while (!Serial)
    delay(10);

  // Initialize MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    // Halt program if sensor initialization fails
    while (1) {
      delay(10);
    }
  }

  // Configure MPU6050 options
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // Set accelerometer range to ±8g
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);       // Set gyroscope range to ±500°/s
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);    // Set low-pass filter bandwidth to 21 Hz

  // Delay for sensor stabilization
  delay(100);
}

void loop() {
  // Define events to hold sensor readings
  sensors_event_t a, g, temp;

  // Retrieve readings from the MPU6050 sensor
  mpu.getEvent(&a, &g, &temp);

  // Print sensor data in CSV format for Serial Studio visualization
  Serial.print("$");               // Start of data packet
  Serial.print(a.acceleration.x);  // X-axis acceleration (m/s^2)
  Serial.print(",");
  Serial.print(a.acceleration.y);  // Y-axis acceleration (m/s^2)
  Serial.print(",");
  Serial.print(a.acceleration.z);  // Z-axis acceleration (m/s^2)
  Serial.print(",");
  Serial.print(g.gyro.x);  // X-axis gyroscope (deg/s)
  Serial.print(",");
  Serial.print(g.gyro.y);  // Y-axis gyroscope (deg/s)
  Serial.print(",");
  Serial.print(g.gyro.z);  // Z-axis gyroscope (deg/s)
  Serial.print(",");
  Serial.print(temp.temperature);  // Temperature (°C)
  Serial.print(";");               // End of data packet
  Serial.print("\n");

  // Delay for stable sampling rate
  delay(10);
}