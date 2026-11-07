/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

/* Host driver: prints a golden sketch's boot declaration and frames to stdout. */

#include <stdio.h>
#include <string.h>

#include "serialstudio.h"

static const double mpu_rows[3][7] = {{0.12, -0.34, 9.81, 0.01, -0.02, 0.03, 24.5},
                                      {0.15, -0.31, 9.79, 0.02, -0.01, 0.03, 24.56},
                                      {-0.08, 0.22, 9.83, -0.05, 0.04, -0.01, 24.61}};
static const double gps_rows[3][3] = {{48.8584, 2.2945, 35.0}, {48.8585, 2.2946, 35.4}, {48.8586, 2.2947, 0}};
static const char* adc_rows[3][6] = {{"0,25", "1,50", "2,75", "3,00", "4,25", "4,90"},
                                     {"0,30", "1,55", "2,80", "3,05", "4,30", "4,95"},
                                     {"0,35", "1,60", "2,85", "3,10", "4,35", "5,00"}};
static const double ctl_rows[3][3] = {{3, 87.5, 1.2}, {3, 92.0, 1.25}, {1, 0.0, 1.25}};
static const long fmt_rows[3][3] = {{16384, 8, 1200}, {8192, 0, 1210}, {-4096, 8, 1190}};
static const double level_rows[4] = {1.5, 1.6, 1.7, 1.8};
static const long door_rows[3][2] = {{1, 0}, {0, 1}, {1, 1}};
static const char* adc_names[6] = {"ADC 0", "ADC 1", "ADC 2", "ADC 3", "ADC 4", "ADC 5"};

static int hex_mode;

static void out(void* user, const char* data, size_t len)
{
  size_t i;
  (void)user;
  if (!hex_mode)
  {
    fwrite(data, 1, len, stdout);
    return;
  }

  for (i = 0; i < len; ++i)
    printf("%02x", (unsigned char)data[i]);
}

static void mpu6050(ss_t* ss, int row)
{
  static const char* acc[3] = {"Accelerometer X", "Accelerometer Y", "Accelerometer Z"};
  static const char* gyro[3] = {"Gyro X", "Gyro Y", "Gyro Z"};
  static const char* axis[3] = {"x", "y", "z"};
  int i;

  ss_group(ss, "Accelerometer", "accelerometer");
  for (i = 0; i < 3; ++i)
  {
    ss_channel_double(ss, acc[i], mpu_rows[row][i], 2);
    ss_units(ss, "m/s\xc2\xb2");
    ss_widget(ss, axis[i]);
  }

  ss_group(ss, "Gyroscope", "gyro");
  for (i = 0; i < 3; ++i)
  {
    ss_channel_double(ss, gyro[i], mpu_rows[row][3 + i], 2);
    ss_units(ss, "deg/s");
    ss_widget(ss, axis[i]);
  }

  ss_group(ss, "Temperature", NULL);
  ss_channel_double(ss, "Temperature", mpu_rows[row][6], 2);
  ss_units(ss, "\xe2\x84\x83");
  ss_widget(ss, "bar");
  ss_range(ss, 0, 100);
  ss_graph(ss, 0);
}

static void tinygps(ss_t* ss, int row)
{
  ss_group(ss, "GPS Map", "gps");
  ss_channel_double(ss, "Latitude", gps_rows[row][0], 4);
  ss_units(ss, "\xc2\xb0");
  ss_widget(ss, "lat");
  ss_graph(ss, 0);
  ss_channel_double(ss, "Longitude", gps_rows[row][1], 4);
  ss_units(ss, "\xc2\xb0");
  ss_widget(ss, "lon");
  ss_graph(ss, 0);
  if (row == 2)
    ss_channel(ss, "Altitude");
  else
    ss_channel_double(ss, "Altitude", gps_rows[row][2], 1);

  ss_units(ss, "m");
  ss_widget(ss, "alt");
  ss_graph(ss, 0);
}

static void hexadc(ss_t* ss, int row)
{
  int i;
  ss_group(ss, "ADC Readings", "multiplot");
  for (i = 0; i < 6; ++i)
  {
    ss_channel_str(ss, adc_names[i], adc_rows[row][i]);
    ss_units(ss, "Volts");
    ss_plot_max(ss, 5);
    ss_fft(ss, 1);
    ss_fft_sampling_rate(ss, 200);
    ss_graph(ss, 0);
  }

  ss_group(ss, "ADC Readings", "datagrid");
  for (i = 0; i < 6; ++i)
  {
    ss_channel(ss, adc_names[i]);
    ss_units(ss, "Volts");
    ss_index(ss, i + 1);
    ss_graph(ss, 0);
  }
}

static void controls(ss_t* ss, int row)
{
  ss_group(ss, "Status", NULL);
  ss_channel_int(ss, "Gear", (long)ctl_rows[row][0]);
  ss_label(ss, 0, "Park");
  ss_label(ss, 1, "Reverse");
  ss_label(ss, 2, "Neutral");
  ss_label(ss, 3, "Drive");
  ss_labels_default(ss, "Unknown");
  ss_graph(ss, 0);
  ss_channel_double(ss, "Speed", ctl_rows[row][1], 1);
  ss_units(ss, "km/h");
  ss_widget(ss, "gauge");
  ss_widget_min(ss, 0);
  ss_widget_max(ss, 200);
  ss_alarm(ss, 150, 200, 2, "Overspeed");
  ss_graph(ss, 0);
  ss_channel_double(ss, "Tank 2 Pressure", ctl_rows[row][2], 2);
  ss_units(ss, "bar");
  ss_control_group(ss, "Controls");
  ss_control(ss, "Start", SS_CONTROL_BUTTON);
  ss_control(ss, "Pump", SS_CONTROL_TOGGLE);
  ss_on_label(ss, "On");
  ss_off_label(ss, "Off");
  ss_control(ss, "Setpoint", SS_CONTROL_SLIDER);
  ss_output_min(ss, 0);
  ss_output_max(ss, 255);
  ss_output_step(ss, 1);
  ss_control(ss, "Gain", SS_CONTROL_KNOB);
  ss_output_min(ss, 0);
  ss_output_max(ss, 10);
  ss_output_step(ss, 0.5);
}

static void formats(ss_t* ss, int row)
{
  ss_group(ss, "Decoded", NULL);
  ss_channel_int(ss, "Accel", fmt_rows[row][0]);
  ss_units(ss, "g");
  ss_format(ss, SS_FORMAT_FIXED);
  ss_format_width(ss, 16);
  ss_format_signed(ss, 1);
  ss_format_fraction_bits(ss, 15);
  ss_channel_int(ss, "Door", fmt_rows[row][1]);
  ss_format(ss, SS_FORMAT_BITS);
  ss_format_width(ss, 8);
  ss_format_bit_offset(ss, 3);
  ss_format_bit_width(ss, 1);
  ss_label(ss, 0, "Closed");
  ss_label(ss, 1, "Open");
  ss_graph(ss, 0);
  ss_channel_int(ss, "Temp", fmt_rows[row][2]);
  ss_units(ss, "C");
  ss_format(ss, SS_FORMAT_SCALE);
  ss_format_scale(ss, 0.0625);
  ss_format_offset(ss, -40);
}

static void textfield(ss_t* ss, int row)
{
  ss_group(ss, "Status", NULL);
  ss_channel_double(ss, "Level", level_rows[row], 2);
  ss_units(ss, "m");
  ss_control_group(ss, "Tuning");
  ss_control(ss, "Frequency", SS_CONTROL_TEXT);
  ss_output_min(ss, 0);
  ss_output_max(ss, 50);
  ss_output_units(ss, "Hz");
  ss_output_decimals(ss, 1);
}

static int on_tuner(void* user, const char* name, double value)
{
  (void)name;
  if (value > 50)
  {
    ss_reply_message((ss_t*)user, "out of range");
    return 2;
  }

  return 0;
}

static void ledpanel(ss_t* ss, int row)
{
  ss_group(ss, "Doors", SS_GROUP_WIDGET_LEDPANEL);
  ss_channel_int(ss, "Front Left", door_rows[row][0]);
  ss_led_high(ss, 1);
  ss_graph(ss, 0);
  ss_channel_int(ss, "Trunk", door_rows[row][1]);
  ss_led_high(ss, 1);
  ss_graph(ss, 0);
}

int main(int argc, char** argv)
{
  static const char* titles[] = {"MPU6050", "TinyGPS", "Hexadecimal ADC", "Pump Controller",
                                 "Formats", "MPU6050", "Tuner", "Body Control"};
  static const char* names[] = {"mpu6050", "tinygps", "hexadc", "controls",
                                "formats", "mpu6050_bin", "textfield", "ledpanel"};
  static void (*sketches[])(ss_t*, int) = {mpu6050, tinygps, hexadc, controls,
                                           formats, mpu6050, textfield, ledpanel};
  static const uint8_t start[2] = {0xAA, 0x55};
  ss_t ss;
  int pick = -1;
  int rows = 3;
  int i;

  for (i = 0; i < 8 && argc > 1; ++i)
    if (strcmp(argv[1], names[i]) == 0)
      pick = i;

  if (pick < 0)
  {
    fprintf(stderr, "usage: c_golden <sketch>\n");
    return 2;
  }

  ss_init(&ss, out, &ss, titles[pick]);
  if (pick == 1)
    ss_set_named(&ss, 1);

  if (pick == 2)
    ss_set_separator(&ss, ';');

  if (pick == 5 && ss_set_binary(&ss, "f32,f32,f32,f32,f32,f32,f32", 0, start, 2, 1) != 0)
    return 3;

  if (pick == 6)
  {
    ss_set_handlers(&ss, on_tuner, NULL);
    rows = 4;
  }

  sketches[pick](&ss, 0);
  ss_send(&ss);
  hex_mode = pick == 5;
  for (i = 0; i < rows; ++i)
  {
    if (pick == 6 && i == 1)
      ss_feed(&ss, "#ss!Frequency=12@7\n", 19);

    if (pick == 6 && i == 2)
      ss_feed(&ss, "#ss!Frequency=99@8\n", 19);

    sketches[pick](&ss, i);
    ss_send(&ss);
    if (hex_mode)
      putchar('\n');
  }

  return 0;
}
