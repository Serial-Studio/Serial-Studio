/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"


namespace mdf {

class ChComment : public MdComment {
 public:
  ChComment();
  explicit ChComment(std::string comment);
 private:

};

}  // namespace mdf

