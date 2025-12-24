/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/atcomment.h"

namespace mdf {

AtComment::AtComment()
: MdComment("AT") {

}

AtComment::AtComment(std::string comment)
: MdComment("AT") {
  Comment(std::move(comment));
}

}  // namespace mdf