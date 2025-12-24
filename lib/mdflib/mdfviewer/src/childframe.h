/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <wx/wx.h>
#include <wx/docmdi.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>

#include "mdf3file.h"
#include "mdf4file.h"
#include "mdfdocument.h"
#include "ca4block.h"
#include "cc4block.h"

namespace mdf::viewer {
class ChildFrame : public wxDocMDIChildFrame {
 public:
  ChildFrame(wxDocument *doc,
            wxView *view,
            wxMDIParentFrame *parent,
            wxWindowID id,
            const wxString& title);
  ChildFrame() = default;

  void Update() override;
 protected:
  [[nodiscard]] MdfDocument* GetDoc();

 private:
  wxTreeCtrl* left_ = nullptr;
  wxListView* property_view_ = nullptr;
  wxListView* history_view_ = nullptr;
  wxListView* measurement_view_ = nullptr;
  wxListView* event_view_ = nullptr;
  wxListView* attachment_view_ = nullptr;
  wxListView* hierarchy_view_ = nullptr;

  wxSplitterWindow* splitter_ = nullptr;
  wxImageList image_list_;
  mdf::detail::BlockPropertyList property_list_;


  void RedrawTreeList();

  void RedrawMdf3Blocks(const mdf::detail::Mdf3File* file);
  void RedrawMdf4Blocks(const mdf::detail::Mdf4File* file);

  void RedrawHistory(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);

  void RedrawMeasurement(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawMeasurement(const mdf::detail::Hd3Block& hd, const wxTreeItemId& root);

  void RedrawHierarchy(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawAttachment(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);
  void RedrawEvent(const mdf::detail::Hd4Block& hd, const wxTreeItemId& root);

  void RedrawDataList(const mdf::detail::DataListBlock& dg, const wxTreeItemId& root);

  void RedrawCgList(const mdf::detail::Dg4Block& dg4, const wxTreeItemId& root);
  void RedrawCgList(const mdf::detail::Dg3Block& dg3, const wxTreeItemId& root);

  void RedrawCnList(const mdf::detail::Cg4Block& cg, const wxTreeItemId& root);
  void RedrawCnList(const mdf::detail::Cg3Block& cg3, const wxTreeItemId& root);

  void RedrawCnBlock(const mdf::detail::Cn4Block &cn4, const wxTreeItemId& root);
  void RedrawCaBlock(const mdf::detail::Ca4Block &ca4, const wxTreeItemId& root);

  void RedrawSrList(const mdf::detail::Cg4Block& cg, const wxTreeItemId& root);
  void RedrawSrList(const mdf::detail::Cg3Block& cg, const wxTreeItemId& root);

  void RedrawDgBlock(const mdf::detail::Dg4Block& dg4, const wxTreeItemId& root);
  void RedrawDgBlock(const mdf::detail::Dg3Block& dg3, const wxTreeItemId& root);

  void RedrawSiBlock(const mdf::detail::Si4Block& si, const wxTreeItemId& root);

  void RedrawCcBlock(const mdf::detail::Cc4Block& cc4, const wxTreeItemId& root);
  void RedrawCcBlock(const mdf::detail::Cc3Block& cc3, const wxTreeItemId& root);

  void RedrawCxList(const std::vector<std::unique_ptr<mdf::detail::MdfBlock>>& cx_list,
                    const wxTreeItemId& root);

  void RedrawChBlock(const mdf::detail::Ch4Block& ch, const wxTreeItemId& root);

  void RedrawListView();
  void RedrawHistoryView();
  void RedrawMeasurementView();
  void RedrawEventView();
  void RedrawAttachmentView();
  void RedrawHierarchyView();

  void OnTreeSelected(wxTreeEvent& event);
  void OnTreeRightClick(wxTreeEvent& event);
  void OnListItemActivated(wxListEvent& event);
  wxDECLARE_EVENT_TABLE();

};
}




