/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file mdfhelper.h
 * \brief Support class for the MDF library.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mdf {
/** \brief Support class for the MDF library. */
class MdfHelper {
 public:
  /** \brief Adds the time zone offset to the time stamp.
   *
   * Adds the time zone offset and dst to the UTC nanoseconds since 1970.
   * @param [in] ns_since_1970 Nano-seconds since 1970
   * @return local time = system time + time zone offset
   */
  static uint64_t NanoSecToLocal(uint64_t ns_since_1970);

  /** \brief return the time zone offset in seconds.
   *
   * Returns the current used time zone offset in seconds
   * @return Time offset in seconds
   */
  static int64_t TimeZoneOffset();

  /**
   * \brief Returns the GMT offset in nanoseconds.
   *
   * This function returns the current GMT offset in nanoseconds.
   * @return GMT offset in nanoseconds.
   */
  static int64_t GmtOffsetNs();

  /**
   * \brief Returns the daylight saving time (DST) offset in nanoseconds.
   *
   * This function returns the current daylight saving time (DST) offset in
   * nanoseconds.
   * @return DST offset in nanoseconds.
   */
  static int64_t DstOffsetNs();

  /** \brief Converts a nanosecond since 1970 to a local ISO date and time
   * string.
   *
   * @param [in] ns_since_1970 Nanosecond since 1970
   * @return Return a date and time string in format YYYY-MM-DD hh:mm:ss.ms
   */
  static std::string NsToLocalIsoTime(uint64_t ns_since_1970);

  /** \brief Converts from nanoseconds to CANopen 7 byte Date array.
   *
   * Converts from nanoseconds since 1970-01-01 to a 7 byte CANopen date array
   * This format is used in CANopen protocol and in ASAM MDF files.
   *
   * 7-byte CANopen date format:
   * \li uint16_t Milliseconds since last minute
   * \li uint8_t Minute (0..59)
   * \li uint8_t Hour (0..23)
   * \li uint8_t Day in month (1..31) + Day in week (1 = Monday..7 = Sunday)
   * \li uint8_t Month (1..12)
   * \li uint8_t Year (0..99)
   * @param [in] ns_since_1970 Nanoseconds since 1970-01-01
   * @return 7-byte CANopen date array
   */
  static std::vector<uint8_t> NsToCanOpenDateArray(uint64_t ns_since_1970);

  /** \brief Converts from nanoseconds to CANopen 6 byte Time array.
 *
 * Converts from nanoseconds since 1970-01-01 to a 6-byte CANopen time array
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 6-byte CANopen time format:
 * \li uint32_t Milliseconds since midnight
 * \li uint16_t Days since 1984

 * @param [in] ns_since_1970 Nanoseconds since 1970-01-01
 * @return 6-byte CANopen date array
 */
  static std::vector<uint8_t> NsToCanOpenTimeArray(uint64_t ns_since_1970);

  /** \brief Converts from a CANopen 7 byte Date array to nanoseconds since
   * 1970.
   *
   * Converts from 7 byte CANopen date array to a uint64_t nanoseconds since
   * 1970. This format is used in CANopen protocol and in ASAM MDF files.
   *
   * 7-byte CANopen date format:
   * \li uint16_t Milliseconds since last minute
   * \li uint8_t Minute (0..59)
   * \li uint8_t Hour (0..23)
   * \li uint8_t Day in month (1..31) + Day in week (1 = Monday..7 = Sunday)
   * \li uint8_t Month (1..12)
   * \li uint8_t Year (0..99)
   * @param [in] buffer 7-byte CANopen date array
   * @return Nanoseconds since 1970-01-01
   */
  static uint64_t CanOpenDateArrayToNs(const std::vector<uint8_t> &buffer);

  /** \brief Converts from a CANopen 6 byte Time array to nanoseconds since
 1970.
 *
 * Converts from nanoseconds since 1970-01-01 to a 6-byte CANopen time array
 * This format is used in CANopen protocol and in ASAM MDF files.
 *
 * 6-byte CANopen time format:
 * \li uint32_t Milliseconds since midnight
 * \li uint16_t Days since 1984

 * @param [in] buffer  6-byte CANopen time array
 * @return Nanoseconds since 1970-01-01
 */
  static uint64_t CanOpenTimeArrayToNs(const std::vector<uint8_t> &buffer);

  /** \brief Converts ns since 1970 UTC to local date DD:MM:YYYY string
   *
   * Generates a local date string with the 'DD:MM:YYYY' format, from an UTC
   * time stamp nanoseconds since 1970-01-01 (midnight). The nanosecond
   * timestamp is commonly used when transfer a timestamp. Note that this may
   * not be used as epoch in different internal clocks.
   *
   * @param [in] ns_since_1970 Nanoseconds since 1970 UTC
   * @return Local date format 'DD:MM:YYYY'
   */
  static std::string NanoSecToDDMMYYYY(uint64_t ns_since_1970);

  /** \brief Converts ns since 1970 UTC to local time HH:MM:SS string
   *
   * Generates a local time string with the 'HH:MM:ss' format, from an UTC time
   * stamp nanoseconds since 1970-01-01 (midnight). The nanosecond  timestamp is
   * commonly used when transfer a timestamp. Note that this may not be used as
   * epoch in different internal clocks.
   *
   * @param [in] ns_since_1970 Nanoseconds since 1970 UTC
   * @return Local date format 'HH:MM:SS'
   */
  static std::string NanoSecToHHMMSS(uint64_t ns_since_1970);
  
  /** \brief Converts ns since 1970 UTC to UTC time in 'HH:MM:SS' format.
 *
 * Generates a UTC time string in the 'HH:MM:SS' format from a timestamp 
 * representing nanoseconds since 1970-01-01 (midnight) UTC. This is useful for 
 * converting a high-precision timestamp to a human-readable time format in UTC.
 *
 * @param [in] timestamp_ns Nanoseconds since 1970 UTC.
 * @return UTC time format 'HH:MM:SS'.
   */
  static std::string NanoSecUtcToHHMMSS(uint64_t timestamp_ns);
  
  /** \brief Converts ns since 1970 UTC to UTC date in 'DD/MM/YYYY' format.
 *
 * Generates a UTC date string in the 'DD/MM/YYYY' format from a timestamp 
 * representing nanoseconds since 1970-01-01 (midnight) UTC. This function is 
 * useful for converting a high-precision timestamp into a human-readable date in UTC.
 *
 * @param [in] timestamp_ns Nanoseconds since 1970 UTC.
 * @return UTC date format 'DD/MM/YYYY'.
   */
  static std::string NanoSecUtcToDDMMYYYY(uint64_t timestamp_ns);
  
  /** \brief Converts ns since 1970 UTC to a time string in 'HH:MM:SS' format in a specified timezone.
 *
 * Generates a time string in the 'HH:MM:SS' format based on a timestamp 
 * representing nanoseconds since 1970-01-01 (midnight) UTC, adjusted for a 
 * specified timezone and daylight saving time (DST) offset. This function 
 * is useful for converting a high-precision timestamp into a human-readable 
 * time in a specific timezone.
 *
 * @param [in] timestamp_ns Nanoseconds since 1970 UTC.
 * @param [in] tz_offset_min Timezone offset in minutes from UTC.
 * @param [in] dst_offset_min Daylight saving time (DST) offset in minutes.
 * @return Time format 'HH:MM:SS' adjusted for the specified timezone and DST offset.
   */
  static std::string NanoSecTzToHHMMSS(uint64_t timestamp_ns, int16_t tz_offset_min, int16_t dst_offset_min);
  
  /** \brief Converts ns since 1970 UTC to a date string in 'DD/MM/YYYY' format in a specified timezone.
 *
 * Generates a date string in the 'DD/MM/YYYY' format based on a timestamp 
 * representing nanoseconds since 1970-01-01 (midnight) UTC, adjusted for a 
 * specified timezone and daylight saving time (DST) offset. This function 
 * is useful for converting a high-precision timestamp into a human-readable 
 * date in a specific timezone.
 *
 * @param [in] timestamp_ns Nanoseconds since 1970 UTC.
 * @param [in] tz_offset_min Timezone offset in minutes from UTC.
 * @param [in] dst_offset_min Daylight saving time (DST) offset in minutes.
 * @return Date format 'DD/MM/YYYY' adjusted for the specified timezone and DST offset.
   */
  static std::string NanoSecTzToDDMMYYYY(uint64_t timestamp_ns, int16_t tz_offset_min, int16_t dst_offset_min);

  /** \brief Remove white space from string.
   *
   * Removes white spaces from the begin and end of the string
   * @param text String to trim
   */
  static void Trim(std::string &text);

  /// Converts a floating point value to a string using number of decimals.\n
  /// It also fix rounding and returning a fixed decimals.
  /// Presenting fixed number of decimals means that it fills up the string with
  /// '0' characters.\n Example: Value: 1.23 and decimals 3,String: (Fixed =
  /// false) "1.23" (Fixed = true) "1.230"\n Optional it can append a unit to
  /// the string (Example: "1.23 m/s").\n \param[in] value  The floating point
  /// value. \param[in] decimals Max number of decimals. \param[in] fixed If it
  /// should show fixed number of decimals. \param[in] unit Appends a unit
  /// string to the output. \return The formatted string.
  static std::string FormatDouble(
      double value, uint8_t decimals, bool fixed = false,
      const std::string &unit = {});  ///< Converts a float to a string.

  static uint64_t NowNs(); ///< Return nano-seconds since 1970.

  /** \brief Converts a Latin1 string to UTF8 string. */
  static std::string Latin1ToUtf8(const std::string &latin1);
  /** \brief Converts a wide UTF16 string to an UTF8 string. */
  static std::string Utf16ToUtf8(const std::wstring &utf16);
  /** \brief Converts a wide UTF8 string to an UTF16 string. */
  static std::wstring Utf8ToUtf16(const std::string &utf8);
  /** \brief Returns tru if this computer uses Little Endian. */
  static bool ComputerUseLittleEndian();

  /** \brief Converts a text string to a byte array. */
  static std::vector<uint8_t> TextToByteArray(const std::string& text);


 static void UnsignedToRaw(bool little_endian, size_t start, size_t length,
                     uint64_t value, uint8_t* raw);

};

}  // namespace mdf