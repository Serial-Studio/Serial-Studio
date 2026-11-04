/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

/* Host stub of the Arduino core: just enough of Stream, F() and PROGMEM for SerialStudio.h. */

#ifndef HOST_ARDUINO_H
#define HOST_ARDUINO_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

class __FlashStringHelper;

#define F(text) (reinterpret_cast<const __FlashStringHelper*>(text))
#define PSTR(text) (text)
#define PROGMEM
#define pgm_read_byte(address) (*reinterpret_cast<const uint8_t*>(address))
#define strncmp_P strncmp

class Print
{
public:
  virtual ~Print() {}
  virtual size_t write(uint8_t byte) = 0;

  size_t print(const char* text)
  {
    size_t n = 0;
    while (text[n])
      write(static_cast<uint8_t>(text[n++]));

    return n;
  }

  size_t print(const __FlashStringHelper* text)
  {
    return print(reinterpret_cast<const char*>(text));
  }

  size_t print(long value)
  {
    char text[32];
    snprintf(text, sizeof text, "%ld", value);
    return print(text);
  }

  size_t print(unsigned long value)
  {
    char text[32];
    snprintf(text, sizeof text, "%lu", value);
    return print(text);
  }

  size_t print(int value)
  {
    return print(static_cast<long>(value));
  }

  size_t print(unsigned int value)
  {
    return print(static_cast<unsigned long>(value));
  }

  size_t print(double value, int digits = 2)
  {
    char text[48];
    snprintf(text, sizeof text, "%.*f", digits, value);
    return print(text);
  }
};

class Stream : public Print
{
public:
  virtual int available() = 0;
  virtual int read() = 0;
};

#endif
