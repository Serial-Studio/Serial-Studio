/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/dgcomment.h"

namespace mdf {

DgComment::DgComment() :
MdComment("DG") {

}

DgComment::DgComment(std::string comment)
    : DgComment() {
  Comment(std::move(comment));
}

}  // namespace mdf