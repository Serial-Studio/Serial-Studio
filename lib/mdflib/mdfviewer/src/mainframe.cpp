/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <string>
#include <filesystem>

#include <wx/config.h>
#include <wx/aboutdlg.h>

#include "mdf/mdfreader.h"

#include "mainframe.h"
#include "windowid.h"


namespace {
#include "img/app.xpm"
}

namespace mdf::viewer {

wxBEGIN_EVENT_TABLE(MainFrame, wxDocMDIParentFrame)
  EVT_UPDATE_UI(kIdSaveAttachment, MainFrame::OnUpdateNoDoc) // Disable the menu if no documents are open
  EVT_UPDATE_UI(kIdShowGroupData, MainFrame::OnUpdateNoDoc)
  EVT_UPDATE_UI(kIdShowChannelData, MainFrame::OnUpdateNoDoc)
  EVT_UPDATE_UI(kIdShowSrData, MainFrame::OnUpdateNoDoc)
  EVT_UPDATE_UI(kIdPlotChannelData, MainFrame::OnUpdateNoDoc)
  EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
  EVT_CLOSE(MainFrame::OnClose)
  EVT_DROP_FILES(MainFrame::OnDropFiles)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxPoint& start_pos, const wxSize& start_size, bool maximized)
    : wxDocMDIParentFrame(wxDocManager::GetDocumentManager(), nullptr, wxID_ANY, title,
      start_pos, start_size) {
#ifdef _WIN32
  wxIcon app("APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
#else
  wxIcon app {wxICON(app)};
#endif
  SetIcon(app);
  wxWindow::SetName("MdfTopWindow");
#if (_MSC_VER)
  wxTopLevelWindowMSW::Maximize(maximized);
 #else
  wxTopLevelWindowNative::Maximize(maximized);
#endif
  wxWindow::DragAcceptFiles(true);

  auto* app_config = wxConfig::Get();
  auto* doc_manager = wxDocManager::GetDocumentManager();

    // FILE
  auto* menu_file = new wxMenu;
  menu_file->Append(wxID_OPEN);
  menu_file->Append(wxID_EXIT);

  doc_manager->FileHistoryUseMenu(menu_file);
  doc_manager->FileHistoryLoad(*app_config);

  // BLOCK_OPERATIONS
  auto* menu_block = new wxMenu;
  menu_block->Append(kIdShowGroupData, "Show Group Data");
  menu_block->Append(kIdShowChannelData, "Show Channel Data");
  menu_block->AppendSeparator();
  menu_block->Append(kIdPlotChannelData, "Plot Channel Data");
  menu_block->AppendSeparator();
  menu_block->Append(kIdSaveAttachment, "Save Attachment File");

  // ABOUT
  auto* menu_about = new wxMenu;
  menu_about->Append(kIdOpenLogFile, "Open Log File");
  menu_about->Append(kIdGnuPlotDownloadPage, "GnuPlot Download Page");
  menu_about->AppendSeparator();
  menu_about->Append(wxID_ABOUT, wxGetStockLabel(wxID_ABOUT));

  auto* menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, wxGetStockLabel(wxID_FILE));
  menu_bar->Append(menu_block, "Data");
  menu_bar->Append(menu_about, wxGetStockLabel(wxID_HELP));
  wxFrameBase::SetMenuBar(menu_bar);
}


void MainFrame::OnClose(wxCloseEvent &event) {

  // If the window is minimized. Do not save as last position

  if (!IsIconized()) {
    bool maximized = IsMaximized();
    wxPoint end_pos = GetPosition();
    wxSize end_size = GetSize();
    auto* app_config = wxConfig::Get();

    if (maximized) {
      app_config->Write("/MainWin/Max",maximized);
    } else {
      app_config->Write("/MainWin/X", end_pos.x);
      app_config->Write("/MainWin/Y", end_pos.y);
      app_config->Write("/MainWin/XWidth", end_size.x);
      app_config->Write("/MainWin/YWidth", end_size.y);
      app_config->Write("/MainWin/Max", maximized);
    }
  }
  event.Skip(true);
}

void MainFrame::OnAbout(wxCommandEvent&) {
  wxAboutDialogInfo info;
  info.SetName("MDF Viewer");
  info.SetVersion("1.0");
  info.SetDescription("Simple MDF file viewer.");

  wxArrayString devs;
  devs.push_back("Ingemar Hedvall");
  info.SetDevelopers(devs);

  info.SetCopyright("(C) 2021 Ingemar Hedvall");
  info.SetLicense("MIT License (https://opensource.org/licenses/MIT)\n"
      "Copyright 2021 Ingemar Hedvall\n"
      "\n"
      "Permission is hereby granted, free of charge, to any person obtaining a copy of this\n"
      "software and associated documentation files (the \"Software\"),\n"
      "to deal in the Software without restriction, including without limitation the rights to use, copy,\n"
      "modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,\n"
      "and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n"
      "\n"
      "The above copyright notice and this permission notice shall be included in all copies or substantial\n"
      "portions of the Software.\n"
      "\n"
      "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,\n"
      "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR\n"
      "PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,\n"
      "DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR\n"
      "IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
  );
  if (!info.HasIcon()) {
#ifdef _WIN32
      wxIcon app("APP_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
#else
      wxIcon app {wxICON(app)};
#endif
      info.SetIcon(app);
  }


  wxAboutBox(info);
}

void MainFrame::OnUpdateNoDoc(wxUpdateUIEvent &event) {
  auto* man = wxDocManager::GetDocumentManager();
  auto* doc = man != nullptr ? man->GetCurrentDocument() : nullptr;
  if (doc == nullptr) {
    event.Enable(false);
  }
}

void MainFrame::OnDropFiles(wxDropFilesEvent& event) {
  const int count = event.GetNumberOfFiles();
  const auto* list = event.GetFiles();
  auto* man = wxDocManager::GetDocumentManager();
  if (count <= 0 || list == nullptr || man == nullptr)  {
    return;
  }

  for (int ii = 0; ii < count; ++ii) {
    try {
      const auto& file = list[ii];
      std::filesystem::path p(file.ToStdWstring());
      if (!std::filesystem::exists(p)) {
        continue;
      }
      const auto& u8str = p.u8string();
      const bool mdf = mdf::IsMdfFile(std::string(u8str.begin(), u8str.end()));
      if (!mdf) {
        continue;
      }
        // Check if the file already is open
      const auto* doc_exist = man->FindDocumentByPath(file);
      if (doc_exist != nullptr) {
        continue;
      }

      man->CreateDocument(file, wxDOC_SILENT);
    }
    catch (const std::exception&) {
      // Not much to do here
      continue;
    }
  }
}

}