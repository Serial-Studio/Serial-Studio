/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/chcomment.h"

namespace mdf {

ChComment::ChComment()
: MdComment("CH") {

}

ChComment::ChComment(std::string comment)
    : MdComment("CH") {
  Comment(MdString(std::move(comment)));
}

}  // namespace mdf