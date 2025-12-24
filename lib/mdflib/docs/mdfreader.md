---
layout: default
title: MDF Reader
---
# MDF Reader

The MDF reader simply read and parse a Measurement Data File (MDF). The file format is
specified by the ASAM standard organisation and an overview of the file format can be found
here: [ASAM MDF Wiki](https://www.asam.net/standards/detail/mdf/wiki/).

The reader object handles all versions of MDF. The object is either created through the MDFFactory class or simply by 
declaring the reader. Example of usage below:

~~~~c++

{% include_relative mdfreaderexample.cpp %}
~~~~

## Reading from the File

When the reader object is created, the file is open and normally the application 
should do an initial read. Reading from disc may take some time. Therefor the 
user may choose from 3 types of initial reads.

- **ReadHeader()**. This function reads in the basic header and basic file information as file history and attachments. This
read is fast but no information about the measurements are read.
- **ReadMeasurementInfo()**. The function do the above and also the measurement (data groups) and its 
sub-measurements (channel groups), but no information about the channels.
- **ReadEverythingButData()**. Does the above but reads in everything but not raw sample and attachment data.
This function is the normal to use as you will get all channel and event information. 

After the initial read the file may be closed and opened later when reading sample or attachment data.

## Attachment Data

Embedded attachment data can be extracted by the ExportAttachmentData(). Supply the attachment object and
a destination file name.

## Reading Samples 
After the initial read, the channels configuration should be available. Before reading the samples, the user
needs to define which channels that should be read. This is done by creating a list of channel subscriptions 
(ISampleObserver). The subscriber objects are responsible to hold all sample values.

After the subscribers are created, the user normally calls the **ReadData()**. This call fills the subscriber with sample
values. This call may take some time and consume memory.

## Read Some Samples
Reading all samples may take very long time but if the files starts to be bigger than 1 GB, the physical memory for 
the read may cause out of memory conditions. To solve that problem, only partial read can be performed.

The **ReadPartialData()** is similar to the **ReadData()** function but the user needs to supply a min and max sample index.
Note that this function still uses the subscribers but just fills the requested samples. This function is much faster
that to read all samples with the ReadData() function.

## Read Sample Reduction Data
If the user wants to read the sample reduction (SR) data fast without reading the sample data, the ReadSrData()
function shall be used. 

## Read VLSD data
Big data files typically stores big data arrays of variable length. Typical are bus, video and picture data. These 
files are typical so large that primary memory cannot store all samples. The **ReadVlsdData()** solves this problem. 
It is similar to reading partial data but it operates on VLSD data. 

The user must first read in a sample to VLSD offset list. This is done by first setting the channels ReadVlsdData 
property to false and then read all samples. The channels data now holds the offset into the VLSD byte array. Using the 
offset list and call the **ReadVlsdData()** function. Note that user needs the supply callback function that is called 
for each sample.