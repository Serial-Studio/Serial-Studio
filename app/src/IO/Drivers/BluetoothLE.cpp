/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/BluetoothLE.h"

#include <QBluetoothUuid>
#include <QJsonObject>
#include <QOperatingSystemVersion>

#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Well-known BLE UUID name resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Single entry in the well-known BLE UUID table.
 */
struct BleKnownUuid {
  const char* uuid;
  const char* name;
};

// clang-format off
/**
 * @brief Friendly names for known BLE UUIDs (uppercase, brace-less form).
 *
 * Holds the Bluetooth SIG service list plus the vendor 128-bit UUIDs Qt cannot
 * resolve on its own; standard characteristic names still come from Qt first.
 */
static constexpr BleKnownUuid BLE_KNOWN_UUIDS[] = {
  // Bluetooth SIG services
  {"00001800-0000-1000-8000-00805F9B34FB", "Generic Access"},
  {"00001801-0000-1000-8000-00805F9B34FB", "Generic Attribute"},
  {"00001802-0000-1000-8000-00805F9B34FB", "Immediate Alert"},
  {"00001803-0000-1000-8000-00805F9B34FB", "Link Loss"},
  {"00001804-0000-1000-8000-00805F9B34FB", "Tx Power"},
  {"00001805-0000-1000-8000-00805F9B34FB", "Current Time Service"},
  {"00001806-0000-1000-8000-00805F9B34FB", "Reference Time Update Service"},
  {"00001807-0000-1000-8000-00805F9B34FB", "Next DST Change Service"},
  {"00001808-0000-1000-8000-00805F9B34FB", "Glucose"},
  {"00001809-0000-1000-8000-00805F9B34FB", "Health Thermometer"},
  {"0000180A-0000-1000-8000-00805F9B34FB", "Device Information"},
  {"0000180D-0000-1000-8000-00805F9B34FB", "Heart Rate"},
  {"0000180E-0000-1000-8000-00805F9B34FB", "Phone Alert Status Service"},
  {"0000180F-0000-1000-8000-00805F9B34FB", "Battery Service"},
  {"00001810-0000-1000-8000-00805F9B34FB", "Blood Pressure"},
  {"00001811-0000-1000-8000-00805F9B34FB", "Alert Notification Service"},
  {"00001812-0000-1000-8000-00805F9B34FB", "Human Interface Device"},
  {"00001813-0000-1000-8000-00805F9B34FB", "Scan Parameters"},
  {"00001814-0000-1000-8000-00805F9B34FB", "Running Speed and Cadence"},
  {"00001815-0000-1000-8000-00805F9B34FB", "Automation IO"},
  {"00001816-0000-1000-8000-00805F9B34FB", "Cycling Speed and Cadence"},
  {"00001818-0000-1000-8000-00805F9B34FB", "Cycling Power"},
  {"00001819-0000-1000-8000-00805F9B34FB", "Location and Navigation"},
  {"0000181A-0000-1000-8000-00805F9B34FB", "Environmental Sensing"},
  {"0000181B-0000-1000-8000-00805F9B34FB", "Body Composition"},
  {"0000181C-0000-1000-8000-00805F9B34FB", "User Data"},
  {"0000181D-0000-1000-8000-00805F9B34FB", "Weight Scale"},
  {"0000181E-0000-1000-8000-00805F9B34FB", "Bond Management Service"},
  {"0000181F-0000-1000-8000-00805F9B34FB", "Continuous Glucose Monitoring"},
  {"00001820-0000-1000-8000-00805F9B34FB", "Internet Protocol Support Service"},
  {"00001821-0000-1000-8000-00805F9B34FB", "Indoor Positioning"},
  {"00001822-0000-1000-8000-00805F9B34FB", "Pulse Oximeter Service"},
  {"00001823-0000-1000-8000-00805F9B34FB", "HTTP Proxy"},
  {"00001824-0000-1000-8000-00805F9B34FB", "Transport Discovery"},
  {"00001825-0000-1000-8000-00805F9B34FB", "Object Transfer Service"},
  {"00001826-0000-1000-8000-00805F9B34FB", "Fitness Machine"},
  {"00001827-0000-1000-8000-00805F9B34FB", "Mesh Provisioning Service"},
  {"00001828-0000-1000-8000-00805F9B34FB", "Mesh Proxy Service"},
  {"00001829-0000-1000-8000-00805F9B34FB", "Reconnection Configuration"},
  {"0000183A-0000-1000-8000-00805F9B34FB", "Insulin Delivery"},
  {"0000183B-0000-1000-8000-00805F9B34FB", "Binary Sensor"},
  {"0000183C-0000-1000-8000-00805F9B34FB", "Emergency Configuration"},
  {"0000183E-0000-1000-8000-00805F9B34FB", "Physical Activity Monitor"},
  {"00001843-0000-1000-8000-00805F9B34FB", "Audio Input Control"},
  {"00001844-0000-1000-8000-00805F9B34FB", "Volume Control"},
  {"00001845-0000-1000-8000-00805F9B34FB", "Volume Offset Control"},
  {"00001846-0000-1000-8000-00805F9B34FB", "Coordinated Set Identification"},
  {"00001847-0000-1000-8000-00805F9B34FB", "Device Time"},
  {"00001848-0000-1000-8000-00805F9B34FB", "Media Control"},
  {"00001849-0000-1000-8000-00805F9B34FB", "Generic Media Control"},
  {"0000184A-0000-1000-8000-00805F9B34FB", "Constant Tone Extension"},
  {"0000184B-0000-1000-8000-00805F9B34FB", "Telephone Bearer"},
  {"0000184C-0000-1000-8000-00805F9B34FB", "Generic Telephone Bearer"},
  {"0000184D-0000-1000-8000-00805F9B34FB", "Microphone Control"},
  {"0000184E-0000-1000-8000-00805F9B34FB", "Audio Stream Control"},
  {"0000184F-0000-1000-8000-00805F9B34FB", "Broadcast Audio Scan"},
  {"00001850-0000-1000-8000-00805F9B34FB", "Published Audio Capabilities"},
  {"00001851-0000-1000-8000-00805F9B34FB", "Basic Audio Announcement"},
  {"00001852-0000-1000-8000-00805F9B34FB", "Broadcast Audio Announcement"},
  {"00001853-0000-1000-8000-00805F9B34FB", "Common Audio"},
  {"00001854-0000-1000-8000-00805F9B34FB", "Hearing Access"},
  {"00001855-0000-1000-8000-00805F9B34FB", "Telephony and Media Audio"},
  {"00001856-0000-1000-8000-00805F9B34FB", "Public Broadcast Announcement"},
  {"00001857-0000-1000-8000-00805F9B34FB", "Electronic Shelf Label"},
  {"00001858-0000-1000-8000-00805F9B34FB", "Gaming Audio"},
  {"00001859-0000-1000-8000-00805F9B34FB", "Mesh Proxy Solicitation"},

  // Bluetooth SIG member services (16-bit)
  {"0000FD0D-0000-1000-8000-00805F9B34FB", "Blecon Advertising Service"},
  {"0000FD6F-0000-1000-8000-00805F9B34FB", "Exposure Notification Service"},
  {"0000FE0F-0000-1000-8000-00805F9B34FB", "Signify Netherlands B.V. Service"},
  {"0000FE2C-0000-1000-8000-00805F9B34FB", "Google Fast Pair Service"},
  {"0000FE59-0000-1000-8000-00805F9B34FB", "Nordic Secure DFU Service"},
  {"0000FEAA-0000-1000-8000-00805F9B34FB", "Eddystone"},
  {"0000FEBB-0000-1000-8000-00805F9B34FB", "Adafruit File Transfer Service"},

  // Nordic Semiconductor
  {"6E400001-B5A3-F393-E0A9-E50E24DCCA9E", "Nordic UART Service"},
  {"6E400002-B5A3-F393-E0A9-E50E24DCCA9E", "UART RX Characteristic"},
  {"6E400003-B5A3-F393-E0A9-E50E24DCCA9E", "UART TX Characteristic"},
  {"00001523-1212-EFDE-1523-785FEABCD123", "Nordic LED and Button Service"},
  {"00001524-1212-EFDE-1523-785FEABCD123", "Button State"},
  {"00001525-1212-EFDE-1523-785FEABCD123", "LED State"},
  {"57A70000-9350-11ED-A1EB-0242AC120002", "Nordic Status Message Service"},
  {"57A70001-9350-11ED-A1EB-0242AC120002", "Status Characteristic"},
  {"14387800-130C-49E7-B877-2881C89CB258", "Nordic Wi-Fi Provisioning Service"},
  {"14387801-130C-49E7-B877-2881C89CB258", "Wi-Fi Provisioning Version"},
  {"14387802-130C-49E7-B877-2881C89CB258", "Wi-Fi Provisioning Control Point"},
  {"14387803-130C-49E7-B877-2881C89CB258", "Wi-Fi Provisioning Data Out"},
  {"0483DADD-6C9D-6CA9-5D41-03AD4FFF4ABB", "Nordic Throughput Service"},
  {"21490000-494A-4573-98AF-F126AF76F490", "Nordic Distance/Direction Finding Service"},
  {"21490001-494A-4573-98AF-F126AF76F490", "Distance Measurement"},
  {"21490002-494A-4573-98AF-F126AF76F490", "Azimuth Measurement"},
  {"21490003-494A-4573-98AF-F126AF76F490", "Elevation Measurement"},
  {"21490004-494A-4573-98AF-F126AF76F490", "DDF Feature"},
  {"21490005-494A-4573-98AF-F126AF76F490", "DDF Control Point"},
  {"00001530-1212-EFDE-1523-785FEABCD123", "Legacy DFU Service"},
  {"00001531-1212-EFDE-1523-785FEABCD123", "Legacy DFU Control Point"},
  {"00001532-1212-EFDE-1523-785FEABCD123", "Legacy DFU Packet"},
  {"00001534-1212-EFDE-1523-785FEABCD123", "Legacy DFU Version"},
  {"8EC90001-F315-4F60-9FB8-838830DAEA50", "DFU Control Point"},
  {"8EC90002-F315-4F60-9FB8-838830DAEA50", "DFU Packet"},
  {"8EC90003-F315-4F60-9FB8-838830DAEA50", "Buttonless DFU Without Bonds"},
  {"8EC90004-F315-4F60-9FB8-838830DAEA50", "Buttonless DFU With Bonds"},
  {"8E400001-F315-4F60-9FB8-838830DAEA50", "Experimental Buttonless DFU"},
  {"E2A00001-EC31-4EC3-A97A-1C34D87E9878", "Edge Impulse Remote Management Service"},
  {"E2A00002-EC31-4EC3-A97A-1C34D87E9878", "Edge Impulse Remote Management RX"},
  {"E2A00003-EC31-4EC3-A97A-1C34D87E9878", "Edge Impulse Remote Management TX"},

  // Nordic Thingy:52
  {"EF680100-9B35-4933-9B10-52FFA9740042", "Thingy Configuration Service"},
  {"EF680101-9B35-4933-9B10-52FFA9740042", "Thingy Device Name"},
  {"EF680102-9B35-4933-9B10-52FFA9740042", "Thingy Advertising Parameters"},
  {"EF680104-9B35-4933-9B10-52FFA9740042", "Thingy Connection Parameters"},
  {"EF680105-9B35-4933-9B10-52FFA9740042", "Thingy Eddystone URL"},
  {"EF680106-9B35-4933-9B10-52FFA9740042", "Thingy Cloud Token"},
  {"EF680107-9B35-4933-9B10-52FFA9740042", "Thingy FW Version"},
  {"EF680108-9B35-4933-9B10-52FFA9740042", "Thingy MTU Request"},
  {"EF680200-9B35-4933-9B10-52FFA9740042", "Thingy Weather Station Service"},
  {"EF680201-9B35-4933-9B10-52FFA9740042", "Thingy Temperature"},
  {"EF680202-9B35-4933-9B10-52FFA9740042", "Thingy Pressure"},
  {"EF680203-9B35-4933-9B10-52FFA9740042", "Thingy Humidity"},
  {"EF680204-9B35-4933-9B10-52FFA9740042", "Thingy Air Quality"},
  {"EF680205-9B35-4933-9B10-52FFA9740042", "Thingy Color"},
  {"EF680206-9B35-4933-9B10-52FFA9740042", "Thingy Configuration"},
  {"EF680300-9B35-4933-9B10-52FFA9740042", "Thingy UI Service"},
  {"EF680301-9B35-4933-9B10-52FFA9740042", "Thingy LED State"},
  {"EF680302-9B35-4933-9B10-52FFA9740042", "Thingy Button State"},
  {"EF680303-9B35-4933-9B10-52FFA9740042", "Thingy EXT Pin"},
  {"EF680400-9B35-4933-9B10-52FFA9740042", "Thingy Motion Service"},
  {"EF680401-9B35-4933-9B10-52FFA9740042", "Thingy Motion Config"},
  {"EF680402-9B35-4933-9B10-52FFA9740042", "Thingy Tap"},
  {"EF680403-9B35-4933-9B10-52FFA9740042", "Thingy Orientation"},
  {"EF680404-9B35-4933-9B10-52FFA9740042", "Thingy Quaternion"},
  {"EF680405-9B35-4933-9B10-52FFA9740042", "Thingy Pedometer"},
  {"EF680406-9B35-4933-9B10-52FFA9740042", "Thingy Raw Data"},
  {"EF680407-9B35-4933-9B10-52FFA9740042", "Thingy Euler"},
  {"EF680408-9B35-4933-9B10-52FFA9740042", "Thingy Rotation Matrix"},
  {"EF680409-9B35-4933-9B10-52FFA9740042", "Thingy Heading"},
  {"EF68040A-9B35-4933-9B10-52FFA9740042", "Thingy Gravity Vector"},
  {"EF680500-9B35-4933-9B10-52FFA9740042", "Thingy Sound Service"},
  {"EF680501-9B35-4933-9B10-52FFA9740042", "Thingy Sound Config"},
  {"EF680502-9B35-4933-9B10-52FFA9740042", "Thingy Speaker Data"},
  {"EF680503-9B35-4933-9B10-52FFA9740042", "Thingy Speaker Status"},
  {"EF680504-9B35-4933-9B10-52FFA9740042", "Thingy Microphone"},
  {"A5B46352-9D13-479F-9FCB-3DCDF0A13F4D", "Thingy Sensor Hub"},

  // micro:bit
  {"E95D0753-251D-470A-A062-FA1922DFA9A8", "micro:bit Accelerometer Service"},
  {"E95DCA4B-251D-470A-A062-FA1922DFA9A8", "micro:bit Accelerometer Data"},
  {"E95DFB24-251D-470A-A062-FA1922DFA9A8", "micro:bit Accelerometer Period"},
  {"E95DF2D8-251D-470A-A062-FA1922DFA9A8", "micro:bit Magnetometer Service"},
  {"E95DFB11-251D-470A-A062-FA1922DFA9A8", "micro:bit Magnetometer Data"},
  {"E95D386C-251D-470A-A062-FA1922DFA9A8", "micro:bit Magnetometer Period"},
  {"E95D9715-251D-470A-A062-FA1922DFA9A8", "micro:bit Magnetometer Bearing"},
  {"E95D9882-251D-470A-A062-FA1922DFA9A8", "micro:bit Button Service"},
  {"E95DDA90-251D-470A-A062-FA1922DFA9A8", "micro:bit Button A State"},
  {"E95DDA91-251D-470A-A062-FA1922DFA9A8", "micro:bit Button B State"},
  {"E95D127B-251D-470A-A062-FA1922DFA9A8", "micro:bit IO Pin Service"},
  {"E95D8D00-251D-470A-A062-FA1922DFA9A8", "micro:bit Pin Data"},
  {"E95D5899-251D-470A-A062-FA1922DFA9A8", "micro:bit Pin AD Configuration"},
  {"E95DB9FE-251D-470A-A062-FA1922DFA9A8", "micro:bit Pin I/O Configuration"},
  {"E95DD822-251D-470A-A062-FA1922DFA9A8", "micro:bit PWM Control"},
  {"E95DD91D-251D-470A-A062-FA1922DFA9A8", "micro:bit LED Service"},
  {"E95D7B77-251D-470A-A062-FA1922DFA9A8", "micro:bit LED Matrix State"},
  {"E95D93EE-251D-470A-A062-FA1922DFA9A8", "micro:bit LED Text"},
  {"E95D0D2D-251D-470A-A062-FA1922DFA9A8", "micro:bit Scrolling Delay"},
  {"E95D93AF-251D-470A-A062-FA1922DFA9A8", "micro:bit Event Service"},
  {"E95DB84C-251D-470A-A062-FA1922DFA9A8", "micro:bit Requirements"},
  {"E95D9775-251D-470A-A062-FA1922DFA9A8", "micro:bit Event"},
  {"E95D23C4-251D-470A-A062-FA1922DFA9A8", "micro:bit Client Requirements"},
  {"E95D5404-251D-470A-A062-FA1922DFA9A8", "micro:bit Client Event"},
  {"E95D93B0-251D-470A-A062-FA1922DFA9A8", "micro:bit DFU Control Service"},
  {"E95D93B1-251D-470A-A062-FA1922DFA9A8", "micro:bit DFU Control"},
  {"E95D6100-251D-470A-A062-FA1922DFA9A8", "micro:bit Temperature Service"},
  {"E95D9250-251D-470A-A062-FA1922DFA9A8", "micro:bit Temperature"},
  {"E95D1B25-251D-470A-A062-FA1922DFA9A8", "micro:bit Temperature Period"},

  // Adafruit Bluefruit
  {"ADAF0100-C332-42A8-93BD-25E905756CB8", "Adafruit Temperature Service"},
  {"ADAF0101-C332-42A8-93BD-25E905756CB8", "Adafruit Temperature"},
  {"ADAF0200-C332-42A8-93BD-25E905756CB8", "Adafruit Accelerometer Service"},
  {"ADAF0201-C332-42A8-93BD-25E905756CB8", "Adafruit Acceleration"},
  {"ADAF0300-C332-42A8-93BD-25E905756CB8", "Adafruit Light Service"},
  {"ADAF0301-C332-42A8-93BD-25E905756CB8", "Adafruit Light Level"},
  {"ADAF0400-C332-42A8-93BD-25E905756CB8", "Adafruit Gyroscope Service"},
  {"ADAF0401-C332-42A8-93BD-25E905756CB8", "Adafruit Gyro"},
  {"ADAF0500-C332-42A8-93BD-25E905756CB8", "Adafruit Magnetometer Service"},
  {"ADAF0501-C332-42A8-93BD-25E905756CB8", "Adafruit Magnetic"},
  {"ADAF0600-C332-42A8-93BD-25E905756CB8", "Adafruit Button Service"},
  {"ADAF0601-C332-42A8-93BD-25E905756CB8", "Adafruit Pressed"},
  {"ADAF0700-C332-42A8-93BD-25E905756CB8", "Adafruit Humidity Service"},
  {"ADAF0701-C332-42A8-93BD-25E905756CB8", "Adafruit Humidity"},
  {"ADAF0800-C332-42A8-93BD-25E905756CB8", "Adafruit Barometric Service"},
  {"ADAF0801-C332-42A8-93BD-25E905756CB8", "Adafruit Pressure"},
  {"ADAF0900-C332-42A8-93BD-25E905756CB8", "Adafruit Addressable Pixel Service"},
  {"ADAF0901-C332-42A8-93BD-25E905756CB8", "Adafruit Pixel Pin"},
  {"ADAF0902-C332-42A8-93BD-25E905756CB8", "Adafruit Pixel Pin Type"},
  {"ADAF0903-C332-42A8-93BD-25E905756CB8", "Adafruit Pixel Data"},
  {"ADAF0904-C332-42A8-93BD-25E905756CB8", "Adafruit Pixel Buffer Size"},
  {"ADAF0A00-C332-42A8-93BD-25E905756CB8", "Adafruit Color Service"},
  {"ADAF0A01-C332-42A8-93BD-25E905756CB8", "Adafruit Color"},
  {"ADAF0B00-C332-42A8-93BD-25E905756CB8", "Adafruit Sound Service"},
  {"ADAF0B01-C332-42A8-93BD-25E905756CB8", "Adafruit Sound Samples"},
  {"ADAF0B02-C332-42A8-93BD-25E905756CB8", "Adafruit Number of Channels"},
  {"ADAF0C00-C332-42A8-93BD-25E905756CB8", "Adafruit Tone Service"},
  {"ADAF0C01-C332-42A8-93BD-25E905756CB8", "Adafruit Tone"},
  {"ADAF0D00-C332-42A8-93BD-25E905756CB8", "Adafruit Quaternion Service"},
  {"ADAF0D01-C332-42A8-93BD-25E905756CB8", "Adafruit Quaternions"},
  {"ADAF0D02-C332-42A8-93BD-25E905756CB8", "Adafruit Calibration In"},
  {"ADAF0D03-C332-42A8-93BD-25E905756CB8", "Adafruit Calibration Out"},
  {"ADAF0E00-C332-42A8-93BD-25E905756CB8", "Adafruit Proximity Service"},
  {"ADAF0E01-C332-42A8-93BD-25E905756CB8", "Adafruit Proximity"},
  {"ADAF0001-C332-42A8-93BD-25E905756CB8", "Adafruit Sensor Measurement Period"},
  {"ADAF0002-C332-42A8-93BD-25E905756CB8", "Adafruit Sensor Service Version"},
  {"ADAF0100-4669-6C65-5472-616E73666572", "Adafruit File Transfer Version"},
  {"ADAF0200-4669-6C65-5472-616E73666572", "Adafruit File Transfer Raw TX/RX"},

  // Texas Instruments OAD
  {"F000FFC0-0451-4000-B000-000000000000", "TI Over-the-Air Download Service"},
  {"F000FFC1-0451-4000-B000-000000000000", "TI Image Identify"},
  {"F000FFC2-0451-4000-B000-000000000000", "TI Image Block"},
  {"F000FFC5-0451-4000-B000-000000000000", "TI OAD Control"},

  // Apple
  {"7905F431-B5CE-4E99-A40F-4B1E122D00D0", "Apple Notification Center Service"},
  {"9FBF120D-6301-42D9-8C58-25E699A21DBD", "Apple Notification Source"},
  {"69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9", "Apple Control Point"},
  {"22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB", "Apple Data Source"},
  {"89D3502B-0F36-433A-8EF4-C502AD55F8DC", "Apple Media Service"},
  {"9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2", "Apple Remote Command"},
  {"2F7CABCE-808D-411F-9A0C-BB92BA96C102", "Apple Entity Update"},
  {"C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7", "Apple Entity Attribute"},

  // Google / Nordic Eddystone configuration
  {"A3C87500-8ED3-4BDF-8A39-A01BEBEDE295", "Eddystone Configuration Service"},
  {"FE2C1233-8366-4814-8EB0-01DE32100BEA", "Fast Pair Model ID"},
  {"FE2C1234-8366-4814-8EB0-01DE32100BEA", "Fast Pair Key-based Pairing"},
  {"FE2C1235-8366-4814-8EB0-01DE32100BEA", "Fast Pair Passkey"},
  {"FE2C1236-8366-4814-8EB0-01DE32100BEA", "Fast Pair Account Key"},
  {"FE2C1237-8366-4814-8EB0-01DE32100BEA", "Fast Pair Data"},

  // Philips Hue
  {"932C32BD-0000-47A2-835A-A8D455B859DD", "Philips Hue Light Control Service"},
  {"932C32BD-0002-47A2-835A-A8D455B859DD", "Philips Hue Light On/Off Toggle"},
  {"932C32BD-0003-47A2-835A-A8D455B859DD", "Philips Hue Light Brightness Level"},
  {"932C32BD-0005-47A2-835A-A8D455B859DD", "Philips Hue Light Color"},
  {"B8843ADD-0000-4AA1-8794-C3F462030BDA", "Philips Hue Light Update Service"},

  // LEGO Wireless Protocol v3
  {"00001623-1212-EFDE-1623-785FEABCD123", "LEGO Wireless Protocol v3 Hub Service"},
  {"00001624-1212-EFDE-1623-785FEABCD123", "LEGO Wireless Protocol v3 Hub Characteristic"},
  {"00001625-1212-EFDE-1623-785FEABCD123", "LEGO Wireless Protocol v3 Bootloader Service"},
  {"00001626-1212-EFDE-1623-785FEABCD123", "LEGO Wireless Protocol v3 Bootloader Characteristic"},

  // MCUmgr / misc vendors
  {"8D53DC1D-1DB7-4CD3-868B-8A527460AA84", "SMP Service"},
  {"DA2E7828-FBCE-4E01-AE9E-261174997C48", "SMP Characteristic"},
  {"54220000-F6A5-4007-A371-722F4EBD8436", "Memfault Diagnostic Service"},
  {"0FDA92B2-44A2-4AF2-84F5-FA682BAA2B8D", "Helium Hotspot Service"},
};
// clang-format on

/**
 * @brief Returns the friendly name for a known UUID, or an empty string.
 */
static QString knownUuidName(const QBluetoothUuid& uuid)
{
  const QString key = uuid.toString(QUuid::WithoutBraces).toUpper();
  for (const auto& entry : BLE_KNOWN_UUIDS)
    if (key == QLatin1StringView(entry.uuid))
      return QString::fromUtf8(entry.name);

  return {};
}

/**
 * @brief Formats an unknown UUID compactly (0xXXXX short form when possible).
 */
static QString rawUuidName(const QBluetoothUuid& uuid)
{
  bool ok           = false;
  const quint16 u16 = uuid.toUInt16(&ok);
  if (ok)
    return QStringLiteral("0x")
         + QString::number(u16, 16).rightJustified(4, QLatin1Char('0')).toUpper();

  return uuid.toString(QUuid::WithoutBraces).toUpper();
}

/**
 * @brief Resolves a friendly display name for a discovered BLE service.
 */
static QString bleServiceName(const QBluetoothUuid& uuid)
{
  const QString known = knownUuidName(uuid);
  if (!known.isEmpty())
    return known;

  bool ok           = false;
  const quint16 u16 = uuid.toUInt16(&ok);
  if (ok) {
    const QString sig =
      QBluetoothUuid::serviceClassToString(static_cast<QBluetoothUuid::ServiceClassUuid>(u16));
    if (!sig.isEmpty())
      return sig;
  }

  return rawUuidName(uuid);
}

/**
 * @brief Parses a UUID string for GATT lookups. Accepts 16/32-bit shorthand ("FFF1",
 *        "0xFFF1") by expanding it onto the Bluetooth base UUID, plus the full 128-bit
 *        form with or without braces. QBluetoothUuid's own QString constructor rejects
 *        the shorthand every script and doc example uses, so never call it directly.
 */
static QBluetoothUuid bleUuidFromString(const QString& uuid)
{
  QString text = uuid.trimmed();
  if (text.startsWith(QLatin1String("0x"), Qt::CaseInsensitive))
    text.remove(0, 2);

  if (text.size() == 4 || text.size() == 8) {
    bool ok           = false;
    const quint32 hex = text.toUInt(&ok, 16);
    if (ok) {
      if (text.size() == 4)
        return QBluetoothUuid(static_cast<quint16>(hex));

      return QBluetoothUuid(hex);
    }
  }

  return QBluetoothUuid(text);
}

/**
 * @brief Resolves a friendly display name for a discovered BLE characteristic.
 */
static QString bleCharacteristicName(const QLowEnergyCharacteristic& characteristic)
{
  const QString qtName = characteristic.name().simplified();
  if (!qtName.isEmpty())
    return qtName;

  const QBluetoothUuid uuid = characteristic.uuid();
  const QString known       = knownUuidName(uuid);
  if (!known.isEmpty())
    return known;

  bool ok           = false;
  const quint16 u16 = uuid.toUInt16(&ok);
  if (ok) {
    const QString sig =
      QBluetoothUuid::characteristicToString(static_cast<QBluetoothUuid::CharacteristicType>(u16));
    if (!sig.isEmpty())
      return sig;
  }

  return rawUuidName(uuid);
}

//--------------------------------------------------------------------------------------------------
// Static shared discovery state
//--------------------------------------------------------------------------------------------------

bool IO::Drivers::BluetoothLE::s_initialized                               = false;
bool IO::Drivers::BluetoothLE::s_adapterAvailable                          = false;
QBluetoothLocalDevice* IO::Drivers::BluetoothLE::s_localDevice             = nullptr;
QBluetoothDeviceDiscoveryAgent* IO::Drivers::BluetoothLE::s_discoveryAgent = nullptr;
QStringList IO::Drivers::BluetoothLE::s_deviceNames;
QList<QBluetoothDeviceInfo> IO::Drivers::BluetoothLE::s_devices;
QList<IO::Drivers::BluetoothLE*> IO::Drivers::BluetoothLE::s_instances;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a BLE driver instance and registers it for shared discovery.
 */
IO::Drivers::BluetoothLE::BluetoothLE()
  : m_deviceIndex(-1)
  , m_deviceConnected(false)
  , m_gattReady(false)
  , m_selectedCharacteristic(-1)
  , m_pendingServiceIndex(-1)
  , m_pendingCharacteristicIndex(-1)
  , m_probeServiceIndex(-1)
  , m_service(nullptr)
  , m_controller(nullptr)
{
  s_instances.append(this);

  connect(this,
          &IO::Drivers::BluetoothLE::deviceIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);
  connect(this,
          &IO::Drivers::BluetoothLE::characteristicIndexChanged,
          this,
          &IO::Drivers::BluetoothLE::configurationChanged);

  connect(this, &IO::Drivers::BluetoothLE::error, this, [=](const QString& message) {
    Misc::Utilities::showMessageBox(tr("BLE I/O Module Error"), message, QMessageBox::Critical);
  });
}

/**
 * @brief Destructor; closes any active connection and unregisters this instance.
 */
IO::Drivers::BluetoothLE::~BluetoothLE()
{
  close();
  s_instances.removeAll(this);
}

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the Bluetooth LE connection. The resolved service + notify UUIDs are
 *        re-staged as pendings so the GATT configuration survives a disconnect. Controller
 *        and service go through deleteLater(): close() can run inside the controller's own
 *        disconnected emission, and deleting the sender mid-emission crashes the backend.
 */
void IO::Drivers::BluetoothLE::close()
{
  const bool wasConnected = m_deviceConnected;
  m_deviceConnected       = false;
  m_gattReady             = false;
  m_probeServiceIndex     = -1;

  const QString liveServiceUuid = m_service ? m_service->serviceUuid().toString() : QString();
  QString liveNotifyUuid;
  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count())
    liveNotifyUuid = m_characteristics.at(m_selectedCharacteristic).uuid().toString();

  if (!liveServiceUuid.isEmpty())
    m_pendingServiceUuid = liveServiceUuid;

  if (!liveNotifyUuid.isEmpty())
    m_pendingNotifyUuid = liveNotifyUuid;

  m_serviceNames.clear();
  m_serviceUuids.clear();
  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  if (m_service) {
    m_service->disconnect(this);
    m_service->deleteLater();
    m_service = nullptr;
  }

  if (m_controller) {
    m_controller->disconnect(this);
    m_controller->disconnectFromDevice();
    m_controller->deleteLater();
    m_controller = nullptr;
  }

  if (wasConnected) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      inst->m_serviceNames.clear();
      inst->m_serviceUuids.clear();
      inst->m_characteristicNames.clear();
      inst->m_selectedCharacteristic = -1;
      Q_EMIT inst->servicesChanged();
      Q_EMIT inst->characteristicsChanged();
      Q_EMIT inst->characteristicIndexChanged();
    }
  }

  Q_EMIT servicesChanged();
  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();
  Q_EMIT deviceConnectedChanged();
}

/**
 * @brief Open once the link is up and GATT (service + notify characteristic) is wired.
 */
bool IO::Drivers::BluetoothLE::isOpen() const noexcept
{
  return m_deviceConnected && m_gattReady;
}

/**
 * @brief Checks if the Bluetooth LE device is readable.
 */
bool IO::Drivers::BluetoothLE::isReadable() const noexcept
{
  return true;
}

/**
 * @brief Checks if the Bluetooth LE device is writable.
 */
bool IO::Drivers::BluetoothLE::isWritable() const noexcept
{
  return true;
}

/**
 * @brief Verifies if the Bluetooth LE device configuration is valid.
 */
bool IO::Drivers::BluetoothLE::configurationOk() const noexcept
{
  return operatingSystemSupported() && adapterAvailable() && deviceIndex() >= 0;
}

/**
 * @brief Writes data to the Bluetooth LE device.
 */
qint64 IO::Drivers::BluetoothLE::write(const QByteArray& data)
{
  if (m_service && m_selectedCharacteristic >= 0
      && m_selectedCharacteristic < m_characteristics.count()) {
    const auto& characteristic = m_characteristics.at(m_selectedCharacteristic);
    if (characteristic.isValid()) {
      m_service->writeCharacteristic(characteristic, data, QLowEnergyService::WriteWithResponse);
      return data.length();
    }

    else {
      qWarning() << "Failed to write to BLE device: invalid characteristic";
      return 0;
    }
  }

  qWarning() << "Failed to write data to BLE device: ensure that a characteristic is selected";
  return 0;
}

/**
 * @brief Writes to a UUID-resolved characteristic for split read/write devices.
 */
qint64 IO::Drivers::BluetoothLE::writeCharacteristic(const QString& uuid, const QByteArray& data)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull()) {
    qWarning() << "BLE writeCharacteristic: invalid UUID" << uuid;
    return 0;
  }

  QLowEnergyService* service = m_service;
  if (!service) {
    for (auto* inst : std::as_const(s_instances))
      if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_service) {
        service = inst->m_service;
        break;
      }
  }

  if (!service) {
    qWarning() << "BLE writeCharacteristic: no active service for the selected device";
    return 0;
  }

  const auto characteristic = service->characteristic(target);
  if (!characteristic.isValid()) {
    qWarning() << "BLE writeCharacteristic: characteristic" << uuid << "not found in service";
    return 0;
  }

  const auto props = characteristic.properties();
  const auto mode  = (props & QLowEnergyCharacteristic::WriteNoResponse)
                     ? QLowEnergyService::WriteWithoutResponse
                     : QLowEnergyService::WriteWithResponse;

  service->writeCharacteristic(characteristic, data, mode);
  return data.length();
}

/**
 * @brief Opens a connection to a Bluetooth LE device.
 */
bool IO::Drivers::BluetoothLE::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  if (!operatingSystemSupported())
    return false;

  if (m_deviceIndex < 0 || m_deviceIndex >= s_devices.count())
    return false;

  if (m_pendingServiceUuid.isEmpty() && m_pendingServiceIndex < 0) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      if (inst->m_service) {
        m_pendingServiceUuid         = inst->m_service->serviceUuid().toString();
        m_pendingCharacteristicIndex = inst->m_selectedCharacteristic + 1;
        break;
      }

      if (!inst->m_pendingServiceUuid.isEmpty()) {
        m_pendingServiceUuid         = inst->m_pendingServiceUuid;
        m_pendingCharacteristicIndex = inst->m_pendingCharacteristicIndex;
        break;
      }
    }
  }

  close();

  auto device  = s_devices.at(m_deviceIndex);
  m_controller = QLowEnergyController::createCentral(device, this);

  connect(m_controller,
          &QLowEnergyController::discoveryFinished,
          this,
          &IO::Drivers::BluetoothLE::onServiceDiscoveryFinished);

  connect(m_controller, &QLowEnergyController::connected, this, [this]() {
    m_deviceConnected = true;
    m_controller->discoverServices();
    Q_EMIT deviceConnectedChanged();
  });

  connect(
    m_controller, &QLowEnergyController::disconnected, this, &IO::Drivers::BluetoothLE::close);

  m_controller->connectToDevice();
  return true;
}

/**
 * @brief Returns @c false on macOS Monterey (Qt < 6.0 only).
 */
bool IO::Drivers::BluetoothLE::operatingSystemSupported() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#  if defined(Q_OS_MAC)
  if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSBigSur)
    return false;
#  endif
#endif

  return true;
}

/**
 * @brief Returns true if a Bluetooth adapter is available on the system.
 */
bool IO::Drivers::BluetoothLE::adapterAvailable() const
{
  return s_adapterAvailable;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of discovered devices (shared across instances).
 */
int IO::Drivers::BluetoothLE::deviceCount() const
{
  return s_devices.count();
}

/**
 * @brief Returns the index of the BLE device selected by this instance.
 */
int IO::Drivers::BluetoothLE::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @brief Returns the index of the characteristic selected by this instance.
 */
int IO::Drivers::BluetoothLE::characteristicIndex() const
{
  if (m_selectedCharacteristic >= 0)
    return m_selectedCharacteristic + 1;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_selectedCharacteristic >= 0)
      return inst->m_selectedCharacteristic + 1;

  return 0;
}

/**
 * @brief Returns the discovered BLE devices with a leading placeholder entry.
 */
QStringList IO::Drivers::BluetoothLE::deviceNames() const
{
  QStringList list;
  list.append(tr("Select Device"));
  list.append(s_deviceNames);
  return list;
}

/**
 * @brief Returns the discovered BLE services with a leading placeholder entry.
 */
QStringList IO::Drivers::BluetoothLE::serviceNames() const
{
  QStringList list;
  list.append(tr("Select Service"));
  list.append(m_serviceNames);
  return list;
}

/**
 * @brief Returns the discovered BLE characteristics with a leading placeholder.
 */
QStringList IO::Drivers::BluetoothLE::characteristicNames() const
{
  QStringList list;
  list.append(tr("Select Characteristic"));
  list.append(m_characteristicNames);
  return list;
}

/**
 * @brief Returns the UUID of the currently selected service, or an empty string.
 */
QString IO::Drivers::BluetoothLE::selectedServiceUuid() const
{
  if (m_service)
    return m_service->serviceUuid().toString();

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex && inst->m_service)
      return inst->m_service->serviceUuid().toString();

  if (!m_pendingServiceUuid.isEmpty())
    return m_pendingServiceUuid;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex
        && !inst->m_pendingServiceUuid.isEmpty())
      return inst->m_pendingServiceUuid;

  return {};
}

/**
 * @brief Returns the UUID of the subscribed notify characteristic, or the pending one.
 */
QString IO::Drivers::BluetoothLE::selectedNotifyCharacteristicUuid() const
{
  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count())
    return m_characteristics.at(m_selectedCharacteristic).uuid().toString();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst == this || inst->m_deviceIndex != m_deviceIndex)
      continue;

    const int idx = inst->m_selectedCharacteristic;
    if (idx >= 0 && idx < inst->m_characteristics.count())
      return inst->m_characteristics.at(idx).uuid().toString();
  }

  if (!m_pendingNotifyUuid.isEmpty())
    return m_pendingNotifyUuid;

  for (auto* inst : std::as_const(s_instances))
    if (inst != this && inst->m_deviceIndex == m_deviceIndex
        && !inst->m_pendingNotifyUuid.isEmpty())
      return inst->m_pendingNotifyUuid;

  return {};
}

/**
 * @brief Marks GATT ready once per connection and announces it on both wiring paths.
 */
void IO::Drivers::BluetoothLE::announceGattReady()
{
  if (m_gattReady)
    return;

  m_gattReady = true;
  Q_EMIT gattReady();
  Q_EMIT configurationChanged();
}

/**
 * @brief Starts the shared BLE device discovery process.
 */
void IO::Drivers::BluetoothLE::startDiscovery()
{
  if (!operatingSystemSupported())
    return;

  initializeSharedState();

  if (!s_adapterAvailable)
    return;

  if (s_discoveryAgent && s_discoveryAgent->isActive())
    return;

  if (s_discoveryAgent) {
    QObject::disconnect(s_discoveryAgent, nullptr, nullptr, nullptr);
    s_discoveryAgent->deleteLater();
    s_discoveryAgent = nullptr;
  }

  s_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
  QObject::connect(s_discoveryAgent,
                   &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                   s_discoveryAgent,
                   [](const QBluetoothDeviceInfo& d) { onDeviceDiscovered(d); });

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QObject::connect(
    s_discoveryAgent,
    static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(
      &QBluetoothDeviceDiscoveryAgent::error),
    s_discoveryAgent,
    [](QBluetoothDeviceDiscoveryAgent::Error e) { onDiscoveryError(e); });
#else
  QObject::connect(s_discoveryAgent,
                   &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                   s_discoveryAgent,
                   [](QBluetoothDeviceDiscoveryAgent::Error e) { onDiscoveryError(e); });
#endif

  s_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

/**
 * @brief Changes the index of the device selected by the user.
 */
void IO::Drivers::BluetoothLE::selectDevice(const int index)
{
  if (!operatingSystemSupported())
    return;

  if (index <= 0 && (m_deviceConnected || m_deviceIndex >= 0))
    return;

  close();

  m_deviceIndex = index - 1;
  Q_EMIT deviceIndexChanged();
}

/**
 * @brief Changes the index of the service selected by the user.
 */
void IO::Drivers::BluetoothLE::selectService(const int index)
{
  if (!operatingSystemSupported())
    return;

  static bool s_forwarding = false;

  if (!m_controller) {
    if (s_forwarding)
      return;

    s_forwarding = true;
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_controller && inst->m_deviceIndex == m_deviceIndex) {
        inst->selectService(index);
        s_forwarding = false;
        return;
      }
    }

    s_forwarding = false;
    return;
  }

  if (m_service) {
    m_service->disconnect(this);
    m_service->deleteLater();
    m_service = nullptr;
  }

  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic = -1;

  if (index >= 1 && index <= m_serviceNames.count()) {
    if (index - 1 >= m_controller->services().count())
      return;

    auto serviceUuid = m_controller->services().at(index - 1);
    m_service        = m_controller->createServiceObject(serviceUuid, this);
    if (m_service) {
      connect(m_service,
              &QLowEnergyService::characteristicChanged,
              this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service,
              &QLowEnergyService::characteristicRead,
              this,
              &IO::Drivers::BluetoothLE::onCharacteristicChanged);
      connect(m_service,
              &QLowEnergyService::stateChanged,
              this,
              &IO::Drivers::BluetoothLE::onServiceStateChanged);
      connect(m_service,
              &QLowEnergyService::errorOccurred,
              this,
              &IO::Drivers::BluetoothLE::onServiceError);

      if (m_service->state() == QLowEnergyService::RemoteService)
        m_service->discoverDetails();
      else
        configureCharacteristics();
    }

    if (!m_service)
      Q_EMIT error(tr("Error while configuring BLE service"));
  }

  Q_EMIT characteristicsChanged();
}

/**
 * @brief Selects a characteristic and enables notifications if possible.
 */
void IO::Drivers::BluetoothLE::setCharacteristicIndex(const int index)
{
  if (!operatingSystemSupported())
    return;

  static bool s_charForwarding = false;

  if (!m_service) {
    if (s_charForwarding)
      return;

    s_charForwarding = true;
    for (auto* inst : std::as_const(s_instances)) {
      if (inst != this && inst->m_service && inst->m_deviceIndex == m_deviceIndex) {
        inst->setCharacteristicIndex(index);
        s_charForwarding = false;
        return;
      }
    }

    s_charForwarding = false;
    return;
  }

  if (index >= 0 && index <= m_characteristics.count())
    m_selectedCharacteristic = index - 1;
  else
    m_selectedCharacteristic = -1;

  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()) {
    const auto& c = m_characteristics.at(m_selectedCharacteristic);

    const auto& cccd = c.clientCharacteristicConfiguration();
    if (cccd.isValid())
      m_service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);

    if (!c.value().isEmpty())
      publishReceivedData(c.value());
  }

  Q_EMIT characteristicIndexChanged();
}

/**
 * @brief Selects the discovered service whose UUID matches, by delegating to selectService.
 */
bool IO::Drivers::BluetoothLE::selectServiceByUuid(const QString& uuid)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull())
    return false;

  for (int i = 0; i < m_serviceUuids.count(); ++i) {
    if (QBluetoothUuid(m_serviceUuids.at(i)) == target) {
      selectService(i + 1);
      return true;
    }
  }

  return false;
}

/**
 * @brief Subscribes to the notify characteristic with the given UUID, by index delegation.
 */
bool IO::Drivers::BluetoothLE::setNotifyCharacteristicByUuid(const QString& uuid)
{
  const QBluetoothUuid target = bleUuidFromString(uuid);
  if (target.isNull())
    return false;

  for (int i = 0; i < m_characteristics.count(); ++i) {
    if (m_characteristics.at(i).uuid() == target) {
      setCharacteristicIndex(i + 1);
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Per-instance private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Queries and registers available characteristics for the selected service.
 */
void IO::Drivers::BluetoothLE::configureCharacteristics()
{
  if (!m_service)
    return;

  m_characteristics.clear();
  m_characteristicNames.clear();
  m_selectedCharacteristic   = -1;
  const auto characteristics = m_service->characteristics();
  for (const auto& c : characteristics) {
    if (!c.isValid())
      continue;

    m_characteristics.append(c);
    m_characteristicNames.append(bleCharacteristicName(c));
  }

  Q_EMIT characteristicsChanged();
  Q_EMIT characteristicIndexChanged();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_characteristicNames = m_characteristicNames;
      Q_EMIT inst->characteristicsChanged();
    }
  }

  if (!m_pendingNotifyUuid.isEmpty()) {
    const bool found = setNotifyCharacteristicByUuid(m_pendingNotifyUuid);
    if (!found && m_probeServiceIndex >= 0 && m_probeServiceIndex + 1 < m_serviceUuids.count()) {
      ++m_probeServiceIndex;
      selectService(m_probeServiceIndex + 1);
      return;
    }

    if (found) {
      m_pendingNotifyUuid.clear();
      m_pendingCharacteristicIndex = -1;
    }

    m_probeServiceIndex = -1;
  }

  else if (m_pendingCharacteristicIndex > 0) {
    setCharacteristicIndex(m_pendingCharacteristicIndex);
    m_pendingCharacteristicIndex = -1;
  }

  announceGattReady();
}

/**
 * @brief Builds the service UUID list after service discovery completes.
 */
void IO::Drivers::BluetoothLE::onServiceDiscoveryFinished()
{
  if (!m_controller)
    return;

  m_serviceNames.clear();
  m_serviceUuids.clear();
  for (const auto& service : m_controller->services()) {
    m_serviceUuids.append(service.toString());
    m_serviceNames.append(bleServiceName(service));
  }

  Q_EMIT servicesChanged();

  for (auto* inst : std::as_const(s_instances)) {
    if (inst != this && inst->m_deviceIndex == m_deviceIndex) {
      inst->m_serviceNames = m_serviceNames;
      inst->m_serviceUuids = m_serviceUuids;
      Q_EMIT inst->servicesChanged();
    }
  }

  bool serviceSelected = false;
  if (m_pendingServiceIndex > 0) {
    selectService(m_pendingServiceIndex);
    m_pendingServiceIndex = -1;
    serviceSelected       = true;
  }

  else if (!m_pendingServiceUuid.isEmpty() && selectServiceByUuid(m_pendingServiceUuid)) {
    m_pendingServiceUuid.clear();
    serviceSelected = true;
  }

  if (!serviceSelected) {
    for (auto* inst : std::as_const(s_instances)) {
      if (inst == this || inst->m_deviceIndex != m_deviceIndex)
        continue;

      if (inst->m_pendingServiceUuid.isEmpty())
        continue;

      if (selectServiceByUuid(inst->m_pendingServiceUuid)) {
        inst->m_pendingServiceUuid.clear();
        serviceSelected = true;
        break;
      }
    }
  }

  if (!serviceSelected && !m_pendingNotifyUuid.isEmpty() && !m_serviceUuids.isEmpty()) {
    m_probeServiceIndex = 0;
    selectService(1);
    serviceSelected = true;
  }

  if (!serviceSelected)
    announceGattReady();
}

/**
 * @brief Notifies any service error that occurs.
 */
void IO::Drivers::BluetoothLE::onServiceError(QLowEnergyService::ServiceError serviceError)
{
  switch (serviceError) {
    case QLowEnergyService::OperationError:
      Q_EMIT error(tr("Operation error"));
      break;
    case QLowEnergyService::CharacteristicWriteError:
      Q_EMIT error(tr("Characteristic write error"));
      break;
    case QLowEnergyService::DescriptorWriteError:
      Q_EMIT error(tr("Descriptor write error"));
      break;
    case QLowEnergyService::UnknownError:
      Q_EMIT error(tr("Unknown error"));
      break;
    case QLowEnergyService::CharacteristicReadError:
      Q_EMIT error(tr("Characteristic read error"));
      break;
    case QLowEnergyService::DescriptorReadError:
      Q_EMIT error(tr("Descriptor read error"));
      break;
    default:
      break;
  }
}

/**
 * @brief Triggers characteristic configuration when service details are discovered.
 */
void IO::Drivers::BluetoothLE::onServiceStateChanged(QLowEnergyService::ServiceState serviceState)
{
  if (serviceState == QLowEnergyService::RemoteServiceDiscovered)
    configureCharacteristics();
}

/**
 * @brief Reads transmitted data from the BLE service.
 */
void IO::Drivers::BluetoothLE::onCharacteristicChanged(const QLowEnergyCharacteristic& info,
                                                       const QByteArray& value)
{
  if (m_selectedCharacteristic == -1) {
    publishReceivedData(value);
    return;
  }

  if (m_selectedCharacteristic >= 0 && m_selectedCharacteristic < m_characteristics.count()
      && info == m_characteristics.at(m_selectedCharacteristic)) {
    publishReceivedData(value);
  }
}

//--------------------------------------------------------------------------------------------------
// Static shared discovery callbacks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the shared Bluetooth adapter detection (called once).
 */
void IO::Drivers::BluetoothLE::initializeSharedState()
{
  if (s_initialized)
    return;

  s_initialized = true;

  s_localDevice = new QBluetoothLocalDevice();

  auto hostMode      = s_localDevice->hostMode();
  s_adapterAvailable = (hostMode != QBluetoothLocalDevice::HostPoweredOff);

  QObject::connect(s_localDevice,
                   &QBluetoothLocalDevice::hostModeStateChanged,
                   s_localDevice,
                   [](QBluetoothLocalDevice::HostMode m) { onHostModeStateChanged(m); });

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();
}

/**
 * @brief Static callback -- registers a discovered BLE device in the shared list.
 */
void IO::Drivers::BluetoothLE::onDeviceDiscovered(const QBluetoothDeviceInfo& device)
{
  if (!(device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
    return;

  if (!device.isValid() || device.name().isEmpty())
    return;

  if (s_devices.contains(device) || s_deviceNames.contains(device.name()))
    return;

  s_devices.append(device);
  s_deviceNames.append(device.name());

  for (auto* inst : std::as_const(s_instances)) {
    if (inst->m_deviceConnected)
      continue;

    Q_EMIT inst->devicesChanged();

    if (!inst->m_pendingIdentifier.isEmpty()) {
      const auto savedAddr = inst->m_pendingIdentifier.value(QStringLiteral("address")).toString();
      const auto savedName = inst->m_pendingIdentifier.value(QStringLiteral("name")).toString();

      const bool addrMatch = !savedAddr.isEmpty() && device.address().toString() == savedAddr;
      const bool nameMatch = !savedName.isEmpty() && device.name() == savedName;
      if (addrMatch || nameMatch) {
        inst->selectDevice(s_devices.count());
        inst->m_pendingIdentifier = {};
      }
    }
  }
}

/**
 * @brief Static callback -- notifies all instances of a discovery error.
 */
void IO::Drivers::BluetoothLE::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error e)
{
  QString message;
  switch (e) {
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
      message = QObject::tr("Invalid Bluetooth adapter!");
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
      message = QObject::tr("Unsuported platform or operating system");
      break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
      message = QObject::tr("Unsupported discovery method");
      break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
      message = QObject::tr("General I/O error");
      break;
    default:
      return;
  }

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->error(message);
}

/**
 * @brief Static callback -- handles Bluetooth adapter power state changes.
 */
void IO::Drivers::BluetoothLE::onHostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
  bool wasAvailable  = s_adapterAvailable;
  s_adapterAvailable = (state != QBluetoothLocalDevice::HostPoweredOff);

  if (wasAvailable == s_adapterAvailable)
    return;

  for (auto* inst : std::as_const(s_instances))
    Q_EMIT inst->adapterAvailabilityChanged();

  if (!s_adapterAvailable) {
    if (s_discoveryAgent && s_discoveryAgent->isActive())
      s_discoveryAgent->stop();

    s_devices.clear();
    s_deviceNames.clear();
    for (auto* inst : std::as_const(s_instances)) {
      inst->close();
      inst->m_deviceIndex = -1;
      Q_EMIT inst->devicesChanged();
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the BLE address and name of the currently selected device.
 */
QJsonObject IO::Drivers::BluetoothLE::deviceIdentifier() const
{
  if (m_deviceIndex < 0 || m_deviceIndex >= s_devices.count())
    return {};

  const auto& device = s_devices.at(m_deviceIndex);
  QJsonObject id;

  const auto addr = device.address().toString();
  if (!addr.isEmpty() && addr != QStringLiteral("00:00:00:00:00:00"))
    id.insert(QStringLiteral("address"), addr);

  const auto name = device.name();
  if (!name.isEmpty())
    id.insert(QStringLiteral("name"), name);

  return id;
}

/**
 * @brief Tries to find a discovered BLE device matching a previously saved identifier.
 */
bool IO::Drivers::BluetoothLE::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  const auto savedAddr = id.value(QStringLiteral("address")).toString();
  const auto savedName = id.value(QStringLiteral("name")).toString();

  int bestScore = 0;
  int bestIndex = -1;

  for (int i = 0; i < s_devices.count(); ++i) {
    const auto& device = s_devices.at(i);
    int score          = 0;

    if (!savedAddr.isEmpty() && device.address().toString() == savedAddr)
      score += 100;

    if (!savedName.isEmpty() && device.name() == savedName)
      score += 10;

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex >= 0) {
    selectDevice(bestIndex + 1);
    m_pendingIdentifier = {};
    return true;
  }

  m_pendingIdentifier = id;
  if (!s_discoveryAgent || !s_discoveryAgent->isActive())
    startDiscovery();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Bluetooth LE configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::BluetoothLE::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty dev;
  dev.key     = QStringLiteral("deviceIndex");
  dev.label   = tr("BLE Device");
  dev.type    = IO::DriverProperty::ComboBox;
  dev.value   = m_deviceIndex;
  dev.options = s_deviceNames;
  props.append(dev);

  IO::DriverProperty svc;
  svc.key   = QStringLiteral("serviceUuid");
  svc.label = tr("Service");
  svc.type  = IO::DriverProperty::Text;
  svc.value = selectedServiceUuid();
  props.append(svc);

  IO::DriverProperty notifyUuid;
  notifyUuid.key   = QStringLiteral("notifyCharacteristicUuid");
  notifyUuid.label = tr("Notify Characteristic");
  notifyUuid.type  = IO::DriverProperty::Text;
  notifyUuid.value = selectedNotifyCharacteristicUuid();
  props.append(notifyUuid);

  IO::DriverProperty ch;
  ch.key     = QStringLiteral("characteristicIndex");
  ch.label   = tr("Characteristic");
  ch.type    = IO::DriverProperty::ComboBox;
  ch.value   = characteristicIndex() - 1;
  ch.options = m_characteristicNames;
  props.append(ch);

  return props;
}

/**
 * @brief Applies a single Bluetooth LE configuration change by key.
 */
void IO::Drivers::BluetoothLE::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("deviceIndex")) {
    m_deviceIndex = value.toInt();
    Q_EMIT deviceIndexChanged();
    return;
  }

  if (key == QLatin1String("serviceUuid")) {
    m_pendingServiceUuid = value.toString();
    return;
  }

  if (key == QLatin1String("notifyCharacteristicUuid")) {
    m_pendingNotifyUuid = value.toString();
    return;
  }

  if (key == QLatin1String("characteristicIndex")) {
    m_selectedCharacteristic     = value.toInt();
    m_pendingCharacteristicIndex = value.toInt() + 1;
    Q_EMIT characteristicIndexChanged();
  }
}
