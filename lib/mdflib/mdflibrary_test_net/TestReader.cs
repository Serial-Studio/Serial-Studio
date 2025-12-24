/*
 * Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
using System;
using System.IO;
using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Runtime.CompilerServices;

namespace mdflibrary_test;
using MdfLibrary;


[TestClass]
public class TestReader
{    
    private static string _mdfDirectory = "k:/test/mdf";
    private static string _testDirectory = "";
    private static bool _skipTest = false;
    private static List<string> _testFiles = new List<string>();

    [ClassInitialize]
    public static void ClassInit(TestContext testContext) 
    {    
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";
            

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
        if (_skipTest)
        {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Error,functionName, 
                "Failed to find the test directory. Dir: " + _testDirectory);
        }
        MdfLibrary.Instance.AddLog(MdfLogSeverity.Trace, functionName, "Read tests started.");
        MdfLibrary.Instance.AddLog(MdfLogSeverity.Trace, functionName, "Test Dir: " + _testDirectory);

        _skipTest &= Directory.Exists(_mdfDirectory);
        if (_skipTest)
        {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Error, functionName, 
                "Failed to find the MDF directory. Dir: " + _mdfDirectory);
        }
        GetMdfFiles(_mdfDirectory);
        if (_testFiles.Count == 0) {
            _skipTest = true;
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Error, functionName, 
                "No MDF files found. Dir: " + _mdfDirectory);
        } else {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Trace, functionName, 
                "MDF files found. Count: " + _testFiles.Count);           
        }

    }


    [ClassCleanup]
    public static void ClassCleanup()
    {
        Console.WriteLine("Read tests exited.");
    }

    private static void GetMdfFiles(string directory)
    {
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        if (!Directory.Exists(directory)) {
            return;
        }
        DirectoryInfo dirInfo = new DirectoryInfo(directory);

        IEnumerable<FileInfo> fileList = dirInfo.EnumerateFiles("*.mf4");
        foreach (FileInfo file in fileList) { 
            _testFiles.Add(file.FullName);  
        }
        
        IEnumerable<DirectoryInfo> dirList = dirInfo.EnumerateDirectories("*");
        foreach (DirectoryInfo dir in dirList) { 
            GetMdfFiles(dir.FullName);
        }
    }
   
    [TestMethod]
    public void TestReadHeader()
    {
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        if (_skipTest) {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Info, 
                functionName, "No test files");
            return;
        }

        foreach (string file in _testFiles) {
            MdfReader reader = new MdfReader(file);
            Assert.IsNull(reader.Header);
            Assert.IsTrue(reader.IsOk);
            Assert.IsTrue(reader.ReadHeader());
            Assert.IsNotNull(reader.File);
            Assert.IsNotNull(reader.Header);
            reader.Close();
        }

    }

    [TestMethod]
    public void TestReadMeasurementInfo()
    {
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        if (_skipTest) {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Info, 
                functionName, "No test files");
            return;
        }

        foreach (string file in _testFiles) {
            MdfReader reader = new MdfReader(file);
            Assert.IsNull(reader.Header);
            Assert.IsTrue(reader.IsOk);
            Assert.IsTrue(reader.ReadMeasurementInfo());
            Assert.IsNotNull(reader.Header);
            Assert.IsNotNull(reader.get_DataGroup(0));
            reader.Close();
        }

    }

    [TestMethod]
    public void TestReadEverythingButData()
    {
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        if (_skipTest) {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Info, 
                functionName, "No test files");
            return;
        }

        foreach (string file in _testFiles) {
            MdfReader reader = new MdfReader(file);
            Assert.IsNull(reader.Header);
            Assert.IsTrue(reader.IsOk);
            Assert.IsTrue(reader.ReadEverythingButData());
            Assert.IsNotNull(reader.Header);
            MdfFile mdfFile = reader.File;
            Assert.IsNotNull(mdfFile);

            reader.Close();
        }

    }

    [TestMethod]
    public void TestReadData()
    {
        MethodBase? method = MethodBase.GetCurrentMethod();
        string functionName = method is not null ? method.Name : "";

        if (_skipTest) {
            MdfLibrary.Instance.AddLog(MdfLogSeverity.Info, 
                functionName, "No test files");
            return;
        }
        uint countFiles = 0;
        foreach (string file in _testFiles) {
            FileInfo fileInfo = new FileInfo(file);
            if (fileInfo.Length > 1e6) { // Avoid to large files
                continue;
            }
            MdfReader reader = new MdfReader(file);
            Assert.IsNull(reader.Header);
            Assert.IsTrue(reader.IsOk);
            Assert.IsTrue(reader.ReadEverythingButData());
            Assert.IsNotNull(reader.Header);
            MdfFile mdfFile = reader.File;
            Assert.IsNotNull(mdfFile);
            MdfDataGroup[] dataGroups = mdfFile.DataGroups; 
            Assert.IsNotNull(dataGroups);
            foreach (MdfDataGroup dataGroup in dataGroups) {
                MdfChannelObserver[] observerList = 
                    MdfLibrary.CreateChannelObserverForDataGroup(dataGroup);
                if (observerList.Length == 0) {
                    MdfLibrary.Instance.AddLog(MdfLogSeverity.Info, functionName,
                        "No observer for data group. File: " + fileInfo.Name);
                    continue;
                }
                reader.ReadData(dataGroup);
            }

            reader.Close();
            ++countFiles;
        }
        MdfLibrary.Instance.AddLog(MdfLogSeverity.Trace, functionName,
            "Read data from files. Number of Files: " + countFiles);

    }
}
