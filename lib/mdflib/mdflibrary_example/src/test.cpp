/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#include <iostream>
#include <sstream>

// Pure C example
#pragma region C
#include <mdflibrary/MdfExport.h>
using namespace MdfLibrary;
using namespace MdfLibrary::ExportFunctions;
void c_example() {
  std::cout << "C example" << std::endl;

  {
    std::cout << "Write" << std::endl;
    auto* Writer = MdfWriterInit(MdfWriterType::Mdf4Basic, "test_c.mf4");
    std::cout << "Writer: " << Writer << std::endl;
    auto* Header = MdfWriterGetHeader(Writer);
    std::cout << "Header: " << Header << std::endl;
    MdfHeaderSetAuthor(Header, "Caller");
    MdfHeaderSetDepartment(Header, "Home Alone");
    MdfHeaderSetDescription(Header, "Testing i");
    MdfHeaderSetProject(Header, "Mdf3WriteHD");
    MdfHeaderSetStartTime(Header, 1000);
    MdfHeaderSetSubject(Header, "PXY");
    auto* History = MdfHeaderCreateFileHistory(Header);
    MdfFileHistorySetTime(History, 1000000);
    MdfFileHistorySetDescription(History, "Initial stuff");
    MdfFileHistorySetToolName(History, "Unit Test");
    MdfFileHistorySetToolVendor(History, "ACME");
    MdfFileHistorySetToolVersion(History, "2.3");
    MdfFileHistorySetUserName(History, "Ducky");

    {  // DataGroup 1
      auto* dg = MdfWriterCreateDataGroup(Writer);
      std::cout << "DataGroup: " << dg << std::endl;
      auto* cg1 = MdfDataGroupCreateChannelGroup(dg);
      std::cout << "ChannelGroup1: " << cg1 << std::endl;
      MdfChannelGroupSetName(cg1, "Test 1");
      MdfChannelGroupSetDescription(cg1, "Test channel group");
      auto* cg2 = MdfDataGroupCreateChannelGroup(dg);
      std::cout << "ChannelGroup2: " << cg2 << std::endl;
      MdfChannelGroupSetName(cg2, "Test 2");
      MdfChannelGroupSetDescription(cg2, "Test channel group");

      auto* si = MdfChannelGroupCreateSourceInformation(cg1);
      std::cout << "SourceInformation: " << si << std::endl;
      MdfSourceInformationSetName(si, "SI-Name");
      MdfSourceInformationSetPath(si, "SI-Path");
      MdfSourceInformationSetDescription(si, "SI-Desc");
      MdfSourceInformationSetType(si, SourceType::Bus);
      MdfSourceInformationSetBus(si, BusType::Can);

      mdf::IChannel* pChannel1[3];
      {
        pChannel1[0] = MdfChannelGroupCreateChannel(cg1);
        std::cout << "Channel: " << pChannel1[0] << std::endl;
        MdfChannelSetName(pChannel1[0], "Time");
        MdfChannelSetDescription(pChannel1[0], "Time channel");
        MdfChannelSetType(pChannel1[0], ChannelType::Master);
        MdfChannelSetSync(pChannel1[0], ChannelSyncType::Time);
        MdfChannelSetDataType(pChannel1[0], ChannelDataType::FloatLe);
        MdfChannelSetDataBytes(pChannel1[0], 4);
        MdfChannelSetUnit(pChannel1[0], "s");
        MdfChannelSetRange(pChannel1[0], 0, 100);
      }

      {
        pChannel1[1] = MdfChannelGroupCreateChannel(cg1);
        MdfChannelSetName(pChannel1[1], "SignedLe");
        MdfChannelSetDescription(pChannel1[1], "int32_t");
        MdfChannelSetType(pChannel1[1], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel1[1], ChannelDataType::SignedIntegerLe);
        MdfChannelSetDataBytes(pChannel1[1], sizeof(int32_t));
      }
      {
        pChannel1[2] = MdfChannelGroupCreateChannel(cg1);
        MdfChannelSetName(pChannel1[2], "SignedBe");
        MdfChannelSetDescription(pChannel1[2], "int8_t");
        MdfChannelSetType(pChannel1[2], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel1[2], ChannelDataType::SignedIntegerLe);
        MdfChannelSetDataBytes(pChannel1[2], sizeof(int8_t));
      }

      mdf::IChannel* pChannel2[3];
      {
        pChannel2[0] = MdfChannelGroupCreateChannel(cg2);
        std::cout << "Channel: " << pChannel2[0] << std::endl;
        MdfChannelSetName(pChannel2[0], "Time");
        MdfChannelSetDescription(pChannel2[0], "Time channel");
        MdfChannelSetType(pChannel2[0], ChannelType::Master);
        MdfChannelSetSync(pChannel2[0], ChannelSyncType::Time);
        MdfChannelSetDataType(pChannel2[0], ChannelDataType::FloatLe);
        MdfChannelSetDataBytes(pChannel2[0], 4);
        MdfChannelSetUnit(pChannel2[0], "s");
        MdfChannelSetRange(pChannel2[0], 0, 100);
      }

      {
        pChannel2[1] = MdfChannelGroupCreateChannel(cg2);
        MdfChannelSetName(pChannel2[1], "SignedLe");
        MdfChannelSetDescription(pChannel2[1], "int32_t");
        MdfChannelSetType(pChannel2[1], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel2[1], ChannelDataType::SignedIntegerLe);
        MdfChannelSetDataBytes(pChannel2[1], sizeof(int32_t));
      }
      {
        pChannel2[2] = MdfChannelGroupCreateChannel(cg2);
        MdfChannelSetName(pChannel2[2], "FloatBe");
        MdfChannelSetDescription(pChannel2[2], "double");
        MdfChannelSetType(pChannel2[2], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel2[2], ChannelDataType::FloatBe);
        MdfChannelSetDataBytes(pChannel2[2], sizeof(double));
      }

      MdfWriterInitMeasurement(Writer);
      MdfWriterStartMeasurement(Writer, 100000000);
      std::cout << "Start measure" << std::endl;
      for (size_t i = 0; i < 50; i++) {
        MdfChannelSetChannelValueAsFloat(pChannel1[1], i * 2);
        MdfChannelSetChannelValueAsFloat(pChannel1[2], i * 3);
        MdfChannelSetChannelValueAsFloat(pChannel2[1], i * 4);
        MdfChannelSetChannelValueAsFloat(pChannel2[2], i * 5);
        MdfWriterSaveSample(Writer, cg1, 100000000 + i * 10000);
        MdfWriterSaveSample(Writer, cg2, 100000000 + i * 10000);
      }
      std::cout << "Stop measure" << std::endl;
      MdfWriterStopMeasurement(Writer, 1100000000);
      MdfWriterFinalizeMeasurement(Writer);
    }
    {  // DataGroup 2
      auto* dg = MdfHeaderCreateDataGroup(Header);
      auto* cg = MdfDataGroupCreateChannelGroup(dg);
      std::cout << "ChannelGroup: " << cg << std::endl;
      MdfChannelGroupSetName(cg, "Test");
      MdfChannelGroupSetDescription(cg, "Test channel group");

      mdf::IChannel* pChannel[3];
      {
        pChannel[0] = MdfChannelGroupCreateChannel(cg);
        std::cout << "Channel: " << pChannel[0] << std::endl;
        MdfChannelSetName(pChannel[0], "Time");
        MdfChannelSetDescription(pChannel[0], "Time channel");
        MdfChannelSetType(pChannel[0], ChannelType::Master);
        MdfChannelSetSync(pChannel[0], ChannelSyncType::Time);
        MdfChannelSetDataType(pChannel[0], ChannelDataType::FloatLe);
        MdfChannelSetDataBytes(pChannel[0], 4);
        MdfChannelSetUnit(pChannel[0], "s");
        MdfChannelSetRange(pChannel[0], 0, 100);
      }

      {
        pChannel[1] = MdfChannelGroupCreateChannel(cg);
        MdfChannelSetName(pChannel[1], "SignedLe");
        MdfChannelSetDescription(pChannel[1], "int32_t");
        MdfChannelSetType(pChannel[1], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel[1], ChannelDataType::SignedIntegerLe);
        MdfChannelSetDataBytes(pChannel[1], sizeof(int32_t));
      }

      {
        pChannel[2] = MdfChannelGroupCreateChannel(cg);
        MdfChannelSetName(pChannel[2], "String");
        MdfChannelSetDescription(pChannel[2], "string");
        MdfChannelSetType(pChannel[2], ChannelType::FixedLength);
        MdfChannelSetDataType(pChannel[2], ChannelDataType::StringAscii);
        MdfChannelSetDataBytes(pChannel[2], 4);
      }

      MdfWriterInitMeasurement(Writer);
      MdfWriterStartMeasurement(Writer, 100000000);
      std::cout << "Start measure" << std::endl;
      for (size_t i = 0; i < 90; i++) {
        MdfChannelSetChannelValueAsFloat(pChannel[1], i * 2);
        MdfChannelSetChannelValueAsString(pChannel[2],
                                          std::to_string(i * 3).c_str());
        MdfWriterSaveSample(Writer, cg, 100000000 + i * 1000);
      }
      std::cout << "Stop measure" << std::endl;
      MdfWriterStopMeasurement(Writer, 1100000000);
      MdfWriterFinalizeMeasurement(Writer);
    }
  }
}
#pragma endregion C

// C++ example
#pragma region C++
#include <mdflibrary/MdfChannelObserver.h>
#include <mdflibrary/MdfReader.h>
#include <mdflibrary/MdfWriter.h>
void cpp_example() {
  std::cout << "C++ example" << std::endl;
  {

    std::cout << "Write Basic" << std::endl;
    MdfWriter Writer(MdfWriterType::Mdf4Basic, "test_cpp.mf4");
    MdfHeader Header = Writer.GetHeader();
    Header.SetAuthor("Caller");
    Header.SetDepartment("Home Alone");
    Header.SetDescription("Testing i");
    Header.SetProject("Mdf3WriteHD");
    Header.SetStartTime(1000);
    Header.SetSubject("PXY");
    MdfFileHistory History = Header.CreateFileHistory();
    History.SetTime(1000000);
    History.SetDescription("Initial stuff");
    History.SetToolName("Unit Test");
    History.SetToolVendor("ACME");
    History.SetToolVersion("2.3");
    History.SetUserName("Ducky");

    MdfDataGroup dg = Writer.CreateDataGroup();
    MdfChannelGroup cg = dg.CreateChannelGroup();
    cg.SetName("Test");
    cg.SetDescription("Test channel group");

    MdfSourceInformation si = cg.CreateSourceInformation();
    si.SetName("SI-Name");
    si.SetPath("SI-Path");
    si.SetDescription("SI-Desc");
    si.SetType(SourceType::Bus);
    si.SetBus(BusType::Can);

    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("Time");
      cn.SetDescription("Time channel");
      cn.SetType(ChannelType::Master);
      cn.SetSync(ChannelSyncType::Time);
      cn.SetDataType(ChannelDataType::FloatLe);
      cn.SetDataBytes(4);
      cn.SetUnit("s");
      cn.SetRange(0, 100);
    }

    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("SignedLe");
      cn.SetDescription("int32_t");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::SignedIntegerLe);
      cn.SetDataBytes(sizeof(int32_t));
    }
    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("SignedBe");
      cn.SetDescription("int8_t");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::SignedIntegerLe);
      cn.SetDataBytes(sizeof(int8_t));
    }
    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("FloatLe");
      cn.SetDescription("float");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::FloatLe);
      cn.SetDataBytes(sizeof(float));
    }
    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("FloatBe");
      cn.SetDescription("double");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::FloatBe);
      cn.SetDataBytes(sizeof(double));
    }
    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("String");
      cn.SetDescription("string");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::StringAscii);
      cn.SetDataBytes(4);
    }
    {
      MdfChannel cn = cg.CreateChannel();
      cn.SetName("ByteArray");
      cn.SetDescription("bytes");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::ByteArray);
      cn.SetDataBytes(4);
    }

    auto channels = cg.GetChannels();
    std::cout << "ChannelGroupGetChannels: " << channels.size() << std::endl;

    Writer.InitMeasurement();
    Writer.StartMeasurement(100000000);
    std::cout << "Start measure" << std::endl;
    for (size_t i = 0; i < 50; i++) {
      channels[1].SetChannelValue((uint64_t)i * 2);
      channels[2].SetChannelValue((uint64_t)i * 3);
      channels[3].SetChannelValue((uint64_t)i * 4);
      channels[4].SetChannelValue((uint64_t)i * 5);
      channels[5].SetChannelValue(std::to_string(i * 6).c_str());
      channels[6].SetChannelValue((uint8_t*)std::to_string(i * 6).c_str(), 4);
      Writer.SaveSample(cg, 100000000 + i * 1000);
      std::cout << "Save sample " << i << std::endl;
    }
    std::cout << "Stop measure" << std::endl;
    Writer.StopMeasurement(1100000000);
    Writer.FinalizeMeasurement();
  }

  {

    std::cout << "Write Can" << std::endl;
    MdfWriter Writer(MdfWriterType::MdfBusLogger, "test_can_cpp.mf4");
    MdfHeader Header = Writer.GetHeader();
    MdfFileHistory History = Header.CreateFileHistory();
    History.SetDescription("Test data types");
    History.SetToolName("MdfWrite");
    History.SetToolVendor("ACME Road Runner Company");
    History.SetToolVersion("1.0");
    History.SetUserName("Ingemar Hedvall");

    Writer.SetBusType(MdfBusType::CAN);
    Writer.SetStorageType(MdfStorageType::MlsdStorage);
    Writer.SetMaxLength(8);
    Writer.CreateBusLogConfiguration();
    Writer.SetPreTrigTime(0.0);
    Writer.SetCompressData(false);
    MdfDataGroup last_dg = Header.GetLastDataGroup();

    MdfChannelGroup can_data_frame = last_dg.GetChannelGroup("CAN_DataFrame");
    MdfChannelGroup can_remote_frame =
        last_dg.GetChannelGroup("CAN_RemoteFrame");
    MdfChannelGroup can_error_frame = last_dg.GetChannelGroup("CAN_ErrorFrame");
    MdfChannelGroup can_overload_frame =
        last_dg.GetChannelGroup("CAN_OverloadFrame");

    Writer.InitMeasurement();
    uint64_t tick_time = 100000000;
    Writer.StartMeasurement(tick_time);
    std::cout << "Start measure" << std::endl;
    for (size_t i = 0; i < 100'000; i++) {
      std::vector<uint8_t> data;
      data.assign(i < 8 ? i + 1 : 8, static_cast<uint8_t>(i + 1));

      CanMessage msg;
      msg.SetMessageId(123);
      msg.SetExtendedId(true);
      msg.SetBusChannel(11);
      msg.SetDataBytes(data);

      Writer.SaveCanMessage(can_data_frame, tick_time, msg);
      Writer.SaveCanMessage(can_remote_frame, tick_time, msg);
      Writer.SaveCanMessage(can_error_frame, tick_time, msg);
      Writer.SaveCanMessage(can_overload_frame, tick_time, msg);
      tick_time += 1'000'000;
    }
    std::cout << "Stop measure" << std::endl;
    Writer.StopMeasurement(1100000000);
    Writer.FinalizeMeasurement();
  }

  {
    std::cout << "Read" << std::endl;
    MdfReader Reader("test_cpp.mf4");
    Reader.ReadEverythingButData();
    auto Header = Reader.GetHeader();
    std::cout << "Author: " << Header.GetAuthor().c_str() << std::endl;
    std::cout << "Department: " << Header.GetDepartment() << std::endl;
    std::cout << "Description: " << Header.GetDescription() << std::endl;
    std::cout << "Project: " << Header.GetProject() << std::endl;
    std::cout << "StartTime: " << Header.GetStartTime() << std::endl;
    std::cout << "Subject: " << Header.GetSubject() << std::endl;

    auto Historys = Header.GetFileHistorys();
    std::cout << "History: " << Historys.size() << std::endl;
    for (const auto& Histroy : Historys) {
      std::cout << "Time: " << Histroy.GetTime() << std::endl;
      std::cout << "Description: " << Histroy.GetDescription() << std::endl;
      std::cout << "ToolName: " << Histroy.GetToolName() << std::endl;
      std::cout << "ToolVendor: " << Histroy.GetToolVendor() << std::endl;
      std::cout << "ToolVersion: " << Histroy.GetToolVersion() << std::endl;
      std::cout << "UserName: " << Histroy.GetUserName() << std::endl;
      std::cout << std::endl;
    }

    auto DataGroups = Header.GetDataGroups();
    std::cout << "DataGroups: " << DataGroups.size() << std::endl;
    for (const auto& DataGroup : DataGroups) {
      auto ChannelGroups = DataGroup.GetChannelGroups();
      std::cout << "ChannelGroups: " << ChannelGroups.size() << std::endl;
      for (const auto& ChannelGroup : ChannelGroups) {
        std::cout << "Name: " << ChannelGroup.GetName() << std::endl;
        std::cout << "Description: " << ChannelGroup.GetDescription()
                  << std::endl;

        auto SourceInformation = ChannelGroup.GetSourceInformation();
        std::cout << "SI Name: " << SourceInformation.GetName() << std::endl;
        std::cout << "SI Path: " << SourceInformation.GetPath() << std::endl;
        std::cout << "SI Description: " << SourceInformation.GetDescription()
                  << std::endl;

        std::cout << "Nof Samples: " << ChannelGroup.GetNofSamples()
                  << std::endl;

        auto Channels = ChannelGroup.GetChannels();
        std::cout << "Channels: " << Channels.size() << std::endl;
        std::vector<MdfLibrary::MdfChannelObserver> Observers;
        for (const auto& Channel : Channels) {
          std::cout << "Name: " << Channel.GetName() << std::endl;
          std::cout << "Description: " << Channel.GetDescription() << std::endl;
          std::cout << "Type: " << static_cast<int>(Channel.GetType())
                    << std::endl;
          std::cout << "Sync: " << static_cast<int>(Channel.GetSync())
                    << std::endl;
          std::cout << "DataType: " << static_cast<int>(Channel.GetDataType())
                    << std::endl;
          std::cout << "DataBytes: " << Channel.GetDataBytes() << std::endl;
          std::cout << "Unit: " << Channel.GetUnit() << std::endl;
          std::cout << std::endl;

          Observers.push_back(
              MdfChannelObserver(DataGroup, ChannelGroup, Channel));
        }

        Reader.ReadData(DataGroup);

        for (size_t i = 0; i < ChannelGroup.GetNofSamples(); i++) {
          std::cout << "Sample: " << i << std::endl;
          for (const auto& Observer : Observers) {
            switch (Observer.GetChannel().GetDataType()) {
              case ChannelDataType::CanOpenDate:
              case ChannelDataType::CanOpenTime: {
                uint64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::UnsignedIntegerLe:
              case ChannelDataType::UnsignedIntegerBe: {
                uint64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::SignedIntegerLe:
              case ChannelDataType::SignedIntegerBe: {
                int64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::FloatLe:
              case ChannelDataType::FloatBe: {
                double channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::StringAscii:
              case ChannelDataType::StringUTF8:
              case ChannelDataType::StringUTF16Le:
              case ChannelDataType::StringUTF16Be: {
                std::string channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::MimeStream:
              case ChannelDataType::MimeSample:
              case ChannelDataType::ByteArray: {
                std::vector<uint8_t> channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value.size()
                          << ", Eng: " << eng_value.size() << std::endl;
                break;
              }
              default:
                break;
            }
          }
          std::cout << std::endl;
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }
  }
}
#pragma endregion C++

int main() {
  c_example();
  cpp_example();
  return 0;
}
