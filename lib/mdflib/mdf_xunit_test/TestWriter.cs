
using System.Reflection;
using Xunit;

namespace mdf_xunit_test
{

    using MdfLibrary;
public class TestWriter : IDisposable
{
    private static string _testDirectory = "";
    private static bool _skipTest = false;

   
    public TestWriter() 
    {    
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        // Log all internal messages onto the console
        Console.WriteLine("Writer tests started.");
        MdfLibrary.Instance.LogEvent += (MdfLogSeverity severity, string function, string message) =>
        {
            Console.WriteLine("{0} {1} {2}", severity, function, message);
        };

        // Set up test directory to %TEMP%/test/mdflibrary/write
        _testDirectory = Path.Combine(Path.GetTempPath(), 
            "test", "mdflibrary", "write");
        if (Directory.Exists(_testDirectory)) {
            Directory.Delete(_testDirectory, true);
        }
        Directory.CreateDirectory(_testDirectory);
        _skipTest = !Directory.Exists(_testDirectory);  
        if (_skipTest)
        {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Error,functionName, 
                "Failed to find the test directory. Dir: " + _testDirectory);
        }
        MdfLibrary.Instance.AddLog(MdfLogSeverity.Trace, functionName, "Test Dir: " + _testDirectory);
    }


   
    public void Dispose()
    {
        Console.WriteLine("Writer tests exited.");
    }

    [Fact]
    public void TestProperties()
    {
        MdfWriter writer = new MdfWriter(MdfWriterType.MdfBusLogger);
        Assert.NotNull(writer);
        Assert.Equal(MdfWriterType.MdfBusLogger, writer.TypeOfWriter);

        string testFile = Path.Combine(_testDirectory, "bus_prop.mf4");
        writer.Init(testFile);
        Assert.Equal("bus_prop", writer.Name);
        
        Assert.NotNull(writer.File);
        Assert.NotNull(writer.Header);
        Assert.True(writer.IsFileNew);

        writer.CompressData = false;
        Assert.False(writer.CompressData);
        writer.CompressData = true;
        Assert.True(writer.CompressData);
        
        writer.MandatoryMemberOnly = false;
        Assert.False(writer.MandatoryMemberOnly);
        writer.MandatoryMemberOnly = true;
        Assert.True(writer.MandatoryMemberOnly);
        
        writer.PreTrigTime = 1.23;
        Assert.Equal(1.23, writer.PreTrigTime);
        Assert.Equal(0u,writer.StartTime );
        Assert.Equal(0u,writer.StopTime );
        
        writer.BusType = MdfBusType.FlexRay;
        Assert.Equal(MdfBusType.FlexRay, writer.BusType);
        
        writer.StorageType = MdfStorageType.MlsdStorage;
        Assert.Equal(MdfStorageType.MlsdStorage, writer.StorageType);   
        
        writer.MaxLength = 11u;
        Assert.Equal(11u, writer.MaxLength);
        writer.MandatoryMemberOnly = true;
        
        Assert.NotNull(writer.CreateDataGroup());
        Assert.True(writer.CreateBusLogConfiguration());
    }
    
    [Fact]
    public void TestNoCompressCanBus()
    {
        MdfWriter writer = new MdfWriter(MdfWriterType.MdfBusLogger);
        string testFile = Path.Combine(_testDirectory, "nocompress_can.mf4");
        writer.Init(testFile);
        writer.CompressData = false;
        writer.MandatoryMemberOnly = false;    
     
        writer.BusType = MdfBusType.CAN;        
        writer.PreTrigTime = 0.0; 
        writer.CreateBusLogConfiguration();
            
        MdfHeader header = writer.Header;
        Assert.NotNull(header);
        
        // The file history is required in MDF 4
        MdfFileHistory fileHistory = header.CreateFileHistory();
        Assert.NotNull(fileHistory);
        fileHistory.Description = "Testing CAN bus logging with compression";
        fileHistory.ToolName = "MdfLibrary";
        fileHistory.ToolVendor = "IH Development";
        fileHistory.ToolVersion = "2.3.0";
        fileHistory.UserName = "Ingemar Hedvall";
        MdfDataGroup lastDataGroup = header.LastDataGroup;
        Assert.NotNull(lastDataGroup);

        MdfChannelGroup dataFrameGroup = lastDataGroup.GetChannelGroup("CAN_DataFrame");
        Assert.NotNull(dataFrameGroup);
        
        writer.InitMeasurement(); // Starts the internal cache
        ulong startTime = MdfLibrary.NowNs();
        writer.StartMeasurement(startTime);
        Assert.Equal(startTime, writer.StartTime);
        CanMessage msg = new CanMessage();
        msg.TypeOfMessage = CanMessageType.Can_DataFrame;
        
        msg.MessageId = 1234;
        msg.DataBytes = [ 1, 2, 3, 4, 5, 6, 7, 8 ];
        
        for (int sample = 0; sample < 1000; ++sample)
        {
           writer.SaveCanMessage(dataFrameGroup, startTime, msg); 
           startTime += 1_000_000_000u; // 1 s     
        }
        writer.StopMeasurement(startTime);
        writer.FinalizeMeasurement();
    }
    
    [Fact]
    public void TestCompressCanBus()
    {
        MdfWriter writer = new MdfWriter(MdfWriterType.MdfBusLogger);
        string testFile = Path.Combine(_testDirectory, "compress_can.mf4");
        writer.Init(testFile);
        writer.CompressData = true;
        writer.MandatoryMemberOnly = true;    
        
        writer.BusType = MdfBusType.CAN;        
        writer.PreTrigTime = 0.0; 
        writer.CreateBusLogConfiguration();
            
        MdfHeader header = writer.Header;
        Assert.NotNull(header);
        
        // The file history is required in MDF 4
        MdfFileHistory fileHistory = header.CreateFileHistory();
        Assert.NotNull(fileHistory);
        fileHistory.Description = "Testing CAN bus logging with compression";
        fileHistory.ToolName = "MdfLibrary";
        fileHistory.ToolVendor = "IH Development";
        fileHistory.ToolVersion = "2.3.0";
        fileHistory.UserName = "Ingemar Hedvall";
        
        MdfDataGroup lastDataGroup = header.LastDataGroup;
        Assert.NotNull(lastDataGroup);

        MdfChannelGroup dataFrameGroup = lastDataGroup.GetChannelGroup("CAN_DataFrame");
        Assert.NotNull(dataFrameGroup);
        
        writer.InitMeasurement(); // Starts the internal cache
        ulong startTime = MdfLibrary.NowNs();
        writer.StartMeasurement(startTime);
        Assert.Equal(startTime, writer.StartTime);
        CanMessage msg = new CanMessage();
        msg.TypeOfMessage = CanMessageType.Can_DataFrame;
        
        msg.MessageId = 1234;
        msg.DataBytes = [ 1, 2, 3, 4, 5, 6, 7, 8 ];
        
        for (int sample = 0; sample < 1000; ++sample)
        {
            writer.SaveCanMessage(dataFrameGroup, startTime, msg); 
            startTime += 1_000_000_000u; // 1 s     
        }
        writer.StopMeasurement(startTime);
        writer.FinalizeMeasurement();
    }
}
}
