//
// Serial Studio Project Tutorial sketch
//
// Author: Alex Spataru
//
// Sends one frame of five values ten times a second, in the format the Project File tutorial
// builds a dashboard for:
//
//   $temperature,humidity,ax,ay,az;
//
// The values are synthesized with sine waves, so the sketch runs on any Arduino-compatible board
// with nothing wired to it. "$" starts a frame and ";" ends it; the values are comma separated.
//
// Baud rate: 115200
//

const unsigned long kIntervalMs = 100;

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  const float t = millis() / 1000.0;

  const float temperature = 24.0 + 3.0 * sin(t * 0.2);
  const float humidity    = 55.0 + 10.0 * sin(t * 0.1 + 1.0);
  const float ax          = 2.0 * sin(t * 1.5);
  const float ay          = 2.0 * cos(t * 1.5);
  const float az          = 9.81 + 0.3 * sin(t * 3.0);

  Serial.print('$');
  Serial.print(temperature, 2);
  Serial.print(',');
  Serial.print(humidity, 2);
  Serial.print(',');
  Serial.print(ax, 3);
  Serial.print(',');
  Serial.print(ay, 3);
  Serial.print(',');
  Serial.print(az, 3);
  Serial.println(';');

  delay(kIntervalMs);
}
