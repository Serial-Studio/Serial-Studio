using System;
using System.IO;
using System.Net;
using Windows.Devices.Display.Core;
using Microsoft.VisualBasic.FileIO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using WinRT;

namespace mdflibrary_test;
using MdfLibrary;


[TestClass]
public class MdfLibraryTest
{
    private const string TestFile1 = @"test1.mf4";
    private const string TestFile2 = @"test2.mf4";
    private const string TestFile3 = @"test3.mf4";
    private const string InvalidFile = @"testi.mf4";

    private const string TestFile4 = @"中文.mf4";
    private const string TestFile5 = @"test5.mf4";
    private static string _testDirectory = "";
    private static bool _skipTest = false;
    
    [ClassInitialize]
    public static void ClassInit(TestContext testContext) 
    {    
        Console.WriteLine("Unit tests started.");
        MdfLibrary.Instance.LogEvent += (MdfLogSeverity severity, string function, string message) =>
        {
            Console.WriteLine("{0} {1} {2}", severity, function, message);
        };

        // Set up test directory to %TEMP%/test/mdflibrary
        _testDirectory = Path.Combine(Path.GetTempPath(), "test", "mdflibrary");
        if (Directory.Exists(_testDirectory))
        {
            Directory.Delete(_testDirectory, true);
        }
        Directory.CreateDirectory(_testDirectory);
        _skipTest = !Directory.Exists(_testDirectory);      
        Console.WriteLine("Unit tests started. Test Dir: " + _testDirectory);
    }


    [ClassCleanup]
    public static void ClassCleanup()
    {
        Console.WriteLine("Unit tests exited.");
    }

    [TestInitialize]
    public  void TestInit()
    {            
    
    }
 
    [TestCleanup]
    public  void TestExit()
    {

    }
    

    [TestMethod]
    public void TestNormalRead()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsNotNull(reader);

        var readHeader = reader.ReadHeader();
        Assert.IsTrue(readHeader);

        var readInfo = reader.ReadMeasurementInfo();
        Assert.IsTrue(readInfo);

        var readAll = reader.ReadEverythingButData();
        Assert.IsTrue(readAll);
    }

    [TestMethod]
    public void TestReader()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader1 = new MdfReader(TestFile1);
        Assert.IsNotNull(reader1);
        reader1.Index = 666;
        Assert.AreEqual(reader1.Index, 666);

        Console.WriteLine("Name: {0}", reader1.Name);
        Assert.IsNotNull(reader1.File);

        // As we not read yet, the following functions should return null
        Assert.IsNull(reader1.Header);
        Assert.IsNull(reader1.get_DataGroup(0));

        Assert.IsTrue(reader1.IsOk);
        reader1.Close();
        Assert.IsTrue(reader1.Open());

        Assert.IsTrue(reader1.ReadHeader());
        Assert.IsTrue(reader1.ReadMeasurementInfo());
        Assert.IsTrue(reader1.ReadEverythingButData());

        Assert.IsNotNull(reader1.Header);
        Assert.IsNotNull(reader1.get_DataGroup(0));
        Assert.IsTrue(reader1.IsOk);
        reader1.Close();

        var reader2 = new MdfReader(InvalidFile);
        Assert.IsFalse(reader2.IsOk);
        Assert.IsNull(reader2.File);
    }

    [TestMethod]
    public void TestFile()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        Assert.IsNotNull(file);

        var attachments = file.Attachments;
        Assert.IsNotNull(attachments);
   
        var datagroups = file.DataGroups;
        Assert.IsNotNull(datagroups);
        Assert.IsTrue(datagroups.Length > 0);

        Console.WriteLine("Name: {0}", file.Name);
        Console.WriteLine("Filename: {0}", file.FileName);
        Console.WriteLine("Version: {0}", file.Version);
        Console.WriteLine("Main Version: {0}", file.MainVersion);
        Console.WriteLine("Minor Version: {0}", file.MinorVersion);
        Console.WriteLine("Program Id: {0}", file.ProgramId);

        Assert.IsNotNull(file.Header);
        Assert.IsTrue(file.Finalized);
        Assert.IsTrue(file.IsMdf4);

        Assert.IsNotNull(file.CreateAttachment());
        attachments = file.Attachments;
        Assert.IsTrue(attachments.Length > 0);

        Assert.IsNotNull(file.CreateDataGroup());
        datagroups = file.DataGroups;
        Assert.IsTrue(datagroups.Length > 0);

        Assert.IsNull(file.FindParentDataGroup(null));
    }

    [TestMethod]
    public void TestHeader()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var header = reader.Header;
        Assert.IsNotNull(header);

        Assert.AreEqual(header.Index, 64);
        Console.WriteLine("Description: {0}", header.Description);
        Console.WriteLine("Author: {0}", header.Author);
        Console.WriteLine("Department: {0}", header.Department);
        Console.WriteLine("Project: {0}", header.Project);
        Console.WriteLine("Subject: {0}", header.Subject);
        Console.WriteLine("Measurement ID: {0}", header.MeasurementId);
        Console.WriteLine("Recorder ID: {0}", header.RecorderId);
        Console.WriteLine("Recorder Index: {0}", header.RecorderIndex);
        Console.WriteLine("Start Time {0}", header.StartTime);
        Console.WriteLine("Start Angle {0}", header.StartAngle);
        Console.WriteLine("Start Distance {0}", header.StartDistance);

        Assert.IsNotNull(header.MetaData);

        var attachments = header.Attachments;
       Assert.IsNotNull(attachments);

        var histories = header.FileHistories;
        Assert.IsNotNull(histories);
        Assert.IsTrue(histories.Length > 0);

        var events = header.Events;
        Assert.IsNotNull(events);
       
        var groups = header.DataGroups;
        Assert.IsNotNull(groups);
        Assert.IsTrue(groups.Length > 0);

        Assert.IsFalse(header.IsStartAngleUsed);
        Assert.IsFalse(header.IsStartDistanceUsed);

        Assert.IsNotNull(header.CreateAttachment());
        Assert.IsNotNull(header.CreateFileHistory());
        Assert.IsNotNull(header.CreateEvent());
        Assert.IsNotNull(header.CreateDataGroup());
    }

    [TestMethod]
    public void TestHistory()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var header = reader.Header;
        var list = header.FileHistories;
        var history = list[0];
        Assert.IsNotNull(history);

        Console.WriteLine("Index: {0}", history.Index);
        Console.WriteLine("Time: {0}", history.Time);
        Console.WriteLine("Description: {0}", history.Description);
        Console.WriteLine("Tool Name: {0}", history.ToolName);
        Console.WriteLine("Tool Vendor: {0}", history.ToolVendor);
        Console.WriteLine("Tool Version: {0}", history.ToolVersion);
        Console.WriteLine("User Name: {0}", history.UserName);

        Assert.IsNotNull(history.MetaData);
    }

    [TestMethod]
    public void TestMetaData()
    {
        if (_skipTest)
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var testFile = Path.Combine(_testDirectory, "meta_data.mf4");
        {
            var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
            Assert.IsTrue(writer.Init(testFile));

            var header = writer.Header;
            header.Description = "Test meta data";
            header.Author = "Ingemar Hedvall";
            
            var metaData = header.CreateMetaData();
            Assert.IsNotNull(metaData, "Failed to create meta data object.");

            var engineCode = new MdfETag();
            engineCode.Name = "EngineCode";
            engineCode.DataType = ETagDataType.StringType;
            metaData.AddCommonProperty(engineCode);

            var history = header.CreateFileHistory();
            Assert.IsNotNull(history);
            history.Description = "File history.";
            history.ToolName = "MdfLibrary Assembly";
            history.ToolVendor = "GitHub:ihedvall/mdflib";
            history.ToolVersion = "2.2";

            var dataGroup = header.CreateDataGroup();
            Assert.IsNotNull(dataGroup);

            var channelGroup = dataGroup.CreateChannelGroup();
            Assert.IsNotNull(channelGroup);
            channelGroup.Name = "Test Group";

            var master = channelGroup.CreateChannel();
            Assert.IsNotNull(master);
            master.Name = "Time";
            master.Type = ChannelType.Master;
            master.Sync = ChannelSyncType.Time;
            master.DataType = ChannelDataType.FloatBe;
            master.DataBytes = 8;

            writer.InitMeasurement();
            var tickTime = (ulong)DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() * 1000000;
            writer.StartMeasurement(tickTime);
            for (ulong sample = 0; sample < 100; ++sample)
            {
                writer.SaveSample(channelGroup, tickTime);
                tickTime += 1000000;
            }
            writer.StopMeasurement(tickTime);
            writer.FinalizeMeasurement();
        }
        {
            var reader = new MdfReader(testFile);
            Assert.IsTrue(reader.IsOk);
            Assert.IsTrue(reader.ReadEverythingButData());
            reader.Close();

            var header = reader.Header;
            var meta = header.MetaData;
            Assert.IsNotNull(meta);

            Console.WriteLine(meta.XmlSnippet);

            Console.WriteLine("TX: {0}", meta.get_PropertyAsString("TX"));
            Console.WriteLine("Time Source: {0}", meta.get_PropertyAsString("time_source"));

            var props = meta.Properties;
            Assert.IsNotNull(props);
            foreach (var prop in props)
            {
                Console.WriteLine(
                    "Name: {0}, Value: {1}",
                    prop.Name,
                    prop.ValueAsString);
            }

            var commons = meta.CommonProperties;
            Assert.IsNotNull(commons);
            foreach (var tag in commons)
            {
                Console.WriteLine(
                    "Name: {0}, Desc: {1}, Unit: {2}, Ref: {3}, Type: {4}, Lang: {5}, R/O: {6}, Value: {7}",
                    tag.Name,
                    tag.Description,
                    tag.Unit,
                    tag.UnitRef,
                    tag.Type,
                    tag.Language,
                    tag.ReadOnly,
                    tag.ValueAsString);
            }
        }
    }

    [TestMethod]
    public void TestDataGroup()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var group = header.DataGroups[0];
        Assert.IsNotNull(group);

        Console.WriteLine("Index: {0}", group.Index);
        Console.WriteLine("Desc: {0}", group.Description);
        Console.WriteLine("Record ID size: {0}", group.RecordIdSize);
        Assert.IsNotNull(group.ChannelGroups);
        Assert.IsNull(group.MetaData);
        Assert.IsFalse(group.IsRead);

        Assert.IsNotNull(group.CreateChannelGroup());
        Assert.IsNotNull(group.CreateMetaData());
        Assert.IsNull(group.FindParentChannelGroup(null));

    }

    [TestMethod]
    public void TestChannelGroup()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        Assert.IsNotNull(group);

        Console.WriteLine("Index: {0}", group.Index);
        Console.WriteLine("Record ID: {0}", group.RecordId);
        Console.WriteLine("Name: {0}", group.Name);
        Console.WriteLine("Desc: {0}", group.Description);
        Console.WriteLine("Saamples: {0}", group.NofSamples);
        Console.WriteLine("Flags: {0:X}", group.Flags);
        Console.WriteLine("Sep: {0}", group.PathSeparator.ToString());

        Assert.IsNotNull(group.Channels);

    }

    [TestMethod]
    public void TestChannel()
    {
        if (!File.Exists(TestFile1))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        var channel = group.Channels[0];
        Assert.IsNotNull(channel);

        Console.WriteLine("Index: {0}", channel.Index);
        Console.WriteLine("Name: {0}", channel.Name);
        Console.WriteLine("Display Name: {0}", channel.DisplayName);
        Console.WriteLine("Description: {0}", channel.Description);
        Console.WriteLine("Unit Used: {0}", channel.UnitUsed);
        Console.WriteLine("Unit: {0}", channel.Unit);
        Console.WriteLine("Channel Type: {0}", channel.Type);
        Console.WriteLine("Sync Type: {0}", channel.Sync);
        Console.WriteLine("Data Type: {0}", channel.DataType);
        Console.WriteLine("Data Bytes: {0}", channel.DataBytes);
        Console.WriteLine("Precision Used: {0}", channel.PrecisionUsed);
        Console.WriteLine("Precision: {0}", channel.Precision);

        Console.WriteLine("Range Used: {0}", channel.RangeUsed);
        Console.WriteLine("Range Min: {0}", channel.Range.Item1);
        Console.WriteLine("Range Max: {0}", channel.Range.Item2);

        Console.WriteLine("Limit Used: {0}", channel.LimitUsed);
        Console.WriteLine("Limit Min: {0}", channel.Limit.Item1);
        Console.WriteLine("Limit Max: {0}", channel.Limit.Item2);

        Console.WriteLine("Ext Limit Used: {0}", channel.ExtLimitUsed);
        Console.WriteLine("Ext Limit Min: {0}", channel.ExtLimit.Item1);
        Console.WriteLine("Ext Limit Max: {0}", channel.ExtLimit.Item2);

        Console.WriteLine("Sampling Rate: {0}", channel.SamplingRate);

        Assert.IsNull(channel.ChannelConversion);
    }

    [TestMethod]
    public void TestSourceInformation()
    {
        if (!File.Exists(TestFile2))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var reader = new MdfReader(TestFile2);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        var info = group.SourceInformation;
        Assert.IsNotNull(info);

        Console.WriteLine("Index: {0}", info.Index);
        Console.WriteLine("Name: {0}", info.Name);
        Console.WriteLine("Desc: {0}", info.Description);
        Console.WriteLine("Path: {0}", info.Path);
        Console.WriteLine("Type: {0}", info.Type);
        Console.WriteLine("Bus: {0}", info.Bus);
        Console.WriteLine("Flags: {0:X}", info.Flags);

        Assert.IsNotNull(info.MetaData);
    }

    [TestMethod]
    public void TestChannelConversion()
    {        
        var testFile = Path.Combine(_testDirectory, "channel_conversion.mf4");
        if (File.Exists(testFile))
        {
            File.Delete(testFile);
        }

        var startTime = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
        {
            var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
            Assert.IsNotNull(writer);
            // The Init() reads in any existing file. Not in this case but
            // it also create the ID and HD block.
            Assert.IsTrue(writer.Init(testFile));

            
            var header = writer.Header;
            header.Author = "Ingemar Hedvall";
            header.Department = "Home Alone";
            header.Description = "Testing channel conversion";
            header.Project = "Mdf4CC";
            header.StartTime = startTime;
            header.Subject = "PXY";

            if (header.FileHistories.Length == 0)
            {
        
                // All MD4 files need to have at least one FH block
                var history = header.CreateFileHistory();
                history.Time = startTime;
                history.Description = "Initial stuff";
                history.ToolName = "Unit Test";
                history.ToolVendor = "ACME";
                history.ToolVersion = "2.3";
                history.UserName = "Ducky";
            }

            var dataGroup = header.CreateDataGroup();
            Assert.IsNotNull(dataGroup);

            var channelGroup = dataGroup.CreateChannelGroup();
            Assert.IsNotNull(channelGroup);
            channelGroup.Name = "Group1";
            
            var channel = channelGroup.CreateChannel();
            Assert.IsNotNull(channel);
            channel.Name = "Channel1";
            channel.DataBytes = 1;
            channel.Type = ChannelType.FixedLength;
            channel.DataType = ChannelDataType.UnsignedIntegerBe;
            
            
            var channelConv = channel.CreateChannelConversion();
            Assert.IsNotNull(channelConv);
            channelConv.Name = "Channel1Conv";
            channelConv.Description = "Test";
            channelConv.Unit = "km/h";
            channelConv.Type = ConversionType.ValueToText;
            
            // No Inverse
            channelConv.Reference(0, "Zero");
            channelConv.Reference(1, "One");   
            channelConv.Reference(2, "Default");

            channelConv.Parameter(0, 0.0);
            channelConv.Parameter(1, 1.0);

            writer.InitMeasurement();
            writer.StartMeasurement(startTime);
            var stopTime = startTime;
            for (var sample = 0; sample < 3; ++sample)
            {
                channel.SetChannelValue(sample);
                writer.SaveSample(channelGroup,stopTime);
                stopTime += 100000000;
            }
            writer.StopMeasurement(stopTime);
            writer.FinalizeMeasurement();
        }
        {
            var reader = new MdfReader(testFile);
            Assert.IsTrue(reader.ReadEverythingButData());
            reader.Close();

            var file = reader.File;
            var header = file.Header;
            var datagroup = header.LastDataGroup;
            var group = datagroup.ChannelGroups[0];
            var channel = group.Channels[0];
            var conv = channel.ChannelConversion;
            
            Assert.IsNotNull(conv);

            Console.WriteLine("Index: {0:X}", conv.Index);
            Console.WriteLine("Name: {0}", conv.Name);
            Console.WriteLine("Desc: {0}", conv.Description);
            Console.WriteLine("Unit Used: {0}", conv.UnitUsed);
            Console.WriteLine("Unit: {0}", conv.Unit);
            Console.WriteLine("Type: {0}", conv.Type);
            Console.WriteLine("Precision Used: {0}", conv.PrecisionUsed);
            Console.WriteLine("Precision: {0}", conv.Precision);
            Console.WriteLine("Range Used: {0}", conv.RangeUsed);
            Console.WriteLine("Range Min: {0}", conv.Range.Item1);
            Console.WriteLine("Range Max: {0}", conv.Range.Item2);
            Console.WriteLine("Flags: {0:X}", conv.Flags);
            Assert.IsNull(conv.Inverse);
            Assert.IsNotNull(conv.CreateInverse());
            
            Assert.AreEqual(3, conv.NofReferences);            
            Assert.AreEqual("Zero", conv.Reference(0));
            
            Assert.AreEqual(2,conv.NofParameters);
            Assert.AreEqual(1.0, conv.Parameter(1));        
        }
    }

    [TestMethod]
    public void TestBusLogging()
    {           
        const string testFile = "can_bus_logger.mf4";
        if (File.Exists(testFile))
        {
            File.Delete(testFile);
        }

        {
            var writer = new MdfWriter(MdfWriterType.MdfBusLogger);
            Assert.IsNotNull(writer);
            Assert.IsTrue(writer.Init(testFile));
            writer.BusType = MdfBusType.CAN;
            writer.StorageType = MdfStorageType.VlsdStorage;
            writer.MaxLength = 8;
            
            var header = writer.Header;
            header.Author = "Caller";
            header.Department = "Home Alone";
            header.Description = "Testing i";
            header.Project = "Mdf3WriteHD";
            header.StartTime = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
            header.Subject = "PXY";

            var history = header.CreateFileHistory(); 
            history.Time = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
            history.Description = "Initial stuff";
            history.ToolName = "Unit Test";
            history.ToolVendor = "ACME";
            history.ToolVersion = "2.3";
            history.UserName = "Ducky";

            Assert.IsTrue(writer.CreateBusLogConfiguration());
            var lastDg = header.LastDataGroup;
            Assert.IsNotNull(lastDg);

            var dataGroup = lastDg.GetChannelGroup("CAN_DataFrame");
            Assert.IsNotNull(dataGroup);
         
            Assert.IsTrue(writer.InitMeasurement());
            var timestamp = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
            writer.StartMeasurement(timestamp);
            Console.WriteLine("Start Time: {0}/{1} ", writer.StartTime, timestamp);
            Assert.AreEqual(writer.StartTime, timestamp);

            var message = new CanMessage();
            message.MessageId = 12;

            message.DataBytes = new byte[] {0x01,0x02};
            for (var sample = 0; sample < 100; ++sample)
            {
                writer.SaveCanMessage(dataGroup, timestamp, message);
                timestamp += 10000000;
            }
            writer.StopMeasurement(timestamp);
            Assert.AreEqual(writer.StopTime, timestamp);
            Assert.IsTrue(writer.FinalizeMeasurement());
        }
        {
            var reader = new MdfReader(testFile);
            Assert.IsTrue(reader.ReadEverythingButData());
            reader.Close();

            var file = reader.File;
            var header = file.Header;
            var datagroup = header.DataGroups[0];

            var time = MdfLibrary.CreateChannelObserverByChannelName(datagroup,
                "t");
            Assert.IsNotNull(time);
            var ident = MdfLibrary.CreateChannelObserverByChannelName(datagroup,
                "CAN_DataFrame.ID");
            Assert.IsNotNull(ident);
            var data = MdfLibrary.CreateChannelObserverByChannelName(datagroup,
                "CAN_DataFrame.DataBytes");
            Assert.IsNotNull(data);

            reader.Open();
            Assert.IsTrue(reader.ReadData(datagroup));
            reader.Close();



            for (uint sample = 0; sample < time.NofSamples; ++sample)
            {               
                double timeValue = 0;
                var timeValid = time.GetEngValueAsFloat(sample, ref timeValue); 
                
                ulong identValue = 0;
                var identValid = ident.GetEngValueAsUnsigned(sample, ref identValue);
                
                var dataValue = Array.Empty<byte>();
                var dataValid = data.GetEngValueAsArray(sample, ref dataValue);            
                var dataString = "";
                data.GetEngValueAsString(sample, ref dataString);
                Console.WriteLine("Valid: {0} {1} {2} {3}", sample, timeValid, identValid, dataValid);
                Console.WriteLine("Sample: {0} {1} {2} {3}", sample, timeValue, identValue, dataString);
                Console.Write("Sample: {0} ", sample);
                foreach (var val in dataValue)
                {
                    Console.Write("{0:X2} ", val);
                }

                Console.WriteLine("");
            }
        }
    }

    [TestMethod]
    public void TestWriter()
    {
        if(File.Exists(TestFile5))
        {
            File.Delete(TestFile5);
        }
        var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
        writer.Init(TestFile5);
        var header = writer.Header;
        header.Author = "Caller";
        header.Department = "Home Alone";
        header.Description = "Testing i";
        header.Project = "Mdf3WriteHD";
        header.StartTime = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
        header.Subject = "PXY";

        var history = header.CreateFileHistory();
        history.Time = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000);
        history.Description = "Initial stuff";
        history.ToolName = "Unit Test";
        history.ToolVendor = "ACME";
        history.ToolVersion = "2.3";
        history.UserName = "Ducky";

        var Event = header.CreateEvent();
        Event.GroupName = "Olle";
        Event.Description = "Olle Event";

        var dg = writer.CreateDataGroup();
        var cg = dg.CreateChannelGroup();
        cg.Name = "Test";
        cg.Description = "Test channel group";

        var si = cg.CreateSourceInformation();
        si.Name = "SI-Name";
        si.Path = "SI-Path";
        si.Description = "SI-Desc";
        si.Type = SourceType.Bus;
        si.Bus = BusType.Can;

        {
            var cn = cg.CreateChannel();
            cn.Name = "Time";
            cn.Description = "Time channel";
            cn.Type = ChannelType.Master;
            cn.Sync = ChannelSyncType.Time;
            cn.DataType = ChannelDataType.FloatLe;
            cn.DataBytes = 4;
            cn.Unit = "s";
            cn.Range = new Tuple<double, double>(0.0, 0.0);
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "UnsignedLe";
            cn.Description = "uint32_t";
            cn.Unit = "°C";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.UnsignedIntegerLe;
            cn.DataBytes = 4;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "UnsignedBe";
            cn.Description = "uint16_t";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.UnsignedIntegerBe;
            cn.DataBytes = 2;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "SignedLe";
            cn.Description = "int32_t";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.SignedIntegerLe;
            cn.DataBytes = 4;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "SignedBe";
            cn.Description = "int8_t";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.SignedIntegerBe;
            cn.DataBytes = 1;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "FloatLe";
            cn.Description = "float";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.FloatLe;
            cn.DataBytes = 4;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "FloatBe";
            cn.Description = "double";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.FloatBe;
            cn.DataBytes = 8;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "String";
            cn.Description = "string";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.StringUTF8;
            cn.DataBytes = 10;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "Array";
            cn.Description = "vector";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.ByteArray;
            cn.DataBytes = 5;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "Date";
            cn.Description = "CANopen Date";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.CanOpenDate;
        }
        {
            var cn = cg.CreateChannel();
            cn.Name = "AbsTime";
            cn.Description = "CANopen Time";
            cn.Type = ChannelType.FixedLength;
            cn.DataType = ChannelDataType.CanOpenTime;
        }

/*        var Attachment = Header.CreateAttachment();
        Attachment.CreatorIndex = 0;
        Attachment.Embedded = true;
        Attachment.Compressed = false;
        Attachment.FileName = "test.txt";
        Attachment.FileType = "text/plain";

        Attachment = Header.CreateAttachment();
        Attachment.CreatorIndex = 0;
        Attachment.Embedded = true;
        Attachment.Compressed = true;
        Attachment.FileName = "test.txt";
        Attachment.FileType = "text/plain";*/

        writer.InitMeasurement();
        var ns1970 = (ulong)(DateTimeOffset.Now.ToUnixTimeMilliseconds() * 1000000); // ns 
        writer.StartMeasurement(ns1970);
        // Write the data
        for (int i = 0; i < 50; i++)
        {
            var cns = cg.Channels;

            cns[0].SetChannelValue(0.01 * i); // Has no meaning
            cns[1].SetChannelValue(i);
            cns[2].SetChannelValue(i);
            cns[3].SetChannelValue(-i);
            cns[4].SetChannelValue(-i);
            cns[5].SetChannelValue(11.1 * i);
            cns[6].SetChannelValue(11.1 * i);

            cns[7].SetChannelValue(i.ToString());

            byte[] temp = new byte[4];
            temp[0] = (byte)i;
            temp[1] = (byte)(i + 1);
            temp[2] = (byte)(i + 2);
            temp[3] = (byte)(i + 3);
            cns[8].SetChannelValue(temp);

            cns[9].SetChannelValue(ns1970);
            cns[10].SetChannelValue(ns1970);

            writer.SaveSample(cg, (ulong)ns1970);
            ns1970 += 10000000;
        }
        writer.StopMeasurement(ns1970);
        writer.FinalizeMeasurement();

    }

    [TestMethod]
    public void TestReader2()
    {
        if (!File.Exists(TestFile4))
        {
            Console.WriteLine("File doesn't exists.");
            return;
        }
        var Reader = new MdfReader(TestFile4);
        if (!Reader.IsOk)
        {
            Assert.Inconclusive("Invalid File");
        }
        Reader.ReadEverythingButData();
        var lVersion = Reader.File.MainVersion;

        // Get file time and other infos
        DateTimeOffset ti = DateTimeOffset.FromUnixTimeMilliseconds((long)Reader.Header.StartTime / 1000000);
        Console.WriteLine($"FileTime = {ti.ToString("%y-%M-%d %h:%m:%s.%fff")}");

        string str;
        if (Reader.File.IsMdf4)
        {
            str = Reader.Header.Description;
            Console.WriteLine($"Header comment = {str}");
            str = "";
            foreach (var history in Reader.Header.FileHistories)
            {
                str += $"{history.Time} {history.UserName} {history.Description} {history.ToolName} {history.ToolVendor} {history.ToolVersion}\n";
            }
            Console.WriteLine($"File history : \n{str}");
            str = "";
            foreach (var Event in Reader.Header.Events)
            {
                str+=$"{Event.Name} {Event.Description} {Event.GroupName} {Event.SyncFactor}\n";
            }
            Console.WriteLine($"Events : \n{str}");
        }
        Console.WriteLine($"Author = {Reader.Header.Author}");
        Console.WriteLine($"Organisation = {Reader.Header.Department}");
        Console.WriteLine($"Project = {Reader.Header.Project}");
        Console.WriteLine($"Subject = {Reader.Header.Subject}");


        var groups = Reader.Header.DataGroups;
        Console.WriteLine("GetNGroups." + groups.Length);
        foreach (var group in groups)
        {
            if (Reader.File.IsMdf4)
            {
                /*Console.WriteLine("GetGroupName." + group.MetaData.);*/

            }
            // Comment from data group:
            Console.WriteLine("Comment = ", group.Description);

            Console.WriteLine("GetNChannels." + group.ChannelGroups.Length);
            foreach (var channels in group.ChannelGroups)
            {
                // Comments from channel group:
                str = channels.Name;
                Console.WriteLine("  CG name = " + str);
                str = channels.Description;
                Console.WriteLine("  CG comment = " + str);

                // Comments from channel groups SI block :
                if (channels.SourceInformation != null)
                {
                    str = channels.SourceInformation.Name;
                    Console.WriteLine("  CG SI name = " + str);
                    str = channels.SourceInformation.Path;
                    Console.WriteLine("  CG SI path = " + str);
                    str = channels.SourceInformation.Description;
                    Console.WriteLine("  CG SI description = " + str);
                }

                Console.WriteLine("GetNSignals." + channels.Channels.Length);
                Console.WriteLine("NofSamples." + channels.NofSamples);

                var subscriber_list = new MdfChannelObserver[channels.Channels.Length];
                uint i = 0;
                foreach (var channel in channels.Channels)
                {
                    Console.WriteLine($" string pszDisplayName={channel.DisplayName};string pszAliasName = {channel.Name};string pszUnit ={channel.Unit};string pszComment = ");
                    var sub = MdfLibrary.CreateChannelObserver(group, channels, channel);
                    subscriber_list[i++] = sub;
                }

                Reader.ReadData(group);
                for (i = 0; i < channels.NofSamples; ++i)
                {
                    Console.WriteLine($"Sample {i}");
                    foreach (var item in subscriber_list)
                    {
                        switch (item.Channel.DataType)
                        {
                            case ChannelDataType.CanOpenDate:
                            case ChannelDataType.CanOpenTime:
                                {
                                    ulong channel_value = 0; // Channel value (no scaling)
                                    ulong eng_value = 0; // Engineering value
                                    item.GetChannelValueAsUnsigned(i, ref channel_value);
                                    item.GetEngValueAsUnsigned(i, ref eng_value);

                                    DateTimeOffset time = DateTimeOffset.FromUnixTimeMilliseconds((long)channel_value / 1000000);
                                    Console.WriteLine($"channel_value = {time.ToString("%y-%M-%d %h:%m:%s.%fff")}");

                                    time = DateTimeOffset.FromUnixTimeMilliseconds((long)eng_value / 1000000);
                                    Console.WriteLine($"eng_value = {time.ToString("%y-%M-%d %h:%m:%s.%fff")}");
                                    break;
                                }
                            case ChannelDataType.UnsignedIntegerLe:
                            case ChannelDataType.UnsignedIntegerBe:
                                {
                                    ulong channel_value = 0; // Channel value (no scaling)
                                    ulong eng_value = 0; // Engineering value
                                    item.GetChannelValueAsUnsigned(i, ref channel_value);
                                    item.GetEngValueAsUnsigned(i, ref eng_value);
                                    Console.WriteLine($"channel_value = {channel_value}, eng_value= {eng_value}");
                                    break;
                                }
                            case ChannelDataType.SignedIntegerLe:
                            case ChannelDataType.SignedIntegerBe:
                                {
                                    long channel_value = 0; // Channel value (no scaling)
                                    long eng_value = 0; // Engineering value
                                    item.GetChannelValueAsSigned(i, ref channel_value);
                                    item.GetEngValueAsSigned(i, ref eng_value);
                                    Console.WriteLine($"channel_value = {channel_value}, eng_value= {eng_value}");
                                    break;
                                }
                            case ChannelDataType.FloatLe:
                            case ChannelDataType.FloatBe:
                                {
                                    double channel_value = 0; // Channel value (no scaling)
                                    double eng_value = 0; // Engineering value
                                    item.GetChannelValueAsFloat(i, ref channel_value);
                                    item.GetEngValueAsFloat(i, ref eng_value);
                                    Console.WriteLine($"channel_value = {channel_value}, eng_value= {eng_value}");
                                    break;
                                }
                            case ChannelDataType.StringAscii:
                            case ChannelDataType.StringUTF8:
                            case ChannelDataType.StringUTF16Le:
                            case ChannelDataType.StringUTF16Be:
                                {
                                    string channel_value = ""; // Channel value (no scaling)
                                    string eng_value = ""; // Engineering value
                                    item.GetChannelValueAsString(i, ref channel_value);
                                    item.GetEngValueAsString(i, ref eng_value);
                                    Console.WriteLine($"channel_value = {channel_value}, eng_value= {eng_value}");
                                    break;
                                }
                            case ChannelDataType.MimeStream:
                            case ChannelDataType.MimeSample:
                            case ChannelDataType.ByteArray:
                                {
                                    var channel_value = new byte[4]; // Channel value (no scaling)
                                    var eng_value = new byte[4]; // Engineering value
                                    item.GetChannelValueAsArray(i, ref channel_value);
                                    item.GetEngValueAsArray(i, ref eng_value);
                                    Console.WriteLine($"channel_value = {channel_value[0]} {channel_value[1]} {channel_value[2]} {channel_value[3]}, eng_value= {eng_value[0]} {eng_value[1]} {eng_value[2]} {eng_value[3]}");
                                    break;
                                }
                            default:
                                break;
                        }
                    }
                }
            }
            /* 
                        // SR Blocks
                        int isr, nsr = Reader.GetNoOfSRBlocks();
                        Console.WriteLine($"    {nsr} SR Blocks\n");
                        for (isr = 0; isr < nsr; isr++)
                        {
                            double dt = Reader.GetSRdt(isr);
                            int lValues = Reader.GetSRCycleCount(isr);
                            Console.WriteLine($"      SR Block {isr + 1}        N = {lValues}, dt = {dt}");
                            if (lValues > 10)
                                lValues = 10;
                            lValues = Reader.CacheSRData(isr, true, 0, lValues - 1);
                            lValues = Reader.CacheSRData(isr, false, 0, lValues - 1);
                            double Min1 = 0, Max1 = 0, Mean1 = 0;
                            double Min2 = 0, Max2 = 0, Mean2 = 0;
                            for (int iv = 0; iv < lValues; iv++)
                            {
                                Reader.GetCachedSRValues(true, iv, ref Min1, ref Max1, ref Mean1);
                                Reader.GetCachedSRValues(false, iv, ref Min2, ref Max2, ref Mean2);
                                Console.WriteLine($"        {Min1} | {Max1} | {Mean1} || {Min2} | {Max2} | {Mean2}");
                            }
                        }
                    }*/
        }
    }
    /*
    [TestMethod]
    public void TestMdf3TimeFlag()
    {
        var now = DateTime.Now;
        var localNanoTime = MdfHelper.GetLocalNanoTimestamp(now);
        {
            const string utcFilename = "mf3_utc_time.mf3";
            File.Delete(utcFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf3Basic);
            writer.Init(utcFilename);

            var header = writer.Header;
            header.SetStartTime(new MdfUtcTimestamp(now));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();

            var reader = new MdfReader(utcFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.StartTime, localNanoTime);
            var fileStartTime = readerHeader.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, localNanoTime);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
        {
            const string localFilename = "mf3_local_time.mf3";
            File.Delete(localFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf3Basic);
            writer.Init(localFilename);
            var header = writer.Header;
            header.SetStartTime(new MdfLocalTimestamp(now));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();
            
            var reader = new MdfReader(localFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.StartTime, localNanoTime);
            var fileStartTime = readerHeader.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, localNanoTime);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
        {
            const string tzFilename = "mf3_tz_time.mf3";
            File.Delete(tzFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf3Basic);
            writer.Init(tzFilename);
            var header = writer.Header;
            var dstMin = now.IsDaylightSavingTime() ? (short)60 : (short)0;
            var tzMin = (short)TimeZoneInfo.Local.GetUtcOffset(now).TotalMinutes;
            header.SetStartTime(new MdfTimezoneTimestamp(now, tzMin, dstMin));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();
            
            var reader = new MdfReader(tzFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.StartTime, localNanoTime);
            var fileStartTime = readerHeader.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, localNanoTime);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
    }
    
    [TestMethod]
    public void TestMdf4TimeFlag()
    {
        var now = DateTime.Now;
        var localNanoTime = MdfHelper.GetLocalNanoTimestamp(now);
        {
            const string utcFilename = "mf4_utc_time.mf4";
            File.Delete(utcFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
            writer.Init(utcFilename);
            var header = writer.Header;
            header.SetStartTime(new MdfUtcTimestamp(now));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();
            
            var reader = new MdfReader(utcFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.GetStartTime().TimeNs, MdfHelper.GetUnixNanoTimestamp(now));
            var fileStartTime = readerHeader.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, MdfHelper.GetUnixNanoTimestamp(now));
            Assert.AreEqual(fileStartTime.TimezoneMin, 0);
            Assert.AreEqual(fileStartTime.DstMin, 0);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
        {
            const string localFilename = "mf4_local_time.mf4";
            File.Delete(localFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
            writer.Init(localFilename);
            var header = writer.Header;
            header.SetStartTime(new MdfLocalTimestamp(now));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();
            
            var reader = new MdfReader(localFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.StartTime, localNanoTime);
            var fileStartTime = readerHeader.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, localNanoTime);
            Assert.AreEqual(fileStartTime.TimezoneMin, 0);
            Assert.AreEqual(fileStartTime.DstMin, 0);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
        {
            const string tzFilename = "mf4_tz_time.mf4";
            File.Delete(tzFilename);
            var writer = new MdfWriter(MdfWriterType.Mdf4Basic);
            writer.Init(tzFilename);
            var header = writer.Header;
            var dstMin = now.IsDaylightSavingTime() ? (short)60 : (short)0;
            var tzMin = (short)TimeZoneInfo.Local.GetUtcOffset(now).TotalMinutes;
            header.SetStartTime(new MdfTimezoneTimestamp(now, tzMin, dstMin));
            writer.InitMeasurement();
            writer.FinalizeMeasurement();
            
            var reader = new MdfReader(tzFilename);
            Assert.IsTrue(reader.ReadHeader());
            var readerHeader = reader.Header;
            Assert.AreEqual(readerHeader.StartTime, MdfHelper.GetUnixNanoTimestamp(now));
            var fileStartTime = reader.Header.GetStartTime();
            Assert.AreEqual(fileStartTime.TimeNs, MdfHelper.GetUnixNanoTimestamp(now));
            Assert.AreEqual(fileStartTime.TimezoneMin, tzMin);
            Assert.AreEqual(fileStartTime.DstMin, dstMin);
            Assert.AreEqual(fileStartTime.Time.ToLocalTime(), now);
        }
    }
    */
}