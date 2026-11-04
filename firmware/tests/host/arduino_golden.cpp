/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

// Host driver: prints a golden sketch's boot declaration and frames to stdout.

#include <stdio.h>
#include <string.h>

#include <Arduino.h>
#include <SerialStudio.h>

namespace
{
const double mpuRows[3][7] = {{0.12, -0.34, 9.81, 0.01, -0.02, 0.03, 24.5},
                              {0.15, -0.31, 9.79, 0.02, -0.01, 0.03, 24.56},
                              {-0.08, 0.22, 9.83, -0.05, 0.04, -0.01, 24.61}};
const double gpsRows[3][3] = {{48.8584, 2.2945, 35.0}, {48.8585, 2.2946, 35.4}, {48.8586, 2.2947, 0}};
const char* const adcRows[3][6] = {{"0,25", "1,50", "2,75", "3,00", "4,25", "4,90"},
                                   {"0,30", "1,55", "2,80", "3,05", "4,30", "4,95"},
                                   {"0,35", "1,60", "2,85", "3,10", "4,35", "5,00"}};
const double ctlRows[3][3] = {{3, 87.5, 1.2}, {3, 92.0, 1.25}, {1, 0.0, 1.25}};
const long fmtRows[3][3] = {{16384, 8, 1200}, {8192, 0, 1210}, {-4096, 8, 1190}};
const double levelRows[4] = {1.5, 1.6, 1.7, 1.8};
const char* const adcNames[6] = {"ADC 0", "ADC 1", "ADC 2", "ADC 3", "ADC 4", "ADC 5"};

class HostStream : public Stream
{
public:
  bool hex = false;

  size_t write(uint8_t byte) override
  {
    if (hex)
      printf("%02x", byte);
    else
      fputc(byte, stdout);

    return 1;
  }

  int available() override
  {
    return 0;
  }

  int read() override
  {
    return -1;
  }
};

void mpu6050(SerialStudio& ss, int row)
{
  const char* const acc[3] = {"Accelerometer X", "Accelerometer Y", "Accelerometer Z"};
  const char* const gyro[3] = {"Gyro X", "Gyro Y", "Gyro Z"};
  const char* const axis[3] = {"x", "y", "z"};

  ss.group(F("Accelerometer"), F("accelerometer"));
  for (int i = 0; i < 3; ++i)
    ss.channel(acc[i], mpuRows[row][i]).units(F("m/s\xc2\xb2")).widget(axis[i]);

  ss.group(F("Gyroscope"), F("gyro"));
  for (int i = 0; i < 3; ++i)
    ss.channel(gyro[i], mpuRows[row][3 + i]).units(F("deg/s")).widget(axis[i]);

  ss.group(F("Temperature"));
  ss.channel(F("Temperature"), mpuRows[row][6])
    .units(F("\xe2\x84\x83"))
    .widget(F("bar"))
    .range(0, 100)
    .graph(false);
}

void tinygps(SerialStudio& ss, int row)
{
  ss.group(F("GPS Map"), F("gps"));
  ss.channel(F("Latitude"), gpsRows[row][0], 4).units(F("\xc2\xb0")).widget(F("lat")).graph(false);
  ss.channel(F("Longitude"), gpsRows[row][1], 4).units(F("\xc2\xb0")).widget(F("lon")).graph(false);
  SerialStudio& altitude =
    row == 2 ? ss.channel(F("Altitude")) : ss.channel(F("Altitude"), gpsRows[row][2], 1);
  altitude.units(F("m")).widget(F("alt")).graph(false);
}

void hexadc(SerialStudio& ss, int row)
{
  ss.group(F("ADC Readings"), F("multiplot"));
  for (int i = 0; i < 6; ++i)
    ss.channel(adcNames[i], adcRows[row][i])
      .units(F("Volts"))
      .plotMax(5)
      .fft(true)
      .fftSamplingRate(200)
      .graph(false);

  ss.group(F("ADC Readings"), F("datagrid"));
  for (int i = 0; i < 6; ++i)
    ss.channel(adcNames[i]).units(F("Volts")).index(i + 1).graph(false);
}

void controls(SerialStudio& ss, int row)
{
  ss.group(F("Status"));
  ss.channel(F("Gear"), static_cast<long>(ctlRows[row][0]))
    .label(0, F("Park"))
    .label(1, F("Reverse"))
    .label(2, F("Neutral"))
    .label(3, F("Drive"))
    .labelsDefault(F("Unknown"))
    .graph(false);
  ss.channel(F("Speed"), ctlRows[row][1], 1)
    .units(F("km/h"))
    .widget(F("gauge"))
    .widgetMin(0)
    .widgetMax(200)
    .alarm(150, 200, 2, F("Overspeed"))
    .graph(false);
  ss.channel(F("Tank 2 Pressure"), ctlRows[row][2]).units(F("bar"));
  ss.controlGroup(F("Controls"));
  ss.control(F("Start"), SS_CONTROL_BUTTON);
  ss.control(F("Pump"), SS_CONTROL_TOGGLE).onLabel(F("On")).offLabel(F("Off"));
  ss.control(F("Setpoint"), SS_CONTROL_SLIDER).outputMin(0).outputMax(255).outputStep(1);
  ss.control(F("Gain"), SS_CONTROL_KNOB).outputMin(0).outputMax(10).outputStep(0.5);
}

void formats(SerialStudio& ss, int row)
{
  ss.group(F("Decoded"));
  ss.channel(F("Accel"), fmtRows[row][0])
    .units(F("g"))
    .format(SS_FORMAT_FIXED)
    .formatWidth(16)
    .formatSigned(true)
    .formatFractionBits(15);
  ss.channel(F("Door"), fmtRows[row][1])
    .format(SS_FORMAT_BITS)
    .formatWidth(8)
    .formatBitOffset(3)
    .formatBitWidth(1)
    .label(0, F("Closed"))
    .label(1, F("Open"))
    .graph(false);
  ss.channel(F("Temp"), fmtRows[row][2])
    .units(F("C"))
    .format(SS_FORMAT_SCALE)
    .formatScale(0.0625)
    .formatOffset(-40);
}

void textfield(SerialStudio& ss, int row)
{
  ss.group(F("Status"));
  ss.channel(F("Level"), levelRows[row]).units(F("m"));
  ss.controlGroup(F("Tuning"));
  ss.control(F("Frequency"), SS_CONTROL_TEXT)
    .outputMin(0)
    .outputMax(50)
    .outputUnits(F("Hz"))
    .outputDecimals(1);
}

SerialStudio* tuner = nullptr;

int onTuner(const char*, double value)
{
  if (value > 50)
  {
    tuner->replyMessage("out of range");
    return 2;
  }

  return 0;
}

void (*const sketches[])(SerialStudio&, int) = {mpu6050, tinygps, hexadc, controls,
                                                 formats, mpu6050, textfield};
const char* const names[] = {"mpu6050", "tinygps", "hexadc", "controls",
                             "formats", "mpu6050_bin", "textfield"};
const char* const titles[] = {"MPU6050", "TinyGPS", "Hexadecimal ADC", "Pump Controller",
                              "Formats", "MPU6050", "Tuner"};
class FeedStream : public HostStream
{
public:
  const char* pending = nullptr;

  int available() override
  {
    return pending && *pending ? 1 : 0;
  }

  int read() override
  {
    return pending && *pending ? *pending++ : -1;
  }
};
} // namespace

int main(int argc, char** argv)
{
  int pick = -1;
  for (int i = 0; i < 7 && argc > 1; ++i)
    if (strcmp(argv[1], names[i]) == 0)
      pick = i;

  if (pick < 0)
  {
    fprintf(stderr, "usage: arduino_golden <sketch>\n");
    return 2;
  }

  FeedStream stream;
  SerialStudio ss(stream, titles[pick]);
  static const uint8_t start[2] = {0xAA, 0x55};
  int rows = 3;
  if (pick == 1)
    ss.named(true);

  if (pick == 2)
    ss.separator(';');

  if (pick == 5)
    ss.binary("f32,f32,f32,f32,f32,f32,f32", false, start, 2, true);

  if (pick == 6)
  {
    tuner = &ss;
    ss.onControl(onTuner);
    rows = 4;
  }

  sketches[pick](ss, 0);
  ss.send();
  stream.hex = pick == 5;
  for (int row = 0; row < rows; ++row)
  {
    if (pick == 6 && row == 1)
      stream.pending = "#ss!Frequency=12@7\n";

    if (pick == 6 && row == 2)
      stream.pending = "#ss!Frequency=99@8\n";

    ss.poll();
    sketches[pick](ss, row);
    ss.send();
    if (stream.hex)
      putchar('\n');
  }

  return 0;
}
