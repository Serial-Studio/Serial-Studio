/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/docview.h>
#include <wx/wx.h>

#include <memory>

#include "mdf/mdffile.h"
#include "mdf/mdfreader.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "mdfblock.h"

namespace mdf::viewer {

class MdfDocument : public wxDocument {
 public:
  MdfDocument() = default;
  ~MdfDocument() override;

  bool OnOpenDocument(const wxString &filename) override;

  [[nodiscard]] const mdf::MdfFile* GetFile() const {
    return !reader_  ? nullptr : reader_->GetFile();
  }
  [[nodiscard]] const mdf::MdfReader* GetReader() const {
    return reader_.get();
  }

  void SetSelectedBlockId(int64_t id, int64_t parent_id, int64_t grand_parent_id) {
    selected_id_ = id;
    parent_id_ = parent_id;
    grand_parent_id_ = grand_parent_id;
  }

  [[nodiscard]] int64_t GetSelectedBlockId() const {
    return selected_id_;
  }

  [[nodiscard]] int64_t GetParentBlockId() const {
    return parent_id_;
  }

  [[nodiscard]] int64_t GetGrandParentBlockId() const {
    return grand_parent_id_;
  }

  [[nodiscard]] mdf::detail::MdfBlock* GetBlock(int64_t id) const; ///< Returns a block pointer by block index.

 private:
  std::unique_ptr<mdf::MdfReader> reader_;
  int64_t selected_id_ = 64; ///< The selected items block index (file position).
  int64_t parent_id_ = -1; ///< The parent index of the selected block.
  int64_t grand_parent_id_ = -1; ///< The grand parent index of the selected block.

  void OnSaveAttachment(wxCommandEvent& event);
  void OnUpdateSaveAttachment(wxUpdateUIEvent& event);

  void OnShowGroupData(wxCommandEvent& event);
  void OnUpdateShowGroupData(wxUpdateUIEvent& event);

  void OnShowChannelData(wxCommandEvent& event);
  void OnUpdateShowChannelData(wxUpdateUIEvent& event);

  void OnShowSrData(wxCommandEvent& event);
  void OnUpdateShowSrData(wxUpdateUIEvent& event);

  void OnPlotChannelData(wxCommandEvent& event);
  void OnUpdatePlotChannelData(wxUpdateUIEvent& event);
  wxDECLARE_DYNAMIC_CLASS(MdfDocument);
  wxDECLARE_EVENT_TABLE();
};



}



