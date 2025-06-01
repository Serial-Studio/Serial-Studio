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
  initAdcPort();
}

//-----------------------------------------------------------------------------
// Serial Studio actions
//-----------------------------------------------------------------------------

void setPullUp() {
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

// Function to set all pins back to INPUT mode
void initAdcPort() {
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
  // Check if there is any serial input available
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "enable-pull-up")
      setPullUp();
    else if (command == "disable-pull-up")
      initAdcPort();
  }

  // Read the analog values from pins A0 to A5
  // Show each one of them over a range of values
  Serial.write(0xC0);
  Serial.write(0xDE);
  Serial.write(map(analogRead(A0), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A1), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A2), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A3), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A4), 0, 1023, 0, 255));
  Serial.write(map(analogRead(A5), 0, 1023, 0, 255));
  Serial.write(0xDE);
  Serial.write(0xC0);

  // Small delay to manage the sampling rate
  delay(1);
}
