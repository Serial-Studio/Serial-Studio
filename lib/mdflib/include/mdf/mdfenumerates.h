/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once
#include <cstdint>

namespace mdf {
/** \brief Enumerate that defines type of bus. Only relevant for bus logging.
 *
 * Enumerate that is used when doing bus logging. The enumerate is used when
 * creating default channel and channel group names.
 *
 * Note that it is possible to have multiple bus types on a single data group.
 */
namespace MdfBusType  {
constexpr uint16_t CAN = 0x0001;      ///< CAN or CAN-FD bus
constexpr uint16_t LIN = 0x0002;      ///< LIN bus
constexpr uint16_t FlexRay = 0x0004;  ///< FlexRay bus
constexpr uint16_t MOST = 0x0008;     ///< MOST bus
constexpr uint16_t Ethernet = 0x0010; ///< Ethernet bus
};

/** \brief Returns the bus type as text. */
std::string MdfBusTypeToString(uint16_t bus_type);

    /** \brief Enumerate that defines how the raw data is stored. By default
 * the fixed length record is stored. Only used when doing bus logging.
 *
 * The fixed length storage is using one SD-block per byte array. The SD block
 * is temporary stored in primary memory instead of store it on disc. This
 * storage type is not recommended for bus logging.
 *
 * The variable length storage uses an extra CG-record for byte array data.
 * The storage type is used for bus logging where payload data is more than 8
 * byte.
 *
 * The maximum length storage shall be used when payload data is 8 bytes or
 * less. It is typically used when logging CAN messages which have 0-8 data
 * payload.
 */
enum class MdfStorageType : int {
  FixedLengthStorage, ///< The default is to use fixed length records.
  VlsdStorage,        ///< Using variable length storage.
  MlsdStorage,        ///< Using maximum length storage
};

}
