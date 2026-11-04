//
// GPS Data Logger Program, declared variant
//
// This is TinyGPS.ino with the dashboard declared by the sketch itself. The SerialStudio
// library sends the GPS Map group and its titles and units at power-up and whenever Serial
// Studio asks, so no project file is needed: set Operation Mode to Quick Plot and connect.
//
// Required Libraries:
// - TinyGPS (Install via Arduino Library Manager)
// - SerialStudio (this repository, firmware/arduino)
//
// Connections:
// - Connect the GPS module's TX pin to Arduino pin 2 (SoftwareSerial RX) and the
//   module's RX pin to Arduino pin 3 (SoftwareSerial TX). On boards with a spare
//   hardware UART (Leonardo, Mega, ...), you can use Serial1 instead: comment out
//   the SoftwareSerial line below and uncomment the HardwareSerial one.
// - Ensure your GPS module operates at 9600 baud for compatibility.
//
// Baud Rates:
// - Serial Studio: 115200 baud
// - GPS Module: 9600 baud
//

#include <SerialStudio.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

// Written against version 1 of the SerialStudio library
#if SS_LIBRARY_VERSION_MAJOR != 1
#error This sketch needs version 1 of the SerialStudio library
#endif

TinyGPS gps;
SerialStudio studio(Serial, F("TinyGPS"));

//HardwareSerial &gpsSerial = Serial1;
SoftwareSerial gpsSerial(2, 3); // For Arduino UNO/Nano

float latitude = 0.0;
float altitude = 0.0;
float longitude = 0.0;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);
}

void loop() {
  while (gpsSerial.available() > 0) {
    char receivedChar = gpsSerial.read();
    gps.encode(receivedChar);
  }

  unsigned long fixAge;
  long rawLatitude, rawLongitude;
  gps.get_position(&rawLatitude, &rawLongitude, &fixAge);

  if (rawLatitude != TinyGPS::GPS_INVALID_ANGLE) {
    latitude = rawLatitude / 1000000.0;
    longitude = rawLongitude / 1000000.0;

    if (gps.f_altitude() != TinyGPS::GPS_INVALID_F_ALTITUDE) {
      altitude = gps.f_altitude();
    } else {
      altitude = 0.0;
    }
  }

  // Answers the app's request for the declaration
  studio.poll();

  studio.group(F("GPS Map"), F("gps"));
  studio.channel(F("Latitude"), latitude, 6).units(F("\xc2\xb0")).widget(F("lat")).graph(false);
  studio.channel(F("Longitude"), longitude, 6).units(F("\xc2\xb0")).widget(F("lon")).graph(false);
  studio.channel(F("Altitude"), altitude, 2).units(F("m")).widget(F("alt")).graph(false);

  // Writes the declaration on the first pass and after a request, a frame on every other pass
  studio.send();

  delay(5);
}
