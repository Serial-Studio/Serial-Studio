/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "datalistblock.h"

#include "di4block.h"
#include "dl4block.h"
#include "dt4block.h"
#include "dv4block.h"
#include "dz4block.h"
#include "hl4block.h"
#include "ld4block.h"
#include "rd4block.h"
#include "ri4block.h"
#include "rv4block.h"
#include "sd4block.h"
#include "sr4block.h"

namespace {
constexpr size_t kIndexNext = 0;
}

namespace mdf::detail {

void DataListBlock::ReadBlockList(std::streambuf& buffer, size_t data_index) {
  if (block_list_.empty() && Link(data_index) > 0) {
    SetFilePosition(buffer, Link(data_index));
    std::string block_type = ReadBlockType(buffer);

    SetFilePosition(buffer, Link(data_index));
    if (block_type == "DT") {
      auto dt = std::make_unique<Dt4Block>();
      dt->Init(*this);
      dt->Read(buffer);
      block_list_.emplace_back(std::move(dt));
    } else if (block_type == "DZ") {
      auto dz = std::make_unique<Dz4Block>();
      dz->Init(*this);
      dz->Read(buffer);
      block_list_.emplace_back(std::move(dz));
    } else if (block_type == "DV") {
      auto dv_block = std::make_unique<Dv4Block>();
      dv_block->Init(*this);
      dv_block->Read(buffer);
      block_list_.emplace_back(std::move(dv_block));
    } else if (block_type == "DI") {
      auto di_block = std::make_unique<Di4Block>();
      di_block->Init(*this);
      di_block->Read(buffer);
      block_list_.emplace_back(std::move(di_block));
    } else if (block_type == "DL") {
      for (auto link = Link(data_index); link > 0; /* no increment here*/) {
        SetFilePosition(buffer, link);
        auto dl = std::make_unique<Dl4Block>();
        dl->Init(*this);
        dl->Read(buffer);
        link = dl->Link(kIndexNext);
        block_list_.emplace_back(std::move(dl));
      }
    } else if (block_type == "LD") {
      for (auto link = Link(data_index); link > 0; /* no increment here*/) {
        SetFilePosition(buffer, link);
        auto ld_block = std::make_unique<Ld4Block>();
        ld_block->Init(*this);
        ld_block->Read(buffer);
        link = ld_block->Link(kIndexNext);
        block_list_.emplace_back(std::move(ld_block));
      }
    } else if (block_type == "HL") {
      auto hl = std::make_unique<Hl4Block>();
      hl->Init(*this);
      hl->Read(buffer);
      block_list_.emplace_back(std::move(hl));
    } else if (block_type == "SR") {
      for (auto link = Link(data_index); link > 0; /* no increment here*/) {
        SetFilePosition(buffer, link);
        auto sr = std::make_unique<Sr4Block>();
        sr->Init(*this);
        sr->Read(buffer);
        link = sr->Link(kIndexNext);
        block_list_.emplace_back(std::move(sr));
      }
    } else if (block_type == "RD") {
      auto rd = std::make_unique<Rd4Block>();
      rd->Init(*this);
      rd->Read(buffer);
      block_list_.emplace_back(std::move(rd));
    } else if (block_type == "RV") {
      auto rv_block = std::make_unique<Rv4Block>();
      rv_block->Init(*this);
      rv_block->Read(buffer);
      block_list_.emplace_back(std::move(rv_block));
    } else if (block_type == "RI") {
      auto ri_block = std::make_unique<Ri4Block>();
      ri_block->Init(*this);
      ri_block->Read(buffer);
      block_list_.emplace_back(std::move(ri_block));
    } else if (block_type == "SD") {
      auto sd = std::make_unique<Sd4Block>();
      sd->Init(*this);
      sd->Read(buffer);
      block_list_.emplace_back(std::move(sd));
    }
  }
}

void DataListBlock::ReadLinkList(std::streambuf& buffer, size_t data_index,
                                 uint32_t nof_link) {
  if (block_list_.empty()) {
    for (uint32_t ii = 0; ii < nof_link; ++ii) {
      auto link = Link(data_index + ii);
      if (link <= 0) {
        continue;
      }
      SetFilePosition(buffer, link);
      std::string block_type = ReadBlockType(buffer);

      SetFilePosition(buffer, link);
      // Note that we only read in data block that this list own. If it points
      // to a CG or CN block, this means that someone else is reading in this
      // block.
      if (block_type == "DT") {
        auto dt = std::make_unique<Dt4Block>();
        dt->Init(*this);
        dt->Read(buffer);
        block_list_.emplace_back(std::move(dt));
      } else if (block_type == "DZ") {
        auto dz = std::make_unique<Dz4Block>();
        dz->Init(*this);
        dz->Read(buffer);
        block_list_.emplace_back(std::move(dz));
      } else if (block_type == "DV") {
        auto dv_block = std::make_unique<Dv4Block>();
        dv_block->Init(*this);
        dv_block->Read(buffer);
        block_list_.emplace_back(std::move(dv_block));
      } else if (block_type == "DI") {
        auto di_block = std::make_unique<Di4Block>();
        di_block->Init(*this);
        di_block->Read(buffer);
        block_list_.emplace_back(std::move(di_block));
      } else if (block_type == "RD") {
        auto rd = std::make_unique<Rd4Block>();
        rd->Init(*this);
        rd->Read(buffer);
        block_list_.emplace_back(std::move(rd));
      } else if (block_type == "RV") {
        auto rv_block = std::make_unique<Rv4Block>();
        rv_block->Init(*this);
        rv_block->Read(buffer);
        block_list_.emplace_back(std::move(rv_block));
      } else if (block_type == "RI") {
        auto ri_block = std::make_unique<Ri4Block>();
        ri_block->Init(*this);
        ri_block->Read(buffer);
        block_list_.emplace_back(std::move(ri_block));
      } else if (block_type == "SD") {
        auto sd = std::make_unique<Sd4Block>();
        sd->Init(*this);
        sd->Read(buffer);
        block_list_.emplace_back(std::move(sd));
      }
    }
  }
}

void DataListBlock::WriteBlockList(std::streambuf& buffer, size_t first_index) {

  for (size_t index = 0; index < block_list_.size(); ++index) {
    auto* block = block_list_[index].get();
    if (block == nullptr) {
      continue;
    }
    if (block->FilePosition() > 0) {
      continue;
    }

    block->Write(buffer);
    UpdateLink(buffer, first_index + index, block->FilePosition());
  }
}

MdfBlock* DataListBlock::Find(int64_t index) const {
  for (auto& p : block_list_) {
    if (!p) {
      continue;
    }
    auto* pp = p->Find(index);
    if (pp != nullptr) {
      return pp;
    }
  }
  return MdfBlock::Find(index);
}

uint64_t DataListBlock::DataSize() const {  // NOLINT
  uint64_t count = 0;
  for (const auto& block : block_list_) {
    if (!block) {
      continue;
    }
    const auto* data_list = dynamic_cast<const DataListBlock*>(block.get());
    const auto* data_block = dynamic_cast<const DataBlock*>(block.get());
    if (data_list != nullptr) {
      count += data_list->DataSize();
    } else if (data_block != nullptr) {
      count += data_block->DataSize();
    }
  }
  return count;
}

void DataListBlock::ClearData() { // NOLINT
  for ( auto& block : block_list_) {
    if (auto* data_block = dynamic_cast<DataBlock*>(block.get());
        data_block != nullptr) {
      data_block->ClearData();
    }
    if (auto* data_list_block = dynamic_cast<DataListBlock*>(block.get());
        data_list_block != nullptr) {
      data_list_block->ClearData();
    }
  }
}

void DataListBlock::CopyDataToBuffer(std::streambuf& from_file,
                      std::vector<uint8_t> &buffer,
                      uint64_t &buffer_index) const {
  for (const auto &block : block_list_) {
    const auto *data_list = dynamic_cast<const DataListBlock *>(block.get());
    const auto *data_block = dynamic_cast<const DataBlock *>(block.get());
    if (data_list != nullptr) {
      data_list->CopyDataToBuffer(from_file, buffer, buffer_index);
    } else if (data_block != nullptr) {
      data_block->CopyDataToBuffer(from_file, buffer, buffer_index);
    }
  }
}

bool DataListBlock::IsRdBlock() const {
  for (const auto& block : block_list_) {
    if (!block) {
      continue;
    }
    if (const auto* list = dynamic_cast<const DataListBlock*>(block.get())) {
      if (list->IsRdBlock()) {
        return true;
      }
    }
    if (block->BlockType() == "DZ") {
      const auto* dz4 = dynamic_cast<const Dz4Block*>(block.get());
      if (dz4->OrigBlockType() == "RD") {
        return true;
      }
    }
    if (block->BlockType() == "RD") {
      return true;
    }
  }
  return false;
}

void DataListBlock::GetDataBlockList(std::vector<DataBlock*>& block_list) const {
  for (const auto& block : block_list_) {
    if (!block) {
      continue;
    }

    auto* data_list = dynamic_cast<DataListBlock*>(block.get());
    auto* data_block = dynamic_cast<DataBlock*>(block.get());
    if (data_list != nullptr) {
      data_list->GetDataBlockList(block_list);
    } else if (data_block != nullptr) {
      block_list.push_back(data_block);
    }
  }
}

}  // namespace mdf::detail