/*
 * Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
using System;
using System.IO;
using System.Reflection;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;
using Microsoft.VisualStudio.TestPlatform.CommunicationUtilities.ObjectModel;

namespace mdflibrary_test;
using MdfLibrary;

[TestClass]
public class TestCanMessage
{
    [TestMethod]
    public void TestProperties()
    {
        CanMessage msg = new CanMessage();
        msg.TypeOfMessage = CanMessageType.Can_RemoteFrame;
        Assert.AreEqual(msg.TypeOfMessage, CanMessageType.Can_RemoteFrame);

        msg.MessageId = 123u;
        Assert.AreEqual(msg.MessageId, 123u);
        Assert.AreEqual(msg.CanId, 123u);
        Assert.IsFalse(msg.ExtendedId);
        msg.ExtendedId = true;
        Assert.IsTrue(msg.ExtendedId);
        
        msg.Dlc = 8;
        Assert.AreEqual(msg.Dlc, 8u);
   
        byte[] data = [ 1, 2, 3, 4, 5, 6, 7, 8 ];
        msg.DataBytes = data;
        Assert.AreEqual(msg.DataBytes.Length, 8);
        CollectionAssert.AreEqual(msg.DataBytes, data);
        Assert.AreEqual(msg.DataLength, 8u);
        
        msg.Dir = false;
        Assert.IsFalse(msg.Dir);
        msg.Dir = true;
        Assert.IsTrue(msg.Dir);        
        
        msg.Srr = false;
        Assert.IsFalse(msg.Srr);
        msg.Srr = true;
        Assert.IsTrue(msg.Srr); 
        
        msg.Brs = false;
        Assert.IsFalse(msg.Brs);
        msg.Brs = true;
        Assert.IsTrue(msg.Brs);     
        
        msg.Esi = false;
        Assert.IsFalse(msg.Esi);
        msg.Esi = true;
        Assert.IsTrue(msg.Esi);
        
        msg.Rtr = false;
        Assert.IsFalse(msg.Rtr);
        msg.Rtr = true;
        Assert.IsTrue(msg.Rtr);        
        
        msg.WakeUp = false;
        Assert.IsFalse(msg.WakeUp);
        msg.WakeUp = true;
        Assert.IsTrue(msg.WakeUp);
        
        msg.SingleWire = false;
        Assert.IsFalse(msg.SingleWire);
        msg.SingleWire = true;
        Assert.IsTrue(msg.SingleWire);
    
        msg.R0 = false;
        Assert.IsFalse(msg.R0);
        msg.R0 = true;
        Assert.IsTrue(msg.R0);        
    
        msg.R1 = false;
        Assert.IsFalse(msg.R1);
        msg.R1 = true;
        Assert.IsTrue(msg.R1);          
    
        msg.BusChannel = 56;
        Assert.AreEqual(msg.BusChannel, 56 );
        
        msg.BitPosition = 312;
        Assert.AreEqual(msg.BitPosition, 312 );

        msg.ErrorType = CanErrorType.BIT_ERROR;
        Assert.AreEqual(msg.ErrorType, CanErrorType.BIT_ERROR);
        
        msg.FrameDuration = 3124u;
        Assert.AreEqual(msg.FrameDuration, 3124u );
        
        msg.Crc = 1234u;
        Assert.AreEqual(msg.Crc, 1234u );      
        
        msg.Timestamp = 1.23;
        Assert.AreEqual(msg.Timestamp, 1.23 );   
    }
}
