/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"

namespace mdf {

class AtComment : public MdComment {
 public:
  AtComment();
  explicit AtComment(std::string comment);
 private:
};

}  // namespace mdf

