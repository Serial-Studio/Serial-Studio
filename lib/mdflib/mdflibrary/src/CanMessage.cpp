/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "CanMessage.h"

namespace MdfLibrary {
CanMessage::CanMessage() : msg_( new mdf::CanMessage() ) {}
CanMessage::~CanMessage() { this->!CanMessage(); }
CanMessage::!CanMessage() {
  if (msg_ != nullptr) {
    delete msg_;
    msg_ = nullptr;
  }
}

void CanMessage::TypeOfMessage::set(CanMessageType type) {
  if (msg_ != nullptr) {
    msg_->TypeOfMessage(static_cast<mdf::MessageType>(type));
  }
}

CanMessageType CanMessage::TypeOfMessage::get() {
  return msg_ != nullptr ?
    static_cast<CanMessageType>(msg_->TypeOfMessage())
  : CanMessageType::Can_DataFrame;
}

void CanMessage::MessageId::set(uint32_t msgId) {
  if (msg_ != nullptr) {
    msg_->MessageId(msgId);
  }
}

uint32_t CanMessage::MessageId::get() {
  return msg_ != nullptr ? msg_->MessageId() : 0;
}

uint32_t CanMessage::CanId::get() {
  return msg_ != nullptr ? msg_->CanId() : 0;
}

void CanMessage::ExtendedId::set(bool extendedId) {
  if (msg_ != nullptr) {
    msg_->ExtendedId(extendedId);
  }
}

bool CanMessage::ExtendedId::get() {
  return msg_ != nullptr ? msg_->ExtendedId() : false;
}

void CanMessage::Dlc::set(uint8_t dlc) {
  if (msg_ != nullptr) {
    msg_->Dlc(dlc);
  }
}

uint8_t CanMessage::Dlc::get() {
  return msg_ != nullptr ? msg_->Dlc() : 0;
}

void CanMessage::DataLength::set(uint32_t dataLength) {
  if (msg_ != nullptr) {
    msg_->DataLength(dataLength);
  }
}

uint32_t CanMessage::DataLength::get() {
  return msg_ != nullptr ? static_cast<uint32_t>(msg_->DataLength()) : 0;
}

void CanMessage::DataBytes::set(array<uint8_t>^ dataList) {
  if (dataList == nullptr && msg_ != nullptr) {
    msg_->DataBytes({});
  } else if (msg_ != nullptr) {
    std::vector<uint8_t> tempList(dataList->Length);
    for (int i = 0; i < dataList->Length; ++i) {
      tempList[i] = dataList[i];
    }
    msg_->DataBytes(tempList);
  }
}

array<uint8_t>^ CanMessage::DataBytes::get() {
  if (msg_ != nullptr) {
    const auto& tempList = msg_->DataBytes();
    auto dataList = gcnew array<uint8_t>(static_cast<int>(tempList.size()));
    for (int i = 0; i < static_cast<int>(tempList.size()); ++i) {
      dataList[i] = tempList[i];
    }
    return dataList;
  } else {
    return gcnew array<uint8_t>(0);
  }
}

void CanMessage::Dir::set(bool transmit) {
  if (msg_ != nullptr) {
    msg_->Dir(transmit);
  }
}

bool CanMessage::Dir::get() {
  return msg_ != nullptr ? msg_->Dir() : false;
}

void CanMessage::Srr::set(bool srr) {
  if (msg_ != nullptr) {
    msg_->Srr(srr);
  }
}

bool CanMessage::Srr::get() {
  return msg_ != nullptr ? msg_->Srr() : false;
}

void CanMessage::Edl::set(bool edl) {
  if (msg_ != nullptr) {
    msg_->Edl(edl);
  }
}

bool CanMessage::Edl::get() {
  return msg_ != nullptr ? msg_->Edl() : false;
}

void CanMessage::Brs::set(bool brs) {
  if (msg_ != nullptr) {
    msg_->Brs(brs);
  }
}

bool CanMessage::Brs::get() {
  return msg_ != nullptr ? msg_->Brs() : false;
}

void CanMessage::Esi::set(bool esi) {
  if (msg_ != nullptr) {
    msg_->Esi(esi);
  }
}

bool CanMessage::Esi::get() {
  return msg_ != nullptr ? msg_->Esi() : false;
}

void CanMessage::Rtr::set(bool rtr) {
  if (msg_ != nullptr) {
    msg_->Rtr(rtr);
  }
}

bool CanMessage::Rtr::get() {
  return msg_ != nullptr ? msg_->Rtr() : false;
}

void CanMessage::WakeUp::set(bool wakeUp) {
  if (msg_ != nullptr) {
    msg_->WakeUp(wakeUp);
  }
}

bool CanMessage::WakeUp::get() {
  return msg_ != nullptr ? msg_->WakeUp() : false;
}

void CanMessage::SingleWire::set(bool singleWire) {
  if (msg_ != nullptr) {
    msg_->SingleWire(singleWire);
  }
}

bool CanMessage::SingleWire::get() {
  return msg_ != nullptr ? msg_->SingleWire() : false;
}

void CanMessage::R0::set(bool r0Bit) {
  if (msg_ != nullptr) {
    msg_->R0(r0Bit);
  }
}

bool CanMessage::R0::get() {
  return msg_ != nullptr ? msg_->R0() : false;
}

void CanMessage::R1::set(bool r1Bit) {
  if (msg_ != nullptr) {
    msg_->R1(r1Bit);
  }
}

bool CanMessage::R1::get() {
  return msg_ != nullptr ? msg_->R1() : false;
}

void CanMessage::BusChannel::set(uint8_t channel) {
  if (msg_ != nullptr) {
    msg_->BusChannel(channel);
  }
}

uint8_t CanMessage::BusChannel::get() {
  return msg_ != nullptr ? msg_->BusChannel() : 0;
}

void CanMessage::BitPosition::set(uint16_t position) {
  if (msg_ != nullptr) {
    msg_->BitPosition(position);
  }
}

uint16_t CanMessage::BitPosition::get() {
  return msg_ != nullptr ? msg_->BitPosition() : 0;
}

void CanMessage::ErrorType::set(CanErrorType type) {
  if (msg_ != nullptr) {
    msg_->ErrorType(static_cast<mdf::CanErrorType>(type));
  }
}

CanErrorType CanMessage::ErrorType::get() {
  return msg_ != nullptr ?
    static_cast<CanErrorType>(msg_->ErrorType()) : CanErrorType::UNKNOWN_ERROR;
}

void CanMessage::Reset() {
  if (msg_ != nullptr) {
    msg_->Reset();
  }
}

uint8_t CanMessage::DlcToLength(uint8_t dlc) {
  return static_cast<uint8_t>(mdf::CanMessage::DlcToLength(dlc));
}



}  // namespace MdfLibrary
