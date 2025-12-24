/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/canmessage.h>

namespace MdfLibrary {

public enum class CanErrorType : uint8_t {
  UNKNOWN_ERROR = 0, ///< Unspecified error.
  BIT_ERROR = 1,     ///< CAN bit error.
  FORM_ERROR = 2,    ///< CAN format error.
  BIT_STUFFING_ERROR = 3, ///< Bit stuffing error.
  CRC_ERROR = 4, ///< Checksum error.
  ACK_ERROR = 5 ///< Acknowledgement error.
};

public enum class CanMessageType : int {
  Can_DataFrame = 0, ///< Normal CAN message
  Can_RemoteFrame, ///< Remote frame message.
  Can_ErrorFrame, ///< Error message.
  Can_OverloadFrame, ///< Overload frame message.
};

public ref class CanMessage {
public:
  CanMessage();
  virtual ~CanMessage();
  property CanMessageType TypeOfMessage {void set(CanMessageType type); CanMessageType get(); }

  property uint32_t MessageId { void set(uint32_t msgId); uint32_t get();}
  property uint32_t CanId { uint32_t get();}
  property bool ExtendedId { void set(bool extendedId); bool get();}
  property uint8_t Dlc { void set(uint8_t dlc); uint8_t get();}
  property uint32_t DataLength { void set(uint32_t dataLength); uint32_t get();}
  property array<uint8_t>^ DataBytes {
    void set(array<uint8_t>^ dataList);
    array<uint8_t>^ get();
  }

  property bool Dir { void set(bool transmit); bool get();}
  property bool Srr { void set(bool srr); bool get();}
  property bool Edl { void set(bool edl); bool get();}
  property bool Brs { void set(bool brs); bool get();}
  property bool Esi { void set(bool esi); bool get();}
  property bool Rtr { void set(bool rtr); bool get();}
  property bool WakeUp { void set(bool wakeUp); bool get();}
  property bool SingleWire { void set(bool singleWire); bool get();}
  property bool R0 { void set(bool r0Bit); bool get();}
  property bool R1 { void set(bool r1Bit); bool get();}

  property uint8_t BusChannel { void set(uint8_t channel); uint8_t get();}
  property uint16_t BitPosition { void set(uint16_t position); uint16_t get();}
  property CanErrorType ErrorType {
    void set(CanErrorType type);
    CanErrorType get();
  }

  property uint32_t FrameDuration {
    void set(uint32_t duration) {
      if (msg_ != nullptr ) { msg_->FrameDuration(duration);}
    }
    uint32_t get() {return msg_ != nullptr ? msg_->FrameDuration() : 0; }
  }

  property uint32_t Crc {
    void set(uint32_t crc) {
      if (msg_ != nullptr ) { msg_->Crc(crc);}
    }
    uint32_t get() {return msg_ != nullptr ? msg_->Crc() : 0; }
  }

  property double Timestamp {
    void set(double timestamp) {
      if (msg_ != nullptr ) { msg_->Timestamp(timestamp);}
    }
    double get() {return msg_ != nullptr ? msg_->Timestamp() : 0.0; }
  }

  void Reset();
  static uint8_t DlcToLength(uint8_t dlc);
protected:
  !CanMessage();
internal:
  mdf::CanMessage* msg_ = nullptr;
  
};

} 
