/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <wx/listctrl.h>
#include "mdf/mdfreader.h"

namespace mdf::viewer {

class ChannelObserverListView : public wxListView {
 public:
  explicit ChannelObserverListView(wxWindow *parent, wxWindowID win_id = wxID_ANY,
                          const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize,
                          long style = wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES);
  ~ChannelObserverListView() override;

  void ObserverList(std::unique_ptr<ChannelObserverList>& list) {
    observer_list_ = std::move(list);
  }

  void ResetList() {
    while (!observer_list_->empty()) {
      observer_list_->front()->DetachObserver();
    }
    observer_list_.reset();
  }
 protected:
  [[nodiscard]] wxString OnGetItemText(long item, long column) const override;

 private:
  std::unique_ptr<ChannelObserverList> observer_list_;
};

} //namespace mdf::viewer




