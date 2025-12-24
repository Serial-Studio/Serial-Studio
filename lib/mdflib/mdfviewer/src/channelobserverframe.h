/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <wx/frame.h>
#include "mdf/mdfreader.h"
#include "channelobserverlistview.h"
namespace mdf::viewer {

class ChannelObserverFrame : public wxFrame {
 public:
  ChannelObserverFrame(std::unique_ptr<ChannelObserverList>& observer_list, wxWindow *parent, wxWindowID id,
                       const wxString& title);


 private:
  ChannelObserverListView* list_view_ = nullptr;
};

} // namespace mdf::viewer



