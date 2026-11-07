/*
 * Serial Studio device library
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * C99, no dependencies beyond the C library, no RTOS glue, no heap.
 *
 * The same ss_channel_*() call that sends a value also declares it: when a declaration
 * is due (start-up, or the app's "#ss?" request) one loop pass writes the declaration
 * line instead of a frame, and every other pass writes a frame. Output goes through
 * the write callback; input arrives through ss_feed().
 */

#ifndef SERIALSTUDIO_H
#define SERIALSTUDIO_H

#include <stddef.h>
#include <stdint.h>

#include "ss_vocabulary.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SS_INPUT_BYTES
#define SS_INPUT_BYTES 48
#endif

#ifndef SS_MAX_FIELDS
#define SS_MAX_FIELDS 64
#endif

#ifndef SS_REPLY_BYTES
#define SS_REPLY_BYTES 24
#endif

#ifndef SS_OUTPUT_BYTES
#define SS_OUTPUT_BYTES 64
#endif

#define SS_FORMAT_NONE 0
#define SS_FORMAT_FIXED 1
#define SS_FORMAT_BITS 2
#define SS_FORMAT_SCALE 3

#define SS_CONTROL_BUTTON 0
#define SS_CONTROL_SLIDER 1
#define SS_CONTROL_TOGGLE 2
#define SS_CONTROL_TEXT 3
#define SS_CONTROL_KNOB 4

typedef void (*ss_write_fn)(void* user, const char* data, size_t len);
/* Returns the reply code of a tokened command: 0 accepted, anything else refused. */
typedef int (*ss_control_fn)(void* user, const char* name, double value);
typedef void (*ss_line_fn)(void* user, const char* line);

typedef struct ss_t
{
  ss_write_fn write;
  ss_control_fn on_control;
  ss_line_fn on_line;
  void* user;
  const char* title;
  uint16_t total;
  uint8_t groups;
  uint8_t out_len;
  uint8_t in_len;
  char separator;
  uint8_t bin, big, crc_on, start_len, fcount, fidx, has_ctl, has_reply;
  uint8_t start[4];
  uint8_t ftypes[SS_MAX_FIELDS];
  uint16_t crc;
  const char* fields;
  long reply_token;
  int reply_code;
  char reply_msg[SS_REPLY_BYTES];
  uint8_t due, decl, pass, group, ctlgrp, any, val, named, graph, over, item, arr;
  char out[SS_OUTPUT_BYTES];
  char in[SS_INPUT_BYTES];
} ss_t;

void ss_init(ss_t* ss, ss_write_fn write, void* user, const char* title);
void ss_set_separator(ss_t* ss, char separator);
void ss_set_named(ss_t* ss, int named);
int ss_set_binary(ss_t* ss, const char* fields, int big_endian, const uint8_t* start,
                  uint8_t start_len, int crc16);
void ss_reply_message(ss_t* ss, const char* text);
void ss_set_handlers(ss_t* ss, ss_control_fn on_control, ss_line_fn on_line);
void ss_request_declaration(ss_t* ss);

void ss_group(ss_t* ss, const char* title, const char* widget);
void ss_control_group(ss_t* ss, const char* title);

void ss_channel(ss_t* ss, const char* name);
void ss_channel_int(ss_t* ss, const char* name, long value);
void ss_channel_double(ss_t* ss, const char* name, double value, int places);
void ss_channel_str(ss_t* ss, const char* name, const char* value);
void ss_control(ss_t* ss, const char* title, int kind);

void ss_set_str(ss_t* ss, const char* key, const char* value);
void ss_set_num(ss_t* ss, const char* key, double value);
void ss_set_bool(ss_t* ss, const char* key, int value);
void ss_graph(ss_t* ss, int on);
void ss_range(ss_t* ss, double low, double high);
void ss_label(ss_t* ss, long value, const char* text);
void ss_alarm(ss_t* ss, double low, double high, long severity, const char* label);

void ss_send(ss_t* ss);
void ss_feed(ss_t* ss, const char* data, size_t len);

#define ss_units(ss, v) ss_set_str((ss), SS_KEY_UNITS, (v))
#define ss_widget(ss, v) ss_set_str((ss), SS_KEY_WIDGET, (v))
#define ss_alias(ss, v) ss_set_str((ss), SS_KEY_ALIAS, (v))
#define ss_color(ss, v) ss_set_str((ss), SS_KEY_COLOR, (v))
#define ss_labels_default(ss, v) ss_set_str((ss), SS_KEY_LABELS_DEFAULT, (v))
#define ss_index(ss, v) ss_set_num((ss), SS_KEY_INDEX, (v))
#define ss_plot_min(ss, v) ss_set_num((ss), SS_KEY_PLOT_MIN, (v))
#define ss_plot_max(ss, v) ss_set_num((ss), SS_KEY_PLOT_MAX, (v))
#define ss_widget_min(ss, v) ss_set_num((ss), SS_KEY_WIDGET_MIN, (v))
#define ss_widget_max(ss, v) ss_set_num((ss), SS_KEY_WIDGET_MAX, (v))
#define ss_fft(ss, v) ss_set_bool((ss), SS_KEY_FFT, (v))
#define ss_fft_sampling_rate(ss, v) ss_set_num((ss), SS_KEY_FFT_SAMPLING_RATE, (v))
#define ss_log(ss, v) ss_set_bool((ss), SS_KEY_LOG, (v))
#define ss_led_high(ss, v) ss_set_num((ss), SS_KEY_LED_HIGH, (v))
#define ss_decimal_points(ss, v) ss_set_num((ss), SS_KEY_DECIMAL_POINTS, (v))
#define ss_hide_on_dashboard(ss, v) ss_set_bool((ss), SS_KEY_HIDE_ON_DASHBOARD, (v))
#define ss_format(ss, v) ss_set_num((ss), SS_KEY_FORMAT, (v))
#define ss_format_width(ss, v) ss_set_num((ss), SS_KEY_FORMAT_WIDTH, (v))
#define ss_format_signed(ss, v) ss_set_bool((ss), SS_KEY_FORMAT_SIGNED, (v))
#define ss_format_fraction_bits(ss, v) ss_set_num((ss), SS_KEY_FORMAT_FRACTION_BITS, (v))
#define ss_format_bit_offset(ss, v) ss_set_num((ss), SS_KEY_FORMAT_BIT_OFFSET, (v))
#define ss_format_bit_width(ss, v) ss_set_num((ss), SS_KEY_FORMAT_BIT_WIDTH, (v))
#define ss_format_scale(ss, v) ss_set_num((ss), SS_KEY_FORMAT_SCALE, (v))
#define ss_format_offset(ss, v) ss_set_num((ss), SS_KEY_FORMAT_OFFSET, (v))
#define ss_output_units(ss, v) ss_set_str((ss), SS_KEY_OUTPUT_UNITS, (v))
#define ss_output_decimals(ss, v) ss_set_num((ss), SS_KEY_OUTPUT_DECIMALS, (v))
#define ss_quantize_to_step(ss, v) ss_set_bool((ss), SS_KEY_QUANTIZE_TO_STEP, (v))
#define ss_output_min(ss, v) ss_set_num((ss), SS_KEY_OUTPUT_MIN, (v))
#define ss_output_max(ss, v) ss_set_num((ss), SS_KEY_OUTPUT_MAX, (v))
#define ss_output_step(ss, v) ss_set_num((ss), SS_KEY_OUTPUT_STEP, (v))
#define ss_initial_value(ss, v) ss_set_num((ss), SS_KEY_INITIAL_VALUE, (v))
#define ss_on_label(ss, v) ss_set_str((ss), SS_KEY_ON_LABEL, (v))
#define ss_off_label(ss, v) ss_set_str((ss), SS_KEY_OFF_LABEL, (v))
#define ss_checkable(ss, v) ss_set_bool((ss), SS_KEY_CHECKABLE, (v))

#ifdef __cplusplus
}
#endif

#endif
