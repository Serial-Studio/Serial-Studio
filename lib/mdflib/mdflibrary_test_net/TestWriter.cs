/*
 * Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
using MdfLibrary;
using Microsoft.VisualStudio.TestPlatform.CommunicationUtilities.ObjectModel;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;

namespace mdflibrary_test;
using MdfLibrary;

[TestClass]
public class TestWriter
{
    private static string _testDirectory = "";
    private static bool _skipTest = false;


    [ClassInitialize]
    public static void ClassInit(TestContext testContext) 
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


    [ClassCleanup]
    public static void ClassCleanup()
    {
        Console.WriteLine("Writer tests exited.");
    }

    [TestMethod]
    public void TestProperties()
    {
        MdfWriter writer = new MdfWriter(MdfWriterType.MdfBusLogger);
        Assert.IsNotNull(writer);
        Assert.AreEqual(MdfWriterType.MdfBusLogger, writer.TypeOfWriter);

        string testFile = Path.Combine(_testDirectory, "bus_prop.mf4");
        writer.Init(testFile);
        Assert.AreEqual("bus_prop", writer.Name);
        
        Assert.IsNotNull(writer.File);
        Assert.IsNotNull(writer.Header);
        Assert.IsTrue(writer.IsFileNew);

        writer.CompressData = false;
        Assert.IsFalse(writer.CompressData);
        writer.CompressData = true;
        Assert.IsTrue(writer.CompressData);
        
        writer.MandatoryMemberOnly = false;
        Assert.IsFalse(writer.MandatoryMemberOnly);
        writer.MandatoryMemberOnly = true;
        Assert.IsTrue(writer.MandatoryMemberOnly);
        
        writer.PreTrigTime = 1.23;
        Assert.AreEqual(1.23, writer.PreTrigTime);
        Assert.AreEqual(0u,writer.StartTime );
        Assert.AreEqual(0u,writer.StopTime );
        
        writer.BusType = MdfBusType.FlexRay;
        Assert.AreEqual(MdfBusType.FlexRay, writer.BusType);
        
        writer.StorageType = MdfStorageType.MlsdStorage;
        Assert.AreEqual(MdfStorageType.MlsdStorage, writer.StorageType);   
        
        writer.MaxLength = 11u;
        Assert.AreEqual(11u, writer.MaxLength);
        writer.MandatoryMemberOnly = true;
        
        Assert.IsNotNull(writer.CreateDataGroup());
        Assert.IsTrue(writer.CreateBusLogConfiguration());
    }

    [TestMethod]
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
        Assert.IsNotNull(header);
        
        // The file history is required in MDF 4
        MdfFileHistory fileHistory = header.CreateFileHistory();
        Assert.IsNotNull(fileHistory);
        fileHistory.Description = "Testing CAN bus logging with compression";
        fileHistory.ToolName = "MdfLibrary";
        fileHistory.ToolVendor = "IH Development";
        fileHistory.ToolVersion = "2.3.0";
        fileHistory.UserName = "Ingemar Hedvall";
        MdfDataGroup lastDataGroup = header.LastDataGroup;
        Assert.IsNotNull(lastDataGroup);

        MdfChannelGroup dataFrameGroup = lastDataGroup.GetChannelGroup("CAN_DataFrame");
        Assert.IsNotNull(dataFrameGroup);
        
        writer.InitMeasurement(); // Starts the internal cache
        ulong startTime = MdfLibrary.NowNs();
        writer.StartMeasurement(startTime);
        Assert.AreEqual(startTime, writer.StartTime);
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
    [TestMethod]
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
        Assert.IsNotNull(header);
        
        // The file history is required in MDF 4
        MdfFileHistory fileHistory = header.CreateFileHistory();
        Assert.IsNotNull(fileHistory);
        fileHistory.Description = "Testing CAN bus logging with compression";
        fileHistory.ToolName = "MdfLibrary";
        fileHistory.ToolVendor = "IH Development";
        fileHistory.ToolVersion = "2.3.0";
        fileHistory.UserName = "Ingemar Hedvall";
        
        MdfDataGroup lastDataGroup = header.LastDataGroup;
        Assert.IsNotNull(lastDataGroup);

        MdfChannelGroup dataFrameGroup = lastDataGroup.GetChannelGroup("CAN_DataFrame");
        Assert.IsNotNull(dataFrameGroup);
        
        writer.InitMeasurement(); // Starts the internal cache
        ulong startTime = MdfLibrary.NowNs();
        writer.StartMeasurement(startTime);
        Assert.AreEqual(startTime, writer.StartTime);
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
