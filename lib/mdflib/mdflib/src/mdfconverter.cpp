/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdfconverter.h"
#include <chrono>
#include "mdf/mdflogstream.h"
#include "mdfblock.h"
#include "dg4block.h"
#include "dt4block.h"
#include "cn4block.h"
#include "hl4block.h"
#include "dl4block.h"
#include "dz4block.h"
#include "samplequeue.h"

using namespace std::chrono_literals;

namespace mdf::detail {

MdfConverter::MdfConverter() {
  type_of_writer_ = MdfWriterType::MdfConverter;
}

MdfConverter::~MdfConverter() {
  write_cache_.Exit();
  if (IsOpen()) {
    Close();
  }
}

bool MdfConverter::InitMeasurement() {
  write_cache_.Exit();

  if (IsOpen()) {
    Close();
  }

  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // Set up internal sample buffers so the last channel values can be stored
  const bool prep = PrepareForWriting();
  if (!prep) {
    MDF_ERROR() << "Failed to prepare the file for writing. File: "
                << filename_;
    return false;
  }
  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.

  Open( write_state_ == WriteState::Create ?
    std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
    :  std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }

  // Save the configuration to disc
  const bool write = mdf_file_->Write(*file_);
  //SetDataPosition();  // Set up data position to end of file

  // Keep the file open to save conversion time

  start_time_ = 0;  // Zero indicate not started
  stop_time_ = 0;   // Zero indicate not stopped
  // Start the working thread that handles the samples
  write_cache_.Init();
  return write;
}

bool MdfConverter::FinalizeMeasurement() {
  write_cache_.Exit();

  // Save outstanding non-written blocks and any block updates as
  // sample counters which changes during DG/DT updates
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(*file_);
  const bool signal_data = WriteSignalData(*file_);
  Close();

  write_state_ = WriteState::Finalize;
  return write && signal_data;
}



}  // namespace mdf