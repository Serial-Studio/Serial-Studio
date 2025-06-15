//
// ADC Data Transmission with CRC-16-CCITT Checksum (Command-Driven)
//
// Author: Alex Spataru
//
// This Arduino program reads analog input from six channels (A0 to A5),
// scales the values from 10-bit (0–1023) to 8-bit (0–255), and transmits
// the data as a binary frame over the serial port when explicitly commanded.
//
// The device remains idle until a "poll-data" command is received via serial.
// Upon receiving this command, it captures the current sensor values and
// transmits a single frame. The host (e.g. Serial Studio) is responsible
// for managing the polling rate.
//
// Each transmitted frame includes:
// - Start delimiter: 0xC0 0xDE
// - 6 sensor values (1 byte each)
// - CRC-16-CCITT checksum (2 bytes, big-endian)
//
// This format ensures data integrity and is compatible with Serial Studio’s
// automatic frame validation and decoding.
//
// Requirements:
// - Arduino with 6 analog sensors connected to A0–A5
// - Serial connection at 115200 baud
//
// Connections:
// - Sensor signal outputs to analog pins A0 through A5
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

void initAdcPort() {
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
}

void sendDataFrame() {
  // Define payload data
  uint8_t payload[6];
  payload[0] = map(analogRead(A0), 0, 1023, 0, 255);
  payload[1] = map(analogRead(A1), 0, 1023, 0, 255);
  payload[2] = map(analogRead(A2), 0, 1023, 0, 255);
  payload[3] = map(analogRead(A3), 0, 1023, 0, 255);
  payload[4] = map(analogRead(A4), 0, 1023, 0, 255);
  payload[5] = map(analogRead(A5), 0, 1023, 0, 255);

  // Compute checksum
  uint16_t crc = crc16_ccitt(payload, sizeof(payload));

  // Send start sequence over serial
  Serial.write(0xC0);
  Serial.write(0xDE);

  // Send payload over serial
  for (int i = 0; i < sizeof(payload); ++i)
    Serial.write(payload[i]);

  // Send end sequence over serial
  //Serial.write(0xDE);
  //Serial.write(0xC0);

  // Send checksum over serial
  Serial.write(static_cast<uint8_t>(crc >> 8));
  Serial.write(static_cast<uint8_t>(crc & 0xFF));
}

//-----------------------------------------------------------------------------
// CRC-16-CCITT (poly 0x1021, init 0x0000)
//-----------------------------------------------------------------------------

uint16_t crc16_ccitt(const uint8_t* data, int length) {
  uint16_t crc = 0x0000;
  for (int i = 0; i < length; ++i) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (int j = 0; j < 8; ++j) {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }

  return crc;
}

//-----------------------------------------------------------------------------
// Main loop function
//-----------------------------------------------------------------------------

void loop() {
  // No serial data available, stop.
  if (Serial.available() <= 0)
    return;

  // Read serial command
  String command = Serial.readStringUntil('\n');
  command.trim();

  // Enable pull up resistor
  if (command == "enable-pull-up")
    setPullUp();

  // Disable pull up resistor
  else if (command == "disable-pull-up")
    initAdcPort();

  // Send data
  else if (command == "poll-data")
    sendDataFrame();
}
