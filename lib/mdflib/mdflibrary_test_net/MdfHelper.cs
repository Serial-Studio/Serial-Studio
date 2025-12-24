
namespace mdflibrary_test;

public static class MdfHelper
{
    public static ulong GetUnixNanoTimestamp(DateTime time)
    {
        var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        return (ulong)(time.ToUniversalTime().Subtract(epoch).Ticks * 100);
    }

    public static ulong GetLocalNanoTimestamp(DateTime time)
    {
        var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        var localTime = time.ToLocalTime();
        return (ulong)(localTime.Subtract(epoch).Ticks * 100);
    }



}