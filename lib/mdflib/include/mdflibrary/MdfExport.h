/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdint.h>

namespace mdf {
class MdfReader;
class MdfWriter;
class MdfFile;
class IHeader;
class IDataGroup;
class IChannelGroup;
class IChannel;
class IChannelArray;
class IChannelConversion;
class IChannelObserver;
class ISourceInformation;
class IAttachment;
class IFileHistory;
class IEvent;
class ETag;
class IMetaData;
class CanMessage;
}  // namespace mdf

namespace MdfLibrary {
/** \brief MDF writer types. */
enum class MdfWriterType : int {
  Mdf3Basic = 0,     ///< Basic MDF version 3 writer.
  Mdf4Basic = 1,     ///< Basic MDF version 4 writer.
  MdfBusLogger = 2,  ///< Basic
};

/** \brief Enumerate that defines type of bus. Only relevant for bus logging.
 *
 * Enumerate that is used when doing bus logging. The enumerate is used when
 * creating default channel and channel group names.
 */
enum class MdfBusType : int {
  CAN = 0x01,       ///< CAN or CAN-FD bus
  LIN = 0x02,       ///< LIN bus
  FlexRay = 0x04,   ///< FlexRay bus
  MOST = 0x08,      ///< MOST bus
  Ethernet = 0x10,  ///< Ethernet bus
  UNKNOWN = 0x00    ///< Unknown bus type (Default)
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
enum class MdfStorageType : int {
  FixedLengthStorage,  ///< The default is to use fixed length records.
  VlsdStorage,         ///< Using variable length storage.
  MlsdStorage,         ///< Using maximum length storage
};

/** \brief Channel functional type.
 *
 * Most channels are marked as 'FixedLength' which means that its
 * size in a record is fixed. This works well with most data types but
 * byte arrays and strings may change its size. Instead are these data types
 * marked as 'Variable Length'. Avoid writing variable length data as it
 * may allocate a lot of memory as it flush at the end of the measurement.
 *
 * One channel in channel group (mdf::IChannelGroup), should be marked as a
 * master channel. This channel is typical relative sample time with seconds as
 * unit. The master channel is typical used on the X-axis when plotting data.
 *
 * The 'VirtualMaster' channel can be used if the sample number is linear
 * related to the sample time. The channel conversion (CC) block should
 * define the sample number to time conversion.
 *
 * The 'Sync' channel is used to synchronize an attachment block (file).
 *
 * The 'MaxLength' type is typical used when storing CAN byte array where
 * another channel stores actual bytes stored in a sample. For CAN the size
 * in the max record size is 8 bytes.
 *
 * The 'VirtualData' is similar to the 'VirtualMaster' channel but related to
 * data. Good luck to find a use of this type.
 */
enum class ChannelType : uint8_t {
  FixedLength = 0,     ///< Fixed length data (default type)
  VariableLength = 1,  ///< Variable length data
  Master = 2,          ///< Master channel
  VirtualMaster = 3,   ///< Virtual master channel
  Sync = 4,            ///< Synchronize channel
  MaxLength = 5,       ///< Max length channel
  VirtualData = 6      ///< Virtual data channel
};

/** \brief Synchronization type
 *
 * Defines the synchronization type. The type is 'None' for fixed length
 * channel but should be set for master and synchronization channels.
 */
enum class ChannelSyncType : uint8_t {
  None = 0,      ///< No synchronization (default value)
  Time = 1,      ///< Time type
  Angle = 2,     ///< Angle type
  Distance = 3,  ///< Distance type
  Index = 4      ///< Sample number
};

/** \brief Channel data type.
 *
 * Defines the channel data type. Avoid defining value sizes that doesn't align
 * to a byte size.
 *
 * The Le and Be extension is related to byte order. Little endian (Intel
 * byte order) while big endian (Motorola byte order).
 */
enum class ChannelDataType : uint8_t {
  UnsignedIntegerLe = 0,  ///< Unsigned integer, little endian.
  UnsignedIntegerBe = 1,  ///< Unsigned integer, big endian.
  SignedIntegerLe = 2,    ///< Signed integer, little endian.
  SignedIntegerBe = 3,    ///< Signed integer, big endian.
  FloatLe = 4,            ///< Float, little endian.
  FloatBe = 5,            ///< Float, big endian.
  StringAscii = 6,        ///< Text,  ISO-8859-1 coded
  StringUTF8 = 7,         ///< Text, UTF8 coded.
  StringUTF16Le = 8,      ///< Text, UTF16 coded little endian.
  StringUTF16Be = 9,      ///< Text, UTF16 coded big endian.
  ByteArray = 10,         ///< Byte array.
  MimeSample = 11,        ///< MIME sample byte array.
  MimeStream = 12,        ///< MIME stream byte array.
  CanOpenDate = 13,       ///< 7-byte CANOpen date.
  CanOpenTime = 14,       ///< 6-byte CANOpen time.
  ComplexLe = 15,         ///< Complex value, little endian.
  ComplexBe = 16          ///< Complex value, big endian.
};

/** \brief Channel flags. See also mdf::IChannel::Flags().
 *
 */
namespace CnFlag {
constexpr uint32_t AllValuesInvalid = 0x0001;    ///< All values are invalid.
constexpr uint32_t InvalidValid = 0x0002;        ///< Invalid bit is used.
constexpr uint32_t PrecisionValid = 0x0004;      ///< Precision is used.
constexpr uint32_t RangeValid = 0x0008;          ///< Range is used.
constexpr uint32_t LimitValid = 0x0010;          ///< Limit is used.
constexpr uint32_t ExtendedLimitValid = 0x0020;  ///< Extended limit is used.
constexpr uint32_t Discrete = 0x0040;            ///< Discrete channel.
constexpr uint32_t Calibration = 0x0080;         ///< Calibrated channel.
constexpr uint32_t Calculated = 0x0100;          ///< Calculated channel.
constexpr uint32_t Virtual = 0x0200;             ///< Virtual channel.
constexpr uint32_t BusEvent = 0x0400;            ///< Bus event channel.
constexpr uint32_t StrictlyMonotonous = 0x0800;  ///< Strict monotonously.
constexpr uint32_t DefaultX = 0x1000;            ///< Default x-axis channel.
constexpr uint32_t EventSignal = 0x2000;         ///< Event signal.
constexpr uint32_t VlsdDataStream = 0x4000;      ///< VLSD data stream channel.
}  // namespace CnFlag

/** \brief Type of array.
 *
 */
enum class ArrayType : uint8_t {
  Array = 0, ///< Simple array without attributes
  ScalingAxis = 1, ///< Scaling axis.
  LookUp = 2, ///< Lookup array.
  IntervalAxis = 3, ///< Interval axis.
  ClassificationResult = 4 ///< Classification result.
};

/** \brief Type of storage. */
enum class ArrayStorage : uint8_t {
  CnTemplate = 0, ///< Channel template.
  CgTemplate = 1, ///< Channel group template.
  DgTemplate = 2  ///< Data group template.
};

/** \brief Channel array (CA) block flags. */
namespace CaFlag {
constexpr uint32_t DynamicSize = 0x0001; ///< Dynamic size
constexpr uint32_t InputQuantity = 0x0002; ///< Input quantity.
constexpr uint32_t OutputQuantity = 0x0004; ///< Output quantity.
constexpr uint32_t ComparisonQuantity = 0x0008; ///< Comparison quantity.
constexpr uint32_t Axis = 0x0010; ///< Axis
constexpr uint32_t FixedAxis = 0x0020; ///< Fixed axis.
constexpr uint32_t InverseLayout = 0x0040; ///< Inverse layout.
constexpr uint32_t LeftOpenInterval = 0x0080; ///< Left-over interval.
constexpr uint32_t StandardAxis = 0x0100; ///< Standard axis.
}  // namespace CaFlag

/** \brief Type of conversion formula
 *
 * The type together with the Parameter() function defines
 * the conversion between channel and engineering value.
 *
 */
enum class ConversionType : uint8_t {
  /** \brief 1:1 conversion. No parameters needed. */
  NoConversion = 0,

  /** \brief Linear conversion. 2 parameters.
   * Eng = Ch * Par(1) + Par(0).
   */
  Linear = 1,

  /** \brief Rational function conversion. 6 parameters.
   *  Eng = (Par(0)*Ch^2 + Par(1)*Ch + Par(2)) /
   *  (Par(3)*Ch^2 + Par(4)*Ch + Par(5))
   */
  Rational = 2,
  Algebraic = 3,  ///< Text formula.

  /** \brief Value to value conversion with interpolation.
   * Defined by a list of Key value pairs.
   * Par(n) = key and Par(n+1) value.
   */
  ValueToValueInterpolation = 4,

  /** \brief Value to value conversion without interpolation.
   * Defined by a list of Key value pairs.
   * Par(n) = key and Par(n+1) value.
   */
  ValueToValue = 5,

  /** \brief Value range to value conversion without interpolation.
   * Defined by a list of Key min/max value triplets.
   * Par(3*n) = key min, Par(3*(n+1)) = key max and Par(3*(n+2)) value. Add a
   * default value last, after all the triplets.
   */
  ValueRangeToValue = 6,

  /** \brief Value to text conversion.
   * Defined by a list of key values to text string conversions. This is
   * normally used for enumerated channels.
   * Par(n) value to Ref(n) text. Add a default
   * referenced text last.
   */
  ValueToText = 7,

  /** \brief Value range to text conversion.
   * Defined by a list of key min/max values to text string conversions. This is
   * normally used for enumerated channels.
   * Par(2*n) min key, Par(2(n+1)) max key to Ref(n) value. Add a default
   * referenced text  last.
   */
  ValueRangeToText = 8,

  /** \brief Text to value conversion.
   * Defines a list of text string to value conversion.
   * Ref(n) key to Par(n) value. Add a default value last to the parameter list.
   */
  TextToValue = 9,

  /** \brief Text to text conversion.
   * Defines a list of text string to text conversion.
   * Ref(2*n) key to Ref(2*(n+1)) value.
   * Add a text value last to the parameter list.
   */
  TextToTranslation = 10,

  /** \brief Bitfield to text conversion
   * Currently unsupported conversion.
   */
  BitfieldToText = 11,
  // MDF 3 types
  Polynomial = 30,      ///< MDF 3 polynomial conversion.
  Exponential = 31,     ///< MDF 3 exponential conversion.
  Logarithmic = 32,     ///< MDF 3 logarithmic conversion.
  DateConversion = 33,  ///< MDF 3 Date conversion
  TimeConversion = 34   ///< MDF 3 Time conversion
};

/** \brief Channel conversion flags.
 *
 */
namespace CcFlag {
constexpr uint16_t PrecisionValid = 0x0001;  ///< Precision is used.
constexpr uint16_t RangeValid = 0x0002;      ///< Range is used.
constexpr uint16_t StatusString = 0x0004;    ///< Status string flag.
}  // namespace CcFlag

/** \brief Type of source information. */
enum class SourceType : uint8_t {
  Other = 0,     ///< Unknown source type.
  Ecu = 1,       ///< ECU.
  Bus = 2,       ///< Bus.
  IoDevice = 3,  ///< I/O device.
  Tool = 4,      ///< Tool.
  User = 5       ///< User.
};

/** \brief Type of bus. */
enum class BusType : uint8_t {
  None = 0,      ///< No bus (default).
  Other = 1,     ///< Unknown bus type.
  Can = 2,       ///< CAN bus.
  Lin = 3,       ///< LIN bus.
  Most = 4,      ///< MOST bus.
  FlexRay = 5,   ///< FlexRay bus.
  Kline = 6,     ///< KLINE bus.
  Ethernet = 7,  ///< EtherNet bus.
  Usb = 8        ///< USB bus.
};

/** \brief Source information flags. */
namespace SiFlag {
constexpr uint8_t Simulated = 0x01;  ///< Simulated device.
}

/** \brief Type of event. */
enum class EventType : uint8_t {
  RecordingPeriod = 0,       ///< Specifies a recording period (range).
  RecordingInterrupt = 1,    ///< The recording was interrupted.
  AcquisitionInterrupt = 2,  ///< The data acquisition was interrupted.
  StartRecording = 3,        ///< Start recording event.
  StopRecording = 4,         ///< Stop recording event.
  Trigger = 5,               ///< Generic event (no range).
  Marker = 6                 ///< Another generic event (maybe range).
};

/** \brief Type of synchronization value (default time) */
enum class SyncType : uint8_t {
  SyncTime = 1,      ///< Sync value represent time (s).
  SyncAngle = 2,     ///< Sync value represent angle (rad).
  SyncDistance = 3,  ///< Sync value represent distance (m).
  SyncIndex = 4,     ///< Sync value represent sample index.
};

/** \brief Type of range. */
enum class RangeType : uint8_t {
  RangePoint = 0,  ///< Defines a point
  RangeStart = 1,  ///< First in a range.
  RangeEnd = 2     ///< Last in a range.
};

/** \brief Type of cause. */
enum class EventCause : uint8_t {
  CauseOther = 0,   ///< Unknown source.
  CauseError = 1,   ///< An error generated this event.
  CauseTool = 2,    ///< The tool generated this event.
  CauseScript = 3,  ///< A script generated this event.
  CauseUser = 4,    ///< A user generated this event.
};

/** \brief The e-tag may optional have a data type below. The value in the
 * XML file is of course string but the data type may be used for
 * interpretation of the value. Note that unit property can also be added.
 *
 * Use ISO UTC date and time formats or avoid these data types if possible
 * as they just causing problem at presentation.
 */
enum class ETagDataType : uint8_t {
  StringType = 0,   ///< Text value.
  DecimalType = 1,  ///< Decimal value (use float instead)
  IntegerType = 2,  ///< Integer value
  FloatType = 3,    ///< Floating point value
  BooleanType = 4,  ///< Boolean tru/false value
  DateType = 5,     ///< Date value according to ISO (YYYY-MM-DD).
  TimeType = 6,     ///< Time value ISO
  DateTimeType = 7  ///< Date and Time ISO string (YYYY-MM-DD hh:mm:ss)
};

enum class CanErrorType : uint8_t {
  UNKNOWN_ERROR = 0,       ///< Unspecified error.
  BIT_ERROR = 1,           ///< CAN bit error.
  FORM_ERROR = 2,          ///< CAN format error.
  BIT_STUFFING_ERROR = 3,  ///< Bit stuffing error.
  CRC_ERROR = 4,           ///< Checksum error.
  ACK_ERROR = 5            ///< Acknowledgement error.
};

enum class MessageType : int {
  CAN_DataFrame,      ///< Normal CAN message
  CAN_RemoteFrame,    ///< Remote frame message.
  CAN_ErrorFrame,     ///< Error message.
  CAN_OverloadFrame,  ///< Overload frame message.
};
}  // namespace MdfLibrary

#if defined(_WIN32)
// WINDOWS
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  __declspec(dllexport) ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__linux__)
// LINUX
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__CYGWIN__)
// CYGWIN
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__APPLE__)
// MACOS
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#else
// UNKNOWN
#pragma warning Unknown dynamic link import / export semantics.
#endif

extern "C" {
namespace MdfLibrary { namespace ExportFunctions {
#pragma region MdfReader
EXPORT(mdf::MdfReader*, MdfReader, Init, const char* filename);
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, mdf::MdfReader* reader, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit);
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(bool, IsOk);
EXPORTFEATUREFUNC(const mdf::MdfFile*, GetFile);
EXPORTFEATUREFUNC(const mdf::IHeader*, GetHeader);
EXPORTFEATUREFUNC(const mdf::IDataGroup*, GetDataGroup, size_t index);
EXPORTFEATUREFUNC(bool, Open);
EXPORTFEATUREFUNC(void, Close);
EXPORTFEATUREFUNC(bool, ReadHeader);
EXPORTFEATUREFUNC(bool, ReadMeasurementInfo);
EXPORTFEATUREFUNC(bool, ReadEverythingButData);
EXPORTFEATUREFUNC(bool, ReadData, const mdf::IDataGroup* group);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfWriter
EXPORT(mdf::MdfWriter*, MdfWriter, Init, MdfWriterType type,
       const char* filename);
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, mdf::MdfWriter* writer, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit);
EXPORTFEATUREFUNC(mdf::MdfFile*, GetFile);
EXPORTFEATUREFUNC(mdf::IHeader*, GetHeader);
EXPORTFEATUREFUNC(bool, IsFileNew);
EXPORTFEATUREFUNC(bool, GetCompressData);
EXPORTFEATUREFUNC(void, SetCompressData, bool compress);
EXPORTFEATUREFUNC(double, GetPreTrigTime);
EXPORTFEATUREFUNC(void, SetPreTrigTime, double pre_trig_time);
EXPORTFEATUREFUNC(uint64_t, GetStartTime);
EXPORTFEATUREFUNC(uint64_t, GetStopTime);
EXPORTFEATUREFUNC(MdfBusType, GetBusType);
EXPORTFEATUREFUNC(void, SetBusType, MdfBusType type);
EXPORTFEATUREFUNC(MdfStorageType, GetStorageType);
EXPORTFEATUREFUNC(void, SetStorageType, MdfStorageType type);
EXPORTFEATUREFUNC(uint32_t, GetMaxLength);
EXPORTFEATUREFUNC(void, SetMaxLength, uint32_t length);
EXPORTFEATUREFUNC(bool, CreateBusLogConfiguration);
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup);
EXPORTFEATUREFUNC(bool, InitMeasurement);
EXPORTFEATUREFUNC(void, SaveSample, mdf::IChannelGroup* group, uint64_t time);
EXPORTFEATUREFUNC(void, SaveCanMessage, mdf::IChannelGroup* group,
                  uint64_t time, mdf::CanMessage* message);
EXPORTFEATUREFUNC(void, StartMeasurement, uint64_t start_time);
EXPORTFEATUREFUNC(void, StopMeasurement, uint64_t stop_time);
EXPORTFEATUREFUNC(bool, FinalizeMeasurement);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFile
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFile, FuncName, mdf::MdfFile* file, ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetFileName, char* filename);
EXPORTFEATUREFUNC(void, SetFileName, const char* filename);
EXPORTFEATUREFUNC(size_t, GetVersion, char* version);
EXPORTFEATUREFUNC(int, GetMainVersion);
EXPORTFEATUREFUNC(int, GetMinorVersion);
EXPORTFEATUREFUNC(void, SetMinorVersion, int minor);
EXPORTFEATUREFUNC(size_t, GetProgramId, char* program_id);
EXPORTFEATUREFUNC(void, SetProgramId, const char* program_id);
EXPORTFEATUREFUNC(bool, GetFinalized, uint16_t& standard_flags,
                  uint16_t& custom_flags);
EXPORTFEATUREFUNC(const mdf::IHeader*, GetHeader);
EXPORTFEATUREFUNC(bool, GetIsMdf4);
EXPORTFEATUREFUNC(size_t, GetAttachments,
                  const mdf::IAttachment* pAttachment[]);
EXPORTFEATUREFUNC(size_t, GetDataGroups, const mdf::IDataGroup* pDataGroup[]);
EXPORTFEATUREFUNC(mdf::IAttachment*, CreateAttachment);
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfHeader
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfHeader, FuncName, mdf::IHeader* header, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetAuthor, char* author);
EXPORTFEATUREFUNC(void, SetAuthor, const char* author);
EXPORTFEATUREFUNC(size_t, GetDepartment, char* department);
EXPORTFEATUREFUNC(void, SetDepartment, const char* department);
EXPORTFEATUREFUNC(size_t, GetProject, char* project);
EXPORTFEATUREFUNC(void, SetProject, const char* project);
EXPORTFEATUREFUNC(size_t, GetSubject, char* subject);
EXPORTFEATUREFUNC(void, SetSubject, const char* subject);
EXPORTFEATUREFUNC(size_t, GetMeasurementId, char* uuid);
EXPORTFEATUREFUNC(void, SetMeasurementId, const char* uuid);
EXPORTFEATUREFUNC(size_t, GetRecorderId, char* uuid);
EXPORTFEATUREFUNC(void, SetRecorderId, const char* uuid);
EXPORTFEATUREFUNC(int64_t, GetRecorderIndex);
EXPORTFEATUREFUNC(void, SetRecorderIndex, int64_t index);
EXPORTFEATUREFUNC(uint64_t, GetStartTime);
EXPORTFEATUREFUNC(void, SetStartTime, uint64_t time);
EXPORTFEATUREFUNC(bool, IsStartAngleUsed);
EXPORTFEATUREFUNC(double, GetStartAngle);
EXPORTFEATUREFUNC(void, SetStartAngle, double angle);
EXPORTFEATUREFUNC(bool, IsStartDistanceUsed);
EXPORTFEATUREFUNC(double, GetStartDistance);
EXPORTFEATUREFUNC(void, SetStartDistance, double distance);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(size_t, GetAttachments, mdf::IAttachment* pAttachments[]);
EXPORTFEATUREFUNC(size_t, GetFileHistorys, mdf::IFileHistory* pFileHistorys[]);
EXPORTFEATUREFUNC(size_t, GetEvents, mdf::IEvent* pEvents[]);
EXPORTFEATUREFUNC(size_t, GetDataGroups, mdf::IDataGroup* pDataGroups[]);
EXPORTFEATUREFUNC(mdf::IDataGroup*, GetLastDataGroup);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(mdf::IAttachment*, CreateAttachment);
EXPORTFEATUREFUNC(mdf::IFileHistory*, CreateFileHistory);
#undef CreateEvent  // **** Microsoft !!
EXPORTFEATUREFUNC(mdf::IEvent*, CreateEvent);
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfDataGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                 \
  EXPORT(ReturnType, MdfDataGroup, FuncName, mdf::IDataGroup* group, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetDescription, char* description);
EXPORTFEATUREFUNC(uint8_t, GetRecordIdSize);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(mdf::IChannelGroup*, GetChannelGroupByName,
                  const char* name);
EXPORTFEATUREFUNC(mdf::IChannelGroup*, GetChannelGroupByRecordId,
                  uint64_t record_id);
EXPORTFEATUREFUNC(size_t, GetChannelGroups,
                  mdf::IChannelGroup* pChannelGroups[]);
EXPORTFEATUREFUNC(bool, IsRead);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(mdf::IChannelGroup*, CreateChannelGroup);
EXPORTFEATUREFUNC(const mdf::IChannelGroup*, FindParentChannelGroup,
                  const mdf::IChannel* channel);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                       \
  EXPORT(ReturnType, MdfChannelGroup, FuncName, mdf::IChannelGroup* group, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint64_t, GetRecordId);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(uint64_t, GetNofSamples);
EXPORTFEATUREFUNC(void, SetNofSamples, uint64_t samples);
EXPORTFEATUREFUNC(uint16_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint16_t flags);
EXPORTFEATUREFUNC(wchar_t, GetPathSeparator);
EXPORTFEATUREFUNC(void, SetPathSeparator, wchar_t sep);
EXPORTFEATUREFUNC(size_t, GetChannels, mdf::IChannel* pChannels[]);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(const mdf::ISourceInformation*, GetSourceInformation);
EXPORTFEATUREFUNC(const mdf::IChannel*, GetXChannel,
                  const mdf::IChannel* ref_channel);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(mdf::IChannel*, CreateChannel);
EXPORTFEATUREFUNC(mdf::ISourceInformation*, CreateSourceInformation);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannel
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)               \
  EXPORT(ReturnType, MdfChannel, FuncName, mdf::IChannel* channel, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDisplayName, char* name);
EXPORTFEATUREFUNC(void, SetDisplayName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(bool, IsUnitUsed);
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(ChannelType, GetType);
EXPORTFEATUREFUNC(void, SetType, ChannelType type);
EXPORTFEATUREFUNC(ChannelSyncType, GetSync);
EXPORTFEATUREFUNC(void, SetSync, ChannelSyncType type);
EXPORTFEATUREFUNC(ChannelDataType, GetDataType);
EXPORTFEATUREFUNC(void, SetDataType, ChannelDataType type);
EXPORTFEATUREFUNC(uint32_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint32_t flags);
EXPORTFEATUREFUNC(uint64_t, GetDataBytes);
EXPORTFEATUREFUNC(void, SetDataBytes, uint64_t bytes);
EXPORTFEATUREFUNC(bool, IsPrecisionUsed);
EXPORTFEATUREFUNC(uint8_t, GetPrecision);
EXPORTFEATUREFUNC(bool, IsRangeUsed);
EXPORTFEATUREFUNC(double, GetRangeMin);
EXPORTFEATUREFUNC(double, GetRangeMax);
EXPORTFEATUREFUNC(void, SetRange, double min, double max);
EXPORTFEATUREFUNC(bool, IsLimitUsed);
EXPORTFEATUREFUNC(double, GetLimitMin);
EXPORTFEATUREFUNC(double, GetLimitMax);
EXPORTFEATUREFUNC(void, SetLimit, double min, double max);
EXPORTFEATUREFUNC(bool, IsExtLimitUsed);
EXPORTFEATUREFUNC(double, GetExtLimitMin);
EXPORTFEATUREFUNC(double, GetExtLimitMax);
EXPORTFEATUREFUNC(void, SetExtLimit, double min, double max);
EXPORTFEATUREFUNC(double, GetSamplingRate);
EXPORTFEATUREFUNC(uint64_t, GetVlsdRecordId);
EXPORTFEATUREFUNC(void, SetVlsdRecordId, uint64_t record_id);
EXPORTFEATUREFUNC(uint32_t, GetBitCount);
EXPORTFEATUREFUNC(void, SetBitCount, uint32_t bits);
EXPORTFEATUREFUNC(uint16_t, GetBitOffset);
EXPORTFEATUREFUNC(void, SetBitOffset, uint16_t bits);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(const mdf::ISourceInformation*, GetSourceInformation);
EXPORTFEATUREFUNC(const mdf::IChannelConversion*, GetChannelConversion);
EXPORTFEATUREFUNC(const mdf::IChannelArray*, GetChannelArray);
EXPORTFEATUREFUNC(size_t, GetChannelCompositions, mdf::IChannel* pChannels[]);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
EXPORTFEATUREFUNC(mdf::ISourceInformation*, CreateSourceInformation);
EXPORTFEATUREFUNC(mdf::IChannelConversion*, CreateChannelConversion);
EXPORTFEATUREFUNC(mdf::IChannelArray*, CreateChannelArray);
EXPORTFEATUREFUNC(mdf::IChannel*, CreateChannelComposition);
EXPORTFEATUREFUNC(void, SetChannelValueAsSigned, const int64_t value,
                  bool valid = true, uint64_t array_index = 0);
EXPORTFEATUREFUNC(void, SetChannelValueAsUnSigned, const uint64_t value,
                  bool valid = true, uint64_t array_index = 0);
EXPORTFEATUREFUNC(void, SetChannelValueAsFloat, const double value,
                  bool valid = true, uint64_t array_index = 0);
EXPORTFEATUREFUNC(void, SetChannelValueAsString, const char* value,
                  bool valid = true, uint64_t array_index = 0);
EXPORTFEATUREFUNC(void, SetChannelValueAsArray, const uint8_t* value,
                  size_t size, bool valid = true, uint64_t array_index = 0);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelConversion
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannelConversion, FuncName, \
         mdf::IChannelConversion* conv, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(ConversionType, GetType);
EXPORTFEATUREFUNC(void, SetType, ConversionType type);
EXPORTFEATUREFUNC(bool, IsPrecisionUsed);
EXPORTFEATUREFUNC(uint8_t, GetPrecision);
EXPORTFEATUREFUNC(bool, IsRangeUsed);
EXPORTFEATUREFUNC(double, GetRangeMin);
EXPORTFEATUREFUNC(double, GetRangeMax);
EXPORTFEATUREFUNC(void, SetRange, double min, double max);
EXPORTFEATUREFUNC(uint16_t, GetFlags);
EXPORTFEATUREFUNC(size_t, GetReference, uint16_t index, char* reference);
EXPORTFEATUREFUNC(void, SetReference, uint16_t index, const char* reference);
EXPORTFEATUREFUNC(const mdf::IChannelConversion*, GetInverse);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(size_t, GetFormula, char* formula);
EXPORTFEATUREFUNC(void, SetFormula, const char* formula);
EXPORTFEATUREFUNC(double, GetNofParameters, uint16_t index);
EXPORTFEATUREFUNC(double, GetParameterAsDouble, uint16_t index);
EXPORTFEATUREFUNC(void, SetParameterAsDouble, uint16_t index, double parameter);
EXPORTFEATUREFUNC(uint64_t, GetParameterAsUInt64, uint16_t index);
EXPORTFEATUREFUNC(void, SetParameterAsUInt64, uint16_t index,
                  uint64_t parameter);
EXPORTFEATUREFUNC(mdf::IChannelConversion*, CreateInverse);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelObserver
EXPORT(mdf::IChannelObserver*, MdfChannelObserver, Create,
       mdf::IDataGroup* data_group, mdf::IChannelGroup* channel_group,
       mdf::IChannel* channel);
EXPORT(mdf::IChannelObserver*, MdfChannelObserver, CreateByChannelName,
       mdf::IDataGroup* data_group, const char* channel_name);
EXPORT(size_t, MdfChannelObserver, CreateForChannelGroup,
       mdf::IDataGroup* data_group, mdf::IChannelGroup* channel_group,
       mdf::IChannelObserver* pObservers[]);
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannelObserver, FuncName,   \
         mdf::IChannelObserver* observer, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit);
EXPORTFEATUREFUNC(int64_t, GetNofSamples);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit);
EXPORTFEATUREFUNC(const mdf::IChannel*, GetChannel);
EXPORTFEATUREFUNC(bool, IsMaster);
EXPORTFEATUREFUNC(bool, GetChannelValueAsSigned, uint64_t sample,
                  int64_t& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsUnSigned, uint64_t sample,
                  uint64_t& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsFloat, uint64_t sample, double& value);
EXPORTFEATUREFUNC(bool, GetChannelValueAsString, uint64_t sample, char* value,
                  size_t& size);
EXPORTFEATUREFUNC(bool, GetChannelValueAsArray, uint64_t sample,
                  uint8_t value[], size_t& size);
EXPORTFEATUREFUNC(bool, GetEngValueAsSigned, uint64_t sample, int64_t& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsUnSigned, uint64_t sample,
                  uint64_t& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsFloat, uint64_t sample, double& value);
EXPORTFEATUREFUNC(bool, GetEngValueAsString, uint64_t sample, char* value,
                  size_t& size);
EXPORTFEATUREFUNC(bool, GetEngValueAsArray, uint64_t sample, uint8_t value[],
                  size_t& size);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfSourceInformation
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfSourceInformation, FuncName, \
         mdf::ISourceInformation* source_information, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetPath, char* path);
EXPORTFEATUREFUNC(void, SetPath, const char* path);
EXPORTFEATUREFUNC(SourceType, GetType);
EXPORTFEATUREFUNC(void, SetType, SourceType type);
EXPORTFEATUREFUNC(BusType, GetBus);
EXPORTFEATUREFUNC(void, SetBus, BusType bus);
EXPORTFEATUREFUNC(uint8_t, GetFlags);
EXPORTFEATUREFUNC(void, SetFlags, uint8_t flags);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfAttachment
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                        \
  EXPORT(ReturnType, MdfAttachment, FuncName, mdf::IAttachment* attachment, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex);
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index);
EXPORTFEATUREFUNC(bool, GetEmbedded);
EXPORTFEATUREFUNC(void, SetEmbedded, bool embedded);
EXPORTFEATUREFUNC(bool, GetCompressed);
EXPORTFEATUREFUNC(void, SetCompressed, bool compressed);
EXPORTFEATUREFUNC(bool, GetMd5, char* md5);
EXPORTFEATUREFUNC(size_t, GetFileName, char* name);
EXPORTFEATUREFUNC(void, SetFileName, const char* name);
EXPORTFEATUREFUNC(size_t, GetFileType, char* type);
EXPORTFEATUREFUNC(void, SetFileType, const char* type);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFileHistory
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFileHistory, FuncName,       \
         mdf::IFileHistory* file_history, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(uint64_t, GetTime);
EXPORTFEATUREFUNC(void, SetTime, uint64_t time);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetToolName, char* name);
EXPORTFEATUREFUNC(void, SetToolName, const char* name);
EXPORTFEATUREFUNC(size_t, GetToolVendor, char* vendor);
EXPORTFEATUREFUNC(void, SetToolVendor, const char* vendor);
EXPORTFEATUREFUNC(size_t, GetToolVersion, char* version);
EXPORTFEATUREFUNC(void, SetToolVersion, const char* version);
EXPORTFEATUREFUNC(size_t, GetUserName, char* user);
EXPORTFEATUREFUNC(void, SetUserName, const char* user);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfEvent
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfEvent, FuncName, mdf::IEvent* event, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetGroupName, char* group);
EXPORTFEATUREFUNC(void, SetGroupName, const char* group);
EXPORTFEATUREFUNC(EventType, GetType);
EXPORTFEATUREFUNC(void, SetType, EventType type);
EXPORTFEATUREFUNC(SyncType, GetSync);
EXPORTFEATUREFUNC(void, SetSync, SyncType type);
EXPORTFEATUREFUNC(RangeType, GetRange);
EXPORTFEATUREFUNC(void, SetRange, RangeType type);
EXPORTFEATUREFUNC(EventCause, GetCause);
EXPORTFEATUREFUNC(void, SetCause, EventCause cause);
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex);
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index);
EXPORTFEATUREFUNC(int64_t, GetSyncValue);
EXPORTFEATUREFUNC(void, SetSyncValue, int64_t value);
EXPORTFEATUREFUNC(double, GetSyncFactor);
EXPORTFEATUREFUNC(void, SetSyncFactor, double factor);
EXPORTFEATUREFUNC(const mdf::IEvent*, GetParentEvent);
EXPORTFEATUREFUNC(void, SetParentEvent, const mdf::IEvent* parent);
EXPORTFEATUREFUNC(const mdf::IEvent*, GetRangeEvent);
EXPORTFEATUREFUNC(void, SetRangeEvent, const mdf::IEvent* range);
EXPORTFEATUREFUNC(size_t, GetAttachments,
                  const mdf::IAttachment* pAttachment[]);
EXPORTFEATUREFUNC(double, GetPreTrig);
EXPORTFEATUREFUNC(void, SetPreTrig, double time);
EXPORTFEATUREFUNC(double, GetPostTrig);
EXPORTFEATUREFUNC(void, SetPostTrig, double time);
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData);
EXPORTFEATUREFUNC(void, AddAttachment, const mdf::IAttachment* attachment);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfETag
EXPORT(mdf::ETag*, MdfETag, Init);
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfETag, FuncName, mdf::ETag* etag, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit);
EXPORTFEATUREFUNC(size_t, GetName, char* name);
EXPORTFEATUREFUNC(void, SetName, const char* name);
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc);
EXPORTFEATUREFUNC(void, SetDescription, const char* desc);
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit);
EXPORTFEATUREFUNC(void, SetUnit, const char* unit);
EXPORTFEATUREFUNC(size_t, GetUnitRef, char* unit);
EXPORTFEATUREFUNC(void, SetUnitRef, const char* unit);
EXPORTFEATUREFUNC(size_t, GetType, char* type);
EXPORTFEATUREFUNC(void, SetType, const char* type);
EXPORTFEATUREFUNC(ETagDataType, GetDataType);
EXPORTFEATUREFUNC(void, SetDataType, ETagDataType type);
EXPORTFEATUREFUNC(size_t, GetLanguage, char* language);
EXPORTFEATUREFUNC(void, SetLanguage, const char* language);
EXPORTFEATUREFUNC(bool, GetReadOnly);
EXPORTFEATUREFUNC(void, SetReadOnly, const bool read_only);
EXPORTFEATUREFUNC(size_t, GetValueAsString, char* value);
EXPORTFEATUREFUNC(void, SetValueAsString, const char* value);
EXPORTFEATUREFUNC(double, GetValueAsFloat);
EXPORTFEATUREFUNC(void, SetValueAsFloat, double value);
EXPORTFEATUREFUNC(bool, GetValueAsBoolean);
EXPORTFEATUREFUNC(void, SetValueAsBoolean, bool value);
EXPORTFEATUREFUNC(int64_t, GetValueAsSigned);
EXPORTFEATUREFUNC(void, SetValueAsSigned, int64_t value);
EXPORTFEATUREFUNC(uint64_t, GetValueAsUnsigned);
EXPORTFEATUREFUNC(void, SetValueAsUnsigned, uint64_t value);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfMetaData
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                  \
  EXPORT(ReturnType, MdfMetaData, FuncName, mdf::IMetaData* metadata, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetPropertyAsString, const char* index, char* prop);
EXPORTFEATUREFUNC(void, SetPropertyAsString, const char* index,
                  const char* prop);
EXPORTFEATUREFUNC(double, GetPropertyAsFloat, const char* index);
EXPORTFEATUREFUNC(void, SetPropertyAsFloat, const char* index, double prop);
EXPORTFEATUREFUNC(size_t, GetProperties, mdf::ETag* pProperty[]);
EXPORTFEATUREFUNC(size_t, GetCommonProperties, mdf::ETag* pProperty[]);
EXPORTFEATUREFUNC(void, SetCommonProperties, const mdf::ETag* pProperty[],
                  size_t count);
EXPORTFEATUREFUNC(size_t, GetXmlSnippet, char* xml);
EXPORTFEATUREFUNC(void, SetXmlSnippet, const char* xml);
EXPORTFEATUREFUNC(void, AddCommonProperty, mdf::ETag* tag);
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region mdf::CanMessage
EXPORT(mdf::CanMessage*, CanMessage, Init);
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, CanMessage, FuncName, mdf::CanMessage* can, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit);
EXPORTFEATUREFUNC(uint32_t, GetMessageId);
EXPORTFEATUREFUNC(void, SetMessageId, const uint32_t msgId);
EXPORTFEATUREFUNC(uint32_t, GetCanId);
EXPORTFEATUREFUNC(bool, GetExtendedId);
EXPORTFEATUREFUNC(void, SetExtendedId, const bool extendedId);
EXPORTFEATUREFUNC(uint8_t, GetDlc);
EXPORTFEATUREFUNC(void, SetDlc, const uint8_t dlc);
EXPORTFEATUREFUNC(size_t, GetDataLength);
EXPORTFEATUREFUNC(void, SetDataLength, const uint32_t dataLength);
EXPORTFEATUREFUNC(size_t, GetDataBytes, uint8_t dataList[]);
EXPORTFEATUREFUNC(void, SetDataBytes, const uint8_t* dataList,
                  const size_t size);
EXPORTFEATUREFUNC(uint64_t, GetDataIndex);
EXPORTFEATUREFUNC(void, SetDataIndex, const uint64_t index);
EXPORTFEATUREFUNC(bool, GetDir);
EXPORTFEATUREFUNC(void, SetDir, const bool transmit);
EXPORTFEATUREFUNC(bool, GetSrr);
EXPORTFEATUREFUNC(void, SetSrr, const bool srr);
EXPORTFEATUREFUNC(bool, GetEdl);
EXPORTFEATUREFUNC(void, SetEdl, const bool edl);
EXPORTFEATUREFUNC(bool, GetBrs);
EXPORTFEATUREFUNC(void, SetBrs, const bool brs);
EXPORTFEATUREFUNC(bool, GetEsi);
EXPORTFEATUREFUNC(void, SetEsi, const bool esi);
EXPORTFEATUREFUNC(bool, GetRtr);
EXPORTFEATUREFUNC(void, SetRtr, const bool rtr);
EXPORTFEATUREFUNC(bool, GetWakeUp);
EXPORTFEATUREFUNC(void, SetWakeUp, const bool wakeUp);
EXPORTFEATUREFUNC(bool, GetSingleWire);
EXPORTFEATUREFUNC(void, SetSingleWire, const bool singleWire);
EXPORTFEATUREFUNC(uint8_t, GetBusChannel);
EXPORTFEATUREFUNC(void, SetBusChannel, const uint8_t channel);
EXPORTFEATUREFUNC(uint8_t, GetBitPosition);
EXPORTFEATUREFUNC(void, SetBitPosition, const uint8_t position);
EXPORTFEATUREFUNC(CanErrorType, GetErrorType);
EXPORTFEATUREFUNC(void, SetErrorType, const CanErrorType type);
#undef EXPORTFEATUREFUNC
#pragma endregion
}}  // namespace MdfLibrary::ExportFunctions
}

#undef EXPORT