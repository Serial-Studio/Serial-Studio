/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

#include "serialstudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void flush(ss_t* ss)
{
  if (ss->out_len && ss->write)
    ss->write(ss->user, ss->out, ss->out_len);

  ss->out_len = 0;
}

static void put_c(ss_t* ss, char c)
{
  if (ss->out_len >= SS_OUTPUT_BYTES)
    flush(ss);

  ss->out[ss->out_len++] = c;
}

static void put_s(ss_t* ss, const char* s)
{
  while (*s)
    put_c(ss, *s++);
}

static int is_number_char(char c)
{
  return (c >= '0' && c <= '9') || c == '+' || c == '.' || c == '-';
}

static int is_inert_char(char c)
{
  return c == ' ' || c == ',' || c == ';' || c == ':' || c == '=';
}

static void put_hex(ss_t* ss, char c)
{
  static const char digits[] = "0123456789abcdef";
  put_s(ss, "\\u00");
  put_c(ss, digits[(c >> 4) & 15]);
  put_c(ss, digits[c & 15]);
}

static void put_str(ss_t* ss, const char* s)
{
  size_t i;
  put_c(ss, '"');
  for (i = 0; i < SS_MAX_STRING_BYTES && s[i]; ++i)
  {
    char c = s[i];
    if (c == '"' || c == '\\')
    {
      put_c(ss, '\\');
      put_c(ss, c);
    }
    else if ((unsigned char)c < 0x20 || (is_inert_char(c) && is_number_char(s[i + 1])))
      put_hex(ss, c);
    else
      put_c(ss, c);
  }

  put_c(ss, '"');
}

static void put_raw(ss_t* ss, const char* s)
{
  size_t i;
  for (i = 0; i < SS_MAX_STRING_BYTES && s[i]; ++i)
    put_c(ss, s[i]);
}

static void put_num(ss_t* ss, double v)
{
  char digits[6];
  char text[16];
  unsigned long whole;
  unsigned long fraction;
  int negative;
  int i;
  int last;

  if (v != v)
    v = 0;

  negative = v < 0;
  if (negative)
    v = -v;

  if (v > 4294967040.0)
    v = 4294967040.0;

  whole = (unsigned long)v;
  fraction = (unsigned long)((v - (double)whole) * 1000000.0 + 0.5);
  if (fraction >= 1000000UL)
  {
    whole++;
    fraction -= 1000000UL;
  }

  if (negative && (whole || fraction))
    put_c(ss, '-');

  snprintf(text, sizeof text, "%lu", whole);
  put_s(ss, text);
  if (!fraction)
    return;

  for (i = 5; i >= 0; --i)
  {
    digits[i] = (char)('0' + fraction % 10);
    fraction /= 10;
  }

  last = 5;
  while (digits[last] == '0')
    --last;

  put_c(ss, '.');
  for (i = 0; i <= last; ++i)
    put_c(ss, digits[i]);
}

static void put_long(ss_t* ss, long v)
{
  char text[24];
  snprintf(text, sizeof text, "%ld", v);
  put_s(ss, text);
}

static const char* const kFieldNames[10] = {"i8",  "u8",  "i16", "u16", "i32",
                                            "u32", "i64", "u64", "f32", "f64"};
static const uint8_t kFieldWidths[10] = {1, 1, 2, 2, 4, 4, 8, 8, 4, 8};

static void put_sources(ss_t* ss)
{
  static const char digits[] = "0123456789ABCDEF";
  uint8_t i;

  put_s(ss, ",\"" SS_KEY_SOURCES "\":[{\"" SS_KEY_FRAME_START "\":\"");
  for (i = 0; i < ss->start_len; ++i)
  {
    put_c(ss, digits[ss->start[i] >> 4]);
    put_c(ss, digits[ss->start[i] & 15]);
  }

  put_s(ss, "\",\"" SS_KEY_HEXADECIMAL_DELIMITERS "\":true,\"" SS_KEY_FRAME_DETECTION "\":3");
  if (ss->crc_on)
    put_s(ss, ",\"" SS_KEY_CHECKSUM_ALGORITHM "\":\"CRC-16\"");

  put_s(ss, ",\"" SS_KEY_DECODER_METHOD "\":3,\"" SS_KEY_FRAME_PARSER_TEMPLATE "\":\"" SS_BINARY_TEMPLATE
            "\",\"" SS_KEY_FRAME_PARSER_PARAMS "\":{\"fields\":");
  put_str(ss, ss->fields);
  put_s(ss, ",\"endianness\":\"");
  put_s(ss, ss->big ? "big" : "little");
  put_s(ss, "\"}}]");
}

static void begin(ss_t* ss)
{
  if (ss->pass)
    return;

  ss->pass = 1;
  ss->decl = ss->due;
  ss->group = 0;
  ss->item = 0;
  ss->arr = 0;
  ss->val = 0;
  ss->groups = 0;
  ss->total = 0;
  ss->fidx = 0;
  if (!ss->decl)
    return;

  put_s(ss, SS_PREFIX);
  put_long(ss, SS_WIRE_VERSION);
  put_s(ss, "{\"" SS_KEY_TITLE "\":");
  put_str(ss, ss->title ? ss->title : "");
  if (ss->bin)
    put_sources(ss);

  put_s(ss, ",\"" SS_KEY_GROUPS "\":[");
}

static void open_items(ss_t* ss)
{
  put_s(ss, ss->ctlgrp ? ",\"" SS_KEY_GROUP_TYPE "\":1,\"" SS_KEY_OUTPUT_WIDGETS "\":["
                       : ",\"" SS_KEY_DATASETS "\":[");
}

static void close_array(ss_t* ss)
{
  if (ss->arr)
    put_c(ss, ']');

  ss->arr = 0;
}

static void close_item(ss_t* ss)
{
  if (!ss->item)
    return;

  close_array(ss);
  if (ss->item == 1 && ss->graph)
    put_s(ss, ",\"" SS_KEY_GRAPH "\":true");

  put_c(ss, '}');
  ss->item = 0;
}

static void close_group(ss_t* ss)
{
  if (!ss->group)
    return;

  close_item(ss);
  if (!ss->any)
    open_items(ss);

  put_s(ss, "]}");
  ss->group = 0;
}

static void open_group(ss_t* ss, const char* title, int controls)
{
  begin(ss);
  if (!ss->decl)
    return;

  close_group(ss);
  if (ss->groups >= SS_MAX_GROUPS)
    return;

  if (ss->groups++)
    put_c(ss, ',');

  put_s(ss, "{\"" SS_KEY_TITLE "\":");
  put_str(ss, title);
  ss->group = 1;
  ss->ctlgrp = (uint8_t)controls;
  ss->any = 0;
}

static void open_item(ss_t* ss, const char* name, uint8_t kind)
{
  if (!ss->group || (kind == 1) == (ss->ctlgrp == 1) || ss->total >= SS_MAX_DATASETS)
    return;

  close_item(ss);
  if (ss->any)
    put_c(ss, ',');
  else
    open_items(ss);

  ss->any = 1;
  put_s(ss, "{\"" SS_KEY_TITLE "\":");
  put_str(ss, name);
  ss->item = kind;
  ss->graph = 1;
  ss->total++;
}

static void put_key(ss_t* ss, const char* key)
{
  close_array(ss);
  put_s(ss, ",\"");
  put_s(ss, key);
  put_s(ss, "\":");
}

static void open_array(ss_t* ss, uint8_t kind, const char* key)
{
  if (ss->arr == kind)
  {
    put_c(ss, ',');
    return;
  }

  put_key(ss, key);
  put_c(ss, '[');
  ss->arr = kind;
}

static void frame_prefix(ss_t* ss, const char* name)
{
  if (ss->val)
    put_c(ss, ss->separator);

  if (ss->named)
  {
    put_raw(ss, name);
    put_c(ss, ':');
  }

  ss->val = 1;
}

static void crc_update(ss_t* ss, uint8_t byte)
{
  uint8_t x = (uint8_t)((ss->crc >> 8) ^ byte);
  x ^= (uint8_t)(x >> 4);
  ss->crc = (uint16_t)((ss->crc << 8) ^ ((uint16_t)x << 12) ^ ((uint16_t)x << 5) ^ x);
}

static void pack_value(ss_t* ss, double v)
{
  uint64_t bits;
  uint8_t type;
  uint8_t width;
  uint8_t k;

  if (!ss->val)
  {
    for (k = 0; k < ss->start_len; ++k)
      put_c(ss, (char)ss->start[k]);

    ss->crc = 0xFFFF;
    ss->val = 1;
  }

  if (ss->fidx >= ss->fcount)
    return;

  type = ss->ftypes[ss->fidx++];
  width = kFieldWidths[type];
  if (type == 8)
  {
    float f = (float)v;
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    bits = u;
  }
  else if (type == 9)
    memcpy(&bits, &v, sizeof bits);
  else if (type % 2 == 0)
    bits = (uint64_t)(int64_t)v;
  else
    bits = v < 0 ? 0 : (uint64_t)v;

  for (k = 0; k < width; ++k)
  {
    uint8_t byte = (uint8_t)(bits >> (8 * (ss->big ? width - 1 - k : k)));
    crc_update(ss, byte);
    put_c(ss, (char)byte);
  }
}

static void put_reply(ss_t* ss)
{
  static const char* const titles[3] = {"Reply Token", "Reply Code", "Reply Message"};
  char token[24];
  char code[24];
  const char* values[3];
  int i;

  snprintf(token, sizeof token, "%ld", ss->reply_token);
  snprintf(code, sizeof code, "%d", ss->reply_code);
  values[0] = token;
  values[1] = code;
  values[2] = ss->reply_msg;
  for (i = 0; i < 3; ++i)
  {
    if (ss->named && !values[i][0])
      continue;

    put_c(ss, ss->separator);
    if (ss->named)
    {
      put_s(ss, titles[i]);
      put_c(ss, ':');
    }

    put_s(ss, values[i]);
  }
}

int ss_set_binary(ss_t* ss, const char* fields, int big_endian, const uint8_t* start,
                  uint8_t start_len, int crc16)
{
  const char* p = fields;
  uint8_t i;

  if (!fields || !start || start_len < 1 || start_len > 4)
    return -1;

  ss->fcount = 0;
  while (*p)
  {
    int found = -1;
    int t;
    for (t = 0; t < 10 && found < 0; ++t)
    {
      size_t n = strlen(kFieldNames[t]);
      if (strncmp(p, kFieldNames[t], n) == 0 && (p[n] == ',' || p[n] == 0))
      {
        found = t;
        p += n;
      }
    }

    if (found < 0 || ss->fcount >= SS_MAX_FIELDS)
      return -1;

    ss->ftypes[ss->fcount++] = (uint8_t)found;
    if (*p == ',')
      ++p;
  }

  for (i = 0; i < start_len; ++i)
    ss->start[i] = start[i];

  ss->bin = 1;
  ss->fields = fields;
  ss->big = (uint8_t)(big_endian != 0);
  ss->crc_on = (uint8_t)(crc16 != 0);
  ss->start_len = start_len;
  return ss->fcount ? 0 : -1;
}

void ss_reply_message(ss_t* ss, const char* text)
{
  size_t i;
  for (i = 0; i < SS_REPLY_BYTES - 1 && text[i]; ++i)
  {
    char c = text[i];
    int unsafe = c == ss->separator || c == ',' || c == ';' || c == '\n' || c == '\r';
    ss->reply_msg[i] = unsafe ? ' ' : c;
  }

  ss->reply_msg[i] = 0;
}

void ss_init(ss_t* ss, ss_write_fn write, void* user, const char* title)
{
  memset(ss, 0, sizeof *ss);
  ss->write = write;
  ss->user = user;
  ss->title = title;
  ss->separator = ',';
  ss->due = 1;
}

void ss_set_separator(ss_t* ss, char separator)
{
  ss->separator = separator;
}

void ss_set_named(ss_t* ss, int named)
{
  ss->named = (uint8_t)(named != 0);
}

void ss_set_handlers(ss_t* ss, ss_control_fn on_control, ss_line_fn on_line)
{
  ss->on_control = on_control;
  ss->on_line = on_line;
}

void ss_request_declaration(ss_t* ss)
{
  ss->due = 1;
}

void ss_group(ss_t* ss, const char* title, const char* widget)
{
  open_group(ss, title, 0);
  if (ss->decl && ss->group && widget)
  {
    put_s(ss, ",\"" SS_KEY_WIDGET "\":");
    put_str(ss, widget);
  }
}

void ss_control_group(ss_t* ss, const char* title)
{
  open_group(ss, title, 1);
}

void ss_channel(ss_t* ss, const char* name)
{
  begin(ss);
  if (ss->decl)
    open_item(ss, name, 1);
}

void ss_channel_int(ss_t* ss, const char* name, long value)
{
  ss_channel(ss, name);
  if (ss->decl)
    return;

  if (ss->bin)
  {
    pack_value(ss, (double)value);
    return;
  }

  frame_prefix(ss, name);
  put_long(ss, value);
}

void ss_channel_double(ss_t* ss, const char* name, double value, int places)
{
  char text[48];
  ss_channel(ss, name);
  if (ss->decl)
    return;

  if (ss->bin)
  {
    pack_value(ss, value);
    return;
  }

  frame_prefix(ss, name);
  snprintf(text, sizeof text, "%.*f", places, value);
  put_s(ss, text);
}

void ss_channel_str(ss_t* ss, const char* name, const char* value)
{
  ss_channel(ss, name);
  if (ss->decl)
    return;

  if (ss->bin)
  {
    pack_value(ss, strtod(value, NULL));
    return;
  }

  frame_prefix(ss, name);
  put_s(ss, value);
}

void ss_control(ss_t* ss, const char* title, int kind)
{
  begin(ss);
  if (!ss->decl || !ss->group || !ss->ctlgrp)
    return;

  open_item(ss, title, 2);
  if (!ss->item)
    return;

  ss->has_ctl = 1;
  put_s(ss, ",\"" SS_KEY_OUTPUT_TYPE "\":");
  put_long(ss, kind);
}

void ss_set_str(ss_t* ss, const char* key, const char* value)
{
  if (!ss->decl || !ss->item)
    return;

  put_key(ss, key);
  put_str(ss, value);
}

void ss_set_num(ss_t* ss, const char* key, double value)
{
  if (!ss->decl || !ss->item)
    return;

  put_key(ss, key);
  put_num(ss, value);
}

void ss_set_bool(ss_t* ss, const char* key, int value)
{
  if (!ss->decl || !ss->item)
    return;

  put_key(ss, key);
  put_s(ss, value ? "true" : "false");
}

void ss_graph(ss_t* ss, int on)
{
  if (ss->item == 1)
    ss->graph = (uint8_t)(on != 0);
}

void ss_range(ss_t* ss, double low, double high)
{
  ss_set_num(ss, SS_KEY_WIDGET_MIN, low);
  ss_set_num(ss, SS_KEY_WIDGET_MAX, high);
  ss_set_num(ss, SS_KEY_PLOT_MIN, low);
  ss_set_num(ss, SS_KEY_PLOT_MAX, high);
}

void ss_label(ss_t* ss, long value, const char* text)
{
  if (!ss->decl || ss->item != 1)
    return;

  open_array(ss, 2, SS_KEY_LABELS);
  put_s(ss, "{\"" SS_KEY_VALUE "\":");
  put_long(ss, value);
  put_s(ss, ",\"" SS_KEY_TEXT "\":");
  put_str(ss, text);
  put_c(ss, '}');
}

void ss_alarm(ss_t* ss, double low, double high, long severity, const char* label)
{
  if (!ss->decl || ss->item != 1)
    return;

  open_array(ss, 1, SS_KEY_ALARM_BANDS);
  put_s(ss, "{\"" SS_KEY_MIN "\":");
  put_num(ss, low);
  put_s(ss, ",\"" SS_KEY_MAX "\":");
  put_num(ss, high);
  put_s(ss, ",\"" SS_KEY_SEVERITY "\":");
  put_long(ss, severity);
  put_s(ss, ",\"" SS_KEY_LABEL "\":");
  put_str(ss, label);
  put_c(ss, '}');
}

void ss_send(ss_t* ss)
{
  if (!ss->pass)
    return;

  if (ss->decl)
  {
    close_group(ss);
    put_s(ss, "]}\n");
    ss->due = 0;
  }
  else if (ss->val && ss->bin)
  {
    if (ss->crc_on)
    {
      put_c(ss, (char)(ss->crc >> 8));
      put_c(ss, (char)(ss->crc & 0xFF));
    }
  }
  else if (ss->val)
  {
    if (ss->has_ctl && ss->has_reply)
      put_reply(ss);

    put_c(ss, '\n');
  }

  flush(ss);
  ss->pass = 0;
  ss->decl = 0;
  ss->val = 0;
  ss->group = 0;
  ss->item = 0;
  ss->arr = 0;
  ss->groups = 0;
  ss->total = 0;
}

static void handle_line(ss_t* ss)
{
  char* name;
  char* equals;
  char* text;
  char* token;
  char* end;
  double value;
  int code;

  ss->in[ss->in_len] = 0;
  if (strncmp(ss->in, SS_REQUEST, 4) == 0)
  {
    ss->due = 1;
    return;
  }

  if (strncmp(ss->in, SS_COMMAND, 4) != 0)
  {
    if (ss->on_line)
      ss->on_line(ss->user, ss->in);

    return;
  }

  name = ss->in + 4;
  equals = strchr(name, '=');
  if (!equals || !ss->on_control)
    return;

  *equals = 0;
  text = equals + 1;
  token = strchr(text, '@');
  if (token)
    *token = 0;

  end = text;
  value = strtod(text, &end);
  if (end == text)
    return;

  ss->reply_msg[0] = 0;
  code = ss->on_control(ss->user, name, value);
  if (!token)
    return;

  ss->reply_token = strtol(token + 1, &end, 10);
  if (end == token + 1)
    return;

  ss->reply_code = code;
  ss->has_reply = 1;
  ss_reply_message(ss, ss->reply_msg);
}

void ss_feed(ss_t* ss, const char* data, size_t len)
{
  size_t i;
  for (i = 0; i < len; ++i)
  {
    char c = data[i];
    if (c != '\n' && c != '\r')
    {
      if (ss->in_len < SS_INPUT_BYTES - 1)
        ss->in[ss->in_len++] = c;
      else
        ss->over = 1;

      continue;
    }

    if (ss->in_len > 0 && !ss->over)
      handle_line(ss);

    ss->in_len = 0;
    ss->over = 0;
  }
}
