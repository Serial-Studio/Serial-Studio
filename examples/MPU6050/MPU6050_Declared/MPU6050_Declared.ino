//
// MPU6050 Data Capture Program, declared variant
//
// This is MPU6050.ino with the dashboard declared by the sketch itself. The SerialStudio
// library sends the groups, titles, units and widgets at power-up and whenever Serial Studio
// asks, so no project file is needed: set Operation Mode to Quick Plot and connect.
//
// Required Components:
// - MPU6050 Sensor connected to the Arduino (I2C communication)
//
// Required Libraries:
// - Adafruit MPU6050 and Adafruit Unified Sensor
// - SerialStudio (this repository, firmware/arduino)
//
// Connections:
// - Connect SDA pin of the MPU6050 to the Arduino's A4 (SDA) pin.
// - Connect SCL pin of the MPU6050 to the Arduino's A5 (SCL) pin.
//
// Baud Rate:
// - Serial Studio: 115200 baud
//

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SerialStudio.h>
#include <Wire.h>

// Written against version 1 of the SerialStudio library
#if SS_LIBRARY_VERSION_MAJOR != 1
#error This sketch needs version 1 of the SerialStudio library
#endif

Adafruit_MPU6050 mpu;
SerialStudio studio(Serial, "MPU6050");

void setup(void) {
  Serial.begin(115200);

  while (!Serial)
    delay(10);

  if (!mpu.begin()) {
    // A text line is not a frame, so Quick Plot shows it in the console only
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Answers the app's request for the declaration
  studio.poll();

  studio.group(F("Accelerometer"), F("accelerometer"));
  studio.channel(F("Accelerometer X"), a.acceleration.x).units(F("m/s\xc2\xb2")).widget(F("x"));
  studio.channel(F("Accelerometer Y"), a.acceleration.y).units(F("m/s\xc2\xb2")).widget(F("y"));
  studio.channel(F("Accelerometer Z"), a.acceleration.z).units(F("m/s\xc2\xb2")).widget(F("z"));

  studio.group(F("Gyroscope"), F("gyro"));
  studio.channel(F("Gyro X"), g.gyro.x).units(F("deg/s")).widget(F("x"));
  studio.channel(F("Gyro Y"), g.gyro.y).units(F("deg/s")).widget(F("y"));
  studio.channel(F("Gyro Z"), g.gyro.z).units(F("deg/s")).widget(F("z"));

  studio.group(F("Temperature"));
  studio.channel(F("Temperature"), temp.temperature)
    .units(F("\xe2\x84\x83"))
    .widget(F("bar"))
    .range(0, 100)
    .graph(false);

  // Writes the declaration on the first pass and after a request, a frame on every other pass
  studio.send();

  delay(10);
}
