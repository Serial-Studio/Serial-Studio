/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "childframe.h"

#include <sstream>
#include <filesystem>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include "util/timestamp.h"

#include "mdfdocument.h"
#include "hl4block.h"
#include "dl4block.h"
#include "windowid.h"
#include "ca4block.h"
#include "dt4block.h"
#include "dt3block.h"
#include "dz4block.h"
#include "sr4block.h"
#include "cn4block.h"


using namespace mdf::detail;

namespace {
#include "img/sub.xpm"
#include "img/tree_list.xpm"
// Bitmap index for the tree control (tree_list.bmp)
constexpr int TREE_ROOT = 0;
constexpr int TREE_ID = 1;
constexpr int TREE_HD = 2;
constexpr int TREE_FH_ROOT = 3;
constexpr int TREE_FH = 4;
constexpr int TREE_DG_ROOT = 5;
constexpr int TREE_DG = 6;
constexpr int TREE_AT_ROOT = 7;
constexpr int TREE_AT = 8;
constexpr int TREE_CH_ROOT = 9;
constexpr int TREE_CH = 10;
constexpr int TREE_EV_ROOT = 11;
constexpr int TREE_EV = 12;
constexpr int TREE_CG = 13;
constexpr int TREE_SI = 14;
constexpr int TREE_CN = 15;
constexpr int TREE_CC = 16;
constexpr int TREE_CA = 17;
constexpr int TREE_DT = 18;
constexpr int TREE_SR = 19;
constexpr int TREE_RD = 20;
constexpr int TREE_SD = 21;
constexpr int TREE_DL = 22;
constexpr int TREE_DZ = 23;
constexpr int TREE_HL = 24;

// Fake file positions which select a list of blocks
constexpr int64_t kHistoryPosition = -100;
constexpr int64_t kMeasurementPosition = -200;
constexpr int64_t kAttachmentPosition = -300;
constexpr int64_t kEventPosition = -400;
constexpr int64_t kHierarchyPosition = -500;

class BlockAddress : public wxTreeItemData {
 public:
  explicit BlockAddress(int64_t block_address)
      : block_address_(block_address) {
  }

  [[nodiscard]] int64_t FilePosition() const {
    return block_address_;
  }

 private:
  int64_t block_address_ = 0;
};

wxString CreateBlockText(const mdf::detail::MdfBlock&block) {
  std::ostringstream block_string;
  block_string << block.BlockType();
  const auto comment = block.Comment();
  if (!comment.empty()) {
    block_string << " (" << comment << ")";
  }
  return wxString::FromUTF8(block_string.str());
}

int64_t GetBlockId(const wxTreeCtrl& list, const wxTreeItemId& item) {
  if (!item.IsOk()) {
    return -1;
  }
  const auto* data = dynamic_cast<const BlockAddress*>(list.GetItemData(item));
  return data != nullptr ? data->FilePosition() : -1;
}

wxTreeItemId FindId(const wxTreeCtrl& list, const wxTreeItemId &root, int64_t id) { //NOLINT
   for (auto item = root; item.IsOk(); item = list.GetNextSibling(item)) {
    if (GetBlockId(list, item) == id) {
      return item;
    }
    if (list.ItemHasChildren(item)) {
      wxTreeItemIdValue cookie;
      auto find = FindId(list, list.GetFirstChild(item, cookie), id);
      if (find.IsOk()) {
        return find;
      }
    }
  }
  return {};
}


} // Empty namespace

namespace mdf::viewer {

wxBEGIN_EVENT_TABLE(ChildFrame, wxDocMDIChildFrame) // NOLINT(cert-err58-cpp)
        EVT_TREE_SEL_CHANGED(kIdLeftTree,ChildFrame::OnTreeSelected)
        EVT_LIST_ITEM_ACTIVATED(kIdPropertyList, ChildFrame::OnListItemActivated)
        EVT_TREE_ITEM_RIGHT_CLICK(kIdLeftTree, ChildFrame::OnTreeRightClick)
wxEND_EVENT_TABLE()

ChildFrame::ChildFrame(wxDocument *doc,
                     wxView *view,
                     wxMDIParentFrame *parent,
                     wxWindowID id,
                     const wxString& title)
    : wxDocMDIChildFrame(doc, view, parent, id, title, wxDefaultPosition, wxDefaultSize,
                         wxDEFAULT_FRAME_STYLE, wxASCII_STR(wxFrameNameStr)),
                         image_list_(16,16,false,25) {
#ifdef _WIN32
  wxIcon sub("SUB_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
#else
  wxIcon sub {wxICON(sub)};
#endif
  SetIcon(sub);

  auto* main_panel = new wxPanel(this);

  splitter_ = new wxSplitterWindow(main_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxCLIP_CHILDREN);

  left_ = new wxTreeCtrl(splitter_,kIdLeftTree);
  property_view_ = new wxListView(splitter_, kIdPropertyList, wxDefaultPosition, wxDefaultSize,
                                  wxLC_REPORT | wxLC_SINGLE_SEL);
  property_view_->AppendColumn("Property", wxLIST_FORMAT_LEFT, 150);
  property_view_->AppendColumn("Value", wxLIST_FORMAT_LEFT, 200);
  property_view_->AppendColumn("Description", wxLIST_FORMAT_LEFT, 200);


  history_view_ = new wxListView(splitter_, kIdHistoryList, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
  history_view_->AppendColumn("Time", wxLIST_FORMAT_LEFT, 150);
  history_view_->AppendColumn("User", wxLIST_FORMAT_LEFT, 100);
  history_view_->AppendColumn("Description", wxLIST_FORMAT_LEFT, 100);
  history_view_->AppendColumn("Tool ID", wxLIST_FORMAT_LEFT, 100);
  history_view_->AppendColumn("Tool Vendor", wxLIST_FORMAT_LEFT, 100);
  history_view_->AppendColumn("Tool Version", wxLIST_FORMAT_LEFT, 100);
  history_view_->Hide();

  measurement_view_ = new wxListView(splitter_, kIdMeasurementList, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
  measurement_view_->AppendColumn("Measurement", wxLIST_FORMAT_LEFT, 100);
  measurement_view_->AppendColumn("Samples", wxLIST_FORMAT_LEFT, 100);
  measurement_view_->AppendColumn("Description", wxLIST_FORMAT_LEFT, 400);
  measurement_view_->AppendColumn("Channel Groups", wxLIST_FORMAT_LEFT, 400);
  measurement_view_->Hide();

  event_view_ = new wxListView(splitter_, kIdEventList, wxDefaultPosition, wxDefaultSize,
                                     wxLC_REPORT | wxLC_SINGLE_SEL);
  event_view_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  event_view_->AppendColumn("Type", wxLIST_FORMAT_LEFT, 75);
  event_view_->AppendColumn("Value", wxLIST_FORMAT_LEFT, 75);
  event_view_->AppendColumn("Description", wxLIST_FORMAT_LEFT, 200);
  event_view_->AppendColumn("Range", wxLIST_FORMAT_LEFT, 75);
  event_view_->AppendColumn("Cause", wxLIST_FORMAT_LEFT, 75);
  event_view_->Hide();

  attachment_view_ = new wxListView(splitter_, kIdAttachmentList, wxDefaultPosition, wxDefaultSize,
                               wxLC_REPORT | wxLC_SINGLE_SEL);
  attachment_view_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  attachment_view_->AppendColumn("Embedded", wxLIST_FORMAT_LEFT, 100);
  attachment_view_->AppendColumn("Type", wxLIST_FORMAT_LEFT, 150);
  attachment_view_->AppendColumn("Path", wxLIST_FORMAT_LEFT, 300);
  attachment_view_->Hide();

  hierarchy_view_ = new wxListView(splitter_, kIdHierarchyList, wxDefaultPosition, wxDefaultSize,
                                    wxLC_REPORT | wxLC_SINGLE_SEL);
  hierarchy_view_->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
  hierarchy_view_->AppendColumn("Type", wxLIST_FORMAT_LEFT, 200);
  hierarchy_view_->Hide();

  splitter_->SplitVertically(left_, property_view_, 400);
#ifdef _WIN32
  wxBitmap tree_list("TREE_LIST", wxBITMAP_TYPE_BMP_RESOURCE);
#else
  wxBitmap tree_list {wxICON(tree_list)};
#endif

  image_list_.Add(tree_list);
  left_->SetImageList(&image_list_);

  auto* main_sizer = new wxBoxSizer(wxVERTICAL);
  main_sizer->Add(splitter_, 1 , wxALL | wxGROW,0);
  main_panel->SetSizerAndFit(main_sizer);
}

void ChildFrame::RedrawTreeList() {

  left_->DeleteAllItems();

  auto* doc = wxDynamicCast(GetDocument(), MdfDocument); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
  if (doc == nullptr) {
    return;
  }
  const auto* reader = doc->GetReader();
  if (reader == nullptr) {
    return;
  }

  left_->AddRoot(reader->ShortName(), TREE_ROOT, TREE_ROOT);

  const auto* file = doc->GetFile();
   if (file == nullptr) {
    return;
  }

  const auto* file3 = dynamic_cast<const mdf::detail::Mdf3File*>(file);
  const auto* file4 = dynamic_cast<const mdf::detail::Mdf4File*>(file);

  if (file4 != nullptr) {
    RedrawMdf4Blocks(file4);
  } else if (file3 != nullptr) {
    RedrawMdf3Blocks(file3);
  }

  auto selected_id = doc->GetSelectedBlockId();
  auto found = FindId(left_, left_->GetRootItem(), selected_id);
  if (found.IsOk()) {
    left_->SelectItem(found);
    left_->EnsureVisible(found);
  }
}

void ChildFrame::RedrawMdf3Blocks(const mdf::detail::Mdf3File *file) {
  if (file == nullptr) {
    return;
  }

  auto root = left_->GetRootItem();

  // File identity block
  const auto& id = file->Id();
  std::ostringstream id_string;
  id_string << "ID (" << id.VersionString() << ")";
  left_->AppendItem(root, wxString::FromUTF8(id_string.str()),
                    TREE_ID, TREE_ID, new BlockAddress(id.FilePosition()));

  // Header block
  const auto& hd = file->Hd();
  const auto& hd_string = CreateBlockText(hd);
  auto hd_root = left_->AppendItem(root, hd_string,
                                   TREE_HD, TREE_HD, new BlockAddress(hd.FilePosition()));

  RedrawMeasurement(hd, hd_root);

}

void ChildFrame::RedrawMdf4Blocks(const mdf::detail::Mdf4File *file) {
  if (file == nullptr) {
    return;
  }

  auto root = left_->GetRootItem();

    // File identity block
  const auto& id = file->Id();
  std::ostringstream id_string;
  id_string << "ID (" << id.VersionString() << ")";
  left_->AppendItem(root, wxString::FromUTF8(id_string.str()),
                    TREE_ID, TREE_ID, new BlockAddress(id.FilePosition()));

  // Header block
  const auto& hd = file->Hd();
  const auto& hd_string = CreateBlockText(hd);
  auto hd_root = left_->AppendItem(root, hd_string,
                                   TREE_HD, TREE_HD, new BlockAddress(hd.FilePosition()));

  RedrawHistory(hd, hd_root);
  RedrawMeasurement(hd, hd_root);
  RedrawHierarchy(hd,hd_root);
  RedrawAttachment(hd, hd_root);
  RedrawEvent(hd, hd_root);
}

void ChildFrame::Update() {
  static bool recursive_lock = false;
  wxDocMDIChildFrame::Update();
  if (!recursive_lock) {
    recursive_lock = true;
    RedrawTreeList();
    RedrawListView();
    recursive_lock = false;
  }
}

void ChildFrame::RedrawHistory(const detail::Hd4Block &hd, const wxTreeItemId &root) {
  if (hd.Fh4().empty()) {
    return;
  }
  auto fh_root = left_->AppendItem(root, "History", TREE_FH_ROOT,TREE_FH_ROOT,
                                   new BlockAddress(kHistoryPosition));
  for (const auto& fh4 : hd.Fh4()) {
    if (!fh4) {
      continue;
    }
    left_->AppendItem(fh_root, CreateBlockText(*fh4), TREE_FH, TREE_FH,
                      new BlockAddress(fh4->FilePosition()));
  }
}

void ChildFrame::RedrawMeasurement(const detail::Hd4Block &hd, const wxTreeItemId &root) {
  if (hd.Dg4().empty()) {
    return;
  }
  auto dg_root = left_->AppendItem(root, "Measurement", TREE_DG_ROOT, TREE_DG_ROOT,
                                   new BlockAddress(kMeasurementPosition));
  for (const auto& dg4 : hd.Dg4()) {
    if (!dg4) {
      continue;
    }
    RedrawDgBlock(*dg4,dg_root);
   }
}

void ChildFrame::RedrawMeasurement(const detail::Hd3Block &hd, const wxTreeItemId &root) {
  if (hd.Dg3().empty()) {
    return;
  }
  auto dg_root = left_->AppendItem(root, "Measurement", TREE_DG_ROOT, TREE_DG_ROOT,
                                   new BlockAddress(kMeasurementPosition));
  for (const auto& dg3 : hd.Dg3()) {
    if (!dg3) {
      continue;
    }
    RedrawDgBlock(*dg3,dg_root);
  }
}
void ChildFrame::RedrawHierarchy(const detail::Hd4Block &hd, const wxTreeItemId &root) {
  if (hd.Ch4().empty()) {
    return;
  }
  auto ch_root = left_->AppendItem(root, "Channel Hierarchy", TREE_CH_ROOT, TREE_CH_ROOT,
                                   new BlockAddress(kHierarchyPosition));
  for (const auto& ch4 : hd.Ch4()) {
    if (!ch4) {
      continue;
    }
    RedrawChBlock(*ch4, ch_root);
  }
}

void ChildFrame::RedrawAttachment(const detail::Hd4Block &hd, const wxTreeItemId &root) {
  if (hd.At4().empty()) {
    return;
  }
  auto at_root = left_->AppendItem(root, "Attachment", TREE_AT_ROOT, TREE_AT_ROOT,
                                   new BlockAddress(kAttachmentPosition));
  for (const auto &at4: hd.At4()) {
    if (!at4) {
      continue;
    }
    std::string filename = at4->FileName();
    if (filename.empty()) {
      filename = CreateBlockText(*at4);
    }
    std::ostringstream block_string;
    block_string << at4->BlockType();
    if (!filename.empty()) {
      block_string << " (" << filename << ")";
    }
    left_->AppendItem(at_root, wxString::FromUTF8(block_string.str()),
                      TREE_AT, TREE_AT, new BlockAddress(at4->FilePosition()));
  }
}

void ChildFrame::RedrawEvent(const detail::Hd4Block &hd, const wxTreeItemId &root) {
  if (hd.Ev4().empty()) {
    return;
  }
  auto ev_root = left_->AppendItem(root, "Event", TREE_EV_ROOT, TREE_EV_ROOT,
                                   new BlockAddress(kEventPosition));
  for (const auto& ev4 : hd.Ev4()) {
    if (!ev4) {
      continue;
    }
    left_->AppendItem(ev_root, CreateBlockText(*ev4),
                        TREE_EV, TREE_EV, new BlockAddress(ev4->FilePosition()));
  }
}


void ChildFrame::RedrawDataList(const mdf::detail::DataListBlock& dg, const wxTreeItemId& root) {
  for (const auto& data : dg.DataBlockList()) {
    if (!data) {
      continue;
    }
    std::ostringstream block_string;
    block_string << data->BlockType();

    const auto* data_block = dynamic_cast<const DataBlock*>(data.get());
    const auto* data_block_list = dynamic_cast<const DataListBlock*>(data.get());
    uint64_t nof_data_bytes = data_block != nullptr ? data_block->DataSize() : 0;
    if (data_block_list != nullptr) {
      nof_data_bytes = data_block_list->DataSize();
    }

    if (data->BlockType() == "DT") {
      block_string << " (Size {bytes]: " << nof_data_bytes << ")";
      left_->AppendItem(root, wxString::FromUTF8(block_string.str()),
                        TREE_DT, TREE_DT,
                        new BlockAddress(data->FilePosition()));
    } else if (data->BlockType() == "DV" || data->BlockType() == "DI") {
      block_string << " (Size {bytes]: " << nof_data_bytes << ")";
      left_->AppendItem(root,wxString::FromUTF8(block_string.str()),
                        TREE_DT, TREE_DT,
                        new BlockAddress(data->FilePosition()));
    } else if (data->BlockType() == "DZ") {
      const auto* dz4 = dynamic_cast<const Dz4Block*>(data.get());
      if (dz4 != nullptr) {
        block_string << "/" << dz4->OrigBlockType()
          << " (Size [bytes]: " << dz4->CompressedDataSize()
          << "/" << dz4->DataSize() << ")";
      }
      left_->AppendItem(root,wxString::FromUTF8(block_string.str()),
                        TREE_DZ, TREE_DZ, new BlockAddress(data->FilePosition()));
    } else if (data->BlockType() == "RD" || data->BlockType() == "RV" || data->BlockType() == "RI") {
      block_string << " (Size [bytes]: " << nof_data_bytes << ")";
      left_->AppendItem(root, wxString::FromUTF8(block_string.str()),
                        TREE_RD, TREE_RD, new BlockAddress(data->FilePosition()));
    } else if (data->BlockType() == "SD") {
      block_string << " (Size [bytes]: " << nof_data_bytes << ")";
      left_->AppendItem(root, wxString::FromUTF8(block_string.str()),
                        TREE_SD, TREE_SD, new BlockAddress(data->FilePosition()));
    } else if (data->BlockType() == "DL") {
      block_string << " (Size [bytes]: " << nof_data_bytes << ")";
      auto dl_root = left_->AppendItem(root, wxString::FromUTF8(block_string.str()),
                        TREE_DL, TREE_DL, new BlockAddress(data->FilePosition()));
      const auto* dl = dynamic_cast<const mdf::detail::Dl4Block*>(data.get());
      RedrawDataList(*dl, dl_root);
    } else if (data->BlockType() == "LD") {
      auto ld_root = left_->AppendItem(root, CreateBlockText(*data),
                                       TREE_DL, TREE_DL,
                                       new BlockAddress(data->FilePosition()));
      const auto* data_list = dynamic_cast<const mdf::detail::DataListBlock*>(data.get());
      if (data_list != nullptr) {
        RedrawDataList(*data_list, ld_root);
      }
    } else if (data->BlockType() == "HL") {
      auto hl_root = left_->AppendItem(root, CreateBlockText(*data),
                      TREE_HL, TREE_HL, new BlockAddress(data->FilePosition()));
      const auto* hl = dynamic_cast<const mdf::detail::Hl4Block*>(data.get());
      RedrawDataList(*hl, hl_root);
     } else if (data->BlockType() == "SR") {
      auto sr_root = left_->AppendItem(root, CreateBlockText(*data),
                                       TREE_SR, TREE_SR, new BlockAddress(data->FilePosition()));
      const auto* sr = dynamic_cast<const mdf::detail::Sr4Block*>(data.get());
      RedrawDataList(*sr, sr_root);
    }
  }
}


void ChildFrame::RedrawCgList(const mdf::detail::Dg4Block& dg, const wxTreeItemId& root) {
  if (dg.Cg4().empty()) {
    return;
  }
  for (const auto& cg4 : dg.Cg4()) {
    if (!cg4) {
      continue;
    }

    std::ostringstream sub_string;
    if (!cg4->Comment().empty()) {
      sub_string << cg4->Comment();
    }
    if (!cg4->Name().empty()) {
      if (!sub_string.str().empty()) {
        sub_string << "/";
      }
      sub_string << cg4->Name();
    }
    if (!sub_string.str().empty()) {
      sub_string << " ";
    }
    sub_string << "Samples: " << cg4->NofSamples();

    std::ostringstream cg_string;
    cg_string << cg4->BlockType() << " (" << sub_string.str() << ")";

    auto root_cg = left_->AppendItem(root, cg_string.str(),
                                     TREE_CG, TREE_CG, new BlockAddress(cg4->FilePosition()));

    if (const auto* si = cg4->Source(); si != nullptr) {
      RedrawSiBlock(*si, root_cg);
    }
    RedrawCnList(*cg4, root_cg);
    RedrawSrList(*cg4, root_cg);
  }
}

void ChildFrame::RedrawCgList(const mdf::detail::Dg3Block& dg3, const wxTreeItemId& root) {
  if (dg3.Cg3().empty()) {
    return;
  }
  for (const auto& cg3 : dg3.Cg3()) {
    if (!cg3) {
      continue;
    }

    std::ostringstream sub_string;
    if (!cg3->Comment().empty()) {
      sub_string << cg3->Comment();
    }
    if (!cg3->Name().empty()) {
      if (!sub_string.str().empty()) {
        sub_string << "/";
      }
      sub_string << cg3->Name();
    }
    if (!sub_string.str().empty()) {
      sub_string << " ";
    }
    sub_string << "Samples: " << cg3->NofSamples();

    std::ostringstream cg_string;
    cg_string << cg3->BlockType() << " (" << sub_string.str() << ")";

    auto root_cg = left_->AppendItem(root, cg_string.str(),
                                     TREE_CG, TREE_CG, new BlockAddress(cg3->FilePosition()));
    RedrawCnList(*cg3, root_cg);
    RedrawSrList(*cg3, root_cg);
  }
}

void ChildFrame::RedrawCnList(const detail::Cg4Block &cg, const wxTreeItemId &root) {
  if (cg.Cn4().empty()) {
    return;
  }
  for (const auto& cn : cg.Cn4()) {
    if (!cn) {
      continue;
    }
    RedrawCnBlock(*cn, root);
  }
}

void ChildFrame::RedrawCnBlock(const detail::Cn4Block &cn,
                               const wxTreeItemId &root) {
    std::ostringstream sub_string;
    const std::string cn_name = cn.Name();
    const std::string cn_unit = cn.Unit();

    sub_string << cn_name;
    if (!cn_unit.empty()) {
      sub_string << " [" << cn_unit << "]";
    }

    std::ostringstream cn_string;
    cn_string << cn.BlockType() << " (" << sub_string.str() << ")";


    for (size_t array = 0;cn.ChannelArray(array) != nullptr ; ++array) {
      if (const auto* channel_array = cn.ChannelArray(array); channel_array != nullptr) {
          cn_string << " " << channel_array->DimensionAsString();
      }

    }


    auto cn_root = left_->AppendItem(root, wxString::FromUTF8(cn_string.str()),
                      TREE_CN, TREE_CN, new BlockAddress(cn.FilePosition()));
    const auto* si = cn.Si();
    if (si != nullptr) {
      RedrawSiBlock(*si,cn_root);
    }
    const auto* cc = cn.Cc();
    if (cc != nullptr) {
      RedrawCcBlock(*cc,cn_root);
    }
    RedrawCxList(cn.Cx4(), cn_root);
    RedrawDataList(cn, cn_root);
}

void ChildFrame::RedrawCaBlock(const Ca4Block &ca4, const wxTreeItemId &root) {
  const std::string type = ca4.TypeAsString();
  const std::string storage = ca4.StorageAsString();

  std::ostringstream sub_string;
  sub_string << type << "/" << storage;


  std::ostringstream label;
  label << ca4.BlockType()
    << " (" << sub_string.str() << ") "
    << ca4.DimensionAsString();

 left_->AppendItem(root, wxString::FromUTF8(label.str()), TREE_CA, TREE_CA,
                                   new BlockAddress(ca4.FilePosition()));
}

void ChildFrame::RedrawCnList(const detail::Cg3Block &cg3, const wxTreeItemId &root) {
  if (cg3.Cn3().empty()) {
    return;
  }
  for (const auto& cn3 : cg3.Cn3()) {
    if (!cn3) {
      continue;
    }

    std::ostringstream sub_string;
    std::string cn_name = cn3->Name();
    std::string cn_unit = cn3->Unit();

    sub_string << cn_name;
    if (!cn_unit.empty()) {
      sub_string << " [" << cn_unit << "]";
    }

    std::ostringstream cn_string;
    cn_string << cn3->BlockType() << " (" << sub_string.str() << ")";

    auto cn_root = left_->AppendItem(root, wxString::FromUTF8(cn_string.str()),
                                     TREE_CN, TREE_CN, new BlockAddress(cn3->FilePosition()));

    if ( const auto* cc3 = cn3->Cc3(); cc3 != nullptr) {
      RedrawCcBlock(*cc3,cn_root);
    }


    if (const auto* cd3 = cn3->Cd3(); cd3 != nullptr) {
      left_->AppendItem(cn_root, CreateBlockText(*cd3),
      TREE_CH, TREE_CH, new BlockAddress(cd3->FilePosition()));
    }

    if (const auto* ce3 = cn3->Ce3(); ce3 != nullptr) {
      left_->AppendItem(cn_root, CreateBlockText(*ce3),
      TREE_SI, TREE_SI, new BlockAddress(ce3->FilePosition()));
    }
    RedrawDataList(*cn3, cn_root);
  }
}

void ChildFrame::RedrawSrList(const detail::Cg4Block &cg4, const wxTreeItemId &root) {
  for (const auto& sr4 : cg4.Sr4()) {
    if (!sr4) {
      continue;
    }
    std::ostringstream sub_string;
    sub_string << "Samples: " << sr4->NofSamples()
               <<", Interval: " << sr4->Interval();

    std::ostringstream sr_string;
    sr_string << sr4->BlockType() << " (" << sub_string.str() << ")";
    auto sr_root = left_->AppendItem(root,
           wxString::FromUTF8(sr_string.str()),
          TREE_SR, TREE_SR,
          new BlockAddress(sr4->FilePosition()));

    RedrawDataList(*sr4,sr_root);
  }
}

void ChildFrame::RedrawSrList(const detail::Cg3Block &cg3, const wxTreeItemId &root) {
  for (const auto& sr : cg3.Sr3()) {
    if (!sr) {
      continue;
    }
    auto sr_root = left_->AppendItem(root, sr->BlockType(),
          TREE_SR, TREE_SR, new BlockAddress(sr->FilePosition()));

    RedrawDataList(*sr,sr_root);
  }
}

void ChildFrame::RedrawSiBlock(const detail::Si4Block &si, const wxTreeItemId &root) {
  std::ostringstream sub_string;
  const auto& si_name = si.Name();
  const auto& si_path = si.Path();
  const auto si_desc = si.Comment();

  if (!si_name.empty()) {
    sub_string << si_name;
  }
  if (!si_path.empty()) {
    if (sub_string.str().empty()) {
      sub_string << si_path;
    } else {
      sub_string << ":" << si_path;
    }
  }
  if (!si_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << si_desc;
    } else {
      sub_string << " " << si_desc;
    }
  }

  std::ostringstream si_string;
  si_string << si.BlockType() << " (" << sub_string.str() << ")";

  left_->AppendItem(root, wxString::FromUTF8(si_string.str()),
                                   TREE_SI, TREE_SI, new BlockAddress(si.FilePosition()));
}

void ChildFrame::RedrawDgBlock(const detail::Dg4Block &dg4, const wxTreeItemId &root) {
  std::ostringstream label;
  label << dg4.BlockType();
  const std::string desc = dg4.Description();

  if (!desc.empty()) {
    label << " (" << desc << ")";
  } else if (dg4.Cg4().size() == 1) {
    std::ostringstream sub;
    const auto* cg4 = dg4.Cg4().front().get();
    if (cg4 != nullptr) {
      sub << cg4->Name();
      sub << " Samples: " << cg4->NofSamples();
      label << " (" << sub.str() << ")";
    }

    // If the DG only have one CG, use the CG Name instead

  }
  auto root_dg = left_->AppendItem(root, label.str(), TREE_DG, TREE_DG,
                                   new BlockAddress(dg4.FilePosition()));
  RedrawDataList(dg4,root_dg);
  RedrawCgList(dg4, root_dg);

}

void ChildFrame::RedrawDgBlock(const detail::Dg3Block &dg3, const wxTreeItemId &root) {
  auto root_dg = left_->AppendItem(root, CreateBlockText(dg3),
                                   TREE_DG, TREE_DG, new BlockAddress(dg3.FilePosition()));
  RedrawDataList(dg3,root_dg);
  RedrawCgList(dg3, root_dg);

  if (const auto* tr3 = dg3.Tr3(); tr3 != nullptr) {
    left_->AppendItem(root_dg, CreateBlockText(*tr3),
                      TREE_EV, TREE_EV, new BlockAddress(tr3->FilePosition()));
  }
}


void ChildFrame::RedrawCcBlock(const detail::Cc4Block &cc4, const wxTreeItemId &root) {
  std::ostringstream sub_string;
  const std::string cc_name = cc4.Name();
  const std::string cc_unit = cc4.Unit();
  const std::string cc_desc = cc4.Comment();

  if (!cc_name.empty()) {
    sub_string << cc_name;
  }
  if (!cc_unit.empty()) {
    if (sub_string.str().empty()) {
      sub_string << "[" << cc_unit << "]";
    } else {
      sub_string << " [" << cc_unit << "]";
    }
  }
  if (!cc_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << cc_desc;
    } else {
      sub_string << " " << cc_desc;
    }
  }

  std::ostringstream cc_string;
  cc_string << cc4.BlockType();
  if (!sub_string.str().empty() ) {
    cc_string << " (" << sub_string.str() << ")";
  }

  auto cc_root = left_->AppendItem(root, wxString::FromUTF8(cc_string.str()),
                    TREE_CC, TREE_CC, new BlockAddress(cc4.FilePosition()));

  if ( const auto* reverse = cc4.Cc(); reverse != nullptr) {
    RedrawCcBlock(*reverse, cc_root);
  }
  const auto& ref_list = cc4.References();
  for (const auto& ref : ref_list) {
    if (!ref || ref->BlockType() != "CC") {
      continue;
    }
    const auto* ref4 = dynamic_cast<const mdf::detail::Cc4Block*>(ref.get());
    if (ref4 != nullptr) {
      RedrawCcBlock(*ref4, cc_root);
    }
  }
}

void ChildFrame::RedrawCxList(const std::vector<std::unique_ptr<mdf::detail::MdfBlock>>& cx_list,
                               const wxTreeItemId &root) {
  if (cx_list.empty()) {
    return;
  }

  for (const auto& cx4 : cx_list) {
    if (!cx4) {
      continue;
    }
    if (cx4->BlockType() == "CA") {
      const auto* ca_block = dynamic_cast<const detail::Ca4Block*>(cx4.get());
      if (ca_block == nullptr) {
        continue;
      }
      RedrawCaBlock(*ca_block, root);
    } else if (cx4->BlockType() == "CN") {
      const auto* cn_block = dynamic_cast<const detail::Cn4Block*>(cx4.get());
      if (cn_block == nullptr) {
        continue;
      }
      RedrawCnBlock(*cn_block, root);
    }
  }
}

void ChildFrame::RedrawCcBlock(const detail::Cc3Block &cc3, const wxTreeItemId &root) {
  std::ostringstream sub_string;
  std::string cc_name = cc3.Name();
  std::string cc_unit = cc3.Unit();
  std::string cc_desc = cc3.Comment();

  if (!cc_name.empty()) {
    sub_string << cc_name;
  }
  if (!cc_unit.empty()) {
    if (sub_string.str().empty()) {
      sub_string << "[" << cc_unit << "]";
    } else {
      sub_string << " [" << cc_unit << "]";
    }
  }
  if (!cc_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << cc_desc;
    } else {
      sub_string << " " << cc_desc;
    }
  }

  std::ostringstream cc_string;
  cc_string << cc3.BlockType();
  if (!sub_string.str().empty() ) {
    cc_string << " (" << sub_string.str() << ")";
  }

  left_->AppendItem(root, wxString::FromUTF8(cc_string.str()),
                                   TREE_CC, TREE_CC, new BlockAddress(cc3.FilePosition()));
}

void ChildFrame::RedrawChBlock(const detail::Ch4Block &ch, const wxTreeItemId &root) {
  std::ostringstream sub_string;
  const auto& ch_name = ch.Name();
  std::string ch_desc = ch.Comment();

  if (!ch_name.empty()) {
    sub_string << ch_name;
  }
  if (!ch_desc.empty()) {
    if (sub_string.str().empty()) {
      sub_string << ch_desc;
    } else {
      sub_string << ":" << ch_desc;
    }
  }

  std::ostringstream ch_string;
  ch_string << ch.BlockType();
  if (!sub_string.str().empty() ) {
    ch_string << " (" << sub_string.str() << ")";
  }

  auto ch_root = left_->AppendItem(root, wxString::FromUTF8(ch_string.str()),
                                   TREE_CH, TREE_CH, new BlockAddress(ch.FilePosition()));
  for (const auto& p : ch.Ch()) {
    if (!p) {
      continue;
    }
    RedrawChBlock(*p,ch_root);
  }
}


void ChildFrame::OnTreeSelected(wxTreeEvent& event) {
  const auto selected_item = event.GetItem();
  const auto selected_id = GetBlockId(left_,selected_item);

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }

    // Check which list
  auto* current_window = splitter_->GetWindow2();
  switch (selected_id) {
    case kHistoryPosition:
      if (current_window != history_view_) {
        current_window->Hide();
        splitter_->ReplaceWindow(current_window, history_view_);
        history_view_->Show();
        RedrawHistoryView();
      }
      return;

    case kMeasurementPosition:
      if (current_window != measurement_view_) {
        current_window->Hide();
        splitter_->ReplaceWindow(current_window, measurement_view_);
        measurement_view_->Show();
        RedrawMeasurementView();
      }
      return;

    case kEventPosition:
      if (current_window != event_view_) {
        current_window->Hide();
        splitter_->ReplaceWindow(current_window, event_view_);
        event_view_->Show();
        RedrawEventView();
      }
      return;

    case kAttachmentPosition:
      if (current_window != attachment_view_) {
        current_window->Hide();
        splitter_->ReplaceWindow(current_window, attachment_view_);
        attachment_view_->Show();
        RedrawAttachmentView();
      }
      return;

    case kHierarchyPosition:
      if (current_window != hierarchy_view_) {
        current_window->Hide();
        splitter_->ReplaceWindow(current_window, hierarchy_view_);
        hierarchy_view_->Show();
        RedrawHierarchyView();
      }
      return;

    default:
      // Default is the property view
      break;
  }

  if (current_window != property_view_) {
    current_window->Hide();
    splitter_->ReplaceWindow(current_window, property_view_);
    property_view_->Show();
  }
  if (selected_id == doc->GetSelectedBlockId()) {
    return;
  }

  auto parent_item = selected_item.IsOk() ? left_->GetItemParent(selected_item) : wxTreeItemId();
  auto parent_id = GetBlockId(left_,parent_item);

  auto grand_parent_item = parent_item.IsOk() ? left_->GetItemParent(parent_item) : wxTreeItemId();
  auto grand_parent_id = GetBlockId(left_,grand_parent_item);

  doc->SetSelectedBlockId(selected_id, parent_id, grand_parent_id);
  RedrawListView();
}

void ChildFrame::OnTreeRightClick(wxTreeEvent& event) {
  OnTreeSelected(event);
  wxMenu menu;
  menu.Append(kIdShowGroupData, "Show Group Data");
  menu.Append(kIdShowChannelData, "Show Channel Data");
  menu.Append(kIdShowSrData, "Show Sample Reduction Data");
  menu.AppendSeparator();
  menu.Append(kIdPlotChannelData, "Plot Channel Data");
  menu.AppendSeparator();
  menu.Append(kIdSaveAttachment, "Save Attachment File");
  menu.AppendSeparator();
  menu.Append(kIdOpenLogFile, "Open Log File");

  PopupMenu(&menu);
}

void ChildFrame::OnListItemActivated(wxListEvent& event) {
  if (splitter_->GetWindow2() != property_view_) {
    return;
  }

  auto index = event.GetIndex();
  if (index < 0) {
    return;
  }

  const auto line = static_cast<size_t>(index);
  if (line >= property_list_.size()) {
    return;
  }

  auto block_id = property_list_[line].Link();
  if (block_id <= 0) {
    return;
  }

  auto* doc = wxDynamicCast(GetDocument(),MdfDocument ); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
  if (doc == nullptr) {
    return;
  }

  const auto* file = doc->GetFile();
  if (file == nullptr) {
    return;
  }

  const mdf::detail::MdfBlock* block = nullptr;
  if (file->IsMdf4()) {
    const auto* file4 = dynamic_cast<const mdf::detail::Mdf4File*>(file);
    if (file4 != nullptr) {
      block = file4->Find(block_id);
    }
  } else {
    const auto* file3 = dynamic_cast<const mdf::detail::Mdf3File*>(file);
    if (file3 != nullptr) {
      block = file3->Find(block_id);
    }
  }
  if (block == nullptr) {
    return;
  }
    // If TX block, show the text in a dialog window
  if (block->BlockType() == "TX") {
    if (file->IsMdf4()) {
      const auto* tx = dynamic_cast<const mdf::detail::Tx4Block*>(block);
      if (tx != nullptr) {
        wxMessageBox(wxString::FromUTF8(tx->Text()), "TX Block (Text)", wxICON_INFORMATION);
      }
    } else {
      const auto* tx = dynamic_cast<const mdf::detail::Tx3Block*>(block);
      if (tx != nullptr) {
        wxMessageBox(wxString::FromUTF8(tx->Text()), "TX Block (Text)", wxICON_INFORMATION);
      }
    }
    return;
  }

  if (block->BlockType() == "MD") {
    // Show the XML in a dialog window
    const auto* md = dynamic_cast<const mdf::detail::Md4Block*>(block);
    if (md != nullptr) {
      wxMessageBox(wxString::FromUTF8(md->Text()), "MD block (XML)", wxICON_INFORMATION);
    }
    return;
  }

  if (block->BlockType() == "PR") {
    // Show the XML in a dialog window
    const auto* pr3 = dynamic_cast<const mdf::detail::Pr3Block*>(block);
    if (pr3 != nullptr) {
      wxMessageBox(wxString::FromUTF8(pr3->Text()), "PR block", wxICON_INFORMATION);
    }
    return;
  }

  auto tree_item = FindId(left_,left_->GetRootItem(), block_id);
  int64_t parent_id = -1;
  int64_t grand_parent_id = -1;
  if (tree_item.IsOk()) {
    auto parent_item = left_->GetItemParent(tree_item);
    if (parent_item.IsOk()) {
      parent_id = GetBlockId(left_,parent_item);
      auto grand_parent_item = left_->GetItemParent(parent_item);
      if (grand_parent_item.IsOk()) {
        grand_parent_id = GetBlockId(left_,grand_parent_item);
      }
    }
    doc->SetSelectedBlockId(block_id, parent_id, grand_parent_id);
    left_->SelectItem(tree_item);
    left_->EnsureVisible(tree_item);
    doc->SetSelectedBlockId(block_id, parent_id, grand_parent_id);
  } else {
    doc->SetSelectedBlockId(block_id, parent_id, grand_parent_id);
  }
  RedrawListView();

}

void ChildFrame::RedrawListView() {
  property_view_->DeleteAllItems();
  property_list_.clear();

  if (splitter_->GetWindow2() != property_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }

  auto id = doc->GetSelectedBlockId();
  const auto* block = doc->GetBlock(id);
  if (block == nullptr) {
    return;
  }

  block->GetBlockProperty(property_list_);

  long line = 0;
  for (const auto& prop : property_list_) {
    switch (prop.Type()) {
      case mdf::detail::BlockItemType::HeaderItem:
      {
        auto index = property_view_->InsertItem(line, wxString::FromUTF8(prop.Label()));
        auto font = property_view_->GetItemFont(index);
        font.MakeItalic();
        property_view_->SetItemFont(index, font);
      }
      break;

      case mdf::detail::BlockItemType::LinkItem:
      {
        auto index = property_view_->InsertItem(line, wxString::FromUTF8(prop.Label()));
        if (prop.Link() > 0) {
          property_view_->SetItem(index, 1, wxString::FromUTF8(prop.Value()));
          property_view_->SetItem(index, 2, wxString::FromUTF8(prop.Description()));
        } else if (!prop.Description().empty()) {
          property_view_->SetItem(index, 2, wxString::FromUTF8(prop.Description()));
        }
      }
      break;

      case mdf::detail::BlockItemType::BlankItem:
      {
        auto index = property_view_->InsertItem(line, "");
        property_view_->SetItem(index, 1, "");
        property_view_->SetItem(index, 2, "");
      }
        break;

      default:
      {
        auto index = property_view_->InsertItem(line, wxString::FromUTF8(prop.Label()));
        property_view_->SetItem(index, 1, wxString::FromUTF8(prop.Value()));
        property_view_->SetItem(index, 2, wxString::FromUTF8(prop.Description()));
      }
      break;

    }
    ++line;
  }
}

void ChildFrame::RedrawHistoryView() {
  history_view_->DeleteAllItems();

  if (splitter_->GetWindow2() != history_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto* mdf_file = doc->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  const auto* header = mdf_file->Header();
  if (header == nullptr) {
    return;
  }

  const auto history_list = header->FileHistories();


  long line = 0;
  for (const auto* history : history_list) {
    if (history == nullptr) {
      continue;
    }
    std::ostringstream date_time;
    date_time << util::time::NsToLocalDate(history->Time()) << " "
              << util::time::NsToLocalTime(history->Time(), 0);
    const auto index = history_view_->InsertItem(line, wxString::FromUTF8(date_time.str()));
    history_view_->SetItem(index, 1, wxString::FromUTF8(history->UserName()));
    history_view_->SetItem(index, 2, wxString::FromUTF8(history->Description()));
    history_view_->SetItem(index, 3, wxString::FromUTF8(history->ToolName()));
    history_view_->SetItem(index, 4, wxString::FromUTF8(history->ToolVendor()));
    history_view_->SetItem(index, 5, wxString::FromUTF8(history->ToolVersion()));
    ++line;
  }
}

void ChildFrame::RedrawMeasurementView() {
  measurement_view_->DeleteAllItems();

  if (splitter_->GetWindow2() != measurement_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto* mdf_file = doc->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  const auto* header = mdf_file->Header();
  if (header == nullptr) {
    return;
  }

  const auto measurement_list = header->DataGroups();

  long line = 0;
  for (const auto* meas : measurement_list) {
    if (meas == nullptr) {
      continue;
    }
    const auto group_list = meas->ChannelGroups();
    const auto* group = group_list.size() == 1 ? group_list.front() : nullptr;
      // Insert fictive measurement number
    const auto index = measurement_view_->InsertItem(line, std::to_string(line + 1));
    uint64_t samples = 0;
    std::ostringstream groups;
    for ( const auto* group1 : group_list) {
      if (!groups.str().empty()) {
        groups <<",";
      }
      groups << group1->Name();
      samples += group1->NofSamples();
    }
      // Measurement description
    std::ostringstream desc;
    if (!meas->Description().empty()) {
      desc << meas->Description();
      if (group != nullptr) {
        desc << ", Samples: " << group->NofSamples();
      }
    } else if (group != nullptr) {
      desc << group->Name();
    } else {
      desc << "Measurement " << line + 1;
    }
    measurement_view_->SetItem(index, 1, std::to_string(samples));
    measurement_view_->SetItem(index, 2, wxString::FromUTF8(desc.str()));
    measurement_view_->SetItem(index, 3, wxString::FromUTF8(groups.str()));
    ++line;
  }
}

void ChildFrame::RedrawEventView() {
  event_view_->DeleteAllItems();

  if (splitter_->GetWindow2() != event_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto* mdf_file = doc->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  const auto* header = mdf_file->Header();
  if (header == nullptr) {
    return;
  }

  const auto event_list = header->Events();


  long line = 0;
  for (const auto* event : event_list) {
    if (event == nullptr) {
      continue;
    }
    const auto index = event_view_->InsertItem(line, wxString::FromUTF8(event->Name()));
    event_view_->SetItem(index, 1, event->TypeToString());
    event_view_->SetItem(index, 2, event->ValueToString());
    event_view_->SetItem(index, 3, wxString::FromUTF8(event->Description()));
    event_view_->SetItem(index, 4, event->RangeToString());
    event_view_->SetItem(index, 5, event->CauseToString());
    ++line;
  }
}

void ChildFrame::RedrawAttachmentView() {
  attachment_view_->DeleteAllItems();

  if (splitter_->GetWindow2() != attachment_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto* mdf_file = doc->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  const auto* header = mdf_file->Header();
  if (header == nullptr) {
    return;
  }

  const auto attachment_list = header->Attachments();


  long line = 0;
  for (const auto* attachment : attachment_list) {
    if (attachment == nullptr) {
      continue;
    }
    std::u8string name;
    std::u8string path;
    try {
      std::filesystem::path fullname = std::filesystem::path(reinterpret_cast<const char8_t*>(attachment->FileName().c_str()));
      name = fullname.filename().u8string();
      path = fullname.parent_path().u8string();
    } catch (const std::exception& ) {

    }
    const auto index = attachment_view_->InsertItem(line, wxString::FromUTF8((const char*)name.c_str()));
    attachment_view_->SetItem(index, 1, attachment->IsEmbedded() ? "Yes" : "");
    attachment_view_->SetItem(index, 2, wxString::FromUTF8(attachment->FileType()));
    attachment_view_->SetItem(index, 3, wxString::FromUTF8((const char*)path.c_str()));
    ++line;
  }
}

void ChildFrame::RedrawHierarchyView() {
  hierarchy_view_->DeleteAllItems();

  if (splitter_->GetWindow2() != hierarchy_view_) {
    return;
  }

  auto* doc = GetDoc();
  if (doc == nullptr) {
    return;
  }
  const auto* mdf_file = doc->GetFile();
  if (mdf_file == nullptr) {
    return;
  }

  const auto* header = mdf_file->Header();
  if (header == nullptr) {
    return;
  }

  const auto hierarchy_list = header->ChannelHierarchies();
  long line = 0;
  for (const auto* hierarchy : hierarchy_list) {
    if (hierarchy == nullptr) {
      continue;
    }
    const auto index = hierarchy_view_->InsertItem(line, wxString::FromUTF8(hierarchy->Name()));
    hierarchy_view_->SetItem(index, 1, hierarchy->TypeToString());
    ++line;
  }
}

MdfDocument *ChildFrame::GetDoc() {
  return wxDynamicCast(GetDocument(),MdfDocument ); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
}

}



