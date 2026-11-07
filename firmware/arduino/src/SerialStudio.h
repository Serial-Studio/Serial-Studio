/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Declare a Serial Studio dashboard from a sketch and stream its frames.
 *
 * The same channel() call that prints a value also declares it: when a declaration
 * is due (power-up, or the app's "#ss?" request) one loop pass writes the declaration
 * line instead of a frame, and every other pass writes a frame. Header-only, no heap,
 * every fixed text in flash.
 */

#ifndef SERIAL_STUDIO_H
#define SERIAL_STUDIO_H

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "ss_vocabulary.h"

#ifndef SS_REPLY_BYTES
#define SS_REPLY_BYTES 24
#endif

#ifndef SS_INPUT_BYTES
#define SS_INPUT_BYTES 48
#endif

enum SerialStudioFormat
{
  SS_FORMAT_NONE = 0,
  SS_FORMAT_FIXED = 1,
  SS_FORMAT_BITS = 2,
  SS_FORMAT_SCALE = 3
};

enum SerialStudioControl
{
  SS_CONTROL_BUTTON = 0,
  SS_CONTROL_SLIDER = 1,
  SS_CONTROL_TOGGLE = 2,
  SS_CONTROL_TEXT = 3,
  SS_CONTROL_KNOB = 4
};

template<class S>
class SerialStudioT
{
public:
  /* Returns the reply code of a tokened command: 0 accepted, anything else refused. */
  typedef int (*ControlFn)(const char* name, double value);
  typedef void (*LineFn)(const char* line);

  SerialStudioT(S& port, const char* title) : m_port(port), m_title(title), m_f()
  {
    m_f.due = 1;
  }

  SerialStudioT(S& port, const __FlashStringHelper* title) : m_port(port), m_title(title), m_f()
  {
    m_f.due = 1;
    m_f.titleFlash = 1;
  }

  void separator(char c)
  {
    m_sep = c;
  }

  void named(bool on)
  {
    m_f.named = on;
  }

  void onControl(ControlFn fn)
  {
    m_control = fn;
  }

  void onLine(LineFn fn)
  {
    m_line = fn;
  }

  void requestDeclaration()
  {
    m_f.due = 1;
  }

  /* Packed binary frames: fields is a CSV of i8..u64, f32, f64 in channel order. The start
     bytes must stay valid (a static array); up to four, written before every frame. */
  SerialStudioT& binary(const __FlashStringHelper* fields, bool bigEndian, const uint8_t* start,
                        uint8_t startLen, bool crc16 = true)
  {
    return setBinary(fields, true, bigEndian, start, startLen, crc16);
  }

  SerialStudioT& binary(const char* fields, bool bigEndian, const uint8_t* start,
                        uint8_t startLen, bool crc16 = true)
  {
    return setBinary(fields, false, bigEndian, start, startLen, crc16);
  }

  /* Call from inside the control callback to set the reply message of a tokened command. */
  void replyMessage(const char* text)
  {
    uint8_t i = 0;
    for (; i < SS_REPLY_BYTES - 1 && text[i]; ++i)
    {
      char c = text[i];
      bool unsafe = c == m_sep || c == ',' || c == ';' || c == '\n' || c == '\r';
      m_msg[i] = unsafe ? ' ' : c;
    }

    m_msg[i] = 0;
  }

  template<class T>
  SerialStudioT& group(T title)
  {
    openGroup(false);
    if (m_f.decl && m_f.group)
      str(title);

    return *this;
  }

  template<class T, class W>
  SerialStudioT& group(T title, W widget)
  {
    group(title);
    if (m_f.decl && m_f.group)
    {
      m_port.print(F(",\"" SS_KEY_WIDGET "\":"));
      str(widget);
    }

    return *this;
  }

  template<class T>
  SerialStudioT& controlGroup(T title)
  {
    openGroup(true);
    if (m_f.decl && m_f.group)
      str(title);

    return *this;
  }

  template<class N>
  SerialStudioT& channel(N name)
  {
    begin();
    if (m_f.decl)
      openItem(name, 1);

    return *this;
  }

  template<class N, class V>
  SerialStudioT& channel(N name, V value, uint8_t places = 2)
  {
    begin();
    if (m_f.decl)
    {
      openItem(name, 1);
      return *this;
    }

    if (m_f.bin)
    {
      pack(number(value));
      return *this;
    }

    if (m_f.val)
      m_port.write((uint8_t)m_sep);

    if (m_f.named)
    {
      raw(name);
      m_port.write((uint8_t)':');
    }

    putValue(value, places);
    m_f.val = 1;
    return *this;
  }

  template<class T>
  SerialStudioT& control(T title, SerialStudioControl kind)
  {
    begin();
    if (m_f.decl && m_f.group && m_f.ctlgrp)
    {
      openItem(title, 2);
      if (m_f.item)
      {
        m_f.hasCtl = 1;
        m_port.print(F(",\"" SS_KEY_OUTPUT_TYPE "\":"));
        m_port.print((int)kind);
      }
    }

    return *this;
  }

  template<class V>
  SerialStudioT& prop(const __FlashStringHelper* key, V value)
  {
    return put(key, value, 1);
  }

  template<class V>
  SerialStudioT& units(V v)
  {
    return put(F(SS_KEY_UNITS), v, 1);
  }

  template<class V>
  SerialStudioT& widget(V v)
  {
    return put(F(SS_KEY_WIDGET), v, 1);
  }

  template<class V>
  SerialStudioT& alias(V v)
  {
    return put(F(SS_KEY_ALIAS), v, 1);
  }

  template<class V>
  SerialStudioT& color(V v)
  {
    return put(F(SS_KEY_COLOR), v, 1);
  }

  template<class V>
  SerialStudioT& labelsDefault(V v)
  {
    return put(F(SS_KEY_LABELS_DEFAULT), v, 1);
  }

  SerialStudioT& index(long v)
  {
    return put(F(SS_KEY_INDEX), v, 1);
  }

  SerialStudioT& plotMin(double v)
  {
    return put(F(SS_KEY_PLOT_MIN), v, 1);
  }

  SerialStudioT& plotMax(double v)
  {
    return put(F(SS_KEY_PLOT_MAX), v, 1);
  }

  SerialStudioT& widgetMin(double v)
  {
    return put(F(SS_KEY_WIDGET_MIN), v, 1);
  }

  SerialStudioT& widgetMax(double v)
  {
    return put(F(SS_KEY_WIDGET_MAX), v, 1);
  }

  SerialStudioT& range(double low, double high)
  {
    widgetMin(low);
    widgetMax(high);
    plotMin(low);
    return plotMax(high);
  }

  SerialStudioT& fft(bool v)
  {
    return put(F(SS_KEY_FFT), v, 1);
  }

  SerialStudioT& fftSamplingRate(long v)
  {
    return put(F(SS_KEY_FFT_SAMPLING_RATE), v, 1);
  }

  SerialStudioT& log(bool v)
  {
    return put(F(SS_KEY_LOG), v, 1);
  }

  SerialStudioT& ledHigh(double v)
  {
    return put(F(SS_KEY_LED_HIGH), v, 1);
  }

  SerialStudioT& decimalPoints(long v)
  {
    return put(F(SS_KEY_DECIMAL_POINTS), v, 1);
  }

  SerialStudioT& hideOnDashboard(bool v)
  {
    return put(F(SS_KEY_HIDE_ON_DASHBOARD), v, 1);
  }

  SerialStudioT& format(SerialStudioFormat v)
  {
    return put(F(SS_KEY_FORMAT), (long)v, 1);
  }

  SerialStudioT& formatWidth(long v)
  {
    return put(F(SS_KEY_FORMAT_WIDTH), v, 1);
  }

  SerialStudioT& formatSigned(bool v)
  {
    return put(F(SS_KEY_FORMAT_SIGNED), v, 1);
  }

  SerialStudioT& formatFractionBits(long v)
  {
    return put(F(SS_KEY_FORMAT_FRACTION_BITS), v, 1);
  }

  SerialStudioT& formatBitOffset(long v)
  {
    return put(F(SS_KEY_FORMAT_BIT_OFFSET), v, 1);
  }

  SerialStudioT& formatBitWidth(long v)
  {
    return put(F(SS_KEY_FORMAT_BIT_WIDTH), v, 1);
  }

  SerialStudioT& formatScale(double v)
  {
    return put(F(SS_KEY_FORMAT_SCALE), v, 1);
  }

  SerialStudioT& formatOffset(double v)
  {
    return put(F(SS_KEY_FORMAT_OFFSET), v, 1);
  }

  SerialStudioT& graph(bool on)
  {
    if (m_f.item == 1)
      m_f.graph = on;

    return *this;
  }

  template<class T>
  SerialStudioT& label(long value, T text)
  {
    if (!(m_f.decl && m_f.item == 1))
      return *this;

    openArray(2, F(SS_KEY_LABELS));
    m_port.print(F("{\"" SS_KEY_VALUE "\":"));
    m_port.print(value);
    m_port.print(F(",\"" SS_KEY_TEXT "\":"));
    str(text);
    m_port.write((uint8_t)'}');
    return *this;
  }

  template<class T>
  SerialStudioT& alarm(double low, double high, long severity, T text)
  {
    if (!(m_f.decl && m_f.item == 1))
      return *this;

    openArray(1, F(SS_KEY_ALARM_BANDS));
    m_port.print(F("{\"" SS_KEY_MIN "\":"));
    num(low);
    m_port.print(F(",\"" SS_KEY_MAX "\":"));
    num(high);
    m_port.print(F(",\"" SS_KEY_SEVERITY "\":"));
    m_port.print(severity);
    m_port.print(F(",\"" SS_KEY_LABEL "\":"));
    str(text);
    m_port.write((uint8_t)'}');
    return *this;
  }

  template<class V>
  SerialStudioT& outputUnits(V v)
  {
    return put(F(SS_KEY_OUTPUT_UNITS), v, 2);
  }

  SerialStudioT& outputDecimals(long v)
  {
    return put(F(SS_KEY_OUTPUT_DECIMALS), v, 2);
  }

  SerialStudioT& quantizeToStep(bool v)
  {
    return put(F(SS_KEY_QUANTIZE_TO_STEP), v, 2);
  }

  SerialStudioT& outputMin(double v)
  {
    return put(F(SS_KEY_OUTPUT_MIN), v, 2);
  }

  SerialStudioT& outputMax(double v)
  {
    return put(F(SS_KEY_OUTPUT_MAX), v, 2);
  }

  SerialStudioT& outputStep(double v)
  {
    return put(F(SS_KEY_OUTPUT_STEP), v, 2);
  }

  SerialStudioT& initialValue(double v)
  {
    return put(F(SS_KEY_INITIAL_VALUE), v, 2);
  }

  SerialStudioT& checkable(bool v)
  {
    return put(F(SS_KEY_CHECKABLE), v, 2);
  }

  template<class V>
  SerialStudioT& onLabel(V v)
  {
    return put(F(SS_KEY_ON_LABEL), v, 2);
  }

  template<class V>
  SerialStudioT& offLabel(V v)
  {
    return put(F(SS_KEY_OFF_LABEL), v, 2);
  }

  void send()
  {
    if (!m_f.pass)
      return;

    if (m_f.decl)
    {
      closeGroup();
      m_port.print(F("]}"));
      m_port.write((uint8_t)'\n');
      m_f.due = 0;
    }
    else if (m_f.val && m_f.bin)
    {
      if (m_f.crcOn)
      {
        m_port.write((uint8_t)(m_crc >> 8));
        m_port.write((uint8_t)(m_crc & 0xFF));
      }
    }
    else if (m_f.val)
    {
      if (m_f.hasCtl && m_f.hasReply)
        putReply();

      m_port.write((uint8_t)'\n');
    }

    m_f.pass = 0;
    m_f.decl = 0;
    m_f.val = 0;
    m_f.group = 0;
    m_f.item = 0;
    m_f.arr = 0;
    m_groups = 0;
    m_total = 0;
  }

  void poll()
  {
    while (m_port.available() > 0)
    {
      int c = m_port.read();
      if (c < 0)
        break;

      if (c != '\n' && c != '\r')
      {
        if (m_len < SS_INPUT_BYTES - 1)
          m_buf[m_len++] = (char)c;
        else
          m_f.over = 1;

        continue;
      }

      if (m_len > 0 && !m_f.over)
        handle();

      m_len = 0;
      m_f.over = 0;
    }
  }

private:
  struct Flags
  {
    uint8_t due : 1;
    uint8_t decl : 1;
    uint8_t pass : 1;
    uint8_t group : 1;
    uint8_t ctlgrp : 1;
    uint8_t any : 1;
    uint8_t val : 1;
    uint8_t named : 1;
    uint8_t graph : 1;
    uint8_t over : 1;
    uint8_t titleFlash : 1;
    uint8_t item : 2;
    uint8_t arr : 2;
    uint8_t bin : 1;
    uint8_t big : 1;
    uint8_t crcOn : 1;
    uint8_t fieldsFlash : 1;
    uint8_t hasCtl : 1;
    uint8_t hasReply : 1;
  };

  static char at(const char* p, uint8_t i)
  {
    return p[i];
  }

  static char at(const __FlashStringHelper* p, uint8_t i)
  {
    return (char)pgm_read_byte(reinterpret_cast<const char*>(p) + i);
  }

  static bool isNumberChar(char c)
  {
    return (c >= '0' && c <= '9') || c == '+' || c == '.' || c == '-';
  }

  static bool isInertChar(char c)
  {
    return c == ' ' || c == ',' || c == ';' || c == ':' || c == '=';
  }

  void hex(char c)
  {
    uint8_t high = ((uint8_t)c >> 4) & 15;
    uint8_t low = (uint8_t)c & 15;
    m_port.print(F("\\u00"));
    m_port.write((uint8_t)(high < 10 ? '0' + high : 'a' + high - 10));
    m_port.write((uint8_t)(low < 10 ? '0' + low : 'a' + low - 10));
  }

  template<class P>
  void str(P p)
  {
    m_port.write((uint8_t)'"');
    for (uint8_t i = 0; i < SS_MAX_STRING_BYTES; ++i)
    {
      char c = at(p, i);
      if (!c)
        break;

      if (c == '"' || c == '\\')
      {
        m_port.write((uint8_t)'\\');
        m_port.write((uint8_t)c);
      }
      else if ((uint8_t)c < 0x20 || (isInertChar(c) && isNumberChar(at(p, i + 1))))
        hex(c);
      else
        m_port.write((uint8_t)c);
    }

    m_port.write((uint8_t)'"');
  }

  template<class P>
  void raw(P p)
  {
    for (uint8_t i = 0; i < SS_MAX_STRING_BYTES; ++i)
    {
      char c = at(p, i);
      if (!c)
        break;

      m_port.write((uint8_t)c);
    }
  }

  void num(double v)
  {
    if (v != v)
      v = 0;

    bool negative = v < 0;
    if (negative)
      v = -v;

    if (v > 4294967040.0)
      v = 4294967040.0;

    unsigned long whole = (unsigned long)v;
    unsigned long fraction = (unsigned long)((v - (double)whole) * 1000000.0 + 0.5);
    if (fraction >= 1000000UL)
    {
      whole++;
      fraction -= 1000000UL;
    }

    if (negative && (whole || fraction))
      m_port.write((uint8_t)'-');

    m_port.print(whole);
    if (!fraction)
      return;

    char digits[6];
    for (int8_t i = 5; i >= 0; --i)
    {
      digits[i] = (char)('0' + fraction % 10);
      fraction /= 10;
    }

    int8_t last = 5;
    while (digits[last] == '0')
      --last;

    m_port.write((uint8_t)'.');
    for (int8_t i = 0; i <= last; ++i)
      m_port.write((uint8_t)digits[i]);
  }

  void val(bool v)
  {
    m_port.print(v ? F("true") : F("false"));
  }

  void val(const char* v)
  {
    str(v);
  }

  void val(const __FlashStringHelper* v)
  {
    str(v);
  }

  void val(char* v)
  {
    str(static_cast<const char*>(v));
  }

  template<class V>
  void val(V v)
  {
    num((double)v);
  }

  void putValue(int v, uint8_t)
  {
    m_port.print(v);
  }

  void putValue(unsigned int v, uint8_t)
  {
    m_port.print(v);
  }

  void putValue(long v, uint8_t)
  {
    m_port.print(v);
  }

  void putValue(unsigned long v, uint8_t)
  {
    m_port.print(v);
  }

  void putValue(bool v, uint8_t)
  {
    m_port.write((uint8_t)(v ? '1' : '0'));
  }

  void putValue(float v, uint8_t places)
  {
    m_port.print(v, places);
  }

  void putValue(double v, uint8_t places)
  {
    m_port.print(v, places);
  }

  void putValue(const char* v, uint8_t)
  {
    m_port.print(v);
  }

  void putValue(const __FlashStringHelper* v, uint8_t)
  {
    m_port.print(v);
  }

  void begin()
  {
    if (m_f.pass)
      return;

    m_f.pass = 1;
    m_f.decl = m_f.due;
    m_f.group = 0;
    m_f.item = 0;
    m_f.arr = 0;
    m_f.val = 0;
    m_groups = 0;
    m_total = 0;
    m_fidx = 0;
    if (!m_f.decl)
      return;

    m_port.print(F(SS_PREFIX));
    m_port.print((int)SS_WIRE_VERSION);
    m_port.print(F("{\"" SS_KEY_TITLE "\":"));
    if (m_f.titleFlash)
      str(reinterpret_cast<const __FlashStringHelper*>(m_title));
    else
      str(reinterpret_cast<const char*>(m_title));

    if (m_f.bin)
      putSources();

    m_port.print(F(",\"" SS_KEY_GROUPS "\":["));
  }

  SerialStudioT& setBinary(const void* fields, bool flash, bool bigEndian, const uint8_t* start,
                           uint8_t startLen, bool crc16)
  {
    m_fields = fields;
    m_f.fieldsFlash = flash;
    m_f.bin = 1;
    m_f.big = bigEndian;
    m_f.crcOn = crc16;
    m_start = start;
    m_startLen = startLen > 4 ? 4 : startLen;
    return *this;
  }

  void putSources()
  {
    m_port.print(F(",\"" SS_KEY_SOURCES "\":[{\"" SS_KEY_FRAME_START "\":\""));
    for (uint8_t i = 0; i < m_startLen; ++i)
    {
      uint8_t high = m_start[i] >> 4;
      uint8_t low = m_start[i] & 15;
      m_port.write((uint8_t)(high < 10 ? '0' + high : 'A' + high - 10));
      m_port.write((uint8_t)(low < 10 ? '0' + low : 'A' + low - 10));
    }

    m_port.print(F("\",\"" SS_KEY_HEXADECIMAL_DELIMITERS "\":true,\"" SS_KEY_FRAME_DETECTION "\":3"));
    if (m_f.crcOn)
      m_port.print(F(",\"" SS_KEY_CHECKSUM_ALGORITHM "\":\"CRC-16\""));

    m_port.print(F(",\"" SS_KEY_DECODER_METHOD "\":3,\"" SS_KEY_FRAME_PARSER_TEMPLATE
                   "\":\"" SS_BINARY_TEMPLATE "\",\"" SS_KEY_FRAME_PARSER_PARAMS
                   "\":{\"fields\":"));
    if (m_f.fieldsFlash)
      str(reinterpret_cast<const __FlashStringHelper*>(m_fields));
    else
      str(reinterpret_cast<const char*>(m_fields));

    m_port.print(F(",\"endianness\":\""));
    m_port.print(m_f.big ? F("big") : F("little"));
    m_port.print(F("\"}}]"));
  }

  char fieldChar(uint8_t at) const
  {
    return m_f.fieldsFlash
             ? (char)pgm_read_byte(reinterpret_cast<const char*>(m_fields) + at)
             : reinterpret_cast<const char*>(m_fields)[at];
  }

  /* Index of the k-th field in i8, u8, i16, u16, i32, u32, i64, u64, f32, f64 order; 255 past the end. */
  uint8_t fieldType(uint8_t k) const
  {
    uint8_t at = 0;
    while (k && fieldChar(at))
    {
      if (fieldChar(at++) == ',')
        --k;
    }

    char kind = fieldChar(at);
    if (!kind)
      return 255;

    uint8_t bits = 0;
    for (char c = fieldChar(++at); c >= '0' && c <= '9'; c = fieldChar(++at))
      bits = bits * 10 + (c - '0');

    if (kind == 'f')
      return bits == 64 ? 9 : 8;

    uint8_t base = bits == 8 ? 0 : bits == 16 ? 2 : bits == 32 ? 4 : 6;
    return base + (kind == 'u' ? 1 : 0);
  }

  static double number(const char*)
  {
    return 0;
  }

  static double number(const __FlashStringHelper*)
  {
    return 0;
  }

  template<class V>
  static double number(V v)
  {
    return (double)v;
  }

  void crcUpdate(uint8_t byte)
  {
    uint8_t x = (uint8_t)((m_crc >> 8) ^ byte);
    x ^= (uint8_t)(x >> 4);
    m_crc = (uint16_t)((m_crc << 8) ^ ((uint16_t)x << 12) ^ ((uint16_t)x << 5) ^ x);
  }

  static uint64_t doubleBits(double v)
  {
    uint64_t bits;
    if (sizeof(double) == 8)
    {
      memcpy(&bits, &v, 8);
      return bits;
    }

    float f = (float)v;
    uint32_t u;
    memcpy(&u, &f, 4);
    uint64_t exponent = (u >> 23) & 0xFF;
    if (exponent == 0)
      return (uint64_t)(u >> 31) << 63;

    exponent = exponent == 0xFF ? 0x7FF : exponent + 1023 - 127;
    return ((uint64_t)(u >> 31) << 63) | (exponent << 52) | ((uint64_t)(u & 0x7FFFFF) << 29);
  }

  void pack(double v)
  {
    if (!m_f.val)
    {
      for (uint8_t i = 0; i < m_startLen; ++i)
        m_port.write(m_start[i]);

      m_crc = 0xFFFF;
      m_f.val = 1;
    }

    uint8_t type = fieldType(m_fidx++);
    if (type == 255)
      return;

    static const uint8_t widths[10] PROGMEM = {1, 1, 2, 2, 4, 4, 8, 8, 4, 8};
    uint8_t width = pgm_read_byte(widths + type);
    uint64_t bits;
    if (type == 8)
    {
      float f = (float)v;
      uint32_t u;
      memcpy(&u, &f, 4);
      bits = u;
    }
    else if (type == 9)
      bits = doubleBits(v);
    else if (type % 2 == 0)
      bits = (uint64_t)(int64_t)v;
    else
      bits = v < 0 ? 0 : (uint64_t)v;

    for (uint8_t k = 0; k < width; ++k)
    {
      uint8_t byte = (uint8_t)(bits >> (8 * (m_f.big ? width - 1 - k : k)));
      crcUpdate(byte);
      m_port.write(byte);
    }
  }

  void putReply()
  {
    for (uint8_t i = 0; i < 3; ++i)
    {
      if (m_f.named && i == 2 && !m_msg[0])
        continue;

      m_port.write((uint8_t)m_sep);
      if (m_f.named)
        m_port.print(i == 0 ? F("Reply Token:") : i == 1 ? F("Reply Code:") : F("Reply Message:"));

      if (i == 0)
        m_port.print(m_token);
      else if (i == 1)
        m_port.print(m_code);
      else
        m_port.print(m_msg);
    }
  }

  void closeArray()
  {
    if (m_f.arr)
      m_port.write((uint8_t)']');

    m_f.arr = 0;
  }

  void closeItem()
  {
    if (!m_f.item)
      return;

    closeArray();
    if (m_f.item == 1 && m_f.graph)
      m_port.print(F(",\"" SS_KEY_GRAPH "\":true"));

    m_port.write((uint8_t)'}');
    m_f.item = 0;
  }

  void closeGroup()
  {
    if (!m_f.group)
      return;

    closeItem();
    if (!m_f.any)
      m_port.print(m_f.ctlgrp ? F(",\"" SS_KEY_GROUP_TYPE "\":1,\"" SS_KEY_OUTPUT_WIDGETS "\":[")
                              : F(",\"" SS_KEY_DATASETS "\":["));

    m_port.print(F("]}"));
    m_f.group = 0;
  }

  void openGroup(bool controls)
  {
    begin();
    if (!m_f.decl)
      return;

    closeGroup();
    if (m_groups >= SS_MAX_GROUPS)
      return;

    if (m_groups++)
      m_port.write((uint8_t)',');

    m_port.print(F("{\"" SS_KEY_TITLE "\":"));
    m_f.group = 1;
    m_f.ctlgrp = controls;
    m_f.any = 0;
  }

  template<class N>
  void openItem(N name, uint8_t kind)
  {
    if (!m_f.group || (kind == 1) == (m_f.ctlgrp == 1) || m_total >= SS_MAX_DATASETS)
      return;

    closeItem();
    if (m_f.any)
      m_port.write((uint8_t)',');
    else
      m_port.print(m_f.ctlgrp ? F(",\"" SS_KEY_GROUP_TYPE "\":1,\"" SS_KEY_OUTPUT_WIDGETS "\":[")
                              : F(",\"" SS_KEY_DATASETS "\":["));

    m_f.any = 1;
    m_port.print(F("{\"" SS_KEY_TITLE "\":"));
    str(name);
    m_f.item = kind;
    m_f.graph = 1;
    m_total++;
  }

  void key(const __FlashStringHelper* k)
  {
    closeArray();
    m_port.print(F(",\""));
    m_port.print(k);
    m_port.print(F("\":"));
  }

  void openArray(uint8_t kind, const __FlashStringHelper* k)
  {
    if (m_f.arr == kind)
    {
      m_port.write((uint8_t)',');
      return;
    }

    key(k);
    m_port.write((uint8_t)'[');
    m_f.arr = kind;
  }

  template<class V>
  SerialStudioT& put(const __FlashStringHelper* k, V v, uint8_t kind)
  {
    if (m_f.decl && m_f.item == kind)
    {
      key(k);
      val(v);
    }

    return *this;
  }

  void handle()
  {
    m_buf[m_len] = 0;
    if (strncmp_P(m_buf, PSTR(SS_REQUEST), 4) == 0)
    {
      m_f.due = 1;
      return;
    }

    if (strncmp_P(m_buf, PSTR(SS_COMMAND), 4) != 0)
    {
      if (m_line)
        m_line(m_buf);

      return;
    }

    char* name = m_buf + 4;
    char* eq = strchr(name, '=');
    if (!eq || !m_control)
      return;

    *eq = 0;
    char* text = eq + 1;
    char* token = strchr(text, '@');
    if (token)
      *token = 0;

    char* end = text;
    double value = strtod(text, &end);
    if (end == text)
      return;

    m_msg[0] = 0;
    int code = m_control(name, value);
    if (!token)
      return;

    long tokenValue = strtol(token + 1, &end, 10);
    if (end == token + 1)
      return;

    m_token = tokenValue;
    m_code = code;
    m_f.hasReply = 1;
    replyMessage(m_msg);
  }

  S& m_port;
  const void* m_title;
  ControlFn m_control = nullptr;
  LineFn m_line = nullptr;
  Flags m_f;
  const void* m_fields = nullptr;
  const uint8_t* m_start = nullptr;
  long m_token = 0;
  int m_code = 0;
  uint16_t m_crc = 0;
  uint8_t m_startLen = 0;
  uint8_t m_fidx = 0;
  char m_msg[SS_REPLY_BYTES] = {0};
  uint16_t m_total = 0;
  uint8_t m_groups = 0;
  uint8_t m_len = 0;
  char m_sep = ',';
  char m_buf[SS_INPUT_BYTES];
};

typedef SerialStudioT<Stream> SerialStudio;

#endif
