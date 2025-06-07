//
// GPS Data Logger Program
//
// Author: Alex Spataru
//
// This program reads GPS data (latitude, longitude, and altitude) from a GPS module 
// connected to an Arduino. It then transmits this data over the serial port for 
// easy viewing on Serial Studio.
//
// The program uses the TinyGPS library to parse NMEA sentences from the GPS module.
// Make sure to download and install the TinyGPS library from the Arduino Library 
// Manager before running this code.
//
// Required Libraries:
// - TinyGPS (Install via Arduino Library Manager)
//
// Connections:
// - Connect the GPS module's TX pin to the RX pin of the Arduino's Serial1.
// - Ensure your GPS module operates at 9600 baud for compatibility.
//
// Baud Rates:
// - Serial Studio: 115200 baud
// - GPS Module: 9600 baud
//

#include <TinyGPS.h>
#include <SoftwareSerial.h>

// Initialize a TinyGPS parser instance
TinyGPS gps;

// Set up the serial port for GPS communication (using Serial1 for TX/RX pins)
//HardwareSerial &gpsSerial = Serial1;
SoftwareSerial gpsSerial(2, 3); // For Arduino UNO/Nano

// Variables to store GPS position data
float latitude = 0.0;
float altitude = 0.0;
float longitude = 0.0;

void setup() {
  // Start the primary serial port (Serial Monitor) at a baud rate of 115200
  Serial.begin(115200);

  // Start the GPS serial port at a baud rate of 9600
  gpsSerial.begin(9600);

  // Print an initialization message
  Serial.println("GPS Data Reading Started...");
}

void loop() {
  // Read data from the GPS module and encode it using TinyGPS
  while (gpsSerial.available() > 0) {
    char receivedChar = gpsSerial.read();
    gps.encode(receivedChar);
  }

  // Obtain the current position from the GPS module
  unsigned long fixAge;
  long rawLatitude, rawLongitude;
  gps.get_position(&rawLatitude, &rawLongitude, &fixAge);

  // If valid latitude and longitude are received, update the global variables
  if (rawLatitude != TinyGPS::GPS_INVALID_ANGLE) {
    // Convert from TinyGPS format to standard decimal degrees
    latitude = rawLatitude / 1000000.0;
    longitude = rawLongitude / 1000000.0;

    // Get altitude if it is valid, otherwise set altitude to 0
    if (gps.f_altitude() != TinyGPS::GPS_INVALID_F_ALTITUDE) {
      altitude = gps.f_altitude();
    } else {
      altitude = 0.0;
    }
  }

  // Transmit the position data to Serial Studio
  Serial.print('$');           // Frame start sequence
  Serial.print(latitude, 6);   // Latitude with 6 decimal places (index = 1)
  Serial.print(',');           // Frame separator
  Serial.print(longitude, 6);  // Longitude with 6 decimal places (index = 2)
  Serial.print(',');           // Frame separator
  Serial.print(altitude, 2);   // Altitude with 2 decimal places (index = 3)
  Serial.println(';');         // End of the data packet

  // Wait a little
  delay(5);
}