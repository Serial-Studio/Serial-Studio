/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <wx/listctrl.h>

#include "mdf/isamplereduction.h"

namespace mdf::viewer {

class SampleReductionListView: public wxListView {
 public:
  explicit SampleReductionListView(ISampleReduction& sr_block,wxWindow *parent, wxWindowID win_id = wxID_ANY,
  const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
  long style = wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES);

 protected:
  [[nodiscard]] wxString OnGetItemText(long item, long column) const override;

 private:
  ISampleReduction& sr_block_;
};


} // End namespace mdf::viewer

