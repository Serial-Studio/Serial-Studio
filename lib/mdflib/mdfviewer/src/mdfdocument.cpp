/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <sstream>

#include <filesystem>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

#include <wx/msgdlg.h>

#include "util/logstream.h"
#include "util/csvwriter.h"
#include "mdfdocument.h"
#include "windowid.h"
#include "mdfviewer.h"
#include <mdf/isamplereduction.h>
#include "channelobserverframe.h"
#include "samplereductionframe.h"
#include "cn4block.h"



using namespace util::log;
using namespace mdf;
namespace {
/**
 * Creates a CSV file using sample observers as input.
 *
 * The CSV file is created in the applications temporary directory and the file name is unique.
 * Note that if the observer list only have one channel, the file will add the samples as first column.
 * The same applies if the first channel not is a master channel.
 * @param list Sample observer list containing samples.
 * @return Full path (ANSI coding) to the created file or an empty string if the function fails.
 */
std::string CreateCsvFile(const mdf::ChannelObserverList& list) {
  if (list.empty()) {
    LOG_ERROR() << "No observers in the list.";
    return "";
  }

  const auto& app = wxGetApp();
  std::string csv_file;
  try {
    std::filesystem::path temp_file(app.GetMyTempDir());
    const auto unique  = boost::filesystem::unique_path();
    temp_file.append(unique.string());
    temp_file.replace_extension(".csv");
    csv_file = temp_file.generic_string();
  } catch (const std::exception& error) {
    LOG_ERROR() << "Failed to create the CSV file. Error: " << error.what();
  }

 util::plot::CsvWriter csv(csv_file);

  const bool master = list.size() > 1 && list[0]->IsMaster();

  // Add the header first
  if (!master) {
    // First column is sample number
    csv.AddColumnHeader("Sample", "");
  }
  for (const auto& channel : list) {
     csv.AddColumnHeader(channel->Name(),channel->Unit());
  }
  csv.AddRow();
  // Add the samples
  for (size_t sample = 0; sample < list[0]->NofSamples(); ++sample) {
    // Add the sample number first
    if (!master) {
      // First column is sample number
      csv.AddColumnValue(sample);
    }
    for (const auto& channel : list) {
      std::string value;
      const bool valid  = channel->GetEngValue(sample, value); // Note that we may lose some digits here
      csv.AddColumnValue(valid ? value : "");
    }
    csv.AddRow();
  }

 return csv_file;
}

std::string CreateGnuPlotFile(const mdf::ChannelObserverList& list, const std::string& csv_file, const std::string& title) {
  if (list.empty() || csv_file.empty()) {
    LOG_ERROR() << "No CSV file or observer list is empty.";
    return "";
  }

  std::string gp_file;
  try {
    std::filesystem::path p(csv_file);
    p.replace_extension(".gp");
    gp_file = p.generic_string();
  } catch(const std::exception&) {
    LOG_ERROR() << "Failed to create gnuplot file. CSV File: " << csv_file;
  }
  const bool master = list.size() > 1 && list[0]->IsMaster();
  std::FILE* file = fopen(gp_file.c_str(), "wt");
  std::ostringstream x_label;
  std::ostringstream y_label;
  if (!master) {
    x_label << "Sample";
    y_label << list[0]->Name();
    if (!list[0]->Unit().empty()) {
      y_label << " [" << list[0]->Unit() << "]";
    }
  } else {
    x_label << list[0]->Name();
    if (!list[0]->Unit().empty()) {
      x_label << " [" << list[0]->Unit() << "]";
    }

    y_label << list[1]->Name();
    if (!list[1]->Unit().empty()) {
      y_label << " [" << list[1]->Unit() << "]";
    }
  }

  if (file != nullptr) {
    fprintf(file, "set terminal wxt noenhanced title \"%s\" size 1200, 1000\n", title.c_str());
    fprintf(file, "set datafile separator comma\n");
    fprintf(file, "set key outside autotitle columnheader\n");
    fprintf(file, "set xlabel \"%s\"\n", x_label.str().c_str());
    fprintf(file, "set ylabel \"%s\"\n", y_label.str().c_str());
    fprintf(file, "set autoscale\n");
    fprintf(file, "plot \"%s\" using 1:2 with lines\n",csv_file.c_str());
    fprintf(file, "exit\n");
    fclose(file);
  }
  return gp_file;
}

} // end namespace

namespace mdf::viewer {
wxIMPLEMENT_DYNAMIC_CLASS(MdfDocument, wxDocument) // NOLINT

wxBEGIN_EVENT_TABLE(MdfDocument, wxDocument) // NOLINT
  EVT_UPDATE_UI(kIdSaveAttachment, MdfDocument::OnUpdateSaveAttachment)
  EVT_MENU(kIdSaveAttachment, MdfDocument::OnSaveAttachment)
  EVT_UPDATE_UI(kIdShowGroupData, MdfDocument::OnUpdateShowGroupData)
  EVT_MENU(kIdShowGroupData, MdfDocument::OnShowGroupData)
  EVT_UPDATE_UI(kIdShowChannelData, MdfDocument::OnUpdateShowChannelData)
  EVT_MENU(kIdShowChannelData, MdfDocument::OnShowChannelData)
  EVT_UPDATE_UI(kIdShowSrData, MdfDocument::OnUpdateShowSrData)
  EVT_MENU(kIdShowSrData, MdfDocument::OnShowSrData)
  EVT_UPDATE_UI(kIdPlotChannelData, MdfDocument::OnUpdatePlotChannelData)
  EVT_MENU(kIdPlotChannelData, MdfDocument::OnPlotChannelData)
wxEND_EVENT_TABLE()

bool MdfDocument::OnOpenDocument(const wxString &filename) {

  wxBusyCursor wait;
  reader_ = std::make_unique<MdfReader>(std::string(filename.mb_str(wxConvUTF8))); // Note that the file is now open
  bool parse = reader_->IsOk();
  if (!parse) {
    LOG_ERROR() << "The file is not an MDF file. File: "
                << filename;
    wxMessageBox(wxString("File is not an MDF file"),
                 wxString("Error Open File"),
                 wxOK | wxCENTRE | wxICON_ERROR);
    reader_->Close();
    return false;
  }

  try {
    parse = reader_->ReadEverythingButData();
   } catch (const std::exception &error) {
    LOG_ERROR() << "Parsing error. Error: " << error.what()
                << ", File: " << filename;
    parse = false;
  }
  reader_->Close();
  if (!parse) {
    wxMessageBox(wxString("Failed to parse the file.\nMore information in the log file."),
                 wxString("Error Parsing File"),
                 wxOK | wxCENTRE | wxICON_ERROR);
  }
  return parse && wxDocument::OnOpenDocument(filename);
}

mdf::detail::MdfBlock*MdfDocument::GetBlock(int64_t id) const {
  const auto *file = GetFile();
  if (file == nullptr || id < 0) {
    return nullptr;
  }

  mdf::detail::MdfBlock* block = nullptr;
  if (file->IsMdf4()) {
    const auto *file4 = dynamic_cast<const mdf::detail::Mdf4File *>(file);
    if (file4 != nullptr) {
      block = file4->Find(id);
    }
  } else {
    const auto *file3 = dynamic_cast<const mdf::detail::Mdf3File *>(file);
    if (file3 != nullptr) {
      block = file3->Find(id);
    }
  }
  return block;
}

void MdfDocument::OnSaveAttachment(wxCommandEvent &) {
  const auto id = GetSelectedBlockId();
  const auto *block = GetBlock(id);
  if (block == nullptr) {
    return;
  }
  const auto* at4 = dynamic_cast<const mdf::detail::At4Block*>(block);
  if (at4 == nullptr) {
    return;
  }

  wxFileDialog save_dialog(wxGetActiveWindow(), "Select the file name for the attachment",
                           "",
                           wxString::FromUTF8(at4->FileName()),
                           "All Files (*.*)|*.*",
                           wxFD_OVERWRITE_PROMPT | wxFD_SAVE);
  auto ret = save_dialog.ShowModal();
  if (ret != wxID_OK) {
    return;
  }
  std::string filename = std::string(save_dialog.GetPath().mb_str(wxConvUTF8));
  const auto ok = reader_->ExportAttachmentData(*at4, filename);
  if (!ok) {
    wxMessageBox("Failed to create the file.\nMore information in the log file.",
                 "File Save Error",
                 wxOK | wxCENTRE | wxICON_ERROR);
    return;
  }

  std::ostringstream open;
  open << "Do you want to open the file ?" << std::endl
    << "File: " << filename;
  auto o = wxMessageBox(wxString::FromUTF8(open.str()),"Open File",
                 wxYES_NO | wxNO_DEFAULT | wxCENTRE | wxICON_QUESTION);
  if (o == wxYES) {
    auto& app = wxGetApp();
    app.OpenFile(filename);
  }
}

void MdfDocument::OnUpdateSaveAttachment(wxUpdateUIEvent &event) {
  if (!reader_) {
    event.Enable(false);
    return;
  }
  const auto id = GetSelectedBlockId();
  const auto *block = GetBlock(id);
  if (block == nullptr) {
    event.Enable(false);
    return;
  }
  event.Enable(block->BlockType() == "AT");
}

void MdfDocument::OnShowGroupData(wxCommandEvent &) {
  const auto selected_id = GetSelectedBlockId();
  auto *selected_block = GetBlock(selected_id);
  if (selected_block == nullptr) {
    return;
  }
  IDataGroup* dg = nullptr;
  IChannelGroup* cg = nullptr;
  if (selected_block->BlockType() == "DG") {
    dg = dynamic_cast<IDataGroup*>(selected_block);
    if (dg != nullptr) {
     auto cg_list = dg->ChannelGroups();
     if (!cg_list.empty()) {
       cg = cg_list[0];
     }
    }
  } else if (selected_block->BlockType() == "CG") {
    cg = dynamic_cast<IChannelGroup*>(selected_block);
    const auto parent_id = GetParentBlockId();
    auto *parent_block = GetBlock(parent_id);
    if (parent_block != nullptr) {
      dg = dynamic_cast<IDataGroup*>(parent_block);
    }
  }
  if (dg == nullptr || cg == nullptr) {
    return;
  }

  auto* view = GetFirstView();
  if (view == nullptr) {
    return;
  }

  auto* parent = view->GetFrame();
  if (parent == nullptr) {
    return;
  }

  std::ostringstream title;
  if (!cg->Name().empty()) {
    title << cg->Name();
  }

  if (!dg->Description().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << dg->Description();
  }

  if (!title.str().empty()) {
    title << "/";
  }
  title << reader_->ShortName();

  auto observer_list = std::make_unique<mdf::ChannelObserverList>();
  mdf::CreateChannelObserverForChannelGroup(*dg, *cg, *observer_list);
  const bool read = reader_->ReadData(*dg);
  if (!read) {
    wxMessageBox("The read failed.\nMore information in the log file.",
                 "Failed to Read Data Block", wxCENTRE | wxICON_ERROR);
    return;
  }
  auto data_frame = new ChannelObserverFrame(observer_list, parent, wxID_ANY, title.str());
  data_frame->Show();
}

void MdfDocument::OnUpdateShowGroupData(wxUpdateUIEvent &event) {
  event.Enable(false);
  if (!reader_) {
    return;
  }

  const auto selected_id = GetSelectedBlockId();
  const auto selected_block = GetBlock(selected_id);
  if (selected_block == nullptr) {
    return;
  }

  if (selected_block->BlockType() == "DG") {
    const auto* dg = dynamic_cast<const IDataGroup*>(selected_block);
    if (dg == nullptr) {
      return;
    }
    auto cg_list = dg->ChannelGroups();
    bool enable = cg_list.size() == 1;
    event.Enable(cg_list.size() == 1);
  } else if (selected_block->BlockType() == "CG") {
    const auto* cg_block = dynamic_cast<const IChannelGroup*>(selected_block);
    if (cg_block == nullptr) {
      return;
    }
    bool enable = !cg_block->Channels().empty(); // No channels == VLSD CG block
    const auto channel_list = cg_block->Channels();
    for (const auto* channel : channel_list) {
      const auto* cn4 = dynamic_cast<const detail::Cn4Block*>(channel);
      if (cn4 == nullptr || cn4->DataLink() == 0) {
        continue;
      }
      const auto* block = GetBlock(cn4->DataLink());
      if ( const auto* data_block_list = dynamic_cast<const detail::DataListBlock*>(block);
           data_block_list != nullptr && data_block_list->DataSize() > 100'000'000) {
        enable = false;
        break;
      }
      if (const auto* data_block = dynamic_cast<const detail::DataBlock*>(block);
          data_block != nullptr && data_block->DataSize() > 100'000'000) {
        enable = false;
        break;
      }
      if (const auto* vlsd_block = dynamic_cast<const detail::Cg4Block*>(block);
          vlsd_block != nullptr && vlsd_block->NofDataBytes() > 100'000'000) {
        enable = false;
        break;
      }
    }

    event.Enable(enable);
  }

}

void MdfDocument::OnShowChannelData(wxCommandEvent &) {
  if (!reader_) {
    return;
  }
  const auto* mdf_file = reader_->GetFile();
  if (mdf_file == nullptr) {
    return;
  }



  const IChannel* channel = nullptr;
  const auto selected_id = GetSelectedBlockId();
  const auto *selected_block = GetBlock(selected_id);
  if (selected_block != nullptr && selected_block->BlockType() == "CN") {
    channel = dynamic_cast<const IChannel*>(selected_block);
  }
  if (channel == nullptr) {
    return;
  }

  auto *data_group = mdf_file->FindParentDataGroup(*channel);
  if (data_group == nullptr) {
    return;
  }

  const auto *channel_group = data_group->FindParentChannelGroup(*channel);
  if (channel_group == nullptr) {
    return;
  }

  // Need to show the master channel as well as the data channel
  const auto* x_axis = channel_group->GetXChannel(*channel);
  // Need to show the master channel as well as the data channel

  auto* view = GetFirstView();
  if (view == nullptr) {
    return;
  }

  auto* parent = view->GetFrame();
  if (parent == nullptr) {
    return;
  }

  std::ostringstream title;
  if (!channel->Name().empty()) {
    title << channel->Name();
  }

  if (!channel_group->Name().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << channel_group->Name();
  }

  if (!data_group->Description().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << data_group->Description();
  }

  if (!title.str().empty()) {
    title << "/";
  }
  title << reader_->ShortName();

  auto observer_list = std::make_unique<mdf::ChannelObserverList>();
  if (x_axis != nullptr && x_axis->Index() != channel->Index()) {
    observer_list->emplace_back(std::move(mdf::CreateChannelObserver(
        *data_group, *channel_group, *x_axis)));
  }
  observer_list->emplace_back(std::move(mdf::CreateChannelObserver(
      *data_group, *channel_group, *channel)));

  const bool read = reader_->ReadData(*data_group);
  if (!read) {
    wxMessageBox("The read failed.\nMore information in the log file.",
                 "Failed to Read Data Block", wxCENTRE | wxICON_ERROR);
    return;
  }

  auto data_frame = new ChannelObserverFrame(observer_list, parent, wxID_ANY, title.str());
  data_frame->Show();
}

void MdfDocument::OnUpdateShowChannelData(wxUpdateUIEvent &event) {
  event.Enable(false);
  if (!reader_) {
    return;
  }

  const auto selected_id = GetSelectedBlockId();
  const auto selected_block = GetBlock(selected_id);
  if (selected_block == nullptr) {
    return;
  }

  if (selected_block->BlockType() == "CN") {
    event.Enable(true);
  }
}

void MdfDocument::OnShowSrData(wxCommandEvent &) {
  if (!reader_) {
    return;
  }
  const auto* mdf_file = reader_->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  ISampleReduction* sr_block = nullptr;
  const auto selected_id = GetSelectedBlockId();
  auto *selected_block = GetBlock(selected_id);
  if (selected_block != nullptr && selected_block->BlockType() == "SR") {
    sr_block = dynamic_cast<ISampleReduction*>(selected_block);
  }
  if (sr_block == nullptr) {
    return;
  }
  const auto* channel_group = sr_block->ChannelGroup();
  if (channel_group == nullptr) {
    return;
  }
  const auto *data_group = channel_group->DataGroup();
  if (data_group == nullptr) {
    return;
  }

  auto* view = GetFirstView();
  if (view == nullptr) {
    return;
  }

  auto* parent = view->GetFrame();
  if (parent == nullptr) {
    return;
  }

  std::ostringstream title;
  title << "SR (Samples: " << sr_block << ")";

  if (!channel_group->Name().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << channel_group->Name();
  }
  if (!data_group->Description().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << data_group->Description();
  }

  if (!title.str().empty()) {
    title << "/";
  }
  title << reader_->ShortName();

  const bool read = reader_->ReadSrData(*sr_block);
  if (!read) {
    wxMessageBox("The read failed.\nMore information in the log file.",
                 "Failed to Read Data Block", wxCENTRE | wxICON_ERROR);
    return;
  }

  auto data_frame = new SampleReductionFrame(*sr_block, parent, wxID_ANY, title.str());
  data_frame->Show();
}

void MdfDocument::OnUpdateShowSrData(wxUpdateUIEvent &event) {
  event.Enable(false);
  if (!reader_) {
    return;
  }

  const auto selected_id = GetSelectedBlockId();
  const auto selected_block = GetBlock(selected_id);
  if (selected_block == nullptr) {
    return;
  }

  if (selected_block->BlockType() == "SR") {
    event.Enable(true);
  }
}

void MdfDocument::OnPlotChannelData(wxCommandEvent &) {
  IDataGroup* dg = nullptr;
  IChannelGroup* cg = nullptr;
  IChannel* cn = nullptr;

  const auto selected_id = GetSelectedBlockId();
  auto *selected_block = GetBlock(selected_id);
  if (selected_block != nullptr && selected_block->BlockType() == "CN") {
    cn = dynamic_cast<IChannel*>(selected_block);
  }

  const auto parent_id = GetParentBlockId();
  auto *parent_block = GetBlock(parent_id);
  if (parent_block != nullptr && parent_block->BlockType() == "CG") {
    cg = dynamic_cast<IChannelGroup*>(parent_block);
  }

  const auto grand_parent_id = GetGrandParentBlockId();
  auto *grand_parent_block = GetBlock(grand_parent_id);
  if (grand_parent_block != nullptr && grand_parent_block->BlockType() == "DG") {
    dg = dynamic_cast<IDataGroup*>(grand_parent_block);
  }
  auto& app = wxGetApp();
  if (dg == nullptr || cg == nullptr || cn == nullptr || app.GnuPlot().empty()) {
    return;
  }
  const auto* x_axis = cg->GetXChannel(*cn); // Need to show the master channel as well as the data channel

  std::ostringstream title;
  if (!cn->Name().empty()) {
    title << cn->Name();
  }

  if (!cg->Name().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << cg->Name();
  }

  if (!dg->Description().empty()) {
    if (!title.str().empty()) {
      title << "/";
    }
    title << dg->Description();
  }

  if (!title.str().empty()) {
    title << "/";
  }
  title << reader_->ShortName();

  // Create the observer list
  auto observer_list = std::make_unique<mdf::ChannelObserverList>();
  if (x_axis != nullptr) {
    observer_list->emplace_back(std::move(mdf::CreateChannelObserver(*dg, *cg, *x_axis)));
  }
  observer_list->emplace_back(std::move(mdf::CreateChannelObserver(*dg, *cg, *cn)));

  // Read in the values from the MDF file
  const bool read = reader_->ReadData(*dg);
  if (!read) {
    wxMessageBox("The read failed.\nMore information in the log file.",
                 "Failed to Read Data Block", wxCENTRE | wxICON_ERROR);
    return;
  }

  // Produce a CSV file with the data for later use with the gnuplot script
  const auto csv_file = CreateCsvFile(*observer_list);
  const auto gp_file = CreateGnuPlotFile(*observer_list, csv_file, title.str());
  if (csv_file.empty() || gp_file.empty()) {
    wxMessageBox("Failed to create CSV or GP files.\nMore information in log file.");
    return;
  }

  std::vector<std::string> args {"--persist", gp_file};
  boost::process::process proc(app.ctx_, app.GnuPlot(), args);
  proc.detach();
}

void MdfDocument::OnUpdatePlotChannelData(wxUpdateUIEvent &event) {
  event.Enable(false);
  auto& app = wxGetApp();
  if (!reader_ || app.GnuPlot().empty()) {
    return;
  }

  const auto selected_id = GetSelectedBlockId();
  const auto* selected_block = GetBlock(selected_id);
  if (selected_block == nullptr) {
    return;
  }

  if (selected_block->BlockType() != "CN") {
    return;
  }
  const auto* cn = dynamic_cast<const IChannel*>(selected_block);
  if (cn == nullptr) {
    return;
  }

  // Note that plotting of strings and absolute times are tricky.
  // The timestamp (CAN Open Date/Time) is possible but labeling the tics is
  // almost impossible.
  // If channel data is an array, the more complex plots are required.
  if (!cn->IsNumber() || cn->ChannelArray(0) != nullptr)  {
    return;
  }
  event.Enable(true);
}

MdfDocument::~MdfDocument() {
  // Close all ObserverFrame
  wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
  std::vector<wxWindow*>  win_list;
  while (node)
  {
    wxWindow* win = node->GetData();
    // do something with "win"
    if (auto* obs_win = dynamic_cast<ChannelObserverFrame*>(win);obs_win != nullptr) {
      win_list.push_back(win);
    }
    if (auto* sr_win = dynamic_cast<SampleReductionFrame*>(win);sr_win != nullptr) {
      win_list.push_back(win);
    }
    node = node->GetNext();
  }
  for (auto* obs : win_list) {
    if (obs != nullptr) {
      auto close = obs->Close(true);
    }
  }
}

} // namespace mdf::viewer