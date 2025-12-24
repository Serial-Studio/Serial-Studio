/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>

#include "mdf/mdcomment.h"

namespace mdf {

class DgComment : public MdComment {
 public:
  DgComment();
  explicit DgComment(std::string comment);
};

}  // namespace mdf

