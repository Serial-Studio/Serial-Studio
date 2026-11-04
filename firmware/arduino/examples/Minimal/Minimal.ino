/*
 * Serial Studio device library: the smallest declared sketch.
 *
 * Open Serial Studio in Quick Plot mode and connect. The dashboard builds itself from the
 * declaration this sketch sends at power-up and whenever the app asks for it.
 *
 * SPDX-License-Identifier: MIT
 */

#include <SerialStudio.h>

SerialStudio studio(Serial, F("Minimal"));

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  studio.poll();

  studio.group(F("Analog"));
  studio.channel(F("A0"), analogRead(A0)).units(F("counts")).range(0, 1023);
  studio.channel(F("Uptime"), millis() / 1000.0, 1).units(F("s")).graph(false);
  studio.send();

  delay(50);
}
