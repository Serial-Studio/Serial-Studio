/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "channelobserverlistview.h"

namespace mdf::viewer {

ChannelObserverListView::ChannelObserverListView(wxWindow *parent, wxWindowID win_id, const wxPoint &pos,
                                                 const wxSize &size, long style)
: wxListView(parent,win_id,pos,size,style) {

}

ChannelObserverListView::~ChannelObserverListView() {
  if (observer_list_) {
    observer_list_->clear();
  }
  observer_list_.reset();
}

wxString ChannelObserverListView::OnGetItemText(long item, long column) const {
  if (item < 0 || !observer_list_) {
    return "?";
  }
  const auto sample = static_cast<size_t>(item);
  std::ostringstream text;
  if (column == 0) {
    text << sample;
    return text.str();
  }

  const auto observer_index = static_cast<size_t>(column - 1);
  if (observer_index >= observer_list_->size()) {
    return "?";
  }

  const auto& observer = observer_list_->at(observer_index);
  if (!observer) {
    return "";
  }

  if (observer->IsArray()) {
    return wxString::FromUTF8(observer->EngValueToString(sample));
  }
  std::string value;
  bool valid = observer->GetEngValue(sample, value);
  text << (valid ? value : "*");

  const auto& channel = observer->Channel();
  /*
  const auto* conversion = channel.ChannelConversion();

  if (conversion != nullptr) {
     switch(conversion->Type()) {
      case ConversionType::NoConversion:
      case ConversionType::DateConversion:
      case ConversionType::TimeConversion:
        break;

      default:    // Show channel value
        valid = observer->GetChannelValue(sample, value);
        text << " (" << (valid ? value : "*") << ")";
        break;
    }
  }
  */
  return wxString::FromUTF8(text.str());
}

} // namespace mdf::viewer