//
// ADC Data Transmission Program
//
// Author: Alex Spataru
//
// This program reads data from six analog input pins (A0 to A5) on an Arduino.
// It scales the raw analog readings (0-1023) to 8-bit values (0-255) and
// transmits them as binary data over the serial port.
//
// Required Components:
// - 6 analog sensors connected to the Arduino's analog pins (A0 to A5)
//
// Connections:
// - Connect the signal outputs of the sensors to the analog pins A0 to A5 on
//   the Arduino.
//
// Baud Rate:
// - Serial Monitor: 115200 baud
//

//-----------------------------------------------------------------------------
// Initialization function
//-----------------------------------------------------------------------------

void setup() {
  // Initialize the serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Wait for the serial port to initialize
  while (!Serial)
    ;

  // Configure the ADC pins (A0 to A5) as input pins
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
}

//-----------------------------------------------------------------------------
// Main loop function
//-----------------------------------------------------------------------------

void loop() {
  // Read the analog values from pins A0 to A5
  // Show each one of them over a range of values
  Serial.write("$");
  Serial.write(map(analogRead(A0), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A1), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A2), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A3), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A4), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A5), 0, 1023, 0, 255));
  Serial.write(";");

  // Small delay to manage the sampling rate
  delay(1);
}
