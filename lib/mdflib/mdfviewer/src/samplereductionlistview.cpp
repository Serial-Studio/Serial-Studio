/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "samplereductionlistview.h"

#include <sstream>

#include <mdf/ichannelgroup.h>

namespace mdf::viewer {

SampleReductionListView::SampleReductionListView(ISampleReduction &sr_block,
                                                 wxWindow *parent,
                                                 wxWindowID win_id,
                                                 const wxPoint &pos,
                                                 const wxSize &size,
                                                 long style)
: wxListView(parent,win_id,pos,size,style),
  sr_block_(sr_block){

  AppendColumn(wxString::FromUTF8("Sample"));

  const auto* channel_group = sr_block_.ChannelGroup();
  if (channel_group == nullptr) {
    return;
  }
  const auto channel_list = channel_group->Channels();
  for ( const auto* channel : channel_list) {
    if (channel == nullptr) {
      continue;
    }

    std::string unit = channel->Unit();
    {
      std::ostringstream label;
      label << channel->Name() << "-Mean";
      if (!unit.empty()) {
        label << " [" << unit << "]";
      }
      AppendColumn(wxString::FromUTF8(label.str()));
    }

    {
      std::ostringstream label;
      label << channel->Name() << "-Min";
      if (!unit.empty()) {
        label << " [" << unit << "]";
      }
      AppendColumn(wxString::FromUTF8(label.str()));
    }

    {
      std::ostringstream label;
      label << channel->Name() << "-Max";
      if (!unit.empty()) {
        label << " [" << unit << "]";
      }
      AppendColumn(wxString::FromUTF8(label.str()));
    }
  }
  SetItemCount( static_cast<long>(sr_block_.NofSamples()));
  for (int col = 0; col < wxListCtrl::GetColumnCount(); ++col) {
    wxListCtrl::SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);
  }
}

wxString SampleReductionListView::OnGetItemText(long item, long column) const {
  const auto* channel_group = sr_block_.ChannelGroup();
  const auto nof_samples = sr_block_.NofSamples();
  const auto sample = static_cast<uint64_t>(item);

  if (item < 0 || channel_group == nullptr || sample >= nof_samples || column < 0) {
    return "?";
  }

  // First column should always be sample number
  if (column == 0) {
    return std::to_string(sample);
  }

  const auto channel_list = channel_group->Channels();
  const auto channel_index = static_cast<size_t>(column - 1) / 3; // Compensate for 3 values/channel
  const auto value_index =  static_cast<size_t>(column - 1) % 3;
  const auto* channel = channel_index < channel_list.size() ? channel_list[channel_index] : nullptr;

  if (channel == nullptr) {
    return "?";
  }
  const auto* channel_array = channel->ChannelArray(0);
  const auto array_size = channel_array != nullptr ? channel_array->NofArrayValues() : 1;

  std::ostringstream temp;
  for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
    SrValue<std::string> value;
    sr_block_.GetEngValue(*channel, sample, array_index, value);
    if (!temp.str().empty()) {
      temp << ";";
    }
    switch (value_index) {
      case 0: // Mean value
        temp << (value.MeanValid ? value.MeanValue : "*");
        break;

      case 1: // Min value
        temp << (value.MinValid ? value.MinValue : "*");
        break;

      case 2: // Max value
        temp << (value.MaxValid ? value.MaxValue : "*");
        break;

      default:
        temp << "?";
        break;
    }
  }
  return wxString::FromUTF8(temp.str());
}

} // End namespace mdf::viewer