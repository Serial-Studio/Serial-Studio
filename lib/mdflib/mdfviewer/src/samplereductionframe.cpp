/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "samplereductionframe.h"
#include "samplereductionlistview.h"

#include <mdf/ichannelgroup.h>
namespace {
#include "img/sub.xpm"
}
namespace mdf::viewer {

SampleReductionFrame::SampleReductionFrame(ISampleReduction &sr_block,
                                           wxWindow *parent,
                                           wxWindowID id,
                                           const wxString &title)
: wxFrame(parent, id,title,wxDefaultPosition,parent->GetClientSize(),
          wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT),
  sr_block_(sr_block) {

#ifdef _WIN32
  wxIcon sub("SUB_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
#else
  wxIcon sub {wxICON(sub)};
#endif

  SetIcon(sub);
  list_view_ = new SampleReductionListView(sr_block_, this);
}

SampleReductionFrame::~SampleReductionFrame() {
}

} // End namespace mdf::viewer