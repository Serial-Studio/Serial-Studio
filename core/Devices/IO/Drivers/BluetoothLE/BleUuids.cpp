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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/BluetoothLE/BleUuids.h"

#include <QBluetoothUuid>
#include <QJsonObject>
#include <QOperatingSystemVersion>

#include "IO/ConnectionManager.h"
#include "IO/Drivers/BluetoothLE.h"
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

namespace IO::Drivers::BleDetail {

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
QString bleServiceName(const QBluetoothUuid& uuid)
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
QBluetoothUuid bleUuidFromString(const QString& uuid)
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
QString bleCharacteristicName(const QLowEnergyCharacteristic& characteristic)
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

}  // namespace IO::Drivers::BleDetail
