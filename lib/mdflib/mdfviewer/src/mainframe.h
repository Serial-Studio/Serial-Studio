/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/wx.h>
#include <wx/docmdi.h>

namespace mdf::viewer {
class MainFrame : public wxDocMDIParentFrame {
 public:
  MainFrame(const wxString& title, const wxPoint& start_pos, const wxSize& start_size, bool maximized);
 private:
  void OnClose(wxCloseEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnUpdateNoDoc(wxUpdateUIEvent& event);
  void OnDropFiles(wxDropFilesEvent& event);
wxDECLARE_EVENT_TABLE();
};
}

