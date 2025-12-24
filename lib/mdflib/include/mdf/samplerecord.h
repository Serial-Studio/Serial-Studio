/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file samplerecord.h
 * Structure that is used when saving samples to a file.
 */
#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
namespace mdf {

/** \struct SampleRecord samplerecord.h "mdf/samplerecord.h"
 * \brief Simple record buffer structure.
 *
 * The sample record structure is used when writing samples to an MDF file. The
 * struct has a timestamp, record ID and the raw record bytes.
 */
struct SampleRecord {
  uint64_t timestamp = 0;  ///< Nanosecond since midnight 1970-01-01 UTC.
  uint64_t record_id = 0;  ///< Unique record ID within the data group.
  std::vector<uint8_t> record_buffer;  ///< Record buffer (fixed size).
  bool vlsd_data = false; ///< True if the vlsd_buffer holds data.
  std::vector<uint8_t> vlsd_buffer; ///< VLSD record buffer.

  /** \brief Returns the size of a record buffer. */
  [[nodiscard]] size_t SampleSize() const {
    // Note that record ID size is missing
    // Fixed record length + VLSD data + VLSD length
    return record_buffer.size() + vlsd_buffer.size() + 4;
  }

  void Clear() {
    timestamp = 0;
    record_id = 0;
    record_buffer.clear();
    record_buffer.shrink_to_fit();
    vlsd_data = false;
    vlsd_buffer.clear();
    vlsd_buffer.shrink_to_fit();
  }

};

}  // namespace mdf
