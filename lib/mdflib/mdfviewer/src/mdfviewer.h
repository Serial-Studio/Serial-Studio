/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <wx/wx.h>
#include <boost/asio.hpp>

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

namespace mdf::viewer {

class MdfViewer : public wxApp {
 public:

  bool OnInit() override;
  int OnExit() override;

  void OpenFile(const std::string& filename);

  // ANSI coding
  const std::string& GetMyTempDir() const {
    return my_temp_dir_;
  }

  const std::string& GnuPlot() const {
    return gnuplot_;
  }
  boost::asio::io_context ctx_;
 private:
  std::string notepad_; ///< Path to notepad.exe if it exist
  std::string gnuplot_; ///< Path to gnuplot.exe if it exist
  std::string my_temp_dir_; ///< Temporary directory for this application taht is removed at exit

  void OnOpenLogFile(wxCommandEvent& event);
  void OnUpdateOpenLogFile(wxUpdateUIEvent& event);
  void OnGnuPlotDownloadPage(wxCommandEvent& event);
  void OnUpdateGnuPlotDownloadPage(wxUpdateUIEvent& event);
  wxDECLARE_EVENT_TABLE();
};
} // namespace

wxDECLARE_APP(mdf::viewer::MdfViewer);
