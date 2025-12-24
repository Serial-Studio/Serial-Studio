/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ifilehistory.h"

namespace mdf {

void IFileHistory::SetFhComment(const FhComment &fh_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(fh_comment.ToXml());
  }
}

void IFileHistory::GetFhComment(FhComment &fh_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    fh_comment.FromXml(meta_data->XmlSnippet());
  }
}
}