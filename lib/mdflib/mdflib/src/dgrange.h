/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/
/** \file
 * This file define the DG range support class.
 */
#pragma once

#include "mdf/idatagroup.h"
#include <map>
#include "cgrange.h"

namespace mdf {

/** \brief Support class for handling range reads of an MDF file.*/
class DgRange final {
 public:
  DgRange() = delete;
  DgRange(const IDataGroup& data_group, size_t min_sample,
          size_t max_sample);
  [[nodiscard]] bool IsUsed(uint64_t record_id) const;
  [[nodiscard]] bool IsReady() const;
  [[nodiscard]] CgRange* GetCgRange(uint64_t record_id);
  [[nodiscard]] size_t MinSample() const { return min_sample_;}
  [[nodiscard]] size_t MaxSample() const { return max_sample_;};
 private:
  const IDataGroup& data_group_;
  size_t min_sample_ = 0;
  size_t max_sample_ = 0;
  std::map<uint64_t, CgRange> cg_list_; ///< Record ID -> CG range
};

}  // namespace mdf

