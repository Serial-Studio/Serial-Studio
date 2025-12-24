---
layout: default
title: MDF Writer
---

# MDF Writer

The MDF Writer object creates Measurement Data Files. The file format is
specified by the ASAM standard organisation and an overview of the file format can be found
here: [ASAM MDF Wiki](https://www.asam.net/standards/detail/mdf/wiki/).

Unlike the MDF reader that is somewhat general, the writer exist in several application types.
The following writer application types exist.

- **BasicMdf3Writer**. This writer creates an MDF file according to the version 3 of the standard (Original Bosch). It is 
useful when producing files with just measurement data and most external tools can handle this format. It assumes that
the samples are received in time order (live recording).
- **BasicMdf4Writer**. Produces an MDF file according to version 4. The user may add both metadata information and 
file attachments. It assumes that the samples are received in time order (live recording).
- **MdfBusLogger**. The MDF 4 version defines how a bus logger should be configured and how the samples should be 
stored. It assumes that the samples are received in time order (live recording).
- **MdfConverter**. It stores samples according to version 4 but unlike the above writes it doesn't need the the 
samples in time order. It is typical used in conversion applications.

All writer types can compress the data and define types of storage (Fixed, VLSD or MLSD).


## Creating a File

Creating an MDF file require some knowledge of how the file is formatted and which blocks exist. The bus loggers
are the simplest to configure as the **CreateBusLogConfiguration()** function creates the needed blocks. Adding CAN 
messages as samples is done by the **SaveCanMessage()** call instead of the **SaveSample()** function.

I case of a non-bus application, the user must do the channel configuration by itself. A good idea is to take a look 
at the unit test code (TestWrite.cpp). 

First create the preferred type of writer. Then initialize the file by assigning the writer object a file name. 
If the file exist, it read in the existing blocks in the file, so it is possible to append the file.

Assuming the file is created, the writer file object now consist of an ID and HD (header) block. The user shall
now add at least a file history block, the metadata and configure the active measurement with channels and channel 
groups. Note that this requires some knowledge about the MDF file structure,

The file is now prepare for receiving samples. Set the pre-trig time and initialize the measurement. Now start
adding samples. This is done by setting engineering values to each channel and then adding the channel group
to the save sample function. 

Note that nothing is stored into a file yet. The internal cache stores all samples in an internal cache which size 
is set by the pre-trig time. The user now select to start the measurement and later stopping the measurement. 
Note that between initializing and stopping the measurement, only data samples are stored onto the disc.

After stopping the measurement, the user may add some more information as events and attachments. The 
measurement is finished by calling finalize measurement function.

