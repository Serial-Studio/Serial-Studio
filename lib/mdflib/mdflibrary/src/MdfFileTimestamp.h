#pragma once
#include <cstdint>
#include "mdf/imdftimestamp.h"



namespace MdfLibrary {
using System::DateTime;

public interface class IMdfFileTimestamp {
  property uint64_t TimeNs { uint64_t get(); }
  property int16_t TimezoneMin { int16_t get(); }
  property int16_t DstMin { int16_t get(); }
  property DateTime Time { DateTime get(); }
};


public ref class MdfFileUtcTimestamp : public IMdfFileTimestamp {
public:
  explicit MdfFileUtcTimestamp(uint64_t utc_time_ns);
  property uint64_t TimeNs { virtual uint64_t get(); }
  property int16_t TimezoneMin { virtual int16_t get(); }
  property int16_t DstMin { virtual int16_t get(); }
  property DateTime Time { virtual DateTime get(); }

private:
  uint64_t utc_time_ns_;
};

public ref class MdfFileLocalTimestamp : public IMdfFileTimestamp {
public:
  explicit MdfFileLocalTimestamp(uint64_t local_time_ns);
  property uint64_t TimeNs { virtual uint64_t get(); }
  property int16_t TimezoneMin { virtual int16_t get(); }
  property int16_t DstMin { virtual int16_t get(); }
  property DateTime Time { virtual DateTime get(); }

private:
  uint64_t local_time_ns_;
};

public ref class MdfFileLocalTimestampTz : public IMdfFileTimestamp {
public:
  explicit MdfFileLocalTimestampTz(uint64_t local_time_ns, int16_t tz_min);
  property uint64_t TimeNs { virtual uint64_t get(); }
  property int16_t TimezoneMin { virtual int16_t get(); }
  property int16_t DstMin { virtual int16_t get(); }
  property DateTime Time { virtual DateTime get(); }

private:
  uint64_t local_time_ns_;
  int16_t tz_min_;
};


public ref class MdfFileTimezoneTimestamp : public IMdfFileTimestamp {
public:
  MdfFileTimezoneTimestamp(uint64_t time_ns, int16_t timezone_min,
                           int16_t dst_min);
  property uint64_t TimeNs { virtual uint64_t get(); }
  property int16_t TimezoneMin { virtual int16_t get(); }
  property int16_t DstMin { virtual int16_t get(); }
  property DateTime Time { virtual DateTime get(); }

private:
  uint64_t time_ns_;
  int16_t timezone_min_;
  int16_t dst_min_;
};

IMdfFileTimestamp^ GetMdfFileTimestampByIMdfTimestamp(const mdf::IMdfTimestamp* timestamp);
}
