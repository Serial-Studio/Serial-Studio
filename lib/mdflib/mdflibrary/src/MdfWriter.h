/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mdf/mdffactory.h>
#include <mdf/mdfwriter.h>

#include "MdfDataGroup.h"
#include "MdfFile.h"
#include "MdfHeader.h"
#include "CanMessage.h"

using namespace System;

namespace MdfLibrary {

/** \brief Enumerate that defines type of bus. Only relevant for bus logging.
 *
 * Enumerate that is used when doing bus logging. The enumerate is used when
 * creating default channel and channel group names.
 */
public enum class MdfBusType : uint16_t {
  CAN = 0x0001,      ///< CAN or CAN-FD bus
  LIN = 0x0002,      ///< LIN bus
  FlexRay = 0x0004,  ///< FlexRay bus
  MOST = 0x0008,     ///< MOST bus
  Ethernet = 0x0010, ///< Ethernet bus
  UNKNOWN = 0x0000   ///< Unknown bus type (Default)
};

/** \brief MDF writer types. */
public enum class MdfWriterType : int {
  Mdf3Basic = 0,  ///< Basic MDF version 3 writer.
  Mdf4Basic = 1,   ///< Basic MDF version 4 writer.
  MdfBusLogger = 2, ///< Bus Logger MDF 4 version only
  MdfConverter = 3, ///< MDF writer for MDF 4 conversion applications.
};

/** \brief Enumerate that defines how the raw data is stored. By default
 * the fixed length record is stored. Only used when doing bus logging.
 *
 * The fixed length storage is using one SD-block per byte array. The SD block
 * is temporary stored in primary memory instead of store it on disc. This
 * storage type is not recommended for bus logging.
 *
 * The variable length storage uses an extra CG-record for byte array data.
 * The storage type is used for bus logging where payload data is more than 8
 * byte.
 *
 * The maximum length storage shall be used when payload data is 8 bytes or
 * less. It is typically used when logging CAN messages which have 0-8 data
 * payload.
 */
public enum class MdfStorageType : int {
  FixedLengthStorage, ///< The default is fixed length records.
  VlsdStorage,        ///< Using variable length storage.
  MlsdStorage,        ///< Using maximum length storage
};


/** \brief Interface against an MDF writer object.
 *
 * The MDF writer purpose is to create MDF files. It simplifies the writing
 * into some steps.
 *
 * The first is to create type of writer. This is done by using the
 * MdfFactory::CreateMdfWriter() function.
 *
 * The next step is to call the Init() function with a filename with valid path.
 * The Init function will create the actual MDF file object and check if it
 * exist or not. If it exist, it reads in the existing file content so it can
 * be appended.
 *
 * The next step is to prepare the file for a new measurement. This is done
 * by creating the DG/CG/CN/CC blocks that defines the measurement. Note that
 * it is the last DG block that is the target.
 *
 * Next step is to call the InitMeasurement() function. This create a thread
 * that handle the queue of samples. The function also write the configuration
 * to the file and closes it.
 *
 * The user shall know starts adding samples to the queue by first setting
 * the current channel value to each channel and then call the SaveSample()
 * function for each channel group (CG). Note that no samples are saved to the
 * file yet. The max queue size is set to the pre-trig time, see PreTrigTime().
 *
 * At some point the user shall call the StartMeasurement() function. The
 * sample queue is now saved onto the file. The save rate is actually dependent
 * if CompressData() is enabled or not. If compression is used the data is
 * saved at the 4MB size or a 10 minute max. If not using compression, the
 * samples are saved each 10 second.
 *
 * The user shall now call StopMeasurement() which flush out the remaining
 * sample queue. After stopping the queue, the user may add some extra block
 * as event (EV) and attachment (AT) blocks.
 *
 * The FinalizeMeasurement() function. Stops the thread and write all
 * unwritten blocks to the file.
 *
 */
public ref class MdfWriter {
 public:
  MdfWriter(MdfWriterType writer_type);
  virtual ~MdfWriter();

  property String^ Name { String ^ get(); };
  property MdfWriterType TypeOfWriter {
    MdfWriterType get() {
      return writer_ != nullptr ?
                 static_cast<MdfWriterType>(writer_->TypeOfWriter())
                 : MdfWriterType::Mdf4Basic;
    }
  }

  property MdfFile^ File { MdfFile^ get(); };
  property MdfHeader^ Header { MdfHeader^ get(); };
  property bool IsFileNew { bool get(); };
  property bool CompressData {void set(bool compress); bool get(); }

  property bool MandatoryMemberOnly {
    void set(bool mandatory) {
      if (writer_ != nullptr) {
        writer_->MandatoryMembersOnly(mandatory);
      }
    }
    bool get() {
      return writer_ != nullptr ? writer_->MandatoryMembersOnly() : false;
    }
  }

  property bool SavePeriodic {
    void set(bool periodic) {
      if (writer_ != nullptr) {
        writer_->SavePeriodic(periodic);
      }
    }
    bool get() {
      return writer_ != nullptr ? writer_->IsSavePeriodic() : true;
    }
  }

  property double PreTrigTime {	void set(double preTrigTime);double get(); }
  property uint64_t StartTime {	uint64_t get(); }
  property uint64_t StopTime { uint64_t get(); }
  property MdfBusType BusType {	void set(MdfBusType type);MdfBusType get(); }
  property MdfStorageType StorageType {
    void set(MdfStorageType type);
    MdfStorageType get();
  }
  property uint32_t MaxLength {	void set(uint32_t length);uint32_t get(); }
  
  bool Init(String^ path_name);

  MdfDataGroup^ CreateDataGroup();
  bool CreateBusLogConfiguration();
  
  bool InitMeasurement();

  void SaveSample(MdfChannelGroup^ group, uint64_t time);
  void SaveSample(MdfDataGroup^ data_group, 
                  MdfChannelGroup^ channel_group, uint64_t time);

  void SaveCanMessage(MdfChannelGroup^ group, uint64_t time,
    const CanMessage^ message);
  void SaveCanMessage(MdfDataGroup^ data_group, 
                      MdfChannelGroup^ channel_group, 
                      uint64_t time,
                      const CanMessage^ message);
  void StartMeasurement(uint64_t start_time);
  void StartMeasurement(DateTime start_time);
  void StartMeasurement(IMdfTimeStamp^ start_time);
  void StopMeasurement(uint64_t stop_time);
  void StopMeasurement(DateTime stop_time);
  bool FinalizeMeasurement();

 protected:
  !MdfWriter();
  internal :
    mdf::MdfWriter* writer_ = nullptr;
};
}  // namespace MdfLibrary
