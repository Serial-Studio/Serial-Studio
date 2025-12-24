/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/iattachment.h"

namespace mdf {

void IAttachment::SetAtComment(const AtComment &at_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(at_comment.ToXml());
  }
}

void IAttachment::GetAtComment(AtComment &at_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    at_comment.FromXml(meta_data->XmlSnippet());
  }
}

}  // namespace mdf
