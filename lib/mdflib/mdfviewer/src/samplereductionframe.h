/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <wx/frame.h>

#include "mdf/isamplereduction.h"
namespace mdf::viewer {
class SampleReductionListView;

class SampleReductionFrame : public wxFrame {
 public:
  SampleReductionFrame(ISampleReduction& sr_block, wxWindow *parent, wxWindowID id,
                        const wxString& title);
  ~SampleReductionFrame() override;

 private:
  ISampleReduction& sr_block_;
  SampleReductionListView* list_view_ = nullptr;
};


} // End namespace mdf::viewer


